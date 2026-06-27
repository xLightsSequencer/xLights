#pragma once

#include <map>
#include <string>
#include <utility>
#include <vector>

// Rotational symmetrize generator for submodel node selections.
// Extracted wx-free from SubModelsDialog::Symmetrize so both the desktop
// dialog and the iPad submodel editor can share the algorithm.
namespace SubModelSymmetrize {

struct Options {
    int degreeOfSymmetry{8};   // how many rotational copies (incl. original)
    bool clockwise{false};     // rotation direction of the generated copies
    bool bottomToTop{false};   // build order: false = originals at top of grid
    bool squarifyAspect{true}; // correct a non-square node cloud before matching
    bool handleCenterNode{true};
};

struct Result {
    bool success{false};
    // Full replacement strand list (original strands + generated copies),
    // ordered per Options::bottomToTop. Only valid when success.
    std::vector<std::string> strands;
    std::vector<int> unmatchedNodes; // populated on failure
    std::string aspectAdvisory;      // non-empty when caller may want to squarify
};

// `coords` maps 1-based node index -> screen (x,y) (e.g. from
// Model::GetScreenLocations). `w`/`h` are the screen pixel extents used for
// spatial binning. `strands` is the submodel's current strand list (range
// strings). Returns generated + original strands or a failure with the list
// of nodes that could not be matched.
Result Symmetrize(const std::map<int, std::pair<float, float>>& coords,
                  int w, int h,
                  const std::vector<std::string>& strands,
                  const Options& opts);

// Returns true when the node cloud's aspect ratio is far enough from square
// that the caller should offer to squarify (mirrors the desktop prompt).
bool ShouldOfferSquarify(const std::map<int, std::pair<float, float>>& coords);

} // namespace SubModelSymmetrize
