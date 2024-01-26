#pragma once

#include "auxiliary.h"
#include "sat_interface.h"
#include <algorithm>
#include <cinttypes>
#include <iostream>
#include <map>
#include <ostream>
#include <stddef.h>
#include <unordered_map>
#include <vector>

namespace qesto {
using SATSPC::Lit;
using SATSPC::Var;
using std::hash;
using std::ostream;
using std::pair;

#define __PL           (std::cerr << __FILE__ << ":" << __LINE__ << std::endl).flush();
#define CONTAINS(s, e) (((s).find(e)) != ((s).end()))

enum NodeType {
    FALSE = 0,
    TRUE = 1,
    LITERAL = 2,
    NEGATION = 3,
    AND = 4,
    OR = 5
};

class ID {
  public:
    ID(NodeType t, size_t i) : index(i), type(t) {}
    ID() : index(-1), type(OR) {}
    inline NodeType get_type() const { return type; }
    inline size_t get_index() const { return index; }
    inline uint64_t toInt() const { // careful with precedence of "<<"!
        return (static_cast<uint64_t>(type) << 32) + index;
    }
    static ID fromInt(uint64_t i) { return ID(i); }
#if USE_CMS
    static ID mkLit(const Lit &l) {
        return ID(LITERAL, static_cast<size_t>(l.toInt()));
    }
    static Lit getLit(const ID &l) { return CMSat::Lit::toLit(l.index); }
#else
    static Lit getLit(const ID &l) { return SATSPC::toLit(l.index); }

    static ID mkLit(const Lit &l) {
        return ID(LITERAL, static_cast<size_t>(SATSPC::toInt(l)));
    }
#endif

  private:
    ID(uint64_t i) : index(i & 0xFFFFFFFF), type((NodeType)(i >> 32)) {
        assert((i >> 32) <= 6);
        // assert(0<=(i>>32));
    };
    size_t index;
    NodeType type;
};

inline bool operator==(const ID &i1, const ID &i2) {
    return i1.get_type() == i2.get_type() && i1.get_index() == i2.get_index();
}

inline bool operator<(const ID &i1, const ID &i2) {
    const NodeType t1 = i1.get_type();
    const NodeType t2 = i2.get_type();
    if (t1 < t2)
        return true;
    if (t1 > t2)
        return false;
    return (i1.get_index() < i2.get_index());
}

inline bool operator!=(const ID &i1, const ID &i2) {
    return i1.get_type() != i2.get_type() || i1.get_index() != i2.get_index();
}

inline std::ostream &operator<<(std::ostream &o, ID id) {
    o << '[';
    switch (id.get_type()) {
    case LITERAL: o << "LITERAL"; break;
    case NEGATION: o << "NEGATION"; break;
    case AND: o << "AND"; break;
    case OR: o << "OR"; break;
    case FALSE: o << "FALSE"; break;
    case TRUE: o << "TRUE"; break;
    default: assert(0); o << "ERROR";
    }
    return o << ',' << id.get_index() << ']';
}
} // namespace qesto

namespace std {
template <> struct hash<qesto::ID> {
    inline size_t operator()(const qesto::ID& i) const { return i.toInt(); }
};

template <> struct hash<pair<qesto::ID, bool>> {
    inline size_t operator()(std::pair<qesto::ID, bool> p) const {
        return p.second ? p.first.toInt() : ~p.first.toInt();
    }
};

#if USE_CMS
template <> struct hash<SATSPC::Lit> {
    inline size_t operator()(const SATSPC::Lit &l) const { return l.toInt(); }
};
#else
template <> struct hash<SATSPC::Lit> {
    inline size_t operator()(const SATSPC::Lit &l) const {
        return SATSPC::toInt(l);
    }
};
#endif
} // namespace std

