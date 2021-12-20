

template<typename T, typename D>
inline PostScore _Index::_QuantizedForwardIter<T, D>::next() {
    auto p = PostScore(
        B::did_cache[B::cur_cache],
        score_cache[B::cur_cache]);
    step_cache();
    return p;
}

template<typename T, typename D>
inline PostScore _Index::_QuantizedForwardIter<T, D>::nextGEQ(uint32_t target_did) {
    auto p = PostScore(
        B::did_cache[B::cur_cache],
        score_cache[B::cur_cache]);
    //step_cache(); // IMPORTANT: don't increment it, this is WRONG!
    return p;
}

template<typename T, typename D>
inline void _Index::_QuantizedForwardIter<T, D>::clear_cache() {
    B::cur_cache = 0;
    B::did_cache.clear();
    score_cache.clear();
}

template<typename T, typename D>
inline void _Index::_QuantizedForwardIter<T, D>::step_block() {
    B::cur_byte += B::r
        .blocks_meta[B::cur_block].did_bsize;
    B::cur_byte2 += B::r
        .blocks_meta[B::cur_block].score_bsize;
    B::cur_block++;
}


template<typename T, uint32_t BLOCK, typename D>
inline void _Index::_QuantizedBackInserter<T, BLOCK, D>::_append(PostScore p) {
    B::did_cache.emplace_back(p.doc_id);
    score_cache.emplace_back(p.score);
}

template<typename T, uint32_t BLOCK, typename D>
inline void _Index::_QuantizedBackInserter<T, BLOCK, D>::clear_cache() {
    B::did_cache.clear();
    score_cache.clear();
}
