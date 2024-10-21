#pragma once
#include "../Index/DocTable.h"
#include "IndexLRUCache.h"
#include <queue>


struct QueryScore {
    uint32_t doc_id;
    float score; /* BM25 score of the document */
    QueryScore(uint32_t a, float b) :doc_id(a), score(b) {}
    static bool less(const QueryScore& a, const QueryScore& b) { return a.score < b.score; }
    static bool greater(const QueryScore& a, const QueryScore& b) { return a.score > b.score; }
};


struct cQueryResultArray {
    struct cQueryResult {
        uint32_t doc_id;
        float score;
        char* url;
        uint64_t offset; /* start offset of doc in file */
        uint32_t len;
    };
    uint32_t n;
    cQueryResult* arr;
    cQueryResultArray(uint32_t a) :n(a) { arr = new cQueryResult[n]; }
    ~cQueryResultArray() { delete arr; }
};


class Query {
public:
    static DocTable* docs;
    static IndexLRUCache* icache;

    static constexpr float k1 = 1.2f;
    static constexpr float b = 0.75f;
    static float BM25_score(Posting p, IndexTerm& index);

    /* get the next document score from QueryProcessor */
    virtual QueryScore get_next() = 0;
    /* get the final topK query result
    IMPORTANT: should call reset_index_state() after*/
    virtual cQueryResultArray* query_result(uint32_t topK) = 0;
    /* call IndexTerm.reset_read_state for every index when this query is finished */
    virtual void reset_index_state() = 0;
    virtual ~Query() {}
    /* declaration: std::priority_queue<T, Container<T>, decltype(&Function)> que(Function) */
    using MinScoreHeap = std::priority_queue<QueryScore, std::vector<QueryScore>, decltype(&QueryScore::greater)>;
    static cQueryResultArray* heap_to_result(MinScoreHeap& heap);
};


/* query for a single term */
class SingleQueryProcessor :public Query {
protected:
    IndexTermIter iiter;
public:
    SingleQueryProcessor(IndexTermIter iter);
    QueryScore get_next();
    cQueryResultArray* query_result(uint32_t topK);
    void reset_index_state();
};


/* <AND> query for a few terms */
class ANDQueryProcessor :public Query {
protected:
    std::vector<IndexTermIter> iiters;
    /* current doc_id for get_nextGEQ */
    uint32_t cur_did;
public:
    ANDQueryProcessor(std::vector<IndexTermIter>& iters);
    QueryScore get_next();
    cQueryResultArray* query_result(uint32_t topK);
    void reset_index_state();
    static bool less_postings(IndexTermIter& a, IndexTermIter& b) {
        return g::ival(a->info).n_docs < g::ival(b->info).n_docs;
    }
};


/*
    <OR> query for a few sub queries, they may be <AND> or single term.
    <OR> query should only be root query, not sub query.
*/
class ORQueryProcessor :public Query {
protected:
    std::vector<Query*> processors;
public:
    ORQueryProcessor(std::vector<Query*>& qp);
    QueryScore get_next(); /* shouldn't use it in ORQueryProcessor */
    cQueryResultArray* query_result(uint32_t topK);
    void reset_index_state();
    ~ORQueryProcessor();
    /* a verg large hash table to keep all the (doc_id, score) during query */
    thread_local static std::vector<float> hashtable;
};