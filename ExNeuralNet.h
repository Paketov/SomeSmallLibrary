#ifndef __NEURALNET_H_HAS_INCLUDED__
#define __NEURALNET_H_HAS_INCLUDED__

#include <malloc.h>
#include <omp.h>

#include "ExTypeTraits.h"


template<typename TypeNum = int>
class NEURALNET
{

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
				} Count; //Get the number of input synapses weight

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
				} IndexMaxWeigth;

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

#define  __LAYER_FIELDS struct {size_t n; size_t pn; TypeNum* v; \
			TypeNum (*ActivateFunc)(TypeNum); TypeNum (*DActivateFunc)(TypeNum); \
			TypeNum (*ReversActivateFunc)(TypeNum);};
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
			ReverseActivateFunc = ActivateFunction = [](TypeNum v) -> TypeNum { return v; };
			DerActivateFunction = [](TypeNum) -> TypeNum { return 1; };
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
				friend NEURAL_LAYER;
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
				typedef TypeNum (*TACTIV_FUNC)(TypeNum);
			public: 
				inline operator TACTIV_FUNC() const { return ActivateFunc;} 
				inline TACTIV_FUNC operator =(TACTIV_FUNC New) { return ActivateFunc = New;} 
			} ActivateFunction;
									
			class { 
				__LAYER_FIELDS;
				typedef TypeNum (*TDACTIV_FUNC)(TypeNum);
			public: 
				inline operator TDACTIV_FUNC() const { return DActivateFunc;} 
				inline TDACTIV_FUNC operator =(TDACTIV_FUNC New) { return DActivateFunc = New;} 
			} DerActivateFunction;

			class { 
				__LAYER_FIELDS;
				typedef TypeNum (*TRACTIV_FUNC)(TypeNum);
			public: 
				inline operator TRACTIV_FUNC() const { return ReversActivateFunc;} 
				inline TRACTIV_FUNC operator =(TRACTIV_FUNC New) { return ReversActivateFunc = New;} 
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

public:
	typedef TypeNum TWEIGTHS;

#define __NEURALNET_FIELDS struct{size_t cl; NEURAL_LAYER** nl; size_t mcn; size_t ic;};
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

	union
	{
		class
		{
			friend NEURALNET;
			__NEURALNET_FIELDS;
		public:
			inline operator size_t() const { return cl; }
		} CountLayers;

		class
		{
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
				
		class
		{
			__NEURALNET_FIELDS;
		public:
			inline operator size_t() const { return (cl == 0)? 0: size_t(nl[cl - 1]->Count); }
		} OutputCount;

		class
		{
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

		class
		{
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

		class
		{
			__NEURALNET_FIELDS;
		public:
			inline operator size_t() const { return mcn; }
		} MaxCountNeuronInLayer;

	};


	void SetWeights(TypeNum SetVal)
	{
		for(size_t i = 0; i < CountLayers.cl; i++)
			CountLayers.nl[i]->SetWeights(SetVal);
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

	bool Recognize(const TypeNum* In, TypeNum* Out) const
	{			
		if(CountLayers == 0)
			return false;
		const size_t cn = MaxCountNeuronInLayer;
		TypeNum* gl = (TypeNum*)malloc(cn * sizeof(TypeNum) * 2), *sl = gl + cn, *og = gl;
		if(gl == nullptr)
			return false;
		NEURAL_LAYER *const*nl = CountLayers.nl, *const*mnl = nl + size_t(CountLayers);
		memcpy(gl, In, InputCount * sizeof(TypeNum));
		for(; nl < mnl; nl++)
		{
			const TypeNum* s = (*nl)->get_row(), *mgl2 = gl + (*nl)->get_count_prev(), *mdst = sl + size_t((*nl)->Count);		//Get array sinaps
			TypeNum (*ActivateFunc)(TypeNum) = (*nl)->ActivateFunction;
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
		memcpy(Out, gl, OutputCount * sizeof(TypeNum));
		free(og);
		return true;
	}

	bool RecognizeParallel(const TypeNum* In, TypeNum* Out) const
	{			
		if(CountLayers == 0)
			return false;
		const size_t cn = MaxCountNeuronInLayer;
		TypeNum* gl = (TypeNum*)malloc(cn * sizeof(TypeNum) * 2), *sl = gl + cn, *og = gl;
		if(gl == nullptr)
			return false;
		NEURAL_LAYER *const*nl = CountLayers.nl, *const*mnl = nl + size_t(CountLayers);
		memcpy(gl, In, InputCount * sizeof(TypeNum));
		for(; nl < mnl; nl++)
		{
			//Go next layer
			int cn = (*nl)->Count, cp = (*nl)->get_count_prev();
			const TypeNum* s = (*nl)->get_row(), const *mj = gl + cp;		//Get array sinaps
			TypeNum (*ActivateFunc)(TypeNum) = (*nl)->ActivateFunction;
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
		memcpy(Out, gl, OutputCount * sizeof(TypeNum));
		free(og);
		return true;
	}


	bool Recognize(const TypeNum* In, TypeNum* Out, TypeNum (*ActivateFunc)(TypeNum InputSumm)) const
	{			
		if(CountLayers == 0)
			return false;
		const size_t cn = MaxCountNeuronInLayer;
		TypeNum* gl = (TypeNum*)malloc(cn * sizeof(TypeNum) * 2), *sl = gl + cn, *og = gl;
		if(gl == nullptr)
			return false;
		NEURAL_LAYER *const*nl = CountLayers.nl, *const*mnl = nl + size_t(CountLayers);
		memcpy(gl, In, InputCount * sizeof(TypeNum));
		for(; nl < mnl; nl++)
		{
			const TypeNum* s = (*nl)->get_row(), *mgl2 = gl + (*nl)->get_count_prev(), *mdst = sl + size_t((*nl)->Count);		//Get array sinaps
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
		memcpy(Out, gl, OutputCount * sizeof(TypeNum));
		free(og);
		return true;
	}

	bool RecognizeParallel(const TypeNum* In, TypeNum* Out, TypeNum (*ActivateFunc)(TypeNum InputSumm)) const
	{			
		if(CountLayers == 0)
			return false;
		const size_t cn = MaxCountNeuronInLayer;
		TypeNum* gl = (TypeNum*)malloc(cn * sizeof(TypeNum) * 2), *sl = gl + cn, *og = gl;
		if(gl == nullptr)
			return false;
		NEURAL_LAYER *const*nl = CountLayers.nl, *const*mnl = nl + size_t(CountLayers);
		memcpy(gl, In, InputCount * sizeof(TypeNum));
		for(; nl < mnl; nl++)
		{
			//Go next layer
			int cn = (*nl)->Count, cp = (*nl)->get_count_prev();
			const TypeNum* s = (*nl)->get_row(), const *mj = gl + cp;		//Get array sinaps
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
		memcpy(Out, gl, OutputCount * sizeof(TypeNum));
		free(og);
		return true;
	}

	bool GetReverse(const TypeNum* In, TypeNum* Out) const
	{
		size_t cn, InCount;
		if(CountLayers == 0)
			return false;
		cn = MaxCountNeuronInLayer;
		TypeNum* gl = (TypeNum*)malloc(cn * sizeof(TypeNum) * 2), *sl = gl + cn, *og = gl;
		if(gl == nullptr)
			return false;
		NEURAL_LAYER *const*mnl = CountLayers.nl, *const*nl = mnl + (size_t(CountLayers) - 1);
		memcpy(gl, In, (InCount = OutputCount) * sizeof(TypeNum));
		
		for(; nl >= mnl; nl--)
		{
			size_t OutCount = (*nl)->get_count_prev();
			TypeNum* s = (*nl)->get_row(), *mglt = gl + InCount;		//Get array sinaps
			TypeNum (*RActivateFunc)(TypeNum) = (*nl)->ReverseActivateFunc;
			for(TypeNum* slt = sl, *m = slt + OutCount; slt < m; slt++)
				*slt = TypeNum(0);
			for(TypeNum* glt = gl; glt < mglt; glt++)
			{
				long double SumWeigths = 0.0;
				const TypeNum* m = s + OutCount;
				for(const TypeNum* st = s; st < m; st++)
					SumWeigths += *st;
				long double tn = RActivateFunc(*glt) / SumWeigths;
				for(TypeNum* slt = sl; s < m; s++, slt++)
					*slt += TypeNum(tn * *s);
			}
			InCount = OutCount;
			std::swap(gl, sl);
		}
		memcpy(Out, gl, InputCount * sizeof(TypeNum));
		free(og);
		return true;
	}

	bool GetReverseParallel(const TypeNum* In, TypeNum* Out) const
	{
		size_t cn, InCount;
		if(CountLayers == 0)
			return false;
		cn = MaxCountNeuronInLayer;
		TypeNum* gl = (TypeNum*)malloc(cn * sizeof(TypeNum) * 2), *sl = gl + cn, *og = gl;
		if(gl == nullptr)
			return false;
		NEURAL_LAYER *const*mnl = CountLayers.nl, *const*nl = mnl + (size_t(CountLayers) - 1);
		memcpy(gl, In, (InCount = OutputCount) * sizeof(TypeNum));
		for(; nl >= mnl; nl--)
		{
			size_t OutCount = (*nl)->get_count_prev(), mglt = InCount;
			TypeNum* s = (*nl)->get_row();		//Get array sinaps
			TypeNum (*RActivateFunc)(TypeNum) = (*nl)->ReverseActivateFunc;
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
		memcpy(Out, gl, InputCount * sizeof(TypeNum));
		free(og);
		return true;
	}
	

};

#endif
