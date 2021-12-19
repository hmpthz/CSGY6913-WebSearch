#pragma once
#include "../global.h"


template<uint32_t BITS>
class LinearQuantizer {
public:
    float minval, maxval;

    LinearQuantizer();
    void update_minmax(float val);
    uint32_t quantize(float val);
    float dequantize(uint32_t x);
};


template<uint32_t BITS>
inline LinearQuantizer<BITS>::LinearQuantizer() {
    minval = std::numeric_limits<float>::max();
    maxval = std::numeric_limits<float>::lowest();
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
    return uint32_t();
}

template<uint32_t BITS>
inline float LinearQuantizer<BITS>::dequantize(uint32_t x) {
    return 0.0f;
}
