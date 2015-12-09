#ifndef __NEURALNET_H_HAS_INCLUDED__
#define __NEURALNET_H_HAS_INCLUDED__

#include <malloc.h>
#include <omp.h>
#include <math.h>
#include <stdlib.h>

#include "ExTypeTraits.h"
/*
    ExNeuronNet 
    Paketov
    2015

Example:
    NEURALNET<float> Net;

    Net.InputCount = 10;
    Net.NewLayer(10);
    Net.NewLayer(10);
    Net.EnableAllBiases();

    float In[10], Out[10], TestOut[10];

    float new_val = -0.5;
    std::arr_set_elements(In, new_val);
    std::arr_set_elements(Out, new_val);

    In[0] = 0.5;
    In[5] = 0.5;
    In[3] = 0.5;

    Out[1] = 0.5;
    Out[3] = 0.5;
    Out[8] = 0.5;

    //Обучаем сеть
    Net.Randomize();
                        
    for(unsigned i = 0; i < 10; i++)
        err = hh.Learn(In, Out, 0.1, 0.000000001, 5000);

    //Проверяем обученность
    double Err = Net.Recognize(In, TestOut);

*/

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


        static TypeNum __fastcall Sig2(TypeNum In) { return (TypeNum(2) * x) / (TypeNum(1) + abs(x)); }
        static TypeNum __fastcall DerSig2(TypeNum In) { TypeNum temp = TypeNum(1) + abs(x); return TypeNum(2) / (temp * temp); }
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
                } Count; /*Количество входных весов*/

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
                } IndexMinWeigth;

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

            /*Задать определённое значение всем входным весам.*/
            inline void SetWeights(TypeNum SetVal)
            {
                for(TypeNum* v = Count.v->get_row() + Count.i * Count.v->get_count_prev(), *m = v + Count.v->get_count_prev(); v < m; v++)
                    *v = SetVal;
            }

            /*Получить входной вес для данного нейрона*/
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
            TypeNum* Biases;\
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
            Count.Biases = nullptr;
            ReverseActivateFunc = ActivateFunction = [](TypeNum v) -> TypeNum { return v; };
            DerActivateFunction = [](TypeNum in) -> TypeNum { return TypeNum(1); };
            SetWeights(TypeNum(0));
        }

        ~NEURAL_LAYER()
        {
            if(Count.v != nullptr)
                free(Count.v);
        }

        /*Задать определённое значение всем весам слоя*/
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
            } Count; /*Количество нейронов в слое*/

            class { 
                __LAYER_FIELDS;
            public: 
                inline operator size_t() const { return n * pn;} 
            } CountSinaps; /*Общее количество весов в слое*/

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

            class { 
                __LAYER_FIELDS;
            public: 
                inline operator bool() const { return Biases != nullptr;} 
                inline bool operator =(bool New) 
                {  
                    if(New)
                    {
                        if(Biases == nullptr)
                        {
                            Biases = (TypeNum*)malloc(n * sizeof(TypeNum));
                            for(size_t i = 0, mi = n;i < n;i++)
                               Biases[i] = TypeNum(0);
                        }
                    }else
                    {
                        if(Biases != nullptr)
                            free(Biases);
                    }
                    return New;
                } 
            } IsHaveBiases; /*Есть ли базис для данного слоя*/

            class{
                __LAYER_FIELDS;
            public: 
                inline TypeNum& operator[](size_t Index) { return Biases[Index]; }
            } Biases;	 /*Интератор для базисов*/
        };

        /*Получить определённый нейрон*/
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
        } CountLayers; /*Получить количество слоёв в сети*/

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
        } InputCount; /*Размер входного вектора*/
                
        class{
            __NEURALNET_FIELDS;
        public:
            inline operator size_t() const { return (cl == 0)? 0: size_t(nl[cl - 1]->Count); }
        } OutputCount;/*Размер выходного вектора*/

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
        } CountNeuron;/*Общее количество нейронов в сети*/

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
        } CountSinaps; /*Общее количество весов в сети*/

        class{
            __NEURALNET_FIELDS;
        public:
            inline operator size_t() const { return mcn; }
        } MaxCountNeuronInLayer; /*Максимальный размер слоя в сети*/
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

    /*
    Проверить ошибку для конкретного входного вектора и ожидаемого выходного.
     @In - Входной вектор размерности InputCount.
     @Out - Ожидаемый выходной вектор размерности OutputCount.
     @return - Возвращает квадратичную ошибку
    */
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

    /*Задать единое значение всем весам сети*/
    void SetWeights(TypeNum SetVal)
    {
        for(size_t i = 0; i < CountLayers.cl; i++)
            CountLayers.nl[i]->SetWeights(SetVal);
    }

    /*Нагенерировать случайные веса во всей сети*/
    void RandomizeWeights()
    {
        srand(time(nullptr));
        auto UnifiedRand = [](){return TypeNum(2) * ((TypeNum)rand() / ((TypeNum) RAND_MAX)) - 1;};
        for(size_t l = 0, ml = CountLayers;l < ml;l++) 
        { 
            TypeNum* w = LayerByIndex(l).get_row();
            TypeNum t =  sqrt(TypeNum(LayerByIndex(l).get_count_prev()));
            for(size_t q = 0, mq = LayerByIndex(l).CountSinaps; q < mq; q++) 
                w[q] = UnifiedRand() / t;
        } 
    }

    /*Нагенерировать случайные базисы для всех слоёв*/
    void RandomizeBiasis()
    {
        srand(time(nullptr));
        auto UnifiedRand = [](){return TypeNum(2) * ((TypeNum)rand() / ((TypeNum) RAND_MAX)) - 1;};
        for(size_t l = 0, ml = CountLayers;l < ml;l++) 
        { 
            if(!LayerByIndex(l).IsHaveBiases)
                continue;
            auto b = LayerByIndex(l).Count.Biases;
            TypeNum t = sqrt(TypeNum(LayerByIndex(l).get_count_prev()));
            for(size_t q = 0, mq = LayerByIndex(l).Count; q < mq; q++) 
                b[q] = UnifiedRand() / t;
        } 
    }

    /*Нагенерировать случайные весы и базисы для всех слоёв*/
    void Randomize()
    {
        RandomizeWeights();
        RandomizeBiasis();
    }

    /*Задать активизационную функцию для всей сети*/
    void SetActivateFuncInAllLayers(TACTIVATE_FUNC NewActivateFunc)
    {
        for(size_t l = 0; l < CountLayers; l++)
            LayerByIndex(l).ActivateFunction = NewActivateFunc;
    }

    /*Задать производную активизационной функции для всей сети*/
    void SetDerActivateFuncInAllLayers(TDER_ACTIVATE_FUNC NewDerActivateFunc)
    {
        for(size_t l = 0; l < CountLayers; l++)
            LayerByIndex(l).DerActivateFunction = NewDerActivateFunc;
    }

    /*Задать инверсированый вариант активизационной функции для всей сети (используется для GetReverse)*/
    void SetReverseActivateFuncInAllLayers(TACTIVATE_FUNC NewRevActivateFunc)
    {
        for(size_t l = 0; l < CountLayers; l++)
            LayerByIndex(l).ReverseActivateFunc = NewRevActivateFunc;
    }

    /*Включить базисы во всех слоях*/
    void EnableAllBiases()
    {
        for(size_t l = 0; l < CountLayers; l++)
            LayerByIndex(l).IsHaveBiases = true;
    }

    /*Отключить базисы во всех слоях*/
    void DisableAllBiases()
    {
        for(size_t l = 0; l < CountLayers; l++)
            LayerByIndex(l).IsHaveBiases = false;
    }

    /*Получить требуемый слой по индексу*/
    inline NEURAL_LAYER& operator[](size_t IndexLayer) { return CountLayers.nl[IndexLayer][0]; }

    /*
    Добавить новый слой
    @CountNeuron - Количество нейронов в создаваемом слое.
    @Index - Индекс, куда будет вставлятся слой
    @retun - Возвращает true в удачном случае
    */
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

    /*
    Удалить конкретный слой из сети
    @Index - Индекс слоя, который будет удаляться
    @retun - Возвращает true в удачном случае
    */
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
    
    /*
     Распознать входной вектор и вывести ассоциированый с ним другой вектор.
     @In - Входной вектор размерности InputCount
     @Out - Выходной вектор размерности OutputCount ассоциированый с @In
     @return - Возвращает true в удачном случае
    */
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
            if((*nl)->IsHaveBiases)
            {
                auto Biases = (*nl)->Count.Biases;
                for(TypeNum* dst = sl; dst < mdst; dst++, Biases++)
                {
                    TypeNum SolveNeu = TypeNum(0);
                    for(const TypeNum* gl2 = gl; gl2 < mgl2; gl2++, s++)
                        SolveNeu += *gl2 * *s;
                    SolveNeu += *Biases;
                    *dst = ActivateFunc(SolveNeu);
                }
            }else
            {
                for(TypeNum* dst = sl; dst < mdst; dst++)
                {
                    TypeNum SolveNeu = TypeNum(0);
                    for(const TypeNum* gl2 = gl; gl2 < mgl2; gl2++, s++)
                        SolveNeu += *gl2 * *s;
                    *dst = ActivateFunc(SolveNeu);
                }	
            }
            /*in sl placed */
            std::swap(gl, sl);
        }
        copy_arr_cast(Out, gl, OutputCount);
        free(og);
        return true;
    }

    /*
     Распознать входной вектор и вывести ассоциированый с ним другой вектор.
     Распараллеливается на все процессоры.
     @In - Входной вектор размерности InputCount
     @Out - Выходной вектор размерности OutputCount ассоциированый с @In
     @return - Возвращает true в удачном случае
    */
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
            if((*nl)->IsHaveBiases)
            {
                auto Biases = (*nl)->Count.Biases;
#pragma omp parallel
                {
#pragma omp for private(cn)
                    for(int i = 0; i < cn; i++)
                    {
                        TypeNum SolveNeu = TypeNum(0);
                        for(const TypeNum *gl2 = gl, const *s2 = s + i * cp; gl2 < mj; gl2++, s2++)
                            SolveNeu += *gl2 * *s2;
                        SolveNeu += *Biases;
                        sl[i] = ActivateFunc(SolveNeu);
                    }
                }
            }else
            {
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
            }
            /*in sl placed */
            std::swap(gl, sl);
        }
        copy_arr_cast(Out, gl, OutputCount);
        free(og);
        return true;
    }

    /*
     Получить обратное представление выходного вектора.
     @In - Вектор, для которого требуется получить обратное представление нейронной сети.
     @Out - Выходной вектор размерности InputCount.
     @return - Возвращает true в удачном случае
    */
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
            if((*nl)->IsHaveBiases)
            {
                auto Biases = (*nl)->Count.Biases;
                for(TypeNum* glt = gl; glt < mglt; glt++, Biases++)
                {
                    long double SumWeigths = 0.0;
                    TypeNum* m = s + OutCount;
                    for(const TypeNum* st = s; st < m; st++)
                        SumWeigths += *st;
                    if(SumWeigths == TypeNum(0)) { s = m; continue; }
                    long double tn = (RActivateFunc(*glt) - *Biases) / SumWeigths;
                    for(TypeNum* slt = sl; s < m; s++, slt++)
                        *slt += TypeNum(tn * *s);
                }
            }else
            {
                for(TypeNum* glt = gl; glt < mglt; glt++)
                {
                    long double SumWeigths = 0.0;
                    TypeNum* m = s + OutCount;
                    for(const TypeNum* st = s; st < m; st++)
                        SumWeigths += *st;
                    if(SumWeigths == TypeNum(0)) { s = m; continue; }
                    long double tn = RActivateFunc(*glt) / SumWeigths;
                    for(TypeNum* slt = sl; s < m; s++, slt++)
                        *slt += TypeNum(tn * *s);
                }			    
            }
            InCount = OutCount;
            std::swap(gl, sl);
        }
        copy_arr_cast(Out, gl, InputCount);
        free(og);
        return true;
    }

    /*
     Получить обратное представление выходного вектора.
     Распараллеливается на все процессоры.
     @In - Вектор, для которого требуется получить обратное представление нейронной сети.
     @Out - Выходной вектор размерности InputCount.
     @return - Возвращает true в удачном случае
    */
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
            if((*nl)->IsHaveBiases)
            {
                auto Biases = (*nl)->Count.Biases;
#pragma omp parallel
                {
#pragma omp for private(mglt)
                    for(int i = 0; i < mglt; i++, Biases++)
                    {
                        long double SumWeigths = 0.0;
                        TypeNum* a = s + i * OutCount, *ms = a + OutCount;
                        for(TypeNum* st = a; st < ms; st++)
                            SumWeigths += *st;
                        if(SumWeigths == TypeNum(0))
                            continue;
                        long double tn = (RActivateFunc(gl[i]) - *Biases) / SumWeigths;
                        for(TypeNum* slt = sl, const*st = a; st < ms; st++, slt++)
                        {
                            register TypeNum r = TypeNum(tn * *st);
#pragma omp atomic
                            *slt += r;				//interlocked add
                        }
                    }
                }
            } else
            {
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
            }
            InCount = OutCount;
            std::swap(gl, sl);
        }
        copy_arr_cast(Out, gl, InputCount);
        free(og);
        return true;
    }

    /*
     Учить сеть.
     @In -  Входной вектор размерности InputCount
     @Out - Вектор размерности OutputCount, который требуется ассоциировать с @In
     @SpeedLern - Скорость обучения (Обычно от 0.1 до 1.0)
     @ErrorMin - Ошибка - при достижении которой, требуется закончить обучение.
     @CountLoop - Максимальное количество прогона примера.
     @return - Возвращает конечную ошибку.
    */
    double Learn(const TypeNum* In, const TypeNum* Result, double SpeedLern = 0.5, double ErrorMin = 0.2, unsigned CountLoop = 5000)
    {
        if(CountLayers == 0)
            return -1.0;

        if(!InitLayerOuts())
            return -1.0;

        double Error;
        for(unsigned MainLoopCounter = 0; MainLoopCounter < CountLoop; MainLoopCounter++)
        {
            Error = 0.0;
            auto SourceNeurons = In;
            
            //Вычисляем результат для входа сети
            for(NEURAL_LAYER **nl = CountLayers.nl, **mnl = nl + size_t(CountLayers); nl < mnl; nl++)
            {
                auto Out = (*nl)->Count.TmpOut;
                auto InputWeigths = (*nl)->get_row();
                auto MaxSourceNeurons = SourceNeurons + (*nl)->get_count_prev(); 
                auto MaxDest = Out + size_t((*nl)->Count);
                auto Derivatives = (*nl)->Count.TmpDer;
                TACTIVATE_FUNC ActivateFunc = (*nl)->ActivateFunction;
                TDER_ACTIVATE_FUNC DerActivateFunc = (*nl)->DerActivateFunction;
                if((*nl)->IsHaveBiases)
                {
                    auto Biases = (*nl)->Count.Biases;
                    for(TypeNum* DestNeuron = Out; DestNeuron < MaxDest; DestNeuron++, Derivatives++, Biases++)
                    { //For each neuron in current layer
                        TypeNum SolveNeu = TypeNum(0);
                        for(const TypeNum* CurSourceNeuron = SourceNeurons; 
                            CurSourceNeuron < MaxSourceNeurons; 
                            CurSourceNeuron++, InputWeigths++) //for each weigth
                            SolveNeu += *CurSourceNeuron * *InputWeigths;
                        SolveNeu += *Biases; 
                        *DestNeuron = ActivateFunc(SolveNeu);
                        *Derivatives = DerActivateFunc(SolveNeu);
                    }
                }else
                {
                    for(TypeNum* DestNeuron = Out; DestNeuron < MaxDest; DestNeuron++, Derivatives++)
                    { //For each neuron in current layer
                        TypeNum SolveNeu = TypeNum(0);
                        for(const TypeNum* CurSourceNeuron = SourceNeurons; 
                            CurSourceNeuron < MaxSourceNeurons; 
                            CurSourceNeuron++, InputWeigths++) //for each weigth
                            SolveNeu += *CurSourceNeuron * *InputWeigths;
                        *DestNeuron = ActivateFunc(SolveNeu);
                        *Derivatives = DerActivateFunc(SolveNeu);
                    }
                }
                SourceNeurons = Out;
            }

            //Правим веса последнего слоя
            {
                NEURAL_LAYER& nl	= LayerByIndex(CountLayers - 1);
                auto Derivatives	= nl.Count.TmpDer;
                auto InputWeigths	= nl.get_row();
                auto Delta			= nl.Count.TmpDelta;
                auto OutPrev		= (CountLayers > 1)? (LayerByIndex(CountLayers - 2).Count.TmpOut): In;
                auto CurOut			= nl.Count.TmpOut;
                size_t CountPrev	= nl.get_count_prev();
                if(nl.IsHaveBiases)
                {
                    auto Biases = nl.Count.Biases;
                    for(size_t q = 0, mq = OutputCount; q < mq; q++)
                    {
                        TypeNum CurError = Result[q] - CurOut[q]; 
                        Error += CurError * CurError; 					
                        TypeNum CurDelta = SpeedLern * (Delta[q] = Derivatives[q] * CurError);//Delta[q] = F`(OUT[q]) * (T[q] - OUT[q])
                        for(size_t p = 0; p < CountPrev; p++)
                            InputWeigths[p] += CurDelta * OutPrev[p];					      //w[p,q](i+1) = q[p,q](i) + n * Delta[q] * OUT[p]
                        InputWeigths += CountPrev;
                        Biases[q] += CurDelta;
                    }
                }else
                {
                    for(size_t q = 0, mq = OutputCount; q < mq; q++)
                    {
                        TypeNum CurError = Result[q] - CurOut[q]; 
                        Error += CurError * CurError; 
                        TypeNum CurDelta = SpeedLern * (Delta[q] = Derivatives[q] * CurError);//Delta[q] = F`(OUT[q]) * (T[q] - OUT[q])
                        for(size_t p = 0; p < CountPrev; p++)
                            InputWeigths[p] += CurDelta * OutPrev[p];					      //w[p,q](i+1) = q[p,q](i) + n * Delta[q] * OUT[p]
                        InputWeigths += CountPrev;
                    }
                }
            }

            //Правим веса внутренних слоёв
            for(int l = CountLayers - 2; l >= 0; l--)
            {
                NEURAL_LAYER& nl		= LayerByIndex(l);
                auto Derivatives		= nl.Count.TmpDer;
                auto InputWeigths		= nl.get_row();
                auto Delta				= nl.Count.TmpDelta;
                auto WeigthToNextLayer	= nl.get_row();
                auto DeltaNextLayer		= LayerByIndex(l + 1).Count.TmpDelta;
                const TypeNum* OutPrev	= (l < 1)? In: LayerByIndex(l - 1).Count.TmpOut;
                size_t NeuronCountInNextLayer = LayerByIndex(l + 1).Count;
                auto CountPrev			= nl.get_count_prev();
                size_t NeuronCount		= nl.Count;
                if(nl.IsHaveBiases)
                {
                    auto Biases = nl.Count.Biases;
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
                        Biases[q] += CurDelta;
                        InputWeigths += CountPrev;
                    }
                }else
                {
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
            Error /= 2.0;
            if(Error < ErrorMin)
                break;
        }

        UninitLayerOuts();
        return Error;
    }

    /*
     Учить сеть.
     Распараллеливается на все процессоры.
     @In -  Входной вектор размерности InputCount
     @Out - Вектор размерности OutputCount, который требуется ассоциировать с @In
     @SpeedLern - Скорость обучения (Обычно от 0.1 до 1.0)
     @ErrorMin - Ошибка - при достижении которой, требуется закончить обучение.
     @CountLoop - Максимальное количество прогона примера.
     @return - Возвращает конечную ошибку.
    */
    double LearnParallel(const TypeNum* In, const TypeNum* Result, double SpeedLern = 0.5, double ErrorMin = 0.2, unsigned CountLoop = 5000)
    {
        if(CountLayers == 0)
            return -1.0;

        if(!InitLayerOuts())
            return -1.0;

        double Error;
        //Вычисляем результат для входа сети
        for(unsigned MainLoopCounter = 0; MainLoopCounter < CountLoop; MainLoopCounter++)
        {
            Error = 0.0;
            auto SourceNeurons = In;
            for(NEURAL_LAYER **nl = CountLayers.nl, **mnl = nl + size_t(CountLayers); nl < mnl; nl++)
            {
                auto Out					= (*nl)->Count.TmpOut;
                auto InputWeigths			= (*nl)->get_row();
                auto CountPrev				= (*nl)->get_count_prev();
                auto MaxSourceNeurons		= SourceNeurons + CountPrev; 
                int CountNeuron				= size_t((*nl)->Count);
                auto Der					= (*nl)->Count.TmpDer;
                TACTIVATE_FUNC ActivateFunc = (*nl)->ActivateFunction;
                TDER_ACTIVATE_FUNC DerActivateFunc = (*nl)->DerActivateFunction;
                if((*nl)->IsHaveBiases)
                {
                    auto Biases = (*nl)->Count.Biases;
#pragma omp parallel
                    {
#pragma omp for private(CountNeuron)
                        for(int i = 0; i < CountNeuron; i++)
                        { //For each neuron in current layer
                            TypeNum SolveNeu = TypeNum(0);
                            for(const TypeNum *CurSourceNeuron = SourceNeurons, const *CurInputWeigth = InputWeigths + i * CountPrev; 
                                CurSourceNeuron < MaxSourceNeurons; 
                                CurSourceNeuron++, CurInputWeigth++)
                                SolveNeu += *CurSourceNeuron * *CurInputWeigth;
                            SolveNeu += Biases[i];
                            Out[i] = ActivateFunc(SolveNeu);
                            Der[i] = DerActivateFunc(SolveNeu);
                        }
                    }
                }else
                {
#pragma omp parallel
                    {
#pragma omp for private(CountNeuron)
                        for(int i = 0; i < CountNeuron; i++)
                        { //For each neuron in current layer
                            TypeNum SolveNeu = TypeNum(0);
                            for(const TypeNum *CurSourceNeuron = SourceNeurons, const *CurInputWeigth = InputWeigths + i * CountPrev; 
                                CurSourceNeuron < MaxSourceNeurons; 
                                CurSourceNeuron++, CurInputWeigth++)
                                SolveNeu += *CurSourceNeuron * *CurInputWeigth;
                            Out[i] = ActivateFunc(SolveNeu);
                            Der[i] = DerActivateFunc(SolveNeu);
                        }
                    }				   
                }
                SourceNeurons = Out;
            }


            //Правим веса последнего слоя
            {
                NEURAL_LAYER& nl			= LayerByIndex(CountLayers - 1);
                auto Derivatives			= nl.Count.TmpDer;
                auto InputWeigths			= nl.get_row();
                auto Delta					= nl.Count.TmpDelta;
                const TypeNum* OutPrev		= (CountLayers > 1)? (LayerByIndex(CountLayers - 2).Count.TmpOut): In;
                auto CurOut					= nl.Count.TmpOut;
                auto CountPrev				= nl.get_count_prev();
                int mq						= OutputCount;
                if(nl.IsHaveBiases)
                {
                    auto Biases = nl.Count.Biases;
#pragma omp parallel
                    {
#pragma omp for private(mq)
                        for(int q = 0; q < mq; q++)
                        {
                            TypeNum CurError = Result[q] - CurOut[q];
#pragma omp atomic
                            Error += CurError * CurError; 					
                            TypeNum CurDelta = SpeedLern * (Delta[q] = Derivatives[q] * CurError);	//Delta[q] = F`(OUT[q]) * (T[q] - OUT[q])
                            TypeNum* w = InputWeigths + q * CountPrev;
                            for(size_t p = 0; p < CountPrev; p++)
                                w[p] += CurDelta * OutPrev[p];					//w[p,q](i+1) = q[p,q](i) + n * Delta[q] * OUT[p]
                            Biases[q] += CurDelta;
                        }
                    }
                }else
                {
#pragma omp parallel
                    {
#pragma omp for private(mq)
                        for(int q = 0; q < mq; q++)
                        {
                            TypeNum CurError = Result[q] - CurOut[q];
#pragma omp atomic
                            Error += CurError * CurError; 					
                            TypeNum CurDelta = SpeedLern * (Delta[q] = Derivatives[q] * CurError);	//Delta[q] = F`(OUT[q]) * (T[q] - OUT[q])
                            TypeNum* w = InputWeigths + q * CountPrev;
                            for(size_t p = 0; p < CountPrev; p++)
                                w[p] += CurDelta * OutPrev[p];					//w[p,q](i+1) = q[p,q](i) + n * Delta[q] * OUT[p]
                        }
                    }				
                }
            }
            //Правим веса внутренних слоёв
            for(int l = CountLayers - 2; l >= 0; l--)
            {		
                NEURAL_LAYER& nl				= LayerByIndex(l);
                auto Derivatives				= nl.Count.TmpDer;
                auto InputWeigths				= nl.get_row();
                const TypeNum* OutPrev			= (l < 1)? In: LayerByIndex(l - 1).Count.TmpOut;
                auto Delta						= nl.Count.TmpDelta;
                auto DeltaNextLayer				= LayerByIndex(l + 1).Count.TmpDelta;
                auto WeigthToNextLayer			= nl.get_row();
                auto CountPrev					= nl.get_count_prev();
                int NeuronCount					= nl.Count;
                size_t NeuronCountInNextLayer	= LayerByIndex(l + 1).Count;
                if(nl.IsHaveBiases)
                {
                    auto Biases = nl.Count.Biases;
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
                            Biases[q] += CurDelta;
                        }
                    }
                }else
                {
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
            Error /= 2.0;
            if(Error < ErrorMin)
                break;
        }
        UninitLayerOuts();
        return Error;
    }
};

#endif
