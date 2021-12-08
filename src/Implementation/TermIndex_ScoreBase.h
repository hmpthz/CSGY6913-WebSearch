#pragma once
#include "../Lexicon.h"
#include "../TermIndex.h"


/*
    var-bytes doc_id are stored in index file
    precomputed impact scores and block meta data are stored in another file
*/
struct TermInfo_Score {
    uint64_t start_off; /* start offset in index file of a term */
    uint64_t start_off2; /* start offset in score file of a term */
    uint32_t n_blocks; /* # of blocks in index file of a term */
    uint32_t n_docs; /* # of docs containing the term */
    TermInfo_Score(uint64_t a, uint64_t b, uint32_t c, uint32_t d) :start_off(a), start_off2(b), n_blocks(c), n_docs(d) {}
    TermInfo_Score() :start_off(-1), start_off2(-1), n_blocks(0), n_docs(0) {}
};

class Lexicon_Score :public _Lexicon<TermInfo_Score, Lexicon_Score> {
public:
    /*
        binary format:
        <str-term> <1-\0> <8-start offset> <8-start offset2> <4-n_blocks> <4-n_docs>
    */

    Iter read_next();
    void write(Iter iter);
};



struct BlockMeta_Score {
    uint32_t last_did; /* last doc_id of the block */
    uint16_t did_bsize; /* byte size of compressed doc_id */
    uint16_t score_bsize; /* byte size of scores, either uncompressed or quantized */
    BlockMeta_Score(uint32_t a, uint16_t b, uint16_t c) :last_did(a), did_bsize(b), score_bsize(c) {}
    BlockMeta_Score() :last_did(0), did_bsize(0), score_bsize(0) {} // WHY PMR needs this default constructor ????
};

template<typename Derived>
class _TermIndex_ScoreBase :public _TermIndex<BlockMeta_Score, Lexicon_Score::Iter, Derived> {
protected:
    /* current offset in the second file which has scores and block meta */
    uint64_t fpos2;

public:
    using B = _TermIndex<BlockMeta_Score, Lexicon_Score::Iter, Derived>;
    _TermIndex_ScoreBase(MemoryCounter& memcnt, Lexicon_Score::Iter& iter) :
        B(memcnt, iter) {
        fpos2 = g::ival(B::info).start_off2;
    }

    inline void seek_fpos(std::ifstream& fin, std::ifstream& fin2) {
        fin.seekg(B::fpos); fin2.seekg(fpos2);
    }
    inline void set_fpos(std::ifstream& fin, std::ifstream& fin2) {
        B::fpos = fin.tellg(); fpos2 = fin2.tellg();
    }
};
