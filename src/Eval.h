/*
 * File:   Eval.h
 * Author: mikolas
 *
 * Created on 18 May 2015, 14:22
 */

#ifndef EVAL_H
#define	EVAL_H
#include "Expressions.h"
#include "Visitors.h"
#include "minisat_auxiliary.h"
#include <unordered_map>
namespace qesto {
   class Eval : MemoizedExpressionVisitor<lbool> {
   public:

      Eval(Expressions& factory, const Substitution& vals)
      : MemoizedExpressionVisitor<lbool>(factory)
      , vals(vals) {
      }

      lbool operator() (ID n) {return visit(n);}

      virtual lbool visit_lit(ID, Lit lit) {
         const auto i=vals.find(var(lit));
         if(i==vals.end()) return Minisat::l_Undef;
         return sign(lit)!=i->second ? Minisat::l_True : Minisat::l_False;
      }

      virtual lbool visit_and(ID, IDVector operands) {
         bool allt=true;
         FOR_EACH(i,operands) {
            const lbool ri=visit(*i);
            if(ri==Minisat::l_False) return Minisat::l_False;
            if(ri==Minisat::l_Undef) allt=false;
         }
         return allt ? Minisat::l_True : Minisat::l_Undef;
      }

      virtual lbool visit_not(ID, ID operand) {
         return neg(visit(operand));
      }

      virtual lbool visit_or(ID, IDVector operands) {
         bool allf=true;
         FOR_EACH(i,operands) {
            const lbool ri=visit(*i);
            if(ri==Minisat::l_True) return Minisat::l_True;
            if(ri==Minisat::l_Undef) allf=false;
         }
         return allf ? Minisat::l_False : Minisat::l_Undef;
      }

      virtual lbool visit_false(ID) { return Minisat::l_False; }
      virtual lbool visit_true(ID) { return Minisat::l_True; }
   private:
      const Substitution& vals;
   };
}
#endif	/* EVAL_H */
