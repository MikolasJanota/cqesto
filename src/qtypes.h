/*
 * File:  qtypes.h
 * Author:  mikolas
 * Created on:  Fri, Aug 01, 2014 4:42:21 PM
 * Copyright (C) 2014, Mikolas Janota
 */
#pragma once
#include "expressions.h"
#include <ostream>
#include <unordered_map>
#include <vector>

namespace qesto {
enum QuantifierType { UNIVERSAL, EXISTENTIAL };
typedef std::pair<QuantifierType, VarVector> Quantification;
typedef std::vector<Quantification> Prefix;

inline QuantifierType opponent(QuantifierType qt) {
    assert(qt == UNIVERSAL || qt == EXISTENTIAL);
    return (qt == UNIVERSAL) ? EXISTENTIAL : UNIVERSAL;
}

typedef struct {
    VarVector free;
    Prefix pref;
    ID output;
} QFla;

inline std::ostream &
print(const std::unordered_map<int, const char *> &var2name, std::ostream &outs,
      const Prefix &pref) {
    outs << "[" << std::endl;

    for (const auto &q : pref) {
        outs << (q.first == UNIVERSAL ? "A" : "E");
        for (const auto v : q.second)
            outs << " " << var2name.find(v)->second;
        outs << std::endl;
    }
    return outs << "]";
}
} // namespace qesto

namespace std {
inline std::ostream &operator<<(std::ostream &outs, const qesto::Prefix &pref) {
    outs << "[" << std::endl;
    for (const auto &q : pref) {
        outs << (q.first == qesto::UNIVERSAL ? "A" : "E");
        for (const auto v : q.second)
            outs << " " << v;
        outs << endl;
    }
    return outs << "]";
}
} // namespace std
