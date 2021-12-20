

inline Posting _Index::FreqForwardIter::next() {
    auto p = Posting(
        did_cache[cur_cache],
        freq_cache[cur_cache]);
    step_cache();
    return p;
}

inline Posting _Index::FreqForwardIter::nextGEQ(uint32_t target_did) {
    auto p = Posting(
        did_cache[cur_cache],
        freq_cache[cur_cache]);
    //step_cache(); // IMPORTANT: don't increment it, this is WRONG!
    return p;
}

inline void _Index::FreqForwardIter::clear_cache() {
    cur_cache = 0;
    did_cache.clear();
    freq_cache.clear();
}

inline void _Index::FreqForwardIter::load_other_cache() {
    uint16_t freqbsize = r
        ->blocks_meta[cur_block].freq_bsize;
    r->bytes.decompress(cur_byte, freqbsize, freq_cache);
    // move cursor
    cur_byte += freqbsize;
}

inline void _Index::FreqForwardIter::step_block() {
    cur_byte += r
        ->blocks_meta[cur_block].did_bsize;
    cur_byte += r
        ->blocks_meta[cur_block].freq_bsize;
    cur_block++;
}


template<uint32_t BLOCK>
inline void _Index::FreqBackInserter<BLOCK>::_append(Posting p) {
    B::did_cache.emplace_back(p.doc_id);
    freq_cache.emplace_back(p.frequency);
}

template<uint32_t BLOCK>
inline void _Index::FreqBackInserter<BLOCK>::clear_cache() {
    B::did_cache.clear();
    freq_cache.clear();
}

template<uint32_t BLOCK>
inline void _Index::FreqBackInserter<BLOCK>::unload_other_cache(uint32_t lastdid, uint16_t didbsize) {
    uint16_t freqbsize = B::r->bytes.compress(freq_cache);
    B::r->blocks_meta.emplace_back(lastdid, didbsize, freqbsize);
}

template<uint32_t BLOCK>
inline void _Index::FreqBackInserter<BLOCK>::try_load_last_cache(uint32_t pre_did) {
    auto& last_block = B::r->blocks_meta.back();
    size_t cur_byte_freq = B::r->bytes.size() - last_block.freq_bsize;
    size_t cur_byte_did = cur_byte_freq - last_block.did_bsize;
    B::r->bytes.decompress(cur_byte_did, last_block.did_bsize, B::did_cache);

    if (B::did_cache.size() >= BLOCK) {
        // last block is full, give up
        B::did_cache.clear();
        return;
    }

    VarBytes::undifference(pre_did, B::did_cache);
    B::r->bytes.decompress(cur_byte_freq, last_block.freq_bsize, freq_cache);

    // erase last block
    B::r->bytes.resize(cur_byte_did);
    B::r->blocks_meta.resize(B::r->block_size() - 1);
    // fblock decrease one
    B::r->fblock--;
}
