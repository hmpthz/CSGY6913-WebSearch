#pragma once
#include "TermIndex_Quantized.h"
#include "../Compression/AdaptiveFloat.h"


namespace _Index {
    class Adaptive :public _Quantized {
        using _Quantized::_Quantized;
    public:
        template<uint32_t, uint32_t> friend class AdaptiveForwardIter;
        template<uint32_t, uint32_t, uint32_t> friend class AdaptiveBackInserter;
    };


    template<uint32_t BITS, uint32_t EXP>
    class AdaptiveForwardIter :public _QuantizedForwardIter<Adaptive, AdaptiveForwardIter<BITS, EXP> > {
    protected:
        AdaptiveFloatQuantizer<BITS, EXP> quantizer;
        Bits::ForwardIter<BITS> bits_iter;
        void load_other_cache() {
            bits_iter.set_byte(B::cur_byte2);
            quantizer.dequantize(bits_iter, B::score_cache, B::did_cache.size());
            // move cursor
            B::cur_byte2 += B::r
                ->blocks_meta[B::cur_block].score_bsize;
        }

    public:
        using B = _QuantizedForwardIter<Adaptive, AdaptiveForwardIter<BITS, EXP> >;
        friend B::B;
        AdaptiveForwardIter(Adaptive& _r, AdaptiveFloatQuantizer<BITS, EXP>& _q) :
            B(_r), quantizer(_q), bits_iter(_r.bits) {}
    };


    template<uint32_t BITS, uint32_t EXP, uint32_t BLOCK>
    class AdaptiveBackInserter :public _QuantizedBackInserter<Adaptive, BLOCK, AdaptiveBackInserter<BITS, EXP, BLOCK> > {
    protected:
        AdaptiveFloatQuantizer<BITS, EXP> quantizer;
        Bits::BackInserter<BITS> bits_inserter;
        void unload_other_cache(uint32_t lastdid, uint16_t didbsize) {
            size_t bsize = B::r->bits.size();
            quantizer.quantize(B::score_cache, bits_inserter);
            bits_inserter.end_byte();
            bsize = B::r->bits.size() - bsize;
            B::r->blocks_meta.emplace_back(lastdid, didbsize, (uint16_t)bsize);
        }

    public:
        using B = _QuantizedBackInserter<Adaptive, BLOCK, AdaptiveBackInserter<BITS, EXP, BLOCK> >;
        friend B::B;
        AdaptiveBackInserter(Adaptive& _r, AdaptiveFloatQuantizer<BITS, EXP>& _q) :
            B(_r), quantizer(_q), bits_inserter(_r.bits) {}
    };
}


namespace Index {
    template<uint32_t BITS, uint32_t EXP, uint32_t BLOCK = g::BLOCK>
    class Adaptive :public _Index::Adaptive {
        using _Index::Adaptive::Adaptive;
    public:
        using ForwardIter = typename _Index::AdaptiveForwardIter<BITS, EXP>;
        using BackInserter = typename _Index::AdaptiveBackInserter<BITS, EXP, BLOCK>;

        inline ForwardIter begin(AdaptiveFloatQuantizer<BITS, EXP>& q) {
            return ForwardIter(*this, q);
        }
        inline BackInserter back_inserter(AdaptiveFloatQuantizer<BITS, EXP>& q) {
            return BackInserter(*this, q);
        }
    };
}
