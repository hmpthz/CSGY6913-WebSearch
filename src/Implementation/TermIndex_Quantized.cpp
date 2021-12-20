#include "TermIndex_Quantized.h"


_Index::_Quantized::_Quantized(MemoryCounter& memcnt, Lexicon_Score::Iter& iter) :
    Base(memcnt, iter), bits(&memcnt) {

}

void _Index::_Quantized::read_next_block(std::ifstream& fin, std::ifstream& fin2) {
    std::istreambuf_iterator<char> ifiter(fin);
    std::istreambuf_iterator<char> ifiter2(fin2);

    uint32_t lastdid;
    uint16_t didbsize, scorebsize;
    fin2.read((char*)&lastdid, sizeof(lastdid));
    fin2.read((char*)&didbsize, sizeof(didbsize));
    fin2.read((char*)&scorebsize, sizeof(scorebsize));

    std::copy_n(ifiter, didbsize, std::back_inserter(bytes));
    ifiter++; // IMPORTANT!!
    std::copy_n(ifiter2, scorebsize, std::back_inserter(bits));
    ifiter2++; // IMPORTANT!!

    blocks_meta.emplace_back(lastdid, didbsize, scorebsize);
}

void _Index::_Quantized::write(bool end, bool write_did, std::ofstream& fout, std::ofstream& fout2) {
    if (write_did) {
        // set start offset
        if (g::ival(info).start_off2 == -1) {
            g::ival(info).start_off = fout.tellp();
            g::ival(info).start_off2 = fout2.tellp();
        }

        std::ostreambuf_iterator<char> ofiter(fout);
        std::ostreambuf_iterator<char> ofiter2(fout2);

        auto bytes_iter = bytes.begin();
        auto bits_iter = bits.begin();
        // write all blocks
        for (auto& meta : blocks_meta) {
            fout2.write((char*)&meta.last_did, sizeof(meta.last_did));
            fout2.write((char*)&meta.did_bsize, sizeof(meta.did_bsize));
            fout2.write((char*)&meta.score_bsize, sizeof(meta.score_bsize));

            std::copy_n(bytes_iter, meta.did_bsize, ofiter);
            // move cursor
            bytes_iter += meta.did_bsize;
            std::copy_n(bits_iter, meta.score_bsize, ofiter2);
            // move cursor
            bits_iter += meta.score_bsize;
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

        auto bits_iter = bits.begin();
        for (auto& meta : blocks_meta) {
            fout2.write((char*)&meta.last_did, sizeof(meta.last_did));
            fout2.write((char*)&meta.did_bsize, sizeof(meta.did_bsize));
            fout2.write((char*)&meta.score_bsize, sizeof(meta.score_bsize));

            std::copy_n(bits_iter, meta.score_bsize, ofiter2);
            bits_iter += meta.score_bsize;
        }

        if (end) {
            g::ival(info).n_blocks = fblock;
            ofiter2 = '\0';
        }
    }
}

void _Index::_Quantized::clear() {
    bytes.clear();
    blocks_meta.clear();
    bits.clear();
}
