#pragma once
#include "TermIndex_ScoreBase.h"
#include "../DocTable.h"


class BM25 {
public:
    static DocTable* docs;

    static constexpr float k1 = 1.2f;
    static constexpr float b = 0.75f;
    template<typename Index_t>
    inline static float score(Posting p, Index_t& index);
};



namespace _Index {
    /* keep precomputed impact scores in uncompressed vector */
    class Score :public _ScoreBase<Score> {
    protected:
        std::pmr::vector<float> scores;

    public:
        using Base = _ScoreBase<Score>;
        friend Base::B;
        Score(MemoryCounter& memcnt, Lexicon_Score::Iter& iter);
        friend class ScoreForwardIter;
        template<uint32_t> friend class ScoreBackInserter;

        /*
            binary format:
            a block: <4-last_did> <2-did_size> <2-score_bsize> <did_size-VarBytes> <scores-floats>
            terminator: <1-0>
        */
        void read_next_block(std::ifstream& fin, std::ifstream& fin2);
        void write(bool end, bool write_did, std::ofstream& fout, std::ofstream& fout2);
        void clear();
    };



    /* already keep uncompressed scores, so it doesn't have a score cache */
    class ScoreForwardIter :public _ForwardIter<Score, ScoreForwardIter> {
    protected:
        /* cursor for scores vector */
        size_t cur_score;

    public:
        using Base = _ForwardIter<Score, ScoreForwardIter>;
        friend Base;
        ScoreForwardIter(Score& _r) :Base(_r), cur_score(0) {}

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



    template<uint32_t BLOCK>
    class ScoreBackInserter :public _BackInserter<Score, BLOCK, ScoreBackInserter<BLOCK> > {
    public:
        using B = _BackInserter<Score, BLOCK, ScoreBackInserter<BLOCK> >;
        friend B;
        ScoreBackInserter(Score& _r) :B(_r) {
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
            throw g::Exception::Unreachable;
        }
    };
}



namespace Index {
    template<uint32_t BLOCK = g::BLOCK>
    class Score :public _Index::Score {
        using _Index::Score::Score;
    public:
        using ForwardIter = _Index::ScoreForwardIter;
        using BackInserter = typename _Index::ScoreBackInserter<BLOCK>;

        inline ForwardIter begin() {
            return ForwardIter(*this);
        }
        inline BackInserter back_inserter() {
            return BackInserter(*this);
        }
    };
}


#include "TermIndex_Score.ipp"
