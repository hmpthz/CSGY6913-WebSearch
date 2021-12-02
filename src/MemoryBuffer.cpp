#include "MemoryBuffer.h"


#ifdef __cpp_aligned_new
void* MemoryCounter::do_allocate(size_t bytes, size_t alignment) {
    byte_size += bytes;

    if (alignment > __STDCPP_DEFAULT_NEW_ALIGNMENT__) {
        return ::operator new (bytes, std::align_val_t{ alignment });
    }
    return ::operator new(bytes);
}
void MemoryCounter::do_deallocate(void* p, size_t bytes, size_t alignment) {
    byte_size -= bytes;

    if (alignment > __STDCPP_DEFAULT_NEW_ALIGNMENT__) {
        return ::operator delete (p, bytes, std::align_val_t{ alignment });
    }
    ::operator delete(p, bytes);
}

#else
void* MemoryCounter::do_allocate(size_t bytes, size_t alignment) {
    byte_size += bytes;

    if (alignment > __STDCPP_DEFAULT_NEW_ALIGNMENT__) {
        _Xbad_alloc();
    }
    return ::operator new(bytes);
}
void MemoryCounter::do_deallocate(void* p, size_t bytes, size_t alignment) {
    byte_size -= bytes;

    ::operator delete(p, bytes);
}

#endif

bool MemoryCounter::do_is_equal(const std::pmr::memory_resource& other) const noexcept {
    return this == &other;
}


void PostingsBuffer::add_postings(uint32_t doc_id, const Posting::Map& postings) {
    for (auto& [term, frequency] : postings) {
        // use PMR for counting memory!
        auto [iter, is_new] = terms.try_emplace(term, Posting::Vec(&memcnt));
        auto& posting_vec = g::ival(iter);
        posting_vec.emplace_back(doc_id, frequency);
    }
}

void PostingsBuffer::clear() {
    terms.clear();
}
