#pragma once
#include <iostream>
#include <fstream>
#include <string>
#include <tuple>
#include <vector>
using ios = std::ios;
using vector_u8 = std::vector<uint8_t>;
using vector_u16 = std::vector<uint16_t>;
using vector_u32 = std::vector<uint32_t>;
using vector_u64 = std::vector<uint64_t>;


/* global parameters and methods */
namespace g {
    /* # of postings per block */
    constexpr uint32_t BLOCK = 64;
    /* # of blocks per term
    don't keep the complete index of a term in memory because it can be large
    during merging there's no limit. used for query cache */
    constexpr uint32_t N_BLOCKS = 65535;
    constexpr uint32_t MB = (uint32_t)1E6;

    /* All exceptions used for projects */
    enum class Exception {
        IndexEndInMemory,
        IndexEndInFile,
        LexiconEndInFile,
        AllInputBuffersExhausted,
        TermNotFound
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

    /* key getter of an pair iterator */
    template<typename _It, typename _Key = typename std::iterator_traits<_It>::value_type::first_type>
    inline _Key& ikey(_It it) { return it->first; }
    /* value getter of an pair iterator */
    template<typename _It, typename _Val = typename std::iterator_traits<_It>::value_type::second_type>
    inline _Val& ival(_It it) { return it->second; }
}


/* varbyte compression */
class CompressedBytes {
public:
    vector_u8 bytes;

    inline size_t byte_size() {
        return bytes.size();
    }
    /* compress the data and append into bytes vecotr
    [return]
    size of compressed data in bytes
    usually size of a block shouldn't be too large, so 16bit is enough */
    uint16_t compress(const vector_u32& data);
    /* [params]
    (start) start index of bytes vector
    (len) # of bytes to be decompressed */
    void decompress(size_t start, size_t len, vector_u32& data);
    /* take the difference between each value in the data
    data should be ascending order
    [params]
    (pre) the value before first value of data */
    static void difference(uint32_t pre, vector_u32& data);
    /* restore original data from differenced data
    [params]
    (pre) the value before first value of data */
    static void undifference(uint32_t pre, vector_u32& data);

    /* A simple stack used in CompressedBytes.compress method */
    class Stack {
    protected:
        uint8_t n;
        uint8_t arr[5]; // store compressed bytes of a single integer
    public:
        Stack() :n(0) {}
        uint8_t size() { return n; }
        void push(uint8_t x) { arr[n++] = x; }
        void pop() { n--; }
        uint8_t top() { return arr[n - 1]; }
    };
};