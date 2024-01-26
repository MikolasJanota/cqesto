/*
 * File:  idvector.h
 * Author:  mikolas
 * Created on:  Fri, May 15, 2015 4:49:30 PM
 * Copyright (C) 2015, Mikolas Janota
 */
#pragma once
#include "data_structures.h"
#include "immutable_vector.h"
namespace qesto {
class IDVector : public ImmutableVector<ID, std::hash<ID>> {
  public:
    using ImmutableVector::ImmutableVector; // inherit constructors

    inline bool contains_true() { return first(TRUE) < size(); }
    inline bool contains_false() { return first(FALSE) < size(); }

    inline bool contains_compl() const {
        const size_t sz = size();
        const size_t first_lit = first(LITERAL);
        if (first_lit >= sz)
            return false;
        ID last = (*this)[first_lit];
        for (size_t i = first_lit + 1; i < sz; ++i) {
            assert(last.get_type() == LITERAL);
            const ID &c = (*this)[i];
            if (c.get_type() != LITERAL)
                return false;
            const Lit clit = ID::getLit(c);
            const Lit lastlit = ID::getLit(last);
            if (lastlit == ~clit)
                return true;
            last = c;
        }
        return false;
    }

  private:
    inline size_t first(NodeType nt) const {
        const size_t sz = size();
        size_t i = 0;
        while (i < sz && ((*this)[i].get_type() < nt))
            ++i;
        return i < sz && ((*this)[i].get_type() == nt) ? i : sz;
    }
};
} // namespace qesto
