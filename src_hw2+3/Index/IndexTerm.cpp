#include "IndexTerm.h"
// uncomment to disable assert()
// #define NDEBUG
#include <cassert>


IndexTerm::IndexTerm(LexiconIter iter) {
    info = iter;
    cur_fpos = g::ival(info).start_offset;
    start_did = 0;
    /* most of index have only 1 or 2 postings */
    did_cache.reserve(2);
    freq_cache.reserve(2);
    cur_cache = 0;
    cur_block = 0;
    cur_byte = 0;
}

size_t IndexTerm::byte_size() {
    return b.byte_size() + (size_t)block_size() * g::SIZE::BLOCKMETA + g::SIZE::INDEXTERM;
}

Posting IndexTerm::get_next() {
    // cache is exhausted, need to load
    if (cur_cache >= did_cache.size()) {
        clear_cache();
        // blocks in memory is exhausted
        if (cur_block >= block_size()) {
            throw g::Exception::IndexEndInMemory;
        }
        else {
            load_cache();
        }
    }

    auto p = Posting(did_cache[cur_cache], freq_cache[cur_cache]);
    cur_cache++;
    return p;
}

size_t IndexTerm::append(Posting p) {
    size_t sz = 0;
    // cache is full
    if (did_cache.size() >= g::BLOCK) {
        sz = unload_cache();
        clear_cache();
    }

    did_cache.emplace_back(p.doc_id);
    freq_cache.emplace_back(p.frequency);
    // increase # of docs 
    g::ival(info).n_docs++;
    return sz;
}

Posting IndexTerm::get_nextGEQ(uint32_t target_did) {
    // cache is exhausted, or the current cache isn't the target block
    if (cur_cache >= did_cache.size() || did_cache.back() < target_did) {
        clear_cache();
        // search through blocks
        while (cur_block < block_size() && blocks_info[cur_block].last_did < target_did) {
            cur_byte += blocks_info[cur_block].did_bsize;
            cur_byte += blocks_info[cur_block].freq_bsize;
            cur_block++;
        }
        // blocks in memory is exhausted
        if (cur_block >= block_size()) {
            throw g::Exception::IndexEndInMemory;
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

    auto p = Posting(did_cache[cur_cache], freq_cache[cur_cache]);
    // IMPORTANT: don't increment it, this is wrong!
    //cur_cache++;
    return p;
}

void IndexTerm::load_cache() {
    // undifference doc_id
    uint32_t pre_did;
    if (cur_block > 0) {
        pre_did = blocks_info[cur_block - 1].last_did;
    }
    else pre_did = start_did;

    uint16_t didbsize = blocks_info[cur_block].did_bsize;
    uint16_t freqbsize = blocks_info[cur_block].freq_bsize;

    b.decompress(cur_byte, didbsize, did_cache);
    CompressedBytes::undifference(pre_did, did_cache);
    // move cursor
    cur_byte += didbsize;

    b.decompress(cur_byte, freqbsize, freq_cache);
    // move cursor
    cur_byte += freqbsize;
    cur_block++;
}

size_t IndexTerm::unload_cache() {
    if (did_cache.size() == 0) return 0;
    // difference doc_id
    uint32_t pre_did;
    if (block_size() > 0) {
        pre_did = blocks_info.back().last_did;
    }
    else pre_did = start_did;
    uint32_t lastdid = did_cache.back();

    CompressedBytes::difference(pre_did, did_cache);

    uint16_t didbsize = b.compress(did_cache);
    uint16_t freqbsize = b.compress(freq_cache);

    blocks_info.emplace_back(lastdid, didbsize, freqbsize);
    size_t sz = (size_t)didbsize + (size_t)freqbsize + g::SIZE::BLOCKMETA;
    return sz;
}

void IndexTerm::clear_cache() {
    cur_cache = 0;
    did_cache.clear();
    freq_cache.clear();
}

size_t IndexTerm::reset_read_state() {
    clear_cache();

    // this means current blocks in memory start from the first block in file
    // so don't need to clear current blocks and read again
    if (start_did == 0) {
        cur_block = 0;
        cur_byte = 0;
        return 0;
    }
    else {
        size_t sz = clear_bytes();
        cur_fpos = g::ival(info).start_offset;
        start_did = 0;
        return sz;
    }
}

size_t IndexTerm::clear_bytes() {
    if (block_size() == 0) return 0;
    start_did = blocks_info.back().last_did;
    size_t sz = b.byte_size() + (size_t)block_size() * g::SIZE::BLOCKMETA;
    cur_block = 0;
    cur_byte = 0;
    b.bytes.clear();
    blocks_info.clear();
    return sz;
}

#ifdef TEXT_FILE
/* I/O File in text format */

size_t IndexTerm::read_next_block(std::ifstream& fin) {
    // in debug text format, we read uncompressed ascii numbers from file
    // so they need to be differenced and compress again into memory
    // use extra static cache, don't mess with member's cache
    thread_local static vector_u32 tmp_did;
    thread_local static vector_u32 tmp_freq;

    // reading...
    // seek to correct position
    fin.seekg(cur_fpos);
    uint32_t lastdid;
    uint32_t n_postings;
    fin >> lastdid >> n_postings;

    uint32_t tmp;
    for (uint32_t i = 0; i < n_postings; i++) {
        fin >> tmp;
        tmp_did.emplace_back(tmp);
    }
    assert(lastdid == tmp_did.back());
    for (uint32_t i = 0; i < n_postings; i++) {
        fin >> tmp;
        tmp_freq.emplace_back(tmp);
    }
    // set cur_fpos
    cur_fpos = fin.tellg();

    // unload tmp cache
    uint32_t pre_did;
    if (block_size() > 0) {
        pre_did = blocks_info.back().last_did;
    }
    else pre_did = start_did;
    CompressedBytes::difference(pre_did, tmp_did);

    uint16_t didbsize = b.compress(tmp_did);
    uint16_t freqbsize = b.compress(tmp_freq);
    // clear tmp cache
    tmp_did.clear();
    tmp_freq.clear();

    blocks_info.emplace_back(lastdid, didbsize, freqbsize);
    size_t sz = (size_t)didbsize + (size_t)freqbsize + g::SIZE::BLOCKMETA;
    return sz;
}

void IndexTerm::write_bytes(std::ofstream& fout, bool end) {
    // in debug text format, we write uncompressed ascii numbers to file
    // so they need to be decompress and undifference again into file
    // use extra static cache, don't mess with member's cache
    thread_local static vector_u32 tmp_did;
    thread_local static vector_u32 tmp_freq;

    // set start offset
    if (g::ival(info).start_offset == -1) {
        g::ival(info).start_offset = fout.tellp();
    }

    cur_byte = 0;
    // write all blocks
    for (cur_block = 0; cur_block < block_size(); cur_block++) {
        // load tmp cache
        auto& meta = blocks_info[cur_block];
        uint32_t pre_did;
        if (cur_block > 0) {
            pre_did = blocks_info[cur_block - 1].last_did;
        }
        else pre_did = start_did;

        b.decompress(cur_byte, meta.did_bsize, tmp_did);
        CompressedBytes::undifference(pre_did, tmp_did);
        // move cursor
        cur_byte += meta.did_bsize;

        b.decompress(cur_byte, meta.freq_bsize, tmp_freq);
        // move cursor
        cur_byte += meta.freq_bsize;

        // writing...
        fout << meta.last_did << ' ' << tmp_did.size() << ' ';
        for (auto i : tmp_did) {
            fout << i << ' ';
        }
        for (auto i : tmp_freq) {
            fout << i << ' ';
        }
        fout << '\n';
        // clear tmp cache
        tmp_did.clear();
        tmp_freq.clear();
    }


    if (end) {
        // minus 1 to make sure is_endfile
        g::ival(info).end_offset = (size_t)fout.tellp() - 1;
        fout << '#' << '\n';
    }
}

#else
/* I/O File in binary format */

size_t IndexTerm::read_next_block(std::ifstream& fin) {
    // seek to correct position
    fin.seekg(cur_fpos);

    std::istreambuf_iterator<char> ifiter(fin);
    
    uint32_t lastdid;
    uint16_t didbsize, freqbsize;
    fin.read((char*)&lastdid, sizeof(lastdid));
    fin.read((char*)&didbsize, sizeof(didbsize));
    fin.read((char*)&freqbsize, sizeof(freqbsize));

    std::copy_n(ifiter, didbsize, std::back_inserter(b.bytes));
    ifiter++; // IMPORTANT!!
    std::copy_n(ifiter, freqbsize, std::back_inserter(b.bytes));
    ifiter++; // IMPORTANT!!
    // set cur_fpos
    cur_fpos = fin.tellg();

    blocks_info.emplace_back(lastdid, didbsize, freqbsize);
    size_t sz = (size_t)didbsize + (size_t)freqbsize + g::SIZE::BLOCKMETA;
    return sz;
}

void IndexTerm::write_bytes(std::ofstream& fout, bool end) {
    // set start offset
    if (g::ival(info).start_offset == -1) {
        g::ival(info).start_offset = fout.tellp();
    }

    std::ostreambuf_iterator<char> ofiter(fout);

    cur_byte = 0;
    // write all blocks
    for (auto& meta : blocks_info) {
        fout.write((char*)&meta.last_did, sizeof(meta.last_did));
        fout.write((char*)&meta.did_bsize, sizeof(meta.did_bsize));
        fout.write((char*)&meta.freq_bsize, sizeof(meta.freq_bsize));

        std::copy_n(b.bytes.begin() + cur_byte, meta.did_bsize, ofiter);
        // move cursor
        cur_byte += meta.did_bsize;
        std::copy_n(b.bytes.begin() + cur_byte, meta.freq_bsize, ofiter);
        // move cursor
        cur_byte += meta.freq_bsize;
    }

    if (end) {
        // minus 1 to make sure is_endfile
        g::ival(info).end_offset = (size_t)fout.tellp() - 1;
        ofiter = '\0';
    }
}

#endif
