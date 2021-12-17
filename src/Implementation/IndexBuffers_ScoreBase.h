#pragma once
#include "TermIndex_Freq.h"
#include "TermIndex_Score.h"
#include "../IndexBuffer.h"


namespace InputBuffer {
    class Freq :public _Base<TermIndex_Freq<>, Lexicon_Freq, Freq> {
    public:
        inline void index_read_blocks(TermIndex& index) {
            index.try_read_blocks<false>(fin);
        }
        inline IndexForwardIter index_begin(TermIndex& index) {
            return index.begin();
        }
        void reset_fpos() {
            lex.reset_fpos();
            fin.seekg(std::ios::beg);
        }
    };


    template<typename Index_t, typename Derived>
    class _ScoreBase :public _Base<Index_t, Lexicon_Score, Derived> {
        using B = _Base<Index_t, Lexicon_Score, Derived>;
    protected:
        std::ifstream fin2;

    public:
        inline void index_read_blocks(Index_t& index) {
            index.try_read_blocks<false>(B::fin, fin2);
        }
        void open_fin(const char* filename, const char* filename2) {
            B::open_fin(filename);
            fin2.open(filename2, std::ios::binary);
        }
        void close_fin() {
            B::close_fin();
            if (fin2.is_open()) fin2.close();
        }
        void reset_fpos() {
            B::lex.reset_fpos();
            B::fin.seekg(std::ios::beg);
            fin2.seekg(std::ios::beg);
        }
    };
}


namespace OutputBuffer {
    template<typename Index_t, typename Derived>
    class _ScoreBase :public _Base<Index_t, Lexicon_Score, Derived> {
        using B = _Base<Index_t, Lexicon_Score, Derived>;
    protected:
        std::ofstream fout2;

    public:
        inline void index_write(bool end, Index_t& index) {
            index.write(end, B::write_did, B::fout, fout2);
        }
        void open_fout(const char* filename, const char* filename2) {
            if (B::write_did) {
                B::open_fout(filename);
            }
            fout2.open(filename2, std::ios::binary);
        }
        void close_fout() {
            B::close_fout();
            if (fout2.is_open()) fout2.close();
        }
    };
}
