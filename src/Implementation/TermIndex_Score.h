#pragma once
#include "../Lexicon.h"
#include "../TermIndex.h"
#include "../Compression/BitVector.h"


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

class Lexicon_Score :public Lexicon<TermInfo_Score, Lexicon_Score> {
public:
    /*
        binary format:
        <str-term> <1-\0> <8-start offset> <8-start offset2> <4-n_blocks> <4-n_docs>
    */

    Iter read_next();
    void write(Iter iter);
};
