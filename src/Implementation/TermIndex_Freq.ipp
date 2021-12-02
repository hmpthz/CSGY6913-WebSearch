

inline Posting IndexForwardIter_Freq::next() {
    auto p = Posting(
        B::did_cache[B::cur_cache],
        freq_cache[B::cur_cache]);
    B::cur_cache++;
    return p;
}

inline Posting IndexForwardIter_Freq::nextGEQ(uint32_t target_did) {
    auto p = Posting(
        B::did_cache[B::cur_cache],
        freq_cache[B::cur_cache]);
    //cur_cache++; // IMPORTANT: don't increment it, this is WRONG!
    return p;
}

inline void IndexForwardIter_Freq::clear_cache() {
    B::cur_cache = 0;
    B::did_cache.clear();
    freq_cache.clear();
}

inline void IndexForwardIter_Freq::load_other_cache() {
    uint16_t freqbsize = B::r
        .blocks_meta[B::cur_block].freq_bsize;
    B::r.bytes.decompress(B::cur_byte, freqbsize, freq_cache);
    // move cursor
    B::cur_byte += freqbsize;
}

inline void IndexForwardIter_Freq::skip_one_block() {
    B::cur_byte += B::r
        .blocks_meta[B::cur_block].did_bsize;
    B::cur_byte += B::r
        .blocks_meta[B::cur_block].freq_bsize;
    B::cur_block++;
}


template<uint32_t BLOCK>
void IndexBackInserter_Freq<BLOCK>::_append(Posting p) {
    B::did_cache.emplace_back(p.doc_id);
    freq_cache.emplace_back(p.frequency);
}

template<uint32_t BLOCK>
void IndexBackInserter_Freq<BLOCK>::clear_cache() {
    B::did_cache.clear();
    freq_cache.clear();
}

template<uint32_t BLOCK>
void IndexBackInserter_Freq<BLOCK>::unload_other_cache(uint32_t lastdid, uint16_t didbsize) {
    uint16_t freqbsize = B::r.bytes.compress(freq_cache);
    B::r.blocks_meta.emplace_back(lastdid, didbsize, freqbsize);
}

template<uint32_t BLOCK>
void IndexBackInserter_Freq<BLOCK>::try_load_last_cache(uint32_t pre_did) {
    auto& last_block = B::r.blocks_meta.back();
    size_t cur_byte_freq = B::r.bytes.size() - last_block.freq_bsize;
    size_t cur_byte_did = cur_byte_freq - last_block.did_bsize;
    B::r.bytes.decompress(cur_byte_did, last_block.did_bsize, B::did_cache);

    if (B::did_cache.size() >= BLOCK) {
        // last block is full, give up
        B::did_cache.clear();
        return;
    }

    VarBytes::undifference(pre_did, B::did_cache);
    B::r.bytes.decompress(cur_byte_freq, last_block.freq_bsize, freq_cache);

    // erase last block
    B::r.bytes.resize(cur_byte_did);
    B::r.blocks_meta.resize(B::r.block_size() - 1);
    // fblock decrease one
    B::r.fblock--;
}
