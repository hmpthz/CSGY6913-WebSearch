

template<uint32_t BITS>
inline void Bits::ForwardIter<BITS>::set_byte(size_t i) {
    ptr = &bytes[i];
    b = *ptr;
    remain_bits_in_byte = 8;
}

template<uint32_t BITS>
inline uint32_t Bits::ForwardIter<BITS>::next() {
    int remain_bits_in_N = (int)BITS;
    int read_bits_in_N = 0; // # of bits that have been read
    uint32_t x = 0; // result bits to be returned

    while (true) {
        if (remain_bits_in_byte <= remain_bits_in_N) {
            // read all bits in current byte
            x |= (uint32_t)b << read_bits_in_N;

            remain_bits_in_N -= remain_bits_in_byte;
            read_bits_in_N += remain_bits_in_byte;

            // move to next byte to read
            ptr++;
            b = *ptr;
            remain_bits_in_byte = 8;

            if (remain_bits_in_N <= 0) {
                break;
            }
        }
        else {
            // only read remain_bits_in_N bits in current byte
            uint32_t mask = (uint32_t)-1 >> (32 - remain_bits_in_N);
            x |= ((uint32_t)b & mask) << read_bits_in_N;

            remain_bits_in_byte -= remain_bits_in_N;
            // shift bits to remove bits that've been read
            b = b >> remain_bits_in_N;

            break;
        }
    }
    return x;
}


template<uint32_t BITS>
inline void Bits::BackInserter<BITS>::append(uint32_t x) {
    int remain_bits_in_N = (int)BITS;

    while (true) {
        if (remain_bits_in_byte <= 0) {
            bytes.emplace_back(0);
            ptr = &bytes.back();
            remain_bits_in_byte = 8;
        }
        
        int appended_bits_in_byte = 8 - remain_bits_in_byte;

        if (remain_bits_in_byte < remain_bits_in_N) {
            // only append remain_bits_in_byte bits in current byte
            uint8_t mask = (uint8_t)-1 >> appended_bits_in_byte;
            *ptr |= ((uint8_t)x & mask) << appended_bits_in_byte;

            // shift bits to remove bits that've been appended
            x = x >> remain_bits_in_byte;
            remain_bits_in_N -= remain_bits_in_byte;
            remain_bits_in_byte = 0;
        }
        else {
            // append all remain_bits_in_N bits in current byte
            *ptr |= x << appended_bits_in_byte;
            remain_bits_in_byte -= remain_bits_in_N;

            break;
        }
    }
}

template<uint32_t BITS>
inline void Bits::BackInserter<BITS>::end_byte() {
    remain_bits_in_byte = 0;
}
