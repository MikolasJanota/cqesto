/*
 * File:  IDVector.h
 * Author:  mikolas
 * Created on:  Fri, May 15, 2015 4:49:30 PM
 * Copyright (C) 2015, Mikolas Janota
 */
#ifndef IDVECTOR_H_25344
#define IDVECTOR_H_25344
namespace qesto {
#include "DataStructures.h"
#include "ImmutableVector.h"
class IDVector : public ImmutableVector<ID,ID_hash> {
public:
  IDVector() : ImmutableVector<ID,ID_hash>() { }
  IDVector(const std::vector<ID>& es) : ImmutableVector<ID,ID_hash>(es) { }
  IDVector(const IDVector& o) : ImmutableVector<ID,ID_hash>(o) { }
  virtual ~IDVector() { }

  inline bool contains_true() {return first(TRUE)<size();}
  inline bool contains_false() {return first(FALSE)<size();}

  inline bool contains_compl() const {
    const size_t sz = size();
    const size_t first_lit = first(LITERAL);
    if (first_lit>=sz) return false;
    ID last = (*this)[first_lit];
    for (size_t i=first_lit+1; i<sz; ++i) {
      assert(last.get_type()==LITERAL);
      const ID& c = (*this)[i];
      if (c.get_type() != LITERAL) return false;
      const Lit clit= SATSPC::toLit((int)c.get_index());
      const Lit lastlit= SATSPC::toLit((int)last.get_index());
      if (lastlit==~clit) return true;
      last = c;
    }
    return false;
  }
private:
  inline size_t first(NodeType nt) const {
    const size_t sz = size();
    size_t i=0;
    while (i<sz && ((*this)[i].get_type()<nt)) ++i;
    return i<sz && ((*this)[i].get_type()==nt) ? i : sz;
  }
};
}
#endif /* IDVECTOR_H_25344 */
