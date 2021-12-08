#include "PyInterface.h"


void load_doctable(const char* filename) {
    BM25::docs = new DocTable;
    BM25::docs->open_fin(filename);
    BM25::docs->read_all();
}


InputBuffer::Freq* get_inputbuf_Origin(uint32_t size_in_MB, const char* lexicon_filename, const char* index_filename) {
    auto buf = new InputBuffer::Freq;
    buf->set_capacity((size_t)size_in_MB * g::MB);
    buf->lex.open_fin(lexicon_filename);
    buf->open_fin(index_filename);
    return buf;
}

#define GET_INPUTBUF(Buf_t) \
    auto buf = new Buf_t; \
    buf->set_capacity((size_t)size_in_MB* g::MB); \
    buf->lex.open_fin(lexicon_filename); \
    buf->open_fin(index_filename, index_filename2); \
    return buf

#define GET_OUTPUTBUF(Buf_t) \
    auto buf = new Buf_t; \
    buf->set_capacity((size_t)size_in_MB* g::MB); \
    buf->lex.open_fout(lexicon_filename); \
    buf->write_did = write_did; \
    buf->open_fout(index_filename, index_filename2); \
    return buf

InputBuffer::Score* get_inputbuf_UncompressedScore(uint32_t size_in_MB, FILENAMES_3) {
    GET_INPUTBUF(InputBuffer::Score);
}


OutputBuffer::Score* get_outputbuf_UncompressedScore(uint32_t size_in_MB, bool write_did, FILENAMES_3) {
    GET_OUTPUTBUF(OutputBuffer::Score);
}


void precompute_to_UncompressedScore(InputBuffer::Freq* srcbuf, OutputBuffer::Score* dstbuf) {
    Transfer::all_data(*srcbuf, *dstbuf);
}
