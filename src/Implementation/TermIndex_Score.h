#pragma once
#include "TermIndex_ScoreBase.h"
#include "../DocTable.h"


/* keep precomputed impact scores in uncompressed vector */
class _TermIndex_Score :public _TermIndex_ScoreBase<_TermIndex_Score> {
protected:
    std::pmr::vector<float> scores;

public:
    using Base = _TermIndex_ScoreBase<_TermIndex_Score>;
    friend Base::B;
    _TermIndex_Score(MemoryCounter& memcnt, Lexicon_Score::Iter& iter);
    friend class IndexForwardIter_Score;
    template<uint32_t> friend class IndexBackInserter_Score;

    /*
        binary format:
        a block: <4-last_did> <2-did_size> <2-freq_size> <did_size-VarBytes> <freq_size-VarBytes>
        terminator: <1-0>
    */
    void read_next_block(std::ifstream& fin, std::ifstream& fin2);
    void write(bool end, bool write_did, std::ofstream& fout, std::ofstream& fout2);
    void clear();
};



/* already keep uncompressed scores, so it doesn't have a score cache */
class IndexForwardIter_Score :public _IndexForwardIter<_TermIndex_Score, IndexForwardIter_Score> {
protected:
    /* cursor for scores vector */
    size_t cur_score;

public:
    using Base = _IndexForwardIter<_TermIndex_Score, IndexForwardIter_Score>;
    friend Base;
    IndexForwardIter_Score(_TermIndex_Score& _r) :Base(_r), cur_score(0) {}

    PostScore next();
    PostScore nextGEQ(uint32_t target_did);

protected:
    void clear_cache();
    inline void clear_other_cursor() { cur_score = 0; }
    void load_other_cache() {}
    void step_block();
    inline void step_cache() {
        cur_cache++; cur_score++;
    }
};



class BM25 {
public:
    static DocTable* docs;

    static constexpr float k1 = 1.2f;
    static constexpr float b = 0.75f;
    template<typename Index_t>
    inline static float score(Posting p, Index_t& index);
};

template<uint32_t BLOCK>
class IndexBackInserter_Score :public _IndexBackInserter<_TermIndex_Score, BLOCK, IndexBackInserter_Score<BLOCK> > {
public:
    using B = _IndexBackInserter<_TermIndex_Score, BLOCK, IndexBackInserter_Score<BLOCK> >;
    friend B;
    IndexBackInserter_Score(_TermIndex_Score& _r) :B(_r) {
        B::construct();
    }

protected:
    /* use posting to compute BM25 score */
    void _append(Posting p);
    void _append(PostScore p);
    void clear_cache();
    void unload_other_cache(uint32_t lastdid, uint16_t didbsize);
    /* this feature is only used for index merging
    shouldn't be called in this class */
    void try_load_last_cache(uint32_t pre_did) {
        throw std::exception();
    }
};



template<uint32_t BLOCK = g::BLOCK>
class TermIndex_Score :public _TermIndex_Score {
    using _TermIndex_Score::_TermIndex_Score;
public:
    using ForwardIter = IndexForwardIter_Score;
    using BackInserter = typename IndexBackInserter_Score<BLOCK>;

    inline ForwardIter begin() {
        return ForwardIter(*this);
    }
    inline typename BackInserter back_inserter() {
        return BackInserter(*this);
    }
};


#include "TermIndex_Score.ipp"
