#pragma once
#include "helper.h"
#include <map>


struct LexiconValue {
    uint64_t start_offset; /* start offset in index file of a term */
    uint64_t end_offset; /* end offset in index file of a term */
    uint32_t n_docs; /* # of docs containing the term */
    LexiconValue(uint64_t a, uint64_t b, uint32_t c) :start_offset(a), end_offset(b), n_docs(c) {}
};
using LexiconMap = std::map<std::string, LexiconValue>;
using LexiconIter = LexiconMap::iterator;


/*
    use map to maintain alphabetical order of terms.
    iterator of the map would be stored in IndexTerm, so it can get the needed info.
    each Lexicon is associated to an IndexBuffer.
    during the merging, Lexicon only keeps terms currently in IndexBuffer.
*/
class Lexicon {
protected:
    LexiconMap terms;
    std::ifstream fin;
    std::ofstream fout;

public:
    inline size_t size() {
        return terms.size();
    }
    /* throw TermNotFound */
    LexiconIter get_iter(const std::string& term);

    LexiconIter add_term(const std::string& term, uint64_t start, uint64_t end, uint32_t n);
    /* if guarantee terms are in ascending order, append after the last item.
    get rid of search time, improve performance of map insertion */
    LexiconIter append_term(const std::string& term, uint64_t start, uint64_t end, uint32_t n);
    inline void erase(LexiconIter iter) {
        terms.erase(iter);
    }

    /*
        text format:
        <term> (space) <start offset> (space) <end offset> (space) <n_docs> (\n)
        binary format:
        <str-term> <1-\0> <8-start offset> <8-end offset> <4-n_docs>
        no delimeter between lexicon items!
    */

    void open_fin(const char* filename);
    void open_fout(const char* filename);
    /* read next term from Lexicon file based on current fin pointer
    [return]
    iterator of the loaded term. end iterator if the file is ended */
    LexiconIter read_next();
    /* write term to disk, append file based on current fout pointer*/
    void write(LexiconIter iter);
    void read_all();
    void clear();
    inline void close_fin() {
        if (fin.is_open()) fin.close();
    }
    inline void close_fout() {
        if (fout.is_open()) fout.close();
    }
};


struct Posting {
    uint32_t doc_id;
    uint32_t frequency;
    Posting(uint32_t a, uint32_t b) :doc_id(a), frequency(b) {}
};
using PostingsVector = std::vector<Posting>;
/* postings from a single document, so each term only has frequency */
using PostingsMap = std::map<std::string, uint32_t>;
using PostingsBufferMap = std::map<std::string, PostingsVector>;
class IndexBufferToMerge;

/*
    during the initial file parsing step, uncompressed postings are added.
    when buffer is full, transfer data to IndexBuffer
*/
class PostingsBuffer {
protected:
    size_t capacity;
    size_t byte_size;
    PostingsBufferMap terms;

public:
    PostingsBuffer();
    /* used for transfer */
    friend IndexBufferToMerge;
    inline void set_capacity(size_t sz) {
        capacity = sz;
    }
    inline bool is_full() {
        return byte_size >= capacity;
    }
    inline bool is_empty() {
        return terms.size() == 0;
    }

    void add_postings(uint32_t doc_id, const PostingsMap& postings);

    void clear();
};


namespace g::SIZE {
    /* posting (doc_id and frequency) */
    constexpr uint32_t POSTING = sizeof(uint32_t) + sizeof(uint32_t);
    /* overhead of PostingsBuffer */
    constexpr uint32_t POSTINGBUF = sizeof(PostingsBuffer);
    /* overhead of a PostingsBufferMap item */
    constexpr uint32_t POSTINGBUF_MAPITEM = sizeof(PostingsBufferMap::value_type);
}