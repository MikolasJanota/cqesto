/*
 * File:   ZigZag.h
 * Author: mikolas
 *
 * Created on 18 May 2015, 12:07
 */

#ifndef ZIGZAG_H
#define	ZIGZAG_H
#include "qtypes.h"
#include "LevelInfo.h"
#include "LevelSolver.h"
namespace qesto {
extern NiceExpressionPrinter* dprn;

   class ZigZag {
   public:

      ZigZag(const Options& options,
             Expressions& factory, QFla& qf)
      : options(options)
      , factory(factory)
      , levels(qf.pref)
      , formula(qf)
      , conflict_count(0)
      , verb(options.get_verbose()) {
        if(options.get_verbose()>3) {
          std::cerr<<"input"<<std::endl; (*dprn)(qf.output)<<std::endl;
        }
         init();
      }

      bool solve();
      std::ostream& print_stats(std::ostream& o) {
        return o<<"c bt_count:"<<conflict_count<<std::endl;
      }
   private:
      lbool solve_(int confl_budget);
      void randomize();

      struct Decision {
         Decision(QuantifierType player,
                  Lit decision_literal)
         : player(player)
         , decision_literal(decision_literal)
         {
         }

         QuantifierType player;
         Lit decision_literal;
      };

      const Options& options;
      Expressions& factory;
      LevelInfo levels;
      QFla formula;
      std::vector<LevelSolver*> solvers;
      size_t conflict_count;
      void init();
      int verb;
   };
}
#endif	/* ZIGZAG_H */

