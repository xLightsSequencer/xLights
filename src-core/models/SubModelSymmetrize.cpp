#include "SubModelSymmetrize.h"

#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <set>

#include "../utils/NodeUtils.h"
#include "../utils/string_utils.h"

#ifndef PI
#define PI 3.14159265358979323846
#endif

namespace {
struct AspectInfo {
    float cx{0}, cy{0};
    float aspectx{1}, aspecty{1};
    bool offer{false};
};

AspectInfo ComputeAspect(const std::map<int, std::pair<float, float>>& coords) {
    AspectInfo ai;
    if (coords.empty())
        return ai;
    float cx = 0, cy = 0;
    for (const auto& x : coords) {
        cx += x.second.first;
        cy += x.second.second;
    }
    cx /= float(coords.size());
    cy /= float(coords.size());
    ai.cx = cx;
    ai.cy = cy;

    float nx = cx, ny = cy, xx = cx, xy = cy;
    float varx = 0, vary = 0;
    for (const auto& x : coords) {
        nx = std::min(nx, x.second.first);
        xx = std::max(xx, x.second.first);
        ny = std::min(ny, x.second.second);
        xy = std::max(xy, x.second.second);
        varx += (x.second.first - cx) * (x.second.first - cx);
        vary += (x.second.second - cy) * (x.second.second - cy);
    }
    float dlx = xx - nx;
    float dly = xy - ny;
    if (dlx > 0 && dly > 0) {
        float mvar = std::max(varx, vary);
        if (mvar > 0) {
            ai.aspectx = sqrtf(varx / mvar);
            ai.aspecty = sqrtf(vary / mvar);
        }
        if (ai.aspectx < .98f || ai.aspecty < .98f) {
            ai.offer = true;
        }
    }
    return ai;
}
} // namespace

namespace SubModelSymmetrize {

bool ShouldOfferSquarify(const std::map<int, std::pair<float, float>>& coords) {
    return ComputeAspect(coords).offer;
}

Result Symmetrize(const std::map<int, std::pair<float, float>>& coordsIn,
                  int w, int h,
                  const std::vector<std::string>& strands,
                  const Options& opts) {
    Result result;
    const int dos = opts.degreeOfSymmetry;
    if (dos < 2 || coordsIn.empty() || w <= 0 || h <= 0) {
        return result;
    }

    std::map<int, std::pair<float, float>> coords = coordsIn;

    AspectInfo ai = ComputeAspect(coords);
    const float cx = ai.cx;
    const float cy = ai.cy;
    if (ai.offer) {
        result.aspectAdvisory = "non-square";
        if (opts.squarifyAspect) {
            for (auto& pt : coords) {
                pt.second.first = (pt.second.first - cx) * ai.aspecty + cx;
                pt.second.second = (pt.second.second - cy) * ai.aspectx + cy;
            }
        }
    }

    bool handleCenterNode = opts.handleCenterNode && (coords.size() % dos == 1);

    std::map<int, std::pair<float, float>> fcoords1, fcoords2;
    std::map<int, float> fturns;
    for (const auto& p : coords) {
        float dx = p.second.first - cx;
        float dy = p.second.second - cy;
        if (dx == 0 && dy == 0) {
            fcoords1[p.first] = std::make_pair(cx, cy);
            fturns[p.first] = 0;
            continue;
        }
        float rad = sqrtf(dx * dx + dy * dy);
        float ang = atan2f(dy, dx);
        if (ang <= 0) {
            ang += float(2 * PI);
        }
        ang *= float(dos);
        float turn = float(ang / (2 * PI));
        if (turn >= dos)
            turn -= dos;
        fturns[p.first] = turn;
        while (ang >= 2 * PI)
            ang -= float(2 * PI);
        ang /= float(dos);
        fcoords1[p.first] = std::make_pair(rad * cosf(ang) + cx, rad * sinf(ang) + cy);
        if (ang < PI / dos / 2)
            ang += 2 * PI / dos;
        fcoords2[p.first] = std::make_pair(rad * cosf(ang) + cx, rad * sinf(ang) + cy);
    }

    std::set<int> nodesNeedMatch;
    std::map<int, std::vector<int>> matchIDToNodeSet;
    std::map<int, int> nodeToMatchIDs;

    int origStrands = strands.size();
    for (unsigned i = 0; i < strands.size(); ++i) {
        auto x = ::Split(NodeUtils::ExpandNodes(strands[strands.size() - 1 - i]), ',');
        for (auto n : x) {
            if (n == "" || n == "0")
                continue;
            nodesNeedMatch.insert((int)std::strtol(n.c_str(), nullptr, 10));
        }
    }

    if (handleCenterNode) {
        bool first = true;
        float ndst = 0;
        int nnode = -1;
        for (const auto& pt : coords) {
            float dx = pt.second.first - cx;
            float dy = pt.second.second - cy;
            float dst = dx * dx + dy * dy;
            if (first || dst < ndst) {
                ndst = dst;
                nnode = pt.first;
            }
            first = false;
        }
        nodesNeedMatch.erase(nnode);
        nodeToMatchIDs[nnode] = matchIDToNodeSet.size();
        matchIDToNodeSet[nodeToMatchIDs[nnode]] = std::vector<int>(dos, nnode);
    }

    int radius = 0;
    while (!nodesNeedMatch.empty()) {
        std::vector<std::vector<std::vector<int>>> bins(w, std::vector<std::vector<int>>(h));

        for (const auto& pt : fcoords1) {
            if (nodeToMatchIDs.count(pt.first))
                continue;
            int bx = int(pt.second.first);
            int by = int(pt.second.second);
            for (int x = bx - radius; x <= bx + radius; ++x) {
                if (x < 0 || x >= w)
                    continue;
                for (int y = by - radius; y <= by + radius; ++y) {
                    if (y < 0 || y >= h)
                        continue;
                    bins[x][y].push_back(pt.first);
                }
            }
        }
        for (const auto& pt : fcoords2) {
            if (nodeToMatchIDs.count(pt.first))
                continue;
            int bx = int(pt.second.first);
            int by = int(pt.second.second);
            for (int x = bx - radius; x <= bx + radius; ++x) {
                if (x < 0 || x >= w)
                    continue;
                for (int y = by - radius; y <= by + radius; ++y) {
                    if (y < 0 || y >= h)
                        continue;
                    bins[x][y].push_back(pt.first);
                }
            }
        }

        for (int x = 0; x < w; ++x) {
            for (int y = 0; y < h; ++y) {
                if (int(bins[x][y].size()) < dos)
                    continue;
                std::vector<std::pair<float, int>> matches;
                for (int pt : bins[x][y]) {
                    if (nodeToMatchIDs.count(pt) != 0)
                        continue;
                    matches.push_back(std::make_pair(fturns[pt], pt));
                }
                if ((int)matches.size() < dos)
                    continue;
                std::sort(matches.begin(), matches.end());

                float tgt = matches[0].first;
                int found = 0;
                for (unsigned j = 0; j < matches.size(); ++j) {
                    if (matches[j].first >= tgt - .5 && matches[j].first <= tgt + .5) {
                        ++found;
                        tgt += 1;
                    }
                    if (found == dos)
                        break;
                }
                if (found != dos)
                    continue;

                std::vector<int> matched;
                tgt = matches[0].first;
                int mid = matchIDToNodeSet.size();
                for (unsigned j = 0; j < matches.size(); ++j) {
                    if (matches[j].first >= tgt - .5 && matches[j].first <= tgt + .5) {
                        ++found;
                        tgt += 1;
                        matched.push_back(matches[j].second);
                        nodeToMatchIDs[matches[j].second] = mid;
                        nodesNeedMatch.erase(matches[j].second);
                    }
                    if (found == dos)
                        break;
                }
                matchIDToNodeSet[mid] = matched;
            }
        }

        ++radius;
        if (radius > 20 && !nodesNeedMatch.empty()) {
            break;
        }
    }

    if (!nodesNeedMatch.empty()) {
        result.unmatchedNodes.assign(nodesNeedMatch.begin(), nodesNeedMatch.end());
        return result;
    }

    std::vector<std::string> generated;
    for (int t = 1; t < dos; ++t) {
        for (int sn = 0; sn < origStrands; ++sn) {
            bool first = true;
            auto x = ::Split(NodeUtils::ExpandNodes(strands[sn]), ',');
            std::string str;
            for (auto n : x) {
                if (first) {
                    first = false;
                } else {
                    str += ",";
                }
                if (n == "" || n == "0")
                    continue;
                int nn = (int)std::strtol(n.c_str(), nullptr, 10);
                auto& matchs = matchIDToNodeSet[nodeToMatchIDs[nn]];
                for (int ii = 0; ii < dos; ++ii) {
                    if (matchs[ii] == nn) {
                        int mapn = opts.clockwise ? matchs[(ii + t) % dos] : matchs[(ii - t + dos) % dos];
                        str += std::to_string(mapn);
                        break;
                    }
                }
            }
            generated.push_back(NodeUtils::CompressNodes(str));
        }
    }

    if (!opts.bottomToTop) {
        result.strands = generated;
        result.strands.insert(result.strands.end(), strands.begin(), strands.end());
    } else {
        result.strands = strands;
        result.strands.insert(result.strands.end(), generated.begin(), generated.end());
    }

    result.success = true;
    return result;
}

} // namespace SubModelSymmetrize
