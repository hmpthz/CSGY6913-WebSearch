#pragma once
#include "global.h"


/*
    use map to maintain alphabetical order of terms.
    iterator of the map would be stored in TermIndex, so it can get the needed info.
*/
template<typename TermInfo_t, typename Derived>
class Lexicon {
protected:
    using Map = std::map<std::string, TermInfo_t>;
    Map terms;
    std::ifstream fin;
    std::ofstream fout;

public:
    using Iter = typename Map::iterator;

    inline size_t size() {
        return terms.size();
    }
    inline void erase(typename Iter iter) {
        terms.erase(iter);
    }

    /* throw TermNotFound */
    typename Iter get_iter(const std::string& term);

    typename Iter add_term(const std::string& term, TermInfo_t& value);
    /* if guarantee terms are in ascending order, append after the last item.
    get rid of search time, improve performance of map insertion */
    typename Iter append_term(const std::string& term, TermInfo_t& value);

    /* read next term from Lexicon file based on current fin pointer
    [return]
    iterator of the loaded term. throw EndInFile. */
    typename Iter read_next(); // <Derived>
    /* write term to disk, append file based on current fout pointer*/
    void write(typename Iter iter); // <Derived>
    void read_all();
    void clear();

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


template<typename T, typename D>
inline typename Lexicon<T, D>::Iter Lexicon<T, D>::get_iter(const std::string& term) {
    auto iter = terms.find(term);
    if (iter != terms.end()) {
        return iter;
    }
    else {
        throw g::Exception::TermNotFound;
    }
}

template<typename TermInfo_t, typename D>
inline typename Lexicon<TermInfo_t, D>::Iter Lexicon<TermInfo_t, D>::add_term(const std::string& term, TermInfo_t& value) {
    auto [iter, is_new] = terms.try_emplace(term, value);
    return iter;
}

template<typename TermInfo_t, typename D>
inline typename Lexicon<TermInfo_t, D>::Iter Lexicon<TermInfo_t, D>::append_term(const std::string& term, TermInfo_t& value) {
    if (terms.size() > 0) {
        // use hint
        return terms.try_emplace(std::prev(terms.end()), term, value);
    }
    else return add_term(term, value);
}

template<typename T, typename D>
inline void Lexicon<T, D>::read_all() {
    try {
        while (true) {
            static_cast<D*>(this)->read_next(); // static polymorphism
        }
    }
    // EndInFile Exception
    catch (g::Exception) {}
}

template<typename T, typename D>
inline void Lexicon<T, D>::clear() {
    terms.clear();
}
