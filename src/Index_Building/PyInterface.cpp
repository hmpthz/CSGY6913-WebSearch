#include "PyInterface.h"


Builder* get_Builder() {
    return new Builder;
}

void delete_Builder(Builder* builder) {
    delete builder;
}

void set_postingbuf_size(Builder* builder, uint32_t size_in_MB) {
    size_t size_in_B = (size_t)size_in_MB * g::MB;
    builder->postingbuf.set_capacity(size_in_B);
}

void set_inputbufs_size(Builder* builder, uint32_t n_way, uint32_t size_in_MB) {
    size_t size_in_B = ((double)size_in_MB / n_way) * g::MB;
    builder->inputbufs.resize(n_way);
    for (auto& inputbuf : builder->inputbufs) {
        inputbuf.set_capacity(size_in_B);
    }
}

void set_outputbuf_size(Builder* builder, uint32_t size_in_MB) {
    size_t size_in_B = (size_t)size_in_MB * g::MB;
    builder->outputbuf.set_capacity(size_in_B);
}

void open_output_doctable_file(Builder* builder, const char* filename) {
    builder->doctable.open_fout(filename);
}

void open_inputbufs_file(Builder* builder, PyObject* index_filename_list, PyObject* lexicon_filename_list) {
    Py_ssize_t n_files = PyList_Size(index_filename_list);
    for (Py_ssize_t i = 0; i < n_files; i++) {
        auto index_filename = PyList_GetItem(index_filename_list, i);
        auto lexicon_filename = PyList_GetItem(lexicon_filename_list, i);
        builder->inputbufs[i].open_fin(PyBytes_AsString(index_filename));
        builder->inputbufs[i].lex.open_fin(PyBytes_AsString(lexicon_filename));
    }
}

void open_outputbuf_file(Builder* builder, const char* index_filename, const char* lexicon_filename) {
    builder->outputbuf.open_fout(index_filename);
    builder->outputbuf.lex.open_fout(lexicon_filename);
}

float postingbuf_capacity_percent(Builder* builder) {
    return builder->postingbuf.capacity_percent();
}

void add_doc_info(Builder* builder, uint64_t start_offset, uint32_t len, const char* url) {
    std::string _url = url;
    builder->doctable.append_doc(start_offset, len, _url);
}

bool doc_tokens_to_postings(Builder* builder, uint32_t doc_id, PyObject* token_list) {
    Posting::Map postings;
    Py_ssize_t len = PyList_Size(token_list);
    for (Py_ssize_t i = 0; i < len; i++) {
        auto token = PyList_GetItem(token_list, i);
        postings[PyBytes_AsString(token)]++;
    }
    builder->postingbuf.add_postings(doc_id, postings);
    return builder->postingbuf.is_full();
}

void postings_to_index(Builder* builder) {
    builder->outputbuf.transfer_from_postings(builder->postingbuf);
    // write remaining data
    builder->outputbuf.write_all();
    // clear
    builder->postingbuf.clear();
    builder->outputbuf.clear(true);
    // close current output file
    builder->outputbuf.close_fout();
    builder->outputbuf.lex.close_fout();
}

void write_doctable(Builder* builder) {
    builder->doctable.compute_avg_len();
    builder->doctable.write_all();
}

bool is_postingsbuf_empty(Builder* builder) {
    return builder->postingbuf.is_empty();
}

void merge(Builder* builder, uint32_t min_docs) {
    builder->outputbuf.min_docs = min_docs;

    // read input files
    for (auto& inputbuf : builder->inputbufs) {
        inputbuf.read_fill();
    }
    // merge loop
    while (true) {
        int min_i = InputBufferToMerge::min_front(builder->inputbufs);
        if (min_i < 0) break; // merge end

        auto& inputbuf = builder->inputbufs[min_i];
        // front and back are not the same term
        if (builder->outputbuf.is_empty() || g::ikey(inputbuf.front().info) != g::ikey(builder->outputbuf.back().info)) {
            auto info = inputbuf.front().info;
            builder->outputbuf.append_empty_index(g::ikey(info));
        }

        Transfer::front_to_back<true>(inputbuf, builder->outputbuf);

        inputbuf.erase_front();
        if (inputbuf.is_empty()) {
            inputbuf.read_fill();
        }
    }
    // write remaining data
    builder->outputbuf.write_all();
    // clear and close
    for (auto& inputbuf : builder->inputbufs) {
        inputbuf.clear(true);
        inputbuf.close_fin();
        inputbuf.lex.close_fin();
    }
    builder->outputbuf.clear(true);
    builder->outputbuf.close_fout();
    builder->outputbuf.lex.close_fout();
}
