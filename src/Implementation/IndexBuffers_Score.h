#pragma once
#include "IndexBuffers_ScoreBase.h"


namespace InputBuffer {
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
