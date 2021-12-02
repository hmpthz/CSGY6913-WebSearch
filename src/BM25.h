#pragma once
#include "DocTable.h"
//#include <cmath>


class BM25Scoring {
public:
    static DocTable* docs;

    template<typename Index_t>
    inline float score(Posting p, Index_t& index) {
        uint32_t N = (uint32_t)docs->size();
        uint32_t ft = g::ival(index.info).n_docs;
        uint32_t fdt = p.frequency;
        uint32_t len_d = docs->get_item(p.doc_id).len;
        float len_avg_d = docs->get_avg_len();
        float K = k1 * ((1 - b) + b * (len_d / len_avg_d));
        float IDF = ((float)(N - ft) + 0.5f) / ((float)ft + 0.5f);
        IDF = std::logf(IDF);
        float weight = ((k1 + 1) * fdt) / (K + fdt);
        return IDF * weight;
    }
};