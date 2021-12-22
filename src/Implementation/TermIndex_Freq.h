#pragma once
#include "../Lexicon.h"
#include "../TermIndex.h"


/* doc_id and frequency are stored in one index file */
struct TermInfo_Freq {
    uint64_t start_off; /* start offset in index file of a term */
    uint32_t n_blocks; /* # of blocks in index file of a term */
    uint32_t n_docs; /* # of docs containing the term */
    TermInfo_Freq(uint64_t a, uint32_t b, uint32_t c) :start_off(a), n_blocks(b), n_docs(c) {}
    TermInfo_Freq() :start_off(-1), n_blocks(0), n_docs(0) {}
};

class Lexicon_Freq :public _Lexicon<TermInfo_Freq, Lexicon_Freq> {
public:
    /*
        text format:
        <term> (space) <start offset> (space) <n_blocks> (space) <n_docs> (\n)
        binary format:
        <str-term> <1-\0> <8-start offset> <4-n_blocks> <4-n_docs>
    */

    Iter read_next();
    void write(Iter iter);
};



struct BlockMeta_Freq {
    uint32_t last_did; /* last doc_id of the block */
    uint16_t did_bsize; /* byte size of compressed doc_id */
    uint16_t freq_bsize; /* byte size of compressed frequency */
    BlockMeta_Freq(uint32_t a, uint16_t b, uint16_t c) :last_did(a), did_bsize(b), freq_bsize(c) {}
    BlockMeta_Freq() :last_did(0), did_bsize(0), freq_bsize(0) {} // WHY PMR needs this default constructor ????
};

namespace _Index {
    class Freq :public _Base<BlockMeta_Freq, Lexicon_Freq::Iter, Freq> {
        using _Base::_Base; // C++11, inherit all constructors;
    public:
        using Base = _Base<BlockMeta_Freq, Lexicon_Freq::Iter, Freq>;
        friend Base; // IMPORTANT!! make sure base class can access protected derived class members
        friend class FreqForwardIter;
        template<uint32_t> friend class FreqBackInserter;

        /*
            text format: uncompressed ascii numbers
            a block: <last_did> (space) <# of postings in this block> (space) [<doc_id> (space) ....] [<frequency> (space) ....] (\n)
            terminator: <'#'> (\n)
            binary format:
            a block: <4-last_did> <2-did_size> <2-freq_size> <did_size-VarBytes> <freq_size-VarBytes>
            terminator: <1-0>
        */

        void read_next_block(std::ifstream& fin);
        void write(bool end, bool write_did, std::ofstream& fout);
        void clear();
        inline void seek_fpos(std::ifstream& fin) {
            fin.seekg(fpos);
        }
        inline void set_fpos(std::ifstream& fin) {
            fpos = fin.tellg();
        }
    };



    class FreqForwardIter :public _ForwardIter<Freq, FreqForwardIter> {
    protected:
        vector_u32 freq_cache;

    public:
        using Base = _ForwardIter<Freq, FreqForwardIter>;
        friend Base;
        FreqForwardIter(Freq& _r) :Base(_r) {
            freq_cache.reserve(g::RESERVE);
        }

        Posting next();
        Posting nextGEQ(uint32_t target_did);

    protected:
        void clear_cache();
        void clear_other_cursor() {}
        void load_other_cache();
        void step_block();
        inline void step_cache() { cur_cache++; }
    };



    template<uint32_t BLOCK>
    class FreqBackInserter :public _BackInserter<Freq, BLOCK, FreqBackInserter<BLOCK> > {
    protected:
        vector_u32 freq_cache;

    public:
        // base class is not instantiated, so compiler doesn't know base class members' names!!
        // HAVE TO use *this* or *Base::* due to name-lookup
        using B = _BackInserter<Freq, BLOCK, FreqBackInserter<BLOCK> >;
        friend B;
        FreqBackInserter(Freq& _r) :B(_r) {
            B::construct(); freq_cache.reserve(g::RESERVE);
        }

    protected:
        void _append(Posting p);
        void clear_cache();
        void unload_other_cache(uint32_t lastdid, uint16_t didbsize);
        void try_load_last_cache(uint32_t pre_did);
    };
}



namespace Index {
    /* final TermIndex specialized class with iterators */
    template<uint32_t BLOCK = g::BLOCK>
    class Freq :public _Index::Freq {
        using _Index::Freq::Freq;
    public:
        using ForwardIter = _Index::FreqForwardIter;
        using BackInserter = typename _Index::FreqBackInserter<BLOCK>;

        inline ForwardIter begin() {
            return ForwardIter(*this);
        }
        inline BackInserter back_inserter() {
            return BackInserter(*this);
        }
    };
}


#include "TermIndex_Freq.ipp"
