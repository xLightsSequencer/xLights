/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include "SubModelOps.h"

#include "../utils/NodeUtils.h"
#include "../utils/string_utils.h"

#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <set>

namespace submodel_ops {

namespace {

// The grid shows strands bottom-up, so display row r is strands[n-1-r].
inline size_t StorageIndex(size_t count, size_t displayRow)
{
    return count - 1 - displayRow;
}

// Expand a strand to one entry per cell so row-shape operations can line
// columns up across rows.
std::vector<std::string> ExpandCells(const std::string& strand)
{
    return Split(NodeUtils::ExpandNodes(strand), ',');
}

std::string CollapseCells(const std::vector<std::string>& cells)
{
    return NodeUtils::CompressNodes(Join(cells, ","));
}

// Renumber every node through `map`, dropping anything that isn't an integer
// (blanks and stray text) exactly as the desktop's ToCLong guard did.
void RenumberNodes(std::vector<std::string>& strands,
                   const std::function<long(long)>& map)
{
    for (auto& strand : strands) {
        const auto cells = ExpandCells(strand);
        std::vector<std::string> out;
        out.reserve(cells.size());
        for (const auto& cell : cells) {
            char* end = nullptr;
            const long val = std::strtol(cell.c_str(), &end, 10);
            if (end == cell.c_str() || *end != '\0') {
                continue;
            }
            out.push_back(std::to_string(map(val)));
        }
        strand = CollapseCells(out);
    }
}

} // namespace

std::string ReverseRow(const std::string& row)
{
    const auto nodes = Split(row, ',');
    std::string out;
    for (auto it = nodes.rbegin(); it != nodes.rend(); ++it) {
        if (it != nodes.rbegin()) {
            out += ",";
        }
        if (it->find('-') != std::string::npos) {
            const auto range = Split(*it, '-');
            if (range.size() == 2) {
                out += range[1] + "-" + range[0];
                continue;
            }
        }
        out += *it;
    }
    return out;
}

void TransformAllStrands(std::vector<std::string>& strands,
                         const std::function<std::string(const std::string&)>& fn)
{
    for (size_t i = 0; i < strands.size(); ++i) {
        const size_t idx = StorageIndex(strands.size(), i);
        strands[idx] = fn(strands[idx]);
    }
}

std::string ExpandStrand(const std::string& strand)
{
    return NodeUtils::ExpandNodes(strand);
}

std::string CompressStrand(const std::string& strand)
{
    return NodeUtils::CompressNodes(strand);
}

std::string BlanksToZeros(const std::string& strand)
{
    std::string out;
    for (const auto& cell : Split(strand, ',')) {
        if (!out.empty()) {
            out += ",";
        }
        out += cell.empty() ? "0" : cell;
    }
    return out;
}

std::string ZerosToBlanks(const std::string& strand)
{
    std::string out;
    for (const auto& cell : Split(strand, ',')) {
        if (!out.empty()) {
            out += ",";
        }
        if (cell != "0") {
            out += cell;
        }
    }
    return out;
}

std::string RemoveBlanksAndZeros(const std::string& strand)
{
    std::string out;
    for (const auto& cell : Split(strand, ',')) {
        if (cell.empty() || cell == "0") {
            continue;
        }
        if (!out.empty()) {
            out += ",";
        }
        out += cell;
    }
    return out;
}

void FlipHorizontal(std::vector<std::string>& strands)
{
    for (auto& strand : strands) {
        strand = ReverseRow(strand);
    }
}

void FlipVertical(std::vector<std::string>& strands)
{
    if (strands.size() < 2) {
        return;
    }
    std::reverse(strands.begin(), strands.end());
}

void ReverseNodes(std::vector<std::string>& strands, int nodeCount)
{
    const long max = nodeCount + 1;
    RenumberNodes(strands, [max](long v) { return max - v; });
}

void ShiftNodes(std::vector<std::string>& strands, int nodeCount, int amount)
{
    if (amount == 0 || nodeCount <= 0) {
        return;
    }
    const long max = nodeCount;
    RenumberNodes(strands, [max, amount](long v) {
        long n = v + amount;
        if (n > max) {
            n -= max;
        } else if (n < 1) {
            n += max;
        }
        return n;
    });
}

void PivotRowsColumns(std::vector<std::string>& strands)
{
    if (strands.empty()) {
        return;
    }

    std::vector<std::vector<std::string>> rows;
    size_t widest = 0;
    for (size_t i = 0; i < strands.size(); ++i) {
        rows.push_back(ExpandCells(strands[StorageIndex(strands.size(), i)]));
        widest = std::max(widest, rows.back().size());
    }

    std::vector<std::vector<std::string>> pivoted;
    pivoted.reserve(widest);
    for (size_t col = 0; col < widest; ++col) {
        std::vector<std::string> built;
        built.reserve(rows.size());
        for (const auto& row : rows) {
            built.push_back(col < row.size() ? row[col] : std::string());
        }
        pivoted.push_back(std::move(built));
    }

    strands.clear();
    for (size_t i = widest; i-- > 0;) {
        strands.push_back(CollapseCells(pivoted[i]));
    }
}

void CombineStrands(std::vector<std::string>& strands)
{
    if (strands.empty()) {
        return;
    }

    std::string joined;
    for (size_t i = 0; i < strands.size(); ++i) {
        if (i != 0) {
            joined += ",";
        }
        joined += NodeUtils::ExpandNodes(strands[StorageIndex(strands.size(), i)]);
    }

    strands.clear();
    strands.push_back(NodeUtils::CompressNodes(joined));
}

void MakeRowsUniform(std::vector<std::string>& strands, PadMode mode)
{
    if (strands.empty()) {
        return;
    }

    std::vector<std::vector<std::string>> rows;
    size_t widest = 0;
    for (size_t i = 0; i < strands.size(); ++i) {
        rows.push_back(ExpandCells(strands[StorageIndex(strands.size(), i)]));
        widest = std::max(widest, rows.back().size());
    }

    for (size_t i = 0; i < rows.size(); ++i) {
        std::vector<std::string>& row = rows[i];
        std::vector<std::string> padded;

        if (mode == PadMode::Front) {
            padded.assign(widest - row.size(), std::string());
            padded.insert(padded.end(), row.begin(), row.end());
        } else if (mode == PadMode::Rear) {
            padded = row;
            padded.resize(widest);
        } else {
            // Bresenham-style even spread of the row's nodes across `widest`
            // cells. The bounds check on `taken` is belt-and-braces: the error
            // term should never call for more nodes than the row holds.
            const int step = 2 * static_cast<int>(row.size());
            int error = step - static_cast<int>(widest);
            size_t taken = 0;
            for (size_t s = 0; s < widest; ++s) {
                if (error > 0 && taken < row.size()) {
                    padded.push_back(row[taken]);
                    ++taken;
                    error -= 2 * static_cast<int>(widest);
                } else {
                    padded.emplace_back();
                }
                error += step;
            }
        }

        strands[StorageIndex(strands.size(), i)] = CollapseCells(padded);
    }
}

void RemoveDuplicatesInRow(std::vector<std::string>& strands, int displayRow, bool suppress)
{
    if (displayRow < 0 || static_cast<size_t>(displayRow) >= strands.size()) {
        return;
    }
    const size_t idx = StorageIndex(strands.size(), static_cast<size_t>(displayRow));
    auto cells = ExpandCells(strands[idx]);

    if (suppress) {
        std::set<std::string> seen;
        for (auto& cell : cells) {
            if (cell.empty() || cell == "0") {
                continue;
            }
            if (!seen.insert(cell).second) {
                cell.clear();
            }
        }
    } else {
        auto end = cells.end();
        for (auto it = cells.begin(); it != end; ++it) {
            end = std::remove(it + 1, end, *it);
        }
        cells.erase(end, cells.end());
    }

    strands[idx] = CollapseCells(cells);
}

void RemoveAllDuplicates(std::vector<std::string>& strands, bool leftToRight, bool suppress)
{
    if (strands.empty()) {
        return;
    }

    std::vector<std::vector<std::string>> rows;
    size_t widest = 0;
    for (size_t i = 0; i < strands.size(); ++i) {
        rows.push_back(ExpandCells(strands[StorageIndex(strands.size(), i)]));
        widest = std::max(widest, rows.back().size());
    }

    // Repeats are either blanked in place or tagged for removal; tagging keeps
    // the column alignment intact while the whole grid is scanned.
    static const std::string kDropped = "\x01drop";
    std::set<std::string> seen;

    auto visit = [&](std::string& cell) {
        if (cell.empty() || cell == "0") {
            return;
        }
        if (!seen.insert(cell).second) {
            cell = suppress ? std::string() : kDropped;
        }
    };

    if (leftToRight) {
        for (size_t col = 0; col < widest; ++col) {
            for (auto& row : rows) {
                if (col < row.size()) {
                    visit(row[col]);
                }
            }
        }
    } else {
        for (auto& row : rows) {
            for (auto& cell : row) {
                visit(cell);
            }
        }
    }

    for (size_t i = 0; i < rows.size(); ++i) {
        auto& row = rows[i];
        row.erase(std::remove(row.begin(), row.end(), kDropped), row.end());
        strands[StorageIndex(strands.size(), i)] = CollapseCells(row);
    }
}

// ---------------------------------------------------------------------------
// Geometric point ordering
// ---------------------------------------------------------------------------

namespace {

constexpr float kPi = 3.14159265358979323846f;
// Nudge the start angle just off the axis so a node sitting exactly on it
// sorts to the intended end rather than landing on the wrap boundary.
constexpr float kStartNudge = 0.02f;

std::pair<float, float> CoordOf(const std::map<int, std::pair<float, float>>& coords, int node)
{
    const auto it = coords.find(node);
    return it == coords.end() ? std::pair<float, float>(0.0f, 0.0f) : it->second;
}

// Parse a cell as a node index; returns 0 for blanks and non-numeric text.
int NodeOf(const std::string& cell)
{
    if (cell.empty()) {
        return 0;
    }
    char* end = nullptr;
    const long v = std::strtol(cell.c_str(), &end, 10);
    if (end == cell.c_str()) {
        return 0;
    }
    return static_cast<int>(v);
}

std::pair<float, float> Centroid(const std::map<int, std::pair<float, float>>& coords,
                                 const std::set<int>& nodes)
{
    if (nodes.empty()) {
        return { 0.0f, 0.0f };
    }
    float cx = 0.0f;
    float cy = 0.0f;
    for (const int n : nodes) {
        const auto c = CoordOf(coords, n);
        cx += c.first;
        cy += c.second;
    }
    return { cx / nodes.size(), cy / nodes.size() };
}

// Reorder one expanded strand. Each node carries the run of blanks that
// preceded it so the gaps travel with their node; trailing blanks are moved
// to the front, which is what the desktop did.
std::string OrderOneStrand(const std::map<int, std::pair<float, float>>& coords,
                           const std::string& expanded,
                           std::pair<float, float> centre,
                           bool radial,
                           float startAngle,
                           bool reverse)
{
    std::vector<std::pair<int, int>> nodeAndBlanksBefore;
    int blanks = 0;
    for (const auto& cell : Split(expanded, ',')) {
        if (cell.empty() || cell == "0") {
            ++blanks;
        } else {
            nodeAndBlanksBefore.emplace_back(NodeOf(cell), blanks);
            blanks = 0;
        }
    }

    if (nodeAndBlanksBefore.empty()) {
        return expanded;
    }

    if (radial) {
        std::sort(nodeAndBlanksBefore.begin(), nodeAndBlanksBefore.end(),
                  [&](const std::pair<int, int>& l, const std::pair<int, int>& r) {
                      const auto cl = CoordOf(coords, l.first);
                      const auto cr = CoordOf(coords, r.first);
                      const float dxl = cl.first - centre.first;
                      const float dyl = cl.second - centre.second;
                      const float dxr = cr.first - centre.first;
                      const float dyr = cr.second - centre.second;
                      const float dl = dxl * dxl + dyl * dyl;
                      const float dr = dxr * dxr + dyr * dyr;
                      return reverse ? (dl > dr) : (dl < dr);
                  });
    } else {
        std::sort(nodeAndBlanksBefore.begin(), nodeAndBlanksBefore.end(),
                  [&](const std::pair<int, int>& l, const std::pair<int, int>& r) {
                      const auto cl = CoordOf(coords, l.first);
                      const auto cr = CoordOf(coords, r.first);
                      float angl = std::atan2(cl.second - centre.second, cl.first - centre.first) - startAngle;
                      float angr = std::atan2(cr.second - centre.second, cr.first - centre.first) - startAngle;
                      while (angl < 0) {
                          angl += 2 * kPi;
                      }
                      while (angr < 0) {
                          angr += 2 * kPi;
                      }
                      return reverse ? (angl < angr) : (angl > angr);
                  });
    }

    nodeAndBlanksBefore[0].second += blanks;

    std::string res;
    for (const auto& [node, before] : nodeAndBlanksBefore) {
        res.append(static_cast<size_t>(before), ',');
        res += std::to_string(node);
        res += ",";
    }
    return NodeUtils::CompressNodes(res.substr(0, res.size() - 1));
}

} // namespace

std::vector<std::string> OrderPointsChoices()
{
    static const char* kStarts[] = {
        "Start From Model Inside", "Start From Model Outside",
        "Start From Model CCW", "Start From Model CW",
        "Start From Up", "Start From Down", "Start From Left", "Start From Right"
    };
    static const char* kSpins[] = {
        "CW Around Model Center", "CW Around Submodel Center", "CW Around Strand Center",
        "CCW Around Model Center", "CCW Around Submodel Center", "CCW Around Strand Center"
    };

    std::vector<std::string> out;
    for (const char* start : kStarts) {
        for (const char* spin : kSpins) {
            out.push_back(std::string("Circumferential|") + start + "|" + spin);
        }
    }
    for (const char* dir : { "From Near To Far", "From Far To Near" }) {
        for (const char* ctr : { "Model Center", "Submodel Center", "Strand Center" }) {
            out.push_back(std::string("Radial|") + dir + "|" + ctr);
        }
    }
    return out;
}

bool ParseOrderPointsChoice(const std::string& choice, OrderPointsOptions& out)
{
    const auto parts = Split(choice, '|');
    if (parts.size() != 3) {
        return false;
    }
    out = OrderPointsOptions();

    if (parts[0] == "Radial") {
        out.radial = true;
        if (parts[2] == "Model Center") {
            out.center = OrderCenter::Model;
        } else if (parts[2] == "Submodel Center") {
            out.center = OrderCenter::Submodel;
        } else if (parts[2] == "Strand Center") {
            out.center = OrderCenter::Strand;
        } else {
            return false;
        }
        if (parts[1] == "From Far To Near") {
            out.reverse = true;
        } else if (parts[1] != "From Near To Far") {
            return false;
        }
        return true;
    }

    if (parts[0] != "Circumferential") {
        return false;
    }

    if (parts[2] == "CW Around Model Center") {
        out.center = OrderCenter::Model;
    } else if (parts[2] == "CCW Around Model Center") {
        out.center = OrderCenter::Model;
        out.reverse = true;
    } else if (parts[2] == "CW Around Submodel Center") {
        out.center = OrderCenter::Submodel;
    } else if (parts[2] == "CCW Around Submodel Center") {
        out.center = OrderCenter::Submodel;
        out.reverse = true;
    } else if (parts[2] == "CW Around Strand Center") {
        out.center = OrderCenter::Strand;
    } else if (parts[2] == "CCW Around Strand Center") {
        out.center = OrderCenter::Strand;
        out.reverse = true;
    } else {
        return false;
    }

    if (parts[1] == "Start From Up") {
        out.startAngle = kPi / 2;
    } else if (parts[1] == "Start From Down") {
        out.startAngle = 3 * kPi / 2;
    } else if (parts[1] == "Start From Right") {
        out.startAngle = 0;
    } else if (parts[1] == "Start From Left") {
        out.startAngle = kPi;
    } else if (parts[1] == "Start From Model Inside") {
        out.startAngle = 0;
        out.startModelRelative = true;
    } else if (parts[1] == "Start From Model Outside") {
        out.startAngle = kPi;
        out.startModelRelative = true;
    } else if (parts[1] == "Start From Model CW") {
        out.startAngle = kPi / 2;
        out.startModelRelative = true;
    } else if (parts[1] == "Start From Model CCW") {
        out.startAngle = 3 * kPi / 2;
        out.startModelRelative = true;
    } else {
        return false;
    }

    out.startAngle += out.reverse ? -kStartNudge : kStartNudge;
    return true;
}

void OrderPoints(std::vector<std::string>& strands,
                 const std::map<int, std::pair<float, float>>& coords,
                 const OrderPointsOptions& opts,
                 int firstDisplayRow,
                 int lastDisplayRow)
{
    if (strands.empty() || coords.empty()) {
        return;
    }

    const std::pair<float, float> modelCentre = Centroid(
        coords, [&] {
            std::set<int> all;
            for (const auto& [node, _] : coords) {
                all.insert(node);
            }
            return all;
        }());

    std::set<int> selected;
    for (const auto& strand : strands) {
        for (const auto& cell : ExpandCells(strand)) {
            if (cell.empty() || cell == "0") {
                continue;
            }
            selected.insert(NodeOf(cell));
        }
    }
    if (selected.empty()) {
        return;
    }
    const std::pair<float, float> submodelCentre = Centroid(coords, selected);

    const int lastRow = static_cast<int>(strands.size()) - 1;
    firstDisplayRow = std::max(firstDisplayRow, 0);
    lastDisplayRow = std::min(lastDisplayRow, lastRow);

    for (int row = firstDisplayRow; row <= lastDisplayRow; ++row) {
        const size_t idx = StorageIndex(strands.size(), static_cast<size_t>(row));
        const std::string expanded = NodeUtils::ExpandNodes(strands[idx]);

        std::set<int> rowNodes;
        for (const auto& cell : Split(expanded, ',')) {
            if (cell.empty() || cell == "0") {
                continue;
            }
            rowNodes.insert(NodeOf(cell));
        }
        if (rowNodes.empty()) {
            continue;
        }
        const std::pair<float, float> rowCentre = Centroid(coords, rowNodes);

        std::pair<float, float> centre = modelCentre;
        if (opts.center == OrderCenter::Submodel) {
            centre = submodelCentre;
        } else if (opts.center == OrderCenter::Strand) {
            centre = rowCentre;
        }

        float angle = opts.startAngle;
        if (opts.startModelRelative) {
            float toModel = std::atan2(modelCentre.second - rowCentre.second,
                                       modelCentre.first - rowCentre.first);
            if (toModel < 0) {
                toModel += 2 * kPi;
            }
            angle += toModel;
        }

        strands[idx] = OrderOneStrand(coords, expanded, centre, opts.radial, angle, opts.reverse);
    }
}

} // namespace submodel_ops
