#pragma once
#include "IndexBuffer_ScoreBase.h"
#include "TermIndex_Freq.h"
#include "TermIndex_Score.h"
#include "TermIndex_Linear.h"
#include "TermIndex_Log.h"
#include "TermIndex_Adapt.h"


namespace InputBuffer {
    class Freq :public _Base<Index::Freq<>, Lexicon_Freq, Freq> {
    public:
        inline void index_read_blocks(TermIndex& index) {
            index.try_read_blocks<false>(fin);
        }
        inline IndexForwardIter index_begin(TermIndex& index) {
            return index.begin();
        }
    };


    class Score :public _ScoreBase<Index::Score<>, Score> {
    public:
        inline IndexForwardIter index_begin(TermIndex& index) {
            return index.begin();
        }
    };


    template<uint32_t BITS>
    class Linear :public _ScoreBase<Index::Linear<BITS>, Linear<BITS> > {
    public:
        using B = _ScoreBase<Index::Linear<BITS>, Linear<BITS> >;
        LinearQuantizer<BITS> quantizer;
        inline typename B::IndexForwardIter index_begin(typename B::TermIndex& index) {
            return index.begin(quantizer);
        }
        inline void read_quantizer() {
            quantizer.read(B::fin2);
        }
    };


    template<uint32_t BITS>
    class Log :public _ScoreBase<Index::Log<BITS>, Log<BITS> > {
    public:
        using B = _ScoreBase<Index::Log<BITS>, Log<BITS> >;
        LogQuantizer<BITS> quantizer;
        inline typename B::IndexForwardIter index_begin(typename B::TermIndex& index) {
            return index.begin(quantizer);
        }
        inline void read_quantizer() {
            quantizer.read(B::fin2);
        }
    };


    template<uint32_t BITS, uint32_t EXP>
    class Adaptive :public _ScoreBase<Index::Adaptive<BITS, EXP>, Adaptive<BITS, EXP> > {
    public:
        using B = _ScoreBase<Index::Adaptive<BITS, EXP>, Adaptive<BITS, EXP> >;
        AdaptiveFloatQuantizer<BITS, EXP> quantizer;
        inline typename B::IndexForwardIter index_begin(typename B::TermIndex& index) {
            return index.begin(quantizer);
        }
    };
}


namespace OutputBuffer {
    class Score :public _ScoreBase<Index::Score<>, Score> {
    public:
        inline IndexBackInserter index_back_inserter(TermIndex& index) {
            return index.back_inserter();
        }
    };


    template<uint32_t BITS>
    class Linear :public _ScoreBase<Index::Linear<BITS>, Linear<BITS> > {
    public:
        using B = _ScoreBase<Index::Linear<BITS>, Linear<BITS> >;
        LinearQuantizer<BITS> quantizer;
        inline typename B::IndexBackInserter index_back_inserter(typename B::TermIndex& index) {
            return index.back_inserter(quantizer);
        }
        inline void write_quantizer() {
            quantizer.write(B::fout2);
        }
    };


    template<uint32_t BITS>
    class Log :public _ScoreBase<Index::Log<BITS>, Log<BITS> > {
    public:
        using B = _ScoreBase<Index::Log<BITS>, Log<BITS> >;
        LogQuantizer<BITS> quantizer;
        inline typename B::IndexBackInserter index_back_inserter(typename B::TermIndex& index) {
            return index.back_inserter(quantizer);
        }
        inline void write_quantizer() {
            quantizer.write(B::fout2);
        }
    };


    template<uint32_t BITS, uint32_t EXP>
    class Adaptive :public _ScoreBase<Index::Adaptive<BITS, EXP>, Adaptive<BITS, EXP> > {
    public:
        using B = _ScoreBase<Index::Adaptive<BITS, EXP>, Adaptive<BITS, EXP> >;
        AdaptiveFloatQuantizer<BITS, EXP> quantizer;
        inline typename B::IndexBackInserter index_back_inserter(typename B::TermIndex& index) {
            return index.back_inserter(quantizer);
        }
    };
}
