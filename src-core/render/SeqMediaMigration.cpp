/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include "SeqMediaMigration.h"

#include <algorithm>
#include <cctype>
#include <cstdlib>
#include <set>
#include <string>
#include <vector>

#include <log.h>

#include "../utils/FileUtils.h"
#include "../utils/UtilClasses.h"
#include "Effect.h"
#include "EffectLayer.h"
#include "Element.h"
#include "SequenceElements.h"
#include "SequenceMedia.h"

namespace seqmedia {

int ConvertGifVideoEffectsToPictures(SequenceElements& elements,
                                     const std::vector<MediaCompatibilityIssue>& gifIssues)
{
    // ffmpeg can transcode an animated GIF into mp4/mov, but the result is
    // typically poor (palette / dithering / fps metadata get mangled). The
    // PicturesEffect already plays animated GIFs natively, so for GIF-backed
    // Video effects we swap the effect type to Pictures and map the
    // parameters across.
    if (gifIssues.empty()) return 0;

    std::set<std::string> gifResolved;
    for (const auto& issue : gifIssues) {
        gifResolved.insert(issue.filePath);
    }

    int rewritten = 0;
    std::set<std::string> staleVideoKeys;
    std::set<std::string> newImageKeys;

    auto rewriteEffectLayers = [&](Element* elem) {
        for (int layer = 0; layer < (int)elem->GetEffectLayerCount(); ++layer) {
            EffectLayer* el = elem->GetEffectLayer(layer);
            for (int k = 0; k < el->GetEffectCount(); ++k) {
                Effect* ef = el->GetEffect(k);
                if (ef->GetEffectName() != "Video") continue;
                SettingsMap& sm = ef->GetSettings();
                const std::string stored = sm["E_FILEPICKERCTRL_Video_Filename"];
                if (stored.empty()) continue;

                std::string lower = stored;
                std::transform(lower.begin(), lower.end(), lower.begin(),
                               [](unsigned char c) { return std::tolower(c); });
                if (lower.size() < 4 || lower.substr(lower.size() - 4) != ".gif") continue;

                std::string resolved = FileUtils::FixFile("", stored);
                if (gifResolved.find(resolved) == gifResolved.end() &&
                    gifResolved.find(stored) == gifResolved.end()) {
                    continue;
                }

                // Pictures FrameRateAdj is 0..200 with no negative/reverse
                // support, so clamp non-positive Video_Speed values to 1.0.
                std::string videoSpeed = sm["E_TEXTCTRL_Video_Speed"];
                {
                    char* endp = nullptr;
                    const char* s = videoSpeed.c_str();
                    double sp = std::strtod(s, &endp);
                    if (endp == s || sp <= 0.0) videoSpeed = "1.0";
                }

                // Drop all E_* (Video-effect-specific) settings; preserve
                // B_* / T_* / X_* / palette which are layer-level and apply
                // to the Pictures effect equally.
                std::vector<std::string> toErase;
                for (const auto& it : sm) {
                    if (it.first.size() > 2 && it.first[0] == 'E' && it.first[1] == '_') {
                        toErase.push_back(it.first);
                    }
                }
                for (const auto& key : toErase) sm.erase(key);

                sm["E_TEXTCTRL_Pictures_Filename"] = stored;
                sm["E_TEXTCTRL_Pictures_FrameRateAdj"] = videoSpeed;
                sm["E_TEXTCTRL_Pictures_Speed"] = "1.0";
                sm["E_CHECKBOX_LoopGIF"] = "1";
                sm["E_CHECKBOX_SuppressGIFBackground"] = "1";
                sm["E_CHECKBOX_Pictures_PixelOffsets"] = "0";
                sm["E_CHECKBOX_Pictures_Shimmer"] = "0";
                sm["E_CHECKBOX_Pictures_TransparentBlack"] = "0";
                sm["E_CHECKBOX_Pictures_WrapX"] = "0";
                sm["E_TEXTCTRL_Pictures_TransparentBlack"] = "0";
                sm["E_CHOICE_Pictures_Direction"] = "none";
                sm["E_CHOICE_Scaling"] = "Scale To Fit";
                sm["E_SLIDER_PicturesXC"] = "0";
                sm["E_SLIDER_PicturesYC"] = "0";
                sm["E_SLIDER_Pictures_StartScale"] = "100";
                sm["E_SLIDER_Pictures_EndScale"] = "100";

                ef->SetEffectName("Pictures");
                ef->IncrementChangeCount();
                ++rewritten;

                staleVideoKeys.insert(stored);
                staleVideoKeys.insert(resolved);
                newImageKeys.insert(stored);
            }
        }
    };

    for (size_t e = 0; e < elements.GetElementCount(); ++e) {
        Element* elem = elements.GetElement(e);
        if (elem == nullptr) continue;
        rewriteEffectLayers(elem);
        if (elem->GetType() == ElementType::ELEMENT_TYPE_MODEL) {
            ModelElement* me = dynamic_cast<ModelElement*>(elem);
            for (int j = 0; j < me->GetStrandCount(); ++j) {
                StrandElement* se = me->GetStrand(j);
                rewriteEffectLayers(se);
            }
            for (int j = 0; j < me->GetSubModelAndStrandCount(); ++j) {
                Element* sme = me->GetSubModel(j);
                if (sme->GetType() == ElementType::ELEMENT_TYPE_SUBMODEL) {
                    rewriteEffectLayers(sme);
                }
            }
        }
    }

    if (rewritten > 0) {
        spdlog::info("Converted {} animated GIF Video effect(s) to Pictures effects", rewritten);

        // Move the GIF entries from the SequenceMedia video cache to the image
        // cache so the Sequence Settings → Media tab shows them under the
        // right type and the renderer's lookup hits the multi-frame
        // ImageCacheEntry path that PicturesEffect uses for animated GIFs.
        auto& seqMedia = elements.GetSequenceMedia();
        for (const auto& key : staleVideoKeys) {
            seqMedia.RemoveMedia(key);
        }
        for (const auto& key : newImageKeys) {
            seqMedia.GetImage(key);
        }
    }
    return rewritten;
}

}
