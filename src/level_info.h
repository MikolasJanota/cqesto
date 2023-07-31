
#ifndef LEVELINFO
#define LEVELINFO
#include "data_structures.h"
#include "qtypes.h"
#include <type_traits>
namespace qesto {
class LevelInfo {
  public:
    LevelInfo(const Prefix &pref);

    size_t ix(Var v) const {
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wtype-limits"
        assert(v >= 0);
        return static_cast<size_t>(v);
#pragma GCC diagnostic pop
    }

    inline size_t qlev(Var v) const {
        const auto vi = ix(v);
        assert(vi < vis.size());
        return vis[vi].second;
    }

    inline size_t qlev(Lit l) const { return qlev(var(l)); }

    inline QuantifierType type(Var v) const {
        const auto vi = ix(v);
        assert(vi < vis.size());
        return vis[vi].first;
    }

    inline size_t qlev_count() const { return pref.size(); }

    inline size_t existential_count() const { return ex_count; }

    inline size_t universal_count() const { return un_count; }

    inline Var maxv() const { return mxv; }

    inline const VarVector &level_vars(size_t lev) const {
        assert(lev < pref.size());
        return pref[lev].second;
    }

    inline QuantifierType level_type(size_t lev) const {
        assert(lev < pref.size());
        return pref[lev].first;
    }

    inline QuantifierType type(Lit l) const { return type(var(l)); }

    inline size_t max_qlev(const std::vector<Lit> &v) const {
        assert(v.size());
        size_t r = qlev(v[0]);
        for (size_t i = 1; i < v.size(); ++i) {
            const auto q = qlev(v[i]);
            if (q > r)
                r = q;
        }
        return r;
    }

    inline size_t max_qt(QuantifierType qt) const {
        Var m = 0;

        for (const auto &bi : pref) {
            if (bi.first != qt)
                continue;

            for (const auto v : bi.second) {
                if (v > m)
                    m = v;
            }
        }
        return m;
    }

  private:
    const Prefix pref;
    Var mxv;
    size_t ex_count, un_count;
    std::vector<std::pair<QuantifierType, size_t>> vis;

    LevelInfo(const LevelInfo &) { assert(0); }
};

inline ostream &operator<<(ostream &outs, const LevelInfo &levs) {
    for (size_t ql = 0; ql < levs.qlev_count(); ++ql) {
        outs << "[" << levs.level_type(ql);
        for (const auto v : levs.level_vars(ql))
            outs << " " << v;
        outs << "]";
    }
    return outs;
}
} // namespace qesto
#endif

