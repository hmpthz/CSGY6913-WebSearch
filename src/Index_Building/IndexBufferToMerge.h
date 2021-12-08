#pragma once
#include "../Implementation/TermIndex_Freq.h"
#include "../IndexBuffer.h"


class InputBufferToMerge :public InputBuffer::_Base<TermIndex_Freq<>, Lexicon_Freq, InputBufferToMerge> {
public:
    inline void index_read_blocks(TermIndex_Freq<>& index) {
        index.try_read_blocks<false>(fin);
    }
    inline TermIndex_Freq<>::ForwardIter index_begin(TermIndex_Freq<>& index) {
        return index.begin();
    }
    /* compare the front IndexTerm in each IndexBuffer, get the term with min byte order
    if all IndexBuffers are empty, throw exception */
    static int min_front(std::vector<InputBufferToMerge>& bufs);
};



class OutputBufferToMerge :public OutputBuffer::_Base<TermIndex_Freq<>, Lexicon_Freq, OutputBufferToMerge> {
public:
    inline void index_write(bool end, TermIndex_Freq<>& index) {
        index.write(end, write_did, fout);
    }
    inline TermIndex_Freq<>::BackInserter index_back_inserter(TermIndex_Freq<>& index) {
        return index.back_inserter();
    }
    /* convert all postings to index, initially IndexBuffer should be empty */
    void transfer_from_postings(PostingsBuffer& srcbuf);
};