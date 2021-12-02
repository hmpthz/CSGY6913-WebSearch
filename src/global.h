#pragma once
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
using vector_u32 = std::vector<uint32_t>;
#include <map>


struct Posting {
    uint32_t doc_id;
    uint32_t frequency;
    Posting(uint32_t a, uint32_t b) :doc_id(a), frequency(b) {}
    using Vec = std::pmr::vector<Posting>;
    /* postings from a single document, so each term only has frequency */
    using Map = std::map<std::string, uint32_t>;
};

struct PostScore {
    uint32_t doc_id;
    float score; /* BM25 score of the document */
    PostScore(uint32_t a, float b) :doc_id(a), score(b) {}
    inline static bool less(const PostScore& a, const PostScore& b) {
        return a.score < b.score;
    }
    inline static bool greater(const PostScore& a, const PostScore& b) {
        return a.score > b.score;
    }
};


/* global parameters and methods */
namespace g {
    /* # of postings per block */
    constexpr uint32_t BLOCK = 128;
    /* # of blocks kept in memory for each term's index
    may not keep the complete index in memory if it's large
    however it's never been used, just a try for template programming */
    constexpr uint32_t N_BLOCKS = 16384;
    constexpr uint32_t MB = 1024 * 1024;
    constexpr uint32_t RESERVE = 4;

    /* All exceptions used for projects */
    enum class Exception {
        EndInFile,
        TermNotFound,
        BufferFull
    };

    /* used for reading string in binary file, copy characters until '\0' */
    template<typename _InIt, typename _OutIt>
    inline void copy_until_zero(_InIt _First, _InIt _Last, _OutIt _Dest) {
        if (_First == _Last) return;
        do {
            *_Dest++ = *_First++;
            if (_First == _Last) break;
            else if (*_First == 0) {
                _First++; // skip '\0'
                break;
            }
        } while (true);
    }

    /* MUST use typename instead of class */
    /* key getter of an pair iterator */
    template<typename _It, typename _Key = typename std::iterator_traits<_It>::value_type::first_type>
    inline _Key& ikey(_It it) { return it->first; }
    /* value getter of an pair iterator */
    template<typename _It, typename _Val = typename std::iterator_traits<_It>::value_type::second_type>
    inline _Val& ival(_It it) { return it->second; }
}
