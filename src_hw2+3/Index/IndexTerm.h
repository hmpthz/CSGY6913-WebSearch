#pragma once
#include "helper.h"
#include "Lexicon.h"


/* byte size of a block shouldn't be too large so use 16bit integer */
struct BlockMetaData {
    uint32_t last_did; /* last doc_id of the block */
    uint16_t did_bsize; /* byte size of compressed doc_id */
    uint16_t freq_bsize; /* byte size of compressed frequency */
    BlockMetaData(uint32_t a, uint16_t b, uint16_t c) :last_did(a), did_bsize(b), freq_bsize(c) {}
};

/*
    each IndexTerm should either be used for read or write, kept in an input or output buffer.
    postings are stored in compressed form in memory, but need to maintain an uncompressed cache.
*/
class IndexTerm {
protected:
    /* "pre_did" for the first block in memory, used for taking difference
    initial state start_did = 0.
    start_did = last_did of the last block in memory, so after clear bytes the next block can use it. */
    uint32_t start_did;

    /* cursor for file offset, cache, blocks and compressed bytes in memory */

    /* indicate if the file operation of this term is finished while reading */
    uint64_t cur_fpos;
    uint32_t cur_cache;
    uint32_t cur_block;
    size_t cur_byte;

    CompressedBytes b;
    /* associated to CompressedBytes, uncompressed cache don't have meta data */
    std::vector<BlockMetaData> blocks_info;
    /* a block of uncompressed, undifferenced doc_id and frequency cache */
    vector_u32 did_cache;
    vector_u32 freq_cache;


public:
    LexiconIter info;

    IndexTerm(LexiconIter iter);
    size_t byte_size();
    inline uint32_t block_size() {
        return (uint32_t)blocks_info.size();
    }
    inline bool is_blocks_full() {
        return block_size() >= g::N_BLOCKS;
    }
    inline bool is_endfile() {
        return cur_fpos >= g::ival(info).end_offset;
    }
    /* get the next posting from cache based on cursor
    if cache is exhausted, load cache from next block
    if it's the last block in memory, throw IndexEndInMemory */
    Posting get_next();
    /* append posting to the cache, increase # of docs in lexicon
    [return]
    0: if cache is not full 
    added bytes: if cache is full, unload cache*/
    size_t append(Posting p);
    /* get the next posting that docid equals or greater than target_did
    search through blocks to get the target block
    if no block in memory satisfies the target, throw IndexEndInMemory */
    Posting get_nextGEQ(uint32_t target_did);
    /* load data from CompressedBytes to cache based on cur_block, cur_byte
    should clear cache before this method
    guarantee cursors are not out of bound, move cur_block, cur_byte afterwards */
    void load_cache();
    /* unload from cache to CompressedBytes and generate meta data
    should clear cache after this method */
    size_t unload_cache();
    void clear_cache();
    /* reset all cursors to the first block, clear cache and blocks so that it can be queried again.
    the reset depends on whether current blocks in memory start from the first block in file,
    [return]
    0: remain current blocks in memory, don't need to read from the first block
    cleared bytes: current blocks in memory are cleared, need to read from the first block */
    size_t reset_read_state();

    /*
        text format: uncompressed ascii numbers
        a block: <last_did> (space) <# of postings in this block> (space) [<doc_id> (space) ....] [<frequency> (space) ....] (\n)
        terminator: <'#'> (\n)
        binary format:
        a block: <4-last_did> <2-did_size> <2-freq_size> <did_size-CompressedBytes> <freq_size-CompressedBytes>
        no delimeter between blocks!
        terminator: <1-0>
    */

    /* should check is_endfile before this method */
    size_t read_next_block(std::ifstream& fin);
    /* write all CompressedBytes with meta data, won't write cache data
    if start offset in the lexicon hasn't been set, set to the current file pointer.
    [params]
    (end) whether to set the end offset in lexicon, then write terminator of this index list */
    void write_bytes(std::ofstream& fout, bool end);
    /* clear all CompressedBytes with meta data, set start_did, reset cur_block, cur_byte */
    size_t clear_bytes();
};


namespace g::SIZE {
    /* estimated # of postings in cache, some index have full block cache while most of them have only 1 or 2 */
    constexpr uint32_t POSTINGCACHE_ESTIMATED = BLOCK / 4;
    /* overhead of IndexTerm, includes size of cache */
    constexpr uint32_t INDEXTERM = sizeof(IndexTerm) + 2 * sizeof(uint32_t) * POSTINGCACHE_ESTIMATED;
    constexpr uint32_t BLOCKMETA = sizeof(BlockMetaData);
}