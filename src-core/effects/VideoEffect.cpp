/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include "../../include/video-16.xpm"
#include "../../include/video-24.xpm"
#include "../../include/video-32.xpm"
#include "../../include/video-48.xpm"
#include "../../include/video-64.xpm"

#include <cstdlib>
#include <filesystem>
#include <memory>
#include <mutex>
#include <unordered_map>
#include <spdlog/fmt/fmt.h>

#include "VideoEffect.h"
#include "UtilFunctions.h"
#include "media/AudioManager.h"
#include "../media/VideoReader.h"
#include "../render/Effect.h"
#include "../render/RenderBuffer.h"
#include "UtilClasses.h"
#include "../render/SequenceFile.h"
#include "../render/SequenceMedia.h"
#include "../render/RenderContext.h"
#include "../render/EffectLayer.h"
#include "../render/Element.h"
#include "../render/SequenceElements.h"
#include "../models/Model.h"
#include "UtilFunctions.h"
#include "utils/ExternalHooks.h"

#include "utils/FileUtils.h"

#include "Parallel.h"
#include "ispc/VideoFunctions.ispc.h"

#include <log.h>

// Fallback defaults (used until OnMetadataLoaded replaces them with Video.json values).
double VideoEffect::sSpeedDefault = 1.0;
double VideoEffect::sSpeedMin = -1000;
double VideoEffect::sSpeedMax = 1000;
int VideoEffect::sSpeedDivisor = 100;
int VideoEffect::sCropMin = 0;
int VideoEffect::sCropMax = 100;
int VideoEffect::sCropLeftDefault = 0;
int VideoEffect::sCropRightDefault = 100;
int VideoEffect::sCropTopDefault = 100;
int VideoEffect::sCropBottomDefault = 0;
double VideoEffect::sStartTimeDefault = 0.0;
int VideoEffect::sSampleSpacingDefault = 0;
bool VideoEffect::sSyncAudioDefault = false;
bool VideoEffect::sAspectRatioDefault = false;
std::string VideoEffect::sDurationTreatmentDefault = "Normal";

// Frame-parallel classification support. A Video frame is a pure function of
// curPeriod for the stateless duration treatments, but only when the reader
// serves frames position-independently (AVFoundation: shared per-file decoder
// + pts-indexed frame cache). FFmpeg readers decode forward from their current
// position, so cloned per-frame readers would each re-decode the stream and
// aren't provably order-independent. Which impl a file gets is only known once
// a reader is opened, so classification is per-file: unknown files stay
// Stateful, Render() records the impl type on open (keyed on the raw settings
// filename — the only name classification can see), and later frames of the
// effect classify Pure. The record is sticky-false so a file that ever fell
// back to FFmpeg never windows.
namespace {
std::mutex sFrameIndependentLock;
std::unordered_map<std::string, bool> sFrameIndependentFiles;

void NoteVideoFileFrameIndependence(const std::string& settingsFilename, bool independent) {
    if (settingsFilename.empty()) {
        return;
    }
    std::lock_guard<std::mutex> lk(sFrameIndependentLock);
    auto it = sFrameIndependentFiles.find(settingsFilename);
    if (it == sFrameIndependentFiles.end()) {
        sFrameIndependentFiles.emplace(settingsFilename, independent);
    } else {
        it->second = it->second && independent;
    }
}

bool IsVideoFileFrameIndependent(const std::string& settingsFilename) {
    std::lock_guard<std::mutex> lk(sFrameIndependentLock);
    auto it = sFrameIndependentFiles.find(settingsFilename);
    return it != sFrameIndependentFiles.end() && it->second;
}
} // namespace

RenderableEffect::FrameParallelism VideoEffect::GetFrameParallelism(const SettingsMap& settings) const
{
    // Opt-in (XL_VIDEO_PARALLEL=1) until the AVFoundation bridge handles the
    // window access pattern efficiently. Windowed video is byte-identical
    // (gated 56/56 both axes 2026-07-20) but measured SLOWER wall-clock on
    // every video sequence: a cache-miss decode holds the SharedDecoder's
    // unique lock, stalling all cache-hit clones of that file (and the shared
    // window pool with them); two rows on one file sit at the eviction
    // boundary of the 64-frame cache, and each evicted-frame miss re-decodes
    // from the previous H.264 keyframe; and the per-handle repeated-frame
    // fast path never hits when consecutive frames land on different clones.
    // Decode-outside-the-mutex + corridor-aware eviction are the follow-up
    // (plans/render-perf/02, ENGINE.md §9); flip the default when they land.
    static const bool sParallelVideo = []() {
        const char* e = getenv("XL_VIDEO_PARALLEL");
        return e != nullptr && *e != '0';
    }();
    if (!sParallelVideo) {
        return FrameParallelism::Stateful;
    }
    if (settings.GetBool("CHECKBOX_SynchroniseWithAudio", sSyncAudioDefault)) {
        // Reads the sequence's media file, not the filename setting the
        // registry is keyed on.
        return FrameParallelism::Stateful;
    }
    std::string dt = settings.Get("CHOICE_Video_DurationTreatment", sDurationTreatmentDefault);
    if (dt != "Normal" && dt != "Slow/Accelerate") {
        // Loop counts end-of-video wraps across frames; Manual/Manual and
        // Loop integrate the (value-curvable) speed frame by frame.
        return FrameParallelism::Stateful;
    }
    if (settings.GetBool("CHECKBOX_Video_AspectRatio", sAspectRatioDefault)) {
        // With animated crops the serial path retargets via Resize(), which
        // sets exact dimensions (dropping the aspect fit), while a fresh
        // clone reader re-applies the fit — different scaled output.
        static const char* const cropVCs[] = {
            "VALUECURVE_Video_CropLeft", "VALUECURVE_Video_CropRight",
            "VALUECURVE_Video_CropTop", "VALUECURVE_Video_CropBottom"
        };
        for (const char* vc : cropVCs) {
            if (settings.Get(vc, "").find("Active=TRUE") != std::string::npos) {
                return FrameParallelism::Stateful;
            }
        }
    }
    return IsVideoFileFrameIndependent(settings.Get("FILEPICKERCTRL_Video_Filename", ""))
               ? FrameParallelism::Pure
               : FrameParallelism::Stateful;
}

VideoEffect::VideoEffect(int id) : RenderableEffect(id, "Video", video_16, video_24, video_32, video_48, video_64)
{
}

VideoEffect::~VideoEffect()
{
}

void VideoEffect::OnMetadataLoaded()
{
    sSpeedDefault = GetDoubleDefault("Video_Speed", sSpeedDefault);
    sSpeedMin = GetMinFromMetadata("Video_Speed", sSpeedMin);
    sSpeedMax = GetMaxFromMetadata("Video_Speed", sSpeedMax);
    sSpeedDivisor = GetDivisorFromMetadata("Video_Speed", sSpeedDivisor);
    // All crop sliders share the same min/max range in Video.json.
    sCropMin = (int)GetMinFromMetadata("Video_CropLeft", sCropMin);
    sCropMax = (int)GetMaxFromMetadata("Video_CropLeft", sCropMax);
    sCropLeftDefault = GetIntDefault("Video_CropLeft", sCropLeftDefault);
    sCropRightDefault = GetIntDefault("Video_CropRight", sCropRightDefault);
    sCropTopDefault = GetIntDefault("Video_CropTop", sCropTopDefault);
    sCropBottomDefault = GetIntDefault("Video_CropBottom", sCropBottomDefault);
    sStartTimeDefault = GetDoubleDefault("Video_Starttime", sStartTimeDefault);
    sSampleSpacingDefault = GetIntDefault("SampleSpacing", sSampleSpacingDefault);
    sSyncAudioDefault = GetBoolDefault("SynchroniseWithAudio", sSyncAudioDefault);
    sAspectRatioDefault = GetBoolDefault("Video_AspectRatio", sAspectRatioDefault);
    sDurationTreatmentDefault = GetStringDefault("Video_DurationTreatment", sDurationTreatmentDefault);
}

std::list<std::string> VideoEffect::CheckEffectSettings(const SettingsMap& settings, AudioManager* media, Model* model, Effect* eff, bool renderCache)
{
    std::list<std::string> res = RenderableEffect::CheckEffectSettings(settings, media, model, eff, renderCache);

    std::string filename = FileUtils::FixFile("", settings.Get("E_FILEPICKERCTRL_Video_Filename", ""));

    if (settings.GetBool("E_CHECKBOX_SynchroniseWithAudio", 0) == 1) {
        if (media != nullptr) {
            filename = media->FileName();
        }
    }

    if (filename.empty()) {
        res.push_back(fmt::format("    ERR: Video effect video file '{}' does not exist. Model '{}', Start {}", filename, model->GetFullName(), FORMATTIME(eff->GetStartTimeMS())));
    } else {
        auto& mm = eff->GetParentEffectLayer()->GetParentElement()->GetSequenceElements()->GetSequenceMedia();
        auto videoEntry = mm.GetVideo(filename);
        videoEntry->MarkIsUsed();

        if (videoEntry->GetResolvedPath().empty()) {
            res.push_back(fmt::format("    ERR: Video effect video file '{}' does not exist. Model '{}', Start {}", filename, model->GetFullName(), FORMATTIME(eff->GetStartTimeMS())));
        } else {
            if (!videoEntry->IsEmbedded()) {
                if (!FileUtils::IsFileInShowDir(std::string(), filename)) {
                    res.push_back(fmt::format("    WARN: Video effect video file '{}' not under show directory. Model '{}', Start {}", filename, model->GetFullName(), FORMATTIME(eff->GetStartTimeMS())));
                }
            }

            VideoReader* videoreader = new VideoReader(videoEntry->GetResolvedPath(), 100, 100, false, true, true);
            if (videoreader == nullptr || videoreader->GetLengthMS() == 0) {
                res.push_back(fmt::format("    ERR: Video effect video file '{}' could not be understood. Format may not be supported. Model '{}', Start {}", filename, model->GetFullName(), FORMATTIME(eff->GetStartTimeMS())));
            } else if (videoreader != nullptr) {
                double starttime = settings.GetDouble("E_TEXTCTRL_Video_Starttime", sStartTimeDefault);
                std::string treatment = settings.Get("E_CHOICE_Video_DurationTreatment", sDurationTreatmentDefault);

                if (treatment == "Normal") {
                    int videoduration = videoreader->GetLengthMS() - starttime;
                    int effectduration = eff->GetEndTimeMS() - eff->GetStartTimeMS();
                    if (videoduration < effectduration) {
                        res.push_back(fmt::format("    WARN: Video effect video file '{}' is shorter {} than effect duration {}. Model '{}', Start {}", filename, FORMATTIME(videoduration), FORMATTIME(effectduration), model->GetFullName(), FORMATTIME(eff->GetStartTimeMS())));
                    }
                }

                if (!renderCache) {
                    int vh = videoreader->GetHeight();
                    int vw = videoreader->GetHeight();

#define VIDEOSIZETHRESHOLD 10
                    if (vh > VIDEOSIZETHRESHOLD * model->GetDefaultBufferHt() || vw > VIDEOSIZETHRESHOLD * model->GetDefaultBufferWi()) {
                        float scale = std::max((float)vh / model->GetDefaultBufferHt(), (float)vw / model->GetDefaultBufferWi());
                        res.push_back(fmt::format("    WARN: Video effect video file '{}' is {:.1f} times the height or width of the model ... xLights is going to need to do lots of work to resize the video. Model '{}', Start {}", filename, scale, model->GetFullName(), FORMATTIME(eff->GetStartTimeMS())));
                    }
                }
            }

            if (videoreader != nullptr) {
                delete videoreader;
            }
        }
    }

    std::string bufferstyle = settings.Get("B_CHOICE_BufferStyle", "Default");
    std::string transform = settings.Get("B_CHOICE_BufferTransform", "None");
    std::string camera = settings.Get("B_CHOICE_PerPreviewCamera", "2D");
    int w, h;
    model->GetBufferSize(bufferstyle, camera, transform, w, h, settings.GetInt("B_SPINCTRL_BufferStagger", 0));

    if (w < 2 || h < 2)
    {
        res.push_back(fmt::format("    ERR: Video effect video file '{}' cannot render onto model as it is not high or wide enough ({},{}). Model '{}', Start {}", filename, w, h, model->GetFullName(), FORMATTIME(eff->GetStartTimeMS())));
    }

    return res;
}

bool VideoEffect::IsVideoFile(std::string filename)
{
    return VideoReader::IsVideoFile(filename);
}

void VideoEffect::adjustSettings(const std::string &version, Effect *effect, bool removeDefaults)
{
    // give the base class a chance to adjust any settings
    if (RenderableEffect::needToAdjustSettings(version))
    {
        RenderableEffect::adjustSettings(version, effect, removeDefaults);
    }

    SettingsMap &settings = effect->GetSettings();

    // if the old loop setting is prsent then clear it and change the duration treatment
    bool loop = settings.GetBool("E_CHECKBOX_Video_Loop", false);
    if (loop)
    {
        settings["E_CHOICE_Video_DurationTreatment"] = "Loop";
        settings.erase("E_CHECKBOX_Video_Loop");
    }

    if (settings.Contains("E_SLIDER_Video_Starttime")) {
        settings.erase("E_SLIDER_Video_Starttime");
    }

    // Resolve broken paths first, then convert to relative for portability
    std::string file = settings["E_FILEPICKERCTRL_Video_Filename"];
    if (!file.empty() && !FileExists(file)) {
        std::string fixed = FileUtils::FixFile("", file);
        if (!fixed.empty() && fixed != file) {
            settings["E_FILEPICKERCTRL_Video_Filename"] = fixed;
            file = fixed;
        }
    }
    if (!file.empty()) {
        if (std::filesystem::path(file).is_absolute()) {
            if (!FileExists(file, false)) {
                std::string fixed = FileUtils::FixFile("", file);
                std::string rel = FileUtils::MakeRelativeFile(fixed);
                settings["E_FILEPICKERCTRL_Video_Filename"] = rel.empty() ? fixed : rel;
            } else {
                std::string rel = FileUtils::MakeRelativeFile(file);
                if (!rel.empty())
                    settings["E_FILEPICKERCTRL_Video_Filename"] = rel;
            }
        }
        // Register with SequenceMedia so it appears in the Media tab
        auto& media = effect->GetParentEffectLayer()->GetParentElement()->GetSequenceElements()->GetSequenceMedia();
        media.GetVideo(settings["E_FILEPICKERCTRL_Video_Filename"]);
    }
}

std::list<std::string> VideoEffect::GetFileReferences(Model* model, const SettingsMap &SettingsMap) const
{
    std::list<std::string> res;
    if (SettingsMap["E_FILEPICKERCTRL_Video_Filename"] != "") {
        res.push_back(SettingsMap["E_FILEPICKERCTRL_Video_Filename"]);
    }
    return res;
}

bool VideoEffect::CleanupFileLocations(RenderContext* ctx, SettingsMap &SettingsMap)
{
    bool rc = false;
    std::string file = SettingsMap["E_FILEPICKERCTRL_Video_Filename"];
    if (FileExists(file))
    {
        if (!ctx->IsInShowFolder(file))
        {
            SettingsMap["E_FILEPICKERCTRL_Video_Filename"] = ctx->MoveToShowFolder(file, std::string(1, std::filesystem::path::preferred_separator) + "Videos");
            rc = true;
        }
    }

    return rc;
}

void VideoEffect::Render(Effect* effect, const SettingsMap& SettingsMap, RenderBuffer& buffer)
{
    float offset = buffer.GetEffectTimeIntervalPosition();

    int cl = GetValueCurveInt("Video_CropLeft", sCropLeftDefault, SettingsMap, offset, sCropMin, sCropMax, buffer.GetStartTimeMS(), buffer.GetEndTimeMS());
    int cr = GetValueCurveInt("Video_CropRight", sCropRightDefault, SettingsMap, offset, sCropMin, sCropMax, buffer.GetStartTimeMS(), buffer.GetEndTimeMS());
    int ct = GetValueCurveInt("Video_CropTop", sCropTopDefault, SettingsMap, offset, sCropMin, sCropMax, buffer.GetStartTimeMS(), buffer.GetEndTimeMS());
    int cb = GetValueCurveInt("Video_CropBottom", sCropBottomDefault, SettingsMap, offset, sCropMin, sCropMax, buffer.GetStartTimeMS(), buffer.GetEndTimeMS());

    Render(buffer,
           SettingsMap["FILEPICKERCTRL_Video_Filename"],
           SettingsMap.GetDouble("TEXTCTRL_Video_Starttime", sStartTimeDefault),
           std::min(cl, cr),
           std::max(cl, cr),
           std::max(ct, cb),
           std::min(ct, cb),
           SettingsMap.GetBool("CHECKBOX_Video_AspectRatio", sAspectRatioDefault),
           SettingsMap.Get("CHOICE_Video_DurationTreatment", sDurationTreatmentDefault),
           SettingsMap.GetBool("CHECKBOX_SynchroniseWithAudio", sSyncAudioDefault),
           SettingsMap.GetBool("CHECKBOX_Video_TransparentBlack", false),
           SettingsMap.GetInt("TEXTCTRL_Video_TransparentBlack", 0),
           GetValueCurveDouble("Video_Speed", sSpeedDefault, SettingsMap, offset, sSpeedMin, sSpeedMax, buffer.GetStartTimeMS(), buffer.GetEndTimeMS(), sSpeedDivisor),
           SettingsMap.GetInt("TEXTCTRL_SampleSpacing", sSampleSpacingDefault));
}

class VideoRenderCache : public EffectRenderCache {
public:
    VideoRenderCache()
	{
		_videoframerate = -1;
		_videoreader = nullptr;
        _loops = 0;
        _frameMS = 50;
        _nextManualMS = 0;
	};
    virtual ~VideoRenderCache() {
		if (_videoreader != nullptr)
		{
			delete _videoreader;
			_videoreader = nullptr;
		}
	};

    VideoReader* _videoreader;
	int _videoframerate;
	int _loops;
    int _frameMS;
    int _nextManualMS = 0;
    int _openedWidth = 0;
    int _openedHeight = 0;
    bool _openedAspect = false;
    bool _openedNative = false;
};

void VideoEffect::Render(RenderBuffer &buffer, std::string filename,
    double starttime, int cropLeft, int cropRight, int cropTop, int cropBottom, bool aspectratio, std::string durationTreatment, bool synchroniseAudio, bool transparentBlack, int transparentBlackLevel, double speed, uint32_t sampleSpacing)
{
    

    if (cropLeft > cropRight)
    {
        std::swap(cropLeft, cropRight);
    }

    if (cropBottom > cropTop)
    {
        std::swap(cropTop, cropBottom);
    }

    if (cropLeft == cropRight)
    {
        if (cropLeft == 0)
        {
            cropRight++;
        }
        else
        {
            cropLeft--;
        }
    }
    if (cropBottom == cropTop)
    {
        if (cropBottom == 0)
        {
            cropTop++;
        }
        else
        {
            cropBottom--;
        }
    }

    VideoRenderCache *cache = (VideoRenderCache*)buffer.infoCache[id];
    if (cache == nullptr) {
        cache = new VideoRenderCache();
        buffer.infoCache[id] = cache;
    }

    int &_loops = cache->_loops;
    VideoReader* &_videoreader = cache->_videoreader;
    int& _frameMS = cache->_frameMS;
    int& _nextManualMS = cache->_nextManualMS;

    // The raw settings filename — the key GetFrameParallelism classifies on
    // (classification can't resolve paths); recorded once the reader impl is
    // known below.
    const std::string settingsFilename = filename;

    if (synchroniseAudio)
    {
        starttime = 0;
        durationTreatment = "Normal";
        if (buffer.GetMedia() != nullptr)
        {
            filename = buffer.GetMedia()->FileName();
            starttime = (double)buffer.curEffStartPer * (double)buffer.frameTimeInMs / 1000.0;
        }
    }

    // we always reopen video on first frame or if it is not open or if the filename has changed
    if (buffer.needToInit)
    {
        buffer.needToInit = false;

        _loops = 0;
        _nextManualMS = 0;
        _frameMS = buffer.frameTimeInMs;
        // Held aside rather than deleted: readers with frame-independent
        // access can be reused if this init targets the same file with the
        // same open parameters. Frame-parallel clone buffers re-init on every
        // window, so without reuse each 24-frame window would recreate the
        // reader and re-prime it. Anything left in oldReader is deleted on
        // every exit path from this block.
        std::unique_ptr<VideoReader> oldReader(_videoreader);
        _videoreader = nullptr;

        if (buffer.BufferHt == 1) {
            spdlog::warn("VideoEffect::Cannot render video onto a 1 pixel high model. Have you set it to single line?");
        } else if (auto* sm = buffer.GetSequenceMedia(); sm != nullptr) {
            auto vidEntry = sm->GetVideo(filename);
            if (!vidEntry) {
                spdlog::warn("VideoEffect: No video file specified.");
            } else {
                vidEntry->MarkIsUsed();
                std::string resolved = vidEntry->GetResolvedPath();
                if (resolved.empty() || !FileExists(resolved)) {
                    spdlog::warn("VideoEffect: Cannot find video file '{}'.", filename);
                } else {
                    filename = resolved;
                    // have to open the file
                    int width = buffer.BufferWi * 100 / (cropRight - cropLeft);
                    int height = buffer.BufferHt * 100 / (cropTop - cropBottom);

                    bool useNativeResolution = (sampleSpacing > 0);

                    if (oldReader != nullptr && oldReader->SupportsFrameIndependentAccess() &&
                        oldReader->GetFilename() == resolved &&
                        cache->_openedAspect == aspectratio && cache->_openedNative == useNativeResolution &&
                        cache->_openedWidth == width && cache->_openedHeight == height) {
                        if (oldReader->AtEnd()) {
                            // AtEnd is sticky once a request ran past the video
                            // end and the Loop treatment consults it — clear it
                            // rather than carrying it into this effect (a
                            // frame-independent reader needs no reposition).
                            oldReader->Seek(0, false);
                        }
                        _videoreader = oldReader.release();
                    } else {
                        oldReader.reset();
                        _videoreader = new VideoReader(filename, width, height, aspectratio, useNativeResolution, true);
                        cache->_openedWidth = width;
                        cache->_openedHeight = height;
                        cache->_openedAspect = aspectratio;
                        cache->_openedNative = useNativeResolution;
                    }

                    if (_videoreader == nullptr)
                    {
                        spdlog::warn("VideoEffect: Failed to load video file {}.", (const char *)filename.c_str());
                    }
                    else
                    {
                        // extract the video length
                        int videolen = _videoreader->GetLengthMS();

                        if (videolen == 0)
                        {
                            spdlog::warn("VideoEffect: Video {} was read as 0 length.", (const char *)filename.c_str());
                        }

                        if (!_videoreader->SupportsFrameIndependentAccess()) {
                            // Positional (FFmpeg) readers need their position
                            // established: read the first frame ... if i dont it
                            // thinks the first frame i read is the first frame.
                            // Frame-independent readers serve any timestamp
                            // identically without priming, so skip the frame-0
                            // decode and the seek.
                            _videoreader->GetNextFrame(0);

                            if (starttime != 0)
                            {
                                spdlog::debug("Video effect initialising ... seeking to start location for the video {}.", (float)starttime);
                                _videoreader->Seek(starttime * 1000);
                            }
                        }

                        if (!synchroniseAudio) {
                            NoteVideoFileFrameIndependence(settingsFilename, _videoreader->SupportsFrameIndependentAccess());
                        }

                        if (durationTreatment == "Slow/Accelerate")
                        {
                            int effectFrames = buffer.curEffEndPer - buffer.curEffStartPer + 1;
                            int videoFrames = (videolen - (starttime * 1000)) / buffer.frameTimeInMs;
                            float speedFactor = (float)videoFrames / (float)effectFrames;
                            _frameMS = (int)((float)buffer.frameTimeInMs * speedFactor);
                        }
                        spdlog::debug("Video effect length: {}, video length: {}, startoffset: {}, duration treatment: {}.",
                            (buffer.curEffEndPer - buffer.curEffStartPer + 1) * _frameMS, videolen, (float)starttime,
                            durationTreatment);
                    }
                }
            }
        }
        else
        {
            if (buffer.curPeriod == buffer.curEffStartPer)
            {
                spdlog::warn("VideoEffect: Video file '{}' not found.", filename);
            }
        }
    }

    if (_videoreader != nullptr && sampleSpacing == 0) {
        int width = buffer.BufferWi * 100 / (cropRight - cropLeft);
        int height = buffer.BufferHt * 100 / (cropTop - cropBottom);
        bool vwidthEq = width == cache->_openedWidth;
        bool vheightEq = height == cache->_openedHeight;
        if (!vwidthEq || !vheightEq) {
            // Try to retarget the existing reader to the new size first — recreating
            // the AVURLAsset on every size change leaks ~48-byte FigAsset entries
            // into MediaToolbox's process-global cache. Fall back to delete+new
            // when the impl can't resize in place (FFmpeg today).
            if (!_videoreader->Resize(width, height)) {
                delete _videoreader;
                _videoreader = new VideoReader(filename, width, height, aspectratio, false, true);
            }
            cache->_openedWidth = width;
            cache->_openedHeight = height;
        }
    }

    if (_videoreader != nullptr) {
        // Corridor identity: every clone of one row+effect shares it, so the
        // shared decoder keeps one forward decode chain per corridor instead
        // of stealing chains between corridors (each steal that repositions
        // re-decodes a GOP prefix).
        _videoreader->SetStreamGroup(
            (std::hash<std::string>{}(buffer.cur_model) ^
             ((uint64_t)(uint32_t)buffer.curEffStartPer * 0x9E3779B97F4A7C15ULL)) |
            1ULL);
    }

    if (_videoreader != nullptr && _videoreader->GetLengthMS() > 0)
    {
        long frame = 0;
        
        if (durationTreatment == "Manual")
        {
            frame = starttime * 1000 + _nextManualMS;
            _nextManualMS += speed * _frameMS;
        }
        else if (durationTreatment == "Manual and Loop")
        {
            frame = starttime * 1000 + _nextManualMS;

            while (frame < 0)
            {
                frame += _videoreader->GetLengthMS();
            }

            while (frame > _videoreader->GetLengthMS())
            {
                frame -= _videoreader->GetLengthMS();
            }

            _nextManualMS += speed * _frameMS;
        }
        else
        {
            frame = starttime * 1000 + (buffer.curPeriod - buffer.curEffStartPer) * _frameMS - _loops * (_videoreader->GetLengthMS() + _frameMS);
        }

        // get the image for the current frame
        VideoFrame* image = _videoreader->GetNextFrame(frame);

        // if we have reached the end and we are to loop
        if (_videoreader->AtEnd() && durationTreatment == "Loop")
        {
            // jump back to start and try to read frame again
            _loops++;
            frame = starttime * 1000 + (buffer.curPeriod - buffer.curEffStartPer) * _frameMS - _loops * (_videoreader->GetLengthMS() + _frameMS);
            if (frame < 0)
            {
                frame = 0;
            }
            spdlog::debug("Video effect loop #{} at frame {} to video frame {}.", _loops, buffer.curPeriod - buffer.curEffStartPer, frame);

            _videoreader->Seek(0);
            image = _videoreader->GetNextFrame(frame);
        }

            // check it looks valid
            if (image != nullptr && frame >= 0) {

                // This handles normal scaling of videos
                if (sampleSpacing == 0) {
                    int xoffset = cropLeft * _videoreader->GetWidth() / 100;
                    int yoffset = cropBottom * _videoreader->GetHeight() / 100;
                    int xtail = (100 - cropRight) * _videoreader->GetWidth() / 100;
                    int ytail = (100 - cropTop) * _videoreader->GetHeight() / 100;
                    int startx = (buffer.BufferWi - _videoreader->GetWidth() * (cropRight - cropLeft) / 100) / 2;
                    int starty = (buffer.BufferHt - _videoreader->GetHeight() * (cropTop - cropBottom) / 100) / 2;

                    ispc::VideoData rdata;
                    rdata.width = _videoreader->GetWidth() - xoffset - xtail;
                    rdata.height = _videoreader->GetHeight() - yoffset - ytail;
                    rdata.buffer_width = buffer.BufferWi;
                    rdata.buffer_height = buffer.BufferHt;
                    rdata.ytail = ytail;
                    rdata.xtail = xtail;
                    rdata.yoffset = yoffset;
                    rdata.xoffset = xoffset;
                    rdata.transparentBlack = transparentBlack;
                    rdata.transparentBlackLevel = transparentBlackLevel;
                    rdata.ch = _videoreader->GetPixelChannels();
                    rdata.image_width = _videoreader->GetWidth();
                    rdata.image_height = _videoreader->GetHeight();
                    rdata.image = image->data;
                    rdata.startx = startx;
                    rdata.starty = starty;
                    rdata.bufferData = (void*)&buffer;
                    rdata.sampleSpacing = sampleSpacing;

                    int max = buffer.BufferHt * buffer.BufferWi;
                    constexpr int bfBlockSize = 4096;
                    int blocks = max / bfBlockSize + 1;

                    parallel_for(0, blocks, [&rdata, &buffer, max](int y) {
                        int start = y * bfBlockSize;
                        int end = start + bfBlockSize;
                        if (end > max) {
                            end = max;
                        }
                        VideoEffectProcess(rdata, start, end, (ispc::uint8_t4 *)buffer.GetPixels());
                    });
                } else {
                    // this handles video sampling where we sample pixels from the image rather than scaling it and thus washing out the colours
                    int xneeded = buffer.BufferWi * sampleSpacing;
                    int yneeded = buffer.BufferHt * sampleSpacing;
                    int imageWidthAfterCropping = ((100 - cropLeft + cropRight) * image->width) / 200;
                    int imageHeightAferCropping = ((cropTop + 100 - cropBottom) * image->width) / 200;
                    int startx = imageWidthAfterCropping / 2 - xneeded / 2 + (cropLeft * image->width) / 100;
                    int starty = imageHeightAferCropping / 2 - yneeded / 2 + ((100 - cropTop) * image->height) / 100;

                    int xoffset = cropLeft * _videoreader->GetWidth() / 100;
                    int yoffset = cropBottom * _videoreader->GetHeight() / 100;
                    int xtail = (100 - cropRight) * _videoreader->GetWidth() / 100;
                    int ytail = (100 - cropTop) * _videoreader->GetHeight() / 100;

                    ispc::VideoData rdata;
                    rdata.width = buffer.BufferWi;
                    rdata.height = buffer.BufferHt;
                    rdata.ytail = ytail;
                    rdata.xtail = xtail;
                    rdata.yoffset = yoffset;
                    rdata.xoffset = xoffset;
                    rdata.transparentBlack = transparentBlack;
                    rdata.transparentBlackLevel = transparentBlackLevel;
                    rdata.ch = _videoreader->GetPixelChannels();
                    rdata.image_width = _videoreader->GetWidth();
                    rdata.image_height = _videoreader->GetHeight();
                    rdata.image = image->data;
                    rdata.startx = startx;
                    rdata.starty = starty;
                    rdata.bufferData = (void*)&buffer;
                    rdata.sampleSpacing = sampleSpacing;

                    int max = buffer.BufferHt * buffer.BufferWi;
                    constexpr int bfBlockSize = 4096;
                    int blocks = max / bfBlockSize + 1;

                    parallel_for(0, blocks, [&rdata, &buffer, max](int y) {
                        int start = y * bfBlockSize;
                        int end = start + bfBlockSize;
                        if (end > max) {
                            end = max;
                        }
                        VideoEffectProcessSample(rdata, start, end, (ispc::uint8_t4 *)buffer.GetPixels());
                        });
                }
            } else {
                if (durationTreatment == "Normal") {
                    // display a blue background to show we have gone past end of video
                    for (int y = 0; y < buffer.BufferHt; y++) {
                        for (int x = 0; x < buffer.BufferWi; x++) {
                            buffer.SetPixel(x, y, xlBLUE);
                        }
                    }
                }
            }
    }
    else
    {
        // display a red background to show we have a problem
        for (int y = 0; y < buffer.BufferHt; y++)
        {
            for (int x = 0; x < buffer.BufferWi; x++)
            {
                buffer.SetPixel(x, y, xlRED);
            }
        }
    }
}
