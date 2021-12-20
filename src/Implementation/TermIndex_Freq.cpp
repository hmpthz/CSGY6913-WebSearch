#include "TermIndex_Freq.h"


void _Index::Freq::clear() {
    bytes.clear();
    blocks_meta.clear();
}


#ifndef TEXT_FILE
/* I/O File in binary format */

Lexicon_Freq::Iter Lexicon_Freq::read_next() {
    std::istreambuf_iterator<char> ifiter(fin);
    std::istreambuf_iterator<char> ifiter_end;

    std::string term;
    g::copy_until_zero(ifiter, ifiter_end, std::back_inserter(term));

    TermInfo_Freq val;
    fin.read((char*)&val.start_off, sizeof(val.start_off));
    fin.read((char*)&val.n_blocks, sizeof(val.n_blocks));
    fin.read((char*)&val.n_docs, sizeof(val.n_docs));

    if (!fin.eof()) {
        return append_term(term, val);
    }
    else {
        throw g::Exception::EndInFile;
    }
}

void Lexicon_Freq::write(Iter iter) {
    fout << g::ikey(iter) << '\0';
    auto& val = g::ival(iter);
    fout.write((char*)&val.start_off, sizeof(val.start_off));
    fout.write((char*)&val.n_blocks, sizeof(val.n_blocks));
    fout.write((char*)&val.n_docs, sizeof(val.n_docs));
}


void _Index::Freq::read_next_block(std::ifstream& fin) {
    std::istreambuf_iterator<char> ifiter(fin);

    uint32_t lastdid;
    uint16_t didbsize, freqbsize;
    fin.read((char*)&lastdid, sizeof(lastdid));
    fin.read((char*)&didbsize, sizeof(didbsize));
    fin.read((char*)&freqbsize, sizeof(freqbsize));

    std::copy_n(ifiter, didbsize, std::back_inserter(bytes));
    ifiter++; // IMPORTANT!!
    std::copy_n(ifiter, freqbsize, std::back_inserter(bytes));
    ifiter++; // IMPORTANT!!

    blocks_meta.emplace_back(lastdid, didbsize, freqbsize);
}

void _Index::Freq::write(bool end, bool write_did, std::ofstream& fout) {
    // set start offset
    if (g::ival(info).start_off == -1) {
        g::ival(info).start_off = fout.tellp();
    }

    std::ostreambuf_iterator<char> ofiter(fout);

    auto bytes_iter = bytes.begin();
    // write all blocks
    for (auto& meta : blocks_meta) {
        fout.write((char*)&meta.last_did, sizeof(meta.last_did));
        fout.write((char*)&meta.did_bsize, sizeof(meta.did_bsize));
        fout.write((char*)&meta.freq_bsize, sizeof(meta.freq_bsize));

        std::copy_n(bytes_iter, meta.did_bsize, ofiter);
        // move cursor
        bytes_iter += meta.did_bsize;
        std::copy_n(bytes_iter, meta.freq_bsize, ofiter);
        // move cursor
        bytes_iter += meta.freq_bsize;
    }

    if (end) {
        g::ival(info).n_blocks = fblock;
        ofiter = '\0';
    }
}

#else
/* I/O File in text format */

Lexicon_Freq::Iter Lexicon_Freq::read_next() {
    std::string term;
    TermInfo_Freq val;
    fin >> term >> val.start_off >> val.n_blocks >> val.n_docs;
    if (!fin.eof()) {
        return append_term(term, val);
    }
    else {
        throw g::Exception::EndInFile;
    }
}

void Lexicon_Freq::write(Iter iter) {
    fout << g::ikey(iter) << ' ';
    auto& val = g::ival(iter);
    fout << val.start_off << ' ' << val.n_blocks << ' '
        << val.n_docs << '\n';
}


void _Index::Freq::read_next_block(std::ifstream& fin) {
    // in debug text format, we read uncompressed ascii numbers from file
    // so they need to be differenced and compress again into memory
    // use extra static cache, don't mess with member's cache
    thread_local static vector_u32 tmp_did;
    thread_local static vector_u32 tmp_freq;

    // reading...
    uint32_t lastdid;
    uint32_t n_postings;
    fin >> lastdid >> n_postings;

    uint32_t tmp;
    for (uint32_t i = 0; i < n_postings; i++) {
        fin >> tmp;
        tmp_did.emplace_back(tmp);
    }
    for (uint32_t i = 0; i < n_postings; i++) {
        fin >> tmp;
        tmp_freq.emplace_back(tmp);
    }

    // unload tmp cache
    uint32_t pre_did;
    if (block_size() > 0) {
        pre_did = blocks_meta.back().last_did;
    }
    else pre_did = start_did;
    VarBytes::difference(pre_did, tmp_did);

    uint16_t didbsize = bytes.compress(tmp_did);
    uint16_t freqbsize = bytes.compress(tmp_freq);
    // clear tmp cache
    tmp_did.clear();
    tmp_freq.clear();

    blocks_meta.emplace_back(lastdid, didbsize, freqbsize);
}

void _Index::Freq::write(bool end, bool write_did, std::ofstream& fout) {
    // in debug text format, we write uncompressed ascii numbers to file
    // so they need to be decompress and undifference again into file
    // use extra static cache, don't mess with member's cache
    thread_local static vector_u32 tmp_did;
    thread_local static vector_u32 tmp_freq;

    // set start offset
    if (g::ival(info).start_off == -1) {
        g::ival(info).start_off = fout.tellp();
    }

    size_t cur_byte = 0;
    // write all blocks
    for (uint32_t cur_block = 0; cur_block < block_size(); cur_block++) {
        // load tmp cache
        auto& meta = blocks_meta[cur_block];
        uint32_t pre_did;
        if (cur_block > 0) {
            pre_did = blocks_meta[cur_block - 1].last_did;
        }
        else pre_did = start_did;

        bytes.decompress(cur_byte, meta.did_bsize, tmp_did);
        VarBytes::undifference(pre_did, tmp_did);
        // move cursor
        cur_byte += meta.did_bsize;

        bytes.decompress(cur_byte, meta.freq_bsize, tmp_freq);
        // move cursor
        cur_byte += meta.freq_bsize;

        // writing...
        fout << meta.last_did << ' ' << tmp_did.size() << ' ';
        for (auto i : tmp_did) {
            fout << i << ' ';
        }
        for (auto i : tmp_freq) {
            fout << i << ' ';
        }
        fout << '\n';
        // clear tmp cache
        tmp_did.clear();
        tmp_freq.clear();
    }


    if (end) {
        g::ival(info).n_blocks = fblock;
        fout << '#' << '\n';
    }
}

#endif // !TEXT_FILE
