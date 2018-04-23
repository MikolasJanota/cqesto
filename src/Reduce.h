#ifndef REDUCE_HH_1795
#define REDUCE_HH_1795
#include "DataStructures.h"
#include "Transform.h"
namespace qesto {
   class Reduce : private Transform {
   public:
      Reduce(Expressions& factory, const Substitution& substitution)
      : Transform(factory)
      , substitution(substitution) {
      }

      ID operator()(ID node) { return visit(node); }
   private:
      virtual ID visit_lit(ID node, Lit lit) {
         const Var v = var(lit);
         const auto f = substitution.find(v);
         if (f==substitution.end()) return node;
         return !sign(lit) == f->second ? factory.make_true() : factory.make_false();
      }
   private:
      const Substitution& substitution;
   };
}
#endif
