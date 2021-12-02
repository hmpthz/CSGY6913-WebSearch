#pragma once
#include "QueryProcessor.h"
#include <Python.h>

#define PY_API extern "C" __declspec(dllexport)


struct cQueryTermsArray {
    struct cQueryTerms {
        uint32_t n;
        uint32_t* idxs; /* indexes of valid query terms list in a document passed by python */
        uint32_t* freqs; /* frequencies of valid query terms list */
    };
    uint32_t* terms_n_docs; /* the inverted list lengths of the terms */
    uint32_t n;
    cQueryTerms* arr;
    cQueryTermsArray(std::vector<IndexTermIter>& iiters, cQueryResultArray* query_res);
    ~cQueryTermsArray();
};


PY_API void init(const char* index_filename, const char* lexicon_filename, const char* doctable_filename);
PY_API void set_LRUCache_size(uint32_t size_in_MB);
PY_API void load_LRUCache(const char* filename);
PY_API void dump_LRUCache(const char* filename);

PY_API Query* get_SingleQueryProcessor(const char* term);
PY_API Query* get_ANDQueryProcessor(PyObject* term_list);
PY_API Query* get_ORQueryProcessor(PyObject* processor_list);
PY_API cQueryResultArray* get_QueryResult(Query* processor, uint32_t topK);

PY_API void delete_QueryProcessor(Query* processor);
PY_API void delete_QueryResult(cQueryResultArray* query_res, cQueryTermsArray* terms_info);
PY_API void delete_all();

PY_API cQueryTermsArray* get_QueryTerms(PyObject* term_list, cQueryResultArray* query_res);
PY_API cQueryResultArray* test_query_result();