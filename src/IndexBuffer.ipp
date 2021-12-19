#include "IndexBuffer.h"


template<typename I, typename L>
inline void _IndexBufferBase<I, L>::clear(bool clear_Lexicon) {
    ilist.clear();
    if (clear_Lexicon) {
        lex.clear();
    }
}


template<bool N_DOCS, typename SrcBuf_t, typename DstBuf_t>
void Transfer::front_to_back(SrcBuf_t& srcbuf, DstBuf_t& dstbuf) {
    auto& srcindex = srcbuf.front();
    auto& dstindex = dstbuf.back();
    auto srciter = srcbuf.index_begin(srcindex);
    auto dstiter = dstbuf.index_back_inserter(dstindex);

    while (true) {
        try {
            if (srciter.has_next()) {
                dstiter.append<N_DOCS>(srciter.next());
            }
            else if (srcindex.is_endfile()) {
                break;
            }
            else {
                srcbuf.index_read_blocks(srcindex);
            }
        }
        // dst BufferFull
        catch (g::Exception) {
            // write data then clear
            dstbuf.write_except_back();
            dstbuf.erase_except_back();
        }
    }

    try {
        dstiter.destruct();
    }
    catch (g::Exception) {
        dstbuf.write_except_back();
        dstbuf.erase_except_back();
    }
}

template<bool N_DOCS, typename SrcBuf_t, typename DstBuf_t>
void Transfer::all_data(SrcBuf_t& srcbuf, DstBuf_t& dstbuf) {
    srcbuf.read_fill();

    while (true) {
        auto info = srcbuf.front().info;
        dstbuf.append_empty_index(g::ikey(info));
        // copy "Lexicon::Val.n_docs" as well
        if constexpr (!N_DOCS) {
            g::ival(dstbuf.back().info).n_docs = g::ival(info).n_docs;
        }
        // if don't have to write doc_id file, copy "Lexicon::Val.start_off" as well
        if (!dstbuf.write_did) {
            g::ival(dstbuf.back().info).start_off = g::ival(info).start_off;
        }

        front_to_back<N_DOCS>(srcbuf, dstbuf);

        srcbuf.erase_front();
        if (srcbuf.is_empty()) {
            srcbuf.read_fill();
            // still empty, input buffer has been exhausted
            if (srcbuf.is_empty()) {
                break;
            }
        }
    }

    dstbuf.write_all();
    dstbuf.clear(true);

    srcbuf.close_fin();
    srcbuf.lex.close_fin();
    dstbuf.close_fout();
    dstbuf.lex.close_fout();
}


template<typename Buffer_t>
inline bool InputBuffer::SequentialIter<Buffer_t>::has_next() {
    while (true) {
        if (iter.has_next()) {
            return true;
        }
        // this index is exhausted, move to next index
        else if (index_p->is_endfile()) {
            r.erase_front();
            if (r.is_empty()) {
                r.read_fill();
                // still empty, input buffer has been exhausted
                if (r.is_empty()) {
                    return false;
                }
            }

            index_p = &r.front();
            iter = r.index_begin(*index_p);
        }
        // need to read more blocks
        else {
            r.index_read_blocks(*index_p);
        }
    }
}

template<typename Buffer_t>
inline typename Buffer_t::Type InputBuffer::SequentialIter<Buffer_t>::next() {
    return iter.next();
}


template<typename I, typename L, typename D>
inline void InputBuffer::_Base<I, L, D>::erase_front() {
    B::lex.erase(B::ilist.front().info);
    B::ilist.pop_front();
}

template<typename I, typename L, typename D>
inline void InputBuffer::_Base<I, L, D>::read_fill() {
    try {
        while (!B::is_full()) {
            auto lex_iter = B::lex.read_next();
            // why PMR container's emplace_back cannot pass an rvalue????????
            B::ilist.emplace_back(B::memcnt, lex_iter);
            //B::ilist.emplace_back(B::memcnt, B::lex.read_next());
            static_cast<D*>(this)->index_read_blocks(B::ilist.back());
        }
    }
    // Lexicon EndInFile Exception
    catch (g::Exception) {}

    std::cout << "InputBuffer::read_fill " << B::ilist.size() << " terms\n";
}


template<typename I, typename Lexicon_t, typename D>
inline void OutputBuffer::_Base<I, Lexicon_t, D>::append_empty_index(const std::string& term) {
    typename Lexicon_t::Val val;
    // create an Lexicon item with initial values.
    auto lex_iter = B::lex.append_term(term, val);
    B::ilist.emplace_back(B::memcnt, lex_iter);
}

template<typename I, typename L, typename D>
inline void OutputBuffer::_Base<I, L, D>::write_except_back() {
    // write except last one
    if (B::ilist.size() > 1) {
        auto backiter = std::prev(B::ilist.end());
        for (auto iter = B::ilist.begin(); iter != backiter; iter++) {
            // ignore terms that too few docs contain 
            if (g::ival(iter->info).n_docs >= min_docs) {
                // write terminator as well
                static_cast<D*>(this)->index_write(true, *iter);
                // write Lexicon
                B::lex.write(iter->info);
            }
        }
    }
    // IMPORTANT!! don't write the last index
    //if (g::ival(B::ilist.back().info).n_docs >= min_docs) {
    //    // last one don't write terminator and lexicon
    //    static_cast<D*>(this)->index_write(false, B::ilist.back());
    //}

    std::cout << "OutputBuffer::write_except_back " << B::ilist.size() - 1 << " terms\n";
}

template<typename I, typename L, typename D>
inline void OutputBuffer::_Base<I, L, D>::write_all() {
    for (auto iter = B::ilist.begin(); iter != B::ilist.end(); iter++) {
        // ignore terms that too few docs contain 
        if (g::ival(iter->info).n_docs >= min_docs) {
            // write terminator as well
            static_cast<D*>(this)->index_write(true, *iter);
            // write Lexicon
            B::lex.write(iter->info);
        }
    }

    std::cout << "OutputBuffer::write_all " << B::ilist.size() << " terms\n";
}

template<typename I, typename L, typename D>
inline void OutputBuffer::_Base<I, L, D>::erase_except_back() {
    auto iter = B::ilist.begin();
    auto backiter = std::prev(B::ilist.end());
    while (iter != backiter) {
        // erase
        B::lex.erase(iter->info);
        iter = B::ilist.erase(iter);
    }
    // IMPORTANT!! don't clear the last index
    //if (g::ival(B::ilist.front().info).n_docs >= min_docs) {
    //    B::ilist.front().update_start_did();
    //    B::ilist.front().clear();
    //}
}
