#pragma once
#include "global.h"


struct DocTableItem {
    uint64_t start_off; /* start offset of doc in file */
    uint32_t len; /* # of terms */
    std::string url;
    template<typename _string>
    DocTableItem(uint64_t a, uint32_t b, _string&& c) :
        start_off(a), len(b), url(std::forward<_string>(c)) {}
};

/*
    doc_id is the order the pages are parsed
    will keep the whole table in memory
*/
class DocTable {
protected:
    uint64_t total_len; /* total # of terms in collection */
    float avg_len; /* average number of terms in each document */
    std::vector<DocTableItem> table;
    std::ifstream fin;
    std::ofstream fout;

public:
    DocTable();
    inline size_t size() {
        return table.size();
    }
    inline float get_avg_len() {
        return avg_len;
    }
    inline DocTableItem& get_item(uint32_t doc_id) {
        return table[doc_id];
    }

    /* NOTE!! always move url string !! */
    void append_doc(uint64_t offset, uint32_t len, std::string& s);
    void compute_avg_len();

    /*
        text format:
        <avg_len> (\n)
        <start offset> (space) <# of terms> (space) <url> (\n)
        binary format:
        <4-avg_len>
        <8-start offset> <4-# of terms> <str-url> <1-\0>
    */

    /* read all documents from DocTable file */
    void read_all();
    /* write all data from memory to the disk, include the first avg_len */
    void write_all();
    void clear(bool remain_statistics);

    void open_fin(const char* filename) {
        fin.open(filename, std::ios::binary);
    }
    void open_fout(const char* filename) {
        fout.open(filename, std::ios::binary);
    }
    void close_fin() {
        if (fin.is_open()) fin.close();
    }
    void close_fout() {
        if (fout.is_open()) fout.close();
    }
};
