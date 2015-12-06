#ifndef __NEURALNET_H_HAS_INCLUDED__
#define __NEURALNET_H_HAS_INCLUDED__

#include <malloc.h>
#include <omp.h>
#include <math.h>
#include <stdlib.h>

#include "ExTypeTraits.h"


template<typename TypeNum = int>
class NEURALNET
{
public:
	typedef TypeNum TWEIGTHS;

	typedef TypeNum (__fastcall * TACTIVATE_FUNC)(TypeNum);
	typedef TypeNum (__fastcall * TDER_ACTIVATE_FUNC)(TypeNum);
	typedef TypeNum (__fastcall * TREVERSE_ACTIVATE_FUNC)(TypeNum);
	struct ACTIV_FUNCTIONS
	{

		static TypeNum __fastcall ReverseSig(TypeNum In) 
		{
			if(In < 0)
				return -log(TypeNum(1) /TypeNum(0.000000001) - TypeNum(1));
			if(In > 1)
				return -log(TypeNum(1) /TypeNum(1) - TypeNum(1));
			return -log(TypeNum(1) / In - TypeNum(1));
		};
		static TypeNum __fastcall Sig(TypeNum In) { return TypeNum(1) / (TypeNum(1) + exp(-In)); } //Сигмоидальная активизационная функция
		static TypeNum __fastcall DerSig(TypeNum In) { TypeNum e = exp(In); return e / (e + 1) * (e + 1); } //Производная сигмоидальной функции
	};

private:

	class NEURAL_LAYER
	{
		friend NEURALNET;
		class __NEURON
		{

		public:
			inline __NEURON(NEURAL_LAYER* p, size_t n) { Count.v = p; Count.i = n; }

			union{
				class{
					friend __NEURON;
					struct {NEURAL_LAYER* v; size_t i;};
				public:
					inline operator size_t() const { return v->get_count_prev(); }
				} Count; //Get count of input weight

				class{
					struct {NEURAL_LAYER* v; size_t i;};
				public:
					operator size_t ()
					{
					    size_t r = 0;
						TypeNum* vn = v->get_row() + i * v->get_count_prev();
						TypeNum Min = vnv[r]; 
						for(size_t i = 1, mc = v->get_count_prev();i < mc;i++)
						{
							if(vn[i] < Min)
							   Min = vn[r = i];
						}
						return r;
					}
				} IndexMinWeigth; //Get index with minimum weigth

				class{
					struct {NEURAL_LAYER* v; size_t i;};
				public:
					operator size_t ()
					{
					    size_t r = 0;
						TypeNum* vn = v->get_row() + i * v->get_count_prev();
						TypeNum Max = vn[r]; 
						for(size_t i = 1, mc = v->get_count_prev(); i < mc; i++)
						{
							if(vn[i] > Max)
								Max = vn[r = i];
						}
						return r;
					}
				} IndexMaxWeigth; //Get index with maximum weigth

				class{
					struct {NEURAL_LAYER* v; size_t i;};
				public: 
					inline operator long double() const 
					{ 
						long double r = 0.0;
						for(const TypeNum* c = v->get_row() + i * v->get_count_prev(), const *m = c + v->get_count_prev(); c < m; c++)
							r += *c;
						return r;
					} 
				} SumWeigths;
			};

			inline void SetWeights(TypeNum SetVal)
			{
				for(TypeNum* v = Count.v->get_row() + Count.i * Count.v->get_count_prev(), *m = v + Count.v->get_count_prev(); v < m; v++)
					*v = SetVal;
			}

			/*Get or set sinaps weight*/
			inline TypeNum& operator[] (size_t Index) { return Count.v->at(Count.i, Index); }
		};

		friend __NEURON;

		inline TypeNum& at(size_t Neuron, size_t Sinaps) { return Count.v[Neuron * Count.pn + Sinaps]; }

		inline TypeNum* get_row() { return Count.v; }

		inline size_t get_count_prev() const { return Count.pn; }

		bool set_count_prev(size_t NewCount)
		{
			if(Count.pn == NewCount)
				return true;
			const size_t l = NewCount * Count.n * sizeof(TypeNum);
			auto r = (TypeNum*)realloc(Count.v, l);
			if(r == nullptr)
				return false;
			Count.v = r;
			Count.pn = NewCount;
			return true;
		}

#define  __LAYER_FIELDS struct {\
			size_t n; \
			size_t pn; \
			TypeNum* v; \
			TACTIVATE_FUNC ActivateFunc; \
			TDER_ACTIVATE_FUNC DActivateFunc; \
			TREVERSE_ACTIVATE_FUNC ReversActivateFunc;\
			TypeNum* TmpDer; TypeNum* TmpDelta; TypeNum* TmpOut;\
		};
	public:

		NEURAL_LAYER(size_t nCountNeuronInPrevLayer, size_t nCountNeuron)
		{
			const size_t l = nCountNeuronInPrevLayer * nCountNeuron * sizeof(TypeNum);
			Count.v = (TypeNum*)malloc(l);
			Count.pn = nCountNeuronInPrevLayer;
			if(Count.v == nullptr)
			{
				Count.n = 0;
				return;
			}
			Count.n = nCountNeuron;
			Count.TmpDer = nullptr;
			Count.TmpDelta = nullptr;
			Count.TmpOut = nullptr;
			ReverseActivateFunc = ActivateFunction = [](TypeNum v) -> TypeNum { return v; };
			DerActivateFunction = [](TypeNum in) -> TypeNum { return 1; };
			SetWeights(TypeNum(0));
		}
		~NEURAL_LAYER()
		{
			if(Count.v != nullptr)
				free(Count.v);
		}

		inline void SetWeights(TypeNum SetVal)
		{
			for(TypeNum* v = Count.v, *m = v + CountSinaps; v < m; v++)
				*v = SetVal;
		}

		union
		{
			class {
				friend NEURAL_LAYER; friend NEURALNET;
				__LAYER_FIELDS;
			public:
				inline operator size_t() const { return n;}
			} Count; //Get count neuron in layer

			class { 
				__LAYER_FIELDS;
			public: 
				inline operator size_t() const { return n * pn;} 
			} CountSinaps;

			class{
				__LAYER_FIELDS;
			public: 
				inline operator long double() const 
				{ 
					long double r = 0.0;
					for(const TypeNum* m = v + n * pn, const *c = v;c < m;c++)
						r += *c;
					return r;
				} 
			} SumWeigths;
			
			class { 
				__LAYER_FIELDS;
			public: 
				inline operator TACTIVATE_FUNC() const { return ActivateFunc;} 
				inline TACTIVATE_FUNC operator =(TACTIVATE_FUNC New) { return ActivateFunc = New;} 
			} ActivateFunction;
									
			class { 
				__LAYER_FIELDS;
			public: 
				inline operator TDER_ACTIVATE_FUNC() const { return DActivateFunc;} 
				inline TDER_ACTIVATE_FUNC operator =(TDER_ACTIVATE_FUNC New) { return DActivateFunc = New;} 
			} DerActivateFunction;

			class { 
				__LAYER_FIELDS;
			public: 
				inline operator TREVERSE_ACTIVATE_FUNC() const { return ReversActivateFunc;} 
				inline TREVERSE_ACTIVATE_FUNC operator =(TREVERSE_ACTIVATE_FUNC New) { return ReversActivateFunc = New;} 
			} ReverseActivateFunc;
		};

		//Get neuron
		inline __NEURON operator[](size_t Index) { return __NEURON(this, Index); }
	};

	static void sync_mcn(size_t Countlayers, NEURAL_LAYER** Layers, size_t *Mcn)
	{
		size_t maxn = 0;
		if(Countlayers > 0)
			maxn = Layers[0]->get_count_prev();
		for(size_t i = 0; i < Countlayers;i++)
		{
		   size_t c = Layers[i]->Count;
		   if(c > maxn)
			   maxn = c;
		}
		*Mcn = maxn;
	}

	template<typename T1, typename T2>
	static void copy_arr_cast(T1* Dest, T2* Source, size_t Count)
	{
	    if(std::is_equal<typename std::remove_const<T2>::type, T1>::value)
		{
			memcpy(Dest, Source, Count * sizeof(T1));
		}else
		{
			auto d = Dest, md = d + Count;
			for(auto s = Source; d < md; d++, s++)
				*d = *s;
		}
	}

	NEURAL_LAYER& LayerByIndex(size_t Index) { return CountLayers.nl[Index][0]; }

	bool InitLayerOuts()
	{
		for(size_t l = 0; l < CountLayers; l++)
		{
			LayerByIndex(l).Count.TmpDer = (TypeNum*)malloc(LayerByIndex(l).Count * sizeof(TypeNum));
			if(LayerByIndex(l).Count.TmpDer == nullptr)
				goto lblErrOut;
			LayerByIndex(l).Count.TmpDelta = (TypeNum*)malloc(LayerByIndex(l).Count * sizeof(TypeNum));
			if(LayerByIndex(l).Count.TmpDelta == nullptr)
				goto lblErrOut;
			LayerByIndex(l).Count.TmpOut = (TypeNum*)malloc(LayerByIndex(l).Count * sizeof(TypeNum));
			if(LayerByIndex(l).Count.TmpOut == nullptr)
				goto lblErrOut;
		}
		return true;
lblErrOut:
		UninitLayerOuts();
		return false;
	}

	void UninitLayerOuts()
	{
		for(size_t l = 0; l < CountLayers; l++)
		{
			if(LayerByIndex(l).Count.TmpDer != nullptr){
				free(LayerByIndex(l).Count.TmpDer);
				LayerByIndex(l).Count.TmpDer = nullptr;
			}
			if(LayerByIndex(l).Count.TmpDer != nullptr){
				free(LayerByIndex(l).Count.TmpDelta);
				LayerByIndex(l).Count.TmpDelta = nullptr;
			}
			if(LayerByIndex(l).Count.TmpDer != nullptr){
				free(LayerByIndex(l).Count.TmpOut);
				LayerByIndex(l).Count.TmpOut = nullptr;
			}
		}
	}

public:
	
#define __NEURALNET_FIELDS struct{size_t cl; NEURAL_LAYER** nl; size_t mcn; size_t ic;};

	union
	{
		class{
			friend NEURALNET;
			__NEURALNET_FIELDS;
		public:
			inline operator size_t() const { return cl; }
		} CountLayers;

		class{
			__NEURALNET_FIELDS;
		public:
			inline operator size_t()  const { return (cl == 0)? 0: nl[0]->get_count_prev(); }
			size_t operator =(size_t NewInCount)
			{	
				if(cl == 0)
					return ic = NewInCount;
				if(nl[0]->set_count_prev(NewInCount))
				{
					if(NewInCount > mcn)
						mcn = NewInCount;
					ic = NewInCount;
				}
				sync_mcn(cl, nl, &mcn);
				return NewInCount;
			}
		} InputCount;
				
		class{
			__NEURALNET_FIELDS;
		public:
			inline operator size_t() const { return (cl == 0)? 0: size_t(nl[cl - 1]->Count); }
		} OutputCount;

		class{
			__NEURALNET_FIELDS;
		public:
			inline operator size_t() const
			{
				size_t res = 0;
				for(size_t i = 0;i < cl;i++)
					res += nl[i]->Count;
				return res; 
			}
		} CountNeuron;

		class{
			__NEURALNET_FIELDS;
		public:
			inline operator size_t() const
			{
				if(cl == 0)
					return 0;
				size_t res = 0;
				for(size_t i = 0;i < cl;i++)
				{
					size_t c = nl[i]->Count;
					res += c * c;
				}
				return res; 
			}
		} CountSinaps;

		class{
			__NEURALNET_FIELDS;
		public:
			inline operator size_t() const { return mcn; }
		} MaxCountNeuronInLayer;
	};

	NEURALNET()
	{
		CountLayers.mcn = CountLayers.cl = 0;
		CountLayers.nl = nullptr;
		CountLayers.ic = 1;
	}

	~NEURALNET()
	{
		for(size_t i = 0; i < CountLayers.cl;i++)
		   delete CountLayers.nl[i];

		if(CountLayers.nl != nullptr)
			free(CountLayers.nl);
	}

	template<typename InputVectorType, typename OutputVectorType>
	typename std::enable_if<
		std::is_convertible<InputVectorType, TypeNum>::value && 
		std::is_convertible<TypeNum, OutputVectorType>::value ,
		double
	>::type
    CheckError(const InputVectorType* InputVector, const OutputVectorType* ExpectedVector) const
	{
		auto OutputVector = (TypeNum*)malloc(OutputCount * sizeof(TypeNum));
		if(OutputVector == nullptr)
			return -1.0;
		if(!Recognize(InputVector, OutputVector))
			goto lblOutErr;
		double RetError = 0.0;
		for(size_t i = 0, mi = OutputCount;i < mi;i++)
		{
			TypeNum Err = TypeNum(ExpectedVector[i]) - OutputVector[i];
			RetError += Err * Err;
		}
		free(OutputVector);
		return RetError / 2.0;

lblOutErr:
		free(OutputVector);
		return -1.0;
	}

	void SetWeights(TypeNum SetVal)
	{
		for(size_t i = 0; i < CountLayers.cl; i++)
			CountLayers.nl[i]->SetWeights(SetVal);
	}

	void RandomizeWeights()
	{
		srand(time(nullptr));
		auto UnifiedRand = [](){return TypeNum(2) * ((TypeNum)rand() / ((TypeNum) RAND_MAX)) - 1;};
		for(size_t l = 0, ml = CountLayers;l < ml;l++) 
		{ 
			TypeNum* w = LayerByIndex(l).get_row();
			TypeNum CountInputNeuron =  sqrt(TypeNum(LayerByIndex(l).get_count_prev()));

			for(size_t q = 0, mq = LayerByIndex(l).CountSinaps; q < mq; q++) 
			{ 
				w[q] = UnifiedRand() / CountInputNeuron;
			} 
		} 
	}

	void SetActivateFuncInAllLayers(TACTIVATE_FUNC NewActivateFunc)
	{
		for(size_t l = 0; l < CountLayers; l++)
			LayerByIndex(l).ActivateFunction = NewActivateFunc;
	}

	void SetDerActivateFuncInAllLayers(TDER_ACTIVATE_FUNC NewDerActivateFunc)
	{
		for(size_t l = 0; l < CountLayers; l++)
			LayerByIndex(l).DerActivateFunction = NewDerActivateFunc;
	}

	void SetReverseActivateFuncInAllLayers(TACTIVATE_FUNC NewRevActivateFunc)
	{
		for(size_t l = 0; l < CountLayers; l++)
			LayerByIndex(l).ReverseActivateFunc = NewRevActivateFunc;
	}

	//Get layer
	inline NEURAL_LAYER& operator[](size_t IndexLayer) { return CountLayers.nl[IndexLayer][0]; }

	bool NewLayer(size_t CountNeuron, size_t Index = 0xffffffff)
	{
		if(Index > CountLayers) 
			Index = CountLayers;
		auto nl = (NEURAL_LAYER**)realloc(CountLayers.nl, (CountLayers + 1) * sizeof(NEURAL_LAYER*));
		if(nl == nullptr)
			return false;
		if(Index != CountLayers)
			memmove(nl + (Index + 1), nl + Index,  (CountLayers - Index) * sizeof(NEURAL_LAYER*));

		if(Index == CountLayers) //If last layer
		{
			nl[Index] = new NEURAL_LAYER((Index != 0) ? nl[Index - 1]->Count: CountLayers.ic, CountNeuron);	
		}else
		{
			nl[Index] = new NEURAL_LAYER(nl[Index + 1]->get_count_prev(), CountNeuron);	
			nl[Index + 1]->set_count_prev(CountNeuron);
		}
		CountLayers.nl = nl;
		CountLayers.cl++;
		sync_mcn(CountLayers.cl, CountLayers.nl, &CountLayers.mcn);
		return true;
	}

	bool RemoveLayer(size_t Index)
	{
		if(Index >= CountLayers) 
			return false;
		auto nl = CountLayers.nl;
		delete nl[Index];
		if(Index != (CountLayers - 1)) //If not last layer
		{
			memmove(nl + Index, nl + (Index + 1),  (CountLayers - (Index + 1)) * sizeof(NEURAL_LAYER*));
			nl[Index]->set_count_prev((Index == 0)? CountLayers.ic: nl[Index - 1]->Count);
		}
		nl = (NEURAL_LAYER**)realloc(nl, (CountLayers.cl - 1) * sizeof(NEURAL_LAYER*));	
		CountLayers.cl--;
		if(nl != nullptr)
			CountLayers.nl = nl;
		sync_mcn(CountLayers.cl, CountLayers.nl, &CountLayers.mcn);
		return true;
	}
	
	template<typename InputVectorType, typename OutputVectorType>
	typename std::enable_if<
		std::is_convertible<InputVectorType, TypeNum>::value && 
		std::is_convertible<TypeNum, OutputVectorType>::value ,
		bool
	>::type
	Recognize(const InputVectorType* In, OutputVectorType* Out) const
	{			
		if(CountLayers == 0)
			return false;
		const size_t cn = MaxCountNeuronInLayer;
		TypeNum* gl = (TypeNum*)malloc(cn * sizeof(TypeNum) * 2), *sl = gl + cn, *og = gl;
		if(gl == nullptr)
			return false;
		copy_arr_cast(gl, In, InputCount);
		for(NEURAL_LAYER *const*nl = CountLayers.nl, *const*mnl = nl + size_t(CountLayers); nl < mnl; nl++)
		{
			const TypeNum* s = (*nl)->get_row(), *mgl2 = gl + (*nl)->get_count_prev(), *mdst = sl + size_t((*nl)->Count);		//Get array sinaps
			TACTIVATE_FUNC ActivateFunc = (*nl)->ActivateFunction;
			for(TypeNum* dst = sl; dst < mdst; dst++)
			{
				TypeNum SolveNeu = TypeNum(0);
				for(const TypeNum* gl2 = gl; gl2 < mgl2; gl2++, s++)
					SolveNeu += *gl2 * *s;
				*dst = ActivateFunc(SolveNeu);
			}
			/*in sl placed */
			std::swap(gl, sl);
		}
		copy_arr_cast(Out, gl, OutputCount);
		free(og);
		return true;
	}

	template<typename InputVectorType, typename OutputVectorType>
	typename std::enable_if<
		std::is_convertible<InputVectorType, TypeNum>::value && 
		std::is_convertible<TypeNum, OutputVectorType>::value ,
		bool
	>::type
	RecognizeParallel(const InputVectorType* In, OutputVectorType* Out) const
	{			
		if(CountLayers == 0)
			return false;
		const size_t cn = MaxCountNeuronInLayer;
		TypeNum* gl = (TypeNum*)malloc(cn * sizeof(TypeNum) * 2), *sl = gl + cn, *og = gl;
		if(gl == nullptr)
			return false;
		copy_arr_cast(gl, In, InputCount);
		for(NEURAL_LAYER *const*nl = CountLayers.nl, *const*mnl = nl + size_t(CountLayers); nl < mnl; nl++)
		{
			//Go next layer
			int cn = (*nl)->Count, cp = (*nl)->get_count_prev();
			const TypeNum* s = (*nl)->get_row(), const *mj = gl + cp;		//Get array sinaps
			TACTIVATE_FUNC ActivateFunc = (*nl)->ActivateFunction;
#pragma omp parallel
			{
#pragma omp for private(cn)
				for(int i = 0; i < cn; i++)
				{
					TypeNum SolveNeu = TypeNum(0);
					for(const TypeNum *gl2 = gl, const *s2 = s + i * cp; gl2 < mj; gl2++, s2++)
						SolveNeu += *gl2 * *s2;
					sl[i] = ActivateFunc(SolveNeu);
				}
			}
			/*in sl placed */
			std::swap(gl, sl);
		}
		copy_arr_cast(Out, gl, OutputCount);
		free(og);
		return true;
	}

	template<typename InputVectorType, typename OutputVectorType>
	typename std::enable_if<
		std::is_convertible<InputVectorType, TypeNum>::value && 
		std::is_convertible<TypeNum, OutputVectorType>::value ,
		bool
	>::type
	GetReverse(const InputVectorType* In, OutputVectorType* Out) const
	{

		size_t cn, InCount;
		if(CountLayers == 0)
			return false;
		cn = MaxCountNeuronInLayer;
		TypeNum* gl = (TypeNum*)malloc(cn * sizeof(TypeNum) * 2), *sl = gl + cn, *og = gl;
		if(gl == nullptr)
			return false;
		copy_arr_cast(gl, In, InCount = OutputCount);
		
		for(NEURAL_LAYER *const*mnl = CountLayers.nl, *const*nl = mnl + (size_t(CountLayers) - 1); nl >= mnl; nl--)
		{
			size_t OutCount = (*nl)->get_count_prev();
			TypeNum* s = (*nl)->get_row(), *mglt = gl + InCount;		//Get array sinaps
			TREVERSE_ACTIVATE_FUNC RActivateFunc = (*nl)->ReverseActivateFunc;
			for(TypeNum* slt = sl, *m = slt + OutCount; slt < m; slt++)
				*slt = TypeNum(0);
			for(TypeNum* glt = gl; glt < mglt; glt++)
			{
				long double SumWeigths = 0.0;
				TypeNum* m = s + OutCount;
				for(const TypeNum* st = s; st < m; st++)
					SumWeigths += *st;
				if(SumWeigths == TypeNum(0))
				{
					s = m;
					continue;
				}
				long double tn = RActivateFunc(*glt) / SumWeigths;
				for(TypeNum* slt = sl; s < m; s++, slt++)
					*slt += TypeNum(tn * *s);
			}
			InCount = OutCount;
			std::swap(gl, sl);
		}
		copy_arr_cast(Out, gl, InputCount);
		free(og);
		return true;
	}

	template<typename InputVectorType, typename OutputVectorType>
	typename std::enable_if<
		std::is_convertible<InputVectorType, TypeNum>::value && 
		std::is_convertible<TypeNum, OutputVectorType>::value ,
		bool
	>::type
	GetReverseParallel(const InputVectorType* In, OutputVectorType* Out) const
	{
		size_t cn, InCount;
		if(CountLayers == 0)
			return false;
		cn = MaxCountNeuronInLayer;
		TypeNum* gl = (TypeNum*)malloc(cn * sizeof(TypeNum) * 2), *sl = gl + cn, *og = gl;
		if(gl == nullptr)
			return false;
		copy_arr_cast(gl, In, InCount = OutputCount);
		for(NEURAL_LAYER *const*mnl = CountLayers.nl, *const*nl = mnl + (size_t(CountLayers) - 1); nl >= mnl; nl--)
		{
			size_t OutCount = (*nl)->get_count_prev(), mglt = InCount;
			TypeNum* s = (*nl)->get_row();		//Get array sinaps
			TREVERSE_ACTIVATE_FUNC RActivateFunc = (*nl)->ReverseActivateFunc;
			for(TypeNum* slt = sl, *m = slt + OutCount; slt < m; slt++)
				*slt = TypeNum(0);
#pragma omp parallel
			{
#pragma omp for private(mglt)
				for(int i = 0; i < mglt; i++)
				{
					long double SumWeigths = 0.0;
					TypeNum* a = s + i * OutCount, *ms = a + OutCount;
					for(TypeNum* st = a; st < ms; st++)
						SumWeigths += *st;
					if(SumWeigths == TypeNum(0))
						continue;
					long double tn = RActivateFunc(gl[i]) / SumWeigths;
					for(TypeNum* slt = sl, const*st = a; st < ms; st++, slt++)
					{
						register TypeNum r = TypeNum(tn * *st);
#pragma omp atomic
						*slt += r;				//interlocked add
					}
				}
			}
			InCount = OutCount;
			std::swap(gl, sl);
		}
		copy_arr_cast(Out, gl, InputCount);
		free(og);
		return true;
	}
	
	double Learn(const TypeNum* In, const TypeNum* Result, double SpeedLern = 0.5, double ErrorMin = 0.2, unsigned CountLoop = 5000)
	{
		if(CountLayers == 0)
			return -1.0;

		if(!InitLayerOuts())
			return -1.0;

		double Error = 0.0;
		for(unsigned MainLoopCounter = 0; MainLoopCounter < CountLoop; MainLoopCounter++)
		{
			const TypeNum* gl = In;
			for(NEURAL_LAYER **nl = CountLayers.nl, **mnl = nl + size_t(CountLayers); nl < mnl; nl++)
			{
				TypeNum* sl = (*nl)->Count.TmpOut;
				TypeNum* s = (*nl)->get_row();
				const TypeNum* mgl2 = gl + (*nl)->get_count_prev(); 
				TypeNum* mdst = sl + size_t((*nl)->Count);
				TypeNum* Der = (*nl)->Count.TmpDer;
				TACTIVATE_FUNC ActivateFunc = (*nl)->ActivateFunction;
				TDER_ACTIVATE_FUNC DerActivateFunc = (*nl)->DerActivateFunction;
				for(TypeNum* dst = sl; dst < mdst; dst++, Der++)
				{ //For each neuron in current layer
					TypeNum SolveNeu = TypeNum(0);
					for(const TypeNum* gl2 = gl; gl2 < mgl2; gl2++, s++) //for each weigth
						SolveNeu += *gl2 * *s;
					*dst = ActivateFunc(SolveNeu);
					*Der = DerActivateFunc(SolveNeu);
				}
				gl = sl;
			}
			//output in gl vector
			Error = 0.0;

			//Правим веса последнего слоя
			{
				const TypeNum* Derivatives	= LayerByIndex(CountLayers - 1).Count.TmpDer;
				TypeNum* InputWeigths	= LayerByIndex(CountLayers - 1).get_row();
				TypeNum* Delta			= LayerByIndex(CountLayers - 1).Count.TmpDelta;
				const TypeNum* OutPrev	= (CountLayers > 1)? (LayerByIndex(CountLayers - 2).Count.TmpOut): In;
				TypeNum* CurOut			= LayerByIndex(CountLayers - 1).Count.TmpOut;
				size_t CountPrev		= LayerByIndex(CountLayers - 1).get_count_prev();
				for(size_t q = 0, mq = OutputCount; q < mq; q++)
				{
					TypeNum CurError = Result[q] - CurOut[q]; 
					Error += CurError * CurError; 					
					TypeNum CurDelta = SpeedLern * (Delta[q] = Derivatives[q] * CurError);			//Delta[q] = F`(OUT[q]) * (T[q] - OUT[q])
					for(size_t p = 0; p < CountPrev; p++)
						InputWeigths[p] += CurDelta * OutPrev[p];					//w[p,q](i+1) = q[p,q](i) + n * Delta[q] * OUT[p]
					InputWeigths += CountPrev;
				}
			}
			Error /= 2.0;
			if(Error < ErrorMin)
				break;

			//Правим веса внутренних слоёв
			for(int l = CountLayers - 2; l >= 0; l--)
			{				
				const TypeNum* Derivatives	= LayerByIndex(l).Count.TmpDer;
				TypeNum* InputWeigths	= LayerByIndex(l).get_row();
				const TypeNum* OutPrev	= (l < 1)? In: LayerByIndex(l - 1).Count.TmpOut;
				TypeNum* Delta			= LayerByIndex(l).Count.TmpDelta;
				const TypeNum* DeltaNextLayer = LayerByIndex(l + 1).Count.TmpDelta;
				TypeNum* WeigthToNextLayer = LayerByIndex(l).get_row();
				size_t CountPrev		= LayerByIndex(l).get_count_prev();
				size_t NeuronCount		= LayerByIndex(l).Count;
				size_t NeuronCountInNextLayer	= LayerByIndex(l + 1).Count;
				for(size_t q = 0; q < NeuronCount; q++)
				{
					TypeNum CurDelta = TypeNum(0);
					for(size_t k = 0;k < NeuronCountInNextLayer; k++)
						CurDelta += DeltaNextLayer[k] * WeigthToNextLayer[k];
					WeigthToNextLayer += NeuronCountInNextLayer;
					Delta[q] = (CurDelta *= Derivatives[q]);
					CurDelta *= SpeedLern;
					for(size_t p = 0; p < CountPrev; p++)
						InputWeigths[p] += CurDelta * OutPrev[p];
					InputWeigths += CountPrev;
				}
			}
		
		}

		UninitLayerOuts();
		return Error;
	}

	double LearnParallel(const TypeNum* In, const TypeNum* Result, double SpeedLern = 0.5, double ErrorMin = 0.2, unsigned CountLoop = 5000)
	{
		if(CountLayers == 0)
			return -1.0;

		if(!InitLayerOuts())
			return -1.0;

		double Error = 0.0;
		for(unsigned MainLoopCounter = 0; MainLoopCounter < CountLoop; MainLoopCounter++)
		{
			const TypeNum* gl = In;
			for(NEURAL_LAYER **nl = CountLayers.nl, **mnl = nl + size_t(CountLayers); nl < mnl; nl++)
			{
				TypeNum* sl			= (*nl)->Count.TmpOut;
				TypeNum* s			= (*nl)->get_row();
				size_t CountPrev	= (*nl)->get_count_prev();
				const TypeNum* mj	= gl + CountPrev; 
				int CountNeuron		= size_t((*nl)->Count);
				TypeNum* Der		= (*nl)->Count.TmpDer;
				TACTIVATE_FUNC ActivateFunc = (*nl)->ActivateFunction;
				TDER_ACTIVATE_FUNC DerActivateFunc = (*nl)->DerActivateFunction;
#pragma omp parallel
				{
#pragma omp for private(CountNeuron)
					for(int i = 0; i < CountNeuron; i++)
					{ //For each neuron in current layer
						TypeNum SolveNeu = TypeNum(0);
						for(const TypeNum *gl2 = gl, const *s2 = s + i * CountPrev; gl2 < mj; gl2++, s2++)
							SolveNeu += *gl2 * *s2;
						sl[i] = ActivateFunc(SolveNeu);
						Der[i] = DerActivateFunc(SolveNeu);
					}
				}
				gl = sl;
			}
			//output in gl vector
			Error = 0.0;

			//Правим веса последнего слоя
			{
				const TypeNum* Derivatives	= LayerByIndex(CountLayers - 1).Count.TmpDer;
				TypeNum* InputWeigths		= LayerByIndex(CountLayers - 1).get_row();
				TypeNum* Delta				= LayerByIndex(CountLayers - 1).Count.TmpDelta;
				const TypeNum* OutPrev		= (CountLayers > 1)? (LayerByIndex(CountLayers - 2).Count.TmpOut): In;
				TypeNum* CurOut				= LayerByIndex(CountLayers - 1).Count.TmpOut;
				size_t CountPrev			= LayerByIndex(CountLayers - 1).get_count_prev();
				int mq						= OutputCount;
#pragma omp parallel
				{
#pragma omp for private(mq)
					for(int q = 0; q < mq; q++)
					{
						TypeNum CurError = Result[q] - CurOut[q]; 
						Error += CurError * CurError; 					
						TypeNum CurDelta = SpeedLern * (Delta[q] = Derivatives[q] * CurError);	//Delta[q] = F`(OUT[q]) * (T[q] - OUT[q])
						TypeNum* w = InputWeigths + q * CountPrev;
						for(size_t p = 0; p < CountPrev; p++)
							w[p] += CurDelta * OutPrev[p];					//w[p,q](i+1) = q[p,q](i) + n * Delta[q] * OUT[p]
					}
				}
			}

			Error /= 2.0;
			if(Error < ErrorMin)
				break;

			//Правим веса внутренних слоёв
			for(int l = CountLayers - 2; l >= 0; l--)
			{				
				const TypeNum* Derivatives	= LayerByIndex(l).Count.TmpDer;
				TypeNum* InputWeigths		= LayerByIndex(l).get_row();
				const TypeNum* OutPrev		= (l < 1)? In: LayerByIndex(l - 1).Count.TmpOut;
				TypeNum* Delta				= LayerByIndex(l).Count.TmpDelta;
				const TypeNum* DeltaNextLayer = LayerByIndex(l + 1).Count.TmpDelta;
				TypeNum* WeigthToNextLayer	= LayerByIndex(l).get_row();
				size_t CountPrev			= LayerByIndex(l).get_count_prev();
				int NeuronCount				= LayerByIndex(l).Count;
				size_t NeuronCountInNextLayer	= LayerByIndex(l + 1).Count;
#pragma omp parallel
				{
#pragma omp for private(NeuronCount)
					for(int q = 0; q < NeuronCount; q++)
					{
						TypeNum CurDelta = TypeNum(0);
						TypeNum* w = WeigthToNextLayer + q * NeuronCountInNextLayer;
						for(size_t k = 0;k < NeuronCountInNextLayer; k++)
							CurDelta += DeltaNextLayer[k] * w[k];
						Delta[q] = (CurDelta *= Derivatives[q]);
						CurDelta *= SpeedLern;
						w = InputWeigths + q * CountPrev;
						for(size_t p = 0; p < CountPrev; p++)
							w[p] += CurDelta * OutPrev[p];
					}
				}
			}
		}
		UninitLayerOuts();
		return Error;
	}
};

#endif
