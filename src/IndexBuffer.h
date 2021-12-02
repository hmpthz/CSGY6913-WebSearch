#pragma once
#include "MemoryBuffer.h"


template<typename Index_t, typename Lexicon_t>
class IndexBufferBase :public BufferBase {
protected:
    using List = std::pmr::list<Index_t>;
    List ilist;

public:
    using Iter = typename List::iterator;
    Lexicon_t lex;

    IndexBufferBase() :ilist(&memcnt) {}
    inline bool is_empty() {
        return ilist.empty();
    }

    void clear(bool clear_Lexicon = true);
};


/* sequentially transfer data from input buffer to output buffer */
namespace Transfer {
    /* transfer a single index object
    input and output buffer may be differnt types, so BackInserter are responsible for conversion
    before transfer, may append an empty index at the back of output buffer
    after transfer, front index and lexicon item of input buffer may be erased */
    template<typename SrcBuf_t, typename DstBuf_t>
    static void front_to_back(SrcBuf_t& srcbuf, DstBuf_t& dstbuf);


    template<typename Index_t, typename Lexicon_t, typename Derived>
    class InputBuffer :public IndexBufferBase<Index_t, Lexicon_t> {
        using B = IndexBufferBase<Index_t, Lexicon_t>;
    protected:
        std::ifstream fin;
        uint64_t fsize;

    public:
        Index_t& front() {
            return B::ilist.front();
        }
        /* pass correct arguments for TermIndex.try_read_blocks method */
        void index_read_blocks(Index_t& index); // <Derived>
        typename Index_t::ForwardIter index_begin(Index_t& index); // <Derived>

        void erase_front();
        /* read lexicon items and blocks of associated index, until buffer is full */
        void read_fill();

        void open_fin(const char* filename);
        void close_fin() {
            if (fin.is_open()) fin.close();
        }
        uint64_t get_fsize() { return fsize; }
    };


    template<typename Index_t, typename Lexicon_t, typename Derived>
    class OutputBuffer :public IndexBufferBase<Index_t, Lexicon_t> {
        using B = IndexBufferBase<Index_t, Lexicon_t>;
    protected:
        std::ofstream fout;
        bool write_did;
        uint32_t min_docs;

    public:
        OutputBuffer() : B() { set_params(); }
        /* [params]
        (write_did) whether var-bytes doc_id will be written
        in some case different types can share the same doc_id index file
        (min_docs) minimum # of docs that a term should contain in order to be written
        many terms only occur in 1 or 2 documents, may filter them to reduce file size */
        void set_params(bool _write_did = true, uint32_t _min_docs = 0) {
            write_did = _write_did; min_docs = _min_docs;
        }
        Index_t& back() {
            return B::ilist.back();
        }
        /* pass correct arguments for TermIndex.write method */
        void index_write(bool end, Index_t& index); // <Derived>
        typename Index_t::BackInserter index_back_inserter(Index_t& index); // <Derived>

        void append_empty_index(const std::string& term);
        /* write data of all index with terminator, and associated lexicon items
        except the last one because it may still have data to be transferred from other buffers */
        void write_except_back();
        void write_all();
        /* clear all data of index and lexicon item, except the last one */
        void erase_except_back();

        void open_fout(const char* filename) {
            fout.open(filename, std::ios::binary);
        }
        void close_fout() {
            if (fout.is_open()) fout.close();
        }
    }; 
}


#include "IndexBuffer.ipp"
