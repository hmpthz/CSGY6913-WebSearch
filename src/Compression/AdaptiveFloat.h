#pragma once
#include "../global.h"
#include "BitVector.h"


template<uint32_t BITS, uint32_t EXP>
class AdaptiveFloatQuantizer {
public:
    void quantize(std::vector<float>& arr, Bits::BackInserter<BITS>& inserter);
    void dequantize(Bits::ForwardIter<BITS>& iter, std::vector<float>& arr, size_t len);
};


template<uint32_t BITS, uint32_t EXP>
inline void AdaptiveFloatQuantizer<BITS, EXP>::quantize(std::vector<float>& arr, Bits::BackInserter<BITS>& inserter) {
    constexpr uint32_t MANT = BITS - 1 - EXP;
    int8_t sign[g::BLOCK];

    // get sign, abs and exp range
    int exp_max = std::numeric_limits<int>::lowest();
    for (size_t i = 0; i < arr.size(); i++) {
        sign[i] = g::sgn(arr[i]);
        if (sign[i] == 0) {
            continue;
        }

        arr[i] = std::abs(arr[i]);
        int exp;
        std::frexpf(arr[i], &exp);
        if (exp > exp_max) {
            exp_max = exp;
        }
    }
    int exp_bias = exp_max - ((1 << EXP) - 1);
    // add exp_bias byte of this block
    inserter.append_byte((uint8_t)exp_bias);

    // value minmax
    float value_min = std::powf(2.f, (float)exp_bias) * (1.f + std::powf(2.f, -(float)MANT));
    float value_max = std::powf(2.f, (float)exp_max) * (2.f - std::powf(2.f, -(float)MANT));

    for (size_t i = 0; i < arr.size(); i++) {
        if (sign[i] == 0) {
            inserter.append(0);
            continue;
        }
        // clip range
        if (arr[i] < 0.5f * value_min) {
            arr[i] = 0;
            inserter.append(0);
            continue;
        }
        else if (arr[i] < value_min) {
            arr[i] = value_min;
        }
        else if (arr[i] > value_max) {
            arr[i] = value_max;
        }

        int exp;
        float mantissa = std::frexpf(arr[i], &exp);
        // now exp should be positive
        exp = exp - exp_bias;

        uint32_t q = 0;
        // quantize mantissa
        // mant: [0.5, 1.0) -> [0.0, 1.0)
        mantissa = 2.f * mantissa - 1;
        if (sign[i] > 0) {
            // q_mant: [2**m, 2**(m+1)-1]
            q = Bits::map<MANT>(mantissa) + (1 << MANT);
        }
        else {
            // q_mant: [1, 2**m-1]
            q = Bits::map<MANT, 1>(mantissa) + 1;
        }
        // concatenate exp and mantissa
        q = (exp << (MANT + 1)) | q;
        inserter.append(q);
    }
}

template<uint32_t BITS, uint32_t EXP>
inline void AdaptiveFloatQuantizer<BITS, EXP>::dequantize(Bits::ForwardIter<BITS>& iter, std::vector<float>& arr, size_t len) {
    constexpr uint32_t MANT = BITS - 1 - EXP;
    constexpr uint32_t MANT_MASK = ~((uint32_t)-1 << (MANT + 1));
    int exp_bias = (int)(int8_t)iter.next_byte();

    for (size_t i = 0; i < len; i++) {
        uint32_t q = iter.next();
        // extract exp
        int exp = q >> (MANT + 1);
        exp = exp + exp_bias;
        // convert mantissa
        q = q & MANT_MASK;
        if (q >= (1 << MANT)) {
            q -= (1 << MANT);
            float mantissa = Bits::norm<MANT>(q);
            mantissa = 0.5f * (mantissa + 1);
            arr.emplace_back(mantissa * std::powf(2.f, (float)exp));
        }
        else if (q >= 1) {
            q -= 1;
            float mantissa = Bits::norm<MANT, 1>(q);
            mantissa = 0.5f * (mantissa + 1);
            arr.emplace_back(-mantissa * std::powf(2.f, (float)exp));
        }
        else {
            arr.emplace_back(0.f);
        }
    }
}
