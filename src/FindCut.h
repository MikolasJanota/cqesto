/*
 * File:   FindCut.h
 * Author: mikolas
 *
 * Created on 18 May 2015, 15:00
 */

#ifndef FINDCUT_H
#define	FINDCUT_H
#include "Expressions.h"
#include "Visitors.h"
#include "minisat_auxiliary.h"
#include "Eval.h"
#include <unordered_map>
namespace qesto {
   class FindCut : MemoizedExpressionVisitor<bool> {
   public:
      typedef std::unordered_map<Var, bool> Vals;

      FindCut(Expressions& factory, Eval& vals)
      : MemoizedExpressionVisitor<bool>(factory)
      , vals(vals)
      {}

      void operator() (ID n) { visit(n);}
      const std::unordered_set<ID,ID_hash,ID_equal>& get_cut() const {return cut;}

      virtual bool visit_lit(ID node, Lit) {
         const lbool v=vals(node);
         if(v!=Minisat::l_Undef)
            cut.insert(v==Minisat::l_True ? node : factory.make_not(node));
         return true;
      }

      virtual bool visit_not(ID node, ID operand) {
         const lbool v=vals(node);
         if(v!=Minisat::l_Undef) { cut.insert(v==Minisat::l_True ? node : factory.make_not(node)); return true; }
         return visit(operand);
      }

      virtual bool visit_or(ID node, IDVector operands) {
         const lbool v=vals(node);
         if(v!=Minisat::l_Undef) { cut.insert(v==Minisat::l_True ? node : factory.make_not(node)); return true; }
         return visit_ops(operands);
      }

      virtual bool visit_and(ID node, IDVector operands) {
         const lbool v=vals(node);
         if(v!=Minisat::l_Undef) { cut.insert(v==Minisat::l_True ? node : factory.make_not(node)); return true; }
         return visit_ops(operands);
      }

      virtual bool visit_false(ID) { return true; }
      virtual bool visit_true(ID node) { cut.insert(node); return true; }
   private:
      Eval& vals;
      std::unordered_set<ID,ID_hash,ID_equal> cut;
      inline bool visit_ops(IDVector ops) {
         FOR_EACH(i,ops) visit(*i);
         return true;
      }
   };
}
#endif	/* FINDCUT_H */

