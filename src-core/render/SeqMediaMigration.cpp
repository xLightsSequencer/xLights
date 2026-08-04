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
#include <filesystem>
#include <set>
#include <string>
#include <vector>

#include <log.h>

#include "../media/VideoTranscoder.h"
#include "../utils/FileUtils.h"
#include "../utils/UtilClasses.h"
#include "Effect.h"
#include "EffectLayer.h"
#include "Element.h"
#include "SequenceElements.h"
#include "SequenceMedia.h"

namespace seqmedia {

namespace {

// Walk every effect layer a model element owns - its own, its strands, and its
// submodels - handing each layer to `fn`.
void ForEachEffectLayer(SequenceElements& elements, const std::function<void(EffectLayer*)>& fn)
{
    auto visit = [&](Element* elem) {
        if (elem == nullptr) return;
        for (int layer = 0; layer < (int)elem->GetEffectLayerCount(); ++layer) {
            fn(elem->GetEffectLayer(layer));
        }
    };
    for (size_t e = 0; e < elements.GetElementCount(); ++e) {
        Element* elem = elements.GetElement(e);
        if (elem == nullptr) continue;
        visit(elem);
        if (elem->GetType() != ElementType::ELEMENT_TYPE_MODEL) continue;
        ModelElement* me = dynamic_cast<ModelElement*>(elem);
        if (me == nullptr) continue;
        for (int j = 0; j < me->GetStrandCount(); ++j) {
            visit(me->GetStrand(j));
        }
        for (int j = 0; j < me->GetSubModelAndStrandCount(); ++j) {
            Element* sme = me->GetSubModel(j);
            if (sme != nullptr && sme->GetType() == ElementType::ELEMENT_TYPE_SUBMODEL) {
                visit(sme);
            }
        }
    }
}

}


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

    auto rewriteEffectLayers = [&](EffectLayer* el) {
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
    };

    ForEachEffectLayer(elements, rewriteEffectLayers);

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


VideoConversionResult ConvertIncompatibleVideos(SequenceElements& elements,
                                                const std::vector<MediaCompatibilityIssue>& issues,
                                                const FileProgressCallback& onFile,
                                                const FrameProgressCallback& onFrame)
{
    VideoConversionResult result;

    // Audio issues are skipped - they need re-encoding outside xLights.
    // Animated GIFs are split out: transcoding one to mp4/mov mangles the
    // palette and frame timing, so the owning Video effect becomes a Pictures
    // effect instead.
    std::vector<MediaCompatibilityIssue> gifIssues;
    std::vector<std::pair<std::string, std::string>> jobs; // (source, target)
    for (const auto& issue : issues) {
        if (!issue.isVideo || !issue.canConvert()) continue;
        if (issue.isAnimatedGif()) {
            gifIssues.push_back(issue);
            continue;
        }
        std::string target = VideoTranscoder::SuggestedOutputPath(issue.filePath);
        if (target == issue.filePath) {
            // Source is already .mov (e.g. qtrle codec) - don't overwrite it.
            std::filesystem::path p(issue.filePath);
            p.replace_filename(p.stem().string() + "_converted.mov");
            target = p.string();
        }
        jobs.emplace_back(issue.filePath, target);
    }

    result.gifEffectsConverted = ConvertGifVideoEffectsToPictures(elements, gifIssues);
    result.attempted = (int)jobs.size();
    if (jobs.empty()) return result;

    // A cancel aborts the transcode, which then reports a plain error - so
    // latch the caller's "stop" answer to tell the two apart afterwards.
    bool cancelled = false;
    VideoTranscoder::ProgressCallback frameCb;
    if (onFrame) {
        frameCb = [&](int frame, int total) -> bool {
            if (onFrame(frame, total)) return true;
            cancelled = true;
            return false;
        };
    }

    for (size_t i = 0; i < jobs.size(); ++i) {
        const auto& [src, dst] = jobs[i];
        if (onFile && !onFile(i, jobs.size(), src)) {
            result.cancelled = true;
            break;
        }
        std::string err = VideoTranscoder::Transcode(src, dst, frameCb);
        if (cancelled) {
            result.cancelled = true;
            std::error_code ec;
            std::filesystem::remove(dst, ec);
            break;
        }
        if (!err.empty()) {
            spdlog::error("Video conversion failed for {}: {}", src, err);
            result.failures.push_back(std::filesystem::path(src).filename().string() + ": " + err);
            // Drop any partial output so it isn't mistaken for a finished file.
            std::error_code ec;
            std::filesystem::remove(dst, ec);
        } else {
            result.convertedFiles[src] = dst;
        }
    }
    result.converted = (int)result.convertedFiles.size();

    if (result.cancelled) return result;

    // Repoint every Video effect whose resolved source matches something we
    // converted. Track the original stored keys so only those stale entries
    // are evicted from SequenceMedia - clearing the whole cache would drop
    // images, SVGs and audio too.
    std::set<std::string> staleCacheKeys;
    std::set<std::string> newCacheKeys;
    ForEachEffectLayer(elements, [&](EffectLayer* el) {
        for (int k = 0; k < el->GetEffectCount(); ++k) {
            Effect* ef = el->GetEffect(k);
            if (ef->GetEffectName() != "Video") continue;
            SettingsMap& sm = ef->GetSettings();
            const std::string stored = sm["E_FILEPICKERCTRL_Video_Filename"];
            if (stored.empty()) continue;
            auto it = result.convertedFiles.find(FileUtils::FixFile("", stored));
            if (it == result.convertedFiles.end()) continue;

            staleCacheKeys.insert(stored);
            staleCacheKeys.insert(FileUtils::FixFile("", stored));

            // Preserve the relative-vs-absolute shape of the stored value, but
            // take the destination's filename (it can differ, e.g. the
            // _converted.mov case above).
            std::filesystem::path storedPath(stored);
            std::string newStored;
            if (storedPath.is_absolute()) {
                newStored = it->second;
            } else {
                std::filesystem::path p = storedPath;
                p.replace_filename(std::filesystem::path(it->second).filename());
                newStored = p.string();
            }
            sm["E_FILEPICKERCTRL_Video_Filename"] = newStored;
            newCacheKeys.insert(newStored);
            ef->IncrementChangeCount();
            ++result.effectsUpdated;
        }
    });

    auto& seqMedia = elements.GetSequenceMedia();
    for (const auto& key : staleCacheKeys) {
        seqMedia.RemoveMedia(key);
    }
    // Pre-register the new files so they show up in the media list immediately
    // rather than only when the renderer first touches them.
    for (const auto& key : newCacheKeys) {
        seqMedia.GetVideo(key);
    }
    return result;
}

}
