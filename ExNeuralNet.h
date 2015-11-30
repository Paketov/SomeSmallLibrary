#ifndef __NEURALNET_H_HAS_INCLUDED__
#define __NEURALNET_H_HAS_INCLUDED__

#include <malloc.h>
#include "ExTypeTraits.h"

template<typename TypeNum = int>
class NEURALNET
{

	class NEURAL_LAYER
	{
		friend NEURALNET;
		class __SINAPS_IN
		{

		public:
			inline __SINAPS_IN(NEURAL_LAYER* p, size_t n)
			{
				Count.v = p;
				Count.i = n;
			}

			union{
				class{
					friend __SINAPS_IN;
					struct {NEURAL_LAYER* v; size_t i;};
				public:
					operator size_t() const { return v->count_in_prev(); }
				} Count;
			};

			void ClearWeight(TypeNum SetVal)
			{
				TypeNum* v = Count.v->get_row();
				const size_t mc = Count;
				for(size_t i = 0;i < mc;i++)
				{
					v[i] = SetVal;
				}
			}

			/*Get or set sinaps weight*/
			inline TypeNum& operator[] (size_t Index) { return Count.v->at(Count.i, Index);}
		};
		friend __SINAPS_IN;
		inline TypeNum& at(size_t Neuron, size_t Sinaps) { return Count.v[Neuron * Count.pn + Sinaps]; }

		inline TypeNum* get_row() { return Count.v; }

		inline size_t count_in_prev() const { return Count.pn; }

		inline bool set_count_prev_neu(size_t NewCount)
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
			memset(Count.v, 0, l);
			Count.n = nCountNeuron;
			Count.pn = nCountNeuronInPrevLayer;
		}
		~NEURAL_LAYER()
		{
			if(Count.v != nullptr)
				free(Count.v);
		}


		void ClearAllWeight(TypeNum SetVal)
		{
			TypeNum* v = Count.v;
			const size_t mc = Count.n * Count.n;
			for(size_t i = 0;i < mc;i++)
			{
				v[i] = SetVal;
			}
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

		inline __SINAPS_IN operator[](size_t Index) { return __SINAPS_IN(this, Index); }
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
		{
		   delete CountLayers.nl[i];
		}
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
			inline operator size_t()  const
			{ 
				if(cl == 0)
					return 0;
				return nl[0]->count_in_prev(); 
			}

			size_t operator =(size_t NewInCount)
			{	
				if(cl == 0)
				{
					ic = NewInCount;
					return NewInCount;
				}
				if(nl[0]->set_count_prev_neu(NewInCount))
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
			inline operator size_t()  const
			{ 
				if(cl == 0)
					return 0;
				return nl[cl - 1]->Count; 
			}
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
			inline operator size_t() const {return mcn; }
		} MaxCountNeuronInLayer;

	};

	inline NEURAL_LAYER& operator[](size_t IndexLayer)
	{
		return CountLayers.nl[IndexLayer][0];
	}

	bool NewLayer(size_t CountNeuron, size_t IndexInsert = 0xffffffff)
	{
		if(IndexInsert > CountLayers) 
			IndexInsert = CountLayers;
		auto nl = (NEURAL_LAYER**)realloc(CountLayers.nl, (CountLayers + 1) * sizeof(NEURAL_LAYER*));
		if(nl == nullptr)
			return false;
		if(IndexInsert != CountLayers)
			memmove(nl + (IndexInsert + 1), nl + IndexInsert,  (CountLayers - IndexInsert) * sizeof(NEURAL_LAYER*));

		if(IndexInsert == CountLayers) //If last layer
		{
			nl[IndexInsert] = new NEURAL_LAYER((IndexInsert != 0) ? nl[IndexInsert - 1]->Count: CountLayers.ic, CountNeuron);	
		}else
		{
			nl[IndexInsert] = new NEURAL_LAYER(nl[IndexInsert + 1]->count_in_prev(), CountNeuron);	
			nl[IndexInsert + 1]->set_count_prev_neu(CountNeuron);
		}
		if(CountNeuron > CountLayers.mcn)		  
			CountLayers.mcn = CountNeuron;
		CountLayers.nl = nl;
		CountLayers.cl++;
		return true;
	}

	bool Recognize(const TypeNum* In, TypeNum* Out) const
	{			
		if(MaxCountNeuronInLayer == 0)
			return false;

		const size_t cn = MaxCountNeuronInLayer, cl = CountLayers, LenLay = cn * sizeof(TypeNum);
		TypeNum* gl = (TypeNum*)malloc(LenLay * 2), *sl = gl + cn, *og = gl;
		if(gl == nullptr)
			return false;

		memcpy(gl, In, InCount * sizeof(TypeNum));
		
		for(size_t il = 0; il < cl; il++)
		{
			NEURAL_LAYER* nl = CountLayers.nl[il];
			const size_t cn = nl->Count;			//Count neuron in this layer
			const size_t cp = nl->count_in_prev();	//Count neuron in previous layer
			const TypeNum* s = nl->get_row();		//Get array sinaps

			for(size_t CurNeu = 0; CurNeu < cn; CurNeu++)
			{
				TypeNum SolveNeu = TypeNum(0);
				for(size_t CurSinaps = 0; CurSinaps < cp; CurSinaps++)
				{
					SolveNeu += gl[CurSinaps] * s[CurNeu * cp + CurSinaps];
				}
				sl[CurNeu] = SolveNeu;
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
