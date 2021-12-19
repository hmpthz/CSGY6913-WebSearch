#pragma once
#include "../Implementation/IndexBuffer_Specialized.h"
#include "tasks.ipp"
//#include <mimalloc-new-delete.h>

#include <Python.h>
#define PY_API extern "C" __declspec(dllexport)


PY_API void load_doctable(const char* filename);

#define FILENAMES_3 const char* lexicon_filename, const char* index_filename, const char* index_filename2
PY_API InputBuffer::Freq* get_inputbuf_Origin(uint32_t size_in_MB, const char* lexicon_filename, const char* index_filename);
PY_API InputBuffer::Score* get_inputbuf_UncompressedScore(uint32_t size_in_MB, FILENAMES_3);


PY_API OutputBuffer::Score* get_outputbuf_UncompressedScore(uint32_t size_in_MB, bool write_did, FILENAMES_3);


PY_API void precompute_to_UncompressedScore(InputBuffer::Freq* srcbuf, OutputBuffer::Score* dstbuf);
