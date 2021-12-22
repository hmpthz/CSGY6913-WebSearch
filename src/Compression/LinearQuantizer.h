#pragma once
#include "../global.h"


template<uint32_t BITS>
class LinearQuantizer {
public:
    float minval, maxval;

    LinearQuantizer();
    void update_minmax(float val);
    uint32_t quantize(float val);
    float dequantize(uint32_t q);
    void read(std::ifstream& fin);
    void write(std::ofstream& fout);
};


template<uint32_t BITS>
inline LinearQuantizer<BITS>::LinearQuantizer() {
    minval = 1.f;
    maxval = 1.f;
}

template<uint32_t BITS>
inline void LinearQuantizer<BITS>::update_minmax(float val) {
    if (val < minval) {
        minval = val;
    }
    else if (val > maxval) {
        maxval = val;
    }
}

template<uint32_t BITS>
inline uint32_t LinearQuantizer<BITS>::quantize(float val) {
    val = (val - minval) / (maxval - minval);
    return Bits::map<BITS>(val);
}

template<uint32_t BITS>
inline float LinearQuantizer<BITS>::dequantize(uint32_t q) {
    float val = Bits::norm<BITS>(q);
    return minval + val * (maxval - minval);
}

template<uint32_t BITS>
inline void LinearQuantizer<BITS>::read(std::ifstream& fin) {
    fin.read((char*)&minval, sizeof(minval));
    fin.read((char*)&maxval, sizeof(maxval));
}

template<uint32_t BITS>
inline void LinearQuantizer<BITS>::write(std::ofstream& fout) {
    fout.write((char*)&minval, sizeof(minval));
    fout.write((char*)&maxval, sizeof(maxval));
}
