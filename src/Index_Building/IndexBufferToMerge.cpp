#include "IndexBufferToMerge.h"


int InputBufferToMerge::min_front(std::vector<InputBufferToMerge>& bufs) {
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
    return min_i;
}

void OutputBufferToMerge::transfer_from_postings(PostingsBuffer& srcbuf) {
    for (auto& [term, posting_vec] : srcbuf.terms) {
        append_empty_index(term);
        auto dstiter = ilist.back().back_inserter();
        for (auto& p : posting_vec) {
            try {
                dstiter.append<true>(p);
            }
            // BufferFull Exception
            catch (g::Exception) {
                write_except_back();
                erase_except_back();
            }
        }
        try {
            dstiter.destruct();
        }
        catch (g::Exception) {
            write_except_back();
            erase_except_back();
        }
    }
}
