#include <filesystem>


template<typename I, typename L>
inline void IndexBufferBase<I, L>::clear(bool clear_Lexicon) {
    ilist.clear();
    if (clear_Lexicon) {
        lex.clear();
    }
}


template<typename SrcBuf_t, typename DstBuf_t>
void Transfer::front_to_back(SrcBuf_t& srcbuf, DstBuf_t& dstbuf) {
    auto& srcindex = srcbuf.front();
    auto& dstindex = dstbuf.back();
    auto srciter = srcbuf.index_begin(srcindex);
    auto dstiter = dstbuf.index_back_inserter(dstindex);

    while (true) {
        try {
            if (srciter.has_next()) {
                dstiter.append(srciter.next());
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


template<typename I, typename L, typename D>
inline void Transfer::InputBuffer<I, L, D>::erase_front() {
    B::lex.erase(B::ilist.front().info);
    B::ilist.pop_front();
}

template<typename I, typename L, typename D>
inline void Transfer::InputBuffer<I, L, D>::read_fill() {
    std::cout << "Transfer::InputBuffer::read_fill\n";

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

    std::cout << "Transfer::InputBuffer::read_fill END\n";
}

template<typename I, typename L, typename D>
inline void Transfer::InputBuffer<I, L, D>::open_fin(const char* filename) {
    fsize = std::filesystem::file_size(filename);
    fin.open(filename, std::ios::binary);
}


template<typename I, typename L, typename D>
inline void Transfer::OutputBuffer<I, L, D>::append_empty_index(const std::string& term) {
    TermInfo_Freq val;
    // create an Lexicon item with initial values.
    auto lex_iter = B::lex.append_term(term, val);
    B::ilist.emplace_back(B::memcnt, lex_iter);
}

template<typename I, typename L, typename D>
inline void Transfer::OutputBuffer<I, L, D>::write_except_back() {
    std::cout << "Transfer::OutputBuffer::write_except_back\n";

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

    if (g::ival(B::ilist.back().info).n_docs >= min_docs) {
        // last one don't write terminator and lexicon
        static_cast<D*>(this)->index_write(false, B::ilist.back());
    }

    std::cout << "Transfer::OutputBuffer::write_except_back END\n";
}

template<typename I, typename L, typename D>
inline void Transfer::OutputBuffer<I, L, D>::write_all() {
    std::cout << "Transfer::OutputBuffer::write_all\n";

    for (auto iter = B::ilist.begin(); iter != B::ilist.end(); iter++) {
        // ignore terms that too few docs contain 
        if (g::ival(iter->info).n_docs >= min_docs) {
            // write terminator as well
            static_cast<D*>(this)->index_write(true, *iter);
            // write Lexicon
            B::lex.write(iter->info);
        }
    }

    std::cout << "Transfer::OutputBuffer::write_all END\n";
}

template<typename I, typename L, typename D>
inline void Transfer::OutputBuffer<I, L, D>::erase_except_back() {
    auto iter = B::ilist.begin();
    auto backiter = std::prev(B::ilist.end());
    while (iter != backiter) {
        // erase
        B::lex.erase(iter->info);
        iter = B::ilist.erase(iter);
    }
    // the last one still needs to clear data
    B::ilist.front().update_start_did();
    B::ilist.front().clear();
}
