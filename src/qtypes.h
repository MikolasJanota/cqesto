/*
 * File:  qtypes.h
 * Author:  mikolas
 * Created on:  Fri, Aug 01, 2014 4:42:21 PM
 * Copyright (C) 2014, Mikolas Janota
 */
#ifndef QTYPES_HH_15436
#define QTYPES_HH_15436
#include <vector>
#include <unordered_map>
#include <ostream>
#include <vector>
#include "Expressions.h"

namespace qesto {
   enum QuantifierType {UNIVERSAL, EXISTENTIAL};
   typedef std::pair<QuantifierType, VarVector> Quantification;
   typedef std::vector<Quantification> Prefix;

   inline QuantifierType opponent(QuantifierType qt) {
      assert(qt==UNIVERSAL||qt==EXISTENTIAL);
      return (qt==UNIVERSAL) ? EXISTENTIAL : UNIVERSAL;
   }
   
   typedef struct {
      Prefix pref;
      ID output;
   } QFla;

   inline std::ostream& print(
                              const unordered_map<int, const char*>& var2name,
                              std::ostream& outs,
                              const Prefix& pref) {
      outs << "[" << std::endl;

      FOR_EACH(i, pref) {
         Quantification q = *i;
         outs << (q.first == UNIVERSAL ? "A" : "E");
         FOR_EACH(vi, q.second) outs << " " << var2name.find(*vi)->second;
         outs << std::endl;
      }
      return outs << "]";
   }
}

namespace std {
  inline std::ostream& operator << (std::ostream& outs, const qesto::Prefix& pref) {
    outs<<"["<<std::endl;
    FOR_EACH(i,pref) {
      qesto::Quantification q=*i;
      outs<<(q.first==qesto::UNIVERSAL?"A":"E");
      FOR_EACH(vi,q.second) outs<<" "<<*vi;
      outs<<endl;
    }
    return outs<<"]";
  }
}
#endif /* QTYPES_HH_15436 */
