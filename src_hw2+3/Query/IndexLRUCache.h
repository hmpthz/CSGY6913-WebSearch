#pragma once
#include "../Index/IndexBuffer.h"
#include <unordered_map>


/* map terms to list nodes (iterator) that are kept in LRUCache memory */
using IndexCacheMap = std::unordered_map<std::string, IndexTermIter>;

class IndexLRUCache : public IndexBuffer {
protected:
    IndexCacheMap map;

public:
    IndexLRUCache();
    /* check if the size is too small, throw exception */
    void set_capacity(size_t sz);
    /* handle IndexEndInMemory Exception, read following blcoks with N_BLOCKS limit */
    Posting get_next_limit(IndexTerm& index);
    /* wrapper method of IndexTerm.get_nextGEQ
    handle IndexEndInMemory Exception, read following blcoks with N_BLOCKS limit
    if endfile, throw IndexEndInFile */
    Posting get_nextGEQ_limit(IndexTerm& index, uint32_t target_did);
    /* read_next_block until buffer is full or endfile or blocks reach N_BLOCKS limit */
    void read_blocks_limit(IndexTerm& index);
    /* wrapper method of IndexTerm.reset_read_state 
    if current blocks in memory are cleared, read_blocks_limit starts from the first block*/
    void reset_read_state_limit(IndexTerm& index);

    /* get IndexTerm from LRUCache memory, move it to list front
    if not in IndexCacheMap, call put_index */
    IndexTermIter get_index(const std::string& term);
    /* try to get term from Lexicon, read index from file and put it into list front
    if LRUCache is full, estimate index size and remove list back until it can be put into memory
    if term not in Lexicon, throw TermNotFound */
    IndexTermIter put_index(const std::string& term);

    /* read terms from file then put corresponding index into LRUCache*/
    void load_cache_list(const char* filename);
    /* write corresponding terms of index in LRUCache to file*/
    void dump_cache_list(const char* filename);
};


namespace g::SIZE {
    /* estimated size of a compressed posting (doc_id 2.0bytes, frequency 1.0bytes) */
    constexpr float POSTING_ESTIMATED = 3.0f;
    /* overhead of an IndexCacheMap item */
    constexpr uint32_t INDEXLRU_MAPITEM = sizeof(IndexCacheMap::value_type);
    /* added overhead of IndexLRUCache from IndexBuffer */
    constexpr uint32_t INDEXLRU_ADDED = sizeof(IndexLRUCache) - INDEXBUF;
}