#ifndef __FILE_CACHE_H_HAS_INCLUDED__
#define __FILE_CACHE_H_HAS_INCLUDED__

#include <fcntl.h>
#include <sys/types.h>
#include <io.h>
#include <sys/stat.h>
#include <time.h>

#include "ExDynamicBuf.h"
#include "ExHashTableDynamic.h"
#include "ExSpinLock.h"


#undef max


#if defined(_DEBUG)
#define CHECK_RATING_LIST \
	if(!CheckRatingList()) { throw "Rating list has been corrupted!"; }
#else
#define CHECK_RATING_LIST 
#endif


/*
CACHE_INFO must have methods:
void Cache(const char* Path, void* Buf, size_t SizeBuf, time_t LastModifTime)
void Uncache(const char* Path, void* Buf, size_t SizeBuf, time_t LastModifTime)
*/

struct DEFAULT_CACHE_DATA
{
	void Cache(const char* Path, void* Buf, size_t SizeBuf, time_t LastModifTime) const {}
	void Recache(const char* Path, void* Buf, size_t SizeBuf, time_t LastModifTime) const {}
	void Uncache(const char* Path, void* Buf, size_t SizeBuf, time_t LastModifTime) const {}
};

template<typename CACHE_INFO = DEFAULT_CACHE_DATA>
class FILE_CACHE
{
public:
	typedef enum 
	{
		OK,
		NOT_HAVE_FILE,
		NOT_ALLOC_MEM,
		FILE_OUT_OF_MAX_MEM_BOUND,
		FILE_NOT_HAVE_IN_CACHE,
		FILE_HAS_BEEN_RESIZED
	} STAT;
private:
	struct CACHED_FILE;
	typedef std::chrono::system_clock			TIME;
	typedef TIME::duration						DURATION;
	typedef TIME::time_point					TIME_POINT;
	typedef HASH_TABLE_DYN<CACHED_FILE, size_t> TYPE_TABLE;



	struct OPENED_FILE
	{
		time_t LastModifTime;
		size_t SizeFile;
		int	   Descriptor;
		inline OPENED_FILE(): Descriptor(-1) { } 
		inline ~OPENED_FILE() { if(Descriptor != -1) close(Descriptor); }

		bool Open(const char* Name)
		{
			if(Descriptor != -1) close(Descriptor);
			Descriptor = open(Name, O_RDONLY|O_BINARY);
			struct stat s;
			if((Descriptor == -1) || (stat(Name, &s) != 0)) return false;
			SizeFile = s.st_size;
			LastModifTime = s.st_mtime;
			return true;
		}

	};

	struct CACHED_FILE: public CACHE_INFO
	{	
	private:
		friend FILE_CACHE;
		friend TYPE_TABLE;
		friend FAST_ALLOC;

		CACHED_FILE()
		{
			CountReaded = 1;
			Path = nullptr;
			Buffer = nullptr;
		}

		~CACHED_FILE()
		{
			if(Buffer != nullptr) ___free(Buffer);
			if(Path != nullptr) free(Path);
		}

		bool SetKey(const char* Name)
		{
			Path = strdup(Name);
			size_t h = 0;
			for(const char* k = Path; *k != '\0'; k++) h = 31 * h + *k;
			PathHash = h;
			return true;
		}

		static size_t IndexByKey(const char* Key, size_t MaxCount)
		{
			size_t h = 0;
			for(const char* k = Key; *k != '\0'; k++) h = 31 * h + *k;
			return h % MaxCount; 
		}
		static size_t IndexByKey(CACHED_FILE* Key, size_t MaxCount) { return Key->PathHash % MaxCount; }
		bool CmpKey(CACHED_FILE* Key) const { return (Key->PathHash == PathHash) && (strcmp(Path, Key->Path) == 0); }
		size_t IndexInBound(size_t MaxCount) const { return PathHash % MaxCount; }
		bool CmpKey(const char* Key) const { return strcmp(Key, Path) == 0; }

		void RatingUp(FILE_CACHE* Cache)
		{
			auto n = Next;
			if(n != &Cache->RatingList) 
			{	
				auto p = Prev;
				n->Prev = p;
				Next = n->Next;
				Next->Prev = this;
				n->Next = this;
				Prev = n;
				p->Next = n;	
			}
			if(Cache->CachedListEnd == &Cache->RatingList)
				Cache->CachedListEnd = this;
			else if(Cache->CachedListEnd == this)
				Cache->CachedListEnd = Prev;
		}

		bool UpdateInfo()
		{
			struct stat s;
			if(stat(Path, &s) != 0) return false;
			SizeFile = s.st_size;
			LastModifTime = s.st_mtime;
			return true;
		}

		void AddInList(FILE_CACHE* Cache)
		{
			Next = Cache->RatingList.Next;
			Next->Prev = Cache->RatingList.Next = this;
			Prev = &Cache->RatingList;
			Cache->CountInUncached++;
		}

		void RemoveFromList(FILE_CACHE* Cache)
		{
			Next->Prev = Prev;
			Prev->Next = Next;
			if(Cache->CachedListEnd == this)
				Cache->CachedListEnd = Next;
			if(Buffer == nullptr) 
				Cache->CountInUncached--;
			else
				Cache->CurSize -= SizeFile;
		}

		STAT DoCache(OPENED_FILE* File)
		{
			if((Buffer = ___malloc(File->SizeFile)) == nullptr) return STAT::NOT_ALLOC_MEM;
			int Readed = read(File->Descriptor, Buffer, File->SizeFile);
			SizeFile = Readed;
			LastModifTime = File->LastModifTime;
			CACHE_INFO::Cache(Path, Buffer, SizeFile, LastModifTime);
			return STAT::OK;
		}

		STAT Recache(OPENED_FILE* File)
		{
			Locker.LockWrite();
			void* NewPlace = ___realloc(Buffer, File->SizeFile);
			if(NewPlace == nullptr) 
			{
				Locker.UnlockWrite();
				return STAT::NOT_ALLOC_MEM;
			}
			Buffer = NewPlace;
			int Readed = read(File->Descriptor, Buffer, File->SizeFile);
			SizeFile = Readed;
			LastModifTime = File->LastModifTime;
			CACHE_INFO::Recache(Path, Buffer, SizeFile, LastModifTime);
			Locker.UnlockWrite();
			return STAT::OK;
		}

		void UpdateStat(TIME_POINT CurTime, FILE_CACHE* Cache)
		{
			if((LastTestTime + Cache->PeriodUpdateStat) < CurTime)
			{
				LastTestTime = CurTime;
				CountReaded = 0;
			}
		}

		CACHED_FILE* Next;
		CACHED_FILE* Prev;
	public:
		float CountReadPerSec(TIME_POINT CurTime) const
		{
			if(Path == nullptr) return std::numeric_limits<float>::max();
			double TimePassed = std::chrono::duration_cast<std::chrono::milliseconds>(CurTime.time_since_epoch()).count();
			TimePassed -= std::chrono::duration_cast<std::chrono::milliseconds>(LastTestTime.time_since_epoch()).count();
			return (TimePassed < 100.0) ? std::numeric_limits<float>::max(): (CountReaded / TimePassed * 1000.0);
		}
		char*		Path;
		void*		Buffer;
		size_t		PathHash;
		size_t		SizeFile;
		time_t		LastModifTime;
		TIME_POINT	LastTestTime;
		intptr_t	CountReaded;
		READ_WRITE_LOCK<unsigned short> Locker;
	};

public:

	class READ_INTERATOR
	{
		friend FILE_CACHE;
		CACHED_FILE* CachedFile;
		void Set(CACHED_FILE* n)
		{
			CachedFile = n;
			CachedFile->Locker.LockRead();
		}
	public:
		inline READ_INTERATOR(): CachedFile(nullptr) { }	
		inline ~READ_INTERATOR() { Release(); }
		READ_INTERATOR(const READ_INTERATOR&) throw(char*)
		{ 
			throw "Not copy this object!";
		}

		READ_INTERATOR& operator=(const READ_INTERATOR&) throw(char*)
		{ 
			throw "Not copy this object!"; 
			return *this;
		}
		void Release() 
		{ 
			if(CachedFile != nullptr) 
			{
				CachedFile->Locker.UnlockRead(); 
				CachedFile = nullptr; 
			}  
		}

		const void* Buf() const { return (CachedFile == nullptr)? nullptr: CachedFile->Buffer; }
		size_t SizeBuf() const { return (CachedFile == nullptr)? 0: CachedFile->SizeFile; }
		const char* Path() const { return (CachedFile == nullptr)? nullptr: CachedFile->Path; }
		time_t LastModificate() const { return (CachedFile == nullptr)? 0: CachedFile->LastModifTime; }
		CACHE_INFO* UserData() const { return CachedFile; }
	};

private:

	CACHED_FILE*	CachedListEnd;
	CACHED_FILE		RatingList;
	TYPE_TABLE		Table;
	float			PerSecReadAdding;
	size_t			MaxInPreparedList;
	size_t			MaxSizeBuff;
	size_t			CurSize;
	size_t			CountInUncached;
	std::chrono::seconds PeriodUpdateStat;
	READ_WRITE_LOCK<size_t> Locker;


	template<typename T>
	CACHED_FILE* InsertInTable(T arg)
	{	
		if(Table.IsFull) Table.ResizeBeforeInsert((Table.Count < 3)? 3: (TYPE_TABLE::TINDEX)(Table.Count * 1.61803398875f));
		return Table.Insert(arg);
	}

	template<typename T>
	void RemoveFromTable(T arg)
	{	
		if(Table.Remove(arg) != nullptr)
		{	
			if((TYPE_TABLE::TINDEX)(Table.Count * 1.7f) < Table.AllocCount)
				Table.ResizeAfterRemove();
		}
	}

	bool AddFile(const char* Path, TIME_POINT CurTime)
	{
		struct stat s;
		if(stat(Path, &s) != 0) return false;
		if(s.st_size >= MaxSizeBuff) return false;
		if(CountInUncached >= MaxInPreparedList)
		{
			if(((RatingList.Next->LastTestTime + std::chrono::milliseconds(1000)) >= CurTime)
				|| (RatingList.Next->CountReadPerSec(CurTime) >= PerSecReadAdding)) return false;

			auto t = RatingList.Next;
			RemoveFile(t);

			auto NewFileReg = InsertInTable(Path);
			if(NewFileReg == nullptr) return false;
			NewFileReg->SizeFile = s.st_size;
			NewFileReg->LastModifTime = s.st_mtime;
			NewFileReg->LastTestTime = CurTime;
			NewFileReg->AddInList(this);
		}else
		{
			auto NewFileReg = InsertInTable(Path);
			if(NewFileReg == nullptr) return false;
			NewFileReg->SizeFile = s.st_size;
			NewFileReg->LastModifTime = s.st_mtime;
			NewFileReg->LastTestTime = CurTime;
			NewFileReg->AddInList(this);
		}
		CHECK_RATING_LIST;
		return true;
	}

	bool BeforeRead(CACHED_FILE* CachFile, TIME_POINT CurTime)
	{
		float CachFileReadPerSec = CachFile->CountReadPerSec(CurTime);
		if((CachFile->Buffer == nullptr) && (CachFile->Next == CachedListEnd))
		{
			if(((CachFile->SizeFile + CurSize) >= MaxSizeBuff) && 
				((CachFile->Next->CountReadPerSec(CurTime) >= CachFileReadPerSec) ||
				(CachFileReadPerSec == std::numeric_limits<float>::max())))
				return true;

			OPENED_FILE File;
			if(!File.Open(CachFile->Path))
			{
				RemoveFile(CachFile);
				return false;
			}
			if((CachFile->SizeFile + CurSize) >= MaxSizeBuff)
			{
				if(!ClearPlaceForFirstInUncached(CurTime)) return true;
			}
			if(CachFile->DoCache(&File) != STAT::OK) return true;
			CachFile->RatingUp(this);
			CurSize += CachFile->SizeFile;
			CountInUncached--;
			CHECK_RATING_LIST;
			return true;
		}
		if(CachFile->Next->CountReadPerSec(CurTime) < CachFileReadPerSec)
			CachFile->RatingUp(this);
		CHECK_RATING_LIST;
		return true;
	}

	bool ClearPlaceForFirstInUncached(TIME_POINT CurTime)
	{
		intptr_t CommonDelSize = MaxSizeBuff - CurSize;
		CACHED_FILE * CachFile = CachedListEnd->Prev;
		float CachFileReadPerSec = CachFile->CountReadPerSec(CurTime);

		auto c = CachedListEnd;
		for(; c != &RatingList; c = c->Next)
		{
			if(CachFile->SizeFile <= CommonDelSize) break;
			if(c->CountReadPerSec(CurTime) < CachFileReadPerSec)
				CommonDelSize += c->SizeFile;
			else
				break;
		}
		if(CachFile->SizeFile <= CommonDelSize)
		{
			for(CACHED_FILE* i = CachedListEnd, *t; i != c; i = t)
			{
				t = i->Next;
				RemoveFile(i);
			}
		}else
		{
			return false;
		}
		return true;
	}

	void RemoveFile(CACHED_FILE* CachFile)
	{
		CachFile->Locker.LockWrite();
		CachFile->CACHE_INFO::Uncache(CachFile->Path, CachFile->Buffer, CachFile->SizeFile, CachFile->LastModifTime);
		CachFile->RemoveFromList(this);
		RemoveFromTable(CachFile);
	}

	void UpdateFileCached(CACHED_FILE* CachFile)
	{
		if(CachFile->Buffer == nullptr)
		{
			if(!CachFile->UpdateInfo()) 
				RemoveFile(CachFile);
		}else
		{
			OPENED_FILE File;
			if(!File.Open(CachFile->Path))
			{
				RemoveFile(CachFile);
			}else if(CachFile->LastModifTime != File.LastModifTime)
			{
				intptr_t Diffr = File.SizeFile - CachFile->SizeFile;
				if((CachFile->SizeFile < File.SizeFile) && ((Diffr + CurSize) >= MaxSizeBuff))
					RemoveFile(CachFile);
				else if(CachFile->Recache(&File) == STAT::OK)
					CurSize += Diffr;
				else
					RemoveFile(CachFile);
			}	
		}
	}


	bool CheckRatingList()
	{
		for(auto i = CachedListEnd; i != &RatingList; i = i->Next)
		{
			if(i->Buffer == nullptr)
				return false;
		}
		for(auto i = CachedListEnd->Prev; i != &RatingList; i = i->Prev)
		{
			if(i->Buffer != nullptr)
				return false;
		}
		return true;
	}
public:

	FILE_CACHE()
	{
		CachedListEnd = &RatingList;
		CurSize = 0;
		MaxSizeBuff = 1024 * 1024 * 100; //100 mb
		MaxInPreparedList = 30;
		CountInUncached = 0;
		PeriodUpdateStat = std::chrono::seconds(5);
		PerSecReadAdding = 1.f;

		RatingList.SizeFile = 0;
		RatingList.CountReaded = 1;
		RatingList.PathHash = 0;
		RatingList.LastTestTime = TIME_POINT::max();
		RatingList.Prev = RatingList.Next = &RatingList;
	}


	size_t GetEmployedSize() const { return CurSize; }
	size_t GetMaxSize() const { return MaxSizeBuff; }
	void SetMaxSize(size_t NewSize)
	{
		Locker.LockWriteYield();
		if(MaxSizeBuff >= NewSize)
		{
			MaxSizeBuff = NewSize;
			Locker.UnlockWrite();
			return;
		}
		for(auto i = CachedListEnd; i != &RatingList; )
		{
			if(CurSize <= NewSize) break;
			auto t = i->Next;
			RemoveFile(i);
			i = t;
		}
		MaxSizeBuff = NewSize;
		Locker.UnlockWrite();
	}

	std::chrono::seconds GetPeriodUpdateStat() const { return PeriodUpdateStat; }
	void SetPeriodUpdateStat(std::chrono::seconds NewPeriod)
	{
		Locker.LockWriteYield();
		PeriodUpdateStat = NewPeriod;
		Locker.UnlockWrite();
	}

	float GetPerSecReadAdding() const { return PerSecReadAdding; }
	void SetPerSecReadAdding(float NewPeriod) { PerSecReadAdding = NewPeriod; }

	size_t GetMaxCountOfPrepared() const { return MaxInPreparedList; }
	void SetMaxCountOfPrepared(size_t NewPeriod) { MaxInPreparedList = NewPeriod; }

	bool ReadFile(const char* Path, READ_INTERATOR& Interator)
	{
		Interator.Release();
		Locker.LockWriteYield();
		auto r = Table.Search(Path);
		TIME_POINT CurTime = TIME::now();
		if(r == nullptr)
		{
			AddFile(Path, CurTime);
		}else
		{
			r->UpdateStat(CurTime, this);
			r->CountReaded++;
			if(BeforeRead(r, CurTime))
			{
				if(r->Buffer != nullptr)
				{				
					Interator.Set(r);
					Locker.UnlockWrite();
					return true;
				}
			}
		}			
		Locker.UnlockWrite();
		return false;
	}

	void UpdateAllCache()
	{
		Locker.LockWriteYield();
		for(auto i = RatingList.Next; i != &RatingList; )
		{
			auto t = i->Next;
			UpdateFileCached(i);
			i = t;
		}
		CHECK_RATING_LIST;
		Locker.UnlockWrite();
	}


	void UpdateFile(const char* Path)
	{
		Locker.LockWriteYield();
		auto r = Table.Search(Path);
		if(r != nullptr)
			UpdateFileCached(r);
		CHECK_RATING_LIST;
		Locker.UnlockWrite();
	}

	void Clear()
	{
		Locker.LockWriteYield();
		for(auto i = RatingList.Next; i != &RatingList; )
		{
			auto t = i->Next;
			RemoveFile(i);
			i = t;
		}
		CHECK_RATING_LIST;
		Locker.UnlockWrite();
	}
	void Uncache(const char* Path)
	{
		Locker.LockWriteYield();
		auto r = Table.Search(Path);
		if(r != nullptr)
		{
			RemoveFile(r);
		}
		CHECK_RATING_LIST;
		UpdateFileCached(r);
		Locker.UnlockWrite();
	}
};


typedef FILE_CACHE<DEFAULT_CACHE_DATA> FILE_CACHE_DEFAULT;

#endif