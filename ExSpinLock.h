#ifndef __EX_SPIN_LOCK_H_HASBEEN_INCLUDED__
#define __EX_SPIN_LOCK_H_HASBEEN_INCLUDED__

#include <atomic>
#include <thread>

template<typename TypeFlag = unsigned>
class READ_WRITE_LOCK
{
	std::atomic<TypeFlag> Locker;
public:
	inline READ_WRITE_LOCK(): Locker(1) {}

	inline bool TryLockRead()
	{
		TypeFlag v = Locker; 
		if (v != 0) 
			return Locker.compare_exchange_strong(v, v + 1); 
		return false; 
	}
	inline void LockRead()
	{
		TypeFlag v;
		do{
			v = Locker; 
			if (v == 0) continue;
		}while(!Locker.compare_exchange_strong(v, v + 1)); 
	}
	inline void LockReadYield()
	{
		TypeFlag v;
		do{
			v = Locker; 
			if (v == 0)
			{
				std::this_thread::yield();
				continue;
			}
		}while(!Locker.compare_exchange_strong(v, v + 1)); 
	}
	inline void UnlockRead() { --Locker; }

	inline bool TryLockWrite()
	{
		TypeFlag v = 1;
		return Locker.compare_exchange_strong(v, 0);
	}
	inline void LockWrite() { for(TypeFlag v = 1; !Locker.compare_exchange_strong(v, 0); v = 1); }
	inline void LockWriteYield()
	{
		for(TypeFlag v = 1; !Locker.compare_exchange_strong(v, 0); v = 1)
			std::this_thread::yield();
	}
	inline void UnlockWrite() { Locker = 1; }
};



/*

	[ThreadOwner]
		EnterSafeRegion(); //Wait while some another thread working with data

	[thread1]
		OccupyRead();			//Occupy place
		WaitRegionYield();	//Wait until ThreadOwner reaches EnterSafeRegion()
		making some jobs...
		ReleaseRead();			//Release place

	[thread2]
		OccupyWrite();
		WaitRegionYield();
		making some jobs...
		ReleaseWrite();
*/
template<typename TypeFlag = unsigned>
class SAFE_REGION
{
	std::atomic<TypeFlag> SafeRegionWaiter;
	static const TypeFlag TstBit = 1 << (sizeof(TypeFlag) * 8 - 1);
public:
	SAFE_REGION(): SafeRegionWaiter(0) {}

	bool EnterSafeRegion()
	{   
		/*	
		SafeRegionWaiter = 0 - Wait writing thread
		SafeRegionWaiter = 1 - Not have waiting threads
		SafeRegionWaiter = 2..n - Have waiting read threads
		SafeRegionWaiter & 0x800..0 - threadowner wait some operations
		*/
		if(SafeRegionWaiter != 1)
		{
			SafeRegionWaiter |= TstBit;
			for(TypeFlag v = (TstBit + 1); !SafeRegionWaiter.compare_exchange_strong(v, 1); v = (TstBit + 1))
				std::this_thread::yield();
			return true;
		}
		return false;
	}

	inline bool TryOccupyRead()
	{
		TypeFlag v = SafeRegionWaiter; 
		if (v & ~TstBit) 
			return SafeRegionWaiter.compare_exchange_strong(v, v + 1); 
		return false; 
	}
	void OccupyRead() { while(!TryOccupyRead()); }
	void OccupyReadYield() { while(!TryOccupyRead()) std::this_thread::yield(); }
	void ReleaseRead() { --SafeRegionWaiter; }

	inline bool TryOccupyWrite()
	{
		TypeFlag v = 1; 
		return SafeRegionWaiter.compare_exchange_strong(v, 0); 
	}
	void OccupyWrite() { while(!TryOccupyWrite()); }
	void OccupyWriteYield() { while(!TryOccupyWrite()) std::this_thread::yield(); }
	void ReleaseWrite() { ++SafeRegionWaiter; }

	bool TryWaitRegion() const { return SafeRegionWaiter & TstBit; }
	void WaitRegion() const { while(!TryWaitRegion()); }
	void WaitRegionYield() const { while(!TryWaitRegion()) std::this_thread::yield(); }
};

#endif