/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include "AutoMapper.h"

#include "ImportMappingNode.h"
#include "models/Model.h"
#include "render/RenderContext.h"
#include "utils/string_utils.h"

#include <spdlog/spdlog.h>

#include <regex>
#include <string>

namespace {

// Strip the same set of separators / punctuation that the desktop dialog's
// AggressiveAutomap removed. Kept identical so behaviour matches the
// existing wx version.
std::string StripPunctuation(const std::string& in) {
    std::string out;
    out.reserve(in.size());
    for (char c : in) {
        switch (c) {
        case ' ': case '-': case '_': case '(': case ')':
        case ':': case ';': case '\\': case '|': case '{': case '}':
        case '[': case ']': case '+': case '=': case '*': case '^':
        case '#': case ',': case '.':
            break;
        default:
            out.push_back(c);
            break;
        }
    }
    return out;
}

std::vector<std::string> SplitSlash(const std::string& s) {
    std::vector<std::string> parts;
    std::string cur;
    for (char c : s) {
        if (c == '/') {
            parts.push_back(cur);
            cur.clear();
        } else {
            cur.push_back(c);
        }
    }
    parts.push_back(cur);
    return parts;
}

} // namespace

namespace AutoMapper {

bool MatchNorm(const std::string& target, const std::string& candidate,
               const std::string&, const std::string&,
               const std::list<std::string>&) {
    return Lower(Trim(target)) == candidate;
}

bool MatchAggressive(const std::string& target, const std::string& candidate,
                     const std::string&, const std::string&,
                     const std::list<std::string>& aliases) {
    std::string strippedCandidate = StripPunctuation(candidate);
    if (StripPunctuation(Lower(Trim(target))) == strippedCandidate) {
        return true;
    }
    // OldName alias prefix — the rename history form. Plain string compare
    // (no punctuation strip) so users with renames don't get over-matched.
    for (const auto& it : aliases) {
        if (Lower(Trim(it)) == "oldname:" + candidate) {
            return true;
        }
    }
    // Plain alias match — try both punctuation-stripped and exact, so an
    // alias "Mega Tree (Vendor)" can match a vendor candidate
    // "megatreevendor" without the user needing to maintain a punctuation-
    // free duplicate alias.
    for (const auto& it : aliases) {
        std::string aliasNorm = Lower(Trim(it));
        if (aliasNorm == candidate) return true;
        if (StripPunctuation(aliasNorm) == strippedCandidate) return true;
    }
    return false;
}

bool MatchRegex(const std::string& target, const std::string& candidate,
                const std::string& pattern, const std::string& replacement,
                const std::list<std::string>&) {
    if (Lower(Trim(candidate)) != Lower(Trim(replacement))) {
        return false;
    }
    static std::regex r;
    static std::string lastPattern;
    static bool valid = false;
    try {
        if (pattern != lastPattern) {
            r = std::regex(pattern, std::regex::ECMAScript | std::regex::icase);
            lastPattern = pattern;
            valid = true;
        }
    } catch (const std::regex_error&) {
        valid = false;
        lastPattern = pattern;
    }
    if (!valid) {
        return false;
    }
    return std::regex_match(target, r);
}

void Run(const std::vector<ImportMappingNode*>& roots,
         const std::vector<AvailableSource>& available,
         RenderContext& renderContext,
         MatcherFn lambda_model, MatcherFn lambda_strand, MatcherFn lambda_node,
         const std::string& extra1, const std::string& extra2,
         const std::string& mg,
         bool selectOnly,
         const std::unordered_set<const ImportMappingNode*>& selectedTargets) {
    bool selectMapAvail = false;
    bool selectMapTarget = false;
    if (selectOnly) {
        for (const auto& a : available) {
            if (a.selected) { selectMapAvail = true; break; }
        }
        selectMapTarget = !selectedTargets.empty();
    }

    for (auto* model : roots) {
        if (model == nullptr) {
            spdlog::warn("AutoMapper::Run: null root encountered, skipping");
            continue;
        }
        bool isTargetSelected = selectedTargets.count(model) != 0;
        if (selectMapTarget && !isTargetSelected) {
            continue;
        }

        auto aliases = model->GetAliases();
        bool typeMatch = (model->IsGroup() && (mg == "B" || mg == "G")) ||
                         (!model->IsGroup() && (mg == "B" || mg == "M"));
        if (!typeMatch) continue;

        // Cache the layout model once per destination model so both the
        // slashed-path strand-alias lookup and the fallback can use it
        // without repeated map lookups.
        Model* layoutModel = renderContext.GetModel(model->GetCoreModel());

        for (const auto& src : available) {
            if (selectMapAvail && !src.selected) continue;
            const std::string& availName = src.canonicalName;

            if (availName.find('/') != std::string::npos) {
                auto parts = SplitSlash(availName);
                if (lambda_model(model->GetCoreModel(), parts[0], extra1, extra2, aliases)) {
                    // matched the model name ... need to look at strands and submodels
                    for (unsigned int k = 0; k < model->GetChildCount(); ++k) {
                        auto* strand = model->GetNthChild(k);
                        if (strand == nullptr) continue;
                        // Use the submodel's own aliases (from the layout) for
                        // strand matching so that e.g. a submodel aliased
                        // "15 spinners - all" correctly matches that part of
                        // "SS Spinner Left/15 Spinners - All".
                        std::list<std::string> strandAliases;
                        if (layoutModel != nullptr) {
                            Model* sm2 = layoutModel->GetSubModel(strand->GetCoreStrand());
                            if (sm2 != nullptr) {
                                strandAliases = sm2->GetAliases();
                            }
                        }
                        const auto& strandAliasesToUse = strandAliases.empty() ? aliases : strandAliases;
                        if (!lambda_strand(strand->GetCoreStrand(), parts[1], extra1, extra2, strandAliasesToUse)) {
                            continue;
                        }
                        if (parts.size() == 2) {
                            if (strand->GetMapping().empty()) {
                                strand->Map(src.displayName, "Strand");
                            }
                        } else {
                            for (unsigned int m = 0; m < strand->GetChildCount(); ++m) {
                                auto* node = strand->GetNthChild(m);
                                if (node == nullptr) continue;
                                if (!node->GetMapping().empty()) continue;
                                if (lambda_node(node->GetCoreNode(), parts[2], extra1, extra2, aliases)) {
                                    if (parts.size() == 3) {
                                        node->Map(src.displayName, "Node");
                                    }
                                }
                            }
                        }
                    }
                }
            } else {
                // match model to model
                if (model->GetMapping().empty() &&
                    lambda_model(model->GetCoreModel(), availName, extra1, extra2, aliases)) {
                    model->Map(src.displayName, src.modelType);
                }
            }
        }

    }

    // Process selected submodels independently
    if (selectMapTarget) {
        for (auto* model : roots) {
            if (model == nullptr) continue;
            for (unsigned int k = 0; k < model->GetChildCount(); ++k) {
                auto* submodel = model->GetNthChild(k);
                if (submodel == nullptr) continue;
                bool isSubmodelSelected = selectedTargets.count(submodel) != 0;
                if (selectMapTarget && !isSubmodelSelected) continue;

                for (const auto& src : available) {
                    if (selectMapAvail && !src.selected) continue;
                    const std::string& availName = src.canonicalName;
                    Model* layoutModel = renderContext.GetModel(model->GetCoreModel());
                    if (layoutModel == nullptr) continue;
                    const auto& mAliases = layoutModel->GetAliases();
                    Model* sm = layoutModel->GetSubModel(submodel->GetCoreStrand());
                    if (sm == nullptr) continue;
                    const auto& smAliases = sm->GetAliases();
                    if (!submodel->GetMapping().empty()) continue;

                    if (lambda_strand(submodel->GetModelName(), availName, extra1, extra2, smAliases)) {
                        submodel->Map(src.displayName, "SubModel");
                    } else {
                        for (const auto& modelAlias : mAliases) {
                            if (lambda_strand(modelAlias + "/" + submodel->GetCoreStrand(),
                                              availName, extra1, extra2, smAliases)) {
                                submodel->Map(src.displayName, "SubModel");
                                break;
                            }
                        }
                    }
                }
            }
        }
    }
}

void RunSubModelFallback(const std::vector<ImportMappingNode*>& roots,
                         const std::vector<AvailableSource>& available,
                         RenderContext& renderContext,
                         bool selectOnly,
                         const std::unordered_set<const ImportMappingNode*>& selectedTargets) {
    bool selectMapAvail = false;
    bool selectMapTarget = false;
    if (selectOnly) {
        for (const auto& a : available) {
            if (a.selected) { selectMapAvail = true; break; }
        }
        selectMapTarget = !selectedTargets.empty();
    }

    for (auto* model : roots) {
        if (model == nullptr) continue;
        if (!model->GetMapping().empty()) continue;
        if (selectMapTarget && selectedTargets.count(model) == 0) continue;

        Model* layoutModel = renderContext.GetModel(model->GetCoreModel());

        // Step 1: match unmapped submodels by name against non-slashed sources.
        for (unsigned int k = 0; k < model->GetChildCount(); ++k) {
            auto* sm = model->GetNthChild(k);
            if (sm == nullptr || !sm->GetMapping().empty()) continue;
            const std::string smName = Lower(Trim(sm->GetCoreStrand()));
            for (const auto& src : available) {
                if (selectMapAvail && !src.selected) continue;
                if (!sm->GetMapping().empty()) break;
                const std::string& availName = src.canonicalName;
                if (availName.find('/') != std::string::npos) continue;
                if (smName == availName) {
                    sm->Map(src.displayName, "Unknown");
                }
            }
        }

        // Step 2: match still-unmapped submodels by their layout aliases.
        if (layoutModel == nullptr) continue;
        for (unsigned int k = 0; k < model->GetChildCount(); ++k) {
            auto* sm = model->GetNthChild(k);
            if (sm == nullptr || !sm->GetMapping().empty()) continue;
            Model* sm2 = layoutModel->GetSubModel(sm->GetCoreStrand());
            if (sm2 == nullptr) continue;
            const auto& smAliases = sm2->GetAliases();
            if (smAliases.empty()) continue;
            for (const auto& src : available) {
                if (selectMapAvail && !src.selected) continue;
                if (!sm->GetMapping().empty()) break;
                const std::string& availName = src.canonicalName;
                if (availName.find('/') != std::string::npos) continue;
                const std::string strippedAvail = StripPunctuation(availName);
                for (const auto& alias : smAliases) {
                    std::string aliasNorm = Lower(Trim(alias));
                    if (aliasNorm == availName || StripPunctuation(aliasNorm) == strippedAvail) {
                        sm->Map(src.displayName, "Unknown");
                        break;
                    }
                }
            }
        }
    }
}

} // namespace AutoMapper
