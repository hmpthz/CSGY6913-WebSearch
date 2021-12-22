#include "PyInterface.h"
#include "../Compression/BitVector.h"
#include <limits>
#include <mimalloc-new-delete.h>


void t_2_index() {
    //load_doctable("D:\\Downloads\\WSE_index\\doctable.bin");
    auto& buffreq = *get_inputbuf_Origin(300, "D:\\Downloads\\WSE_index\\lexicon.bin", "D:\\Downloads\\WSE_index\\index.bin");
    auto& bufscore = *get_inputbuf_UncompressedScore(300, "D:\\Downloads\\WSE_index\\s_lex.bin", "D:\\Downloads\\WSE_index\\s_index.bin", "D:\\Downloads\\WSE_index\\score.bin");

}

void t_bits() {
    Bits::Vec bits;
    size_t i;
    {
        auto iter = Bits::BackInserter<8>(bits);
        iter.append(56); iter.append(3); iter.append(91); iter.append(20);
        iter.end_byte();
        i = bits.size();
        iter.append(21);
        iter.end_byte();
    }
    {
        auto iter = Bits::ForwardIter<8>(bits);
        iter.set_byte(0);
        std::cout << iter.next() << iter.next() << iter.next() << iter.next();
        iter.set_byte(i);
        std::cout << iter.next() << '\n';
    }
}

void t_linear() {
    constexpr uint32_t B = 8;
    Bits::Vec bits;
    LinearQuantizer<B> quantizer;
    Bits::BackInserter<B> bits_inserter(bits);
    float scores[] = { -1.65, 0.67, 2.85, -0.23, 1.17, 2.71 };

    size_t i = 0;
    for (; i < 6; i++) {
        quantizer.update_minmax(scores[i]);
    }

    size_t bsize = bits.size();
    i = 0;
    for (; i < 4; i++) {
        bits_inserter.append(quantizer.quantize(scores[i]));
    }
    bits_inserter.end_byte();
    bsize = bits.size() - bsize;
    for (; i < 6; i++) {
        bits_inserter.append(quantizer.quantize(scores[i]));
    }
    bits_inserter.end_byte();


    Bits::ForwardIter<B> bits_iter(bits);
    i = 0;
    bits_iter.set_byte(i);
    for (; i < 4; i++) {
        float score = quantizer.dequantize(bits_iter.next());
        std::cout << score << ' ';
    }
    bits_iter.set_byte(bsize);
    for (; i < 6; i++) {
        float score = quantizer.dequantize(bits_iter.next());
        std::cout << score << ' ';
    }
}

void t_log() {
    constexpr uint32_t B = 6;
    Bits::Vec bits;
    LogQuantizer<B> quantizer;
    Bits::BackInserter<B> bits_inserter(bits);
    float scores[] = { -1.65, 0.67, 2.85, -0.23, 1.17, 2.71 };

    size_t i = 0;
    for (; i < 6; i++) {
        quantizer.update_minmax(scores[i]);
    }
    quantizer.log_minmax();
    std::cout << quantizer.minexp << ' ' << quantizer.maxexp << ' ' << quantizer.neg_minexp << ' ' << quantizer.neg_maxexp << '\n';

    size_t bsize = bits.size();
    i = 0;
    for (; i < 4; i++) {
        bits_inserter.append(quantizer.quantize(scores[i]));
    }
    bits_inserter.end_byte();
    bsize = bits.size() - bsize;
    for (; i < 6; i++) {
        bits_inserter.append(quantizer.quantize(scores[i]));
    }
    bits_inserter.end_byte();


    Bits::ForwardIter<B> bits_iter(bits);
    i = 0;
    bits_iter.set_byte(i);
    for (; i < 4; i++) {
        float score = quantizer.dequantize(bits_iter.next());
        std::cout << score << ' ';
    }
    bits_iter.set_byte(bsize);
    for (; i < 6; i++) {
        float score = quantizer.dequantize(bits_iter.next());
        std::cout << score << ' ';
    }
}

void t_adapt() {
    constexpr uint32_t B = 12;
    Bits::Vec bits;
    AdaptiveFloatQuantizer<B, 4> quantizer;
    Bits::BackInserter<B> bits_inserter(bits);
    std::vector<float> scores1 = { -1.65, 0.67, 2.85, -0.23 };
    std::vector<float> scores2 = { 1.17, 2.71 };

    size_t bsize = bits.size();
    quantizer.quantize(scores1, bits_inserter);
    bits_inserter.end_byte();
    bsize = bits.size() - bsize;
    quantizer.quantize(scores2, bits_inserter);
    bits_inserter.end_byte();


    std::vector<float> scores3;
    Bits::ForwardIter<B> bits_iter(bits);
    bits_iter.set_byte(0);
    quantizer.dequantize(bits_iter, scores3, 4);
    bits_iter.set_byte(bsize);
    quantizer.dequantize(bits_iter, scores3, 2);
    for (float val : scores3) {
        std::cout << val << ' ';
    }
}


int main() {
    //t_2_index();
    //t_bits();
    //t_linear();
    //t_log();
    //t_adapt();
    uint8_t a = (uint8_t)-16;
    std::cout << (int)(int8_t)a;

    return 0;
}