#pragma once
#include "../global.h"


template<uint32_t BITS>
class LogQuantizer {
public:
    float minexp, maxexp;
    float neg_minexp, neg_maxexp;

    LogQuantizer();
    void update_minmax(float val);
    void log_minmax();
    uint32_t quantize(float val);
    float dequantize(uint32_t q);
    void read(std::ifstream& fin);
    void write(std::ofstream& fout);
};


template<uint32_t BITS>
inline LogQuantizer<BITS>::LogQuantizer() {
    minexp = 1.f;
    maxexp = 1.f;
    neg_minexp = 1.f;
    neg_maxexp = 1.f;
}

template<uint32_t BITS>
inline void LogQuantizer<BITS>::update_minmax(float val) {
    if (val > 0) {
        if (val < minexp) {
            minexp = val;
        }
        else if (val > maxexp) {
            maxexp = val;
        }
    }
    else if (val < 0) {
        val = std::abs(val);
        if (val < neg_minexp) {
            neg_minexp = val;
        }
        else if (val > neg_maxexp) {
            neg_maxexp = val;
        }
    }
}

template<uint32_t BITS>
inline void LogQuantizer<BITS>::log_minmax() {
    if (minexp < 0 || neg_minexp < 0) {
        throw g::Exception::Unreachable;
    }
    minexp = std::log2f(minexp);
    maxexp = std::log2f(maxexp);
    neg_minexp = std::log2f(neg_minexp);
    neg_maxexp = std::log2f(neg_maxexp);
}

template<uint32_t BITS>
inline uint32_t LogQuantizer<BITS>::quantize(float val) {
    if (val > 0) {
        float x = std::log2f(val);
        x = (x - minexp) / (maxexp - minexp);
        return Bits::map<(BITS - 1)>(x) + (1 << (BITS - 1));
    }
    else if (val < 0) {
        float x = std::log2f(std::abs(val));
        x = (x - neg_minexp) / (neg_maxexp - neg_minexp);
        return Bits::map<(BITS - 1), 1>(x) + 1;
    }
    else return 0;
}

template<uint32_t BITS>
inline float LogQuantizer<BITS>::dequantize(uint32_t q) {
    // positive value
    if (q >= (1 << (BITS - 1))) {
        q -= 1 << (BITS - 1);
        float x = Bits::norm<(BITS - 1)>(q);
        x = minexp + x * (maxexp - minexp);
        return std::powf(2, x);
    }
    // negative value
    else if (q >= 1) {
        q -= 1;
        float x = Bits::norm<(BITS - 1), 1>(q);
        x = neg_minexp + x * (neg_maxexp - neg_minexp);
        return -std::powf(2, x);
    }
    else return 0;
}

template<uint32_t BITS>
inline void LogQuantizer<BITS>::read(std::ifstream& fin) {
    fin.read((char*)&minexp, sizeof(minexp));
    fin.read((char*)&maxexp, sizeof(maxexp));
    fin.read((char*)&neg_minexp, sizeof(neg_minexp));
    fin.read((char*)&neg_maxexp, sizeof(neg_maxexp));
}

template<uint32_t BITS>
inline void LogQuantizer<BITS>::write(std::ofstream& fout) {
    fout.write((char*)&minexp, sizeof(minexp));
    fout.write((char*)&maxexp, sizeof(maxexp));
    fout.write((char*)&neg_minexp, sizeof(neg_minexp));
    fout.write((char*)&neg_maxexp, sizeof(neg_maxexp));
}
