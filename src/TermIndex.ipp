

template<typename B, typename LexiconIter_t, typename D>
inline TermIndex<B, LexiconIter_t, D>::TermIndex(MemoryCounter& memcnt, LexiconIter_t& iter) :
    bytes(&memcnt), blocks_meta(&memcnt), info(iter) {
    start_did = 0;
    fpos = g::ival(info).start_off;
    fblock = 0;
}

template<typename Block_t, typename LexiconIter_t, typename Derived>
inline void TermIndex<Block_t, LexiconIter_t, Derived>::update_start_did() {
    // used for further blocks take doc_id difference
    start_did = blocks_meta.back().last_did;
}

template<typename B, typename L, typename D>
template<bool LIMIT, typename ...inFiles>
inline void TermIndex<B, L, D>::try_read_blocks(inFiles&&... infiles) {
    if (block_size() > 0) {
        update_start_did();
        static_cast<D*>(this)->clear();
    }

    uint32_t n_blocks = g::ival(info).n_blocks;
    uint32_t max_blocks;
    if constexpr (LIMIT) {
        max_blocks = std::min(n_blocks, fblock + g::N_BLOCKS);
    }
    else {
        max_blocks = n_blocks;
    }

    static_cast<D*>(this)->seek_fpos(infiles...);
    auto memcnt = static_cast<MemoryCounter*>(bytes.get_allocator().resource());

    while (fblock < max_blocks && !memcnt->is_full()) {
        static_cast<D*>(this)->read_next_block(infiles...);
        fblock++;
    }

    static_cast<D*>(this)->set_fpos(infiles...);
}


template<typename TermIndex_t, typename D>
inline IndexForwardIter<TermIndex_t, D>::IndexForwardIter(TermIndex_t& _r) :r(_r) {
    cur_cache = 0;
    cur_block = 0;
    cur_byte = 0;
    // most of the terms only have very few postings;
    did_cache.reserve(g::RESERVE);
    // this means current blocks in memory don't start from the first block in file
    // so need to clear and reload from the first block
    if (r.start_did != 0) {
        r.clear();
        r.start_did = 0;
        r.fpos = g::ival(r.info).start_off;
        r.fblock = 0;
    }
}

template<typename T, typename D>
inline void IndexForwardIter<T, D>::load_cache() {
    // undifference doc_id
    uint32_t pre_did;
    if (cur_block > 0) {
        pre_did = r.blocks_meta[cur_block - 1].last_did;
    }
    else pre_did = r.start_did;

    uint16_t didbsize = r.blocks_meta[cur_block].did_bsize;
    r.bytes.decompress(cur_byte, didbsize, did_cache);
    VarBytes::undifference(pre_did, did_cache);
    // move cursor
    cur_byte += didbsize;

    static_cast<D*>(this)->load_other_cache();
    cur_block++;
}

template<typename T, typename D>
inline bool IndexForwardIter<T, D>::has_next() {
    // cache is exhausted, need to load
    if (cur_cache >= did_cache.size()) {
        static_cast<D*>(this)->clear_cache();
        // blocks in memory is exhausted
        if (cur_block >= r.block_size()) {
            // set byte cursor to 0
            cur_block = 0;
            cur_byte = 0;
            // index end in memory
            return false;
        }
        else {
            load_cache();
        }
    }
    return true;
}

template<typename T, typename D>
inline bool IndexForwardIter<T, D>::has_nextGEQ(uint32_t target_did) {
    // cache is exhausted, or the current cache isn't the target block
    while (cur_cache >= did_cache.size() || did_cache.back() < target_did) {
        static_cast<D*>(this)->clear_cache();
        // search through blocks
        while (cur_block < r.block_size() && r.blocks_meta[cur_block].last_did < target_did) {
            static_cast<D*>(this)->skip_one_block();
        }
        // blocks in memory is exhausted
        if (cur_block >= r.block_size()) {
            // set byte cursor to 0
            cur_block = 0;
            cur_byte = 0;
            // index end in memory
            return false;
        }
        // target block found
        else {
            load_cache();
        }
    }

    // search through cache to get nextGEQ
    for (; cur_cache < did_cache.size(); cur_cache++) {
        if (did_cache[cur_cache] >= target_did) {
            break;
        }
    }
    return true;
}


template<typename T, uint32_t BLOCK, typename D>
inline void IndexBackInserter<T, BLOCK, D>::construct() {
    if (r.block_size() > 0) {
        uint32_t pre_did;
        if (r.block_size() > 1) {
            auto& second_last_block = r.blocks_meta[r.block_size() - 2];
            pre_did = second_last_block.last_did;
        }
        else pre_did = r.start_did;

        static_cast<D*>(this)->try_load_last_cache(pre_did);
    }
}

template<typename T, uint32_t BLOCK, typename D>
inline void IndexBackInserter<T, BLOCK, D>::destruct() {
    if (did_cache.size() > 0) {
        unload_cache();
    }
}

template<typename T, uint32_t BLOCK, typename D>
template<typename Arg>
inline void IndexBackInserter<T, BLOCK, D>::append(Arg p) {
    static_cast<D*>(this)->_append(p);
    // update term info
    g::ival(r.info).n_docs++;
    // check if cache is full
    if (did_cache.size() >= BLOCK) {
        unload_cache();
    }
}

template<typename T, uint32_t BLOCK, typename D>
inline void IndexBackInserter<T, BLOCK, D>::unload_cache() {
    // difference doc_id
    uint32_t pre_did;
    if (r.block_size() > 0) {
        pre_did = r.blocks_meta.back().last_did;
    }
    else pre_did = r.start_did;

    uint32_t lastdid = did_cache.back();
    VarBytes::difference(pre_did, did_cache);
    uint16_t didbsize = r.bytes.compress(did_cache);

    static_cast<D*>(this)->unload_other_cache(lastdid, didbsize);
    // update term info
    r.fblock++;

    static_cast<D*>(this)->clear_cache();

    auto memcnt = static_cast<MemoryCounter*>(r.bytes.get_allocator().resource());
    if (memcnt->is_full()) {
        throw g::Exception::BufferFull;
    }
}
