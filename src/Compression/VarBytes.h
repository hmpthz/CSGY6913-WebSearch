#pragma once
#include "../global.h"
#include <array>


/*
    use PMR for better managing memory
    constructor should have memory resource
*/
class VarBytes :public std::pmr::vector<uint8_t> {
    using vector::vector;
public:
    /* compress the data and append into bytes vecotr
    [return]
    size of compressed data in bytes
    usually size of a block shouldn't be too large, so 16bit is enough */
    uint16_t compress(const vector_u32& data) noexcept;
    /* [params]
    (start) start index of bytes vector
    (len) # of bytes to be decompressed */
    void decompress(size_t start, size_t len, vector_u32& data) noexcept;
    /* take the difference between each value in the data
    data should be ascending order
    [params]
    (pre) the value before first value of data */
    inline static void difference(uint32_t pre, vector_u32& data) noexcept {
        for (auto& val : data) {
            auto tmp = val;
            val -= pre; pre = tmp;
        }
    }
    /* restore original data from differenced data
    [params]
    (pre) the value before first value of data */
    inline static void undifference(uint32_t pre, vector_u32& data) noexcept {
        for (auto& val : data) {
            val += pre; pre = val;
        }
    }

    /* A simple stack used in VarBytes.compress method */
    class Stack {
    protected:
        uint8_t n;
        std::array<uint8_t, 5> arr; // store compressed bytes of a single integer
    public:
        Stack() :n(0) {}
        inline uint8_t size() { return n; }
        inline void push(uint8_t x) { arr[n++] = x; }
        inline void pop() { n--; }
        inline uint8_t top() { return arr[n - 1]; }
    };
};
