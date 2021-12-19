#pragma once
#include "../Implementation/IndexBuffer_Specialized.h"


template<typename Buffer_t>
void quantize(InputBuffer::Score& srcbuf, Buffer_t dstbuf) {

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
inline double MSE(InputBuffer::Score& baseline, Buffer_t& buf) {
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
    return sum / n;
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