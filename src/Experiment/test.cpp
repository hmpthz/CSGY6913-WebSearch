#include "PyInterface.h"
#include "../Compression/BitVector.h"
#include <limits>
#include <mimalloc-new-delete.h>


void test_2_index() {
    //load_doctable("D:\\Downloads\\WSE_index\\doctable.bin");
    auto& buffreq = *get_inputbuf_Origin(300, "D:\\Downloads\\WSE_index\\lexicon.bin", "D:\\Downloads\\WSE_index\\index.bin");
    auto& bufscore = *get_inputbuf_UncompressedScore(300, "D:\\Downloads\\WSE_index\\s_lex.bin", "D:\\Downloads\\WSE_index\\s_index.bin", "D:\\Downloads\\WSE_index\\score.bin");

    get_sample_scores(bufscore);
}

void test_bits() {
    Bits::Vec bits;
    {
        auto iter = Bits::BackInserter<7>(bits);
        iter.append(56); iter.append(3); iter.append(91); iter.append(20);
        iter.end_byte();
    }
    {
        auto iter = Bits::ForwardIter<7>(bits);
        iter.set_byte(0);
        std::cout << iter.next() << iter.next() << iter.next() << iter.next() << '\n';
    }
}


int main() {
    test_2_index();
    //test_bits();
}