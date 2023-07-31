/*
 * File:   auxiliary.hh
 * Author: mikolas
 *
 * Created on October 12, 2011
 */
#pragma once
#include <iostream>
#include <sys/time.h>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#ifndef __MINGW32__
#include <sys/resource.h>
#endif
#include <assert.h>
#include <string.h>

#define __PL (std::cerr << __FILE__ << ":" << __LINE__ << std::endl).flush();

#ifdef __MINGW32__
static inline double read_cpu_time() { return 0; }
#else
static inline double read_cpu_time() {
    struct rusage ru;
    getrusage(RUSAGE_SELF, &ru);
    return (double)ru.ru_utime.tv_sec + (double)ru.ru_utime.tv_usec / 1000000;
}
#endif

// struct sizePairHash {
namespace std {
template <> struct hash<std::pair<size_t, size_t>> {
    inline size_t operator()(const std::pair<size_t, size_t> &p) const {
        return p.first ^ p.second;
    }
};
} // namespace std

struct cstrHash {
    inline size_t operator()(const char *s) const {
        size_t hash = 5381;
        char c;
        while ((c = *s++))
            hash = ((hash << 5) + hash) + c; /* hash * 33 + c */
        return hash;
    }
};

struct cstrEq {
    inline size_t operator()(const char *s1, const char *s2) const {
        return strcmp(s1, s2) == 0;
    };
};

template <class K, class V>
bool contains(const std::unordered_map<K, V> &es, const K &e) {
    return es.find(e) != es.end();
}

template <class K, class V>
V get(const std::unordered_map<K, V> &es, const K &e) {
    const auto j = es.find(e);
    assert(j != es.end());
    return j->second;
}

inline bool contains(const std::vector<bool> &es, int e) {
    assert(e >= 0);
    const auto ix = (size_t)e;
    return ix < es.size() ? es[ix] : false;
}

template <class K>
inline bool insert_chk(std::unordered_set<K> &es, const K &e) {
    const auto i = es.insert(e);
    const bool a = i.second;
    assert(a);
    return a;
}

inline bool erase(std::vector<bool> &es, int e) {
    assert(e >= 0);
    const auto ix = (size_t)e;
    if (ix >= es.size())
        return false;
    const bool rv = es[ix];
    es[ix] = false;
    return rv;
}

inline bool insert(std::vector<bool> &es, int e) {
    assert(e >= 0);
    const auto ix = (size_t)e;
    if (ix >= es.size())
        es.resize(ix + 1, false);
    const bool rv = es[ix];
    es[ix] = e;
    return rv;
}

template <class K> bool contains(const std::unordered_set<K> &es, const K &e) {
    return es.find(e) != es.end();
}

inline std::ostream &operator<<(std::ostream &outs,
                                const std::vector<bool> &ns) {
    outs << '<';
    bool f = true;
    for (size_t i = 0; i < ns.size(); ++i) {
        if (!ns[i])
            continue;
        if (!f)
            outs << ' ';
        f = false;
        outs << i;
    }
    return outs << '>';
}

inline std::ostream &operator<<(std::ostream &outs,
                                const std::vector<size_t> &ns) {
    outs << '<';
    for (size_t i = 0; i < ns.size(); ++i) {
        if (i)
            outs << ' ';
        outs << ns[i];
    }
    return outs << '>';
}

double luby(double y, int x);
