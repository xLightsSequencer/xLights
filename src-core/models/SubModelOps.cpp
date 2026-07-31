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

} // namespace submodel_ops
