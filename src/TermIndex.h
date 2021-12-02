#pragma once
#include "MemoryBuffer.h"
#include "Compression/VarBytes.h"


template<typename Block_t, typename LexiconIter_t, typename Derived>
class TermIndex {
protected:
    /* "pre_did" for the first block in memory, used for taking difference
    initial state start_did = 0.
    start_did = last_did of the last block in memory when clear() is called, so that the next block can use it. */
    uint32_t start_did;

    /* current offset in the index file */
    uint64_t fpos;
    /* total # of blocks that've been read into memory */
    uint32_t fblock;

    VarBytes bytes;
    std::pmr::vector<Block_t> blocks_meta;

public:
    LexiconIter_t info;

    /* [params]
    (memcnt) MemoryResource Counter, limit buffer size
    (iter) associated Lexicon iter
    */
    TermIndex(MemoryCounter& memcnt, LexiconIter_t& iter);
    inline uint32_t block_size() {
        return (uint32_t)blocks_meta.size();
    }
    inline bool is_endfile() {
        return fblock >= g::ival(info).n_blocks;
    }

    /* Instantiated class would have iterators
    should declare friend class for iterators */
    //ForwardIter begin(...);
    //BackInserter back_inserter(...);
    template<typename, typename> friend class IndexForwardIter;
    template<typename, uint32_t, typename> friend class IndexBackInserter;

    /* read blocks until LIMIT is reached or buffer is full
    should check is_endfile() before
    [params]
    (LIMIT) whether limits N_BLOCKS in memory
    (inFiles...) ifstream or other types of file */
    template<bool LIMIT, typename ...inFiles>
    void try_read_blocks(inFiles&&... infiles);
    /* read data of next block from file(s) into memory */
    template<typename ...inFiles>
    void read_next_block(inFiles... infiles); // <Derived>
    /* write all data from memory to file(s)
    [params]
    (end) whether it is the end of this index list
    (write_did) whether var-bytes doc_id will be written
    (outFiles...) ofstream or other types of file */
    template<typename ...outFiles>
    void write(bool end, bool write_did, outFiles... outfiles); // <Derived>
    void clear(); // <Derived>
    void update_start_did();

    /* call before read blocks */
    template<typename ...inFiles>
    void seek_fpos(inFiles... infiles); // <Derived>
    /* call after read blocks */
    template<typename ...inFiles>
    void set_fpos(inFiles... infiles); // <Derived>
};



/*
    keep a block of uncompressed cache
    different types of TermIndex may have different types of data
    (but all have doc_ids)
*/
template<typename TermIndex_t, typename Derived>
class IndexForwardIter {
protected:
    TermIndex_t& r; // reference

    uint32_t cur_cache;
    uint32_t cur_block;
    size_t cur_byte;
    vector_u32 did_cache;

public:
    IndexForwardIter(TermIndex_t& _r);
    /* may have different return types */
    //Posting next();
    //Posting nextGEQ(uint32_t target_did);
    /* should be called before next() method
    may load cache or read blocks etc. */
    bool has_next();
    /* should be called nextGEQ() method
    may load cache, read blocks and search through blocks etc. */
    bool has_nextGEQ(uint32_t target_did);

protected:
    void load_cache();
    void clear_cache();

    /* load data other than doc_id */
    void load_other_cache();
    void skip_one_block();
};



template<typename TermIndex_t, uint32_t BLOCK, typename Derived>
class IndexBackInserter {
protected:
    TermIndex_t& r; // reference

    vector_u32 did_cache;

public:
    /* con/destructor of base class CANNOT use static polymorphism!!!
    should call construct() method in derived class constructor */
    IndexBackInserter(TermIndex_t& _r) :r(_r) {}
    /* call destruct manually */
    void destruct();

    /* append data into cache
    if cache size is full, may unload cache, update term info etc.
    throw BufferfULL
    [params]
    (Arg) may have different types of input parameters */
    template<typename Arg>
    void append(Arg p);

protected:
    void construct();
    /* actual append implemented by derived class */
    template<typename Arg>
    void _append(Arg p);

    /* throw BufferFull */
    void unload_cache();
    void clear_cache();

    /* unload data other than doc_id, then add a new block meta. */
    void unload_other_cache(uint32_t lastdid, uint16_t didbsize);
    /* should be called inside derived class constructor
    try to load the last block in memory
    if the last block in memory is full, give up loading cache
    else, also resize to erase last block
    this may used for continue appending, until the block in cache is full again */
    void try_load_last_cache(uint32_t pre_did);
};


#include "TermIndex.ipp"
