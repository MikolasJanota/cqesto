#include "level_info.h"
using namespace qesto;
LevelInfo::LevelInfo(const Prefix &pref) : pref(pref) {
    mxv = 0;
    vis.resize(1);
    ex_count = un_count = 0;
    for (size_t level = 0; level < pref.size(); ++level) {
        const QuantifierType qt = pref[level].first;
        for (const auto v : pref[level].second) {
            const auto vix = ix(v);
            if (qt == EXISTENTIAL)
                ++ex_count;
            else
                ++un_count;
            if (mxv < v) {
                mxv = v;
                vis.resize(vix + 1);
            }
            vis[vix].first = qt;
            vis[vix].second = level;
        }
    }
}
