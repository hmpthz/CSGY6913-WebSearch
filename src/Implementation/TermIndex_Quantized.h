#pragma once
#include "TermIndex_ScoreBase.h"


class _TermIndex_Quantized :public _TermIndex_ScoreBase< _TermIndex_Quantized> {

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
        throw std::exception();
    }
};


#include "TermIndex_Quantized.ipp"
