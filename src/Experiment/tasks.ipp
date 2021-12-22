#pragma once
#include "../Implementation/IndexBuffer_Specialized.h"


namespace Task {
    template<uint32_t BITS>
    void quantize_linear(InputBuffer::Score& srcbuf, OutputBuffer::Linear<BITS>& dstbuf) {
        srcbuf.read_fill();
        auto iter = InputBuffer::SequentialIter(srcbuf);
        while (iter.has_next()) {
            float score = iter.next().score;
            dstbuf.quantizer.update_minmax(score);
        }

        srcbuf.reset_fpos();
        dstbuf.write_quantizer();
        Transfer::all_data(srcbuf, dstbuf);
    }

    template<uint32_t BITS>
    void quantize_log(InputBuffer::Score& srcbuf, OutputBuffer::Log<BITS>& dstbuf) {
        srcbuf.read_fill();
        auto iter = InputBuffer::SequentialIter(srcbuf);
        while (iter.has_next()) {
            float score = iter.next().score;
            dstbuf.quantizer.update_minmax(score);
        }
        dstbuf.quantizer.log_minmax();

        srcbuf.reset_fpos();
        dstbuf.write_quantizer();
        Transfer::all_data(srcbuf, dstbuf);
    }

    template<uint32_t BITS, uint32_t EXP>
    void quantize_adaptive(InputBuffer::Score& srcbuf, OutputBuffer::Adaptive<BITS, EXP>& dstbuf) {
        Transfer::all_data(srcbuf, dstbuf);
    }

    template<typename Buffer_t>
    void test_scores(Buffer_t& buf) {
        buf.read_fill();
        auto iter = InputBuffer::SequentialIter(buf);
        float sum = 0;
        while (iter.has_next()) {
            sum += iter.next().score;
        }
    }

    template<>
    inline void test_scores<InputBuffer::Freq>(InputBuffer::Freq& buf) {
        buf.read_fill();
        auto iter = InputBuffer::SequentialIter(buf);
        float sum = 0;
        while (iter.has_next()) {
            sum += BM25::score(iter.next(), iter.current_index());
        }
    }

    template<typename Buffer_t>
    inline void MSE(InputBuffer::Score& baseline, Buffer_t& buf) {
        baseline.read_fill();
        auto baseiter = InputBuffer::SequentialIter(baseline);
        buf.read_fill();
        auto iter = InputBuffer::SequentialIter(buf);

        double sum = 0;
        uint64_t n = 0;
        while (baseiter.has_next() && iter.has_next()) {
            float score1 = baseiter.next().score;
            float score2 = iter.next().score;
            sum += (double)((score1 - score2) * (score1 - score2));
            n++;
        }
        double mse = sum / n;
        std::cout << "[MSE] " << mse << '\n';
    }

    inline void get_sample_scores(InputBuffer::Score& buf) {
        buf.read_fill();
        while (buf.size() > 1) {
            auto& index = buf.front();
            if (g::ival(index.info).n_blocks < 2) {
                buf.erase_front();
                continue;
            }
            auto iter = buf.index_begin(index);
            for (uint32_t i = 0; i < g::BLOCK; i++) {
                std::cout << iter.next().score << ',';
            }
            std::cout << '\n';
            buf.erase_front();
        }
    }
}
