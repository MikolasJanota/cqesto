/* 
 * File:   MaxQLev.h
 * Author: mikolas
 *
 * Created on 18 May 2015, 15:43
 */

#ifndef MAXQLEV_H
#define	MAXQLEV_H
#include "Expressions.h"
#include "Visitors.h"
#include "minisat_auxiliary.h"
#include "Eval.h"
#include "LevelInfo.h"
#include <unordered_map>
namespace qesto {
   class MaxQLev : MemoizedExpressionVisitor<int> {
   public:
      MaxQLev(Expressions& factory,
              const std::unordered_set<Var>& vars,
              const LevelInfo& levs)
      : MemoizedExpressionVisitor<int>(factory)
      , vars(vars)
      , levs(levs) { }
      
      int operator() (ID n) { return visit(n);}
      
      virtual int visit_lit(ID, Lit lit) { 
         const Var v=var(lit);
         const auto i=vars.find(v);
         if(i==vars.end()) return -1;
         return levs.qlev(v);
      }
    
      virtual int visit_and(ID, IDVector operands) { 
         int rv=-1;
         FOR_EACH(i,operands) rv=std::max(rv,visit(*i));
         return rv;
      }
      
      virtual int visit_not(ID, ID operand) { return visit(operand); }
      
      virtual int visit_or(ID, IDVector operands) { 
         int rv=-1;
         FOR_EACH(i,operands) rv=std::max(rv,visit(*i));
         return rv;
      }
      
      virtual int visit_false(ID) { return -1; }
      virtual int visit_true(ID) { return -1; }      
   private:
      const std::unordered_set<Var>& vars;
      const LevelInfo& levs;
   };
}
#endif	/* MAXQLEV_H */

