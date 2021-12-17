#pragma once
#include "MemoryBuffer.h"
#include <utility>


template<typename Index_t, typename Lexicon_t>
class _IndexBufferBase :public BufferBase {
protected:
    using List = std::pmr::list<Index_t>;
    List ilist;

public:
    using Iter = typename List::iterator;
    Lexicon_t lex;

    _IndexBufferBase() :ilist(&memcnt) {}
    inline size_t size() {
        return ilist.size();
    }
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
    after transfer, front index and lexicon item of input buffer may be erased
    [params]
    (N_DOCS) see "IndexBackInserter.append" */
    template<bool N_DOCS = false, typename SrcBuf_t, typename DstBuf_t>
    static void front_to_back(SrcBuf_t& srcbuf, DstBuf_t& dstbuf);

    /* transfer all data from input buffer to output buffer
    before the transfer, both buffers should be empty and files are opened
    after the transfer, both buffers will be empty and files are closed
    [params]
    (N_DOCS) see "IndexBackInserter.append"
        if it's false, when output buffer append a new index, copy "Lexicon::Val.n_docs" as well
        index can get the true n_docs directly instead of incrementing by 1 for each append.
        so that BM25 score can compute the correct result while appending */
    template<bool N_DOCS = false, typename SrcBuf_t, typename DstBuf_t>
    static void all_data(SrcBuf_t& srcbuf, DstBuf_t& dstbuf);
}


namespace InputBuffer {
    template<typename Index_t, typename Lexicon_t, typename Derived>
    class _Base :public _IndexBufferBase<Index_t, Lexicon_t> {
    protected:
        std::ifstream fin;

    public:
        using B = _IndexBufferBase<Index_t, Lexicon_t>;
        using TermIndex = typename Index_t;
        using IndexForwardIter = typename Index_t::ForwardIter;
        using Type = decltype(std::declval<IndexForwardIter>().next());
        template<typename> friend class SequentialIter;

        inline TermIndex& front() {
            return B::ilist.front();
        }
        /* pass correct arguments for TermIndex.try_read_blocks method */
        void index_read_blocks(TermIndex& index); // <Derived>
        IndexForwardIter index_begin(TermIndex& index); // <Derived>

        void erase_front();
        /* read lexicon items and blocks of associated index, until buffer is full */
        void read_fill();

        void open_fin(const char* filename) {
            fin.open(filename, std::ios::binary);
        }
        void close_fin() {
            if (fin.is_open()) fin.close();
        }
        void reset_fpos(); // <Derived>
    };


    /* iterator to read all index in buffer instead of just one
    constructor must be called after read_fill */
    template<typename Buffer_t>
    class SequentialIter {
    protected:
        Buffer_t& r;
        typename Buffer_t::TermIndex* index_p;
        typename Buffer_t::IndexForwardIter iter;

    public:
        SequentialIter(Buffer_t& _r) : r(_r), index_p(&_r.front()), iter(_r.index_begin(_r.front())) {}
        bool has_next();
        typename Buffer_t::Type next();
    };
}


namespace OutputBuffer {
    template<typename Index_t, typename Lexicon_t, typename Derived>
    class _Base :public _IndexBufferBase<Index_t, Lexicon_t> {
    protected:
        std::ofstream fout;

    public:
        using B = _IndexBufferBase<Index_t, Lexicon_t>;
        using TermIndex = typename Index_t;
        using IndexBackInserter = typename Index_t::BackInserter;
        /* whether var-bytes doc_id will be written
        in some case different types can share the same doc_id index file */
        bool write_did;
        /* minimum # of docs that a term should contain in order to be written
        many terms only occur in 1 or 2 documents, may filter them to reduce file size */
        uint32_t min_docs;
        _Base() :B(), write_did(true), min_docs(0) {}
        inline TermIndex& back() {
            return B::ilist.back();
        }
        /* pass correct arguments for TermIndex.write method */
        void index_write(bool end, TermIndex& index); // <Derived>
        IndexBackInserter index_back_inserter(TermIndex& index); // <Derived>

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
