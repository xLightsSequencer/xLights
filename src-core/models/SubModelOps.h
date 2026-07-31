#pragma once

/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include <functional>
#include <map>
#include <string>
#include <utility>
#include <vector>

// Geometry / node-index operations on a submodel's strand list, extracted
// wx-free from SubModelsPanel so the desktop panel and the iPad submodel
// editor run the same maths. Companion to SubModelSymmetrize, which already
// holds the rotational-symmetry generator.
//
// A "strand" is one row of the submodel's node grid, stored as a node-range
// string ("1-50,60,70-80"). Empty entries and "0" both mean "no node here";
// the two spellings are distinguishable and several operations convert
// between them deliberately.
//
// **Row order.** `strands[0]` is the BOTTOM row as the editor presents it, so
// display row r is `strands[size() - 1 - r]`. Every operation here that cares
// about row order uses that convention, matching the desktop panel exactly.
namespace submodel_ops {

// How MakeRowsUniform pads short rows out to the longest row's length.
enum class PadMode {
    Distribute, // spread the existing nodes evenly across the row
    Front,      // pad at the start
    Rear        // pad at the end
};

// Reverse one strand's node order, flipping "a-b" ranges to "b-a" so the
// wiring direction reverses with it. Entries that aren't a clean two-part
// range are copied through untouched.
std::string ReverseRow(const std::string& row);

// Apply `fn` to every strand, walking in display order (top row first) to
// match the desktop.
void TransformAllStrands(std::vector<std::string>& strands,
                         const std::function<std::string(const std::string&)>& fn);

// Per-strand string transforms, for use with TransformAllStrands.
std::string ExpandStrand(const std::string& strand);
std::string CompressStrand(const std::string& strand);
std::string BlanksToZeros(const std::string& strand);
std::string ZerosToBlanks(const std::string& strand);
std::string RemoveBlanksAndZeros(const std::string& strand);

// Mirror every strand left-to-right.
void FlipHorizontal(std::vector<std::string>& strands);

// Mirror the row order top-to-bottom. A single-row submodel is left alone.
void FlipVertical(std::vector<std::string>& strands);

// Renumber every node n -> (nodeCount + 1 - n), i.e. reverse the model's
// node numbering under the selection.
void ReverseNodes(std::vector<std::string>& strands, int nodeCount);

// Add `amount` to every node index, wrapping through the model's node count
// so the selection walks around the model. Negative shifts backwards.
void ShiftNodes(std::vector<std::string>& strands, int nodeCount, int amount);

// Transpose the grid: rows become columns.
void PivotRowsColumns(std::vector<std::string>& strands);

// Concatenate every strand into a single row, in display order.
void CombineStrands(std::vector<std::string>& strands);

// Pad every row out to the longest row's length.
void MakeRowsUniform(std::vector<std::string>& strands, PadMode mode);

// De-duplicate within one display row. `suppress` blanks the repeats in
// place (keeping the row's length and alignment); otherwise they are removed
// and the row shortens.
void RemoveDuplicatesInRow(std::vector<std::string>& strands, int displayRow, bool suppress);

// De-duplicate across the whole grid, scanning either left-to-right
// (column-major) or top-to-bottom (row-major). `suppress` blanks repeats in
// place; otherwise they are removed and rows shorten.
void RemoveAllDuplicates(std::vector<std::string>& strands, bool leftToRight, bool suppress);

} // namespace submodel_ops
