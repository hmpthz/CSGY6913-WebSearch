#pragma once
#include "global.h"
#include <memory_resource>
#include <list>


/*
    a memory counter for PMR containers
    it simply hook the (de)allocate to get the accurate memory usage
*/
class MemoryCounter :public std::pmr::memory_resource {
protected:
    size_t byte_size;
    size_t byte_capacity;

    void* do_allocate(size_t bytes, size_t alignment);
    void do_deallocate(void* p, size_t bytes, size_t alignment);
    bool do_is_equal(const std::pmr::memory_resource& other) const noexcept;

public:
    MemoryCounter() :
        byte_size(0), byte_capacity(0) {}

    inline size_t size() { return byte_size; }
    inline size_t capacity() { return byte_capacity; }
    inline void set_capacity(size_t sz) {
        byte_capacity = sz;
    }
    inline bool is_full() {
        return byte_size >= byte_capacity;
    }
    inline bool is_full(size_t sz) {
        return byte_size + sz >= byte_capacity;
    }
};


/*
    container(s) in buffer would be PMR and pass MemoryCounter while constructing
    so the buffer may limit memory capacity
    this design makes NO SENSE in real practice, just a try for using PMR.
*/
class BufferBase {
protected:
    MemoryCounter memcnt;

public:
    inline void set_capacity(size_t sz) { memcnt.set_capacity(sz); }
    inline bool is_full() { return memcnt.is_full(); }
    inline bool is_full(size_t sz) { return memcnt.is_full(sz); }
    inline float capacity_percent() {
        return ((100.0f * memcnt.size()) / memcnt.capacity());
    }
};



class PostingsBuffer :public BufferBase {
protected:
    using Map = std::pmr::map<std::string, Posting::Vec>;
    Map terms;

public:
    PostingsBuffer() :terms(&memcnt) {}
    /* used for transfer */
    friend class OutputBufferToMerge;

    inline bool is_empty() {
        return terms.empty();
    }

    void add_postings(uint32_t doc_id, const Posting::Map& postings);
    void clear();
};
