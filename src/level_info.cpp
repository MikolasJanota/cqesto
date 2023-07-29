#include "level_info.h"
using namespace qesto;
LevelInfo::LevelInfo(const Prefix &pref) : pref(pref) {
    mxv = -1;
    ex_count = un_count = 0;
    for (size_t level = 0; level < pref.size(); ++level) {
        const QuantifierType qt = pref[level].first;
        for (const auto v : pref[level].second) {
            assert(v >= 0);
            if (qt == EXISTENTIAL)
                ++ex_count;
            else
                ++un_count;
            if (mxv < v) {
                mxv = v;
                vis.resize(mxv + 1);
            }
            vis[(size_t)v].first = qt;
            vis[(size_t)v].second = level;
        }
    }
}
