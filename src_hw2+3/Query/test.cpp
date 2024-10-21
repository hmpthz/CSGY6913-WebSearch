#include "PyInterface.h"
#include <algorithm>

using _lexitem = std::pair<std::string, LexiconValue>;

class _lexicon :public Lexicon {
public:
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
	//_lexicon().test();

	init("D:\\Downloads\\WSE_index\\index.bin", "D:\\Downloads\\WSE_index\\testlex.bin", "D:\\Downloads\\WSE_index\\doctable.bin");
	set_LRUCache_size(100);

	//auto lexiter1 = Query::icache->lex.get_iter("cat");
	//auto lexiter2 = Query::icache->lex.get_iter("mouse");
	//Lexicon lex;
	//lex.open_fout("D:\\Downloads\\WSE_index\\testlex.bin");
	//auto& val1 = g::ival(lexiter1);
	//auto& val2 = g::ival(lexiter2);
	//lex.add_term(g::ikey(lexiter1), val1.start_offset, val1.end_offset, val1.n_docs);
	//lex.add_term(g::ikey(lexiter2), val2.start_offset, val2.end_offset, val2.n_docs);
	//lex.write(lex.get_iter(g::ikey(lexiter1)));
	//lex.write(lex.get_iter(g::ikey(lexiter2)));

	std::vector<IndexTermIter> iiters;
	iiters.emplace_back(Query::icache->get_index("cat"));
	iiters.emplace_back(Query::icache->get_index("mouse"));
	std::vector<Query*> processors;
	processors.emplace_back(new SingleQueryProcessor(iiters[0]));
	processors.emplace_back(new SingleQueryProcessor(iiters[1]));

	try {
		ANDQueryProcessor(iiters).query_result(10);
	}
	catch (g::Exception) {}

}