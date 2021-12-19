#pragma once
#include "IndexBuffer_ScoreBase.h"
#include "TermIndex_Freq.h"
#include "TermIndex_Score.h"


namespace InputBuffer {
    class Freq :public _Base<TermIndex_Freq<>, Lexicon_Freq, Freq> {
    public:
        inline void index_read_blocks(TermIndex& index) {
            index.try_read_blocks<false>(fin);
        }
        inline IndexForwardIter index_begin(TermIndex& index) {
            return index.begin();
        }
    };


    class Score :public _ScoreBase<TermIndex_Score<>, Score> {
    public:
        inline IndexForwardIter index_begin(TermIndex& index) {
            return index.begin();
        }
    };
}


namespace OutputBuffer {
    class Score :public _ScoreBase<TermIndex_Score<>, Score> {
    public:
        inline IndexBackInserter index_back_inserter(TermIndex& index) {
            return index.back_inserter();
        }
    };
}
