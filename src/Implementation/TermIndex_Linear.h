#pragma once
#include "TermIndex_Quantized.h"
#include "../Compression/LinearQuantizer.h"


namespace _Index {
    class Linear :public _Quantized {
        using _Quantized::_Quantized;
    public:
        template<uint32_t> friend class LinearForwardIter;
        template<uint32_t, uint32_t> friend class LinearBackInserter;
    };


    template<uint32_t BITS>
    class LinearForwardIter :public _QuantizedForwardIter<Linear, LinearForwardIter<BITS> > {
    protected:
        LinearQuantizer<BITS> quantizer;
        Bits::ForwardIter<BITS> bits_iter;
        void load_other_cache() {
            bits_iter.set_byte(B::cur_byte2);
            for (size_t i = 0; i < B::did_cache.size(); i++) {
                float score = quantizer.dequantize(bits_iter.next());
                B::score_cache.emplace_back(score);
            }
            // move cursor
            B::cur_byte2 += B::r
                ->blocks_meta[B::cur_block].score_bsize;
        }

    public:
        using B = _QuantizedForwardIter<Linear, LinearForwardIter<BITS> >;
        friend B::B;
        LinearForwardIter(Linear& _r, LinearQuantizer<BITS>& _q) :
            B(_r), quantizer(_q), bits_iter(_r.bits) {}
    };


    template<uint32_t BITS, uint32_t BLOCK>
    class LinearBackInserter :public _QuantizedBackInserter<Linear, BLOCK, LinearBackInserter<BITS, BLOCK> > {
    protected:
        LinearQuantizer<BITS> quantizer;
        Bits::BackInserter<BITS> bits_inserter;
        void unload_other_cache(uint32_t lastdid, uint16_t didbsize) {
            size_t bsize = B::r->bits.size();
            for (float val : B::score_cache) {
                bits_inserter.append(quantizer.quantize(val));
            }
            bits_inserter.end_byte();
            bsize = B::r->bits.size() - bsize;
            B::r->blocks_meta.emplace_back(lastdid, didbsize, (uint16_t)bsize);
        }

    public:
        using B = _QuantizedBackInserter<Linear, BLOCK, LinearBackInserter<BITS, BLOCK> >;
        friend B::B;
        LinearBackInserter(Linear& _r, LinearQuantizer<BITS>& _q) :
            B(_r), quantizer(_q), bits_inserter(_r.bits) {}
    };
}


namespace Index {
    template<uint32_t BITS, uint32_t BLOCK = g::BLOCK>
    class Linear :public _Index::Linear {
        using _Index::Linear::Linear;
    public:
        using ForwardIter = typename _Index::LinearForwardIter<BITS>;
        using BackInserter = typename _Index::LinearBackInserter<BITS, BLOCK>;

        inline ForwardIter begin(LinearQuantizer<BITS>& q) {
            return ForwardIter(*this, q);
        }
        inline BackInserter back_inserter(LinearQuantizer<BITS>& q) {
            return BackInserter(*this, q);
        }
    };
}