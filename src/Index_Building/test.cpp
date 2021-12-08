#include "PyInterface.h"
#include <algorithm>
#include <mimalloc-new-delete.h>


void testfile() {
    MemoryCounter memcnt;
    memcnt.set_capacity(999999);
    Lexicon_Freq lex;

    //TermInfo_Freq val;
    //auto lex_iter1 = lex.append_term("aaa", val);
    //auto index1 = TermIndex_Freq<>(memcnt, lex_iter1);
    //{
    //    auto back_inserter = index1.back_inserter();
    //    for (uint32_t doc_id = 0; doc_id < 128; doc_id++) {
    //        back_inserter.append(Posting(doc_id, doc_id % 4 + 1));
    //    }
    //}
    //auto lex_iter2 = lex.append_term("bbb", val);
    //auto index2 = TermIndex_Freq<>(memcnt, lex_iter2);
    //{
    //    auto back_inserter = index1.back_inserter();
    //    for (uint32_t doc_id = 128; doc_id < 140; doc_id++) {
    //        back_inserter.append(Posting(doc_id, doc_id % 9 + 2));
    //    }
    //}

    //lex.open_fout("D:\\hzflp\\Briefcase\\Projects\\WebSearch_Final\\out\\lex.bin");
    //std::ofstream fout("D:\\hzflp\\Briefcase\\Projects\\WebSearch_Final\\out\\a.bin", std::ios::binary);
    //index1.write(true, fout);
    //lex.write(lex_iter1);
    //index2.write(true, fout);
    //lex.write(lex_iter2);


    std::ifstream fin("D:\\hzflp\\Briefcase\\Projects\\WebSearch_Final\\out\\a.bin", std::ios::binary);
    lex.open_fin("D:\\hzflp\\Briefcase\\Projects\\WebSearch_Final\\out\\lex.bin");
    auto lex_iter1 = lex.read_next();
    auto lex_iter2 = lex.read_next();
    auto index1 = TermIndex_Freq<>(memcnt, lex_iter1);
    auto index2 = TermIndex_Freq<>(memcnt, lex_iter2);
    auto iter = index1.begin();
    int i = 0;
    while (true) {
        try {
            std::cout << iter.nextGEQ(i).doc_id << ' ';
            i += 2;
        }
        catch (g::Exception) {
            if (index1.is_endfile())
                break;
            else {
                index1.try_read_blocks<false>(fin);
            }
        }
    }
}


class _lexicon :public Lexicon_Freq {
public:
    using _lexitem = std::pair<std::string, TermInfo_Freq>;
    static bool greater(_lexitem& a, _lexitem& b) {
        if (a.second.n_docs == b.second.n_docs) {
            return a.first < b.first;
        }
        return a.second.n_docs > b.second.n_docs;
    }

    void test() {
        open_fin("D:\\Downloads\\WSE_index\\lexicon.bin");
        read_all();
        close_fin();

        std::vector<_lexitem> arr(terms.begin(), terms.end());
        std::sort(arr.begin(), arr.end(), _lexicon::greater);
        open_fout("D:\\Downloads\\WSE_index\\sorted_terms.txt");
        for (auto& item : arr) {
            fout << item.first << ' ' << item.second.n_docs << '\n';
        }
        close_fout();
    }
};


int main() {
    _lexicon().test();
    //DocTable doctable;
    //doctable.open_fin("D:\\Downloads\\WSE_index\\doctable.bin");
    //doctable.read_all();
    //std::cout << doctable.get_avg_len();

    return 0;
}