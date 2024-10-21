#pragma once
#include "helper.h"
#include "DocTable.h"
#include "IndexBuffer.h"
#include <Python.h>

#define PY_API extern "C" __declspec(dllexport)


class Builder {
public:
    DocTable doctable;
    PostingsBuffer postingbuf;
    // n-way merge, multiple input buffers
    std::vector<IndexBufferToMerge> inputbufs;
    // one output buffer
    IndexBufferToMerge outputbuf;
};


PY_API Builder* get_Builder();
PY_API void delete_Builder(Builder* builder);

PY_API void set_postingbuf_size(Builder* builder, uint32_t size_in_MB);
PY_API void set_inputbufs_size(Builder* builder, uint32_t n_way, uint32_t size_in_MB);
PY_API void set_outputbuf_size(Builder* builder, uint32_t size_in_MB);
PY_API void open_output_doctable_file(Builder* builder, const char* filename);
PY_API void open_inputbufs_file(Builder* builder, PyObject* index_filename_list, PyObject* lexicon_filename_list);
PY_API void open_outputbuf_file(Builder* builder, const char* index_filename, const char* lexicon_filename);

PY_API void add_doc_info(Builder* builder, uint64_t start_offset, uint32_t len, const char* url);
/* use python to parse and tokenize documents 
convert python tokens list to PostingsMap, and add into PostingsBuffer
if the buffer is full, return true */
PY_API bool doc_tokens_to_postings(Builder* builder, uint32_t doc_id, PyObject* token_list);
/* transfer full PostingsBuffer to IndexBuffer, write to opened output file */
PY_API void postings_to_index(Builder* builder);
PY_API void write_doctable(Builder* builder);
PY_API bool is_postingsbuf_empty(Builder* builder);
/* set filenames, read, merge and write.
merge currently opened input files, write to opened output file.
guarantee that for each term, the prior IndexBuffer has lower doc_id, so merge can be simple. */
PY_API void merge(Builder* builder);

PY_API void test();