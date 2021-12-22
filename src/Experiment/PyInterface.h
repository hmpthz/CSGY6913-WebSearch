#pragma once
#include "../Implementation/IndexBuffer_Specialized.h"
#include "tasks.ipp"
#include <mimalloc-new-delete.h>

#include <Python.h>
#define PY_API extern "C" __declspec(dllexport)


PY_API void load_doctable(const char* filename);

#define FILENAMES_3 const char* lexicon_filename, const char* index_filename, const char* index_filename2
PY_API InputBuffer::Freq* get_inputbuf_Origin(uint32_t size_in_MB, const char* lexicon_filename, const char* index_filename);
PY_API InputBuffer::Score* get_inputbuf_UncompressedScore(uint32_t size_in_MB, FILENAMES_3);
PY_API InputBuffer::Linear<6>* get_inputbuf_linear6(uint32_t size_in_MB, FILENAMES_3);
PY_API InputBuffer::Linear<8>* get_inputbuf_linear8(uint32_t size_in_MB, FILENAMES_3);
PY_API InputBuffer::Linear<10>* get_inputbuf_linear10(uint32_t size_in_MB, FILENAMES_3);
PY_API InputBuffer::Linear<12>* get_inputbuf_linear12(uint32_t size_in_MB, FILENAMES_3);
PY_API InputBuffer::Log<6>* get_inputbuf_log6(uint32_t size_in_MB, FILENAMES_3);
PY_API InputBuffer::Log<8>* get_inputbuf_log8(uint32_t size_in_MB, FILENAMES_3);
PY_API InputBuffer::Log<10>* get_inputbuf_log10(uint32_t size_in_MB, FILENAMES_3);
PY_API InputBuffer::Log<12>* get_inputbuf_log12(uint32_t size_in_MB, FILENAMES_3);
PY_API InputBuffer::Adaptive<6, 3>* get_inputbuf_adaptive6(uint32_t size_in_MB, FILENAMES_3);
PY_API InputBuffer::Adaptive<8, 4>* get_inputbuf_adaptive8(uint32_t size_in_MB, FILENAMES_3);
PY_API InputBuffer::Adaptive<10, 4>* get_inputbuf_adaptive10(uint32_t size_in_MB, FILENAMES_3);
PY_API InputBuffer::Adaptive<12, 4>* get_inputbuf_adaptive12(uint32_t size_in_MB, FILENAMES_3);


PY_API OutputBuffer::Score* get_outputbuf_UncompressedScore(uint32_t size_in_MB, bool write_did, FILENAMES_3);
PY_API OutputBuffer::Linear<6>* get_outputbuf_linear6(uint32_t size_in_MB, bool write_did, FILENAMES_3);
PY_API OutputBuffer::Linear<8>* get_outputbuf_linear8(uint32_t size_in_MB, bool write_did, FILENAMES_3);
PY_API OutputBuffer::Linear<10>* get_outputbuf_linear10(uint32_t size_in_MB, bool write_did, FILENAMES_3);
PY_API OutputBuffer::Linear<12>* get_outputbuf_linear12(uint32_t size_in_MB, bool write_did, FILENAMES_3);
PY_API OutputBuffer::Log<6>* get_outputbuf_log6(uint32_t size_in_MB, bool write_did, FILENAMES_3);
PY_API OutputBuffer::Log<8>* get_outputbuf_log8(uint32_t size_in_MB, bool write_did, FILENAMES_3);
PY_API OutputBuffer::Log<10>* get_outputbuf_log10(uint32_t size_in_MB, bool write_did, FILENAMES_3);
PY_API OutputBuffer::Log<12>* get_outputbuf_log12(uint32_t size_in_MB, bool write_did, FILENAMES_3);
PY_API OutputBuffer::Adaptive<6, 3>* get_outputbuf_adaptive6(uint32_t size_in_MB, bool write_did, FILENAMES_3);
PY_API OutputBuffer::Adaptive<8, 4>* get_outputbuf_adaptive8(uint32_t size_in_MB, bool write_did, FILENAMES_3);
PY_API OutputBuffer::Adaptive<10, 4>* get_outputbuf_adaptive10(uint32_t size_in_MB, bool write_did, FILENAMES_3);
PY_API OutputBuffer::Adaptive<12, 4>* get_outputbuf_adaptive12(uint32_t size_in_MB, bool write_did, FILENAMES_3);


PY_API void precompute_to_UncompressedScore(InputBuffer::Freq* srcbuf, OutputBuffer::Score* dstbuf);
PY_API void quantize_linear6(InputBuffer::Score* srcbuf, OutputBuffer::Linear<6>* dstbuf);
PY_API void quantize_linear8(InputBuffer::Score* srcbuf, OutputBuffer::Linear<8>* dstbuf);
PY_API void quantize_linear10(InputBuffer::Score* srcbuf, OutputBuffer::Linear<10>* dstbuf);
PY_API void quantize_linear12(InputBuffer::Score* srcbuf, OutputBuffer::Linear<12>* dstbuf);
PY_API void quantize_log6(InputBuffer::Score* srcbuf, OutputBuffer::Log<6>* dstbuf);
PY_API void quantize_log8(InputBuffer::Score* srcbuf, OutputBuffer::Log<8>* dstbuf);
PY_API void quantize_log10(InputBuffer::Score* srcbuf, OutputBuffer::Log<10>* dstbuf);
PY_API void quantize_log12(InputBuffer::Score* srcbuf, OutputBuffer::Log<12>* dstbuf);
PY_API void quantize_adaptive6(InputBuffer::Score* srcbuf, OutputBuffer::Adaptive<6, 3>* dstbuf);
PY_API void quantize_adaptive8(InputBuffer::Score* srcbuf, OutputBuffer::Adaptive<8, 4>* dstbuf);
PY_API void quantize_adaptive10(InputBuffer::Score* srcbuf, OutputBuffer::Adaptive<10, 4>* dstbuf);
PY_API void quantize_adaptive12(InputBuffer::Score* srcbuf, OutputBuffer::Adaptive<12, 4>* dstbuf);


PY_API void test_Origin(InputBuffer::Freq* buf);
PY_API void test_UncompressedScore(InputBuffer::Score* buf);
PY_API void test_linear6(InputBuffer::Linear<6>* buf);
PY_API void test_linear8(InputBuffer::Linear<8>* buf);
PY_API void test_linear10(InputBuffer::Linear<10>* buf);
PY_API void test_linear12(InputBuffer::Linear<12>* buf);
PY_API void test_log6(InputBuffer::Log<6>* buf);
PY_API void test_log8(InputBuffer::Log<8>* buf);
PY_API void test_log10(InputBuffer::Log<10>* buf);
PY_API void test_log12(InputBuffer::Log<12>* buf);
PY_API void test_adaptive6(InputBuffer::Adaptive<6, 3>* buf);
PY_API void test_adaptive8(InputBuffer::Adaptive<8, 4>* buf);
PY_API void test_adaptive10(InputBuffer::Adaptive<10, 4>* buf);
PY_API void test_adaptive12(InputBuffer::Adaptive<12, 4>* buf);


PY_API void MSE_linear6(InputBuffer::Score* baseline, InputBuffer::Linear<6>* buf);
PY_API void MSE_linear8(InputBuffer::Score* baseline, InputBuffer::Linear<8>* buf);
PY_API void MSE_linear10(InputBuffer::Score* baseline, InputBuffer::Linear<10>* buf);
PY_API void MSE_linear12(InputBuffer::Score* baseline, InputBuffer::Linear<12>* buf);
PY_API void MSE_log6(InputBuffer::Score* baseline, InputBuffer::Log<6>* buf);
PY_API void MSE_log8(InputBuffer::Score* baseline, InputBuffer::Log<8>* buf);
PY_API void MSE_log10(InputBuffer::Score* baseline, InputBuffer::Log<10>* buf);
PY_API void MSE_log12(InputBuffer::Score* baseline, InputBuffer::Log<12>* buf);
PY_API void MSE_adaptive6(InputBuffer::Score* baseline, InputBuffer::Adaptive<6, 3>* buf);
PY_API void MSE_adaptive8(InputBuffer::Score* baseline, InputBuffer::Adaptive<8, 4>* buf);
PY_API void MSE_adaptive10(InputBuffer::Score* baseline, InputBuffer::Adaptive<10, 4>* buf);
PY_API void MSE_adaptive12(InputBuffer::Score* baseline, InputBuffer::Adaptive<12, 4>* buf);
