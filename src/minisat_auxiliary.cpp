#include "minisat_auxiliary.h"
using namespace SATSPC;

ostream& SATSPC::print(ostream& out, const vec<Lit>& lv) {for (int i=0;i<lv.size();++i) out <<  lv[i] << " "; return out;}

ostream& SATSPC::print(ostream& out, const vector<Lit>& lv) {for (size_t i=0;i<lv.size();++i) out << lv[i] << " "; return out;}

ostream& SATSPC::print_model(ostream& out,const vec<lbool>& lv) { 
  return print_model(out, lv, 1, lv.size()-1); 
}

ostream& SATSPC::print_model(ostream& out, const vec<lbool>& lv, int l, int r) {
    for (int i=l;i<=r;++i) {
        int v=0;
        if(i>l) out<<" ";
        if (lv[i]==l_True) v=i;
        else if (lv[i]==l_False) v=-i;
        else if (lv[i]==l_Undef) v=0;
        else assert (false);
        out<<v;
    }
    return out;
}
