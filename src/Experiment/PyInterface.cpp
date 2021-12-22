#include "PyInterface.h"


#define PRINT_FUNCNAME() std::cout<<__func__<<'\n'

void load_doctable(const char* filename) {
    PRINT_FUNCNAME();
    BM25::docs = new DocTable;
    BM25::docs->open_fin(filename);
    BM25::docs->read_all();
}


InputBuffer::Freq* get_inputbuf_Origin(uint32_t size_in_MB, const char* lexicon_filename, const char* index_filename) {
    PRINT_FUNCNAME();
    auto buf = new InputBuffer::Freq;
    buf->set_capacity((size_t)size_in_MB * g::MB);
    buf->lex.open_fin(lexicon_filename);
    buf->open_fin(index_filename);
    return buf;
}

#define COMMA ,
#define GET_INPUTBUF(Buf_t) \
    auto buf = new Buf_t; \
    buf->set_capacity((size_t)size_in_MB* g::MB); \
    buf->lex.open_fin(lexicon_filename); \
    buf->open_fin(index_filename, index_filename2); \
    return buf

#define GET_INPUTBUF_READQUANT(Buf_t) \
    auto buf = new Buf_t; \
    buf->set_capacity((size_t)size_in_MB* g::MB); \
    buf->lex.open_fin(lexicon_filename); \
    buf->open_fin(index_filename, index_filename2); \
    buf->read_quantizer(); \
    return buf

#define GET_OUTPUTBUF(Buf_t) \
    auto buf = new Buf_t; \
    buf->set_capacity((size_t)size_in_MB* g::MB); \
    buf->lex.open_fout(lexicon_filename); \
    buf->write_did = write_did; \
    buf->open_fout(index_filename, index_filename2); \
    return buf

InputBuffer::Score* get_inputbuf_UncompressedScore(uint32_t size_in_MB, FILENAMES_3) {
    PRINT_FUNCNAME(); GET_INPUTBUF(InputBuffer::Score);
}
InputBuffer::Linear<6>* get_inputbuf_linear6(uint32_t size_in_MB, FILENAMES_3) {
    PRINT_FUNCNAME(); GET_INPUTBUF_READQUANT(InputBuffer::Linear<6>);
}
InputBuffer::Linear<8>* get_inputbuf_linear8(uint32_t size_in_MB, FILENAMES_3) {
    PRINT_FUNCNAME(); GET_INPUTBUF_READQUANT(InputBuffer::Linear<8>);
}
InputBuffer::Linear<10>* get_inputbuf_linear10(uint32_t size_in_MB, FILENAMES_3) {
    PRINT_FUNCNAME(); GET_INPUTBUF_READQUANT(InputBuffer::Linear<10>);
}
InputBuffer::Linear<12>* get_inputbuf_linear12(uint32_t size_in_MB, FILENAMES_3) {
    PRINT_FUNCNAME(); GET_INPUTBUF_READQUANT(InputBuffer::Linear<12>);
}
InputBuffer::Log<6>* get_inputbuf_log6(uint32_t size_in_MB, FILENAMES_3) {
    PRINT_FUNCNAME(); GET_INPUTBUF_READQUANT(InputBuffer::Log<6>);
}
InputBuffer::Log<8>* get_inputbuf_log8(uint32_t size_in_MB, FILENAMES_3) {
    PRINT_FUNCNAME(); GET_INPUTBUF_READQUANT(InputBuffer::Log<8>);
}
InputBuffer::Log<10>* get_inputbuf_log10(uint32_t size_in_MB, FILENAMES_3) {
    PRINT_FUNCNAME(); GET_INPUTBUF_READQUANT(InputBuffer::Log<10>);
}
InputBuffer::Log<12>* get_inputbuf_log12(uint32_t size_in_MB, FILENAMES_3) {
    PRINT_FUNCNAME(); GET_INPUTBUF_READQUANT(InputBuffer::Log<12>);
}
InputBuffer::Adaptive<6, 3>* get_inputbuf_adaptive6(uint32_t size_in_MB, FILENAMES_3) {
    PRINT_FUNCNAME(); GET_INPUTBUF(InputBuffer::Adaptive<6 COMMA 3>);
}
InputBuffer::Adaptive<8, 4>* get_inputbuf_adaptive8(uint32_t size_in_MB, FILENAMES_3) {
    PRINT_FUNCNAME(); GET_INPUTBUF(InputBuffer::Adaptive<8 COMMA 4>);
}
InputBuffer::Adaptive<10, 4>* get_inputbuf_adaptive10(uint32_t size_in_MB, FILENAMES_3) {
    PRINT_FUNCNAME(); GET_INPUTBUF(InputBuffer::Adaptive<10 COMMA 4>);
}
InputBuffer::Adaptive<12, 4>* get_inputbuf_adaptive12(uint32_t size_in_MB, FILENAMES_3) {
    PRINT_FUNCNAME(); GET_INPUTBUF(InputBuffer::Adaptive<12 COMMA 4>);
}


OutputBuffer::Score* get_outputbuf_UncompressedScore(uint32_t size_in_MB, bool write_did, FILENAMES_3) {
    PRINT_FUNCNAME(); GET_OUTPUTBUF(OutputBuffer::Score);
}
OutputBuffer::Linear<6>* get_outputbuf_linear6(uint32_t size_in_MB, bool write_did, FILENAMES_3) {
    PRINT_FUNCNAME(); GET_OUTPUTBUF(OutputBuffer::Linear<6>);
}
OutputBuffer::Linear<8>* get_outputbuf_linear8(uint32_t size_in_MB, bool write_did, FILENAMES_3) {
    PRINT_FUNCNAME(); GET_OUTPUTBUF(OutputBuffer::Linear<8>);
}
OutputBuffer::Linear<10>* get_outputbuf_linear10(uint32_t size_in_MB, bool write_did, FILENAMES_3) {
    PRINT_FUNCNAME(); GET_OUTPUTBUF(OutputBuffer::Linear<10>);
}
OutputBuffer::Linear<12>* get_outputbuf_linear12(uint32_t size_in_MB, bool write_did, FILENAMES_3) {
    PRINT_FUNCNAME(); GET_OUTPUTBUF(OutputBuffer::Linear<12>);
}
OutputBuffer::Log<6>* get_outputbuf_log6(uint32_t size_in_MB, bool write_did, FILENAMES_3) {
    PRINT_FUNCNAME(); GET_OUTPUTBUF(OutputBuffer::Log<6>);
}
OutputBuffer::Log<8>* get_outputbuf_log8(uint32_t size_in_MB, bool write_did, FILENAMES_3) {
    PRINT_FUNCNAME(); GET_OUTPUTBUF(OutputBuffer::Log<8>);
}
OutputBuffer::Log<10>* get_outputbuf_log10(uint32_t size_in_MB, bool write_did, FILENAMES_3) {
    PRINT_FUNCNAME(); GET_OUTPUTBUF(OutputBuffer::Log<10>);
}
OutputBuffer::Log<12>* get_outputbuf_log12(uint32_t size_in_MB, bool write_did, FILENAMES_3) {
    PRINT_FUNCNAME(); GET_OUTPUTBUF(OutputBuffer::Log<12>);
}
OutputBuffer::Adaptive<6, 3>* get_outputbuf_adaptive6(uint32_t size_in_MB, bool write_did, FILENAMES_3) {
    PRINT_FUNCNAME(); GET_OUTPUTBUF(OutputBuffer::Adaptive<6 COMMA 3>);
}
OutputBuffer::Adaptive<8, 4>* get_outputbuf_adaptive8(uint32_t size_in_MB, bool write_did, FILENAMES_3) {
    PRINT_FUNCNAME(); GET_OUTPUTBUF(OutputBuffer::Adaptive<8 COMMA 4>);
}
OutputBuffer::Adaptive<10, 4>* get_outputbuf_adaptive10(uint32_t size_in_MB, bool write_did, FILENAMES_3) {
    PRINT_FUNCNAME(); GET_OUTPUTBUF(OutputBuffer::Adaptive<10 COMMA 4>);
}
OutputBuffer::Adaptive<12, 4>* get_outputbuf_adaptive12(uint32_t size_in_MB, bool write_did, FILENAMES_3) {
    PRINT_FUNCNAME(); GET_OUTPUTBUF(OutputBuffer::Adaptive<12 COMMA 4>);
}


void precompute_to_UncompressedScore(InputBuffer::Freq* srcbuf, OutputBuffer::Score* dstbuf) {
    PRINT_FUNCNAME(); Transfer::all_data(*srcbuf, *dstbuf);
}
void quantize_linear6(InputBuffer::Score* srcbuf, OutputBuffer::Linear<6>* dstbuf) {
    PRINT_FUNCNAME(); Task::quantize_linear(*srcbuf, *dstbuf);
}
void quantize_linear8(InputBuffer::Score* srcbuf, OutputBuffer::Linear<8>* dstbuf) {
    PRINT_FUNCNAME(); Task::quantize_linear(*srcbuf, *dstbuf);
}
void quantize_linear10(InputBuffer::Score* srcbuf, OutputBuffer::Linear<10>* dstbuf) {
    PRINT_FUNCNAME(); Task::quantize_linear(*srcbuf, *dstbuf);
}
void quantize_linear12(InputBuffer::Score* srcbuf, OutputBuffer::Linear<12>* dstbuf) {
    PRINT_FUNCNAME(); Task::quantize_linear(*srcbuf, *dstbuf);
}
void quantize_log6(InputBuffer::Score* srcbuf, OutputBuffer::Log<6>* dstbuf) {
    PRINT_FUNCNAME(); Task::quantize_log(*srcbuf, *dstbuf);
}
void quantize_log8(InputBuffer::Score* srcbuf, OutputBuffer::Log<8>* dstbuf) {
    PRINT_FUNCNAME(); Task::quantize_log(*srcbuf, *dstbuf);
}
void quantize_log10(InputBuffer::Score* srcbuf, OutputBuffer::Log<10>* dstbuf) {
    PRINT_FUNCNAME(); Task::quantize_log(*srcbuf, *dstbuf);
}
void quantize_log12(InputBuffer::Score* srcbuf, OutputBuffer::Log<12>* dstbuf) {
    PRINT_FUNCNAME(); Task::quantize_log(*srcbuf, *dstbuf);
}
void quantize_adaptive6(InputBuffer::Score* srcbuf, OutputBuffer::Adaptive<6, 3>* dstbuf) {
    PRINT_FUNCNAME(); Task::quantize_adaptive(*srcbuf, *dstbuf);
}
void quantize_adaptive8(InputBuffer::Score* srcbuf, OutputBuffer::Adaptive<8, 4>* dstbuf) {
    PRINT_FUNCNAME(); Task::quantize_adaptive(*srcbuf, *dstbuf);
}
void quantize_adaptive10(InputBuffer::Score* srcbuf, OutputBuffer::Adaptive<10, 4>* dstbuf) {
    PRINT_FUNCNAME(); Task::quantize_adaptive(*srcbuf, *dstbuf);
}
void quantize_adaptive12(InputBuffer::Score* srcbuf, OutputBuffer::Adaptive<12, 4>* dstbuf) {
    PRINT_FUNCNAME(); Task::quantize_adaptive(*srcbuf, *dstbuf);
}


void test_Origin(InputBuffer::Freq* buf) {
    PRINT_FUNCNAME(); Task::test_scores(*buf);
}
void test_UncompressedScore(InputBuffer::Score* buf) {
    PRINT_FUNCNAME(); Task::test_scores(*buf);
}
void test_linear6(InputBuffer::Linear<6>* buf) {
    PRINT_FUNCNAME(); Task::test_scores(*buf);
}
void test_linear8(InputBuffer::Linear<8>* buf) {
    PRINT_FUNCNAME(); Task::test_scores(*buf);
}
void test_linear10(InputBuffer::Linear<10>* buf) {
    PRINT_FUNCNAME(); Task::test_scores(*buf);
}
void test_linear12(InputBuffer::Linear<12>* buf) {
    PRINT_FUNCNAME(); Task::test_scores(*buf);
}
void test_log6(InputBuffer::Log<6>* buf) {
    PRINT_FUNCNAME(); Task::test_scores(*buf);
}
void test_log8(InputBuffer::Log<8>* buf) {
    PRINT_FUNCNAME(); Task::test_scores(*buf);
}
void test_log10(InputBuffer::Log<10>* buf) {
    PRINT_FUNCNAME(); Task::test_scores(*buf);
}
void test_log12(InputBuffer::Log<12>* buf) {
    PRINT_FUNCNAME(); Task::test_scores(*buf);
}
void test_adaptive6(InputBuffer::Adaptive<6, 3>* buf) {
    PRINT_FUNCNAME(); Task::test_scores(*buf);
}
void test_adaptive8(InputBuffer::Adaptive<8, 4>* buf) {
    PRINT_FUNCNAME(); Task::test_scores(*buf);
}
void test_adaptive10(InputBuffer::Adaptive<10, 4>* buf) {
    PRINT_FUNCNAME(); Task::test_scores(*buf);
}
void test_adaptive12(InputBuffer::Adaptive<12, 4>* buf) {
    PRINT_FUNCNAME(); Task::test_scores(*buf);
}


void MSE_linear6(InputBuffer::Score* baseline, InputBuffer::Linear<6>* buf) {
    PRINT_FUNCNAME(); Task::MSE(*baseline, *buf);
}
void MSE_linear8(InputBuffer::Score* baseline, InputBuffer::Linear<8>* buf) {
    PRINT_FUNCNAME(); Task::MSE(*baseline, *buf);
}
void MSE_linear10(InputBuffer::Score* baseline, InputBuffer::Linear<10>* buf) {
    PRINT_FUNCNAME(); Task::MSE(*baseline, *buf);
}
void MSE_linear12(InputBuffer::Score* baseline, InputBuffer::Linear<12>* buf) {
    PRINT_FUNCNAME(); Task::MSE(*baseline, *buf);
}
void MSE_log6(InputBuffer::Score* baseline, InputBuffer::Log<6>* buf) {
    PRINT_FUNCNAME(); Task::MSE(*baseline, *buf);
}
void MSE_log8(InputBuffer::Score* baseline, InputBuffer::Log<8>* buf) {
    PRINT_FUNCNAME(); Task::MSE(*baseline, *buf);
}
void MSE_log10(InputBuffer::Score* baseline, InputBuffer::Log<10>* buf) {
    PRINT_FUNCNAME(); Task::MSE(*baseline, *buf);
}
void MSE_log12(InputBuffer::Score* baseline, InputBuffer::Log<12>* buf) {
    PRINT_FUNCNAME(); Task::MSE(*baseline, *buf);
}
void MSE_adaptive6(InputBuffer::Score* baseline, InputBuffer::Adaptive<6, 3>* buf) {
    PRINT_FUNCNAME(); Task::MSE(*baseline, *buf);
}
void MSE_adaptive8(InputBuffer::Score* baseline, InputBuffer::Adaptive<8, 4>* buf) {
    PRINT_FUNCNAME(); Task::MSE(*baseline, *buf);
}
void MSE_adaptive10(InputBuffer::Score* baseline, InputBuffer::Adaptive<10, 4>* buf) {
    PRINT_FUNCNAME(); Task::MSE(*baseline, *buf);
}
void MSE_adaptive12(InputBuffer::Score* baseline, InputBuffer::Adaptive<12, 4>* buf) {
    PRINT_FUNCNAME(); Task::MSE(*baseline, *buf);
}
