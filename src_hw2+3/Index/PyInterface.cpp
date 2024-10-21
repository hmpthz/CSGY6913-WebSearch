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

void add_doc_info(Builder* builder, uint64_t start_offset, uint32_t len, const char* url) {
    std::string _url = url;
    builder->doctable.append_doc(start_offset, len, _url);
}

bool doc_tokens_to_postings(Builder* builder, uint32_t doc_id, PyObject* token_list) {
    PostingsMap postings;
    Py_ssize_t len = PyList_Size(token_list);
    for (Py_ssize_t i = 0; i < len; i++) {
        auto token = PyList_GetItem(token_list, i);
        postings[PyBytes_AsString(token)]++;
    }
    builder->postingbuf.add_postings(doc_id, postings);
    return builder->postingbuf.is_full();
}

void postings_to_index(Builder* builder) {
    IndexBufferToMerge::transfer_from_postings(builder->postingbuf, builder->outputbuf);
    // write remaining data
    builder->outputbuf.o_write_all();
    // clear
    builder->postingbuf.clear();
    builder->outputbuf.erase_all();
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

void merge(Builder* builder) {
    // read input files
    for (auto& inputbuf : builder->inputbufs) {
        inputbuf.i_read_fill();
    }
    // merge loop
    try {
        while (true) {
            auto& inputbuf = IndexBufferToMerge::min_front(builder->inputbufs);
            IndexBufferToMerge::transfer_front_to_back(inputbuf, builder->outputbuf);
            inputbuf.i_erase_front();
            if (inputbuf.is_empty()) {
                inputbuf.i_read_fill();
            }
        }
    }
    // AllInputBuffersExhausted Exception
    catch (g::Exception) { }
    // write remaining data
    builder->outputbuf.o_write_all();
    // clear and close
    for (auto& inputbuf : builder->inputbufs) {
        inputbuf.erase_all();
        inputbuf.close_fin();
        inputbuf.lex.close_fin();
    }
    builder->outputbuf.erase_all();
    builder->outputbuf.close_fout();
    builder->outputbuf.lex.close_fout();
}

void test() {
    IndexBufferToMerge inputbuf;
    IndexBufferToMerge outputbuf;
    inputbuf.set_capacity(100 * g::MB);
    outputbuf.set_capacity(50 * g::MB);

    inputbuf.open_fin("WebSearch\\out\\index0_0.bin");
    inputbuf.lex.open_fin("WebSearch\\out\\lexicon0_0.bin");
    outputbuf.open_fout("WebSearch\\out\\testindex.bin");
    outputbuf.lex.open_fout("WebSearch\\out\\testlexicon.bin");

    while (true) {
        if (inputbuf.is_empty()) {
            inputbuf.i_read_fill();
            // still empty, so the input is over
            if (inputbuf.is_empty()) break;
        }

        std::cout << "remaining # of index: " << inputbuf.size() << '\n';
        IndexBufferToMerge::transfer_front_to_back(inputbuf, outputbuf);
        inputbuf.i_erase_front();
    }
    outputbuf.o_write_all();
}

