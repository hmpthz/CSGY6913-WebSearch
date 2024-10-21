#include "Lexicon.h"
#include <filesystem>
namespace fs = std::filesystem;


LexiconIter Lexicon::get_iter(const std::string& term) {
    auto iter = terms.find(term);
    if (iter != terms.end()) {
        return iter;
    }
    else {
        throw g::Exception::TermNotFound;
    }
}

LexiconIter Lexicon::add_term(const std::string& term, uint64_t start, uint64_t end, uint32_t n) {
    auto [iter, is_new] = terms.try_emplace(term, start, end, n);
    return iter;
}

LexiconIter Lexicon::append_term(const std::string& term, uint64_t start, uint64_t end, uint32_t n) {
    if (terms.size() > 0) {
        // use hint
        return terms.try_emplace(std::prev(terms.end()), term, start, end, n);
    }
    else return add_term(term, start, end, n);
}

void Lexicon::open_fin(const char* filename) {
    fin.open(fs::path(filename), ios::binary);
}

void Lexicon::open_fout(const char* filename) {
    fout.open(fs::path(filename), ios::binary);
}

void Lexicon::read_all() {
    try {
        while (true) {
            read_next();
        }
    }
    // LexiconEndInFile Exception
    catch (g::Exception) {}
}

void Lexicon::clear() {
    terms.clear();
}

#ifdef TEXT_FILE
/* I/O File in text format */

LexiconIter Lexicon::read_next() {
    std::string term;
    uint64_t start, end;
    uint32_t n;
    fin >> term >> start >> end >> n;
    if (fin.tellg() != -1) {
        return append_term(term, start, end, n);
    }
    else {
        throw g::Exception::LexiconEndInFile;
    }
}

void Lexicon::write(LexiconIter iter) {
    fout << g::ikey(iter) << ' ';
    auto& val = g::ival(iter);
    fout << val.start_offset << ' ' << val.end_offset << ' '
        << val.n_docs << '\n';
}

#else
/* I/O File in binary format */

LexiconIter Lexicon::read_next() {
    std::istreambuf_iterator<char> ifiter(fin);
    std::istreambuf_iterator<char> ifiter_end;

    std::string term;
    g::copy_until_zero(ifiter, ifiter_end, std::back_inserter(term));

    uint64_t start, end;
    uint32_t n;
    fin.read((char*)&start, sizeof(start));
    fin.read((char*)&end, sizeof(end));
    fin.read((char*)&n, sizeof(n));

    if (fin.tellg() != -1) {
        return append_term(term, start, end, n);
    }
    else {
        throw g::Exception::LexiconEndInFile;
    }
}

void Lexicon::write(LexiconIter iter) {
    fout << g::ikey(iter) << '\0';
    auto& val = g::ival(iter);
    fout.write((char*)&val.start_offset, sizeof(val.start_offset));
    fout.write((char*)&val.end_offset, sizeof(val.end_offset));
    fout.write((char*)&val.n_docs, sizeof(val.n_docs));
}

#endif


PostingsBuffer::PostingsBuffer() {
    byte_size = g::SIZE::POSTINGBUF;
}

void PostingsBuffer::add_postings(uint32_t doc_id, const PostingsMap& postings) {
    for (auto& [term, frequency] : postings) {

        auto [iter, is_new] = terms.try_emplace(term, PostingsVector());
        if (is_new) {
            byte_size += 1.2f * g::SIZE::POSTINGBUF_MAPITEM;
        }
        auto& posting_vec = g::ival(iter);
        posting_vec.emplace_back(doc_id, frequency);
        byte_size += 1.2f * g::SIZE::POSTING;
    }
}

void PostingsBuffer::clear() {
    byte_size = g::SIZE::POSTINGBUF;
    terms.clear();
}
