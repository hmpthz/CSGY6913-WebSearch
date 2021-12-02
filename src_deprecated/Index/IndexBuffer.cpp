#include "IndexBuffer.h"
#include <filesystem>
// uncomment to disable assert()
// #define NDEBUG
#include <cassert>
namespace fs = std::filesystem;


IndexBuffer::IndexBuffer() {
    byte_size = g::SIZE::INDEXBUF;
}

Posting IndexBuffer::get_next(IndexTerm& index) {
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
            read_blocks(index);
        }
    }
    // get again, this time don't need try-catch because we've read following blocks
    return index.get_next();
}

void IndexBuffer::read_blocks(IndexTerm& index) {
    while (!index.is_endfile() && !is_full()) {
        byte_size += index.read_next_block(fin);
    }
}

void IndexBuffer::open_fin(const char* filename) {
    fin.open(fs::path(filename), ios::binary);
}

void IndexBuffer::open_fout(const char* filename) {
    fout.open(fs::path(filename), ios::binary);
}


IndexBufferToMerge& IndexBufferToMerge::min_front(std::vector<IndexBufferToMerge>& bufs) {
    std::string min_term = { (char)0xff,'\0' }; // every string is less than this
    int min_i = -1;
    for (int i = 0; i < bufs.size(); i++) {
        if (bufs[i].is_empty()) continue; // not break
        auto& term = g::ikey(bufs[i].ilist.front().info);
        if (term < min_term) {
            min_term = term;
            min_i = i;
        }
    }
    if (min_i != -1) {
        return bufs[min_i];
    }
    else {
        throw g::Exception::AllInputBuffersExhausted;
    }
}

void IndexBufferToMerge::transfer_from_postings(PostingsBuffer& srcbuf, IndexBufferToMerge& dstbuf) {
    for (auto& [term, posting_vec] : srcbuf.terms) {
        dstbuf.o_append_empty_index(term);
        auto& dstindex = dstbuf.ilist.back();
        for (auto& p : posting_vec) {
            dstbuf.byte_size += dstindex.append(p);
            // handle buffer full
            if (dstbuf.is_full()) {
                dstbuf.o_write_except_back();
                dstbuf.o_erase_except_back();
            }
        }
    }
}

void IndexBufferToMerge::transfer_front_to_back(IndexBufferToMerge& srcbuf, IndexBufferToMerge& dstbuf) {
    auto& srcindex = srcbuf.ilist.front();
    // front and back are not the same term
    if (dstbuf.is_empty() || g::ikey(srcindex.info) != g::ikey(dstbuf.ilist.back().info)) {
        dstbuf.o_append_empty_index(g::ikey(srcindex.info));
    }
    auto& dstindex = dstbuf.ilist.back();

    try {
        while (true) {
            dstbuf.byte_size += dstindex.append(srcbuf.get_next(srcindex));
            if (dstbuf.is_full()) {
                dstbuf.o_write_except_back();
                dstbuf.o_erase_except_back();
            }
        }
    }
    // IndexEndInFile Exception
    catch (g::Exception) { }
    // transfer over
}

void IndexBufferToMerge::i_erase_front() {
    byte_size -= ilist.front().byte_size();
    lex.erase(ilist.front().info);
    ilist.pop_front();
}

void IndexBufferToMerge::i_read_fill() {
    if (!fin.is_open()) return;
    std::cout << "IndexBufferToMerge::i_read_fill\n";

    try {
        while (!is_full()) {
            // read an Lexicon item and append IndexTerm
            ilist.emplace_back(lex.read_next());
            byte_size += g::SIZE::INDEXTERM;
            read_blocks(ilist.back());
        }
    }
    // LexiconEndInFile Exception
    catch (g::Exception) { }
    std::cout << "IndexBufferToMerge::i_read_fill END\n";
}

void IndexBufferToMerge::o_append_empty_index(const std::string& term) {
    // create an Lexicon item with uninitialized start offset and end offset
    ilist.emplace_back(lex.append_term(term, -1, -1, 0));
    // need to count IndexTerm overhead
    byte_size += g::SIZE::INDEXTERM;
}

void IndexBufferToMerge::o_write_except_back() {
    if (!fout.is_open()) return;
    std::cout << "IndexBufferToMerge::o_write_except_back\n";
    // unload cache and write except last one
    if (ilist.size() > 1) {
        auto backiter = std::prev(ilist.end());
        for (auto iter = ilist.begin(); iter != backiter; iter++) {
            byte_size += iter->unload_cache();
            iter->clear_cache();
            // write terminator as well
            iter->write_bytes(fout, true);

            // write Lexicon
            lex.write(iter->info);
        }
    }

    // last one don't unload cache, write terminator and lexicon
    ilist.back().write_bytes(fout, false);
    std::cout << "IndexBufferToMerge::o_write_except_back END\n";
}

void IndexBufferToMerge::o_write_all() {
    if (!fout.is_open()) return;
    std::cout << "IndexBufferToMerge::o_write_all\n";
    for (auto iter = ilist.begin(); iter != ilist.end(); iter++) {
        byte_size += iter->unload_cache();
        iter->clear_cache();
        iter->write_bytes(fout, true);
        lex.write(iter->info);
    }
    std::cout << "IndexBufferToMerge::o_write_all END\n";
}

void IndexBufferToMerge::o_erase_except_back() {
    // only one IndexTerm, don't erase anything
    if (ilist.size() == 1) {
        byte_size -= ilist.front().clear_bytes();
        return;
    }

    auto iter = ilist.begin();
    auto backiter = std::prev(ilist.end());
    while (iter != backiter) {
        // decrease buffer size
        byte_size -= iter->byte_size();
        // erase
        lex.erase(iter->info);
        iter = ilist.erase(iter);
    }
    // the last one still needs to clear bytes
    byte_size -= ilist.front().clear_bytes();
}

void IndexBufferToMerge::erase_all() {
    auto iter = ilist.begin();
    while (iter != ilist.end()) {
        byte_size -= iter->byte_size();
        lex.erase(iter->info);
        iter = ilist.erase(iter);
    }
    assert(byte_size == g::SIZE::INDEXBUF);
}
