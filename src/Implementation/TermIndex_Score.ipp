

template<typename Index_t>
inline float BM25::score(Posting p, Index_t& index) {
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


inline PostScore IndexForwardIter_Score::next() {
    auto p = PostScore(
        did_cache[cur_cache],
        r->scores[cur_score]);
    step_cache();
    return p;
}

inline PostScore IndexForwardIter_Score::nextGEQ(uint32_t target_did) {
    auto p = PostScore(
        did_cache[cur_cache],
        r->scores[cur_score]);
    //step_cache(); // IMPORTANT: don't increment it, this is WRONG!
    return p;
}

inline void IndexForwardIter_Score::clear_cache() {
    cur_cache = 0;
    did_cache.clear();
}

inline void IndexForwardIter_Score::step_block() {
    cur_byte += r
        ->blocks_meta[cur_block].did_bsize;
    // get actual # of postings of a block
    cur_score += r
        ->blocks_meta[cur_block].score_bsize / sizeof(float);
    cur_block++;
}


template<uint32_t BLOCK>
inline void IndexBackInserter_Score<BLOCK>::_append(Posting p) {
    B::did_cache.emplace_back(p.doc_id);
    B::r->scores.emplace_back(BM25::score(p, *B::r));
}

template<uint32_t BLOCK>
inline void IndexBackInserter_Score<BLOCK>::_append(PostScore p) {
    B::did_cache.emplace_back(p.doc_id);
    B::r->scores.emplace_back(p.score);
}

template<uint32_t BLOCK>
inline void IndexBackInserter_Score<BLOCK>::clear_cache() {
    B::did_cache.clear();
}

template<uint32_t BLOCK>
inline void IndexBackInserter_Score<BLOCK>::unload_other_cache(uint32_t lastdid, uint16_t didbsize) {
    B::r->blocks_meta.emplace_back(
        lastdid, didbsize, (uint16_t)(B::did_cache.size() * sizeof(float)));
}
