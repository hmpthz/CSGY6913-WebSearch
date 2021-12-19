#pragma once
#include "../global.h"


template<uint32_t BITS>
class LogQuantizer {
public:
    float minlog, maxlog;
    float neg_minlog, neg_maxlog;

    LogQuantizer();
    void update_minmax(float val);
    void log_minmax();
    uint32_t quantize(float val);
    float dequantize(uint32_t x);
};


template<uint32_t BITS>
inline LogQuantizer<BITS>::LogQuantizer() {
    minlog = std::numeric_limits<float>::max();
    maxlog = 0;
    neg_minlog = std::numeric_limits<float>::max();
    neg_maxlog = 0;
}

template<uint32_t BITS>
inline void LogQuantizer<BITS>::update_minmax(float val) {
    if (val > 0) {

    }
    else if (val < 0) {

    }
}

template<uint32_t BITS>
inline void LogQuantizer<BITS>::log_minmax() {
    if (minlog < 0 || neg_minlog < 0) {
        throw g::Exception::Unreachable;
    }
    minlog = std::logf(minlog);
    maxlog = std::logf(maxlog);
    neg_minlog = std::logf(neg_minlog);
    neg_maxlog = std::logf(neg_maxlog);
}

template<uint32_t BITS>
inline uint32_t LogQuantizer<BITS>::quantize(float val) {
    return uint32_t();
}

template<uint32_t BITS>
inline float LogQuantizer<BITS>::dequantize(uint32_t x) {
    return 0.0f;
}
