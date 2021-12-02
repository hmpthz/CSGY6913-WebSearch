#pragma once
#include "helper.h"
#include "IndexTerm.h"
#include <list>


using IndexTermList = std::list<IndexTerm>;
using IndexTermIter = IndexTermList::iterator;

/*
    class to be derived
    for simplicity, size of Lexicon is not counted as buffer size.
*/
class IndexBuffer {
protected:
    size_t capacity;
    size_t byte_size;
    IndexTermList ilist;
    std::ifstream fin;
    std::ofstream fout;

public:
    Lexicon lex;

    IndexBuffer();
    inline void set_capacity(size_t sz) {
        capacity = sz;
    }
    inline bool is_full() {
        return byte_size >= capacity;
    }
    inline bool is_empty() {
        return ilist.empty();
    }
    inline size_t size() {
        return ilist.size();
    }

    /* wrapper method of IndexTerm.get_next
    handle IndexEndInMemory Exception, read following blcoks
    if endfile, throw IndexEndInFile */
    Posting get_next(IndexTerm& index);
    /* read_next_block until buffer is full or endfile */
    void read_blocks(IndexTerm& index);

    void open_fin(const char* filename);
    void open_fout(const char* filename);
    inline void close_fin() {
        if (fin.is_open()) fin.close();
    }
    inline void close_fout() {
        if (fout.is_open()) fout.close();
    }
};


/*
    during input or output, because Lexicon is not counted as buffer size,
    Lexicon item will be removed or added along with IndexTerm list item to save memory
*/
class IndexBufferToMerge : public IndexBuffer {
public:
    /* compare the front IndexTerm in each IndexBuffer, get the term with min byte order
    if all IndexBuffers are empty, throw exception */
    static IndexBufferToMerge& min_front(std::vector<IndexBufferToMerge>& bufs);
    /* convert all postings to index, initially IndexBuffer should be empty */
    static void transfer_from_postings(PostingsBuffer& srcbuf, IndexBufferToMerge& dstbuf);
    /* transfer the front IndexTerm in input buffer to the back IndexTerm in output buffer
    if front and back are not the same term, append empty index with the front term in the back of dstbuf.
    if index in input buffer is exhausted, clear and try to read remaining blocks from file 
    if output buffer is full, write all index to file and clear
    after transfer, should erase front IndexTerm in input buffer */
    static void transfer_front_to_back(IndexBufferToMerge& srcbuf, IndexBufferToMerge& dstbuf);
    
    /* methods for input buffer */

    void i_erase_front();
    /* read lexicon items from file, then read blocks of index based on lexicon info until buffer is full */
    void i_read_fill();

    /* methods for output buffer*/

    void o_append_empty_index(const std::string& term);
    /* unload cache of all IndexTerms except the last one,
    because last one may still have data to be transferred.
    then write all bytes, also write associated Lexicon item */
    void o_write_except_back();
    /* unload cache and write include last one */
    void o_write_all();
    /* clear bytes, erase all IndexTerm list items and associated Lexicon item, except the last one*/
    void o_erase_except_back();
    /* clear bytes, erase include last one */
    void erase_all();
};


namespace g::SIZE {
    /* overhead of IndexBuffer */
    constexpr uint32_t INDEXBUF = sizeof(IndexBufferToMerge);
}