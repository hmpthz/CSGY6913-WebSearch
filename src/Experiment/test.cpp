#include "PyInterface.h"
#include "../Compression/BitVector.h"
#include <mimalloc-new-delete.h>


void test_2_index() {
    //load_doctable("D:\\Downloads\\WSE_index\\doctable.bin");
    auto& buffreq = *get_inputbuf_Origin(300, "D:\\Downloads\\WSE_index\\lexicon.bin", "D:\\Downloads\\WSE_index\\index.bin");
    auto& bufscore = *get_inputbuf_UncompressedScore(300, "D:\\Downloads\\WSE_index\\s_lex.bin", "D:\\Downloads\\WSE_index\\s_index.bin", "D:\\Downloads\\WSE_index\\score.bin");

    //buffreq.read_fill();
    //buffreq.erase_front();
    //buffreq.erase_front();
    //auto& index1 = buffreq.front();
    //auto iiter1 = buffreq.index_begin(index1);

    //while (true) {
    //    if (iiter1.has_next()) {
    //        std::cout << iiter1.next().doc_id << ' ';
    //    }
    //    else if (index1.is_endfile()) {
    //        break;
    //    }
    //    else {
    //        buffreq.index_read_blocks(index1);
    //    }
    //}

    bufscore.read_fill();
    bufscore.erase_front();
    bufscore.erase_front();
    auto& index2 = bufscore.front();
    auto iiter2 = bufscore.index_begin(index2);

    while (true) {
        if (iiter2.has_next()) {
            std::cout << iiter2.next().doc_id << ' ';
        }
        else if (index2.is_endfile()) {
            break;
        }
        else {
            bufscore.index_read_blocks(index2);
        }
    }
}

void test_bits() {
    Bits::Vec bits;
    {
        auto iter = Bits::BackInserter<12>(bits);
        iter.append(120); iter.append(3); iter.append(90); iter.append(20);
        iter.end_byte();
    }
    {
        auto iter = Bits::ForwardIter<12>(bits);
        iter.set_byte(0);
        std::cout << iter.next() << iter.next() << iter.next() << iter.next() << '\n';
    }
}


int main() {
    test_bits();
}