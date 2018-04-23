/*
 * File:  SetPolarities.h
 * Author:  mikolas
 * Created on:  Thu, May 28, 2015 1:52:10 PM
 * Copyright (C) 2015, Mikolas Janota
 */
#ifndef SETPOLARITIES_H_27143
#define SETPOLARITIES_H_27143
#include "Visitors.h"
#include "EncoderToSAT.h"
namespace qesto {
  class SetPolarities : protected MemoizedExpressionArgumentVisitor<bool,bool> {
    public:
      SetPolarities(Expressions& factory, EncoderToSAT& enc)
        : MemoizedExpressionArgumentVisitor<bool,bool>(factory)
          , enc(enc) { }

      void operator()(ID expression) {
        visit(expression,true);
      }

      virtual bool visit_or(ID node, bool polarity, IDVector operands) {
        enc.set_polarity(node,polarity);
        FOR_EACH(i,operands) visit(*i,polarity);
        return polarity;
      }

      virtual bool visit_and(ID node, bool polarity, IDVector operands) {
        enc.set_polarity(node,polarity);
        FOR_EACH(i,operands) visit(*i,polarity);
        return polarity;
      }

      virtual bool visit_not(ID, bool polarity, ID operand) { return visit(operand, !polarity); }
      virtual bool visit_lit(ID, bool argument, Lit) {return argument;}
      virtual bool visit_false(ID, bool argument) {return argument;}
      virtual bool visit_true(ID, bool argument) {return argument;}
    protected:
      EncoderToSAT& enc;
  };
}
#endif /* SETPOLARITIES_H_27143 */
