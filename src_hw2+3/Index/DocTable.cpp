#include "DocTable.h"
#include <filesystem>
namespace fs = std::filesystem;


DocTable::DocTable() {
    clear(false);
}

void DocTable::append_doc(uint64_t offset, uint32_t len, std::string& s) {
    total_len += len;
    table.emplace_back(offset, len, std::move(s));
}

void DocTable::compute_avg_len() {
    avg_len = total_len / (float)table.size();
}

void DocTable::open_fin(const char* filename) {
    fin.open(fs::path(filename), ios::binary);
}

void DocTable::open_fout(const char* filename) {
    fout.open(fs::path(filename), ios::binary);
}

void DocTable::clear(bool remain_statistics) {
    if (!remain_statistics) {
        total_len = 0;
        avg_len = 0;
    }
    table.clear();
}

#ifdef TEXT_FILE
/* I/O File in text format */

void DocTable::read_all() {
    fin >> avg_len;
    while (true) {
        uint64_t offset;
        uint32_t len;
        std::string s;

        fin >> offset >> len >> s;
        if (fin.tellg() != -1) {
            append_doc(offset, len, s);
        }
        else break;
    }
}

void DocTable::write_all() {
    fout << avg_len << '\n';
    for (auto& item : table) {
        fout << item.start_offset << ' ' << item.len << ' ' << item.url << '\n';
    }
}

#else
/* I/O File in binary format */

void DocTable::read_all() {
    std::istreambuf_iterator<char> ifiter(fin);
    std::istreambuf_iterator<char> ifiter_end;

    fin.read((char*)&avg_len, sizeof(avg_len));
    while (true) {
        uint64_t offset;
        uint32_t len;
        std::string s;

        fin.read((char*)&offset, sizeof(offset));
        fin.read((char*)&len, sizeof(len));
        g::copy_until_zero(ifiter, ifiter_end, std::back_inserter(s));

        if (fin.tellg() != -1) {
            append_doc(offset, len, s);
        }
        else break;
    }
}

void DocTable::write_all() {
    fout.write((char*)&avg_len, sizeof(avg_len));
    for (auto& item : table) {
        fout.write((char*)&item.start_offset, sizeof(item.start_offset));
        fout.write((char*)&item.len, sizeof(item.len));
        fout << item.url << '\0';
    }
}

#endif
