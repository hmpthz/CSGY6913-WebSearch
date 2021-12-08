#pragma once
#include "IndexBuffers_ScoreBase.h"


namespace InputBuffer {
    class Score :public _ScoreBase<TermIndex_Score<>, Score> {
    public:
        inline TermIndex_Score<>::ForwardIter index_begin(TermIndex_Score<>& index) {
            return index.begin();
        }
    };
}


namespace OutputBuffer {
    class Score :public _ScoreBase<TermIndex_Score<>, Score> {
    public:
        inline TermIndex_Score<>::BackInserter index_back_inserter(TermIndex_Score<>& index) {
            return index.back_inserter();
        }
    };
}
