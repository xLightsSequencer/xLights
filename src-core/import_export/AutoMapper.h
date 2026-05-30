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
#include <list>
#include <string>
#include <unordered_set>
#include <vector>

class ImportMappingNode;
class RenderContext;
struct AvailableSource;

namespace AutoMapper {

// Source-vs-destination matcher. Called per (destination_name, candidate)
// pair. Extra slots are reused by the regex matcher for the regex pattern
// and the substitution model name from .xmaphint files. Aliases are the
// destination model's aliases — checked by aggressive and norm strategies.
using MatcherFn = std::function<bool(const std::string& targetName,
                                     const std::string& candidate,
                                     const std::string& extra1,
                                     const std::string& extra2,
                                     const std::list<std::string>& aliases)>;

// Plain case-insensitive trimmed equality. The default matcher.
bool MatchNorm(const std::string& target, const std::string& candidate,
               const std::string& extra1, const std::string& extra2,
               const std::list<std::string>& aliases);

// Strips common punctuation/whitespace from both sides before comparing,
// also matches against `OldName:<candidate>` aliases. Used as the second
// pass after MatchNorm.
bool MatchAggressive(const std::string& target, const std::string& candidate,
                     const std::string& extra1, const std::string& extra2,
                     const std::list<std::string>& aliases);

// Regex matcher used by .xmaphint files: extra1 is the regex pattern,
// extra2 is the substitution candidate. Returns true if `target` matches
// the pattern AND `candidate` (lowered/trimmed) equals extra2.
bool MatchRegex(const std::string& target, const std::string& candidate,
                const std::string& pattern, const std::string& replacement,
                const std::list<std::string>& aliases);

// Run one auto-map pass over the destination tree.
//
// `roots` lists the top-level destination nodes (the desktop's
// `_dataModel`'s children, the iPad's mapping-tree roots).
// `available` is the source-candidate list (canonical-cased + selection
// state pre-computed by the caller).
// `renderContext` is used to look up aliases on submodels of the user's
// layout.
// `mg` is a one-character filter: "B" both, "M" models only, "G" groups
// only.
// `selectOnly` is set when the caller wants the run scoped to the
// `selectedTargets` / available `selected` flags.
// `selectedTargets` is the set of pointers to selected destination nodes.
void Run(const std::vector<ImportMappingNode*>& roots,
         const std::vector<AvailableSource>& available,
         RenderContext& renderContext,
         MatcherFn lambda_model, MatcherFn lambda_strand, MatcherFn lambda_node,
         const std::string& extra1, const std::string& extra2,
         const std::string& mg,
         bool selectOnly,
         const std::unordered_set<const ImportMappingNode*>& selectedTargets);

// Called once after all Run() passes complete. For destination models that
// didn't get a direct mapping, attempts to map their unmapped submodels in
// two steps:
//   1. Direct name match: submodel name vs non-slashed available name.
//   2. Alias match: submodel alias (from layout) vs non-slashed available name.
// Running this after the aggressive pass ensures correct alias-based slashed
// matches (e.g. "ModelAlias/SubmodelAlias") take priority over coincidental
// plain-name matches from step 1.
void RunSubModelFallback(const std::vector<ImportMappingNode*>& roots,
                         const std::vector<AvailableSource>& available,
                         RenderContext& renderContext,
                         bool selectOnly,
                         const std::unordered_set<const ImportMappingNode*>& selectedTargets);

} // namespace AutoMapper
