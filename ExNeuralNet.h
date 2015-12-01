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
					inline operator size_t() const { return v->count_in_prev(); }
				} Count; //Get the number of input synapses weight

				class{
					struct {NEURAL_LAYER* v; size_t i;};
				public:
					operator size_t ()
					{
					    size_t r = 0;
						TypeNum* vn = v->get_row() + i * v->count_in_prev();
						TypeNum Min = vnv[r]; 
						for(size_t i = 1, mc = v->count_in_prev();i < mc;i++)
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
						TypeNum* vn = v->get_row() + i * v->count_in_prev();
						TypeNum Max = vn[r]; 
						for(size_t i = 1, mc = v->count_in_prev(); i < mc; i++)
						{
							if(vn[i] > Max)
								Max = vn[r = i];
						}
						return r;
					}
				} IndexMaxWeigth;
			};

			inline void ClearWeights(TypeNum SetVal)
			{
				TypeNum* v = Count.v->get_row() + Count.i * Count.v->count_in_prev();
				for(size_t i = 0, mc = Count;i < mc;i++)
					v[i] = SetVal;
			}

			/*Get or set sinaps weight*/
			inline TypeNum& operator[] (size_t Index) { return Count.v->at(Count.i, Index); }
		};

		friend __NEURON;

		inline TypeNum& at(size_t Neuron, size_t Sinaps) { return Count.v[Neuron * Count.pn + Sinaps]; }

		inline TypeNum* get_row() { return Count.v; }

		inline size_t count_in_prev() const { return Count.pn; }

		inline bool set_count_prev(size_t NewCount)
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
			memset(Count.v, 0, l);
			Count.n = nCountNeuron;
		}
		~NEURAL_LAYER()
		{
			if(Count.v != nullptr)
				free(Count.v);
		}

		inline void ClearWeights(TypeNum SetVal)
		{
			TypeNum* v = Count.v;
			for(size_t i = 0, mc = CountSinaps; i < mc;i++)
				v[i] = SetVal;
		}

		union
		{
			class {
				friend NEURAL_LAYER;
				struct {size_t n; size_t pn; TypeNum* v;};
			public:
				inline operator size_t() const { return n;}
			} Count; //Get count neuron in layer

			class { 
				struct {size_t n; size_t pn; TypeNum* v;};
			public: 
				inline operator size_t() const { return n * pn;} 
			} CountSinaps;
		};

		inline __NEURON operator[](size_t Index) { return __NEURON(this, Index); }
	};

public:


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
			inline operator size_t()  const { return (cl == 0)? 0: nl[0]->count_in_prev(); }

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
				return NewInCount;
			}
		} InCount;
				
		class
		{
			__NEURALNET_FIELDS;
		public:
			inline operator size_t() const { return (cl == 0)? 0: size_t(nl[cl - 1]->Count); }
		} OutCount;

		class
		{
			__NEURALNET_FIELDS;
		public:
			inline operator size_t() const
			{
				size_t res = 0;
				for(size_t i = 0;i < CountLayers;i++)
					res += CountLayers.nl[i]->Count;
				return res; 
			}
		} CountAllNeuron;

		class
		{
			__NEURALNET_FIELDS;
		public:
			inline operator size_t() const { return mcn; }
		} MaxCountNeuronInLayer;

	};


	void ClearWeights(TypeNum SetVal)
	{
		for(size_t i = 0;i < CountLayers.cl;i++)
			CountLayers.nl[i]->ClearWeights(SetVal);
	}

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
			nl[Index] = new NEURAL_LAYER(nl[Index + 1]->count_in_prev(), CountNeuron);	
			nl[Index + 1]->set_count_prev(CountNeuron);
		}
		if(CountNeuron > CountLayers.mcn)		  
			CountLayers.mcn = CountNeuron;
		CountLayers.nl = nl;
		CountLayers.cl++;
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
		return true;
	}

	bool Recognize(const TypeNum* In, TypeNum* Out) const
	{			
		if(MaxCountNeuronInLayer == 0)
			return false;
		const size_t cn = MaxCountNeuronInLayer;
		TypeNum* gl = (TypeNum*)malloc(cn * sizeof(TypeNum) * 2), *sl = gl + cn, *og = gl;
		if(gl == nullptr)
			return false;
		NEURAL_LAYER *const*nl = CountLayers.nl, *const*mnl = nl + size_t(CountLayers);
		memcpy(gl, In, InCount * sizeof(TypeNum));
		for(; nl < mnl; nl++)
		{
			const TypeNum* s = (*nl)->get_row(), *mgl2 = gl + (*nl)->count_in_prev(), *mdst = sl + size_t((*nl)->Count);		//Get array sinaps
			for(TypeNum* dst = sl; dst < mdst; dst++)
			{
				TypeNum SolveNeu = TypeNum(0);
				for(const TypeNum* gl2 = gl; gl2 < mgl2; gl2++, s++)
					SolveNeu += *gl2 * *s;
				*dst = SolveNeu;
			}
			/*in sl placed */
			std::swap(gl, sl);
		}
		memcpy(Out, gl, OutCount * sizeof(TypeNum));
		free(og);
		return true;
	}

	bool RecognizeParallel(const TypeNum* In, TypeNum* Out) const
	{			
		if(MaxCountNeuronInLayer == 0)
			return false;
		const size_t cn = MaxCountNeuronInLayer;
		TypeNum* gl = (TypeNum*)malloc(cn * sizeof(TypeNum) * 2), *sl = gl + cn, *og = gl;
		if(gl == nullptr)
			return false;
		NEURAL_LAYER *const*nl = CountLayers.nl, *const*mnl = nl + size_t(CountLayers);
		memcpy(gl, In, InCount * sizeof(TypeNum));
		for(; nl < mnl; nl++)
		{
			//Go next layer
			int cn = (*nl)->Count, cp = (*nl)->count_in_prev();
			const TypeNum* s = (*nl)->get_row(), const *mj = gl + cp;		//Get array sinaps
#pragma omp parallel
			{
#pragma omp for private(cn)
				for(int i = 0; i < cn; i++)
				{
					TypeNum SolveNeu = TypeNum(0);
					for(const TypeNum *gl2 = gl, const *s2 = s + i * cp; gl2 < mj; gl2++, s2++)
						SolveNeu += *gl2 * *s2;
					sl[i] = SolveNeu;
				}
			}
			/*in sl placed */
			std::swap(gl, sl);
		}
		memcpy(Out, gl, OutCount * sizeof(TypeNum));
		free(og);
		return true;
	}

};

#endif
