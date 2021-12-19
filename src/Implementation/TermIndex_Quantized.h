#pragma once
#include "TermIndex_ScoreBase.h"
#include "../Compression/BitVector.h"


class _TermIndex_Quantized :public _TermIndex_ScoreBase<_TermIndex_Quantized> {
protected:
    Bits::Vec bits;

public:
    using Base = _TermIndex_ScoreBase<_TermIndex_Quantized>;
    friend Base::B;
    _TermIndex_Quantized(MemoryCounter& memcnt, Lexicon_Score::Iter& iter);

    /*
        binary format:
        a block: <4-last_did> <2-did_size> <2-score_bsize> <did_size-VarBytes> <quantized_scores-bytes(BitVector)>
        terminator: <1-0>
    */
    void read_next_block(std::ifstream& fin, std::ifstream& fin2);
    void write(bool end, bool write_did, std::ofstream& fout, std::ofstream& fout2);
    void clear();
};



template<typename TermIndex_t, typename Derived>
class _IndexForwardIter_Quantized :public _IndexForwardIter<TermIndex_t, Derived> {
protected:
    /* cursor for the quantized bytes */
    size_t cur_byte2;
    std::vector<float> score_cache;

public:
    using B = _IndexForwardIter<TermIndex_t, Derived>;
    _IndexForwardIter_Quantized(TermIndex_t& _r) :B(_r), cur_byte2(0) {
        score_cache.reserve(g::RESERVE);
    }

    PostScore next();
    PostScore nextGEQ(uint32_t target_did);

protected:
    void clear_cache();
    inline void clear_other_cursor() { cur_byte2 = 0; }
    void step_block();
    inline void step_cache() { B::cur_cache++; }
};



template<typename TermIndex_t, uint32_t BLOCK, typename Derived>
class _IndexBackInserter_Quantized :public _IndexBackInserter<TermIndex_t, BLOCK, Derived> {
protected:
    std::vector<float> score_cache;

public:
    using B = _IndexBackInserter<TermIndex_t, BLOCK, Derived>;
    _IndexBackInserter_Quantized(TermIndex_t& _r) :B(_r) {
        B::contruct(); score_cache.reserve(g::RESERVE);
    }

protected:
    void _append(PostScore p);
    void clear_cache();
    void try_load_last_cache(uint32_t pre_did) {
        throw g::Exception::Unreachable;
    }
};


#include "TermIndex_Quantized.ipp"
