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

}

template<uint32_t BITS, uint32_t EXP>
inline void AdaptiveFloatQuantizer<BITS, EXP>::dequantize(Bits::ForwardIter<BITS>& iter, std::vector<float>& arr, size_t len) {

}
