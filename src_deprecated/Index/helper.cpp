#include "helper.h"


uint16_t CompressedBytes::compress(const vector_u32& data) {
    // temp stack for reversely store
    Stack stack;
    size_t total = 0;

    for (auto val : data) {
        if (val == 0) {
            emplace_back(0);
            total += 1;
            continue;
        }
        // mod and divide, get bytes in reverse order
        do {
            uint8_t b = val % 0b10000000;
            val /= 0b10000000;
            if (stack.size() > 0) {
                // not the first byte, set hightest bit to 1
                b = b | 0b10000000;
            }
            stack.push(b);
        } while (val != 0);

        total += stack.size();
        // append into bytes vector
        do {
            emplace_back(stack.top());
            stack.pop();
        } while (stack.size() > 0);
    }
    return (uint16_t)total;
}

void CompressedBytes::decompress(size_t start, size_t len, vector_u32& data) {
    uint32_t val = 0;
    
    auto end = std::min(start + len, size());
    for (size_t i = start; i < end; i++) {
        auto b = (*this)[i];
        if (b < 0b10000000) {
            // highest bit is 0, so it's the last byte of an integer
            data.emplace_back(val + b);
            val = 0;
        }
        else {
            // set hightst bit to 0
            b = b & 0b01111111;
            val += b;
            val *= 0b10000000;
        }
    }
}

void CompressedBytes::difference(uint32_t pre, vector_u32& data) {
    for (auto& val : data) {
        auto tmp = val;
        val -= pre;
        pre = tmp;
    }
}

void CompressedBytes::undifference(uint32_t pre, vector_u32& data) {
    for (auto& val : data) {
        val += pre;
        pre = val;
    }
}
