#include "TermIndex_Score.h"


DocTable* BM25::docs = NULL;


void _TermIndex_Score::clear() {
    bytes.clear();
    blocks_meta.clear();
    scores.clear();
}


Lexicon_Score::Iter Lexicon_Score::read_next() {
    std::istreambuf_iterator<char> ifiter(fin);
    std::istreambuf_iterator<char> ifiter_end;

    std::string term;
    g::copy_until_zero(ifiter, ifiter_end, std::back_inserter(term));

    TermInfo_Score val;
    fin.read((char*)&val.start_off, sizeof(val.start_off));
    fin.read((char*)&val.start_off2, sizeof(val.start_off2));
    fin.read((char*)&val.n_blocks, sizeof(val.n_blocks));
    fin.read((char*)&val.n_docs, sizeof(val.n_docs));

    if (!fin.eof()) {
        return append_term(term, val);
    }
    else {
        throw g::Exception::EndInFile;
    }
}

void Lexicon_Score::write(Iter iter) {
    fout << g::ikey(iter) << '\0';
    auto& val = g::ival(iter);
    fout.write((char*)&val.start_off, sizeof(val.start_off));
    fout.write((char*)&val.start_off2, sizeof(val.start_off2));
    fout.write((char*)&val.n_blocks, sizeof(val.n_blocks));
    fout.write((char*)&val.n_docs, sizeof(val.n_docs));
}


_TermIndex_Score::_TermIndex_Score(MemoryCounter& memcnt, Lexicon_Score::Iter& iter) :
    Base(memcnt, iter), scores(&memcnt) {
}

void _TermIndex_Score::read_next_block(std::ifstream& fin, std::ifstream& fin2) {
    std::istreambuf_iterator<char> ifiter(fin);
    std::istreambuf_iterator<char> ifiter2(fin2);

    uint32_t lastdid;
    uint16_t didbsize, scorebsize;
    fin2.read((char*)&lastdid, sizeof(lastdid));
    fin2.read((char*)&didbsize, sizeof(didbsize));
    fin2.read((char*)&scorebsize, sizeof(scorebsize));

    std::copy_n(ifiter, didbsize, std::back_inserter(bytes));
    ifiter++; // IMPORTANT!!

    auto prev_size = scores.size();
    // resize to append
    scores.resize(prev_size + (scorebsize / sizeof(float)));
    // get the end of pointer to be appended
    // treat float vector as bytes!
    char* to_append = (char*)(scores.data() + prev_size);
    std::copy_n(ifiter2, scorebsize, to_append);
    ifiter2++; // IMPORTANT!!

    blocks_meta.emplace_back(lastdid, didbsize, scorebsize);
}

void _TermIndex_Score::write(bool end, bool write_did, std::ofstream& fout, std::ofstream& fout2) {
    if (write_did) {
        // set start offset
        if (g::ival(info).start_off2 == -1) {
            g::ival(info).start_off = fout.tellp();
            g::ival(info).start_off2 = fout2.tellp();
        }

        std::ostreambuf_iterator<char> ofiter(fout);
        std::ostreambuf_iterator<char> ofiter2(fout2);

        auto bytes_iter = bytes.begin();
        // treat float vector as bytes!
        char* scores_pointer = (char*)scores.data();
        // write all blocks
        for (auto& meta : blocks_meta) {
            fout2.write((char*)&meta.last_did, sizeof(meta.last_did));
            fout2.write((char*)&meta.did_bsize, sizeof(meta.did_bsize));
            fout2.write((char*)&meta.score_bsize, sizeof(meta.score_bsize));

            std::copy_n(bytes_iter, meta.did_bsize, ofiter);
            // move cursor
            bytes_iter += meta.did_bsize;
            std::copy_n(scores_pointer, meta.score_bsize, ofiter2);
            // move cursor
            scores_pointer += meta.score_bsize;
        }

        if (end) {
            g::ival(info).n_blocks = fblock;
            ofiter = '\0';
            ofiter2 = '\0';
        }
    }
    else {
        if (g::ival(info).start_off2 == -1) {
            g::ival(info).start_off2 = fout2.tellp();
        }

        std::ostreambuf_iterator<char> ofiter2(fout2);

        char* scores_pointer = (char*)scores.data();
        for (auto& meta : blocks_meta) {
            fout2.write((char*)&meta.last_did, sizeof(meta.last_did));
            fout2.write((char*)&meta.did_bsize, sizeof(meta.did_bsize));
            fout2.write((char*)&meta.score_bsize, sizeof(meta.score_bsize));

            std::copy_n(scores_pointer, meta.score_bsize, ofiter2);
            scores_pointer += meta.score_bsize;
        }

        if (end) {
            g::ival(info).n_blocks = fblock;
            ofiter2 = '\0';
        }
    }
}
