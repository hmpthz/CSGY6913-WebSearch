#pragma once
#include "../global.h"


namespace Bits {
    /*
    each block contains many bits, which are packed contiguously
    but different blocks may be separated by byte
    */
    using Vec = std::pmr::vector<uint8_t>;


    /* start from a byte, sequentially read N bits at a time */
    template<uint32_t N>
    class ForwardIter {
    protected:
        Vec& bytes;
        /* pointer of current byte */
        uint8_t* ptr;
        /* remaining # of bits that haven't been read in current byte */
        uint32_t remain_bits;

    public:
        ForwardIter(Vec& a) :bytes(a) {}
        /* start from i_th byte */
        void set_byte(size_t i);
        /* get the next N bits, if remain_bits are not enough, move to next byte */
        uint32_t next();
    };


    template<uint32_t N>
    class BackInserter {
    protected:
        Vec& bytes;
        uint8_t* ptr;
        /* remaining # of bits that haven't been filled in current byte */
        uint32_t remain_bits;

    public:
        BackInserter(Vec& a) :bytes(a) {}
        /* append N bits, if remain_bits are not enough, append a new byte */
        void append(uint32_t x);
        /* start from a new byte */
        void end_byte();
    };
}