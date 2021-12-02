#include "PyInterface.h"


void init(const char* index_filename, const char* lexicon_filename, const char* doctable_filename) {
    Query::docs = new DocTable;
    Query::docs->open_fin(doctable_filename);
    Query::docs->read_all();
    Query::docs->close_fin();

    Query::icache = new IndexLRUCache;
    Query::icache->open_fin(index_filename);
    Query::icache->lex.open_fin(lexicon_filename);
    Query::icache->lex.read_all();
    Query::icache->lex.close_fin();

    ORQueryProcessor::hashtable.resize(Query::docs->size(), 0);
}

void set_LRUCache_size(uint32_t size_in_MB) {
    size_t size_in_B = (size_t)size_in_MB * g::MB;
    Query::icache->set_capacity(size_in_B);
}

void load_LRUCache(const char* filename) {
    Query::icache->load_cache_list(filename);
}

void dump_LRUCache(const char* filename) {
    Query::icache->dump_cache_list(filename);
}

Query* get_SingleQueryProcessor(const char* term) {
    try {
        auto iiter = Query::icache->get_index(term);
        return new SingleQueryProcessor(iiter);
    }
    // TermNotFound Exception
    catch (g::Exception) { return NULL;  }
}

Query* get_ANDQueryProcessor(PyObject* term_list) {
    try {
        std::vector<IndexTermIter> iiters;
        Py_ssize_t len = PyList_Size(term_list);
        for (Py_ssize_t i = 0; i < len; i++) {
            auto term_obj = PyList_GetItem(term_list, i);
            auto iiter = Query::icache->get_index(PyBytes_AsString(term_obj));
            iiters.emplace_back(iiter);
        }

        return new ANDQueryProcessor(iiters);
    }
    // TermNotFound Exception
    catch (g::Exception) { return NULL; }
}

Query* get_ORQueryProcessor(PyObject* processor_list) {
    std::vector<Query*> processors;
    Py_ssize_t len = PyList_Size(processor_list);
    for (Py_ssize_t i = 0; i < len; i++) {
        auto processor_obj = PyList_GetItem(processor_list, i);
        Query* processor = (Query*)PyLong_AsVoidPtr(processor_obj);
        processors.emplace_back(processor);
    }

    return new ORQueryProcessor(processors);
}

cQueryResultArray* get_QueryResult(Query* processor, uint32_t topK) {
    auto result = processor->query_result(topK);
    processor->reset_index_state();
    return result;
}

void delete_QueryProcessor(Query* processor) {
    delete processor;
}

void delete_QueryResult(cQueryResultArray* query_res, cQueryTermsArray* terms_info) {
    delete query_res;
    delete terms_info;
}

void delete_all() {
    delete Query::docs;
    delete Query::icache;
    Query::docs = NULL;
    Query::icache = NULL;
}

cQueryTermsArray* get_QueryTerms(PyObject* term_list, cQueryResultArray* query_res) {
    // suppose these terms are all valid, so don't have to catch exception
    std::vector<IndexTermIter> iiters;
    Py_ssize_t len = PyList_Size(term_list);
    for (Py_ssize_t i = 0; i < len; i++) {
        auto term_obj = PyList_GetItem(term_list, i);
        auto iiter = Query::icache->get_index(PyBytes_AsString(term_obj));
        iiters.emplace_back(iiter);
    }
    return new cQueryTermsArray(iiters, query_res);
}

cQueryResultArray* test_query_result() {
    cQueryResultArray* res = new cQueryResultArray(2);
    res->arr[0].doc_id = 0;
    res->arr[0].score = 1.11f;
    res->arr[0].url = const_cast<char*>("aaaaa");
    res->arr[0].offset = 0;
    res->arr[1].doc_id = 1;
    res->arr[1].score = 2.22f;
    res->arr[1].url = const_cast<char*>("bbbbb");
    res->arr[1].offset = 1894;
    return res;
}

cQueryTermsArray::cQueryTermsArray(std::vector<IndexTermIter>& iiters, cQueryResultArray* query_res) {
    terms_n_docs = new uint32_t[iiters.size()];
    n = query_res->n;
    arr = new cQueryTerms[n];

    for (int i = 0; i < iiters.size(); i++) {
        terms_n_docs[i] = g::ival(iiters[i]->info).n_docs;
    }

    // iterate each doc
    for (int i = 0; i < query_res->n; i++) {
        uint32_t cur_did = query_res->arr[i].doc_id;
        vector_u32 vec_i;
        vector_u32 vec_freq;

        // iterate each term to check if they exist in current doc
        for (int j = 0; j < iiters.size(); j++) {
            // check by nextGEQ
            try {
                auto GEQ_p = Query::icache->get_nextGEQ_limit(*iiters[j], cur_did);
                // remember to reset
                Query::icache->reset_read_state_limit(*iiters[j]);

                if (cur_did != GEQ_p.doc_id) continue;
                vec_i.emplace_back(j);
                vec_freq.emplace_back(GEQ_p.frequency);
            }
            // IndexEndInFile Exception
            catch (g::Exception) { continue; }
        }

        // transform vector to TermsInfo
        arr[i].n = vec_i.size();
        arr[i].idxs = new uint32_t[arr[i].n];
        arr[i].freqs = new uint32_t[arr[i].n];
        for (int j = 0; j < arr[i].n; j++) {
            arr[i].idxs[j] = vec_i[j];
            arr[i].freqs[j] = vec_freq[j];
        }
    }
}

cQueryTermsArray::~cQueryTermsArray() {
    delete terms_n_docs;
    for (int i = 0; i < n; i++) {
        delete arr[i].idxs;
        delete arr[i].freqs;
    }
    delete arr;
}
