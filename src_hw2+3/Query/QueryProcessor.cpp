#include "QueryProcessor.h"


DocTable* Query::docs = NULL;
IndexLRUCache* Query::icache = NULL;
thread_local std::vector<float> ORQueryProcessor::hashtable;


float Query::BM25_score(Posting p, IndexTerm& index) {
    uint32_t N = (uint32_t)Query::docs->size();
    uint32_t ft = g::ival(index.info).n_docs;
    uint32_t fdt = p.frequency;
    uint32_t len_d = Query::docs->get_item(p.doc_id).len;
    float len_avg_d = Query::docs->get_avg_len();
    float K = k1 * ((1 - b) + b * (len_d / len_avg_d));
    float IDF = ((float)(N - ft) + 0.5f) / ((float)ft + 0.5f);
    IDF = std::logf(IDF);
    float weight = ((k1 + 1) * fdt) / (K + fdt);
    return IDF * weight;
}

cQueryResultArray* Query::heap_to_result(MinScoreHeap& heap) {
    auto result = new cQueryResultArray((uint32_t)heap.size());
    // extract min scores, reversely put into array, so that array is descending order
    for (int i = (int)heap.size() - 1; i >= 0; i--) {
        auto& qs = heap.top();
        auto& doc_item = Query::docs->get_item(qs.doc_id);
        result->arr[i].doc_id = qs.doc_id;
        result->arr[i].score = qs.score;
        result->arr[i].url = const_cast<char*>(doc_item.url.c_str());
        result->arr[i].offset = doc_item.start_offset;
        result->arr[i].len = doc_item.len;

        heap.pop();
    }
    return result;
}

SingleQueryProcessor::SingleQueryProcessor(IndexTermIter iter) {
    iiter = iter;
}

ANDQueryProcessor::ANDQueryProcessor(std::vector<IndexTermIter>& iters) {
    iiters = std::move(iters);
    cur_did = 0;
    // sort that # of postings of terms in the array are ascending
    std::sort(iiters.begin(), iiters.end(), ANDQueryProcessor::less_postings);
}

ORQueryProcessor::ORQueryProcessor(std::vector<Query*>& qp) {
    processors = std::move(qp);
}


QueryScore SingleQueryProcessor::get_next() {
    auto p = icache->get_next(*iiter);
    auto score = BM25_score(p, *iiter);
    return QueryScore(p.doc_id, score);
}

QueryScore ANDQueryProcessor::get_next() {
    // Document-At-A-Time Query Processing
    uint32_t term_i = 0;
    // frequencies of every term for the same doc_id 
    vector_u32 freqs(iiters.size(), 0);

    // call get_nextGEQ until every term has the same doc_id
    while (term_i < iiters.size()) {
        auto p = Query::icache->get_nextGEQ_limit(*iiters[term_i], cur_did);
        // get did from the first term, then compare to others
        if (term_i > 0 && p.doc_id > cur_did) {
            cur_did = p.doc_id;
            term_i = 0;
            continue;
        }
        cur_did = p.doc_id;
        freqs[term_i] = p.frequency;
        term_i++;
    }
    // finally add all the BM25 score
    float score = 0;
    for (int i = 0; i < iiters.size(); i++) {
        score += BM25_score(Posting(cur_did, freqs[i]), *iiters[i]);
    }
    // increase by 1, used for the next time
    cur_did++;
    return QueryScore(cur_did - 1, score);
}

QueryScore ORQueryProcessor::get_next() {
    throw std::exception();
}


cQueryResultArray* SingleQueryProcessor::query_result(uint32_t topK) {
    MinScoreHeap heap(QueryScore::greater);
    try {
        while (true) {
            auto qs = get_next();
            if (heap.size() >= topK) {
                if (QueryScore::greater(qs, heap.top())) {
                    heap.pop();
                    heap.emplace(qs);
                }
            }
            else heap.emplace(qs);
        }
    }
    // IndexEndInFile Exception
    catch (g::Exception) {}
    return heap_to_result(heap);
}

cQueryResultArray* ANDQueryProcessor::query_result(uint32_t topK) {
    MinScoreHeap heap(QueryScore::greater);
    try {
        while (true) {
            auto qs = get_next();
            if (heap.size() >= topK) {
                if (QueryScore::greater(qs, heap.top())) {
                    heap.pop();
                    heap.emplace(qs);
                }
            }
            else heap.emplace(qs);
        }
    }
    // IndexEndInFile Exception
    catch (g::Exception) {}
    return heap_to_result(heap);
}

cQueryResultArray* ORQueryProcessor::query_result(uint32_t topK) {
    MinScoreHeap heap(QueryScore::greater);
    for (auto processor : processors) {
        try {
            while (true) {
                auto p = processor->get_next();
                // add all scores into hashtable +=
                ORQueryProcessor::hashtable[p.doc_id] += p.score;
            }
        }
        catch (g::Exception) {}
    }
    // put into heap
    for (uint32_t did = 0; did < hashtable.size(); did++) {
        const auto score = hashtable[did];
        if (score == 0) continue;

        if (heap.size() >= topK) {
            if (score > heap.top().score) {
                heap.pop();
                heap.emplace(did, score);
            }
        }
        else heap.emplace(did, score);
        // clear to zero
        hashtable[did] = 0;
    }
    return heap_to_result(heap);
}


void SingleQueryProcessor::reset_index_state() {
    icache->reset_read_state_limit(*iiter);
}

void ANDQueryProcessor::reset_index_state() {
    for (auto& iiter : iiters) {
        icache->reset_read_state_limit(*iiter);
    }
}

void ORQueryProcessor::reset_index_state() {
    for (auto processor : processors) {
        processor->reset_index_state();
    }
}

ORQueryProcessor::~ORQueryProcessor() {
    for (auto processor : processors) {
        delete processor;
    }
}
