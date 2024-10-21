#include "IndexLRUCache.h"
#include <filesystem>
namespace fs = std::filesystem;


IndexLRUCache::IndexLRUCache() {
    byte_size += g::SIZE::INDEXLRU_ADDED;
}

void IndexLRUCache::set_capacity(size_t sz) {
    constexpr size_t min_size = g::SIZE::INDEXBUF + g::SIZE::INDEXLRU_ADDED
        + (size_t)(g::SIZE::POSTING_ESTIMATED * g::BLOCK + g::SIZE::BLOCKMETA) * g::N_BLOCKS;
    if (sz <= min_size) {
        std::cout << "[ERROR] LRUCache size too small!!\n";
        throw std::exception();
    }
    capacity = sz;
}

Posting IndexLRUCache::get_next_limit(IndexTerm& index) {
    try {
        return index.get_next();
    }
    // IndexEndInMemory Exception
    catch (g::Exception) {
        if (index.is_endfile()) {
            throw g::Exception::IndexEndInFile;
        }
        else {
            // clear index in memory and continue to read from file
            byte_size -= index.clear_bytes();
            read_blocks_limit(index);
        }
    }
    // get again, this time don't need try-catch because we've read following blocks
    return index.get_next();
}

Posting IndexLRUCache::get_nextGEQ_limit(IndexTerm& index, uint32_t target_did) {
    while (true) {
        try {
            return index.get_nextGEQ(target_did);
        }
        // IndexEndInMemory Exception
        catch (g::Exception) {
            if (index.is_endfile()) {
                throw g::Exception::IndexEndInFile;
            }
            else {
                byte_size -= index.clear_bytes();
                read_blocks_limit(index);
            }
        }
    }
}

void IndexLRUCache::read_blocks_limit(IndexTerm& index) {
    while (!index.is_endfile() && !is_full() && !index.is_blocks_full()) {
        byte_size += index.read_next_block(fin);
    }
}

void IndexLRUCache::reset_read_state_limit(IndexTerm& index) {
    size_t sz = index.reset_read_state();
    if (sz > 0) {
        byte_size -= sz;
        read_blocks_limit(index);
    }
}

IndexTermIter IndexLRUCache::get_index(const std::string& term) {
    auto cachemap_iter = map.find(term);
    if (cachemap_iter == map.end()) {
        return put_index(term);
    }
    auto& index_iter = g::ival(cachemap_iter);
    // move to front
    ilist.splice(ilist.begin(), ilist, index_iter);
    return index_iter;
}

IndexTermIter IndexLRUCache::put_index(const std::string& term) {
    auto lex_iter = lex.get_iter(term);
    auto& lex_val = g::ival(lex_iter);

    // size of the index to be put into LRUcache
    size_t index_size;
    // # of blocks of the index, check if it exceeds N_BLOCKS limit
    uint32_t n_blocks = (uint32_t)std::ceilf((float)lex_val.n_docs / g::BLOCK);
    // the whole index list can be put into LRUcache
    if (n_blocks <= g::N_BLOCKS) {
        index_size = g::SIZE::INDEXTERM + (lex_val.end_offset - lex_val.start_offset);
    }
    else { // only put N_BLOCKS, estimate size
        index_size = g::SIZE::INDEXTERM
            + (size_t)(g::SIZE::POSTING_ESTIMATED * g::BLOCK + g::SIZE::BLOCKMETA) * g::N_BLOCKS;
    }

    // remove index at the back until it's not full
    while (byte_size + index_size >= capacity) {
        auto back_iter = std::prev(ilist.end());
        byte_size -= back_iter->byte_size();
        ilist.erase(back_iter);
    }

    // insert the index to the front
    ilist.emplace_front(lex_iter);
    byte_size += g::SIZE::INDEXTERM;
    auto iiter = ilist.begin();
    read_blocks_limit(*iiter);
    // put into cachemap
    map.try_emplace(term, iiter);
    return iiter;
}

void IndexLRUCache::load_cache_list(const char* filename) {
    std::ifstream tmp_fin(fs::path(filename), ios::binary);
    while (true) {
        std::string term;
        tmp_fin >> term;
        if (tmp_fin.tellg() == -1) break;

        ilist.emplace_back(lex.get_iter(term));
        byte_size += g::SIZE::INDEXTERM;
        auto iiter = std::prev(ilist.end());
        read_blocks_limit(*iiter);
        map.try_emplace(term, iiter);
    }
}

void IndexLRUCache::dump_cache_list(const char* filename) {
    std::ofstream tmp_fout(fs::path(filename), ios::binary);
    for (auto& index : ilist) {
        tmp_fout << g::ikey(index.info) << '\n';
    }
}
