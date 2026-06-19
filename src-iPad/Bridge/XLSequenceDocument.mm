/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#import "XLSequenceDocument.h"
#import "XLGridMetalBridge.h"
#import <CoreGraphics/CoreGraphics.h>
#include "iPadRenderContext.h"

#include "render/Element.h"
#include "render/RenderUtils.h"
#include "render/EffectLayer.h"
#include "render/Effect.h"
#include "render/SequenceElements.h"
#include "render/SequenceMedia.h"
#include "render/SequenceFile.h"
#include "render/SequencePackage.h"
#include "render/RenderEngine.h"
#include "render/FSEQFile.h"
#include "render/ModelVideoExporter.h"
#include "render/ModelGifExporter.h"
#import "XLHousePreviewVideoExporter.h"
#include "utils/UtilFunctions.h"
#include "utils/string_utils.h"
#include "lyrics/PhonemeDictionary.h"
#include "lyrics/LyricBreakdown.h"
#include "render/SequenceViewManager.h"
#include "effects/RenderableEffect.h"
#include "effects/SketchSVGImport.h"
#include "effects/EffectManager.h"
#include "effects/ShaderEffect.h"
#include "graphics/xlGraphicsAccumulators.h"
#include "media/AudioManager.h"
#include "media/NoteImporter.h"
#include "media/OnsetDetector.h"
#include "media/SoundClassifier.h"
#include "media/TempoDetector.h"
#include "media/PitchDetector.h"
#include "media/ChordDetector.h"
#include "media/Spectrogram.h"
#include "media/AIModelStore.h"
#include "media/StemSeparator.h"
#include "../../dependencies/libxlsxwriter/third_party/minizip/unzip.h"
#include <xlsxwriter.h>
#include <filesystem>
#include "media/MediaCompatibility.h"
#include "media/VideoReader.h"
#include "render/ValueCurve.h"
#include "render/ColorCurve.h"
#include "import_export/ExportModels.h"
#include "import_export/ExportEffectsReport.h"
#include "models/Model.h"
#include "models/Node.h"
#include "models/ModelManager.h"
#include "models/DMX/DmxModel.h"
#include "models/ModelGroup.h"
#include "models/SubModel.h"
#include "models/ViewObject.h"
#include "models/ViewObjectManager.h"
#include "models/MeshObject.h"
#include "models/ImageObject.h"
#include "models/GridlinesObject.h"
#include "models/TerrainObject.h"
#include "models/RulerObject.h"
#include "models/BoxedScreenLocation.h"
#include "models/TwoPointScreenLocation.h"
#include "XmlSerializer/XmlSerializingVisitor.h"
#include "XmlSerializer/XmlSerializer.h"
#include "XmlSerializer/XmlSerializeFunctions.h"
#include "XmlSerializer/GdtfParser.h"
#include "utils/NodeUtils.h"
#include "models/ArchesModel.h"
#include "models/ThreePointScreenLocation.h"
#include "models/ControllerConnection.h"
#include "controllers/ControllerCaps.h"
#include "outputs/Controller.h"
#include "outputs/OutputManager.h"
#include "outputs/ControllerEthernet.h"
#include "outputs/ControllerNull.h"
#include "outputs/ControllerSerial.h"
#include "outputs/SerialOutput.h"
#include "outputs/Output.h"
#include "discovery/Discovery.h"
#include "controllers/FPP.h"
#include "outputs/ArtNetOutput.h"
#include "outputs/TwinklyOutput.h"
#include "outputs/DDPOutput.h"
#include "controllers/Pixlite16.h"
#include "controllers/WLED.h"
#include "controllers/BaseController.h"
#include "controllers/ControllerUploadData.h"
#include "controllers/ExportSettings.h"
#include "models/Pixels.h"
#include "render/UICallbacks.h"
#include "utils/ip_utils.h"
#include "models/ImageModel.h"
#include "models/LabelModel.h"
#include "models/MultiPointModel.h"
#include "models/PolyLineModel.h"
#include "models/CandyCaneModel.h"
#include "models/ChannelBlockModel.h"
#include "models/CircleModel.h"
#include "models/CubeModel.h"
#include "models/CustomModel.h"
#include "models/IciclesModel.h"
#include "models/MatrixModel.h"
#include "models/SingleLineModel.h"
#include "models/SpinnerModel.h"
#include "models/SphereModel.h"
#include "models/StarModel.h"
#include "models/TreeModel.h"
#include "models/WindowFrameModel.h"
#include "models/WreathModel.h"
#include "models/DMX/DmxFloodlight.h"
#include "models/DMX/DmxModel.h"
#include "models/DMX/DmxColorAbility.h"
#include "models/DMX/DmxColorAbilityRGB.h"
#include "models/DMX/DmxColorAbilityCMY.h"
#include "models/DMX/DmxColorAbilityWheel.h"
#include "models/DMX/DmxPresetAbility.h"
#include "models/DMX/DmxBeamAbility.h"
#include "models/DMX/DmxShutterAbility.h"
#include "models/DMX/DmxDimmerAbility.h"
#include "models/DMX/DmxMotor.h"
#include "models/DMX/DmxMovingHead.h"
#include "models/DMX/DmxMovingHeadComm.h"
#include "models/DMX/DmxGeneral.h"
#include "models/DMX/DmxMovingHeadAdv.h"
#include "models/DMX/DmxSkull.h"
#include "models/DMX/DmxServo.h"
#include "models/DMX/DmxServo3D.h"
#include "models/DMX/Servo.h"
#include "models/DMX/DmxImage.h"
#include "models/DMX/Mesh.h"
#include "utils/FileUtils.h"
#include "utils/ExternalHooks.h"
#include "utils/XsqFileScanner.h"
#include "utils/CurlManager.h"

#import <Security/Security.h>
#include "utils/xlImage.h"
#include "xLightsVersion.h"
#include "globals.h"
#include "diagnostics/CheckSequenceReport.h"
#include "diagnostics/SequenceChecker.h"

#import "XLCheckSequenceIssue.h"
#import "XLFindEffectResult.h"

#include <nlohmann/json.hpp>
#include <spdlog/spdlog.h>

#include <algorithm>
#include <functional>
#include <cctype>
#include <array>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <memory>
#include <sstream>
#include <string>
#include <unordered_map>
#include <unordered_set>

#import <os/proc.h>

// Shift-Effects undo snapshot. Bulk shift is destructive at the 0
// boundary (truncation / deletion), so reversing it by re-shifting is
// lossy. Instead we capture the full re-creatable state of every effect
// on every layer we touch, keyed by the live EffectLayer* (valid for
// the session — undo runs against the same loaded sequence), and
// restore by clearing + re-adding. `selectedOnly` snapshots only the
// in-scope layers, but stores every effect on them so a restore re-adds
// any that the shift deleted.
namespace {
struct ShiftEffectSnap {
    int id;
    std::string name;
    std::string settings;
    std::string palette;
    int startMS;
    int endMS;
    int selected;
    bool protectd;
};
struct ShiftLayerSnap {
    EffectLayer* layer;
    std::vector<ShiftEffectSnap> effects;
};
}

// Private helpers. extrasFor: takes a C++ Model* so it lives here
// (not in the ObjC++-free header). Declared up front so it can be
// called from any method below without ordering concerns.
@interface XLSequenceDocument ()
- (NSDictionary<NSString*, id>*)extrasFor:(Model*)m;
- (NSDictionary<NSString*, id>*)controllerConnectionFor:(Model*)m;
- (std::string)joinIndexedNames:(NSArray<NSString*>*)names;
- (void)recalcModelStartChannels;
- (void)reworkAndRecalcStartChannels;
- (void)recalcAndMarkControllersDirty;
@end

// Controller-property descriptor builders are defined further down
// (alongside controllerPropertiesForName); forward-declare them so
// the global-output-settings methods above can use them too.
static NSMutableDictionary* CtrlIntProp(NSString* key, NSString* label, int value, int minV, int maxV);
static NSMutableDictionary* CtrlBoolProp(NSString* key, NSString* label, BOOL value);
static NSMutableDictionary* CtrlEnumProp(NSString* key, NSString* label, int index, NSArray<NSString*>* options);
static NSMutableDictionary* CtrlStringProp(NSString* key, NSString* label, NSString* _Nullable value, BOOL editable);
static int IndexOfString(NSArray<NSString*>* options, const std::string& v);

@implementation XLSequenceDocument {
    std::unique_ptr<iPadRenderContext> _context;
    // Snapshot of `SequenceElements::GetChangeCount()` at the last
    // successful load / save. Current count == snapshot ⇒ clean.
    unsigned int _lastSavedChangeCount;
    // A7: time-step (seconds) reported by the last classifySound run
    // so Swift callers can turn per-second confidence arrays into MS
    // lookups.
    float _lastClassificationTimeStep;
    // A6: cached spectrogram data. Populated lazily by
    // -ensureSpectrogramComputed; cleared whenever the sequence is
    // closed. `_spectrogramAudioHash` keeps us from handing out a
    // spectrogram that no longer matches the loaded audio after a
    // reload.
    Spectrogram _spectrogram;
    std::string _spectrogramAudioHash;

    // xsqz / sequence-package lifecycle. `_openPackage` owns the
    // extracted temp directory — destroying it (on close) wipes the
    // temp dir. `_packagePath` is the original `.xsqz` the session
    // was opened from; `-saveSequence` re-packs the temp dir back
    // there. `_previousShowFolder` remembers the show dir that was
    // active before the package open so we can restore it on close.
    std::unique_ptr<SequencePackage> _openPackage;
    std::string _packagePath;
    std::string _previousShowFolder;

    // FPP Connect — `discoverFPPInstances` builds this list and the
    // upload methods reuse it (the post-discovery FPP* objects carry
    // authenticated curl handles, version, mode, etc.). Cleared by
    // `releaseFPPInstances` when the SwiftUI sheet dismisses.
    std::list<FPP*> _fppInstances;

    // FPP auth delegate (C++) bridged to a Swift prompt handler.
    // Lifetime: owned by the document; shared across every FPP* in
    // `_fppInstances` via `fpp->_authDelegate`. Defined below in the
    // FPP Connect (Slice A) section.
    std::unique_ptr<class XLiPadDiscoveryAuthDelegate> _fppAuthDelegate;

    // Shift-Effects undo snapshots, keyed by token. The token is a
    // monotonically increasing id handed to Swift; `restoreShiftSnapshot:`
    // consumes the entry.
    std::map<NSInteger, std::vector<ShiftLayerSnap>> _shiftSnapshots;
    NSInteger _nextShiftToken;
}

// Block signature for the Swift-side password prompt. The bridge
// stores a copy in `_fppAuthDelegate` and invokes it on the main
// thread when an FPP returns 401.
typedef void (^XLFPPAuthPromptCompletion)(NSString* _Nullable user,
                                           NSString* _Nullable password,
                                           BOOL savePassword);
typedef void (^XLFPPAuthPromptHandler)(NSString* host,
                                        XLFPPAuthPromptCompletion completion);

@synthesize lastClassificationTimeStep = _lastClassificationTimeStep;

- (instancetype)init {
    self = [super init];
    if (self) {
        _context = std::make_unique<iPadRenderContext>();
    }
    return self;
}

- (BOOL)loadShowFolder:(NSString*)path {
    return _context->LoadShowFolder(std::string([path UTF8String]));
}

- (BOOL)loadShowFolder:(NSString*)path mediaFolders:(NSArray<NSString*>*)mediaFolders {
    std::list<std::string> folders;
    for (NSString* f in mediaFolders) {
        folders.push_back(std::string([f UTF8String]));
    }
    return _context->LoadShowFolder(std::string([path UTF8String]), folders);
}

+ (BOOL)obtainAccessToPath:(NSString*)path enforceWritable:(BOOL)enforceWritable {
    if (path.length == 0) return NO;
    return ObtainAccessToURL(std::string([path UTF8String]), enforceWritable) ? YES : NO;
}

+ (NSString*)sketchDefFromSVGFile:(NSString*)path {
    if (path.length == 0) return @"";
    std::string def = SketchDefFromSVGFile(std::string([path UTF8String]));
    return [NSString stringWithUTF8String:def.c_str()];
}

// MARK: - Media relocation

- (NSString*)showFolderPath {
    if (!_context) return @"";
    const std::string& s = _context->GetShowDirectory();
    return [NSString stringWithUTF8String:s.c_str()];
}

- (NSArray<NSString*>*)mediaFolderPaths {
    NSMutableArray<NSString*>* out = [NSMutableArray array];
    if (!_context) return out;
    for (const auto& mf : _context->GetMediaFolders()) {
        [out addObject:[NSString stringWithUTF8String:mf.c_str()]];
    }
    return out;
}

- (NSString*)moveFileToShowFolder:(NSString*)sourcePath
                        subdirectory:(NSString*)subdirectory {
    if (!_context || sourcePath.length == 0) return nil;
    std::string result = _context->MoveToShowFolder(
        std::string([sourcePath UTF8String]),
        std::string([(subdirectory ?: @"") UTF8String]),
        /*reuse*/ false);
    if (result.empty()) return nil;
    return [NSString stringWithUTF8String:result.c_str()];
}

- (NSString*)copyFileToMediaFolder:(NSString*)sourcePath
                       mediaFolderPath:(NSString*)mediaFolderPath
                        subdirectory:(NSString*)subdirectory {
    if (!_context || sourcePath.length == 0 || mediaFolderPath.length == 0) {
        return nil;
    }
    std::string result = _context->CopyToMediaFolder(
        std::string([sourcePath UTF8String]),
        std::string([mediaFolderPath UTF8String]),
        std::string([(subdirectory ?: @"") UTF8String]));
    if (result.empty()) return nil;
    return [NSString stringWithUTF8String:result.c_str()];
}

- (BOOL)pathIsInShowOrMediaFolder:(NSString*)path {
    if (!_context || path.length == 0) return NO;
    return _context->IsInShowOrMediaFolder(
        std::string([path UTF8String])) ? YES : NO;
}

- (NSString*)makeRelativePath:(NSString*)path {
    if (!_context || path.length == 0) return path ?: @"";
    std::string s = _context->MakeRelativePath(
        std::string([path UTF8String]));
    return [NSString stringWithUTF8String:s.c_str()];
}

- (BOOL)openSequence:(NSString*)path {
    BOOL ok = _context->OpenSequence(std::string([path UTF8String]));
    if (ok) {
        // Re-run the post-load view-select dance so a non-master
        // saved view is actually populated. `PrepareViews` inside
        // core's `LoadSequencerFile` only push_backs empty slots
        // for each view — it doesn't fill `mAllViews[currentView]`
        // for the saved view the same way a later SelectView call
        // would. Desktop hides this via
        // `ViewsModelsPanel::InitializeCheckboxes`'s
        // `SelectView(GetViewName(GetCurrentView()))` call; iPad
        // has no equivalent panel, so we do the same work here so
        // the grid shows the right models immediately. Without
        // this, the user has to click away to Master and back to
        // force the populate.
        int cur = _context->GetSequenceElements().GetCurrentView();
        if (cur > 0) {
            [self setCurrentViewIndex:cur];
        }
        [self markSequenceClean];
    }
    return ok;
}

- (BOOL)openPackagedSequence:(NSString*)pkgPath {
    if (!_context || pkgPath.length == 0) return NO;
    std::string pkgStr([pkgPath UTF8String]);

    // Obtain access BEFORE the existence probe — on iOS, paths from
    // iCloud Drive / external Files providers need the security-scope
    // bookmark re-resolved to reactivate scope in this thread before
    // anything (even `getattrlist`) will work. The Swift-side handler
    // copies the package into the app sandbox before calling us, so
    // the path we see here is always inside our container — but keep
    // this call ordered first anyway for the (rare) case a caller
    // passes us a path needing bookmark resolution.
    ObtainAccessToURL(pkgStr, /*enforceWritable=*/true);

    if (!FileExists(pkgStr)) return NO;

    // Drop any currently-loaded sequence + previously-open package
    // before we set up the new one. Doing this via closeSequence
    // restores the previous show folder, which is the natural state
    // to preserve if the new package open fails below.
    [self closeSequence];

    // Remember the show folder that was active so closeSequence can
    // restore it after this package session ends.
    _previousShowFolder = _context->GetShowDirectory();

    // SequencePackage does the extract. We pass `showDir=""` and
    // `seqXmlFileName=""` because the package's own `.xsq` supplies
    // both — the non-empty constructor parameters are only used by
    // the media-import codepaths on desktop (FindAndCopyAudio,
    // ImportFaceInfo, etc.) which this flow doesn't touch.
    auto pkg = std::make_unique<SequencePackage>(
        std::filesystem::path(pkgStr),
        /*showDir=*/std::string(),
        /*seqXmlFileName=*/std::string(),
        /*models=*/nullptr);
    pkg->Extract();
    const std::filesystem::path& xsq = pkg->GetXsqFile();
    if (xsq.empty() || !FileExists(xsq.string())) {
        printf("openPackagedSequence: no .xsq found in package '%s'\n", pkgStr.c_str());
        // Package destructor wipes the temp dir since we never set
        // SetLeaveFiles(true). Restore the previous show folder
        // since we already captured it above.
        if (!_previousShowFolder.empty()) {
            _context->LoadShowFolder(_previousShowFolder);
            _previousShowFolder.clear();
        }
        return NO;
    }

    // The extracted temp dir is a self-contained show folder (the
    // package carries the subset of xlights_rgbeffects / models /
    // media needed by the sequence). Use `GetTempShowFolder` — the
    // dir containing `xlights_rgbeffects.xml` — rather than the
    // outer temp dir. Older desktop-authored .xsqz files nest
    // everything under a `<showname>/` subfolder, so the outer
    // temp dir is one level too high; GetTempShowFolder auto-
    // adjusts to whatever level the rgbeffects file was extracted
    // at. New packages produced by `Pack()` put files at the zip
    // root, so the two levels coincide.
    const std::string tempShowDir = pkg->GetTempShowFolder();
    if (!_context->LoadShowFolder(tempShowDir)) {
        printf("openPackagedSequence: LoadShowFolder('%s') failed\n", tempShowDir.c_str());
        if (!_previousShowFolder.empty()) {
            _context->LoadShowFolder(_previousShowFolder);
            _previousShowFolder.clear();
        }
        return NO;
    }

    NSString* innerPath = [NSString stringWithUTF8String:xsq.string().c_str()];
    BOOL ok = [self openSequence:innerPath];
    if (!ok) {
        if (!_previousShowFolder.empty()) {
            _context->LoadShowFolder(_previousShowFolder);
            _previousShowFolder.clear();
        }
        return NO;
    }

    // Commit the package state only after successful open. From here
    // on, `-saveSequence` will repack into `pkgStr` and
    // `-closeSequence` will clean up.
    _openPackage = std::move(pkg);
    _packagePath = pkgStr;
    return YES;
}

- (BOOL)isPackagedSequence {
    return (_openPackage != nullptr && !_packagePath.empty()) ? YES : NO;
}

- (NSString*)packagePath {
    if (_packagePath.empty()) return @"";
    return [NSString stringWithUTF8String:_packagePath.c_str()];
}

- (BOOL)newSequenceAtPath:(NSString*)savePath
                       type:(NSString*)type
                  mediaPath:(NSString*)mediaPath
                 durationMS:(int)durationMS
                    frameMS:(int)frameMS {
    if (!_context) return NO;
    if (savePath.length == 0) return NO;
    if (durationMS <= 0 || frameMS <= 0) return NO;
    std::string typeStr([type UTF8String]);
    if (typeStr != "Media" && typeStr != "Animation" && typeStr != "Effect") {
        return NO;
    }

    // Drop any current sequence state before we start. CloseSequence
    // clears SequenceElements and releases the SequenceFile unique_ptr
    // so the fresh save below starts from a clean slate.
    _context->CloseSequence();

    std::string pathStr([savePath UTF8String]);
    ObtainAccessToURL(pathStr, /*enforceWritable=*/true);

    // Configure the new SequenceFile. The two-arg constructor seeds
    // `seq_timing` from `frameMS`; the rest of the properties are set
    // via the public mutators, matching the desktop wizard flow
    // (`SeqFileUtilities.cpp:114`, `SeqSettingsDialog.cpp:1794-1828`).
    SequenceFile sf(pathStr, (uint32_t)frameMS);
    sf.SetSequenceType(typeStr);
    sf.SetSequenceDurationMS(durationMS);
    sf.SetSequenceTiming(std::to_string(frameMS) + " ms");
    if (typeStr == "Media" && mediaPath.length > 0) {
        sf.SetMediaFile(_context->GetShowDirectory(),
                        std::string([mediaPath UTF8String]),
                        /*overwrite_tags=*/false);
    }

    // Desktop "Default Model Blending for New Sequences" pref
    // (@AppStorage("sequence.defaultModelBlending"): "Enabled" |
    // "Disabled"). The new sequence's empty SequenceElements drives the
    // written `ModelBlending` attribute (SequenceFile::Save reads
    // seq_elements.SupportsModelBlending()), so set the flag here before
    // Save. Absent key → leave the SequenceElements default (Enabled),
    // preserving prior iPad behavior.
    {
        CFPropertyListRef v = CFPreferencesCopyAppValue(CFSTR("sequence.defaultModelBlending"),
                                                        kCFPreferencesCurrentApplication);
        if (v) {
            if (CFGetTypeID(v) == CFStringGetTypeID()) {
                char buf[16] = { 0 };
                if (CFStringGetCString((CFStringRef)v, buf, sizeof(buf), kCFStringEncodingUTF8)) {
                    std::string s(buf);
                    if (s == "Disabled") {
                        _context->GetSequenceElements().SetSupportsModelBlending(false);
                    } else if (s == "Enabled") {
                        _context->GetSequenceElements().SetSupportsModelBlending(true);
                    }
                }
            }
            CFRelease(v);
        }
    }

    // Save via the context's (just-cleared) SequenceElements.
    // `Save` only reads the elements to emit XML — no live model
    // wiring required, and CloseSequence above left them empty.
    if (!sf.Save(_context->GetSequenceElements())) {
        return NO;
    }

    // Fall through to the normal open path so SequenceElements,
    // render engine, row info, and audio all wire up through the
    // same code that open-existing uses.
    return [self openSequence:savePath];
}

- (void)closeSequence {
    _context->CloseSequence();
    _lastSavedChangeCount = 0;

    // If the current session came from a `.xsqz`, tear down the
    // package now. The `SequencePackage` destructor removes the
    // extracted temp dir (we never set `SetLeaveFiles(true)`).
    // Then restore the show folder that was active before the
    // package was opened so follow-on UI (sequence picker,
    // model selection, etc.) points back at the user's real
    // show folder instead of the now-deleted temp.
    if (_openPackage) {
        _openPackage.reset();
        _packagePath.clear();
        if (!_previousShowFolder.empty()) {
            _context->LoadShowFolder(_previousShowFolder);
            _previousShowFolder.clear();
        }
    }
}

- (BOOL)isSequenceLoaded {
    return _context->IsSequenceLoaded();
}

- (BOOL)saveSequence {
    if (!_context || !_context->IsSequenceLoaded()) return NO;
    auto* sf = _context->GetSequenceFile();
    if (!sf) return NO;
    const std::string path = sf->GetFullPath();
    if (path.empty()) return NO;

    // Security-scoped access covers files under the show folder
    // (which is the common case — sequences live in the show dir).
    // Obtain access before the write in case the file is outside
    // the show scope (Save-As to an iCloud path).
    ObtainAccessToURL(path, /*enforceWritable=*/true);

    bool ok = sf->Save(_context->GetSequenceElements());
    if (!ok) {
        return NO;
    }

    // xsqz save-back: the `.xsq` just written lives inside the
    // extracted temp dir. Re-pack the whole temp dir back into
    // the original `.xsqz` so the file the user tapped in Files
    // reflects the edits. Repack writes to `.xsqz.tmp` and
    // atomically renames, so a failure here can't corrupt the
    // original — but if it fails, we surface NO to the caller
    // and leave the in-memory sequence dirty (markSequenceClean
    // is conditional on full success).
    if (_openPackage && !_packagePath.empty()) {
        if (!_openPackage->Repack(std::filesystem::path(_packagePath))) {
            return NO;
        }
    }

    [self markSequenceClean];
    return YES;
}

- (NSString*)exportSongRegionToPath:(NSString*)path
                            startMS:(int)startMS
                              endMS:(int)endMS
                               name:(NSString*)regionName {
    if (!_context || !_context->IsSequenceLoaded()) return @"No sequence loaded.";
    auto* sf = _context->GetSequenceFile();
    if (!sf) return @"No sequence file.";
    const int regionDurationMS = endMS - startMS;
    if (regionDurationMS <= 0) return @"Invalid song region.";

    std::filesystem::path outPath([path UTF8String]);
    ObtainAccessToURL(outPath.parent_path().string(), /*enforceWritable=*/true);

    // Step 1 — trim audio to a sibling .m4a (matches desktop, which
    // uses GetRate() both as the sample index scale and as the encode
    // samplerate).
    std::string mediaRel;
    AudioManager* audio = _context->GetCurrentMediaManager();
    if (audio != nullptr) {
        long rate = audio->GetRate();
        long trackSize = audio->GetTrackSize();
        long startSample = (long)((double)startMS * rate / 1000.0);
        long endSample = (long)((double)endMS * rate / 1000.0);
        if (startSample < 0) startSample = 0;
        if (endSample > trackSize) endSample = trackSize;
        if (startSample < endSample) {
            long numSamples = endSample - startSample;
            float* leftData = audio->GetRawLeftDataPtr(startSample);
            float* rightData = audio->GetRawRightDataPtr(startSample);
            if (leftData != nullptr && rightData != nullptr) {
                std::vector<float> leftRegion(leftData, leftData + numSamples);
                std::vector<float> rightRegion(rightData, rightData + numSamples);
                std::filesystem::path m4a = outPath;
                m4a.replace_extension(".m4a");
                if (AudioManager::EncodeAudio(leftRegion, rightRegion, rate, m4a.string(), audio)) {
                    mediaRel = m4a.filename().string();
                }
            }
        }
    }

    // Step 2 — full document, then window it in-place. BuildDocument
    // emits every Effect with absolute startTime/endTime in ms; we
    // drop effects outside the region and clip+offset survivors by
    // -startMS, mirroring desktop's WriteRegionEffects.
    pugi::xml_document doc;
    if (!sf->BuildDocument(doc, _context->GetSequenceElements())) {
        return @"Failed to build sequence document.";
    }

    pugi::xml_node root = doc.child("xsequence");
    if (!root) return @"Malformed sequence document.";

    // Window every Effect node anywhere under ElementEffects.
    std::function<void(pugi::xml_node)> windowEffects = [&](pugi::xml_node parent) {
        for (pugi::xml_node child = parent.first_child(); child; ) {
            pugi::xml_node next = child.next_sibling();
            if (std::string(child.name()) == "Effect") {
                int s = (int)child.attribute("startTime").as_int(0);
                int e = (int)child.attribute("endTime").as_int(0);
                if (e <= startMS || s >= endMS) {
                    parent.remove_child(child);
                } else {
                    int cs = std::max(s, startMS) - startMS;
                    int ce = std::min(e, endMS) - startMS;
                    child.attribute("startTime") = cs;
                    child.attribute("endTime") = ce;
                }
            } else {
                windowEffects(child);
            }
            child = next;
        }
    };
    if (pugi::xml_node elementEffects = root.child("ElementEffects")) {
        windowEffects(elementEffects);
    }

    // Step 3 — head fixups: duration, media reference, region name.
    if (pugi::xml_node head = root.child("head")) {
        if (pugi::xml_node dur = head.child("sequenceDuration")) {
            char durStr[32];
            snprintf(durStr, sizeof(durStr), "%.3f", (double)regionDurationMS / 1000.0);
            dur.text().set(durStr);
        }
        if (pugi::xml_node media = head.child("mediaFile")) {
            media.text().set(mediaRel.c_str());
        }
        if (regionName.length > 0) {
            if (pugi::xml_node song = head.child("song")) {
                song.text().set([regionName UTF8String]);
            }
        }
        if (pugi::xml_node img = head.child("imageDir")) {
            img.text().set("");
        }
    }

    if (!doc.save_file(outPath.c_str(), "  ")) {
        return @"Failed to write sequence file.";
    }
    return @"";
}

- (void)clearUndoHistory {
    if (!_context) return;
    _context->GetSequenceElements().get_undo_mgr().Clear();
}

- (void)purgeRenderCache {
    if (!_context) return;
    _context->PurgeRenderCache();
}

- (void)purgeDownloadCache {
    if (!_context) return;
    _context->PurgeDownloadCache();
}

- (BOOL)saveSequenceAs:(NSString*)path {
    if (!_context || !_context->IsSequenceLoaded()) return NO;
    if (path.length == 0) return NO;
    auto* sf = _context->GetSequenceFile();
    if (!sf) return NO;

    std::string newPath([path UTF8String]);
    const std::string originalPath = sf->GetFullPath();
    sf->SetFullPath(newPath);

    ObtainAccessToURL(newPath, /*enforceWritable=*/true);
    bool ok = sf->Save(_context->GetSequenceElements());
    if (!ok) {
        // Roll back so a subsequent `-saveSequence` doesn't write
        // to the intended-but-failed destination.
        sf->SetFullPath(originalPath);
        return NO;
    }
    [self markSequenceClean];
    return YES;
}

- (NSString*)currentSequencePath {
    if (!_context) return @"";
    auto* sf = _context->GetSequenceFile();
    if (!sf) return @"";
    const std::string& p = sf->GetFullPath();
    return [NSString stringWithUTF8String:p.c_str()];
}

- (NSString*)sequenceFileVersion {
    if (!_context || !_context->IsSequenceLoaded()) return @"";
    auto* sf = _context->GetSequenceFile();
    if (!sf) return @"";
    const std::string& v = sf->GetVersion();
    return [NSString stringWithUTF8String:v.c_str()];
}

- (NSString*)currentAppVersion {
    return [NSString stringWithUTF8String:xlights_version_string.c_str()];
}

+ (NSString*)appVersion {
    return [NSString stringWithUTF8String:xlights_version_string.c_str()];
}

+ (NSString*)licenseText {
    return [NSString stringWithUTF8String:XLIGHTS_LICENSE];
}

// MARK: - Sequence Settings (E-3)

namespace {

/// Map iPad string keys to `HEADER_INFO_TYPES`. Returns nullopt
/// on unknown keys so callers can no-op / return empty.
static std::optional<HEADER_INFO_TYPES> headerTypeFromString(NSString* key) {
    if ([key isEqualToString:@"song"])    return HEADER_INFO_TYPES::SONG;
    if ([key isEqualToString:@"artist"])  return HEADER_INFO_TYPES::ARTIST;
    if ([key isEqualToString:@"album"])   return HEADER_INFO_TYPES::ALBUM;
    if ([key isEqualToString:@"author"])  return HEADER_INFO_TYPES::AUTHOR;
    if ([key isEqualToString:@"email"])   return HEADER_INFO_TYPES::AUTHOR_EMAIL;
    if ([key isEqualToString:@"website"]) return HEADER_INFO_TYPES::WEBSITE;
    if ([key isEqualToString:@"url"])     return HEADER_INFO_TYPES::URL;
    if ([key isEqualToString:@"comment"]) return HEADER_INFO_TYPES::COMMENT;
    return std::nullopt;
}

} // namespace

- (NSString*)headerInfoForKey:(NSString*)key {
    if (!_context || !_context->IsSequenceLoaded()) return @"";
    auto* sf = _context->GetSequenceFile();
    if (!sf) return @"";
    auto t = headerTypeFromString(key);
    if (!t) return @"";
    const std::string& v = sf->GetHeaderInfo(*t);
    return [NSString stringWithUTF8String:v.c_str()];
}

- (BOOL)setHeaderInfo:(NSString*)value forKey:(NSString*)key {
    if (!_context || !_context->IsSequenceLoaded()) return NO;
    auto* sf = _context->GetSequenceFile();
    if (!sf) return NO;
    auto t = headerTypeFromString(key);
    if (!t) return NO;
    std::string v = value ? std::string([value UTF8String]) : std::string();
    if (sf->GetHeaderInfo(*t) == v) return NO;
    sf->SetHeaderInfo(*t, v);
    _context->GetSequenceElements().IncrementChangeCount(nullptr);
    return YES;
}

- (NSString*)currentMediaFilePath {
    if (!_context || !_context->IsSequenceLoaded()) return @"";
    auto* sf = _context->GetSequenceFile();
    if (!sf) return @"";
    const std::string& v = sf->GetMediaFile();
    return [NSString stringWithUTF8String:v.c_str()];
}

- (NSString*)mediaFileHash {
    if (!_context) return @"N/A";
    AudioManager* am = _context->GetCurrentMediaManager();
    if (!am) return @"N/A";
    std::string h = am->Hash();
    return h.empty() ? @"N/A" : [NSString stringWithUTF8String:h.c_str()];
}

- (NSString*)audioTitle {
    if (!_context) return @"";
    AudioManager* am = _context->GetCurrentMediaManager();
    if (!am) return @"";
    return [NSString stringWithUTF8String:am->Title().c_str()];
}

- (NSString*)audioArtist {
    if (!_context) return @"";
    AudioManager* am = _context->GetCurrentMediaManager();
    if (!am) return @"";
    return [NSString stringWithUTF8String:am->Artist().c_str()];
}

- (NSString*)audioAlbum {
    if (!_context) return @"";
    AudioManager* am = _context->GetCurrentMediaManager();
    if (!am) return @"";
    return [NSString stringWithUTF8String:am->Album().c_str()];
}

- (BOOL)setMediaFilePath:(NSString*)path {
    if (!_context || !_context->IsSequenceLoaded()) return NO;
    auto* sf = _context->GetSequenceFile();
    if (!sf) return NO;
    std::string p = path ? std::string([path UTF8String]) : std::string();
    if (sf->GetMediaFile() == p) return NO;
    // Preserve any existing header metadata — user is swapping
    // the file, not re-tagging from the new one.
    sf->SetMediaFile(_context->GetShowDirectory(), p, /*overwrite_tags=*/false);
    _context->GetSequenceElements().IncrementChangeCount(nullptr);
    return YES;
}

- (NSString*)sequenceType {
    if (!_context || !_context->IsSequenceLoaded()) return @"";
    auto* sf = _context->GetSequenceFile();
    if (!sf) return @"";
    return [NSString stringWithUTF8String:sf->GetSequenceType().c_str()];
}

- (BOOL)setSequenceType:(NSString*)type {
    if (!_context || !_context->IsSequenceLoaded()) return NO;
    auto* sf = _context->GetSequenceFile();
    if (!sf || type.length == 0) return NO;
    std::string t([type UTF8String]);
    if (t != "Media" && t != "Animation" && t != "Effect") return NO;
    if (sf->GetSequenceType() == t) return NO;
    sf->SetSequenceType(t);  // auto-clears media + audio for Animation/Effect
    _context->GetSequenceElements().IncrementChangeCount(nullptr);
    return YES;
}

- (BOOL)setFrameIntervalMS:(int)frameMS {
    if (!_context || !_context->IsSequenceLoaded()) return NO;
    auto* sf = _context->GetSequenceFile();
    if (!sf || frameMS <= 0) return NO;
    if (sf->GetFrameMS() == frameMS) return NO;
    sf->SetSequenceTiming(std::to_string(frameMS) + " ms");
    _context->GetSequenceElements().IncrementChangeCount(nullptr);
    return YES;
}

- (BOOL)sequenceSupportsModelBlending {
    if (!_context || !_context->IsSequenceLoaded()) return NO;
    return _context->GetSequenceElements().SupportsModelBlending() ? YES : NO;
}

- (BOOL)setSequenceSupportsModelBlending:(BOOL)enabled {
    if (!_context || !_context->IsSequenceLoaded()) return NO;
    auto& elements = _context->GetSequenceElements();
    if (elements.SupportsModelBlending() == (bool)enabled) return NO;
    elements.SetSupportsModelBlending(enabled ? true : false);
    elements.IncrementChangeCount(nullptr);
    return YES;
}

- (NSString*)renderMode {
    if (!_context || !_context->IsSequenceLoaded()) return @"Erase";
    auto* sf = _context->GetSequenceFile();
    if (!sf) return @"Erase";
    // Desktop stores the literal choice string ("Erase"/"Canvas") on
    // the Nutcracker data layer once the user picks one; an untouched
    // sequence reports the ERASE_MODE sentinel. Normalize both to the
    // two UI-facing strings.
    const std::string mode = sf->GetRenderMode();
    if (mode == "Canvas" || mode == SequenceFile::CANVAS_MODE) return @"Canvas";
    return @"Erase";
}

- (BOOL)setRenderMode:(NSString*)mode {
    if (!_context || !_context->IsSequenceLoaded()) return NO;
    auto* sf = _context->GetSequenceFile();
    if (!sf || mode.length == 0) return NO;
    std::string m([mode UTF8String]);
    if (m != "Erase" && m != "Canvas") return NO;
    const std::string cur = sf->GetRenderMode();
    const bool curCanvas = (cur == "Canvas" || cur == SequenceFile::CANVAS_MODE);
    if ((m == "Canvas") == curCanvas) return NO;
    sf->SetRenderMode(m);
    _context->GetSequenceElements().IncrementChangeCount(nullptr);
    return YES;
}

- (int)sequenceModelCount {
    if (!_context || !_context->IsSequenceLoaded()) return 0;
    int count = 0;
    auto& se = _context->GetSequenceElements();
    for (size_t i = 0; i < se.GetElementCount(); ++i) {
        Element* e = se.GetElement(i);
        if (e && (e->GetType() == ElementType::ELEMENT_TYPE_MODEL
                  || e->GetType() == ElementType::ELEMENT_TYPE_SUBMODEL)) {
            count++;
        }
    }
    return count;
}

- (BOOL)writeAutosaveBackup {
    if (!_context || !_context->IsSequenceLoaded()) return NO;
    auto* sf = _context->GetSequenceFile();
    if (!sf) return NO;

    const std::string origPath = sf->GetFullPath();
    if (origPath.empty()) return NO;

    // Derive <basename>.xbkp alongside the current sequence.
    // Matches desktop's SaveWorking (`xLightsMain.cpp:4610-4614`).
    std::filesystem::path p(origPath);
    std::filesystem::path backup = p;
    backup.replace_extension("xbkp");
    const std::string backupPath = backup.string();

    ObtainAccessToURL(backupPath, /*enforceWritable=*/true);

    // Desktop's mFilePath-swap pattern: change the path, Save,
    // restore. Save only dereferences mFilePath in its final
    // `doc.save_file` call so the swap is safe.
    sf->SetFullPath(backupPath);
    const bool ok = sf->Save(_context->GetSequenceElements());
    sf->SetFullPath(origPath);

    // Writing the .xbkp doesn't count as a user save — leave
    // _lastSavedChangeCount untouched so the dirty dot stays lit.
    return ok ? YES : NO;
}

- (BOOL)isSequenceDirty {
    if (!_context || !_context->IsSequenceLoaded()) return NO;
    return _context->GetSequenceElements().GetChangeCount() != _lastSavedChangeCount
        ? YES : NO;
}

- (void)markSequenceClean {
    if (!_context || !_context->IsSequenceLoaded()) {
        _lastSavedChangeCount = 0;
        return;
    }
    _lastSavedChangeCount = _context->GetSequenceElements().GetChangeCount();
}

- (int)sequenceDurationMS {
    auto* sf = _context->GetSequenceFile();
    return sf ? sf->GetSequenceDurationMS() : 0;
}

- (BOOL)setSequenceDurationMS:(int)ms {
    if (!_context || !_context->IsSequenceLoaded()) return NO;
    auto* sf = _context->GetSequenceFile();
    if (!sf || ms <= 0) return NO;
    if (sf->GetSequenceDurationMS() == ms) return NO;
    sf->SetSequenceDurationMS(ms);
    _context->EnsureSequenceDataSized();
    _context->GetSequenceElements().IncrementChangeCount(nullptr);
    return YES;
}

- (int)frameIntervalMS {
    auto* sf = _context->GetSequenceFile();
    return sf ? sf->GetFrameMS() : 50;
}

- (NSString*)sequenceName {
    auto* sf = _context->GetSequenceFile();
    if (!sf) return @"";
    std::string path = sf->GetFullPath();
    auto pos = path.rfind('/');
    std::string name = (pos != std::string::npos) ? path.substr(pos + 1) : path;
    return [NSString stringWithUTF8String:name.c_str()];
}

- (int)visibleRowCount {
    return _context->GetSequenceElements().GetRowInformationSize();
}

- (NSString*)rowDisplayNameAtIndex:(int)index {
    auto* row = _context->GetSequenceElements().GetRowInformation(index);
    if (!row) return @"";
    return [NSString stringWithUTF8String:row->displayName.c_str()];
}

- (int)rowLayerIndexAtIndex:(int)index {
    auto* row = _context->GetSequenceElements().GetRowInformation(index);
    return row ? row->layerIndex : 0;
}

- (BOOL)rowIsCollapsedAtIndex:(int)index {
    auto* row = _context->GetSequenceElements().GetRowInformation(index);
    return row ? row->Collapsed : NO;
}

- (NSString*)rowModelNameAtIndex:(int)index {
    auto* row = _context->GetSequenceElements().GetRowInformation(index);
    if (!row || !row->element) return @"";
    // Only return a name for model-backed elements. Timing tracks don't map
    // to a displayable model.
    if (row->element->GetType() != ElementType::ELEMENT_TYPE_MODEL &&
        row->element->GetType() != ElementType::ELEMENT_TYPE_SUBMODEL) {
        return @"";
    }
    return [NSString stringWithUTF8String:row->element->GetModelName().c_str()];
}

- (BOOL)rowHasEffectsAtIndex:(int)index {
    if (!_context) return NO;
    auto* row = _context->GetSequenceElements().GetRowInformation(index);
    if (!row || !row->element) return NO;
    if (row->element->GetType() != ElementType::ELEMENT_TYPE_MODEL) return NO;
    return row->element->HasEffects() ? YES : NO;
}

- (NSString*)rowTagColorAtIndex:(int)index {
    if (!_context || !_context->HasModelManager()) return @"";
    auto* row = _context->GetSequenceElements().GetRowInformation(index);
    if (!row || !row->element) return @"";
    if (row->element->GetType() != ElementType::ELEMENT_TYPE_MODEL) return @"";
    Model* m = _context->GetModelManager()[row->element->GetModelName()];
    if (!m) return @"";
    xlColor tag = m->GetTagColour();
    if (tag == xlBLACK) return @"";
    return [NSString stringWithUTF8String:m->GetTagColourAsString().c_str()];
}

- (NSString*)rowNodeMaskColorAtIndex:(int)index {
    if (!_context || !_context->HasModelManager()) return @"";
    auto* row = _context->GetSequenceElements().GetRowInformation(index);
    if (!row || !row->element) return @"";
    if (row->element->GetType() != ElementType::ELEMENT_TYPE_MODEL) return @"";
    Model* m = _context->GetModelManager()[row->element->GetModelName()];
    if (!m) return @"";
    const std::string st = m->GetStringType();
    if (st.rfind("Single Color", 0) != 0 && st != "Node Single Color") return @"";
    if (m->GetNodeCount() <= 0) return @"";
    std::string hex = (std::string)m->GetNodeMaskColor(0);
    return [NSString stringWithUTF8String:hex.c_str()];
}

// MARK: - Timing Rows

- (NSArray<NSNumber*>*)timingRowIndices {
    NSMutableArray<NSNumber*>* out = [NSMutableArray array];
    auto& se = _context->GetSequenceElements();
    int n = se.GetRowInformationSize();
    for (int i = 0; i < n; i++) {
        auto* row = se.GetRowInformation(i);
        if (row && row->element &&
            row->element->GetType() == ElementType::ELEMENT_TYPE_TIMING) {
            [out addObject:@(i)];
        }
    }
    return out;
}

- (BOOL)timingRowIsActiveAtIndex:(int)rowIndex {
    auto* row = _context->GetSequenceElements().GetRowInformation(rowIndex);
    if (!row || !row->element ||
        row->element->GetType() != ElementType::ELEMENT_TYPE_TIMING) return NO;
    auto* te = dynamic_cast<TimingElement*>(row->element);
    return te && te->GetActive() ? YES : NO;
}

- (void)setTimingRowActive:(BOOL)active atIndex:(int)rowIndex {
    auto* row = _context->GetSequenceElements().GetRowInformation(rowIndex);
    if (!row || !row->element ||
        row->element->GetType() != ElementType::ELEMENT_TYPE_TIMING) return;
    auto* te = dynamic_cast<TimingElement*>(row->element);
    if (!te) return;
    // Desktop enforces single-active (RowHeading.cpp:365-371) —
    // activating one timing track clears every other. Deactivating
    // the currently-active one just flips it off.
    auto& se = _context->GetSequenceElements();
    if (active) {
        se.DeactivateAllTimingElements();
        te->SetActive(true);
    } else {
        te->SetActive(false);
    }
}

// B81: hide / show every timing row. Flips SetVisible +
// SetMasterVisible on every timing element then repopulates row
// info so the iPad view reflects the change after `reloadRows`.
// Returns the new hide state so the caller can update any
// dependent UI (menu label flip).
- (void)setAllTimingTracksHidden:(BOOL)hidden {
    auto& se = _context->GetSequenceElements();
    se.HideAllTimingTracks(hidden ? true : false);
    se.PopulateRowInformation();
}

- (void)setHideUnusedSubmodels:(BOOL)hide {
    auto& se = _context->GetSequenceElements();
    se.SetHideUnusedSubmodels(hide ? true : false);
    se.PopulateRowInformation();
}

- (BOOL)hideUnusedSubmodels {
    return _context->GetSequenceElements().GetHideUnusedSubmodels() ? YES : NO;
}

// B82: add every visible timing track to every defined view.
// Mirrors desktop's `AddTimingTracksToAllViews` row-menu entry.
// Returns the count of timing-track-to-view additions performed
// (informational only — callers don't rely on it).
- (int)addAllTimingTracksToAllViews {
    auto& se = _context->GetSequenceElements();
    _context->AbortRender(5000);
    int added = 0;
    for (int i = 0; i < (int)se.GetElementCount(); i++) {
        Element* e = se.GetElement(i);
        if (!e) continue;
        if (e->GetType() != ElementType::ELEMENT_TYPE_TIMING) continue;
        if (!e->GetVisible()) continue;
        se.AddTimingToAllViews(e->GetName());
        ++added;
    }
    if (added > 0) {
        se.PopulateRowInformation();
    }
    return added;
}

- (BOOL)allTimingTracksHidden {
    auto& se = _context->GetSequenceElements();
    int n = se.GetRowInformationSize();
    for (int i = 0; i < n; i++) {
        auto* row = se.GetRowInformation(i);
        if (row && row->element &&
            row->element->GetType() == ElementType::ELEMENT_TYPE_TIMING) {
            // If any timing row is present in the row info, at least one
            // is currently visible — i.e. not all hidden.
            return NO;
        }
    }
    return YES;
}

- (int)timingRowColorIndexAtIndex:(int)rowIndex {
    auto* row = _context->GetSequenceElements().GetRowInformation(rowIndex);
    if (!row) return 0;
    return row->colorIndex;
}

- (NSString*)timingRowElementNameAtIndex:(int)rowIndex {
    auto* row = _context->GetSequenceElements().GetRowInformation(rowIndex);
    if (!row || !row->element) return @"";
    return [NSString stringWithUTF8String:row->element->GetName().c_str()];
}

- (NSString*)rowLayerNameAtIndex:(int)rowIndex {
    auto* row = _context->GetSequenceElements().GetRowInformation(rowIndex);
    if (!row) return @"";
    return [NSString stringWithUTF8String:row->layerName.c_str()];
}

// MARK: - Model Row Metadata

- (BOOL)rowIsModelGroupAtIndex:(int)rowIndex {
    auto* row = _context->GetSequenceElements().GetRowInformation(rowIndex);
    if (!row || !row->element) return NO;
    if (row->element->GetType() != ElementType::ELEMENT_TYPE_MODEL) return NO;
    Model* m = _context->GetModelManager()[row->element->GetModelName()];
    return (m != nullptr && m->GetDisplayAs() == DisplayAsType::ModelGroup) ? YES : NO;
}

- (int)rowLayerCountAtIndex:(int)rowIndex {
    auto* row = _context->GetSequenceElements().GetRowInformation(rowIndex);
    if (!row || !row->element) return 0;
    return (int)row->element->GetEffectLayerCount();
}

- (BOOL)rowIsElementCollapsedAtIndex:(int)rowIndex {
    auto* row = _context->GetSequenceElements().GetRowInformation(rowIndex);
    if (!row || !row->element) return NO;
    return row->element->GetCollapsed() ? YES : NO;
}

- (void)toggleElementCollapsedAtIndex:(int)rowIndex {
    auto* row = _context->GetSequenceElements().GetRowInformation(rowIndex);
    if (!row || !row->element) return;
    row->element->SetCollapsed(!row->element->GetCollapsed());
    _context->GetSequenceElements().PopulateRowInformation();
}

- (void)collapseAllElements {
    auto& se = _context->GetSequenceElements();
    size_t n = se.GetElementCount(se.GetCurrentView());
    for (size_t i = 0; i < n; i++) {
        Element* e = se.GetElement(i, se.GetCurrentView());
        if (!e) continue;
        if (e->GetType() == ElementType::ELEMENT_TYPE_TIMING) continue;
        e->SetCollapsed(true);
    }
    se.PopulateRowInformation();
}

- (void)expandAllElements {
    auto& se = _context->GetSequenceElements();
    size_t n = se.GetElementCount(se.GetCurrentView());
    for (size_t i = 0; i < n; i++) {
        Element* e = se.GetElement(i, se.GetCurrentView());
        if (!e) continue;
        if (e->GetType() == ElementType::ELEMENT_TYPE_TIMING) continue;
        e->SetCollapsed(false);
    }
    se.PopulateRowInformation();
}

- (void)collapseAllModels {
    // Desktop "Collapse All Models" (RowHeading.cpp): hide every
    // ModelElement's strands + submodels (distinct from "Collapse All
    // Layers", which `collapseAllElements` already covers via
    // SetCollapsed).
    auto& se = _context->GetSequenceElements();
    size_t n = se.GetElementCount(se.GetCurrentView());
    for (size_t i = 0; i < n; i++) {
        Element* e = se.GetElement(i, se.GetCurrentView());
        if (!e) continue;
        if (e->GetType() == ElementType::ELEMENT_TYPE_TIMING) continue;
        if (ModelElement* me = dynamic_cast<ModelElement*>(e)) {
            me->ShowStrands(false);
            me->ShowSubModels(false);
        }
    }
    se.PopulateRowInformation();
}

// Scoped delete helpers — mirror desktop RowHeading.cpp
// ID_ROW_MNU_DELETE_MODEL_{SUBMODEL,STRAND,NODE}_EFFECTS. Each clears
// every effect on the relevant layers under the row's ModelElement.
// Returns the number of layers cleared (informational).
- (int)deleteScopedEffectsAtRow:(int)rowIndex scope:(int)scope {
    auto& se = _context->GetSequenceElements();
    auto* row = se.GetRowInformation(rowIndex);
    if (!row || !row->element) return 0;
    ModelElement* me = dynamic_cast<ModelElement*>(row->element);
    if (!me) {
        if (auto* sm = dynamic_cast<SubModelElement*>(row->element)) {
            me = sm->GetModelElement();
        }
    }
    if (!me) return 0;

    se.get_undo_mgr().CreateUndoStep();
    _context->AbortRender(5000);
    int cleared = 0;
    auto clearLayer = [&](EffectLayer* l) {
        if (l && l->GetEffectCount() > 0) {
            l->RemoveAllEffects(&se.get_undo_mgr());
            ++cleared;
        }
    };

    if (scope == 0) { // submodels
        for (int s = 0; s < me->GetSubModelCount(); ++s) {
            SubModelElement* sub = me->GetSubModel(s);
            if (!sub) continue;
            for (int i = 0; i < (int)sub->GetEffectLayerCount(); ++i) {
                clearLayer(sub->GetEffectLayer(i));
            }
        }
    } else if (scope == 1) { // strands
        for (int s = 0; s < me->GetStrandCount(); ++s) {
            StrandElement* st = me->GetStrand(s);
            if (!st) continue;
            for (int i = 0; i < (int)st->GetEffectLayerCount(); ++i) {
                clearLayer(st->GetEffectLayer(i));
            }
        }
    } else { // nodes
        for (int s = 0; s < me->GetStrandCount(); ++s) {
            StrandElement* st = me->GetStrand(s);
            if (!st) continue;
            for (int nidx = 0; nidx < st->GetNodeLayerCount(); ++nidx) {
                clearLayer(st->GetNodeLayer(nidx));
            }
        }
    }
    if (cleared > 0) {
        _context->RenderEffectForModel(me->GetModelName(), 0, 99999999, true);
    }
    return cleared;
}

- (void)expandElementsWithEffects {
    // SEQ-15 "Show All Effects": expand every model element that has effects
    // (so all effects are visible) and collapse the empty ones to declutter.
    auto& se = _context->GetSequenceElements();
    size_t n = se.GetElementCount(se.GetCurrentView());
    for (size_t i = 0; i < n; i++) {
        Element* e = se.GetElement(i, se.GetCurrentView());
        if (!e) continue;
        if (e->GetType() == ElementType::ELEMENT_TYPE_TIMING) continue;
        e->SetCollapsed(!e->HasEffects());
    }
    se.PopulateRowInformation();
}

- (BOOL)renameLayerAtRow:(int)rowIndex name:(NSString*)newName {
    auto* layer = [self effectLayerForRow:rowIndex];
    if (!layer) return NO;
    std::string n = newName ? std::string([newName UTF8String]) : std::string();
    layer->SetLayerName(n);
    _context->GetSequenceElements().PopulateRowInformation();
    return YES;
}

- (BOOL)elementRenderDisabledAtRow:(int)rowIndex {
    auto* row = _context->GetSequenceElements().GetRowInformation(rowIndex);
    if (!row || !row->element) return NO;
    return row->element->IsRenderDisabled() ? YES : NO;
}

- (void)setElementRenderDisabled:(BOOL)disabled atRow:(int)rowIndex {
    auto* row = _context->GetSequenceElements().GetRowInformation(rowIndex);
    if (!row || !row->element) return;
    row->element->SetRenderDisabled(disabled ? true : false);
}

- (int)effectCountOnRow:(int)rowIndex {
    auto* layer = [self effectLayerForRow:rowIndex];
    if (!layer) return 0;
    return layer->GetEffectCount();
}

- (BOOL)timingTrackIsFixedAtRow:(int)rowIndex {
    auto* row = _context->GetSequenceElements().GetRowInformation(rowIndex);
    if (!row || !row->element) return NO;
    if (row->element->GetType() != ElementType::ELEMENT_TYPE_TIMING) return NO;
    TimingElement* te = dynamic_cast<TimingElement*>(row->element);
    return (te && te->IsFixedTiming()) ? YES : NO;
}

- (BOOL)makeTimingTrackVariableAtRow:(int)rowIndex {
    auto* row = _context->GetSequenceElements().GetRowInformation(rowIndex);
    if (!row || !row->element) return NO;
    if (row->element->GetType() != ElementType::ELEMENT_TYPE_TIMING) return NO;
    TimingElement* te = dynamic_cast<TimingElement*>(row->element);
    if (!te || !te->IsFixedTiming()) return NO;
    te->SetFixedTiming(0);
    _context->GetSequenceElements().PopulateRowInformation();
    return YES;
}

- (int)importLyricsAtRow:(int)rowIndex
                 phrases:(NSArray<NSString*>*)phrases
                 startMS:(int)startMS
                   endMS:(int)endMS {
    auto& se = _context->GetSequenceElements();
    auto* row = se.GetRowInformation(rowIndex);
    if (!row || !row->element) return 0;
    if (row->element->GetType() != ElementType::ELEMENT_TYPE_TIMING) return 0;
    TimingElement* te = dynamic_cast<TimingElement*>(row->element);
    if (!te) return 0;
    if (!phrases || phrases.count == 0) return 0;

    // Clean + count non-empty phrases up front.
    NSMutableArray<NSString*>* cleaned = [NSMutableArray array];
    for (NSString* raw in phrases) {
        if (!raw) continue;
        NSString* line = [raw stringByTrimmingCharactersInSet:
                            [NSCharacterSet whitespaceAndNewlineCharacterSet]];
        if (line.length == 0) continue;
        // Strip common smart-quote unicode + illegal XML bits.
        line = [line stringByReplacingOccurrencesOfString:@"’" withString:@"'"];
        line = [line stringByReplacingOccurrencesOfString:@"Ș" withString:@"'"];
        line = [line stringByReplacingOccurrencesOfString:@"“" withString:@"\""];
        line = [line stringByReplacingOccurrencesOfString:@"”" withString:@"\""];
        line = [line stringByReplacingOccurrencesOfString:@"\"" withString:@""];
        line = [line stringByReplacingOccurrencesOfString:@"<" withString:@""];
        line = [line stringByReplacingOccurrencesOfString:@">" withString:@""];
        if (line.length == 0) continue;
        [cleaned addObject:line];
    }
    if (cleaned.count == 0) return 0;

    // Clamp range. Desktop falls back to full-sequence range if the
    // user-entered times are nonsensical.
    int seqEnd = se.GetSequenceEnd();
    if (startMS < 0) startMS = 0;
    if (endMS <= 0 || endMS > seqEnd) endMS = seqEnd;
    if (endMS <= startMS) { startMS = 0; endMS = seqEnd; }
    if (endMS <= startMS) return 0;

    // Replace all layers with a single fresh phrase layer.
    te->SetFixedTiming(0);
    while (te->GetEffectLayerCount() > 0) {
        te->RemoveEffectLayer((int)te->GetEffectLayerCount() - 1);
    }
    EffectLayer* phraseLayer = te->AddEffectLayer();
    if (!phraseLayer) return 0;

    double freq = se.GetFrequency();
    int intervalMS = (endMS - startMS) / (int)cleaned.count;
    int curStart = startMS;
    int added = 0;
    for (NSUInteger i = 0; i < cleaned.count; i++) {
        int curEnd = RoundToMultipleOfPeriod(curStart + intervalMS, freq);
        if (i == cleaned.count - 1 || curEnd > endMS) curEnd = endMS;
        if (curEnd <= curStart) break;
        std::string lbl = [cleaned[i] UTF8String];
        if (phraseLayer->AddEffect(0, lbl, "", "",
                                    curStart, curEnd,
                                    /*EFFECT_NOT_SELECTED*/ 0, false)) {
            added++;
        }
        curStart = curEnd;
    }
    se.PopulateRowInformation();
    return added;
}

- (int)importXTimingFromPath:(NSString*)path {
    if (!path || path.length == 0) return 0;
    SequenceFile* sf = _context->GetSequenceFile();
    if (!sf) return 0;
    int countBefore = _context->GetSequenceElements().GetNumberOfTimingElements();
    std::vector<std::string> filenames;
    filenames.push_back(std::string([path UTF8String]));
    sf->ProcessXTiming(filenames, _context.get());
    int countAfter = _context->GetSequenceElements().GetNumberOfTimingElements();
    if (countAfter > countBefore) {
        // Mirror desktop's post-import behavior: make the newest
        // imported timing track active.
        _context->GetSequenceElements().DeactivateAllTimingElements();
        TimingElement* te = _context->GetSequenceElements().GetTimingElement(countAfter - 1);
        if (te) te->SetActive(true);
        _context->GetSequenceElements().PopulateRowInformation();
    }
    return countAfter - countBefore;
}

- (int)importLorTimingFromPath:(NSString*)path {
    if (!path || path.length == 0) return 0;
    SequenceFile* sf = _context->GetSequenceFile();
    if (!sf) return 0;
    int countBefore = _context->GetSequenceElements().GetNumberOfTimingElements();
    sf->ProcessLorTiming({ std::string([path UTF8String]) }, _context.get());
    int countAfter = _context->GetSequenceElements().GetNumberOfTimingElements();
    if (countAfter > countBefore) {
        _context->GetSequenceElements().DeactivateAllTimingElements();
        TimingElement* te = _context->GetSequenceElements().GetTimingElement(countAfter - 1);
        if (te) te->SetActive(true);
        _context->GetSequenceElements().PopulateRowInformation();
    }
    return countAfter - countBefore;
}

- (int)importPapagayoTimingFromPath:(NSString*)path {
    if (!path || path.length == 0) return 0;
    SequenceFile* sf = _context->GetSequenceFile();
    if (!sf) return 0;
    int countBefore = _context->GetSequenceElements().GetNumberOfTimingElements();
    sf->ProcessPapagayo({ std::string([path UTF8String]) }, _context.get());
    int countAfter = _context->GetSequenceElements().GetNumberOfTimingElements();
    if (countAfter > countBefore) {
        _context->GetSequenceElements().DeactivateAllTimingElements();
        TimingElement* te = _context->GetSequenceElements().GetTimingElement(countAfter - 1);
        if (te) te->SetActive(true);
        _context->GetSequenceElements().PopulateRowInformation();
    }
    return countAfter - countBefore;
}

// IE-1 — shared body for the file-based timing importers. Runs the
// supplied processor block, then mirrors the LOR/Papagayo importers'
// post-import behavior (activate the newest imported track). Returns
// the number of timing tracks added.
- (int)importTimingVia:(void (^)(SequenceFile* sf))process {
    if (!process) return 0;
    SequenceFile* sf = _context->GetSequenceFile();
    if (!sf) return 0;
    int countBefore = _context->GetSequenceElements().GetNumberOfTimingElements();
    process(sf);
    int countAfter = _context->GetSequenceElements().GetNumberOfTimingElements();
    if (countAfter > countBefore) {
        _context->GetSequenceElements().DeactivateAllTimingElements();
        TimingElement* te = _context->GetSequenceElements().GetTimingElement(countAfter - 1);
        if (te) te->SetActive(true);
        _context->GetSequenceElements().PopulateRowInformation();
    }
    return countAfter - countBefore;
}

- (int)importSRTFromPath:(NSString*)path {
    if (!path || path.length == 0) return 0;
    std::string p([path UTF8String]);
    return [self importTimingVia:^(SequenceFile* sf) { sf->ProcessSRT({ p }, self->_context.get()); }];
}

- (int)importAudacityTimingFromPath:(NSString*)path {
    if (!path || path.length == 0) return 0;
    std::string p([path UTF8String]);
    return [self importTimingVia:^(SequenceFile* sf) { sf->ProcessAudacityTimingFiles({ p }, self->_context.get()); }];
}

- (int)importElevenLabsTimingFromPath:(NSString*)path {
    if (!path || path.length == 0) return 0;
    std::string p([path UTF8String]);
    return [self importTimingVia:^(SequenceFile* sf) { sf->ProcessElevenLabsTimingFiles({ p }, self->_context.get()); }];
}

- (int)importVixen3TimingFromPath:(NSString*)path {
    return [self importVixen3TimingFromPath:path selectedIndices:@[]];
}

- (NSArray<NSString*>*)vixen3TimingTrackNamesFromPath:(NSString*)path {
    NSMutableArray<NSString*>* result = [NSMutableArray array];
    if (!path || path.length == 0) return result;
    SequenceFile* sf = _context->GetSequenceFile();
    if (!sf) return result;
    std::string p([path UTF8String]);
    for (const std::string& n : sf->GetVixen3TimingTrackNames(p)) {
        [result addObject:[NSString stringWithUTF8String:n.c_str()]];
    }
    return result;
}

- (int)importVixen3TimingFromPath:(NSString*)path selectedIndices:(NSArray<NSNumber*>*)indices {
    if (!path || path.length == 0) return 0;
    std::string p([path UTF8String]);
    std::vector<int> idx;
    for (NSNumber* n in indices) idx.push_back(n.intValue);
    return [self importTimingVia:^(SequenceFile* sf) { sf->ProcessVixen3Timing(p, idx, self->_context.get()); }];
}

- (int)importLSPTimingFromPath:(NSString*)path {
    if (!path || path.length == 0) return 0;
    std::string p([path UTF8String]);
    return [self importTimingVia:^(SequenceFile* sf) { sf->ProcessLSPTiming({ p }, self->_context.get()); }];
}

- (int)importXLightsSequenceTimingFromPath:(NSString*)path {
    return [self importXLightsSequenceTimingFromPath:path selectedIndices:@[]];
}

- (NSArray<NSString*>*)xLightsTimingTrackNamesFromPath:(NSString*)path {
    NSMutableArray<NSString*>* result = [NSMutableArray array];
    if (!path || path.length == 0) return result;
    SequenceFile* sf = _context->GetSequenceFile();
    if (!sf) return result;
    std::string p([path UTF8String]);
    for (const std::string& n : sf->GetXLightsTimingTrackNames(p, _context.get())) {
        [result addObject:[NSString stringWithUTF8String:n.c_str()]];
    }
    return result;
}

- (int)importXLightsSequenceTimingFromPath:(NSString*)path selectedIndices:(NSArray<NSNumber*>*)indices {
    if (!path || path.length == 0) return 0;
    std::string p([path UTF8String]);
    std::vector<int> idx;
    for (NSNumber* n in indices) idx.push_back(n.intValue);
    return [self importTimingVia:^(SequenceFile* sf) { sf->ProcessXLightsTiming(p, idx, self->_context.get()); }];
}

- (BOOL)exportTimingTrackAtRow:(int)rowIndex toPath:(NSString*)path {
    if (!path || path.length == 0) return NO;
    auto* row = _context->GetSequenceElements().GetRowInformation(rowIndex);
    if (!row || !row->element) return NO;
    if (row->element->GetType() != ElementType::ELEMENT_TYPE_TIMING) return NO;
    TimingElement* te = dynamic_cast<TimingElement*>(row->element);
    if (!te) return NO;
    // Build the full `.xtiming` document:
    //   <?xml version="1.0" encoding="UTF-8"?>
    //   <timing name="..." subType="..." SourceVersion="...">
    //     <EffectLayer><Effect .../>…</EffectLayer>
    //     …
    //   </timing>
    std::string doc;
    doc.reserve(2048);
    doc += "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n";
    doc += "<timing name=\"" + XmlSafe(te->GetName()) + "\" ";
    doc += "subType=\"" + te->GetSubType() + "\" ";
    doc += std::string("SourceVersion=\"") + xlights_version_string + "\">\n";
    doc += te->GetExport();
    doc += "</timing>\n";
    NSData* data = [NSData dataWithBytes:doc.data() length:doc.size()];
    NSError* err = nil;
    if (![data writeToFile:path options:NSDataWritingAtomic error:&err]) {
        NSLog(@"exportTimingTrack failed: %@", err);
        return NO;
    }
    return YES;
}

- (BOOL)exportTimingTracksAtRows:(NSArray<NSNumber*>*)rowIndices toPath:(NSString*)path {
    if (!path || path.length == 0 || rowIndices.count == 0) return NO;
    // `<timings>` wrapper holding one `<timing>` per selected track —
    // same per-track envelope as exportTimingTrackAtRow:.
    std::string doc;
    doc.reserve(4096);
    doc += "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n";
    doc += std::string("<timings SourceVersion=\"") + xlights_version_string + "\">\n";
    int written = 0;
    for (NSNumber* n in rowIndices) {
        auto* row = _context->GetSequenceElements().GetRowInformation(n.intValue);
        if (!row || !row->element) continue;
        if (row->element->GetType() != ElementType::ELEMENT_TYPE_TIMING) continue;
        TimingElement* te = dynamic_cast<TimingElement*>(row->element);
        if (!te) continue;
        doc += "<timing name=\"" + XmlSafe(te->GetName()) + "\" ";
        doc += "subType=\"" + te->GetSubType() + "\" ";
        doc += std::string("SourceVersion=\"") + xlights_version_string + "\">\n";
        doc += te->GetExport();
        doc += "</timing>\n";
        ++written;
    }
    doc += "</timings>\n";
    if (written == 0) return NO;
    NSData* data = [NSData dataWithBytes:doc.data() length:doc.size()];
    NSError* err = nil;
    if (![data writeToFile:path options:NSDataWritingAtomic error:&err]) {
        NSLog(@"exportTimingTracks failed: %@", err);
        return NO;
    }
    return YES;
}

- (BOOL)exportTimingTrackAsPapagayoAtRow:(int)rowIndex toPath:(NSString*)path {
    if (!path || path.length == 0) return NO;
    auto& se = _context->GetSequenceElements();
    auto* row = se.GetRowInformation(rowIndex);
    if (!row || !row->element) return NO;
    if (row->element->GetType() != ElementType::ELEMENT_TYPE_TIMING) return NO;
    TimingElement* te = dynamic_cast<TimingElement*>(row->element);
    if (!te) return NO;
    std::string doc = te->GetPapagayoExport((int)se.GetFrequency());
    // GetPapagayoExport returns "" unless the track has the full
    // phrase/word/phoneme breakdown (3 layers). Don't write an empty file.
    if (doc.empty()) return NO;
    NSData* data = [NSData dataWithBytes:doc.data() length:doc.size()];
    NSError* err = nil;
    if (![data writeToFile:path options:NSDataWritingAtomic error:&err]) {
        NSLog(@"exportTimingTrackAsPapagayo failed: %@", err);
        return NO;
    }
    return YES;
}

- (BOOL)exportModelAsFSEQAtRow:(int)rowIndex toPath:(NSString*)path
                       startMS:(int)startMS endMS:(int)endMS {
    if (!path || path.length == 0) return NO;
    auto* row = _context->GetSequenceElements().GetRowInformation(rowIndex);
    if (!row || !row->element) return NO;
    if (row->element->GetType() == ElementType::ELEMENT_TYPE_TIMING) return NO;
    const std::string modelName = row->element->GetName();
    if (modelName.empty()) return NO;

    Model* model = _context->GetModel(modelName);
    if (!model) return NO;

    RenderEngine* engine = _context->GetRenderEngine();
    if (!engine) return NO;

    SequenceData& fullData = _context->GetSequenceData();
    if (fullData.NumFrames() == 0 || fullData.NumChannels() == 0) return NO;

    // Extract per-model channel data. Uses the current `_sequenceData`
    // — callers should trigger a render before exporting if they want
    // fresh values; the background render engine keeps it current
    // during normal editing.
    auto exported = engine->ExportModelData(modelName, fullData);
    if (!exported.data) return NO;
    SequenceData& modelData = *exported.data;
    const uint32_t totalFrames = (uint32_t)modelData.NumFrames();
    if (totalFrames == 0) return NO;

    // Clamp to requested [startMS, endMS] window. Negative values or
    // zero-length windows fall back to the full sequence.
    const int frameTime = modelData.FrameTime();
    uint32_t startFrame = 0;
    uint32_t endFrame = totalFrames;
    if (startMS >= 0 && endMS > startMS && frameTime > 0) {
        startFrame = std::min<uint32_t>(totalFrames, (uint32_t)(startMS / frameTime));
        endFrame = std::min<uint32_t>(totalFrames,
                                      (uint32_t)((endMS + frameTime - 1) / frameTime));
    }
    if (endFrame <= startFrame) return NO;

    const std::string outPath = [path UTF8String];
    const uint32_t modelChans = (uint32_t)model->GetActChanCount();
    const uint32_t startAddr = (uint32_t)model->NodeStartChannel(0) + 1;

    // Falcon Pi v2 compressed FSEQ sub-sequence. Matches
    // `xLightsFrame::WriteFalconPiModelFile` v2 branch.
    std::unique_ptr<FSEQFile> f(FSEQFile::createFSEQFile(outPath, 2,
                                                         FSEQFile::CompressionType::zstd,
                                                         -99));
    V2FSEQFile* v2 = dynamic_cast<V2FSEQFile*>(f.get());
    if (!v2) return NO;

    v2->setNumFrames(endFrame - startFrame);
    v2->setStepTime(frameTime);
    v2->setChannelCount((uint64_t)startAddr + modelChans - 1);
    v2->m_sparseRanges.push_back(std::pair<uint32_t, uint32_t>(startAddr - 1, modelChans));
    v2->writeHeader();
    // Data buffer only contains the model's channels (0..modelChans-1).
    v2->m_sparseRanges[0] = std::pair<uint32_t, uint32_t>(0, modelChans);
    for (uint32_t fr = startFrame; fr < endFrame; ++fr) {
        v2->addFrame(fr - startFrame, &modelData[fr][0]);
    }
    v2->finalize();
    return YES;
}

- (void)exportModelAsGIFAtRow:(int)rowIndex toPath:(NSString*)path
                      startMS:(int)startMS endMS:(int)endMS
                   completion:(void (^)(BOOL))completion {
    void (^finishNO)(void) = ^{
        if (completion) dispatch_async(dispatch_get_main_queue(), ^{ completion(NO); });
    };
    if (!path || path.length == 0 || !_context) { finishNO(); return; }
    auto* row = _context->GetSequenceElements().GetRowInformation(rowIndex);
    if (!row || !row->element) { finishNO(); return; }
    if (row->element->GetType() == ElementType::ELEMENT_TYPE_TIMING) { finishNO(); return; }
    const std::string modelName = row->element->GetName();
    if (modelName.empty()) { finishNO(); return; }

    Model* model = _context->GetModel(modelName);
    if (!model || model->GetDisplayAs() == DisplayAsType::ModelGroup) { finishNO(); return; }

    RenderEngine* engine = _context->GetRenderEngine();
    if (!engine) { finishNO(); return; }

    SequenceData& fullData = _context->GetSequenceData();
    if (fullData.NumFrames() == 0 || fullData.NumChannels() == 0) { finishNO(); return; }

    // Slice the per-model channel data on the main thread (reads the live
    // _sequenceData) into a private copy; only the encode runs in the
    // background, operating on that copy + immutable model geometry.
    auto exported = engine->ExportModelData(modelName, fullData);
    if (!exported.data) { finishNO(); return; }
    std::shared_ptr<SequenceData> modelData(std::move(exported.data));
    const uint32_t totalFrames = (uint32_t)modelData->NumFrames();
    if (totalFrames == 0) { finishNO(); return; }

    const int frameTime = modelData->FrameTime();
    uint32_t startFrame = 0;
    uint32_t endFrame = totalFrames;
    if (startMS >= 0 && endMS > startMS && frameTime > 0) {
        startFrame = std::min<uint32_t>(totalFrames, (uint32_t)(startMS / frameTime));
        endFrame = std::min<uint32_t>(totalFrames,
                                      (uint32_t)((endMS + frameTime - 1) / frameTime));
    }
    if (endFrame <= startFrame) { finishNO(); return; }

    const int startAddr = model->GetNumberFromChannelString(model->ModelStartChannel);
    const std::string outPath = [path UTF8String];
    const uint32_t sf = startFrame;
    const uint32_t ef = endFrame;
    const uint32_t ft = (uint32_t)frameTime;

    dispatch_async(dispatch_get_global_queue(QOS_CLASS_USER_INITIATED, 0), ^{
        bool ok = ModelGifExporter::WriteModelGif(outPath, modelData.get(),
                                                  sf, ef, model, startAddr, ft);
        if (completion) dispatch_async(dispatch_get_main_queue(), ^{ completion(ok ? YES : NO); });
    });
}

- (BOOL)exportModelAsVideoAtRow:(int)rowIndex toPath:(NSString*)path
                     compressed:(BOOL)compressed highQuality:(BOOL)highQuality
                    forceProRes:(BOOL)forceProRes
                        startMS:(int)startMS endMS:(int)endMS {
    if (!path || path.length == 0) return NO;
    auto* row = _context->GetSequenceElements().GetRowInformation(rowIndex);
    if (!row || !row->element) return NO;
    if (row->element->GetType() == ElementType::ELEMENT_TYPE_TIMING) return NO;
    const std::string modelName = row->element->GetName();
    if (modelName.empty()) return NO;

    Model* model = _context->GetModel(modelName);
    if (!model) return NO;
    // Model groups have no single buffer to export as video (desktop disables
    // the menu for groups); guard here too.
    if (model->GetDisplayAs() == DisplayAsType::ModelGroup) return NO;

    RenderEngine* engine = _context->GetRenderEngine();
    if (!engine) return NO;

    SequenceData& fullData = _context->GetSequenceData();
    if (fullData.NumFrames() == 0 || fullData.NumChannels() == 0) return NO;

    auto exported = engine->ExportModelData(modelName, fullData);
    if (!exported.data) return NO;
    SequenceData& modelData = *exported.data;
    const uint32_t totalFrames = (uint32_t)modelData.NumFrames();
    if (totalFrames == 0) return NO;

    const int frameTime = modelData.FrameTime();
    uint32_t startFrame = 0;
    uint32_t endFrame = totalFrames;
    if (startMS >= 0 && endMS > startMS && frameTime > 0) {
        startFrame = std::min<uint32_t>(totalFrames, (uint32_t)(startMS / frameTime));
        endFrame = std::min<uint32_t>(totalFrames,
                                      (uint32_t)((endMS + frameTime - 1) / frameTime));
    }
    if (endFrame <= startFrame) return NO;

    // startAddr matches desktop DoExportModel: the model's absolute start
    // channel. WriteModelVideo indexes the rebased per-model buffer by
    // (GetFirstChannel() - startAddr + 1).
    const int startAddr = model->GetNumberFromChannelString(model->ModelStartChannel);

    return ModelVideoExporter::WriteModelVideo([path UTF8String], &modelData,
                                               startFrame, endFrame, model, startAddr,
                                               compressed, highQuality, forceProRes)
        ? YES : NO;
}

- (void)exportModelAsVideoAtRow:(int)rowIndex toPath:(NSString*)path
                     compressed:(BOOL)compressed highQuality:(BOOL)highQuality
                    forceProRes:(BOOL)forceProRes
                        startMS:(int)startMS endMS:(int)endMS
                     completion:(void (^)(BOOL))completion {
    void (^finishNO)(void) = ^{
        if (completion) dispatch_async(dispatch_get_main_queue(), ^{ completion(NO); });
    };
    if (!path || path.length == 0 || !_context) { finishNO(); return; }
    auto* row = _context->GetSequenceElements().GetRowInformation(rowIndex);
    if (!row || !row->element) { finishNO(); return; }
    if (row->element->GetType() == ElementType::ELEMENT_TYPE_TIMING) { finishNO(); return; }
    const std::string modelName = row->element->GetName();
    if (modelName.empty()) { finishNO(); return; }

    Model* model = _context->GetModel(modelName);
    if (!model || model->GetDisplayAs() == DisplayAsType::ModelGroup) { finishNO(); return; }

    RenderEngine* engine = _context->GetRenderEngine();
    if (!engine) { finishNO(); return; }

    SequenceData& fullData = _context->GetSequenceData();
    if (fullData.NumFrames() == 0 || fullData.NumChannels() == 0) { finishNO(); return; }

    // Slice the per-model channel data on the main thread (reads the live
    // _sequenceData) into a private copy; only the encode runs in the
    // background, operating on that copy + immutable model geometry. A
    // shared_ptr keeps the copy alive for the duration of the background block.
    auto exported = engine->ExportModelData(modelName, fullData);
    if (!exported.data) { finishNO(); return; }
    std::shared_ptr<SequenceData> modelData(std::move(exported.data));
    const uint32_t totalFrames = (uint32_t)modelData->NumFrames();
    if (totalFrames == 0) { finishNO(); return; }

    const int frameTime = modelData->FrameTime();
    uint32_t startFrame = 0;
    uint32_t endFrame = totalFrames;
    if (startMS >= 0 && endMS > startMS && frameTime > 0) {
        startFrame = std::min<uint32_t>(totalFrames, (uint32_t)(startMS / frameTime));
        endFrame = std::min<uint32_t>(totalFrames,
                                      (uint32_t)((endMS + frameTime - 1) / frameTime));
    }
    if (endFrame <= startFrame) { finishNO(); return; }

    const int startAddr = model->GetNumberFromChannelString(model->ModelStartChannel);
    const std::string outPath = [path UTF8String];
    const uint32_t sf = startFrame;
    const uint32_t ef = endFrame;
    const bool compressedB = compressed;
    const bool hqB = highQuality;
    const bool prB = forceProRes;

    // Encode off the main thread — AVAssetWriter's Finish blocks on its own
    // lower-QoS writer thread, which is a priority inversion (and a UI hang)
    // when run on the main/user-interactive thread.
    dispatch_async(dispatch_get_global_queue(QOS_CLASS_USER_INITIATED, 0), ^{
        bool ok = ModelVideoExporter::WriteModelVideo(outPath, modelData.get(),
                                                      sf, ef, model, startAddr,
                                                      compressedB, hqB, prB);
        if (completion) dispatch_async(dispatch_get_main_queue(), ^{ completion(ok ? YES : NO); });
    });
}

- (void)exportModelAsVideoAtRow:(int)rowIndex toPath:(NSString*)path
                     compressed:(BOOL)compressed highQuality:(BOOL)highQuality
                    forceProRes:(BOOL)forceProRes
                        startMS:(int)startMS endMS:(int)endMS
                    exportWidth:(int)exportWidth exportHeight:(int)exportHeight
                     completion:(void (^)(BOOL))completion {
    void (^finishNO)(void) = ^{
        if (completion) dispatch_async(dispatch_get_main_queue(), ^{ completion(NO); });
    };
    if (!path || path.length == 0 || !_context) { finishNO(); return; }
    auto* row = _context->GetSequenceElements().GetRowInformation(rowIndex);
    if (!row || !row->element) { finishNO(); return; }
    if (row->element->GetType() == ElementType::ELEMENT_TYPE_TIMING) { finishNO(); return; }
    const std::string modelName = row->element->GetName();
    if (modelName.empty()) { finishNO(); return; }

    Model* model = _context->GetModel(modelName);
    if (!model || model->GetDisplayAs() == DisplayAsType::ModelGroup) { finishNO(); return; }

    RenderEngine* engine = _context->GetRenderEngine();
    if (!engine) { finishNO(); return; }

    SequenceData& fullData = _context->GetSequenceData();
    if (fullData.NumFrames() == 0 || fullData.NumChannels() == 0) { finishNO(); return; }

    auto exported = engine->ExportModelData(modelName, fullData);
    if (!exported.data) { finishNO(); return; }
    std::shared_ptr<SequenceData> modelData(std::move(exported.data));
    const uint32_t totalFrames = (uint32_t)modelData->NumFrames();
    if (totalFrames == 0) { finishNO(); return; }

    const int frameTime = modelData->FrameTime();
    uint32_t startFrame = 0;
    uint32_t endFrame = totalFrames;
    if (startMS >= 0 && endMS > startMS && frameTime > 0) {
        startFrame = std::min<uint32_t>(totalFrames, (uint32_t)(startMS / frameTime));
        endFrame = std::min<uint32_t>(totalFrames,
                                      (uint32_t)((endMS + frameTime - 1) / frameTime));
    }
    if (endFrame <= startFrame) { finishNO(); return; }

    const int startAddr = model->GetNumberFromChannelString(model->ModelStartChannel);
    const std::string outPath = [path UTF8String];
    const uint32_t sf = startFrame;
    const uint32_t ef = endFrame;
    const bool compressedB = compressed;
    const bool hqB = highQuality;
    const bool prB = forceProRes;
    const int expW = exportWidth;
    const int expH = exportHeight;

    dispatch_async(dispatch_get_global_queue(QOS_CLASS_USER_INITIATED, 0), ^{
        bool ok = ModelVideoExporter::WriteModelVideo(outPath, modelData.get(),
                                                      sf, ef, model, startAddr,
                                                      compressedB, hqB, prB,
                                                      expW, expH);
        if (completion) dispatch_async(dispatch_get_main_queue(), ^{ completion(ok ? YES : NO); });
    });
}

- (void)exportHousePreviewVideoToPath:(NSString*)path
                                width:(int)width
                               height:(int)height
                          highQuality:(BOOL)highQuality
                              startMS:(int)startMS
                                endMS:(int)endMS
                             progress:(void (^)(double))progress
                           completion:(void (^)(BOOL))completion {
    void (^finishNO)(void) = ^{
        if (completion) dispatch_async(dispatch_get_main_queue(), ^{ completion(NO); });
    };
    if (!path || path.length == 0 || width <= 0 || height <= 0 || !_context) {
        finishNO();
        return;
    }

    SequenceData& seqData = _context->GetSequenceData();
    const uint32_t totalFrames = (uint32_t)seqData.NumFrames();
    if (totalFrames == 0 || seqData.NumChannels() == 0) {
        finishNO();
        return;
    }
    const int frameTime = seqData.FrameTime();
    uint32_t startFrame = 0;
    uint32_t endFrame = totalFrames;
    if (startMS >= 0 && endMS > startMS && frameTime > 0) {
        startFrame = std::min<uint32_t>(totalFrames, (uint32_t)(startMS / frameTime));
        endFrame = std::min<uint32_t>(totalFrames,
                                      (uint32_t)((endMS + frameTime - 1) / frameTime));
    }
    if (endFrame <= startFrame) {
        finishNO();
        return;
    }

    const BOOL is3d = _context->GetLayoutMode3D() ? YES : NO;
    iPadRenderContext* rcPtr = _context.get();
    void* rc = rcPtr;
    NSString* outPath = [path copy];
    const int w = width;
    const int h = height;
    const int sf = (int)startFrame;
    const int ef = (int)endFrame;

    // Stop the live on-screen preview from drawing while we render frames on
    // the background thread (both mutate per-model node colours). Set on the
    // main thread before dispatch so no live draw sneaks in, cleared in the
    // completion. Encoding the full show at full resolution is slow, hence the
    // background queue.
    rcPtr->SetExportInProgress(true);
    dispatch_async(dispatch_get_global_queue(QOS_CLASS_USER_INITIATED, 0), ^{
        BOOL ok = [XLHousePreviewVideoExporter exportToPath:outPath
                                              renderContext:rc
                                                      width:w
                                                     height:h
                                                       is3d:is3d
                                                highQuality:highQuality
                                                 startFrame:sf
                                                   endFrame:ef
                                                   progress:^(double f) {
            if (progress) dispatch_async(dispatch_get_main_queue(), ^{ progress(f); });
        }];
        dispatch_async(dispatch_get_main_queue(), ^{
            rcPtr->SetExportInProgress(false);
            if (completion) completion(ok);
        });
    });
}

- (int)layoutPreviewWidth {
    return _context ? _context->GetPreviewWidth() : 0;
}

- (int)layoutPreviewHeight {
    return _context ? _context->GetPreviewHeight() : 0;
}

- (BOOL)writeFseqToPath:(NSString*)path {
    if (!_context || !path || path.length == 0) return NO;
    if (!_context->IsSequenceLoaded()) return NO;
    return _context->WriteFseq(std::string([path UTF8String])) ? YES : NO;
}

- (BOOL)tryLoadFseqFromPath:(NSString*)fseqPath xsqPath:(NSString*)xsqPath {
    if (!_context || !fseqPath || fseqPath.length == 0) return NO;
    if (!_context->IsSequenceLoaded()) return NO;
    const std::string fseq([fseqPath UTF8String]);
    const std::string xsq = (xsqPath && xsqPath.length > 0)
        ? std::string([xsqPath UTF8String])
        : std::string();
    return _context->TryLoadFseq(fseq, xsq) ? YES : NO;
}

- (BOOL)removeWordsAndPhonemesAtRow:(int)rowIndex {
    auto& se = _context->GetSequenceElements();
    auto* row = se.GetRowInformation(rowIndex);
    if (!row || !row->element) return NO;
    if (row->element->GetType() != ElementType::ELEMENT_TYPE_TIMING) return NO;
    TimingElement* te = dynamic_cast<TimingElement*>(row->element);
    if (!te) return NO;
    if (te->GetEffectLayerCount() <= 1) return NO;
    // Lock guard — same rule as BreakdownPhrases.
    for (int k = (int)te->GetEffectLayerCount() - 1; k > 0; --k) {
        EffectLayer* ck = te->GetEffectLayer(k);
        if (!ck) continue;
        for (auto&& eff : ck->GetAllEffects()) {
            if (eff && eff->IsLocked()) return NO;
        }
    }
    while (te->GetEffectLayerCount() > 1) {
        te->RemoveEffectLayer((int)te->GetEffectLayerCount() - 1);
    }
    se.PopulateRowInformation();
    return YES;
}

- (BOOL)removePhonemesAtRow:(int)rowIndex {
    auto& se = _context->GetSequenceElements();
    auto* row = se.GetRowInformation(rowIndex);
    if (!row || !row->element) return NO;
    if (row->element->GetType() != ElementType::ELEMENT_TYPE_TIMING) return NO;
    TimingElement* te = dynamic_cast<TimingElement*>(row->element);
    if (!te) return NO;
    // Need a phoneme layer (index 2) to strip; keep phrase (0) + words (1).
    if (te->GetEffectLayerCount() <= 2) return NO;
    // Lock guard — refuse if any phoneme mark (layer 2+) is locked.
    for (int k = (int)te->GetEffectLayerCount() - 1; k > 1; --k) {
        EffectLayer* ck = te->GetEffectLayer(k);
        if (!ck) continue;
        for (auto&& eff : ck->GetAllEffects()) {
            if (eff && eff->IsLocked()) return NO;
        }
    }
    while (te->GetEffectLayerCount() > 2) {
        te->RemoveEffectLayer((int)te->GetEffectLayerCount() - 1);
    }
    se.PopulateRowInformation();
    return YES;
}

- (BOOL)rowIsSubmodelAtIndex:(int)rowIndex {
    auto* row = _context->GetSequenceElements().GetRowInformation(rowIndex);
    return (row && row->submodel) ? YES : NO;
}
- (int)rowNestDepthAtIndex:(int)rowIndex {
    auto* row = _context->GetSequenceElements().GetRowInformation(rowIndex);
    return row ? row->nestDepth : 0;
}
- (int)rowStrandIndexAtIndex:(int)rowIndex {
    auto* row = _context->GetSequenceElements().GetRowInformation(rowIndex);
    return row ? row->strandIndex : -1;
}
- (int)rowNodeIndexAtIndex:(int)rowIndex {
    auto* row = _context->GetSequenceElements().GetRowInformation(rowIndex);
    return row ? row->nodeIndex : -1;
}

- (BOOL)rowHasSubmodelsAtIndex:(int)rowIndex {
    auto* row = _context->GetSequenceElements().GetRowInformation(rowIndex);
    if (!row || !row->element) return NO;
    // Only top-level model rows have a strand/submodel container; sub-
    // layer rows (layerIndex > 0) forward to the same ModelElement so
    // the answer is identical, but the disclosure affordance only
    // makes sense on the element's primary row.
    auto* me = dynamic_cast<ModelElement*>(row->element);
    if (!me) return NO;
    if (me->GetSubModelAndStrandCount() > 0) return YES;
    // ModelGroups disclose their member models via ShowStrands too.
    Model* m = _context->GetModelManager()[me->GetModelName()];
    if (m && m->GetDisplayAs() == DisplayAsType::ModelGroup) return YES;
    return NO;
}
- (BOOL)rowShowsSubmodelsAtIndex:(int)rowIndex {
    auto* row = _context->GetSequenceElements().GetRowInformation(rowIndex);
    if (!row || !row->element) return NO;
    auto* me = dynamic_cast<ModelElement*>(row->element);
    return (me && me->ShowStrands()) ? YES : NO;
}
- (void)toggleRowShowSubmodelsAtIndex:(int)rowIndex {
    auto* row = _context->GetSequenceElements().GetRowInformation(rowIndex);
    if (!row || !row->element) return;
    auto* me = dynamic_cast<ModelElement*>(row->element);
    if (!me) return;
    me->ShowStrands(!me->ShowStrands());
    _context->GetSequenceElements().PopulateRowInformation();
}

- (BOOL)rowHasNodesAtIndex:(int)rowIndex {
    auto* row = _context->GetSequenceElements().GetRowInformation(rowIndex);
    if (!row || !row->element) return NO;
    auto* se = dynamic_cast<StrandElement*>(row->element);
    return (se && se->GetNodeLayerCount() > 0) ? YES : NO;
}
- (BOOL)rowShowsNodesAtIndex:(int)rowIndex {
    auto* row = _context->GetSequenceElements().GetRowInformation(rowIndex);
    if (!row || !row->element) return NO;
    auto* se = dynamic_cast<StrandElement*>(row->element);
    return (se && se->ShowNodes()) ? YES : NO;
}
- (void)toggleRowShowNodesAtIndex:(int)rowIndex {
    auto* row = _context->GetSequenceElements().GetRowInformation(rowIndex);
    if (!row || !row->element) return;
    auto* se = dynamic_cast<StrandElement*>(row->element);
    if (!se) return;
    se->ShowNodes(!se->ShowNodes());
    _context->GetSequenceElements().PopulateRowInformation();
}

- (BOOL)insertEffectLayerAboveAtIndex:(int)rowIndex {
    auto* row = _context->GetSequenceElements().GetRowInformation(rowIndex);
    if (!row || !row->element) return NO;
    row->element->InsertEffectLayer(row->layerIndex);
    _context->GetSequenceElements().PopulateRowInformation();
    return YES;
}
- (BOOL)insertEffectLayerBelowAtIndex:(int)rowIndex {
    auto* row = _context->GetSequenceElements().GetRowInformation(rowIndex);
    if (!row || !row->element) return NO;
    int insertAt = row->layerIndex + 1;
    if (insertAt >= (int)row->element->GetEffectLayerCount()) {
        row->element->AddEffectLayer();
    } else {
        row->element->InsertEffectLayer(insertAt);
    }
    _context->GetSequenceElements().PopulateRowInformation();
    return YES;
}
// B47: insert `count` new empty layers immediately below the
// row's own layerIndex. Idempotent for non-positive counts.
// Single `PopulateRowInformation` at the end keeps UI refresh
// cheap. Returns the number actually added.
- (int)insertEffectLayersBelowAtIndex:(int)rowIndex count:(int)count {
    if (count <= 0) return 0;
    auto* row = _context->GetSequenceElements().GetRowInformation(rowIndex);
    if (!row || !row->element) return 0;
    Element* elem = row->element;
    for (int i = 0; i < count; ++i) {
        elem->InsertEffectLayer(row->layerIndex + 1 + i);
    }
    _context->GetSequenceElements().PopulateRowInformation();
    return count;
}

- (BOOL)removeEffectLayerAtIndex:(int)rowIndex {
    auto* row = _context->GetSequenceElements().GetRowInformation(rowIndex);
    if (!row || !row->element) return NO;
    if (row->element->GetEffectLayerCount() <= 1) return NO;
    // Abort render before yanking a layer that may own effects the
    // renderer is walking; matches desktop's `AbortRender()` guard
    // before `RemoveEffectLayer` in `RowHeading.cpp`.
    _context->AbortRender(5000);
    row->element->RemoveEffectLayer(row->layerIndex);
    _context->GetSequenceElements().PopulateRowInformation();
    return YES;
}

// #6507 parity: drag-reorder a top-level row. Mirrors the desktop
// `RowHeading::mouseLeftUp` path — resolve the visible source/dest
// rows to their owning elements' MASTER_VIEW indices, then call the
// shared `MoveSequenceElement`. `destBeforeRowIndex == visibleRowCount`
// (or any out-of-range visible index) drops the element at the end.
- (BOOL)moveTopLevelRowFrom:(int)srcRowIndex toBefore:(int)destBeforeRowIndex {
    auto& se = _context->GetSequenceElements();
    auto isTopLevel = [](Row_Information_Struct* ri) {
        return ri && ri->element && ri->layerIndex == 0
            && ri->strandIndex < 0 && ri->nodeIndex < 0 && !ri->submodel;
    };
    auto* srcRow = se.GetVisibleRowInformation(srcRowIndex);
    if (!isTopLevel(srcRow)) return NO;

    int view = se.GetCurrentView();
    int srcElemIdx = se.GetElementIndex(srcRow->element->GetFullName(), view);
    if (srcElemIdx < 0) return NO;

    int destElemIdx = (int)se.GetElementCount(view);
    auto* destRow = se.GetVisibleRowInformation(destBeforeRowIndex);
    if (isTopLevel(destRow)) {
        int idx = se.GetElementIndex(destRow->element->GetFullName(), view);
        if (idx >= 0) destElemIdx = idx;
    }

    // Desktop guards against the two no-op landings (drop on self, or
    // immediately after self — which leaves order unchanged).
    if (destElemIdx == srcElemIdx || destElemIdx == srcElemIdx + 1) return NO;

    se.MoveSequenceElement(srcElemIdx, destElemIdx, view);
    se.PopulateRowInformation();
    return YES;
}

- (BOOL)moveTopLevelElementNamed:(NSString*)modelName
                     beforeNamed:(NSString*)beforeModelName {
    auto& se = _context->GetSequenceElements();
    int view = se.GetCurrentView();
    int srcElemIdx = se.GetElementIndex([modelName UTF8String], view);
    if (srcElemIdx < 0) return NO;

    int destElemIdx = (int)se.GetElementCount(view);
    if (beforeModelName.length > 0) {
        int idx = se.GetElementIndex([beforeModelName UTF8String], view);
        if (idx >= 0) destElemIdx = idx;
    }
    if (destElemIdx == srcElemIdx || destElemIdx == srcElemIdx + 1) return NO;

    se.MoveSequenceElement(srcElemIdx, destElemIdx, view);
    se.PopulateRowInformation();
    return YES;
}

- (NSString*)topLevelElementNameAfter:(NSString*)modelName {
    auto& se = _context->GetSequenceElements();
    int view = se.GetCurrentView();
    int idx = se.GetElementIndex([modelName UTF8String], view);
    if (idx < 0 || idx + 1 >= (int)se.GetElementCount(view)) return nil;
    Element* next = se.GetElement(idx + 1, view);
    if (!next) return nil;
    return [NSString stringWithUTF8String:next->GetFullName().c_str()];
}

// B56: walk every strand of a model and promote node-level effects
// up to strand level when every node carries an identical "On" or
// "Color Wash" effect at the same time range. After the per-strand
// pass, run the same coalescing one level higher (strands → model).
// Mirrors `xLightsFrame::DoPromoteEffects` (tabSequencer.cpp:4238).
// Pure structural walk — no SequenceData or rendering — so the
// algorithm can live in the bridge without core changes.
- (int)promoteNodeEffectsOnRow:(int)rowIndex {
    auto& se = _context->GetSequenceElements();
    auto* row = se.GetRowInformation(rowIndex);
    if (!row || !row->element) return 0;
    if (row->element->GetType() != ElementType::ELEMENT_TYPE_MODEL) return 0;
    ModelElement* element = dynamic_cast<ModelElement*>(row->element);
    if (!element) return 0;

    auto effectMatches = [](Effect* a, Effect* b) {
        if (!a || !b) return false;
        if (a->GetEffectIndex() != b->GetEffectIndex()) return false;
        if (a->GetStartTimeMS() != b->GetStartTimeMS()) return false;
        if (a->GetEndTimeMS() != b->GetEndTimeMS()) return false;
        return a->GetSettingsAsString() == b->GetSettingsAsString()
            && a->GetPaletteAsString() == b->GetPaletteAsString();
    };

    se.get_undo_mgr().CreateUndoStep();
    _context->AbortRender(5000);
    int promoted = 0;

    // Step 1: nodes → strands. For each strand, look at node 0's effects
    // and try to collapse identical coverage across every other node
    // layer up to the strand layer.
    for (int x = 0; x < element->GetStrandCount(); ++x) {
        StrandElement* strand = element->GetStrand(x);
        if (!strand) continue;
        EffectLayer* target = strand->GetEffectLayer(0);
        // Single-strand models promote straight to model level.
        if (element->GetStrandCount() <= 1) {
            if (element->GetEffectLayer(0)->GetEffectCount() != 0) {
                element->InsertEffectLayer(0);
            }
            target = element->GetEffectLayer(0);
        }
        if (strand->GetNodeLayerCount() == 0) continue;
        NodeLayer* base = strand->GetNodeLayer(0);
        for (int e = base->GetEffectCount() - 1; e >= 0; --e) {
            Effect* eff = base->GetEffect(e);
            if (!eff) continue;
            if (target->HasEffectsInTimeRange(eff->GetStartTimeMS(),
                                                eff->GetEndTimeMS())) continue;
            const std::string& name = eff->GetEffectName();
            if (name != "On" && name != "Color Wash") continue;
            int mp = (eff->GetStartTimeMS() + eff->GetEndTimeMS()) / 2;
            bool collapse = true;
            for (int n = 1; n < strand->GetNodeLayerCount() && collapse; ++n) {
                NodeLayer* node = strand->GetNodeLayer(n);
                int nodeIndex = 0;
                if (!node || !node->HitTestEffectByTime(mp, nodeIndex)
                    || !effectMatches(eff, node->GetEffect(nodeIndex))) {
                    collapse = false;
                }
            }
            if (collapse) {
                target->AddEffect(0, eff->GetEffectName(),
                                   eff->GetSettingsAsString(),
                                   eff->GetPaletteAsString(),
                                   eff->GetStartTimeMS(),
                                   eff->GetEndTimeMS(), false, false);
                ++promoted;
                for (int n = 0; n < strand->GetNodeLayerCount(); ++n) {
                    NodeLayer* node = strand->GetNodeLayer(n);
                    int nodeIndex = 0;
                    if (node && node->HitTestEffectByTime(mp, nodeIndex)) {
                        node->DeleteEffectByIndex(nodeIndex);
                    }
                }
            }
        }
    }

    // Step 2: strands → model (only meaningful when there's > 1 strand).
    if (element->GetStrandCount() > 1) {
        EffectLayer* base = element->GetStrand(0)->GetEffectLayer(0);
        if (element->GetEffectLayer(0)->GetEffectCount() != 0) {
            element->InsertEffectLayer(0);
        }
        EffectLayer* target = element->GetEffectLayer(0);
        for (int e = base->GetEffectCount() - 1; e >= 0; --e) {
            Effect* eff = base->GetEffect(e);
            if (!eff) continue;
            const std::string& name = eff->GetEffectName();
            if (name != "On" && name != "Color Wash") continue;
            int mp = (eff->GetStartTimeMS() + eff->GetEndTimeMS()) / 2;
            bool collapse = true;
            for (int n = 0; n < element->GetStrandCount() && collapse; ++n) {
                StrandElement* strand = element->GetStrand(n);
                if (!strand) { collapse = false; break; }
                for (int l = 0; l < (int)strand->GetEffectLayerCount() && collapse; ++l) {
                    EffectLayer* layer = strand->GetEffectLayer(l);
                    if (layer == base) continue;
                    int nodeIndex = 0;
                    if (!layer || !layer->HitTestEffectByTime(mp, nodeIndex)
                        || !effectMatches(eff, layer->GetEffect(nodeIndex))) {
                        collapse = false;
                    }
                }
            }
            if (collapse) {
                target->AddEffect(0, eff->GetEffectName(),
                                   eff->GetSettingsAsString(),
                                   eff->GetPaletteAsString(),
                                   eff->GetStartTimeMS(),
                                   eff->GetEndTimeMS(), false, false);
                ++promoted;
                for (int n = 0; n < element->GetStrandCount(); ++n) {
                    StrandElement* strand = element->GetStrand(n);
                    if (!strand) continue;
                    for (int l = 0; l < (int)strand->GetEffectLayerCount(); ++l) {
                        EffectLayer* layer = strand->GetEffectLayer(l);
                        int nodeIndex = 0;
                        if (layer && layer->HitTestEffectByTime(mp, nodeIndex)) {
                            layer->DeleteEffectByIndex(nodeIndex);
                        }
                    }
                }
            }
        }
    }

    if (promoted > 0) {
        se.PopulateRowInformation();
        _context->RenderEffectForModel(element->GetModelName(), 0, 99999999, true);
    }
    return promoted;
}

// Line-fit helpers for ramp detection — direct ports of the desktop
// tabSequencer.cpp isOnLine / isOnLineColor / RampLenColor.
static bool ConvertIsOnLine(int x1, int y1, int x2, int y2, int x3, int y3) {
    double diffx = x2 - x1;
    double diffy = y2 - y1;
    double b = y1 - diffy / diffx * x1;
    double ye1 = diffy / diffx * x3 + b;
    return (y3 + 1) >= ye1 && (y3 - 1) <= ye1;
}

static bool ConvertIsOnLineColor(const xlColor& v1, const xlColor& v2, const xlColor& v3,
                                 int x, int x2, int x3) {
    return ConvertIsOnLine(x, v1.Red(), x2, v2.Red(), x3, v3.Red())
        && ConvertIsOnLine(x, v1.Green(), x2, v2.Green(), x3, v3.Green())
        && ConvertIsOnLine(x, v1.Blue(), x2, v2.Blue(), x3, v3.Blue());
}

static int ConvertRampLenColor(int start, std::vector<xlColor>& colors) {
    int s = start + 2;
    for (; s < (int)colors.size(); s++) {
        if (!ConvertIsOnLineColor(colors[start], colors[s - 1], colors[s], start, s - 1, s)) {
            return s - start;
        }
    }
    if (s == (int)colors.size()) {
        return s - start;
    }
    return 0;
}

// Direct port of xLightsFrame::DoConvertDataRowToEffects. Returns the
// number of effects added so the caller can total them across layers.
static int ConvertDataRowToEffects(EffectLayer* layer, xlColorVector& colors, int frameTime, bool eraseExisting) {
    if (eraseExisting)
        layer->DeleteAllEffects();

    colors.push_back(xlBLACK);
    int startTime = 0;
    xlColor lastColor(xlBLACK);
    int added = 0;

    for (size_t x = 0; x + 3 < colors.size(); ++x) {
        if (colors[x] != colors[x + 1]) {
            int len = ConvertRampLenColor((int)x, colors);
            if (len >= 3) {
                HSVValue v1 = colors[x].asHSV();
                HSVValue v2 = colors[x + len - 1].asHSV();

                int stime = (int)x * frameTime;
                int etime = (int)(x + len) * frameTime;
                if (colors[x] == xlBLACK || colors[x + len - 1] == xlBLACK || (v1.hue == v2.hue)) {
                    HSVValue c = colors[x].asHSV();
                    if (colors[x] == xlBLACK) {
                        c = colors[x + len - 1].asHSV();
                    }
                    c.value = 1.0;
                    xlColor c2(c);

                    int i = v1.value * 100.0;
                    int i2 = v2.value * 100.0;
                    std::string settings = "E_TEXTCTRL_Eff_On_Start=" + std::to_string(i) + ",E_TEXTCTRL_Eff_On_End=" + std::to_string(i2);
                    std::string palette = "C_BUTTON_Palette1=" + (std::string)c2 + ",C_CHECKBOX_Palette1=1";
                    if (!layer->HasEffectsInTimeRange(stime, etime)) {
                        layer->AddEffect(0, "On", settings, palette, stime, etime, false, false);
                        ++added;
                    }
                } else {
                    std::string palette = "C_BUTTON_Palette1=" + (std::string)colors[x] + ",C_CHECKBOX_Palette1=1,"
                                          "C_BUTTON_Palette2=" + (std::string)colors[x + len - 1] + ",C_CHECKBOX_Palette2=1";
                    if (!layer->HasEffectsInTimeRange(stime, etime)) {
                        layer->AddEffect(0, "Color Wash", "", palette, stime, etime, false, false);
                        ++added;
                    }
                }
                for (int z = 0; z < len; ++z) {
                    colors[x + z] = xlBLACK;
                }
            }
        }
    }

    for (size_t x = 0; x < colors.size(); ++x) {
        if (lastColor != colors[x]) {
            int time = (int)x * frameTime;
            if (lastColor != xlBLACK) {
                std::string palette = "C_BUTTON_Palette1=" + (std::string)lastColor + ",C_CHECKBOX_Palette1=1";
                if (time != startTime) {
                    if (!layer->HasEffectsInTimeRange(startTime, time)) {
                        layer->AddEffect(0, "On", "", palette, startTime, time, false, false);
                        ++added;
                    }
                }
            }
            startTime = time;
            lastColor = colors[x];
        }
    }
    return added;
}

- (int)convertDataToEffectsOnRow:(int)rowIndex {
    auto& se = _context->GetSequenceElements();
    auto* row = se.GetRowInformation(rowIndex);
    if (!row || !row->element) return 0;
    Element* e = row->element;

    std::string modelName = e->GetModelName();
    if (modelName.empty()) return 0;
    Model* model = _context->GetModel(modelName);
    if (!model) return 0;
    // Desktop refuses ModelGroup / SubModel — there is no single buffer
    // of rendered node data to read back.
    if (model->GetDisplayAs() == DisplayAsType::ModelGroup || model->GetDisplayAs() == DisplayAsType::SubModel) {
        return 0;
    }

    SequenceData& seqData = _context->GetSequenceData();
    if (seqData.NumFrames() == 0 || seqData.NumChannels() == 0) return 0;

    // Render the model over the whole sequence and wait so the node
    // colours we read below reflect the current effects.
    _context->RenderModelAndWait(modelName);
    const int frameTime = seqData.FrameTime();
    const size_t numFrames = seqData.NumFrames();

    se.get_undo_mgr().CreateUndoStep();
    _context->AbortRender(5000);

    int total = 0;
    auto convertLayer = [&](EffectLayer* layer, int strand, int node) {
        if (!layer) return;
        xlColorVector colors;
        colors.reserve(numFrames);
        SingleLineModel ssModel(model->GetModelManager());
        ssModel.Reset(1, *model, strand, node);
        const int32_t startChan = ssModel.NodeStartChannel(0);
        if (startChan < 0 || (size_t)startChan >= seqData.NumChannels()) return;
        for (size_t f = 0; f < numFrames; ++f) {
            ssModel.SetNodeChannelValues(0, &seqData[f][startChan]);
            colors.push_back(ssModel.GetNodeColor(0));
        }
        total += ConvertDataRowToEffects(layer, colors, frameTime, false);
    };

    if (e->GetType() == ElementType::ELEMENT_TYPE_MODEL) {
        ModelElement* el = dynamic_cast<ModelElement*>(e);
        if (!el) return 0;
        for (int i = 0; i < el->GetStrandCount(); ++i) {
            StrandElement* sse = el->GetStrand(i);
            if (!sse) continue;
            for (int j = 0; j < sse->GetNodeLayerCount(); ++j) {
                convertLayer(sse->GetNodeLayer(j), i, j);
            }
        }
    } else if (e->GetType() == ElementType::ELEMENT_TYPE_STRAND) {
        StrandElement* el = dynamic_cast<StrandElement*>(e);
        if (!el) return 0;
        const int strand = el->GetStrand();
        if (row->nodeIndex >= 0) {
            convertLayer(el->GetNodeLayer(row->nodeIndex), strand, row->nodeIndex);
        } else {
            for (int j = 0; j < el->GetNodeLayerCount(); ++j) {
                convertLayer(el->GetNodeLayer(j), strand, j);
            }
        }
    }

    if (total > 0) {
        _context->RenderEffectForModel(modelName, 0, 99999999, true);
    }
    return total;
}

- (int)convertEffectsToPerModelOnRow:(int)rowIndex acrossAllLayers:(BOOL)allLayers {
    auto& se = _context->GetSequenceElements();
    auto* row = se.GetRowInformation(rowIndex);
    if (!row || !row->element) return 0;
    Element* elem = row->element;

    // Walk the relevant layer(s), counting effects whose buffer
    // style ConvertEffectsToPerModel will rewrite. The actual
    // conversion captures undo per modified effect, so the count
    // is purely informational for the UI.
    auto eligibleCount = [](EffectLayer* layer) {
        if (!layer) return 0;
        int n = 0;
        for (int i = 0; i < layer->GetEffectCount(); ++i) {
            Effect* e = layer->GetEffect(i);
            if (!e) continue;
            const auto& bs = e->GetSettings()["B_CHOICE_BufferStyle"];
            if (bs.empty() || bs == "Per Preview" || bs == "Default" || bs == "Single Line") {
                ++n;
            }
        }
        return n;
    };

    se.get_undo_mgr().CreateUndoStep();
    _context->AbortRender(5000);
    int converted = 0;
    if (allLayers) {
        for (int i = 0; i < (int)elem->GetEffectLayerCount(); ++i) {
            EffectLayer* layer = elem->GetEffectLayer(i);
            converted += eligibleCount(layer);
            if (layer) layer->ConvertEffectsToPerModel(se.get_undo_mgr());
        }
    } else {
        if (row->layerIndex >= 0 && row->layerIndex < (int)elem->GetEffectLayerCount()) {
            EffectLayer* layer = elem->GetEffectLayer(row->layerIndex);
            converted = eligibleCount(layer);
            if (layer) layer->ConvertEffectsToPerModel(se.get_undo_mgr());
        }
    }
    if (converted > 0) {
        // Force a re-render of the affected model so the new buffer
        // style takes effect immediately.
        _context->RenderEffectForModel(elem->GetModelName(), 0, 99999999, true);
    }
    return converted;
}

- (int)unusedLayerCountAtRow:(int)rowIndex {
    auto* row = _context->GetSequenceElements().GetRowInformation(rowIndex);
    if (!row || !row->element) return 0;
    Element* elem = row->element;
    int total = (int)elem->GetEffectLayerCount();
    if (total <= 1) return 0;
    int empty = 0;
    for (int i = 0; i < total; ++i) {
        EffectLayer* layer = elem->GetEffectLayer(i);
        if (layer && layer->GetEffectCount() == 0) ++empty;
    }
    // One layer must survive; the delete op never removes the last
    // one. Cap the reportable count accordingly so the menu label
    // matches what the delete will actually do.
    return std::min(empty, total - 1);
}

// B48: delete every layer on the row's element that has zero
// effects. Preserves at least one layer (desktop invariant — an
// element always has ≥ 1 layer). Returns the count of layers
// removed. Walks the layer list from the top so removals don't
// invalidate the remaining indices.
- (int)deleteUnusedLayersOnElementAtRow:(int)rowIndex {
    auto* row = _context->GetSequenceElements().GetRowInformation(rowIndex);
    if (!row || !row->element) return 0;
    Element* elem = row->element;
    int total = (int)elem->GetEffectLayerCount();
    if (total <= 1) return 0;
    _context->AbortRender(5000);
    int removed = 0;
    for (int i = total - 1; i >= 0 && elem->GetEffectLayerCount() > 1; --i) {
        EffectLayer* layer = elem->GetEffectLayer(i);
        if (layer && layer->GetEffectCount() == 0) {
            elem->RemoveEffectLayer(i);
            ++removed;
        }
    }
    if (removed > 0) {
        _context->GetSequenceElements().PopulateRowInformation();
    }
    return removed;
}

- (BOOL)renameTimingTrackAtIndex:(int)rowIndex newName:(NSString*)newName {
    if (!newName || newName.length == 0) return NO;
    auto* row = _context->GetSequenceElements().GetRowInformation(rowIndex);
    if (!row || !row->element) return NO;
    if (row->element->GetType() != ElementType::ELEMENT_TYPE_TIMING) return NO;
    std::string newStr([newName UTF8String]);
    auto& se = _context->GetSequenceElements();
    if (se.ElementExists(newStr)) return NO;
    std::string oldStr = row->element->GetName();
    if (oldStr == newStr) return YES;
    se.RenameTimingTrack(oldStr, newStr);
    row->element->SetName(newStr);
    se.PopulateRowInformation();
    return YES;
}

- (BOOL)deleteTimingTrackAtIndex:(int)rowIndex {
    auto* row = _context->GetSequenceElements().GetRowInformation(rowIndex);
    if (!row || !row->element) return NO;
    if (row->element->GetType() != ElementType::ELEMENT_TYPE_TIMING) return NO;
    std::string name = row->element->GetName();
    // Some effect settings reference the timing track by name; abort
    // in-flight render before the element disappears.
    _context->AbortRender(5000);
    _context->GetSequenceElements().DeleteElement(name);
    return YES;
}

- (BOOL)addTimingTrackNamed:(NSString*)name {
    if (!name || name.length == 0) return NO;
    std::string n([name UTF8String]);
    TimingElement* e = _context->AddTimingElement(n, "");
    return e != nullptr;
}

- (NSString*)addFixedIntervalTimingTrackNamed:(NSString*)name
                                    intervalMS:(int)intervalMS {
    if (!_context || !_context->IsSequenceLoaded()) return @"";
    SequenceFile* sf = _context->GetSequenceFile();
    if (!sf) return @"";
    std::string n = name ? std::string([name UTF8String]) : std::string();
    if (n.empty()) n = "Timing";

    if (intervalMS > 0) {
        sf->AddFixedTimingSection(n, intervalMS, _context.get());
    } else {
        // Empty timing track — same path the existing
        // -addTimingTrackNamed: takes, but routed through
        // SequenceFile so the fixed/non-fixed bookkeeping stays
        // consistent.
        _context->AddTimingElement(n, "");
    }
    _context->GetSequenceElements().IncrementChangeCount(nullptr);
    return [NSString stringWithUTF8String:n.c_str()];
}

- (NSString*)addMetronomeTimingTrackNamed:(NSString*)name
                                intervalMS:(int)intervalMS
                                      tags:(NSArray<NSString*>*)tags
                             minIntervalMS:(int)minIntervalMS
                                 randomize:(BOOL)randomize {
    if (!_context || !_context->IsSequenceLoaded()) return @"";
    SequenceFile* sf = _context->GetSequenceFile();
    if (!sf || intervalMS <= 0) return @"";
    std::string n = name ? std::string([name UTF8String]) : std::string();
    if (n.empty()) n = "Metronome";

    std::vector<std::string> tagVec;
    if (tags) {
        tagVec.reserve(tags.count);
        for (NSString* t in tags) {
            tagVec.emplace_back([t UTF8String]);
        }
    }
    sf->AddMetronomeLabelTimingSection(n, intervalMS, tagVec, _context.get(),
                                        minIntervalMS, randomize ? true : false);
    _context->GetSequenceElements().IncrementChangeCount(nullptr);
    return [NSString stringWithUTF8String:n.c_str()];
}

- (NSString*)addFPPTimingTrackNamed:(NSString*)name
                              subType:(NSString*)subType {
    if (!_context || !_context->IsSequenceLoaded()) return @"";
    SequenceFile* sf = _context->GetSequenceFile();
    if (!sf) return @"";
    std::string n = name ? std::string([name UTF8String]) : std::string();
    if (n.empty()) return @"";
    std::string st = subType ? std::string([subType UTF8String]) : std::string();
    if (st != "FPP Commands" && st != "FPP Effects") return @"";

    sf->AddNewTimingSection(n, _context.get(), st);
    _context->GetSequenceElements().IncrementChangeCount(nullptr);
    return [NSString stringWithUTF8String:n.c_str()];
}

- (NSString*)addLyricTimingTrackNamed:(NSString*)name
                                 words:(NSArray<NSString*>*)words
                              startMS:(NSArray<NSNumber*>*)startMS
                                endMS:(NSArray<NSNumber*>*)endMS {
    if (!_context || !_context->IsSequenceLoaded()) return @"";
    if (words == nil || words.count == 0) return @"";
    if (startMS == nil || endMS == nil) return @"";
    if (words.count != startMS.count || words.count != endMS.count) return @"";

    std::string trackName = (name && name.length > 0)
        ? std::string([name UTF8String])
        : std::string("AutoGen");
    TimingElement* element = _context->AddTimingElement(trackName, "");
    if (!element) return @"";

    EffectLayer* layer = element->GetEffectLayer(0);
    if (!layer) return @"";

    auto& se = _context->GetSequenceElements();
    double freq = se.GetFrequency();
    if (freq <= 0) freq = 50.0; // safe default

    auto roundPair = [&](int sms, int ems) {
        int s = RoundToMultipleOfPeriod(sms, freq);
        int e = RoundToMultipleOfPeriod(ems, freq);
        if (s == e) {
            e = RoundToMultipleOfPeriod((int)(s + 1000.0 / freq), freq);
        }
        return std::make_pair(s, e);
    };

    int previousEnd = -1;
    for (NSUInteger i = 0; i < words.count; ++i) {
        NSString* w = words[i];
        if (w == nil || w.length == 0) continue;

        std::string raw([w UTF8String]);
        // Match desktop GenerateAILyrics: trim whitespace, skip
        // entries that contain nothing but whitespace.
        std::string trimmed = Trim(raw);
        if (trimmed.empty()) continue;
        bool hasText = std::any_of(trimmed.begin(), trimmed.end(),
                                    [](unsigned char ch) { return !std::isspace(ch); });
        if (!hasText) continue;

        auto [s, e] = roundPair([startMS[i] intValue], [endMS[i] intValue]);
        if (e <= s) continue;
        // Avoid overlapping the previous mark — the recognizer
        // sometimes produces back-to-back segments whose rounded
        // boundaries collide. Bias the new mark forward so they sit
        // adjacent rather than overlapping.
        if (s < previousEnd) {
            s = previousEnd;
            if (e <= s) continue;
        }
        layer->AddEffect(0, trimmed, "", "", s, e, EFFECT_NOT_SELECTED, false);
        previousEnd = e;
    }

    se.IncrementChangeCount(nullptr);
    return [NSString stringWithUTF8String:trackName.c_str()];
}

- (BOOL)hasVocalsStems {
    if (!_context) return NO;
    AudioManager* am = _context->GetCurrentMediaManager();
    if (!am) return NO;
    return am->HasStemData() ? YES : NO;
}

- (NSString*)importNotesFromPath:(NSString*)path
                          format:(NSString*)format
                            name:(NSString*)name
                           track:(NSString*)track
                  speedAdjustPct:(int)speedAdjustPct
                   startAdjustMS:(int)startAdjustMS {
    if (!_context || !_context->IsSequenceLoaded()) return @"";
    if (!path || path.length == 0 || !format) return @"";

    std::string file([path UTF8String]);
    std::string fmt([format UTF8String]);
    std::string trk = track ? std::string([track UTF8String]) : std::string("All");

    auto* sf = _context->GetSequenceFile();
    int interval = sf ? sf->GetFrameMS() : 50;
    if (interval <= 0) interval = 50;
    int durationMS = sf ? sf->GetSequenceDurationMS() : 0;

    std::map<int, std::vector<float>> notes;
    if (fmt == "audacity") {
        notes = NoteImporter::LoadAudacityFile(file, interval);
    } else if (fmt == "musicxml") {
        notes = NoteImporter::LoadMusicXMLFile(file, interval, speedAdjustPct, startAdjustMS, trk);
    } else if (fmt == "midi") {
        notes = NoteImporter::LoadMIDIFile(file, interval, speedAdjustPct, startAdjustMS, trk);
    } else {
        return @"";
    }
    if (notes.empty()) return @"";

    std::string trackName = (name && name.length > 0)
        ? std::string([name UTF8String])
        : std::string("Notes");
    TimingElement* element = _context->AddTimingElement(trackName, "");
    if (!element) return @"";
    EffectLayer* layer = element->GetEffectLayer(0);
    if (!layer) return @"";

    auto marks = NoteImporter::BuildNoteMarks(notes, interval, durationMS);
    for (const auto& m : marks) {
        if (m.endMS <= m.startMS) continue;
        layer->AddEffect(0, m.label, "", "", m.startMS, m.endMS, EFFECT_NOT_SELECTED, false);
    }

    _context->GetSequenceElements().IncrementChangeCount(nullptr);
    return [NSString stringWithUTF8String:trackName.c_str()];
}

- (NSArray<NSString*>*)noteImportTracksFromPath:(NSString*)path
                                         format:(NSString*)format {
    NSMutableArray<NSString*>* res = [NSMutableArray array];
    if (!path || path.length == 0 || !format) return res;
    std::string file([path UTF8String]);
    std::string fmt([format UTF8String]);
    if (fmt == "musicxml") {
        for (const auto& t : NoteImporter::MusicXMLTracks(file)) {
            [res addObject:[NSString stringWithUTF8String:t.c_str()]];
        }
    } else if (fmt == "midi") {
        int n = NoteImporter::MIDITrackCount(file);
        for (int i = 1; i <= n; ++i) {
            [res addObject:[NSString stringWithFormat:@"%d", i]];
        }
    }
    return res;
}

- (BOOL)exportCurrentAudioToPath:(NSString*)path {
    if (!_context || !path || path.length == 0) return NO;
    AudioManager* am = _context->GetCurrentMediaManager();
    if (!am) return NO;
    return am->WriteCurrentAudioToFile(std::string([path UTF8String])) ? YES : NO;
}

- (NSString*)writeCurrentToTempWav {
    if (!_context) return nil;
    AudioManager* am = _context->GetCurrentMediaManager();
    if (!am) return nil;
    std::string p = am->WriteCurrentToTempWav();
    if (p.empty()) return nil;
    return [NSString stringWithUTF8String:p.c_str()];
}

- (NSString*)sequenceAudioFilePath {
    if (!_context || !_context->IsSequenceLoaded()) return nil;
    AudioManager* am = _context->GetCurrentMediaManager();
    if (!am) return nil;
    const std::string& fn = am->FileName();
    if (fn.empty()) return nil;
    return [NSString stringWithUTF8String:fn.c_str()];
}

- (int)addTimingMarkAtRow:(int)rowIndex
                  startMS:(int)startMS
                    endMS:(int)endMS
                    label:(NSString*)label {
    auto* row = _context->GetSequenceElements().GetRowInformation(rowIndex);
    if (!row || !row->element) return -1;
    if (row->element->GetType() != ElementType::ELEMENT_TYPE_TIMING) return -1;
    auto* layer = [self effectLayerForRow:rowIndex];
    if (!layer) return -1;
    if (startMS < 0) startMS = 0;
    if (endMS <= startMS) return -1;
    // Reject overlap with any existing mark on the same layer.
    for (int i = 0; i < layer->GetEffectCount(); i++) {
        Effect* other = layer->GetEffect(i);
        if (!other) continue;
        int os = other->GetStartTimeMS();
        int oe = other->GetEndTimeMS();
        if (startMS < oe && endMS > os) return -1;
    }
    std::string lbl = label ? std::string([label UTF8String]) : std::string();
    Effect* e = layer->AddEffect(0, lbl, "", "", startMS, endMS, 0, false);
    if (!e) return -1;
    for (int i = 0; i < layer->GetEffectCount(); i++) {
        if (layer->GetEffect(i) == e) return i;
    }
    return -1;
}

- (BOOL)deleteTimingMarkAtRow:(int)rowIndex atIndex:(int)markIndex {
    auto* row = _context->GetSequenceElements().GetRowInformation(rowIndex);
    if (!row || !row->element) return NO;
    if (row->element->GetType() != ElementType::ELEMENT_TYPE_TIMING) return NO;
    auto* layer = [self effectLayerForRow:rowIndex];
    if (!layer || markIndex < 0 || markIndex >= layer->GetEffectCount()) return NO;
    layer->DeleteEffectByIndex(markIndex);
    return YES;
}

- (BOOL)setTimingMarkLabelAtRow:(int)rowIndex
                        atIndex:(int)markIndex
                          label:(NSString*)label {
    auto* row = _context->GetSequenceElements().GetRowInformation(rowIndex);
    if (!row || !row->element) return NO;
    if (row->element->GetType() != ElementType::ELEMENT_TYPE_TIMING) return NO;
    auto* layer = [self effectLayerForRow:rowIndex];
    if (!layer || markIndex < 0 || markIndex >= layer->GetEffectCount()) return NO;
    Effect* e = layer->GetEffect(markIndex);
    if (!e) return NO;
    std::string lbl = label ? std::string([label UTF8String]) : std::string();
    e->SetEffectName(lbl);
    return YES;
}

- (BOOL)breakdownPhrasesAtRow:(int)rowIndex {
    auto& se = _context->GetSequenceElements();
    auto* row = se.GetRowInformation(rowIndex);
    if (!row || !row->element) return NO;
    if (row->element->GetType() != ElementType::ELEMENT_TYPE_TIMING) return NO;
    if (row->layerIndex != 0) return NO;  // only the phrase layer
    TimingElement* te = dynamic_cast<TimingElement*>(row->element);
    if (!te) return NO;

    EffectLayer* phraseLayer = te->GetEffectLayer(0);
    if (!phraseLayer) return NO;
    if (phraseLayer->GetEffectCount() == 0) return NO;

    // Lock guard: desktop rejects breakdown when any existing
    // word/phoneme mark is locked, to avoid silently wiping work
    // the user pinned. Match that behavior.
    for (int k = (int)te->GetEffectLayerCount() - 1; k > 0; --k) {
        EffectLayer* ck = te->GetEffectLayer(k);
        if (!ck) continue;
        for (auto&& eff : ck->GetAllEffects()) {
            if (eff && eff->IsLocked()) return NO;
        }
    }

    // Discard any existing word+phoneme layers, then add a fresh
    // word layer. Mirrors `RowHeading::BreakdownTimingPhrases`.
    te->SetFixedTiming(0);
    while (te->GetEffectLayerCount() > 1) {
        te->RemoveEffectLayer((int)te->GetEffectLayerCount() - 1);
    }
    EffectLayer* wordLayer = te->AddEffectLayer();
    if (!wordLayer) return NO;

    double freq = se.GetFrequency();
    static const std::string delims = " \t:;,.-_!?{}[]()<>+=|";
    for (int i = 0; i < phraseLayer->GetEffectCount(); i++) {
        Effect* pe = phraseLayer->GetEffect(i);
        if (!pe) continue;
        std::string phrase = pe->GetEffectName();
        if (phrase.empty()) continue;
        std::vector<std::string> words;
        size_t start = 0;
        while (start < phrase.size()) {
            size_t pos = phrase.find_first_of(delims, start);
            if (pos != start) {
                std::string w = phrase.substr(start, (pos == std::string::npos ? phrase.size() : pos) - start);
                if (!w.empty()) words.push_back(std::move(w));
            }
            if (pos == std::string::npos) break;
            start = pos + 1;
        }
        if (words.empty()) continue;
        int phraseStart = pe->GetStartTimeMS();
        int phraseEnd = pe->GetEndTimeMS();
        double intervalMS = double(phraseEnd - phraseStart) / double(words.size());
        int curStart = phraseStart;
        for (int w = 0; w < (int)words.size(); w++) {
            int curEnd = RoundToMultipleOfPeriod(
                phraseStart + int(intervalMS * (w + 1)), freq);
            if (w == (int)words.size() - 1 || curEnd > phraseEnd) {
                curEnd = phraseEnd;
            }
            if (curEnd > curStart) {
                wordLayer->AddEffect(0, words[w], "", "",
                                      curStart, curEnd,
                                      /*EFFECT_NOT_SELECTED*/ 0, false);
            }
            curStart = curEnd;
        }
    }
    se.PopulateRowInformation();
    return YES;
}

- (BOOL)breakdownPhraseAtRow:(int)rowIndex atIndex:(int)phraseIndex {
    auto& se = _context->GetSequenceElements();
    auto* row = se.GetRowInformation(rowIndex);
    if (!row || !row->element) return NO;
    if (row->element->GetType() != ElementType::ELEMENT_TYPE_TIMING) return NO;
    if (row->layerIndex != 0) return NO;
    TimingElement* te = dynamic_cast<TimingElement*>(row->element);
    if (!te) return NO;

    EffectLayer* phraseLayer = te->GetEffectLayer(0);
    if (!phraseLayer) return NO;
    if (phraseIndex < 0 || phraseIndex >= phraseLayer->GetEffectCount()) return NO;
    Effect* pe = phraseLayer->GetEffect(phraseIndex);
    if (!pe) return NO;
    std::string phrase = pe->GetEffectName();
    if (phrase.empty()) return NO;

    int phraseStart = pe->GetStartTimeMS();
    int phraseEnd = pe->GetEndTimeMS();

    // Reject when any locked word/phoneme already sits inside the
    // target range — same safety the row-level breakdown enforces,
    // narrowed to just this phrase's window.
    auto overlapsRange = [&](Effect* eff) {
        if (!eff) return false;
        return eff->GetEndTimeMS() > phraseStart && eff->GetStartTimeMS() < phraseEnd;
    };
    for (int k = (int)te->GetEffectLayerCount() - 1; k > 0; --k) {
        EffectLayer* ck = te->GetEffectLayer(k);
        if (!ck) continue;
        for (auto&& eff : ck->GetAllEffects()) {
            if (eff && eff->IsLocked() && overlapsRange(eff)) return NO;
        }
    }

    // Tokenize the phrase the same way the row-level breakdown does.
    static const std::string delims = " \t:;,.-_!?{}[]()<>+=|";
    std::vector<std::string> words;
    {
        size_t start = 0;
        while (start < phrase.size()) {
            size_t pos = phrase.find_first_of(delims, start);
            if (pos != start) {
                std::string w = phrase.substr(start, (pos == std::string::npos ? phrase.size() : pos) - start);
                if (!w.empty()) words.push_back(std::move(w));
            }
            if (pos == std::string::npos) break;
            start = pos + 1;
        }
    }
    if (words.empty()) return NO;

    // Ensure there's a words layer to write into. The row-level path
    // wipes everything down to the phrase layer; the per-mark path
    // is more surgical — preserve other phrases' words/phonemes.
    te->SetFixedTiming(0);
    EffectLayer* wordLayer = te->GetEffectLayer(1);
    if (!wordLayer) wordLayer = te->AddEffectLayer();
    if (!wordLayer) return NO;

    // Wipe existing word effects that fall inside this phrase's window
    // (and phonemes, if a layer 2 exists). DeleteEffect handles the
    // layer's internal index updates so a copied id list is enough.
    auto wipeOverlapping = [&](EffectLayer* layer) {
        if (!layer) return;
        auto effs = layer->GetAllEffectsByTime(phraseStart, phraseEnd);
        for (auto* eff : effs) layer->DeleteEffect(eff->GetID());
    };
    wipeOverlapping(wordLayer);
    if (te->GetEffectLayerCount() > 2) wipeOverlapping(te->GetEffectLayer(2));

    double freq = se.GetFrequency();
    double intervalMS = double(phraseEnd - phraseStart) / double(words.size());
    int curStart = phraseStart;
    for (int w = 0; w < (int)words.size(); w++) {
        int curEnd = RoundToMultipleOfPeriod(
            phraseStart + int(intervalMS * (w + 1)), freq);
        if (w == (int)words.size() - 1 || curEnd > phraseEnd) {
            curEnd = phraseEnd;
        }
        if (curEnd > curStart) {
            wordLayer->AddEffect(0, words[w], "", "",
                                  curStart, curEnd, 0, false);
        }
        curStart = curEnd;
    }

    se.PopulateRowInformation();
    return YES;
}

// MARK: - Tags (B34 / B35)

- (int)tagPositionAtIndex:(int)index {
    if (index < 0 || index > 9) return -1;
    return _context->GetSequenceElements().GetTagPosition(index);
}

- (void)setTagPositionAtIndex:(int)index positionMS:(int)position {
    if (index < 0 || index > 9) return;
    int duration = _context->GetSequenceFile()
                   ? _context->GetSequenceFile()->GetSequenceDurationMS()
                   : 0;
    if (duration > 0 && position > duration) position = duration;
    _context->GetSequenceElements().SetTagPosition(index, position);
}

- (void)clearAllTags {
    _context->GetSequenceElements().ClearTags();
}

- (BOOL)breakdownWordsAtRow:(int)rowIndex {
    auto& se = _context->GetSequenceElements();
    auto* row = se.GetRowInformation(rowIndex);
    if (!row || !row->element) return NO;
    if (row->element->GetType() != ElementType::ELEMENT_TYPE_TIMING) return NO;
    TimingElement* te = dynamic_cast<TimingElement*>(row->element);
    if (!te) return NO;
    // Need layer 1 (words) populated. If the element only has a
    // phrases layer, the user hasn't broken down phrases yet — punt.
    if (te->GetEffectLayerCount() < 2) return NO;

    // Lock guard — refuse if any existing phoneme mark is locked.
    if (te->GetEffectLayerCount() > 2) {
        EffectLayer* phonemeLayer = te->GetEffectLayer(2);
        if (phonemeLayer) {
            for (auto&& eff : phonemeLayer->GetAllEffects()) {
                if (eff && eff->IsLocked()) return NO;
            }
        }
        te->RemoveEffectLayer(2);
    }
    EffectLayer* wordLayer = te->GetEffectLayer(1);
    if (!wordLayer) return NO;
    EffectLayer* phonemeLayer = te->AddEffectLayer();
    if (!phonemeLayer) return NO;

    PhonemeDictionary& dict = _context->GetPhonemeDictionary();
    double freq = se.GetFrequency();
    auto& undoMgr = se.get_undo_mgr();
    for (int i = 0; i < wordLayer->GetEffectCount(); i++) {
        Effect* effect = wordLayer->GetEffect(i);
        if (!effect) continue;
        std::string word = effect->GetEffectName();
        if (word.empty()) continue;
        BreakdownWord(phonemeLayer,
                       effect->GetStartTimeMS(),
                       effect->GetEndTimeMS(),
                       word, freq, dict, undoMgr);
    }
    se.PopulateRowInformation();
    return YES;
}

// MARK: - Views

- (NSArray<NSString*>*)availableViews {
    auto& se = _context->GetSequenceElements();
    NSMutableArray<NSString*>* out = [NSMutableArray array];
    int n = se.GetViewCount();
    for (int i = 0; i < n; i++) {
        [out addObject:[NSString stringWithUTF8String:se.GetViewName(i).c_str()]];
    }
    return out;
}

- (int)currentViewIndex {
    return _context->GetSequenceElements().GetCurrentView();
}

- (void)setCurrentViewIndex:(int)viewIndex {
    auto& se = _context->GetSequenceElements();
    if (viewIndex < 0 || viewIndex >= se.GetViewCount()) return;

    // Mirror desktop ViewsModelsPanel::SelectView: for non-Master views we
    // must resolve the view's model list, ensure those models are in the
    // sequence, and populate the view slot in mAllViews before switching.
    // Without this, switching to any non-Master view shows an empty grid.
    std::string viewName = se.GetViewName(viewIndex);
    if (viewIndex > 0) {
        std::string modelsString = se.GetViewModels(viewName);
        se.AddMissingModelsToSequence(modelsString);
        se.PopulateView(modelsString, viewIndex);
    }
    se.SetCurrentView(viewIndex);
    se.SetTimingVisibility(viewName);
    se.PopulateRowInformation();
}

// MARK: - Display Elements editor (F-6)
//
// `SequenceViewManager` owns the view metadata (names + ordered
// model-name lists); `SequenceElements` owns the parallel
// `mAllViews[viewIdx]` vector of `Element*`s the grid actually walks
// for the current view. Every mutation below keeps both in lockstep,
// marks the sequence dirty so the 500 ms poll activates Save, and
// posts `XLViewsChanged` so the SwiftUI view picker + any open
// Display Elements sheet refresh.

- (void)postViewsChanged {
    [[NSNotificationCenter defaultCenter]
        postNotificationName:@"XLViewsChanged" object:self];
}

- (BOOL)addViewNamed:(NSString*)name {
    if (!_context || !_context->IsSequenceLoaded()) return NO;
    if (!name) return NO;
    std::string n = std::string([name UTF8String]);
    // Trim whitespace to avoid collisions like " Roof" / "Roof".
    while (!n.empty() && std::isspace((unsigned char)n.front())) n.erase(n.begin());
    while (!n.empty() && std::isspace((unsigned char)n.back())) n.pop_back();
    if (n.empty()) return NO;
    auto& vm = _context->GetSequenceViewManager();
    if (vm.GetView(n) != nullptr) return NO;  // duplicate
    vm.AddView(n);
    // Parallel empty slot on SequenceElements::mAllViews so the new
    // view index is addressable via GetElement(i, viewIdx).
    _context->GetSequenceElements().AddView(n);
    _context->GetSequenceElements().IncrementChangeCount(nullptr);
    [self postViewsChanged];
    return YES;
}

- (BOOL)deleteViewAtIndex:(int)idx {
    if (!_context || !_context->IsSequenceLoaded()) return NO;
    if (idx <= 0) return NO;  // can't delete Master View
    auto& se = _context->GetSequenceElements();
    auto& vm = _context->GetSequenceViewManager();
    if (idx >= vm.GetViewCount()) return NO;
    std::string name = se.GetViewName(idx);
    if (name.empty()) return NO;

    // If deleting the currently-active view, fall back to Master first
    // so row info doesn't dereference a slot that's about to be
    // erased.
    bool wasCurrent = (se.GetCurrentView() == idx);
    if (wasCurrent) {
        se.SetCurrentView(MASTER_VIEW);
        se.SetTimingVisibility("Master View");
    }
    se.RemoveView(idx);
    vm.DeleteView(name);
    if (wasCurrent) {
        se.PopulateRowInformation();
    }
    se.IncrementChangeCount(nullptr);
    [self postViewsChanged];
    return YES;
}

- (BOOL)renameViewAtIndex:(int)idx to:(NSString*)newName {
    if (!_context || !_context->IsSequenceLoaded()) return NO;
    if (idx <= 0) return NO;  // Master View name is fixed
    if (!newName) return NO;
    std::string n = std::string([newName UTF8String]);
    while (!n.empty() && std::isspace((unsigned char)n.front())) n.erase(n.begin());
    while (!n.empty() && std::isspace((unsigned char)n.back())) n.pop_back();
    if (n.empty()) return NO;
    auto& vm = _context->GetSequenceViewManager();
    if (idx >= vm.GetViewCount()) return NO;
    if (vm.GetView(n) != nullptr) return NO;  // collision
    std::string oldName = vm.GetView(idx)->GetName();
    if (oldName == n) return YES;
    vm.RenameView(oldName, n);
    // Desktop doesn't rewrite TimingElement::mViews CSVs on rename —
    // stale view names there are silently dropped at load. Match that
    // behaviour to avoid surprising round-trip side effects.
    _context->GetSequenceElements().IncrementChangeCount(nullptr);
    [self postViewsChanged];
    return YES;
}

- (BOOL)cloneViewAtIndex:(int)idx as:(NSString*)newName {
    if (!_context || !_context->IsSequenceLoaded()) return NO;
    if (!newName) return NO;
    std::string n = std::string([newName UTF8String]);
    while (!n.empty() && std::isspace((unsigned char)n.front())) n.erase(n.begin());
    while (!n.empty() && std::isspace((unsigned char)n.back())) n.pop_back();
    if (n.empty()) return NO;
    auto& se = _context->GetSequenceElements();
    auto& vm = _context->GetSequenceViewManager();
    if (idx < 0 || idx >= vm.GetViewCount()) return NO;
    if (vm.GetView(n) != nullptr) return NO;

    SequenceView* src = vm.GetView(idx);
    if (src == nullptr) return NO;
    SequenceView* dst = vm.AddView(n);
    if (dst == nullptr) return NO;

    // Master View model list comes from the live Element set; every
    // other view stores its own `_modelNames` which we copy directly.
    if (idx == MASTER_VIEW) {
        std::string models;
        for (int i = 0; i < (int)se.GetElementCount(); i++) {
            Element* elem = se.GetElement(i);
            if (elem && elem->GetType() == ElementType::ELEMENT_TYPE_MODEL) {
                if (!models.empty()) models += ",";
                models += elem->GetName();
            }
        }
        dst->SetModels(models);
    } else {
        dst->SetModels(src->GetModelsString());
    }

    // Parallel mAllViews slot for the new view. Timings that belonged
    // to the source view follow (desktop `OnButtonCloneClick` copies
    // the source view's timing memberships onto the clone).
    se.AddView(n);
    std::vector<std::string> timings;
    for (int i = 0; i < (int)se.GetElementCount(idx); i++) {
        Element* elem = se.GetElement(i, idx);
        if (elem && elem->GetType() == ElementType::ELEMENT_TYPE_TIMING) {
            timings.push_back(elem->GetName());
        }
    }
    if (!timings.empty()) {
        se.AddViewToTimings(timings, n);
    }

    se.IncrementChangeCount(nullptr);
    [self postViewsChanged];
    return YES;
}

- (BOOL)moveViewUpAtIndex:(int)idx {
    if (!_context || !_context->IsSequenceLoaded()) return NO;
    // Master View is locked to index 0; user views occupy 1..N-1.
    if (idx <= 1) return NO;
    auto& vm = _context->GetSequenceViewManager();
    if (idx >= vm.GetViewCount()) return NO;
    vm.MoveViewUp(idx);
    _context->GetSequenceElements().IncrementChangeCount(nullptr);
    [self postViewsChanged];
    return YES;
}

- (BOOL)moveViewDownAtIndex:(int)idx {
    if (!_context || !_context->IsSequenceLoaded()) return NO;
    if (idx < 1) return NO;
    auto& vm = _context->GetSequenceViewManager();
    if (idx >= vm.GetViewCount() - 1) return NO;
    vm.MoveViewDown(idx);
    _context->GetSequenceElements().IncrementChangeCount(nullptr);
    [self postViewsChanged];
    return YES;
}

// MARK: Models in a view

- (NSArray<NSString*>*)modelsInViewAtIndex:(int)idx {
    NSMutableArray<NSString*>* out = [NSMutableArray array];
    if (!_context || !_context->IsSequenceLoaded()) return out;
    auto& se = _context->GetSequenceElements();
    auto& vm = _context->GetSequenceViewManager();
    if (idx < 0 || idx >= vm.GetViewCount()) return out;

    if (idx == MASTER_VIEW) {
        // Every model Element in load order. Matches desktop
        // `GetMasterViewModels()` (line 1284 of ViewsModelsPanel.cpp).
        for (int i = 0; i < (int)se.GetElementCount(); i++) {
            Element* elem = se.GetElement(i);
            if (elem && elem->GetType() == ElementType::ELEMENT_TYPE_MODEL) {
                [out addObject:[NSString stringWithUTF8String:elem->GetName().c_str()]];
            }
        }
    } else {
        SequenceView* v = vm.GetView(idx);
        if (!v) return out;
        for (const auto& m : v->GetModels()) {
            [out addObject:[NSString stringWithUTF8String:m.c_str()]];
        }
    }
    return out;
}

- (BOOL)addModel:(NSString*)name toViewAtIndex:(int)idx atPosition:(int)pos {
    if (!_context || !_context->IsSequenceLoaded()) return NO;
    if (idx <= 0) return NO;  // Master membership is derived
    if (!name || name.length == 0) return NO;
    std::string n = std::string([name UTF8String]);
    auto& se = _context->GetSequenceElements();
    auto& vm = _context->GetSequenceViewManager();
    SequenceView* v = vm.GetView(idx);
    if (!v) return NO;
    // Desktop `ViewsModelsPanel::AddSelectedModels` (non-MASTER path)
    // auto-brings the model into the sequence via
    // `AddMissingModelsToSequence` if it's only in the show layout, so
    // users can pick from the full ModelManager roster when editing a
    // user view. Match that: if `n` isn't yet an Element but *is* a
    // known model, add it to Master first.
    Element* elem = se.GetElement(n);
    if (elem == nullptr) {
        Model* m = _context->GetModel(n);
        if (m == nullptr) return NO;  // not in show at all
        se.AddMissingModelsToSequence(n);
        elem = se.GetElement(n);
        if (elem == nullptr) return NO;
        elem->SetVisible(true);
    }
    if (elem->GetType() != ElementType::ELEMENT_TYPE_MODEL) return NO;
    if (v->ContainsModel(n)) return NO;
    v->AddModel(n, pos);
    if (idx == se.GetCurrentView()) {
        se.PopulateView(v->GetModelsString(), idx);
    }
    se.PopulateRowInformation();
    se.IncrementChangeCount(nullptr);
    [self postViewsChanged];
    return YES;
}

- (BOOL)removeModel:(NSString*)name fromViewAtIndex:(int)idx {
    if (!_context || !_context->IsSequenceLoaded()) return NO;
    if (idx <= 0) return NO;
    if (!name || name.length == 0) return NO;
    std::string n = std::string([name UTF8String]);
    auto& se = _context->GetSequenceElements();
    auto& vm = _context->GetSequenceViewManager();
    SequenceView* v = vm.GetView(idx);
    if (!v) return NO;
    if (!v->ContainsModel(n)) return NO;
    v->RemoveModel(n);
    if (idx == se.GetCurrentView()) {
        se.PopulateView(v->GetModelsString(), idx);
        se.PopulateRowInformation();
    }
    se.IncrementChangeCount(nullptr);
    [self postViewsChanged];
    return YES;
}

- (BOOL)moveModel:(NSString*)name inViewAtIndex:(int)idx toPosition:(int)pos {
    if (!_context || !_context->IsSequenceLoaded()) return NO;
    if (idx <= 0) return NO;
    if (!name || name.length == 0) return NO;
    std::string n = std::string([name UTF8String]);
    auto& se = _context->GetSequenceElements();
    auto& vm = _context->GetSequenceViewManager();
    SequenceView* v = vm.GetView(idx);
    if (!v) return NO;
    if (!v->ContainsModel(n)) return NO;
    v->RemoveModel(n);
    v->AddModel(n, pos);
    if (idx == se.GetCurrentView()) {
        se.PopulateView(v->GetModelsString(), idx);
        se.PopulateRowInformation();
    }
    se.IncrementChangeCount(nullptr);
    [self postViewsChanged];
    return YES;
}

- (nullable NSArray<NSString*>*)copyViewToMasterAtIndex:(int)idx {
    if (!_context || !_context->IsSequenceLoaded()) return nil;
    if (idx <= MASTER_VIEW) return nil;
    auto& se = _context->GetSequenceElements();
    auto& vm = _context->GetSequenceViewManager();
    SequenceView* view = vm.GetView(idx);
    if (!view) return nil;

    std::list<std::string> modelList = view->GetModels();
    std::vector<std::string> models(modelList.begin(), modelList.end());
    _context->AbortRender(5000);

    // Drop Master model Elements absent from the view that have no
    // effects; keep (and report) those that do (desktop DoMakeMaster).
    NSMutableArray<NSString*>* kept = [NSMutableArray array];
    for (int i = 0; i < (int)se.GetElementCount(MASTER_VIEW); ++i) {
        Element* elem = se.GetElement(i);
        if (!elem || elem->GetType() == ElementType::ELEMENT_TYPE_TIMING) continue;
        const std::string& name = elem->GetName();
        if (std::find(models.begin(), models.end(), name) != models.end()) continue;
        if (elem->HasEffects()) {
            [kept addObject:[NSString stringWithUTF8String:name.c_str()]];
        } else {
            se.DeleteElement(name);
            --i;
        }
    }

    // Timing Elements always precede models in the Master View; the new
    // models slot in right after them (desktop GetTimingCount offset).
    int timingCount = 0;
    for (int i = 0; i < (int)se.GetElementCount(MASTER_VIEW); ++i) {
        Element* elem = se.GetElement(i);
        if (elem && elem->GetType() == ElementType::ELEMENT_TYPE_TIMING) ++timingCount;
    }
    for (size_t i = 0; i < models.size(); ++i) {
        int existing = se.GetElementIndex(models[i], MASTER_VIEW);
        int dest = (int)i + timingCount;
        if (existing < 0) {
            Element* e = se.AddElement(dest, models[i], "model", true, false, false, false, false);
            if (e) e->AddEffectLayer();
        } else {
            se.MoveSequenceElement(existing, dest, MASTER_VIEW);
        }
    }

    se.PopulateRowInformation();
    se.IncrementChangeCount(nullptr);
    [self postViewsChanged];
    return kept;
}

- (nullable NSString*)importViewConfigFromSequencePath:(NSString*)path {
    if (!_context || !_context->IsSequenceLoaded() || !path) return nil;
    std::string p = [path UTF8String];
    ObtainAccessToURL(p, false);
    pugi::xml_document doc;
    if (!doc.load_file(p.c_str())) return nil;

    pugi::xml_node dispElements;
    for (pugi::xml_node node = doc.document_element().first_child(); node; node = node.next_sibling()) {
        if (std::string_view(node.name()) == "DisplayElements") {
            dispElements = node;
            break;
        }
    }
    if (!dispElements) return nil;

    auto& se = _context->GetSequenceElements();
    auto& vm = _context->GetSequenceViewManager();

    std::vector<std::string> importedModels;
    std::vector<std::string> importedTimings;
    for (pugi::xml_node node = dispElements.first_child(); node; node = node.next_sibling()) {
        std::string name = node.attribute("name").as_string();
        std::string type = node.attribute("type").as_string();
        if (name.empty()) continue;
        if (type == "timing") {
            Element* elem = se.GetElement(name);
            if (elem && elem->GetType() == ElementType::ELEMENT_TYPE_TIMING) {
                importedTimings.push_back(elem->GetName());
            }
        } else if (_context->GetModel(name) != nullptr) {
            importedModels.push_back(name);
        }
    }
    if (importedModels.empty() && importedTimings.empty()) return nil;

    // Uniquify the new view name (desktop CreateUniqueName).
    std::string base = "Imported Master";
    std::string viewName = base;
    int suffix = 1;
    while (vm.GetView(viewName) != nullptr) {
        viewName = base + "_" + std::to_string(suffix++);
    }

    SequenceView* view = vm.AddView(viewName);
    if (!view) return nil;
    std::string joined;
    for (const std::string& m : importedModels) {
        if (!joined.empty()) joined += ",";
        joined += m;
    }
    view->SetModels(joined);
    se.AddView(viewName);
    if (!importedTimings.empty()) {
        se.AddViewToTimings(importedTimings, viewName);
        se.SetTimingVisibility(viewName);
    }

    se.IncrementChangeCount(nullptr);
    [self postViewsChanged];
    return [NSString stringWithUTF8String:viewName.c_str()];
}

// MARK: Element roster + visibility

- (NSArray<NSString*>*)allModelNamesInShow {
    return [self modelsInViewAtIndex:MASTER_VIEW];
}

- (NSArray<NSString*>*)allTimingTrackNames {
    NSMutableArray<NSString*>* out = [NSMutableArray array];
    if (!_context || !_context->IsSequenceLoaded()) return out;
    auto& se = _context->GetSequenceElements();
    for (int i = 0; i < (int)se.GetElementCount(); i++) {
        Element* elem = se.GetElement(i);
        if (elem && elem->GetType() == ElementType::ELEMENT_TYPE_TIMING) {
            [out addObject:[NSString stringWithUTF8String:elem->GetName().c_str()]];
        }
    }
    return out;
}

- (NSArray<NSString*>*)modelsAvailableInShowLayout {
    // ModelManager models that are NOT yet an Element in the sequence.
    // Matches the desktop `ViewsModelsPanel::PopulateModels` logic at
    // lines 534-552 of ViewsModelsPanel.cpp — walk every Model in the
    // manager, include those whose name isn't already in the Master
    // View. Submodels are skipped (desktop never surfaces them as
    // top-level options); the iPad inherits that by filtering on
    // `GetDisplayAs() != SubModel`.
    NSMutableArray<NSString*>* out = [NSMutableArray array];
    if (!_context || !_context->IsSequenceLoaded()) return out;
    auto& se = _context->GetSequenceElements();
    auto& mm = _context->GetModelManager();
    for (auto it = mm.begin(); it != mm.end(); ++it) {
        if (!it->second) continue;
        if (it->second->GetDisplayAs() == DisplayAsType::SubModel) continue;
        if (!se.ElementExists(it->first, MASTER_VIEW)) {
            [out addObject:[NSString stringWithUTF8String:it->first.c_str()]];
        }
    }
    // Sort for predictable ordering — ModelManager iterates in map
    // order (alphabetical by name) already, but a belt-and-suspenders
    // sort avoids surprises when the UI diffs the list between calls.
    [out sortUsingSelector:@selector(localizedCaseInsensitiveCompare:)];
    return out;
}

- (BOOL)addModelToMasterView:(NSString*)name {
    if (!_context || !_context->IsSequenceLoaded()) return NO;
    if (!name || name.length == 0) return NO;
    std::string n = std::string([name UTF8String]);
    auto& se = _context->GetSequenceElements();
    if (se.ElementExists(n, MASTER_VIEW)) return NO;
    // Must be a known model in the show layout.
    Model* m = _context->GetModel(n);
    if (m == nullptr) return NO;
    // Mirror the desktop MASTER_VIEW branch of AddSelectedModels:
    // create the Element visible and bootstrap one empty effect layer.
    Element* e = se.AddElement(n, "model",
                                /*visible=*/true, /*collapsed=*/false,
                                /*active=*/false, /*selected=*/false,
                                /*renderDisabled=*/false);
    if (!e) return NO;
    e->AddEffectLayer();
    se.PopulateRowInformation();
    se.IncrementChangeCount(nullptr);
    [self postViewsChanged];
    return YES;
}

- (BOOL)elementHasEffects:(NSString*)name {
    if (!_context || !_context->IsSequenceLoaded()) return NO;
    if (!name || name.length == 0) return NO;
    std::string n = std::string([name UTF8String]);
    Element* elem = _context->GetSequenceElements().GetElement(n);
    return (elem && elem->HasEffects()) ? YES : NO;
}

- (BOOL)removeElementFromMasterView:(NSString*)name {
    if (!_context || !_context->IsSequenceLoaded()) return NO;
    if (!name || name.length == 0) return NO;
    std::string n = std::string([name UTF8String]);
    auto& se = _context->GetSequenceElements();
    if (!se.ElementExists(n, MASTER_VIEW)) return NO;
    // Desktop `RemoveSelectedModels` MASTER_VIEW branch aborts the
    // render first (issue #4134) to keep render workers from
    // dereferencing the about-to-be-deleted Element pointers.
    _context->AbortRender(5000);
    se.DeleteElement(n);
    // `DeleteElement` already repopulates row info internally; still
    // bump the change count + broadcast so the view picker and the
    // Display Elements sheet refresh.
    se.IncrementChangeCount(nullptr);
    [self postViewsChanged];
    return YES;
}

- (NSArray<NSString*>*)missingModelNamesWithEffects {
    NSMutableArray<NSString*>* out = [NSMutableArray array];
    if (!_context || !_context->IsSequenceLoaded() || !_context->HasModelManager()) return out;
    auto& se = _context->GetSequenceElements();
    auto& mm = _context->GetModelManager();
    for (int i = 0; i < (int)se.GetElementCount(); i++) {
        Element* el = se.GetElement(i);
        if (!el || el->GetType() != ElementType::ELEMENT_TYPE_MODEL) continue;
        const std::string& name = el->GetModelName();
        if (name.empty()) continue;
        if (mm[name] != nullptr) continue;
        if (el->GetEffectCount() <= 0) continue;
        // Skip names that match an existing model's "oldname:" alias —
        // CheckForValidModels auto-remaps those silently, no prompt
        // needed. Match desktop's `IsAlias(name, /*oldnameOnly=*/true)`.
        bool autoMapped = false;
        for (auto it = mm.begin(); it != mm.end(); ++it) {
            if (it->second && it->second->IsAlias(name, /*oldnameOnly=*/true)) {
                autoMapped = true;
                break;
            }
        }
        if (autoMapped) continue;
        [out addObject:[NSString stringWithUTF8String:name.c_str()]];
    }
    return out;
}

- (BOOL)resolveMissingModel:(NSString*)originalName
                 byRenameTo:(NSString*)existingName
                   addAlias:(BOOL)addAlias {
    if (!_context || !_context->IsSequenceLoaded() || !_context->HasModelManager()) return NO;
    if (!originalName || originalName.length == 0) return NO;
    if (!existingName || existingName.length == 0) return NO;
    std::string oldStd = originalName.UTF8String;
    std::string newStd = existingName.UTF8String;
    auto& se = _context->GetSequenceElements();
    auto& mm = _context->GetModelManager();
    Model* target = mm[newStd];
    if (target == nullptr) return NO;
    Element* el = se.GetElement(oldStd);
    if (!el || el->GetType() != ElementType::ELEMENT_TYPE_MODEL) return NO;

    _context->AbortRender(5000);
    el->SetName(newStd);
    static_cast<ModelElement*>(el)->Init(*target);
    if (addAlias) {
        target->AddAlias("oldname:" + oldStd);
        _context->MarkLayoutModelDirty(newStd);
    }
    se.PopulateRowInformation();
    se.IncrementChangeCount(nullptr);
    [self postViewsChanged];
    return YES;
}

- (BOOL)resolveMissingModel:(NSString*)originalName
                   byDelete:(BOOL)delete_ {
    if (!delete_) return NO;
    if (!_context || !_context->IsSequenceLoaded()) return NO;
    if (!originalName || originalName.length == 0) return NO;
    std::string oldStd = originalName.UTF8String;
    auto& se = _context->GetSequenceElements();
    Element* el = se.GetElement(oldStd);
    if (!el || el->GetType() != ElementType::ELEMENT_TYPE_MODEL) return NO;
    _context->AbortRender(5000);
    se.DeleteElement(oldStd);
    se.IncrementChangeCount(nullptr);
    [self postViewsChanged];
    return YES;
}

- (BOOL)elementVisible:(NSString*)name {
    if (!_context || !_context->IsSequenceLoaded()) return NO;
    if (!name || name.length == 0) return NO;
    std::string n = std::string([name UTF8String]);
    Element* elem = _context->GetSequenceElements().GetElement(n);
    if (!elem) return NO;
    if (elem->GetType() == ElementType::ELEMENT_TYPE_TIMING) {
        auto* te = dynamic_cast<TimingElement*>(elem);
        return te && te->GetMasterVisible() ? YES : NO;
    }
    return elem->GetVisible() ? YES : NO;
}

- (BOOL)setElementVisible:(NSString*)name visible:(BOOL)visible {
    if (!_context || !_context->IsSequenceLoaded()) return NO;
    if (!name || name.length == 0) return NO;
    std::string n = std::string([name UTF8String]);
    auto& se = _context->GetSequenceElements();
    Element* elem = se.GetElement(n);
    if (!elem) return NO;
    if (elem->GetType() == ElementType::ELEMENT_TYPE_TIMING) {
        auto* te = dynamic_cast<TimingElement*>(elem);
        if (!te) return NO;
        if (te->GetMasterVisible() == (bool)visible) return YES;
        te->SetMasterVisible(visible ? true : false);
        // If we're currently in Master View, the live visibility
        // needs to track the master flag; SetTimingVisibility reads
        // the right flag for the active view.
        se.SetTimingVisibility(se.GetViewName(se.GetCurrentView()));
    } else {
        if (elem->GetVisible() == (bool)visible) return YES;
        elem->SetVisible(visible ? true : false);
    }
    se.PopulateRowInformation();
    se.IncrementChangeCount(nullptr);
    [self postViewsChanged];
    return YES;
}

// Apply a master-visibility flag to one element, matching the
// model/timing split in setElementVisible: (no row repopulate /
// broadcast here — bulk callers do that once).
static void SetElementMasterVisible(SequenceElements& se, Element* elem, bool visible) {
    if (!elem) return;
    if (elem->GetType() == ElementType::ELEMENT_TYPE_TIMING) {
        auto* te = dynamic_cast<TimingElement*>(elem);
        if (te && te->GetMasterVisible() != visible) {
            te->SetMasterVisible(visible);
        }
    } else if (elem->GetVisible() != visible) {
        elem->SetVisible(visible);
    }
}

- (BOOL)setAllElementsVisible:(BOOL)visible {
    if (!_context || !_context->IsSequenceLoaded()) return NO;
    auto& se = _context->GetSequenceElements();
    for (int i = 0; i < (int)se.GetElementCount(); i++) {
        SetElementMasterVisible(se, se.GetElement(i), visible ? true : false);
    }
    se.SetTimingVisibility(se.GetViewName(se.GetCurrentView()));
    se.PopulateRowInformation();
    se.IncrementChangeCount(nullptr);
    [self postViewsChanged];
    return YES;
}

- (BOOL)hideUnusedElements {
    if (!_context || !_context->IsSequenceLoaded()) return NO;
    auto& se = _context->GetSequenceElements();
    for (int i = 0; i < (int)se.GetElementCount(); i++) {
        Element* elem = se.GetElement(i);
        if (elem && !elem->HasEffects()) {
            SetElementMasterVisible(se, elem, false);
        }
    }
    se.SetTimingVisibility(se.GetViewName(se.GetCurrentView()));
    se.PopulateRowInformation();
    se.IncrementChangeCount(nullptr);
    [self postViewsChanged];
    return YES;
}

- (NSInteger)removeUnusedElements {
    if (!_context || !_context->IsSequenceLoaded()) return 0;
    auto& se = _context->GetSequenceElements();
    std::vector<std::string> toRemove;
    for (int i = 0; i < (int)se.GetElementCount(); i++) {
        Element* elem = se.GetElement(i);
        if (elem && !elem->HasEffects()) {
            toRemove.push_back(elem->GetName());
        }
    }
    if (toRemove.empty()) return 0;
    _context->AbortRender(5000);
    for (const std::string& n : toRemove) {
        se.DeleteElement(n);
    }
    se.IncrementChangeCount(nullptr);
    [self postViewsChanged];
    return (NSInteger)toRemove.size();
}

// MARK: Timing-track per-view membership

- (NSArray<NSString*>*)viewsContainingTiming:(NSString*)timingName {
    NSMutableArray<NSString*>* out = [NSMutableArray array];
    if (!_context || !_context->IsSequenceLoaded()) return out;
    if (!timingName || timingName.length == 0) return out;
    std::string n = std::string([timingName UTF8String]);
    Element* elem = _context->GetSequenceElements().GetElement(n);
    auto* te = dynamic_cast<TimingElement*>(elem);
    if (!te) return out;
    // `mViews` is a comma-separated list of view names. Empty entries
    // can occur after a delete/rename churn — skip them.
    std::string csv = te->GetViews();
    size_t start = 0;
    while (start <= csv.size()) {
        size_t pos = csv.find(',', start);
        std::string part = csv.substr(start, (pos == std::string::npos ? csv.size() : pos) - start);
        // Trim
        while (!part.empty() && std::isspace((unsigned char)part.front())) part.erase(part.begin());
        while (!part.empty() && std::isspace((unsigned char)part.back())) part.pop_back();
        if (!part.empty()) {
            [out addObject:[NSString stringWithUTF8String:part.c_str()]];
        }
        if (pos == std::string::npos) break;
        start = pos + 1;
    }
    return out;
}

- (BOOL)addTiming:(NSString*)timingName toViewNamed:(NSString*)viewName {
    if (!_context || !_context->IsSequenceLoaded()) return NO;
    if (!timingName || timingName.length == 0) return NO;
    if (!viewName || viewName.length == 0) return NO;
    std::string t = std::string([timingName UTF8String]);
    std::string v = std::string([viewName UTF8String]);
    auto& se = _context->GetSequenceElements();
    // Validate: timing exists and target view exists.
    Element* elem = se.GetElement(t);
    if (!elem || elem->GetType() != ElementType::ELEMENT_TYPE_TIMING) return NO;
    if (_context->GetSequenceViewManager().GetView(v) == nullptr) return NO;
    se.AddTimingToView(t, v);
    // Rebind visibility for the currently-active view in case we just
    // added the timing to it.
    se.SetTimingVisibility(se.GetViewName(se.GetCurrentView()));
    se.PopulateRowInformation();
    se.IncrementChangeCount(nullptr);
    [self postViewsChanged];
    return YES;
}

- (BOOL)removeTiming:(NSString*)timingName fromViewNamed:(NSString*)viewName {
    if (!_context || !_context->IsSequenceLoaded()) return NO;
    if (!timingName || timingName.length == 0) return NO;
    if (!viewName || viewName.length == 0) return NO;
    std::string t = std::string([timingName UTF8String]);
    std::string v = std::string([viewName UTF8String]);
    auto& se = _context->GetSequenceElements();
    Element* elem = se.GetElement(t);
    if (!elem || elem->GetType() != ElementType::ELEMENT_TYPE_TIMING) return NO;
    int vIdx = _context->GetSequenceViewManager().GetViewIndex(v);
    if (vIdx < 0) return NO;
    se.DeleteTimingFromView(t, vIdx);
    se.SetTimingVisibility(se.GetViewName(se.GetCurrentView()));
    se.PopulateRowInformation();
    se.IncrementChangeCount(nullptr);
    [self postViewsChanged];
    return YES;
}

- (BOOL)addTimingToAllViews:(NSString*)timingName {
    if (!_context || !_context->IsSequenceLoaded()) return NO;
    if (!timingName || timingName.length == 0) return NO;
    std::string t = std::string([timingName UTF8String]);
    auto& se = _context->GetSequenceElements();
    Element* elem = se.GetElement(t);
    if (!elem || elem->GetType() != ElementType::ELEMENT_TYPE_TIMING) return NO;
    se.AddTimingToAllViews(t);
    se.SetTimingVisibility(se.GetViewName(se.GetCurrentView()));
    se.PopulateRowInformation();
    se.IncrementChangeCount(nullptr);
    [self postViewsChanged];
    return YES;
}

- (EffectLayer*)effectLayerForRow:(int)rowIndex {
    auto* row = _context->GetSequenceElements().GetRowInformation(rowIndex);
    if (!row || !row->element) return nullptr;
    return row->element->GetEffectLayer(row->layerIndex);
}

- (int)effectCountForRow:(int)rowIndex {
    auto* layer = [self effectLayerForRow:rowIndex];
    return layer ? (int)layer->GetEffectCount() : 0;
}

- (NSArray<NSString*>*)effectNamesForRow:(int)rowIndex {
    auto* layer = [self effectLayerForRow:rowIndex];
    if (!layer) return @[];

    NSMutableArray* names = [NSMutableArray array];
    for (int i = 0; i < layer->GetEffectCount(); i++) {
        Effect* e = layer->GetEffect(i);
        [names addObject:[NSString stringWithUTF8String:e->GetEffectName().c_str()]];
    }
    return names;
}

- (NSArray<NSNumber*>*)effectStartTimesForRow:(int)rowIndex {
    auto* layer = [self effectLayerForRow:rowIndex];
    if (!layer) return @[];

    NSMutableArray* times = [NSMutableArray array];
    for (int i = 0; i < layer->GetEffectCount(); i++) {
        Effect* e = layer->GetEffect(i);
        [times addObject:@(e->GetStartTimeMS())];
    }
    return times;
}

- (NSArray<NSNumber*>*)effectEndTimesForRow:(int)rowIndex {
    auto* layer = [self effectLayerForRow:rowIndex];
    if (!layer) return @[];

    NSMutableArray* times = [NSMutableArray array];
    for (int i = 0; i < layer->GetEffectCount(); i++) {
        Effect* e = layer->GetEffect(i);
        [times addObject:@(e->GetEndTimeMS())];
    }
    return times;
}

// MARK: - Model Preview

- (void)setModelColorsAtMS:(int)frameMS {
    _context->SetModelColors(frameMS);
}

- (void*)renderContext {
    return _context.get();
}

- (NSArray<NSString*>*)layoutGroups {
    // Order matches desktop LayoutPanel::Reset: Default, virtuals,
    // named groups. The two virtuals ("All Models", "Unassigned")
    // are not stored in `_namedLayoutGroups`; the model-filter
    // code recognises them by literal string in
    // GetModelsForActivePreview and the *ListSummary helpers.
    NSMutableArray<NSString*>* out = [NSMutableArray array];
    [out addObject:@"Default"];
    [out addObject:@"All Models"];
    [out addObject:@"Unassigned"];
    if (!_context) return out;
    for (const auto& g : _context->GetNamedLayoutGroups()) {
        [out addObject:[NSString stringWithUTF8String:g.name.c_str()]];
    }
    return out;
}

- (NSString*)activeLayoutGroup {
    if (!_context) return @"Default";
    return [NSString stringWithUTF8String:_context->GetActiveLayoutGroup().c_str()];
}

- (void)setActiveLayoutGroup:(NSString*)name {
    if (!_context) return;
    std::string s = name ? std::string([name UTF8String]) : std::string("Default");
    if (_context->GetActiveLayoutGroup() == s) return;
    _context->SetActiveLayoutGroup(s);
    // Each preview pane caches its background texture against the
    // previously-active path; broadcast so they invalidate before the
    // next draw.
    [[NSNotificationCenter defaultCenter]
        postNotificationName:@"XLLayoutGroupChanged" object:self];
}

- (BOOL)createLayoutGroup:(NSString*)name {
    if (!_context || !name) return NO;
    std::string s = [name UTF8String];
    return _context->AddNamedLayoutGroup(s) ? YES : NO;
}

- (BOOL)layoutMode3D {
    if (!_context) return YES;
    return _context->GetLayoutMode3D() ? YES : NO;
}

// MARK: - Layout Editor (Phase J-0, read-only)

- (NSArray<NSString*>*)modelsInActiveLayoutGroup {
    NSMutableArray<NSString*>* out = [NSMutableArray array];
    if (!_context) return out;
    for (Model* m : _context->GetModelsForActivePreview()) {
        if (!m) continue;
        // Layout editor doesn't surface submodels — they share
        // the parent's screenLocation (SubModel.h:29) and aren't
        // standalone layout entities. Matches desktop LayoutPanel
        // which never lists them either.
        if (m->GetDisplayAs() == DisplayAsType::SubModel) continue;
        [out addObject:[NSString stringWithUTF8String:m->GetName().c_str()]];
    }
    return out;
}

- (NSArray<NSDictionary<NSString*, id>*>*)modelsListSummary {
    NSMutableArray<NSDictionary<NSString*, id>*>* out = [NSMutableArray array];
    if (!_context) return out;
    for (Model* m : _context->GetModelsForActivePreview()) {
        if (!m) continue;
        if (m->GetDisplayAs() == DisplayAsType::SubModel) continue;
        if (m->GetDisplayAs() == DisplayAsType::ModelGroup) continue;
        [out addObject:@{
            @"name":               [NSString stringWithUTF8String:m->GetName().c_str()],
            @"displayAs":          [NSString stringWithUTF8String:m->GetDisplayAsString().c_str()],
            @"startChannelString": [NSString stringWithUTF8String:m->GetModelStartChannel().c_str()],
            @"firstChannel":       @(m->GetFirstChannel()),
            @"lastChannel":        @(m->GetLastChannel()),
            @"controllerName":     [NSString stringWithUTF8String:m->GetControllerName().c_str()],
            @"isFromBase":         @(m->IsFromBase() ? YES : NO),
        }];
    }
    return out;
}

// J-18 pass 3 — comma-join positional names for Model::Set
// StrandNames / SetNodeNames. Commas inside an entry are
// stripped (they'd corrupt the wire format). Empty slots stay
// empty so the index ordering is preserved.
- (std::string)joinIndexedNames:(NSArray<NSString*>*)names {
    std::string out;
    bool first = true;
    for (NSString* s in names ?: @[]) {
        std::string entry = s.UTF8String;
        entry.erase(std::remove(entry.begin(), entry.end(), ','), entry.end());
        if (!first) out += ',';
        out += entry;
        first = false;
    }
    return out;
}

// J-20 — Controller Connection sub-dictionary. Mirrors desktop's
// ModelPropertyAdapter::AddControllerProperties. Everything the
// SwiftUI Controller Connection section renders comes from this
// one dictionary, including:
//   - protocol list + current selection
//   - port number + max (gated on protocol type via ControllerCaps)
//   - smart-remote subsection (pixel protocols + caps say so)
//   - per-pixel toggle/value pairs (null pixels, brightness,
//     gamma, color order, direction, group count, zig-zag,
//     smart Ts) — each entry exposes both the `Active` flag and
//     the value plus a `supports` flag so the UI can skip
//     unavailable controls entirely.
//   - serial-only DMX channel + Speed enum
//   - PWM-only gamma + brightness
// "Most ports we expect" mirrors the desktop constant when no
// caps are available — keep the iPad in sync if that changes.
#define IPAD_MOST_CONTROLLER_PORTS 128
- (NSDictionary<NSString*, id>*)controllerConnectionFor:(Model*)m {
    if (!m) return @{};

    NSMutableDictionary<NSString*, id>* out = [NSMutableDictionary dictionary];
    ControllerCaps* caps = m->GetControllerCaps();
    std::string protocol = m->GetControllerProtocol();

    // --- Port ---
    int portMax = IPAD_MOST_CONTROLLER_PORTS;
    if (caps != nullptr && !protocol.empty()) {
        if (m->IsSerialProtocol())            portMax = caps->GetMaxSerialPort();
        else if (m->IsPixelProtocol())        portMax = caps->GetMaxPixelPort();
        else if (m->IsLEDPanelMatrixProtocol()) portMax = caps->GetMaxLEDPanelMatrixPort();
        else if (m->IsVirtualMatrixProtocol()) portMax = caps->GetMaxVirtualMatrixPort();
        else if (m->IsPWMProtocol())           portMax = caps->GetMaxPWMPort();
    }
    out[@"port"]    = @(m->GetControllerPort(1));
    out[@"portMin"] = @0;
    out[@"portMax"] = @(portMax);

    // --- Protocol list + selection ---
    std::vector<std::string> cp;
    int protoIdx = -1;
    m->GetControllerProtocols(cp, protoIdx);
    NSMutableArray<NSString*>* protoOpts = [NSMutableArray array];
    for (const auto& s : cp) {
        [protoOpts addObject:[NSString stringWithUTF8String:s.c_str()]];
    }
    out[@"protocolOptions"] = protoOpts;
    out[@"protocolIndex"]   = @(protoIdx);
    out[@"protocol"]        = [NSString stringWithUTF8String:protocol.c_str()];

    bool isPixel  = m->IsPixelProtocol();
    bool isSerial = m->IsSerialProtocol();
    bool isPWM    = m->IsPWMProtocol();
    out[@"isPixelProtocol"]  = @(isPixel  ? YES : NO);
    out[@"isSerialProtocol"] = @(isSerial ? YES : NO);
    out[@"isPWMProtocol"]    = @(isPWM    ? YES : NO);

    // --- Smart Remote (pixel + caps>0) ---
    if (isPixel) {
        int smartRemoteCount = (caps != nullptr) ? caps->GetSmartRemoteCount() : 15;
        out[@"smartRemoteCount"] = @(smartRemoteCount);
        if (smartRemoteCount > 0) {
            bool useSR = m->IsCtrlPropertySet(ControllerConnection::USE_SMART_REMOTE);
            out[@"useSmartRemote"] = @(useSR ? YES : NO);
            if (useSR) {
                if (m->GetSmartRemote() != 0) {
                    auto srTypes = m->GetSmartRemoteTypes();
                    NSMutableArray<NSString*>* typeOpts = [NSMutableArray array];
                    for (const auto& t : srTypes) {
                        [typeOpts addObject:[NSString stringWithUTF8String:t.c_str()]];
                    }
                    out[@"smartRemoteTypeOptions"] = typeOpts;
                    out[@"smartRemoteTypeIndex"]   = @(m->GetSmartRemoteTypeIndex(m->GetSmartRemoteType()));
                    out[@"smartRemoteType"]        =
                        [NSString stringWithUTF8String:m->GetSmartRemoteType().c_str()];
                }
                NSMutableArray<NSString*>* srValues = [NSMutableArray array];
                for (const auto& v : m->GetSmartRemoteValues(smartRemoteCount)) {
                    [srValues addObject:[NSString stringWithUTF8String:v.c_str()]];
                }
                out[@"smartRemoteOptions"] = srValues;
                out[@"smartRemoteIndex"]   = @(m->GetSmartRemote() - 1);
                if (m->GetNumPhysicalStrings() > 1) {
                    out[@"srMaxCascade"]      = @(m->GetSRMaxCascade());
                    out[@"srMaxCascadeMax"]   = @(smartRemoteCount);
                    out[@"srCascadeOnPort"]   = @(m->GetSRCascadeOnPort() ? YES : NO);
                }
            }
        }
    }

    // --- Serial: DMX channel + Speed ---
    if (isSerial) {
        int dmxChannelMax = (caps != nullptr) ? caps->GetMaxSerialPortChannels() : 512;
        out[@"dmxChannel"]    = @(m->GetControllerDMXChannel());
        out[@"dmxChannelMax"] = @(dmxChannelMax);
        if (protoIdx >= 0 && protoIdx < (int)cp.size() &&
            (cp[protoIdx] != "dmx" || protocol.find("DMX") != std::string::npos)) {
            std::vector<std::string> cs;
            int speedIdx = -1;
            m->GetSerialProtocolSpeeds(cp[protoIdx], cs, speedIdx);
            NSMutableArray<NSString*>* speeds = [NSMutableArray array];
            for (const auto& s : cs) {
                [speeds addObject:[NSString stringWithUTF8String:s.c_str()]];
            }
            out[@"speedOptions"] = speeds;
            out[@"speedIndex"]   = @(speedIdx);
        }
    }

    // --- PWM: gamma + brightness ---
    if (isPWM) {
        out[@"pwmGamma"]      = @((double)m->GetControllerGamma());
        out[@"pwmBrightness"] = @(m->GetControllerBrightness());
    }

    // --- Pixel: per-property toggles. `supports` keys gate
    // whether the UI renders the row at all; `active` / value
    // pairs drive the toggle + sub-field.
    if (isPixel) {
        auto supports = [&](bool defaultYes) -> NSNumber* { return @(defaultYes ? YES : NO); };
        out[@"supportsStartNulls"]   = supports(caps == nullptr || caps->SupportsPixelPortNullPixels());
        out[@"startNullsActive"]     = @(m->IsCtrlPropertySet(ControllerConnection::START_NULLS_ACTIVE) ? YES : NO);
        out[@"startNulls"]           = @(m->GetControllerStartNulls());

        out[@"supportsEndNulls"]     = supports(caps == nullptr || caps->SupportsPixelPortEndNullPixels());
        out[@"endNullsActive"]       = @(m->IsCtrlPropertySet(ControllerConnection::END_NULLS_ACTIVE) ? YES : NO);
        out[@"endNulls"]             = @(m->GetControllerEndNulls());

        out[@"supportsBrightness"]   = supports(caps == nullptr || caps->SupportsPixelPortBrightness());
        out[@"brightnessActive"]     = @(m->IsCtrlPropertySet(ControllerConnection::BRIGHTNESS_ACTIVE) ? YES : NO);
        out[@"brightness"]           = @(m->GetControllerBrightness());

        out[@"supportsGamma"]        = supports(caps == nullptr || caps->SupportsPixelPortGamma());
        out[@"gammaActive"]          = @(m->IsCtrlPropertySet(ControllerConnection::GAMMA_ACTIVE) ? YES : NO);
        out[@"gamma"]                = @((double)m->GetControllerGamma());

        out[@"supportsColorOrder"]   = supports(caps == nullptr || caps->SupportsPixelPortColourOrder());
        out[@"colorOrderActive"]     = @(m->IsCtrlPropertySet(ControllerConnection::COLOR_ORDER_ACTIVE) ? YES : NO);
        NSMutableArray<NSString*>* coOpts = [NSMutableArray array];
        for (const auto& s : Model::CONTROLLER_COLORORDER) {
            [coOpts addObject:[NSString stringWithUTF8String:s.c_str()]];
        }
        out[@"colorOrderOptions"]    = coOpts;
        std::string co = m->GetControllerColorOrder();
        int coIdx = 0;
        for (int i = 0; i < (int)Model::CONTROLLER_COLORORDER.size(); ++i) {
            if (Model::CONTROLLER_COLORORDER[i] == co) { coIdx = i; break; }
        }
        out[@"colorOrderIndex"]      = @(coIdx);

        out[@"supportsDirection"]    = supports(caps == nullptr || caps->SupportsPixelPortDirection());
        out[@"directionActive"]      = @(m->IsCtrlPropertySet(ControllerConnection::REVERSE_ACTIVE) ? YES : NO);
        out[@"directionOptions"]     = @[@"Forward", @"Reverse"];
        out[@"directionIndex"]       = @(m->GetControllerReverse());

        out[@"supportsGroupCount"]   = supports(caps == nullptr || caps->SupportsPixelPortGrouping());
        out[@"groupCountActive"]     = @(m->IsCtrlPropertySet(ControllerConnection::GROUP_COUNT_ACTIVE) ? YES : NO);
        out[@"groupCount"]           = @(m->GetControllerGroupCount());

        out[@"supportsZigZag"]       = supports(caps == nullptr || caps->SupportsPixelZigZag());
        out[@"zigZagActive"]         = @(m->IsCtrlPropertySet(ControllerConnection::ZIG_ZAG_ACTIVE) ? YES : NO);
        out[@"zigZag"]               = @(m->GetControllerZigZag());

        out[@"supportsSmartTs"]      = supports(caps == nullptr || caps->SupportsTs());
        out[@"smartTsActive"]        = @(m->IsCtrlPropertySet(ControllerConnection::TS_ACTIVE) ? YES : NO);
        out[@"smartTs"]              = @(m->GetSmartTs());
    }

    return out;
}

// J-18 — read-only popup summaries. Surface counts + entries so
// SwiftUI can render a row per category and a tap-to-view sheet.
- (NSDictionary<NSString*, id>*)extrasFor:(Model*)m {
    if (!m) return @{};

    NSMutableArray<NSString*>* faceNames = [NSMutableArray array];
    for (const auto& [faceName, _attrs] : m->GetFaceInfo()) {
        [faceNames addObject:[NSString stringWithUTF8String:faceName.c_str()]];
    }

    NSMutableArray<NSString*>* stateNames = [NSMutableArray array];
    for (const auto& [stateName, _attrs] : m->GetStateInfo()) {
        [stateNames addObject:[NSString stringWithUTF8String:stateName.c_str()]];
    }

    NSMutableArray<NSString*>* submodelNames = [NSMutableArray array];
    for (Model* sm : m->GetSubModels()) {
        if (!sm) continue;
        [submodelNames addObject:[NSString stringWithUTF8String:sm->GetName().c_str()]];
    }

    NSMutableArray<NSString*>* aliasNames = [NSMutableArray array];
    for (const std::string& a : m->GetAliases()) {
        [aliasNames addObject:[NSString stringWithUTF8String:a.c_str()]];
    }

    NSMutableArray<NSString*>* strandNames = [NSMutableArray array];
    int numStrands = m->GetNumStrands();
    for (int i = 0; i < numStrands; ++i) {
        std::string s = m->GetStrandName(i, true);
        [strandNames addObject:[NSString stringWithUTF8String:s.c_str()]];
    }

    NSMutableArray<NSString*>* nodeNames = [NSMutableArray array];
    size_t nodeCount = m->GetNodeCount();
    for (size_t i = 0; i < nodeCount; ++i) {
        std::string s = m->GetNodeName(i, true);
        [nodeNames addObject:[NSString stringWithUTF8String:s.c_str()]];
    }

    NSMutableArray<NSString*>* inGroups = [NSMutableArray array];
    if (_context && _context->HasModelManager()) {
        auto groups = _context->GetModelManager().GetGroupsContainingModel(m);
        for (const std::string& g : groups) {
            [inGroups addObject:[NSString stringWithUTF8String:g.c_str()]];
        }
    }

    return @{
        @"faceCount":       @(faceNames.count),
        @"faceNames":       faceNames,
        @"stateCount":      @(stateNames.count),
        @"stateNames":      stateNames,
        @"submodelCount":   @(submodelNames.count),
        @"submodelNames":   submodelNames,
        @"aliasCount":      @(aliasNames.count),
        @"aliasNames":      aliasNames,
        @"hasDimmingCurve": @(m->GetDimmingCurve() != nullptr ? YES : NO),
        @"strandNames":     strandNames,
        @"nodeNames":       nodeNames,
        @"inModelGroups":   inGroups,
    };
}

- (nullable NSDictionary<NSString*, id>*)modelLayoutSummary:(NSString*)name {
    if (!_context || !_context->HasModelManager() || !name) return nil;
    Model* m = _context->GetModelManager()[std::string([name UTF8String])];
    if (!m) return nil;

    auto& loc = m->GetModelScreenLocation();
    glm::vec3 rot = loc.GetRotation();

    NSString* layoutGroup = [NSString stringWithUTF8String:m->GetLayoutGroup().c_str()];
    NSString* controllerName = [NSString stringWithUTF8String:m->GetControllerName().c_str()];
    NSString* displayAs = [NSString stringWithUTF8String:m->GetDisplayAsString().c_str()];

    // J-19 — controller picker options. Mirrors desktop's
    // ModelPropertyAdapter::AddProperties controller-list build:
    // "Use Start Channel" + "No Controller" + every auto-layout
    // controller name. Empty controllerName == "Use Start
    // Channel"; "No Controller" is its own literal value.
    NSMutableArray<NSString*>* controllerOptions = [NSMutableArray array];
    [controllerOptions addObject:@"Use Start Channel"];
    [controllerOptions addObject:@"No Controller"];
    for (const auto& n : _context->GetOutputManager().GetAutoLayoutControllerNames()) {
        [controllerOptions addObject:[NSString stringWithUTF8String:n.c_str()]];
    }
    NSString* controllerSelection;
    if (m->GetControllerName().empty()) {
        controllerSelection = @"Use Start Channel";
    } else {
        // Mirrors the desktop value verbatim — both "No Controller"
        // and named controllers round-trip through GetControllerName.
        controllerSelection = controllerName;
    }
    bool startChannelEditable = m->GetControllerName().empty();

    // J-20 — Low Definition Factor: applies to a subset of model
    // types (`SupportsLowDefinitionRender()`). When unsupported,
    // SwiftUI doesn't render the row at all.
    bool supportsLowDef = m->SupportsLowDefinitionRender();
    int lowDefFactor    = m->GetLowDefFactor();

    // J-20 — Shadow Model For: target-model picker (any non-group
    // model other than self). Empty string means "not a shadow".
    NSMutableArray<NSString*>* shadowOptions = [NSMutableArray array];
    [shadowOptions addObject:@""];  // "(none)" sentinel
    NSString* shadowSelection = [NSString stringWithUTF8String:m->GetShadowModelFor().c_str()];
    for (const auto& [otherName, other] : _context->GetModelManager().GetModels()) {
        if (!other || other == m) continue;
        if (other->GetDisplayAs() == DisplayAsType::ModelGroup) continue;
        [shadowOptions addObject:[NSString stringWithUTF8String:otherName.c_str()]];
    }

    // J-20.2 — DisableUnusedProperties parity. Each model type's
    // adapter on the desktop disables specific property-grid
    // entries that don't apply. iPad surfaces a `disabledKeys`
    // set so the SwiftUI side renders those rows grayed out / non-
    // interactive without having to enumerate per-type cases in
    // the UI layer. Keys are the iPad-bridge names, not the
    // desktop wxPropertyGrid IDs.
    NSMutableArray<NSString*>* disabledKeys = [NSMutableArray array];
    DisplayAsType dispAs = m->GetDisplayAs();
    if (dispAs == DisplayAsType::Image || dispAs == DisplayAsType::Label) {
        // Image / Label: no popup data + no per-string-properties.
        [disabledKeys addObjectsFromArray:@[@"faces", @"states", @"submodels",
                                            @"strands", @"nodes",
                                            @"stringType", @"stringColor",
                                            @"rgbwHandlingIndex",
                                            @"dimmingCurve"]];
    }
    if (dispAs == DisplayAsType::Image) {
        // ImagePropertyAdapter::DisableUnusedProperties also
        // disables ModelPixelSize.
        [disabledKeys addObject:@"pixelSize"];
    }
    if (dispAs == DisplayAsType::Label) {
        // LabelPropertyAdapter::DisableUnusedProperties disables
        // pixel style + black transparency in addition to the
        // shared list.
        [disabledKeys addObjectsFromArray:@[@"pixelStyle", @"blackTransparency"]];
    }
    if (dispAs == DisplayAsType::ChannelBlock) {
        // ChannelBlock has no real "string type" because each
        // channel is its own output.
        [disabledKeys addObjectsFromArray:@[@"stringType", @"stringColor", @"rgbwHandlingIndex"]];
    }

    // J-19 — Size/Location surface depends on the screen-location
    // class. Boxed = X/Y/Z + ScaleX/Y/Z + RotateX/Y/Z. TwoPoint =
    // World + X1/Y1/Z1 + X2/Y2/Z2. ThreePoint = TwoPoint +
    // ModelHeight (+ Shear if supported) + RotateX. SwiftUI picks
    // the right field set based on `screenLocationKind`.
    auto& scrLoc = m->GetModelScreenLocation();
    NSString* screenLocKind = @"boxed";
    NSDictionary* screenLocFields = @{};
    if (auto* boxed = dynamic_cast<BoxedScreenLocation*>(&scrLoc)) {
        screenLocKind = @"boxed";
        screenLocFields = @{
            @"worldX":  @((double)boxed->GetWorldPos_X()),
            @"worldY":  @((double)boxed->GetWorldPos_Y()),
            @"worldZ":  @((double)boxed->GetWorldPos_Z()),
            @"scaleX":  @((double)boxed->GetScaleX()),
            @"scaleY":  @((double)boxed->GetScaleY()),
            @"scaleZ":  @((double)boxed->GetScaleZ()),
            @"rotateX": @((double)boxed->GetRotateX()),
            @"rotateY": @((double)boxed->GetRotateY()),
            @"rotateZ": @((double)boxed->GetRotateZ()),
            @"supportsZScaling": @(boxed->GetSupportsZScaling() ? YES : NO),
        };
    } else if (auto* three = dynamic_cast<ThreePointScreenLocation*>(&scrLoc)) {
        screenLocKind = @"threePoint";
        screenLocFields = @{
            @"worldX":   @((double)three->GetWorldPos_X()),
            @"worldY":   @((double)three->GetWorldPos_Y()),
            @"worldZ":   @((double)three->GetWorldPos_Z()),
            @"x1":       @((double)three->GetWorldPos_X()),
            @"y1":       @((double)three->GetWorldPos_Y()),
            @"z1":       @((double)three->GetWorldPos_Z()),
            @"x2":       @((double)(three->GetX2() + three->GetWorldPos_X())),
            @"y2":       @((double)(three->GetY2() + three->GetWorldPos_Y())),
            @"z2":       @((double)(three->GetZ2() + three->GetWorldPos_Z())),
            @"modelHeight":  @((double)three->GetHeight()),
            @"supportsShear":@(three->GetSupportsShear() ? YES : NO),
            @"modelShear":   @((double)three->GetShear()),
            @"rotateX":      @((double)three->GetRotateX()),
        };
    } else if (auto* two = dynamic_cast<TwoPointScreenLocation*>(&scrLoc)) {
        screenLocKind = @"twoPoint";
        screenLocFields = @{
            @"worldX":  @((double)two->GetWorldPos_X()),
            @"worldY":  @((double)two->GetWorldPos_Y()),
            @"worldZ":  @((double)two->GetWorldPos_Z()),
            @"x1":      @((double)two->GetWorldPos_X()),
            @"y1":      @((double)two->GetWorldPos_Y()),
            @"z1":      @((double)two->GetWorldPos_Z()),
            @"x2":      @((double)(two->GetX2() + two->GetWorldPos_X())),
            @"y2":      @((double)(two->GetY2() + two->GetWorldPos_Y())),
            @"z2":      @((double)(two->GetZ2() + two->GetWorldPos_Z())),
        };
    } else {
        // PolyPoint and other custom screen-location classes: just
        // surface world position. Per-vertex editing already lives
        // in the canvas gesture path.
        screenLocKind = @"other";
        screenLocFields = @{
            @"worldX": @((double)scrLoc.GetWorldPos_X()),
            @"worldY": @((double)scrLoc.GetWorldPos_Y()),
            @"worldZ": @((double)scrLoc.GetWorldPos_Z()),
        };
    }

    // J-19 — String Properties dynamic surface (mirrors desktop's
    // ModelPropertyAdapter::AddProperties): which sub-controls
    // appear under String Type depends on the type itself.
    std::string stringType = m->GetStringType();
    NSString* stringColorMode = @"none";    // single | superstring | none
    NSString* stringColorHex  = @"#FF0000";  // surfaced when mode == single
    int superStringCount      = 0;
    NSMutableArray<NSString*>* superStringColours = [NSMutableArray array];
    if (stringType == "Single Color" ||
        stringType == "Single Color Intensity" ||
        stringType == "Node Single Color") {
        stringColorMode = @"single";
        xlColor c(0, 0, 0);
        bool resolved = false;
        if (stringType == "Single Color Red")  { c = xlColor(255, 0, 0); resolved = true; }
        // Desktop derives the displayed colour from the type when
        // it's a fixed-name "Single Color X" string and from
        // GetCustomColor() otherwise — same logic here.
        if (!resolved) {
            c = m->GetCustomColor();
        }
        stringColorHex = [NSString stringWithFormat:@"#%02X%02X%02X", c.red, c.green, c.blue];
    } else if (stringType == "Superstring") {
        stringColorMode = @"superstring";
        if (m->GetSuperStringColours().empty()) {
            m->InitSuperStringColours();
        }
        const auto& ssc = m->GetSuperStringColours();
        superStringCount = (int)ssc.size();
        for (const auto& c : ssc) {
            [superStringColours addObject:
                [NSString stringWithFormat:@"#%02X%02X%02X", c.red, c.green, c.blue]];
        }
    }
    NSArray<NSString*>* rgbwHandlingOptions =
        @[@"R=G=B -> W", @"RGB Only", @"White Only", @"Advanced", @"White On All"];
    int rgbwHandlingIndex = m->GetRGBWHandlingType();
    if (rgbwHandlingIndex < 0) rgbwHandlingIndex = 0;
    if (rgbwHandlingIndex >= (int)rgbwHandlingOptions.count) {
        rgbwHandlingIndex = 0;
    }
    bool rgbwHandlingEnabled = !(m->HasSingleChannel(stringType) ||
                                  m->GetNodeChannelCount(stringType) < 4);

    // J-8 (desktop-order property sections) — pixel-style options
    // mirror desktop's PIXEL_STYLES_VALUES; index matches
    // PIXEL_STYLE enum so we can round-trip without a lookup.
    NSArray<NSString*>* pixelStyles =
        @[@"Square", @"Smooth", @"Solid Circle", @"Blended Circle"];

    // String-type list mirrors desktop's NODE_TYPE_VALUES in
    // PropertyGridHelpers.cpp. Order is significant — the index
    // is the property-grid representation.
    NSArray<NSString*>* stringTypes = @[
        @"RGB Nodes", @"RBG Nodes", @"GBR Nodes", @"GRB Nodes",
        @"BRG Nodes", @"BGR Nodes", @"Node Single Color",
        @"3 Channel RGB", @"4 Channel RGBW", @"4 Channel WRGB",
        @"Strobes", @"Single Color", @"Single Color Intensity",
        @"Superstring",
        @"WRGB Nodes", @"WRBG Nodes", @"WGBR Nodes", @"WGRB Nodes",
        @"WBRG Nodes", @"WBGR Nodes",
        @"RGBW Nodes", @"RBGW Nodes", @"GBRW Nodes", @"GRBW Nodes",
        @"BRGW Nodes", @"BGRW Nodes", @"RGBWW Nodes",
    ];

    // J-18 — Start Channel / Indiv Start Chans / Model Chain.
    // Per-string indiv channels: when the toggle is off, show
    // only the model-wide start channel; when on, expose one
    // entry per string. Lazy-fill to numStrings so the UI can
    // always assume the array length matches.
    NSMutableArray<NSString*>* indivChannels = [NSMutableArray array];
    int numStrings = m->GetNumStrings();
    if (m->HasIndividualStartChannels()) {
        for (int i = 0; i < numStrings; ++i) {
            std::string ch = m->GetIndividualStartChannel(i);
            if (ch.empty()) ch = m->ComputeStringStartChannel(i);
            [indivChannels addObject:[NSString stringWithUTF8String:ch.c_str()]];
        }
    }
    // Model Chain options: every OTHER model on the same
    // controller + protocol + port, plus the always-present
    // "Beginning" sentinel. Only meaningful when controller +
    // protocol + port are set; otherwise the picker is hidden.
    NSMutableArray<NSString*>* chainOpts = [NSMutableArray array];
    [chainOpts addObject:@"Beginning"];
    bool chainApplicable = !m->GetControllerName().empty() &&
                           !m->GetControllerProtocol().empty() &&
                           m->GetControllerPort() != 0;
    if (chainApplicable) {
        const std::string myCtrl = m->GetControllerName();
        const std::string myProto = m->GetControllerProtocol();
        const int myPort = m->GetControllerPort();
        for (const auto& [otherName, other] : _context->GetModelManager().GetModels()) {
            if (!other || other == m) continue;
            if (other->GetDisplayAs() == DisplayAsType::ModelGroup) continue;
            if (other->GetDisplayAs() == DisplayAsType::SubModel) continue;
            if (other->GetControllerName() != myCtrl) continue;
            if (other->GetControllerProtocol() != myProto) continue;
            if (other->GetControllerPort() != myPort) continue;
            [chainOpts addObject:[NSString stringWithUTF8String:otherName.c_str()]];
        }
    }
    std::string mc = m->GetModelChain();
    if (mc.empty()) mc = "Beginning";

    // Real-world dimension readouts (ruler-calibrated). Desktop
    // ScreenLocationProperties RealWidth/RealHeight/RealDepth via
    // RulerObject::Measure. Only meaningful when a Ruler view-object
    // has been placed + calibrated; otherwise omit so SwiftUI hides
    // the rows.
    NSDictionary* realDimensions = nil;
    if (RulerObject::GetRuler() != nullptr) {
        float rw = scrLoc.GetRealWidth();
        float rh = scrLoc.GetRealHeight();
        float rd = scrLoc.GetRealDepth();
        if (rw > 0 || rh > 0) {
            realDimensions = @{
                @"width":  @((double)rw),
                @"height": @((double)rh),
                @"depth":  @((double)rd),
                @"units":  [NSString stringWithUTF8String:RulerObject::GetUnitDescription().c_str()],
            };
        }
    }

    NSMutableDictionary* summary = [@{
        @"name":                 [NSString stringWithUTF8String:m->GetName().c_str()],
        @"displayAs":            displayAs,
        @"centerX":              @((double)loc.GetHcenterPos()),
        @"centerY":              @((double)loc.GetVcenterPos()),
        @"centerZ":              @((double)loc.GetDcenterPos()),
        @"width":                @((double)loc.GetMWidth()),
        @"height":               @((double)loc.GetMHeight()),
        @"depth":                @((double)loc.GetMDepth()),
        @"rotateX":              @((double)rot.x),
        @"rotateY":              @((double)rot.y),
        @"rotateZ":              @((double)rot.z),
        @"locked":               @(loc.IsLocked() ? YES : NO),
        @"layoutGroup":          layoutGroup,
        @"controllerName":       controllerName,
        // J-19 — controller picker (replaces the free-text field).
        @"controllerSelection":  controllerSelection,
        @"controllerOptions":    controllerOptions,
        @"startChannelEditable": @(startChannelEditable ? YES : NO),
        // J-20 — model-header extras.
        @"supportsLowDefinition": @(supportsLowDef ? YES : NO),
        @"lowDefinitionFactor":   @(lowDefFactor),
        @"shadowModelFor":        shadowSelection,
        @"shadowModelOptions":    shadowOptions,
        @"disabledKeys":          disabledKeys,
        @"startChannel":         @((unsigned long long)m->GetFirstChannel()),
        @"endChannel":           @((unsigned long long)m->GetLastChannel()),
        @"stringCount":          @(m->GetNumPhysicalStrings()),
        @"nodeCount":            @((unsigned long long)m->GetNodeCount()),
        // J-18 — Controller Connection editable fields.
        @"modelStartChannel":          [NSString stringWithUTF8String:m->GetModelStartChannel().c_str()],
        @"hasIndividualStartChannels": @(m->HasIndividualStartChannels() ? YES : NO),
        @"individualStartChannels":    indivChannels,
        @"hasMultipleStrings":         @(!Model::HasOneString(m->GetDisplayAs()) ? YES : NO),
        @"numStrings":                 @(numStrings),
        @"modelChain":                 [NSString stringWithUTF8String:mc.c_str()],
        @"modelChainOptions":          chainOpts,
        @"modelChainApplicable":       @(chainApplicable ? YES : NO),
        // J-18 — read-only summaries for the popup-dialog
        // surfaces (Faces / States / SubModels / Aliases /
        // Strand-Node Names / Dimming / In Model Groups). Each
        // exposes a count + the names list so SwiftUI can show
        // "N defined" + a tap-to-view list.
        @"extras":                     [self extrasFor:m],
        // J-8 — Appearance section.
        @"active":               @(m->IsActive() ? YES : NO),
        @"pixelSize":            @(m->GetPixelSize()),
        @"pixelStyle":           @((int)m->GetPixelStyle()),
        @"pixelStyleOptions":    pixelStyles,
        @"transparency":         @(m->GetTransparency()),
        @"blackTransparency":    @(m->GetBlackTransparency()),
        @"tagColor":             [NSString stringWithUTF8String:m->GetTagColourAsString().c_str()],
        // J-8 — String Properties section.
        @"stringType":           [NSString stringWithUTF8String:m->GetStringType().c_str()],
        @"stringTypeOptions":    stringTypes,
        // J-19 — dynamic per-type fields (mirrors desktop):
        // mode tells SwiftUI which controls to render under
        // String Type; 'single' = one Color row, 'superstring'
        // = colour count + per-index colour rows, 'none' =
        // disabled placeholder.
        @"stringColorMode":      stringColorMode,
        @"stringColor":          stringColorHex,
        @"superStringCount":     @(superStringCount),
        @"superStringColours":   superStringColours,
        @"rgbwHandlingOptions":  rgbwHandlingOptions,
        @"rgbwHandlingIndex":    @(rgbwHandlingIndex),
        @"rgbwHandlingEnabled":  @(rgbwHandlingEnabled ? YES : NO),
        // J-8 — top-of-pane: free-form description.
        @"description":          [NSString stringWithUTF8String:m->description.c_str()],
        // J-19 — Size/Location surface keyed by screen-location
        // class. SwiftUI picks the right field set based on
        // `screenLocationKind`.
        @"screenLocationKind":   screenLocKind,
        @"screenLocationFields": screenLocFields,
        // J-20 — Controller Connection sub-dictionary. See
        // controllerConnectionFor: above for the key shape.
        @"controllerConnection": [self controllerConnectionFor:m],
    } mutableCopy];
    if (realDimensions) summary[@"realDimensions"] = realDimensions;
    return summary;
}

- (BOOL)setLayoutModelProperty:(NSString*)name
                           key:(NSString*)key
                         value:(id)value {
    if (!_context || !name || !key) return NO;
    // Abort any in-flight render before touching Model state — the
    // render workers hold raw Model* references and racing them
    // produces hard-to-reproduce crashes (matches desktop
    // LayoutPanel.cpp's AbortRender() guard before every property
    // edit). Cheap when no render is active.
    _context->AbortRender(5000);
    Model* m = _context->GetModelManager()[std::string([name UTF8String])];
    if (!m) return NO;

    auto& loc = m->GetModelScreenLocation();
    std::string keyStr = [key UTF8String];

    auto asDouble = ^double(BOOL* ok){
        if ([value isKindOfClass:[NSNumber class]]) {
            *ok = YES;
            return [(NSNumber*)value doubleValue];
        }
        *ok = NO;
        return 0.0;
    };
    auto asBool = ^BOOL(BOOL* ok){
        if ([value isKindOfClass:[NSNumber class]]) {
            *ok = YES;
            return [(NSNumber*)value boolValue];
        }
        *ok = NO;
        return NO;
    };
    auto asString = ^NSString*(BOOL* ok){
        if ([value isKindOfClass:[NSString class]]) {
            *ok = YES;
            return (NSString*)value;
        }
        *ok = NO;
        return nil;
    };

    BOOL changed = NO;
    BOOL typeOk = NO;

    if (keyStr == "centerX") {
        double v = asDouble(&typeOk);
        if (!typeOk) return NO;
        if ((float)v != loc.GetHcenterPos()) { m->SetHcenterPos((float)v); changed = YES; }
    } else if (keyStr == "centerY") {
        double v = asDouble(&typeOk);
        if (!typeOk) return NO;
        if ((float)v != loc.GetVcenterPos()) { m->SetVcenterPos((float)v); changed = YES; }
    } else if (keyStr == "centerZ") {
        double v = asDouble(&typeOk);
        if (!typeOk) return NO;
        if ((float)v != loc.GetDcenterPos()) { m->SetDcenterPos((float)v); changed = YES; }
    }
    // J-19 — Boxed-class size/location fields. Each maps to the
    // matching ScreenLocationPropertyHelper case on desktop.
    else if (keyStr == "worldX") {
        double v = asDouble(&typeOk); if (!typeOk) return NO;
        if ((float)v != loc.GetWorldPos_X()) { loc.SetWorldPos_X((float)v); changed = YES; }
    } else if (keyStr == "worldY") {
        double v = asDouble(&typeOk); if (!typeOk) return NO;
        if ((float)v != loc.GetWorldPos_Y()) { loc.SetWorldPos_Y((float)v); changed = YES; }
    } else if (keyStr == "worldZ") {
        double v = asDouble(&typeOk); if (!typeOk) return NO;
        if ((float)v != loc.GetWorldPos_Z()) { loc.SetWorldPos_Z((float)v); changed = YES; }
    } else if (keyStr == "scaleX" || keyStr == "scaleY" || keyStr == "scaleZ") {
        double v = asDouble(&typeOk); if (!typeOk) return NO;
        auto* boxed = dynamic_cast<BoxedScreenLocation*>(&loc);
        if (boxed) {
            if (keyStr == "scaleX" && (float)v != boxed->GetScaleX()) { boxed->SetScaleX((float)v); changed = YES; }
            else if (keyStr == "scaleY" && (float)v != boxed->GetScaleY()) { boxed->SetScaleY((float)v); changed = YES; }
            else if (keyStr == "scaleZ" && (float)v != boxed->GetScaleZ()) { boxed->SetScaleZ((float)v); changed = YES; }
        }
    }
    // J-19 — Two/Three-point endpoint setters. X1/Y1/Z1 moves
    // the world anchor and back-shifts X2/Y2/Z2 to keep the
    // far endpoint stationary (same logic as desktop's
    // ScreenLocationPropertyHelper::OnPropertyGridChange).
    // X2/Y2/Z2 reposition the far endpoint directly.
    else if (keyStr == "x1" || keyStr == "y1" || keyStr == "z1") {
        double v = asDouble(&typeOk); if (!typeOk) return NO;
        auto* two = dynamic_cast<TwoPointScreenLocation*>(&loc);
        if (two) {
            if (keyStr == "x1") {
                float oldWorld = two->GetWorldPos_X();
                two->SetWorldPos_X((float)v);
                two->SetX2(two->GetX2() + oldWorld - (float)v);
            } else if (keyStr == "y1") {
                float oldWorld = two->GetWorldPos_Y();
                two->SetWorldPos_Y((float)v);
                two->SetY2(two->GetY2() + oldWorld - (float)v);
            } else {
                float oldWorld = two->GetWorldPos_Z();
                two->SetWorldPos_Z((float)v);
                two->SetZ2(two->GetZ2() + oldWorld - (float)v);
            }
            changed = YES;
        }
    } else if (keyStr == "x2" || keyStr == "y2" || keyStr == "z2") {
        double v = asDouble(&typeOk); if (!typeOk) return NO;
        auto* two = dynamic_cast<TwoPointScreenLocation*>(&loc);
        if (two) {
            if (keyStr == "x2")      two->SetX2((float)v - two->GetWorldPos_X());
            else if (keyStr == "y2") two->SetY2((float)v - two->GetWorldPos_Y());
            else                     two->SetZ2((float)v - two->GetWorldPos_Z());
            changed = YES;
        }
    } else if (keyStr == "modelHeight") {
        double v = asDouble(&typeOk); if (!typeOk) return NO;
        auto* three = dynamic_cast<ThreePointScreenLocation*>(&loc);
        if (three) {
            // Desktop clamps |height| ≥ 0.01 — same here.
            float h = (float)v;
            if (std::abs(h) < 0.01f) h = h < 0 ? -0.01f : 0.01f;
            if (three->GetHeight() != h) { three->SetHeight(h); changed = YES; }
        }
    } else if (keyStr == "modelShear") {
        double v = asDouble(&typeOk); if (!typeOk) return NO;
        auto* three = dynamic_cast<ThreePointScreenLocation*>(&loc);
        if (three && three->GetSupportsShear() && three->GetShear() != (float)v) {
            three->SetShear((float)v); changed = YES;
        }
    } else if (keyStr == "width") {
        double v = asDouble(&typeOk);
        if (!typeOk) return NO;
        if ((float)v != loc.GetMWidth()) { m->SetWidth((float)v); changed = YES; }
    } else if (keyStr == "height") {
        double v = asDouble(&typeOk);
        if (!typeOk) return NO;
        if ((float)v != loc.GetMHeight()) { m->SetHeight((float)v); changed = YES; }
    } else if (keyStr == "depth") {
        double v = asDouble(&typeOk);
        if (!typeOk) return NO;
        if ((float)v != loc.GetMDepth()) { m->SetDepth((float)v); changed = YES; }
    } else if (keyStr == "rotateX") {
        double v = asDouble(&typeOk);
        if (!typeOk) return NO;
        if ((float)v != loc.GetRotation().x) { loc.SetRotateX((float)v); changed = YES; }
    } else if (keyStr == "rotateY") {
        double v = asDouble(&typeOk);
        if (!typeOk) return NO;
        if ((float)v != loc.GetRotation().y) { loc.SetRotateY((float)v); changed = YES; }
    } else if (keyStr == "rotateZ") {
        double v = asDouble(&typeOk);
        if (!typeOk) return NO;
        if ((float)v != loc.GetRotation().z) { loc.SetRotateZ((float)v); changed = YES; }
    } else if (keyStr == "locked") {
        BOOL v = asBool(&typeOk);
        if (!typeOk) return NO;
        if ((v ? true : false) != loc.IsLocked()) { loc.SetLocked(v ? true : false); changed = YES; }
    } else if (keyStr == "layoutGroup") {
        NSString* s = asString(&typeOk);
        if (!typeOk) return NO;
        std::string newGroup = [s UTF8String];
        if (newGroup != m->GetLayoutGroup()) { m->SetLayoutGroup(newGroup); changed = YES; }
    } else if (keyStr == "controllerName") {
        NSString* s = asString(&typeOk);
        if (!typeOk) return NO;
        std::string newCtrl = [s UTF8String];
        if (newCtrl != m->GetControllerName()) { m->SetControllerName(newCtrl); changed = YES; }
    } else if (keyStr == "lowDefinitionFactor") {
        // J-20 — only set on models that support it; otherwise
        // the row isn't rendered so we shouldn't get here.
        double v = asDouble(&typeOk); if (!typeOk) return NO;
        int iv = (int)v;
        if (iv != m->GetLowDefFactor() && m->SupportsLowDefinitionRender()) {
            m->SetLowDefFactor(iv); changed = YES;
        }
    } else if (keyStr == "shadowModelFor") {
        NSString* s = asString(&typeOk); if (!typeOk) return NO;
        std::string newShadow = s.UTF8String;
        if (newShadow != m->GetShadowModelFor()) {
            m->SetShadowModelFor(newShadow); changed = YES;
        }
    } else if (keyStr == "controllerSelection") {
        // J-19 — picker writes back as Use Start Channel ("") /
        // No Controller (literal) / a named auto-layout controller.
        NSString* s = asString(&typeOk);
        if (!typeOk) return NO;
        std::string sel = [s UTF8String];
        std::string newCtrl = (sel == "Use Start Channel") ? std::string("") : sel;
        if (newCtrl != m->GetControllerName()) {
            m->SetControllerName(newCtrl);
            changed = YES;
        }
    }
    // J-8 (Appearance / String Properties) — new keys exposed by
    // the desktop-order property pane reorganization.
    else if (keyStr == "active") {
        BOOL v = asBool(&typeOk);
        if (!typeOk) return NO;
        if ((v ? true : false) != m->IsActive()) { m->SetActive(v ? true : false); changed = YES; }
    } else if (keyStr == "pixelSize") {
        double v = asDouble(&typeOk);
        if (!typeOk) return NO;
        int iv = (int)v;
        if (iv != m->GetPixelSize()) { m->SetPixelSize(iv); changed = YES; }
    } else if (keyStr == "pixelStyle") {
        double v = asDouble(&typeOk);
        if (!typeOk) return NO;
        int idx = (int)v;
        if (idx < 0) idx = 0;
        if (idx > 3) idx = 3;
        auto newStyle = static_cast<Model::PIXEL_STYLE>(idx);
        if (newStyle != m->GetPixelStyle()) { m->SetPixelStyle(newStyle); changed = YES; }
    } else if (keyStr == "transparency") {
        double v = asDouble(&typeOk);
        if (!typeOk) return NO;
        int iv = (int)v;
        if (iv != m->GetTransparency()) { m->SetTransparency(iv); changed = YES; }
    } else if (keyStr == "blackTransparency") {
        double v = asDouble(&typeOk);
        if (!typeOk) return NO;
        int iv = (int)v;
        if (iv != m->GetBlackTransparency()) { m->SetBlackTransparency(iv); changed = YES; }
    } else if (keyStr == "tagColor") {
        NSString* s = asString(&typeOk);
        if (!typeOk) return NO;
        std::string newCol = s.UTF8String;
        if (newCol != m->GetTagColourAsString()) { m->SetTagColourAsString(newCol); changed = YES; }
    } else if (keyStr == "stringType") {
        NSString* s = asString(&typeOk);
        if (!typeOk) return NO;
        std::string newType = s.UTF8String;
        if (newType != m->GetStringType()) { m->SetStringType(newType); changed = YES; }
    } else if (keyStr == "stringColor") {
        // J-19 — Single-color and "Custom" string types share the
        // model's customColor. Hex round-trip via xlColor's
        // string constructor.
        NSString* s = asString(&typeOk);
        if (!typeOk) return NO;
        std::string hex = s.UTF8String;
        xlColor c(hex);
        std::string newStr;
        char buf[8];
        std::snprintf(buf, sizeof(buf), "#%02X%02X%02X", c.red, c.green, c.blue);
        newStr = buf;
        m->SetCustomColor(newStr);
        changed = YES;
    } else if (keyStr == "superStringCount") {
        double v = asDouble(&typeOk);
        if (!typeOk) return NO;
        int count = (int)v;
        if (count < 1) count = 1;
        if (count > 32) count = 32;
        if (count != (int)m->GetSuperStringColours().size()) {
            m->SetSuperStringColours(count);
            changed = YES;
        }
    } else if (keyStr.starts_with("superStringColour")) {
        // J-21 — per-index Superstring colour set. Key is
        // "superStringColourN" (0-based). Routes through the
        // model's `SetSuperStringColour(int, xlColor)` mutator
        // (matches desktop ModelPropertyAdapter.cpp:1517).
        std::string idxStr = keyStr.substr(std::string("superStringColour").size());
        char* end = nullptr;
        long idx = std::strtol(idxStr.c_str(), &end, 10);
        if (end == idxStr.c_str()) return NO;
        NSString* s = asString(&typeOk);
        if (!typeOk) return NO;
        if (idx < 0 || idx >= (long)m->GetSuperStringColours().size()) return NO;
        xlColor newC(std::string(s.UTF8String));
        if (newC != m->GetSuperStringColours()[idx]) {
            m->SetSuperStringColour((int)idx, newC);
            changed = YES;
        }
    } else if (keyStr == "rgbwHandlingIndex") {
        double v = asDouble(&typeOk);
        if (!typeOk) return NO;
        int idx = (int)v;
        NSArray<NSString*>* opts =
            @[@"R=G=B -> W", @"RGB Only", @"White Only", @"Advanced", @"White On All"];
        if (idx < 0 || idx >= (int)opts.count) return NO;
        std::string newHandling = opts[idx].UTF8String;
        if (newHandling != m->GetRGBWHandling()) {
            m->SetRGBWHandling(newHandling);
            changed = YES;
        }
    } else if (keyStr == "description") {
        NSString* s = asString(&typeOk);
        if (!typeOk) return NO;
        std::string newDesc = s.UTF8String;
        if (newDesc != m->GetDescription()) { m->SetDescription(newDesc); changed = YES; }
    }
    // J-18 — Controller Connection fields.
    else if (keyStr == "modelStartChannel") {
        NSString* s = asString(&typeOk);
        if (!typeOk) return NO;
        std::string newCh = s.UTF8String;
        if (newCh != m->GetModelStartChannel()) {
            m->SetStartChannel(newCh);
            changed = YES;
        }
    } else if (keyStr == "hasIndividualStartChannels") {
        BOOL v = asBool(&typeOk);
        if (!typeOk) return NO;
        bool desired = v ? true : false;
        if (desired != m->HasIndividualStartChannels()) {
            m->SetHasIndividualStartChannels(desired);
            if (desired) {
                // Match desktop: pre-fill any missing per-string
                // entries via `ComputeStringStartChannel` so the
                // UI has something sensible to show / edit.
                int c = m->GetNumStrings();
                while ((int)m->IndivStartChannelCount() < c) {
                    m->AddIndivStartChannel(m->ComputeStringStartChannel(m->IndivStartChannelCount()));
                }
                while ((int)m->IndivStartChannelCount() > c) {
                    m->PopIndivStartChannel();
                }
            } else {
                m->ClearIndividualStartChannels();
            }
            changed = YES;
        }
    } else if (keyStr.rfind("individualStartChannel", 0) == 0) {
        // keyStr = "individualStartChannel<N>" where N is 0..numStrings-1.
        NSString* s = asString(&typeOk);
        if (!typeOk) return NO;
        std::string idxStr = keyStr.substr(strlen("individualStartChannel"));
        int idx = (int)std::strtol(idxStr.c_str(), nullptr, 10);
        if (idx < 0 || idx >= (int)m->IndivStartChannelCount()) return NO;
        std::string newCh = s.UTF8String;
        if (newCh != m->GetIndividualStartChannel(idx)) {
            m->SetIndividualStartChannel(idx, newCh);
            changed = YES;
        }
    } else if (keyStr == "modelChain") {
        NSString* s = asString(&typeOk);
        if (!typeOk) return NO;
        std::string newChain = s.UTF8String;
        // Desktop stores "" for "Beginning"; surface the symbol
        // to the user but write the empty string internally.
        if (newChain == "Beginning") newChain = "";
        if (newChain != m->GetModelChain()) {
            m->SetModelChain(newChain);
            changed = YES;
        }
    }
    // J-20 — Controller Connection setters. Each key here writes
    // through the ControllerConnection object on the Model and
    // flips its CTRL_PROPS active flag when needed. Naming is
    // `cc.<field>` to keep them grouped and disambiguated from
    // the model-header keys above.
    else if (keyStr == "cc.port") {
        double v = asDouble(&typeOk); if (!typeOk) return NO;
        if ((int)v != m->GetControllerPort(1)) { m->SetControllerPort((int)v); changed = YES; }
    } else if (keyStr == "cc.protocol") {
        NSString* s = asString(&typeOk); if (!typeOk) return NO;
        std::string newProto = s.UTF8String;
        if (newProto != m->GetControllerProtocol()) { m->SetControllerProtocol(newProto); changed = YES; }
    } else if (keyStr == "cc.dmxChannel") {
        double v = asDouble(&typeOk); if (!typeOk) return NO;
        if ((int)v != m->GetControllerDMXChannel()) { m->SetControllerDMXChannel((int)v); changed = YES; }
    } else if (keyStr == "cc.speedIndex") {
        double v = asDouble(&typeOk); if (!typeOk) return NO;
        std::vector<std::string> cp; int protoIdx = -1;
        m->GetControllerProtocols(cp, protoIdx);
        if (protoIdx >= 0 && protoIdx < (int)cp.size()) {
            std::vector<std::string> speeds; int curIdx = -1;
            m->GetSerialProtocolSpeeds(cp[protoIdx], speeds, curIdx);
            int newIdx = (int)v;
            if (newIdx >= 0 && newIdx < (int)speeds.size() && newIdx != curIdx) {
                int newSpeed = std::strtol(speeds[newIdx].c_str(), nullptr, 10);
                m->GetCtrlConn().SetSerialProtocolSpeed(newSpeed);
                changed = YES;
            }
        }
    } else if (keyStr == "cc.useSmartRemote") {
        BOOL v = asBool(&typeOk); if (!typeOk) return NO;
        m->GetCtrlConn().UpdateProperty(ControllerConnection::USE_SMART_REMOTE, v ? true : false);
        if (!v) { m->SetSmartRemote(0); }
        changed = YES;
    } else if (keyStr == "cc.smartRemoteIndex") {
        // 0-based picker index; SetSmartRemote uses 1-based ("A"=1).
        double v = asDouble(&typeOk); if (!typeOk) return NO;
        int newSr = (int)v + 1;
        if (newSr != m->GetSmartRemote()) { m->SetSmartRemote(newSr); changed = YES; }
    } else if (keyStr == "cc.smartRemoteTypeIndex") {
        double v = asDouble(&typeOk); if (!typeOk) return NO;
        auto types = m->GetSmartRemoteTypes();
        int idx = (int)v;
        if (idx >= 0 && idx < (int)types.size() && types[idx] != m->GetSmartRemoteType()) {
            m->SetSmartRemoteType(types[idx]); changed = YES;
        }
    } else if (keyStr == "cc.srMaxCascade") {
        double v = asDouble(&typeOk); if (!typeOk) return NO;
        if ((int)v != m->GetSRMaxCascade()) { m->SetSRMaxCascade((int)v); changed = YES; }
    } else if (keyStr == "cc.srCascadeOnPort") {
        BOOL v = asBool(&typeOk); if (!typeOk) return NO;
        if ((v?true:false) != m->GetSRCascadeOnPort()) { m->SetSRCascadeOnPort(v?true:false); changed = YES; }
    }
    // Per-pixel toggle / value pairs. The `xxxActive` keys flip
    // a CTRL_PROPS flag (so the save layer writes the attribute
    // or not); the value keys write the actual int / float /
    // string into the connection.
    else if (keyStr == "cc.startNullsActive") {
        BOOL v = asBool(&typeOk); if (!typeOk) return NO;
        m->GetCtrlConn().UpdateProperty(ControllerConnection::START_NULLS_ACTIVE, v?true:false);
        changed = YES;
    } else if (keyStr == "cc.startNulls") {
        double v = asDouble(&typeOk); if (!typeOk) return NO;
        if ((int)v != m->GetControllerStartNulls()) { m->SetControllerStartNulls((int)v); changed = YES; }
    } else if (keyStr == "cc.endNullsActive") {
        BOOL v = asBool(&typeOk); if (!typeOk) return NO;
        m->GetCtrlConn().UpdateProperty(ControllerConnection::END_NULLS_ACTIVE, v?true:false);
        changed = YES;
    } else if (keyStr == "cc.endNulls") {
        double v = asDouble(&typeOk); if (!typeOk) return NO;
        if ((int)v != m->GetControllerEndNulls()) { m->SetControllerEndNulls((int)v); changed = YES; }
    } else if (keyStr == "cc.brightnessActive") {
        BOOL v = asBool(&typeOk); if (!typeOk) return NO;
        m->GetCtrlConn().UpdateProperty(ControllerConnection::BRIGHTNESS_ACTIVE, v?true:false);
        changed = YES;
    } else if (keyStr == "cc.brightness") {
        double v = asDouble(&typeOk); if (!typeOk) return NO;
        if ((int)v != m->GetControllerBrightness()) { m->SetControllerBrightness((int)v); changed = YES; }
    } else if (keyStr == "cc.gammaActive") {
        BOOL v = asBool(&typeOk); if (!typeOk) return NO;
        m->GetCtrlConn().UpdateProperty(ControllerConnection::GAMMA_ACTIVE, v?true:false);
        changed = YES;
    } else if (keyStr == "cc.gamma") {
        double v = asDouble(&typeOk); if (!typeOk) return NO;
        if ((float)v != m->GetControllerGamma()) { m->SetControllerGamma((float)v); changed = YES; }
    } else if (keyStr == "cc.colorOrderActive") {
        BOOL v = asBool(&typeOk); if (!typeOk) return NO;
        m->GetCtrlConn().UpdateProperty(ControllerConnection::COLOR_ORDER_ACTIVE, v?true:false);
        changed = YES;
    } else if (keyStr == "cc.colorOrderIndex") {
        double v = asDouble(&typeOk); if (!typeOk) return NO;
        int idx = (int)v;
        if (idx >= 0 && idx < (int)Model::CONTROLLER_COLORORDER.size()) {
            std::string co = Model::CONTROLLER_COLORORDER[idx];
            if (co != m->GetControllerColorOrder()) { m->SetControllerColorOrder(co); changed = YES; }
        }
    } else if (keyStr == "cc.directionActive") {
        BOOL v = asBool(&typeOk); if (!typeOk) return NO;
        m->GetCtrlConn().UpdateProperty(ControllerConnection::REVERSE_ACTIVE, v?true:false);
        changed = YES;
    } else if (keyStr == "cc.directionIndex") {
        double v = asDouble(&typeOk); if (!typeOk) return NO;
        if ((int)v != m->GetControllerReverse()) { m->SetControllerReverse((int)v); changed = YES; }
    } else if (keyStr == "cc.groupCountActive") {
        BOOL v = asBool(&typeOk); if (!typeOk) return NO;
        m->GetCtrlConn().UpdateProperty(ControllerConnection::GROUP_COUNT_ACTIVE, v?true:false);
        changed = YES;
    } else if (keyStr == "cc.groupCount") {
        double v = asDouble(&typeOk); if (!typeOk) return NO;
        if ((int)v != m->GetControllerGroupCount()) { m->SetControllerGroupCount((int)v); changed = YES; }
    } else if (keyStr == "cc.zigZagActive") {
        BOOL v = asBool(&typeOk); if (!typeOk) return NO;
        m->GetCtrlConn().UpdateProperty(ControllerConnection::ZIG_ZAG_ACTIVE, v?true:false);
        changed = YES;
    } else if (keyStr == "cc.zigZag") {
        double v = asDouble(&typeOk); if (!typeOk) return NO;
        if ((int)v != m->GetControllerZigZag()) { m->SetControllerZigZag((int)v); changed = YES; }
    } else if (keyStr == "cc.smartTsActive") {
        BOOL v = asBool(&typeOk); if (!typeOk) return NO;
        m->GetCtrlConn().UpdateProperty(ControllerConnection::TS_ACTIVE, v?true:false);
        changed = YES;
    } else if (keyStr == "cc.smartTs") {
        double v = asDouble(&typeOk); if (!typeOk) return NO;
        if ((int)v != m->GetSmartTs()) { m->SetSmartRemoteTs((int)v); changed = YES; }
    } else if (keyStr == "cc.pwmGamma") {
        double v = asDouble(&typeOk); if (!typeOk) return NO;
        if ((float)v != m->GetControllerGamma()) { m->SetControllerGamma((float)v); changed = YES; }
    } else if (keyStr == "cc.pwmBrightness") {
        double v = asDouble(&typeOk); if (!typeOk) return NO;
        if ((int)v != m->GetControllerBrightness()) { m->SetControllerBrightness((int)v); changed = YES; }
    } else {
        spdlog::warn("setLayoutModelProperty: unknown key '{}' for model '{}'",
                     keyStr, [name UTF8String]);
        return NO;
    }

    if (changed) {
        _context->MarkLayoutModelDirty(std::string([name UTF8String]));
        [self recalcModelStartChannels];
    }
    return changed;
}

- (BOOL)saveLayoutChanges {
    if (!_context) return NO;
    return _context->SaveLayoutChanges() ? YES : NO;
}

- (NSString*)axisToolForModel:(NSString*)modelName {
    if (!_context || !modelName || modelName.length == 0) return @"none";
    Model* m = _context->GetModelManager()[modelName.UTF8String];
    if (!m) return @"none";
    switch (m->GetModelScreenLocation().GetAxisTool()) {
        case handles::Tool::Translate: return @"translate";
        case handles::Tool::Scale:     return @"scale";
        case handles::Tool::Rotate:    return @"rotate";
        case handles::Tool::XYTranslate:  return @"xy_trans";
        case handles::Tool::Elevate:   return @"elevate";
        default:                                            return @"none";
    }
}

- (BOOL)setAxisTool:(NSString*)tool forModel:(NSString*)modelName {
    if (!_context || !modelName || modelName.length == 0 || !tool) return NO;
    Model* m = _context->GetModelManager()[modelName.UTF8String];
    if (!m) return NO;
    handles::Tool axisTool;
    if      ([tool isEqualToString:@"translate"]) axisTool = handles::Tool::Translate;
    else if ([tool isEqualToString:@"scale"])     axisTool = handles::Tool::Scale;
    else if ([tool isEqualToString:@"rotate"])    axisTool = handles::Tool::Rotate;
    else if ([tool isEqualToString:@"xy_trans"])  axisTool = handles::Tool::XYTranslate;
    else if ([tool isEqualToString:@"elevate"])   axisTool = handles::Tool::Elevate;
    else                                          return NO;
    m->GetModelScreenLocation().SetAxisTool(axisTool);
    return YES;
}

- (BOOL)deleteVertexAtIndex:(NSInteger)vertexIndex forModel:(NSString*)modelName {
    if (!_context || !modelName || modelName.length == 0) return NO;
    Model* m = _context->GetModelManager()[modelName.UTF8String];
    if (!m) return NO;
    if (m->GetModelScreenLocation().IsLocked()) return NO;
    if (m->GetNumHandles() <= 2) return NO;  // can't drop below a segment
    _context->AbortRender(5000);
    _context->PushLayoutUndoSnapshotForModel(modelName.UTF8String);
    // PolyPoint vertex int convention is 1-based.
    m->DeleteHandle(static_cast<int>(vertexIndex) + 1);
    m->GetModelScreenLocation().SelectSegment(-1);
    m->Reinitialize();
    _context->MarkLayoutModelDirty(modelName.UTF8String);
    [self recalcModelStartChannels];
    return YES;
}

- (BOOL)insertVertexInSegment:(NSInteger)segmentIndex forModel:(NSString*)modelName {
    if (!_context || !modelName || modelName.length == 0) return NO;
    Model* m = _context->GetModelManager()[modelName.UTF8String];
    if (!m) return NO;
    if (m->GetModelScreenLocation().IsLocked()) return NO;
    _context->AbortRender(5000);
    _context->PushLayoutUndoSnapshotForModel(modelName.UTF8String);
    // PolyPointScreenLocation::InsertHandle's zoom / scale params
    // aren't actually consulted — placeholder values are fine.
    m->InsertHandle(static_cast<int>(segmentIndex), 1.0f, 1);
    m->Reinitialize();
    _context->MarkLayoutModelDirty(modelName.UTF8String);
    [self recalcModelStartChannels];
    return YES;
}

- (NSArray<NSString*>*)availableModelTypesForCreation {
    // Curated for first-cut iPad Add-Model. Each name matches a
    // case in `ModelManager::CreateDefaultModel`. Strings here are
    // the literal type tags the factory expects; SwiftUI shows
    // friendlier labels.
    //
    // "Wreath" is intentionally absent — the desktop has marked it
    // deprecated, so we don't want users creating new ones from
    // the iPad. Existing Wreath models in a show still load and
    // render correctly; this only gates the Add Model picker.
    return @[
        @"Arches",
        @"Candy Canes",
        @"Channel Block",
        @"Circle",
        @"Cube",
        @"Custom",
        // LAY-1 — DMX / moving-head model family. Each tag matches a case
        // in CreateDefaultModel and creates a single-point boxed model that
        // the existing tap-to-place flow commits immediately (none use a
        // poly-point location).
        @"DmxFloodArea",
        @"DmxFloodlight",
        @"DmxGeneral",
        @"DmxMovingHead",
        @"DmxMovingHeadAdv",
        @"DmxServo",
        @"DmxServo3d",
        @"DmxSkull",
        @"Icicles",
        @"Image",
        @"Matrix",
        @"MultiPoint",
        @"Poly Line",
        @"Single Line",
        @"Sphere",
        @"Spinner",
        @"Star",
        @"Tree",
        @"Window Frame",
    ];
}

- (BOOL)deleteModel:(NSString*)modelName {
    if (!_context || !modelName || modelName.length == 0) return NO;
    _context->AbortRender(5000);
    if (!_context->GetModelManager().Delete(modelName.UTF8String)) return NO;
    [self recalcModelStartChannels];
    return YES;
}

- (BOOL)renameModel:(NSString*)oldName to:(NSString*)newName {
    if (!_context || !_context->HasModelManager()) return NO;
    if (!oldName || !newName) return NO;
    std::string oldStd = oldName.UTF8String;
    std::string newStd = Model::SafeModelName(newName.UTF8String);
    if (newStd.empty() || oldStd == newStd) return NO;

    auto& mgr = _context->GetModelManager();
    Model* m = mgr[oldStd];
    if (!m) return NO;
    if (m->GetDisplayAs() == DisplayAsType::SubModel) return NO;
    // Refuse collision with existing model OR group.
    if (mgr.GetModel(newStd)) return NO;

    _context->AbortRender(5000);
    if (!mgr.Rename(oldStd, newStd)) return NO;

    _context->MarkModelRenamed(oldStd, newStd);
    _context->MarkLayoutModelDirty(newStd);

    // Any group whose member list referenced the old name will
    // now reference the new — mark those groups dirty too so the
    // patcher rewrites their on-disk `models` attribute.
    for (const auto& [name, model] : mgr.GetModels()) {
        if (!model) continue;
        if (model->GetDisplayAs() != DisplayAsType::ModelGroup) continue;
        auto* mg = static_cast<ModelGroup*>(model);
        if (mg->DirectlyContainsModel(newStd)) {
            _context->MarkLayoutModelDirty(name);
        }
    }
    // ModelChain references (`@OldName`) on downstream models
    // need to be re-resolved against the new name.
    [self recalcModelStartChannels];
    return YES;
}

- (BOOL)setModelAliases:(NSString*)modelName
                aliases:(NSArray<NSString*>*)aliases {
    if (!_context || !_context->HasModelManager()) return NO;
    if (!modelName) return NO;
    Model* m = _context->GetModelManager()[std::string(modelName.UTF8String)];
    if (!m) return NO;
    _context->AbortRender(5000);

    std::list<std::string> next;
    std::unordered_set<std::string> seen;
    for (NSString* a in aliases ?: @[]) {
        std::string s = a.UTF8String;
        // Trim leading/trailing whitespace before lowercasing —
        // Model::AddAlias doesn't trim itself.
        auto notSpace = [](unsigned char ch) { return !std::isspace(ch); };
        s.erase(s.begin(), std::find_if(s.begin(), s.end(), notSpace));
        s.erase(std::find_if(s.rbegin(), s.rend(), notSpace).base(), s.end());
        if (s.empty()) continue;
        std::transform(s.begin(), s.end(), s.begin(),
                       [](unsigned char ch) { return std::tolower(ch); });
        if (!seen.insert(s).second) continue;
        next.push_back(s);
    }
    m->SetAliases(next);
    _context->MarkLayoutModelDirty(std::string(modelName.UTF8String));
    return YES;
}

- (BOOL)setStrandNames:(NSString*)modelName
                 names:(NSArray<NSString*>*)names {
    if (!_context || !_context->HasModelManager() || !modelName) return NO;
    Model* m = _context->GetModelManager()[std::string(modelName.UTF8String)];
    if (!m) return NO;
    _context->AbortRender(5000);
    std::string joined = [self joinIndexedNames:names];
    m->SetStrandNames(joined);
    m->IncrementChangeCount();
    _context->MarkLayoutModelDirty(std::string(modelName.UTF8String));
    return YES;
}

- (BOOL)setNodeNames:(NSString*)modelName
               names:(NSArray<NSString*>*)names {
    if (!_context || !_context->HasModelManager() || !modelName) return NO;
    Model* m = _context->GetModelManager()[std::string(modelName.UTF8String)];
    if (!m) return NO;
    _context->AbortRender(5000);
    std::string joined = [self joinIndexedNames:names];
    m->SetNodeNames(joined);
    m->IncrementChangeCount();
    _context->MarkLayoutModelDirty(std::string(modelName.UTF8String));
    return YES;
}

- (NSArray<NSString*>*)generateNodeNamesForModel:(NSString*)modelName {
    if (!_context || !_context->HasModelManager() || !modelName) return @[];
    Model* m = _context->GetModelManager()[std::string(modelName.UTF8String)];
    if (!m || !m->IsDMXModel()) return @[];
    DmxModel* dmx = dynamic_cast<DmxModel*>(m);
    if (!dmx) return @[];
    NSMutableArray<NSString*>* out = [NSMutableArray array];
    for (const std::string& name : dmx->GenerateNodeNames()) {
        [out addObject:[NSString stringWithUTF8String:name.c_str()]];
    }
    return out;
}

// J-22 — Faces / States / Dimming nested-dictionary helpers.
// All three use the same `FaceStateData` shape on the C++ side
// (map<string, map<string, string>>) so the bridge converters
// are shared, with the inner mutator chosen by caller.
static FaceStateData faceStateFromNSDict(NSDictionary<NSString*, NSDictionary<NSString*, NSString*>*>* entries) {
    FaceStateData out;
    if (!entries) return out;
    for (NSString* k in entries) {
        NSDictionary<NSString*, NSString*>* attrs = entries[k];
        std::map<std::string, std::string> attrMap;
        for (NSString* ak in attrs) {
            NSString* v = attrs[ak];
            attrMap[ak.UTF8String] = v.UTF8String;
        }
        out[k.UTF8String] = attrMap;
    }
    return out;
}
static NSDictionary<NSString*, NSDictionary<NSString*, NSString*>*>* faceStateToNSDict(const FaceStateData& info) {
    NSMutableDictionary<NSString*, NSMutableDictionary<NSString*, NSString*>*>* out =
        [NSMutableDictionary dictionary];
    for (const auto& [k, attrs] : info) {
        NSMutableDictionary<NSString*, NSString*>* attrDict = [NSMutableDictionary dictionary];
        for (const auto& [ak, av] : attrs) {
            attrDict[[NSString stringWithUTF8String:ak.c_str()]] =
                [NSString stringWithUTF8String:av.c_str()];
        }
        out[[NSString stringWithUTF8String:k.c_str()]] = attrDict;
    }
    return out;
}

- (BOOL)setFaceInfo:(NSString*)modelName
            entries:(NSDictionary<NSString*, NSDictionary<NSString*, NSString*>*>*)entries {
    if (!_context || !_context->HasModelManager() || !modelName) return NO;
    Model* m = _context->GetModelManager()[std::string(modelName.UTF8String)];
    if (!m) return NO;
    _context->AbortRender(5000);
    m->SetFaceInfo(faceStateFromNSDict(entries));
    _context->MarkLayoutModelDirty(std::string(modelName.UTF8String));
    return YES;
}

- (BOOL)setStateInfo:(NSString*)modelName
             entries:(NSDictionary<NSString*, NSDictionary<NSString*, NSString*>*>*)entries {
    if (!_context || !_context->HasModelManager() || !modelName) return NO;
    Model* m = _context->GetModelManager()[std::string(modelName.UTF8String)];
    if (!m) return NO;
    _context->AbortRender(5000);
    m->SetStateInfo(faceStateFromNSDict(entries));
    _context->MarkLayoutModelDirty(std::string(modelName.UTF8String));
    // States have a separate state-info save path (used by the
    // DMX state editor); make sure it picks up the change too.
    _context->MarkModelStateDirty(std::string(modelName.UTF8String));
    return YES;
}

- (NSDictionary<NSString*, NSDictionary<NSString*, NSString*>*>*)
        faceInfoForModel:(NSString*)modelName {
    if (!_context || !_context->HasModelManager() || !modelName) return @{};
    Model* m = _context->GetModelManager()[std::string(modelName.UTF8String)];
    if (!m) return @{};
    return faceStateToNSDict(m->GetFaceInfo());
}

- (NSDictionary<NSString*, NSDictionary<NSString*, NSString*>*>*)
        stateInfoForModel:(NSString*)modelName {
    if (!_context || !_context->HasModelManager() || !modelName) return @{};
    Model* m = _context->GetModelManager()[std::string(modelName.UTF8String)];
    if (!m) return @{};
    return faceStateToNSDict(m->GetStateInfo());
}

- (BOOL)setDimmingInfo:(NSString*)modelName
               entries:(NSDictionary<NSString*, NSDictionary<NSString*, NSString*>*>*)entries {
    if (!_context || !_context->HasModelManager() || !modelName) return NO;
    Model* m = _context->GetModelManager()[std::string(modelName.UTF8String)];
    if (!m) return NO;
    _context->AbortRender(5000);
    m->SetDimmingInfo(faceStateFromNSDict(entries));
    _context->MarkLayoutModelDirty(std::string(modelName.UTF8String));
    return YES;
}

- (NSDictionary<NSString*, NSDictionary<NSString*, NSString*>*>*)
        dimmingInfoForModel:(NSString*)modelName {
    if (!_context || !_context->HasModelManager() || !modelName) return @{};
    Model* m = _context->GetModelManager()[std::string(modelName.UTF8String)];
    if (!m) return @{};
    return faceStateToNSDict(m->GetDimmingInfo());
}

// J-23 — Custom-model 3D grid bridge. Exposes `_locations` as
// a nested NSArray for SwiftUI canvas rendering + edits.
- (NSDictionary*)customModelDataForModel:(NSString*)modelName {
    if (!_context || !_context->HasModelManager() || !modelName) return @{};
    Model* m = _context->GetModelManager()[std::string(modelName.UTF8String)];
    auto* cm = dynamic_cast<CustomModel*>(m);
    if (!cm) return @{};
    int w = (int)cm->GetCustomWidth();
    int h = (int)cm->GetCustomHeight();
    int d = (int)cm->GetCustomDepth();
    NSMutableArray<NSArray<NSArray<NSNumber*>*>*>* locs = [NSMutableArray array];
    const auto& data = cm->GetData();
    for (int layer = 0; layer < d; ++layer) {
        NSMutableArray<NSArray<NSNumber*>*>* layerArr = [NSMutableArray array];
        for (int row = 0; row < h; ++row) {
            NSMutableArray<NSNumber*>* rowArr = [NSMutableArray array];
            for (int col = 0; col < w; ++col) {
                int v = 0;
                if (layer < (int)data.size() &&
                    row < (int)data[layer].size() &&
                    col < (int)data[layer][row].size()) {
                    v = data[layer][row][col];
                }
                [rowArr addObject:@(v)];
            }
            [layerArr addObject:rowArr];
        }
        [locs addObject:layerArr];
    }
    return @{
        @"width":     @(w),
        @"height":    @(h),
        @"depth":     @(d),
        @"locations": locs,
        // J-23.2 — expose the bg-image trio so the visual editor
        // can paint the image under the gridlines and respect
        // scale + brightness.
        @"backgroundImage": [NSString stringWithUTF8String:cm->GetCustomBackground().c_str()],
        @"backgroundScale": @(cm->GetCustomBkgScale()),
        @"backgroundBrightness": @(cm->GetCustomBkgBrightness()),
    };
}

- (BOOL)setCustomModelData:(NSString*)modelName
                     width:(int)w
                    height:(int)h
                     depth:(int)d
                 locations:(NSArray<NSArray<NSArray<NSNumber*>*>*>*)locations {
    if (!_context || !_context->HasModelManager() || !modelName) return NO;
    if (w < 1 || h < 1 || d < 1) return NO;
    if (!locations || (int)locations.count != d) return NO;
    Model* m = _context->GetModelManager()[std::string(modelName.UTF8String)];
    auto* cm = dynamic_cast<CustomModel*>(m);
    if (!cm) return NO;

    // Build the C++ 3D vector. Pad / truncate so the result is
    // rectangular and matches the declared dims even if the
    // caller sent slightly off-shape arrays.
    std::vector<std::vector<std::vector<int>>> data(d,
        std::vector<std::vector<int>>(h, std::vector<int>(w, 0)));
    for (int layer = 0; layer < d; ++layer) {
        NSArray<NSArray<NSNumber*>*>* layerArr = locations[layer];
        if (![layerArr isKindOfClass:[NSArray class]]) continue;
        int hh = std::min((int)layerArr.count, h);
        for (int row = 0; row < hh; ++row) {
            NSArray<NSNumber*>* rowArr = layerArr[row];
            if (![rowArr isKindOfClass:[NSArray class]]) continue;
            int ww = std::min((int)rowArr.count, w);
            for (int col = 0; col < ww; ++col) {
                if ([rowArr[col] isKindOfClass:[NSNumber class]]) {
                    data[layer][row][col] = [rowArr[col] intValue];
                }
            }
        }
    }
    _context->AbortRender(5000);
    cm->SetCustomWidth(w);
    cm->SetCustomHeight(h);
    cm->SetCustomDepth(d);
    cm->SetCustomData(data);
    _context->MarkLayoutModelDirty(std::string(modelName.UTF8String));
    [self recalcModelStartChannels];
    return YES;
}

- (BOOL)clearDimmingCurveOnModel:(NSString*)modelName {
    if (!_context || !_context->HasModelManager() || !modelName) return NO;
    Model* m = _context->GetModelManager()[std::string(modelName.UTF8String)];
    if (!m) return NO;
    if (m->GetDimmingCurve() == nullptr) return NO;
    _context->AbortRender(5000);
    m->SetDimmingInfo({});
    _context->MarkLayoutModelDirty(std::string(modelName.UTF8String));
    return YES;
}

- (BOOL)renameSubModelNamed:(NSString*)oldName
                    onModel:(NSString*)parentName
                         to:(NSString*)newName {
    if (!_context || !_context->HasModelManager()) return NO;
    if (!oldName || !parentName || !newName) return NO;
    Model* parent = _context->GetModelManager()[std::string(parentName.UTF8String)];
    if (!parent) return NO;
    std::string oldStd = oldName.UTF8String;
    std::string newStd = Model::SafeModelName(newName.UTF8String);
    if (newStd.empty() || oldStd == newStd) return NO;
    Model* sm = parent->GetSubModel(oldStd);
    if (!sm) return NO;
    if (parent->GetSubModel(newStd)) return NO; // collision
    _context->AbortRender(5000);
    sm->name = newStd;
    parent->IncrementChangeCount();
    _context->MarkLayoutModelDirty(std::string(parentName.UTF8String));
    return YES;
}

- (NSArray<NSDictionary*>*)submodelDetailsForModel:(NSString*)parentName {
    NSMutableArray<NSDictionary*>* out = [NSMutableArray array];
    if (!_context || !_context->HasModelManager() || !parentName) return out;
    Model* parent = _context->GetModelManager()[std::string(parentName.UTF8String)];
    if (!parent) return out;
    for (Model* m : parent->GetSubModels()) {
        auto* sm = dynamic_cast<SubModel*>(m);
        if (!sm) continue;
        NSMutableDictionary* d = [NSMutableDictionary dictionary];
        d[@"name"]        = [NSString stringWithUTF8String:sm->GetName().c_str()];
        d[@"isRanges"]    = @(sm->IsRanges() ? YES : NO);
        d[@"isVertical"]  = @(sm->IsVertical() ? YES : NO);
        d[@"bufferStyle"] = [NSString stringWithUTF8String:sm->GetSubModelBufferStyle().c_str()];
        if (sm->IsRanges()) {
            NSMutableArray<NSString*>* strands = [NSMutableArray array];
            int n = sm->GetNumRanges();
            for (int i = 0; i < n; ++i) {
                [strands addObject:[NSString stringWithUTF8String:sm->GetRange(i).c_str()]];
            }
            d[@"strands"] = strands;
            d[@"subBuffer"] = @"";
        } else {
            d[@"strands"] = @[];
            // Sub-buffer's actual rectangle isn't trivially
            // exposed today; surface the property-grid display
            // string and let the desktop be authoritative for
            // exact editing. (Most submodel editing on iPad will
            // be ranges anyway.)
            d[@"subBuffer"] = [NSString stringWithUTF8String:sm->GetSubModelLines().c_str()];
        }
        [out addObject:d];
    }
    return out;
}

- (BOOL)replaceSubModelsOnModel:(NSString*)parentName
                    withEntries:(NSArray<NSDictionary*>*)entries {
    if (!_context || !_context->HasModelManager() || !parentName || !entries) return NO;
    Model* parent = _context->GetModelManager()[std::string(parentName.UTF8String)];
    if (!parent) return NO;
    _context->AbortRender(5000);
    parent->RemoveAllSubModels();
    for (NSDictionary* d in entries) {
        NSString* nm = d[@"name"];
        if (![nm isKindOfClass:[NSString class]]) continue;
        std::string name = Model::SafeModelName(nm.UTF8String);
        if (name.empty()) continue;
        bool isRanges    = [d[@"isRanges"]   boolValue];
        bool isVertical  = [d[@"isVertical"] boolValue];
        NSString* bsObj  = d[@"bufferStyle"];
        std::string bs   = [bsObj isKindOfClass:[NSString class]]
                            ? std::string(bsObj.UTF8String) : "Default";
        auto* sm = new SubModel(parent, name, isVertical, isRanges, bs);
        parent->AddSubmodel(sm);
        if (isRanges) {
            NSArray<NSString*>* strands = d[@"strands"];
            if ([strands isKindOfClass:[NSArray class]]) {
                if (bs == KEEP_XY) {
                    for (NSString* s in strands) {
                        if (![s isKindOfClass:[NSString class]]) continue;
                        sm->AddRangeXY(s.UTF8String);
                    }
                    sm->CalcRangeXYBufferSize();
                } else {
                    for (NSString* s in strands) {
                        if (![s isKindOfClass:[NSString class]]) continue;
                        sm->AddDefaultBuffer(s.UTF8String);
                    }
                }
            }
        } else {
            NSString* sub = d[@"subBuffer"];
            if ([sub isKindOfClass:[NSString class]]) {
                sm->AddSubbuffer(sub.UTF8String);
            }
        }
        sm->Setup();
    }
    parent->IncrementChangeCount();
    _context->MarkLayoutModelDirty(std::string(parentName.UTF8String));
    return YES;
}

// Convert a parsed SubModelImportData into the dictionary shape
// the SwiftUI submodel editor consumes (matches
// submodelDetailsForModel:).
static NSDictionary* SubModelImportDataToDict(const XmlSerialize::SubModelImportData& sm) {
    NSMutableDictionary* d = [NSMutableDictionary dictionary];
    d[@"name"]        = [NSString stringWithUTF8String:sm.name.c_str()];
    d[@"isRanges"]    = @(sm.isRanges ? YES : NO);
    d[@"isVertical"]  = @(sm.vertical ? YES : NO);
    d[@"bufferStyle"] = [NSString stringWithUTF8String:sm.bufferStyle.c_str()];
    if (sm.isRanges) {
        NSMutableArray<NSString*>* strands = [NSMutableArray array];
        for (const auto& s : sm.strands) {
            [strands addObject:[NSString stringWithUTF8String:s.c_str()]];
        }
        d[@"strands"] = strands;
        d[@"subBuffer"] = @"";
    } else {
        d[@"strands"] = @[];
        d[@"subBuffer"] = [NSString stringWithUTF8String:sm.subBuffer.c_str()];
    }
    return d;
}

- (NSArray<NSDictionary*>*)submodelDetailsFromXmodelFile:(NSString*)path {
    NSMutableArray<NSDictionary*>* out = [NSMutableArray array];
    if (!path || path.length == 0) return out;
    ObtainAccessToURL([path UTF8String]);
    pugi::xml_document doc;
    if (!doc.load_file(path.UTF8String)) return out;
    pugi::xml_node root = doc.document_element();
    if (!root) return out;
    // Desktop ImportSubModel: when the root is a `<models>` wrapper
    // (multi-model export) descend to the first `<model>`.
    if (std::string_view(root.name()) == "models") {
        root = root.first_child();
        while (root && root.type() != pugi::node_element) root = root.next_sibling();
    }
    if (!root) return out;
    for (const auto& sm : XmlSerialize::LoadSubModelsFromXml(root)) {
        [out addObject:SubModelImportDataToDict(sm)];
    }
    return out;
}

- (NSArray<NSString*>*)modelNamesInRGBEffectsFile:(NSString*)path {
    NSMutableArray<NSString*>* out = [NSMutableArray array];
    if (!path || path.length == 0) return out;
    ObtainAccessToURL([path UTF8String]);
    pugi::xml_document doc;
    if (!doc.load_file(path.UTF8String)) return out;
    pugi::xml_node root = doc.document_element();
    if (!root) return out;
    pugi::xml_node models = root.child("models");
    if (!models) return out;
    for (pugi::xml_node m = models.first_child(); m; m = m.next_sibling()) {
        if (m.type() != pugi::node_element) continue;
        const char* mn = m.attribute("name").as_string("");
        if (mn && mn[0]) [out addObject:[NSString stringWithUTF8String:mn]];
    }
    [out sortUsingSelector:@selector(localizedCaseInsensitiveCompare:)];
    return out;
}

- (NSArray<NSDictionary*>*)submodelDetailsFromRGBEffectsFile:(NSString*)path
                                                  modelName:(NSString*)modelName {
    NSMutableArray<NSDictionary*>* out = [NSMutableArray array];
    if (!path || path.length == 0 || !modelName) return out;
    ObtainAccessToURL([path UTF8String]);
    pugi::xml_document doc;
    if (!doc.load_file(path.UTF8String)) return out;
    pugi::xml_node root = doc.document_element();
    if (!root) return out;
    pugi::xml_node models = root.child("models");
    if (!models) return out;
    std::string want(modelName.UTF8String);
    for (pugi::xml_node m = models.first_child(); m; m = m.next_sibling()) {
        if (m.type() != pugi::node_element) continue;
        if (want == m.attribute("name").as_string("")) {
            for (const auto& sm : XmlSerialize::LoadSubModelsFromXml(m)) {
                [out addObject:SubModelImportDataToDict(sm)];
            }
            break;
        }
    }
    return out;
}

- (NSArray<NSDictionary*>*)submodelDetailsFromModel:(NSString*)sourceModel {
    NSMutableArray<NSDictionary*>* out = [NSMutableArray array];
    if (!_context || !_context->HasModelManager() || !sourceModel) return out;
    Model* src = _context->GetModelManager()[std::string(sourceModel.UTF8String)];
    if (!src) return out;
    for (Model* m : src->GetSubModels()) {
        auto* sm = dynamic_cast<SubModel*>(m);
        if (!sm) continue;
        XmlSerialize::SubModelImportData data;
        data.name       = sm->GetName();
        data.isRanges   = sm->IsRanges();
        data.vertical   = sm->IsVertical();
        data.bufferStyle = sm->GetSubModelBufferStyle();
        if (sm->IsRanges()) {
            int n = sm->GetNumRanges();
            for (int i = 0; i < n; ++i) data.strands.push_back(sm->GetRange(i));
        } else {
            data.subBuffer = sm->GetSubModelLines();
        }
        [out addObject:SubModelImportDataToDict(data)];
    }
    return out;
}

- (NSArray<NSString*>*)modelNamesWithSubmodelsExcluding:(NSString*)excluded {
    NSMutableArray<NSString*>* out = [NSMutableArray array];
    if (!_context || !_context->HasModelManager()) return out;
    std::string ex = excluded ? std::string(excluded.UTF8String) : "";
    for (const auto& it : _context->GetModelManager()) {
        Model* m = it.second;
        if (!m || m->GetDisplayAs() == DisplayAsType::ModelGroup) continue;
        if (m->GetName() == ex) continue;
        if (m->GetSubModels().empty()) continue;
        [out addObject:[NSString stringWithUTF8String:m->GetName().c_str()]];
    }
    [out sortUsingSelector:@selector(localizedCaseInsensitiveCompare:)];
    return out;
}

- (NSArray<NSDictionary*>*)submodelDetailsFromCSVFile:(NSString*)path {
    NSMutableArray<NSDictionary*>* out = [NSMutableArray array];
    if (!path || path.length == 0) return out;
    ObtainAccessToURL([path UTF8String]);
    NSError* err = nil;
    NSString* contents = [NSString stringWithContentsOfFile:path
                                                   encoding:NSUTF8StringEncoding
                                                      error:&err];
    if (!contents) return out;
    // Desktop ImportCSVSubModel: each non-empty line is a node
    // range; lines accumulate into one ranges submodel, each line
    // compressed via NodeUtils::CompressNodes. The submodel name
    // is the file stem.
    XmlSerialize::SubModelImportData data;
    data.name = Model::SafeModelName(
        std::string([[[path lastPathComponent] stringByDeletingPathExtension] UTF8String]));
    if (data.name.empty()) data.name = "Imported";
    data.isRanges = true;
    data.vertical = false;
    data.bufferStyle = "Default";
    NSArray<NSString*>* lines = [contents componentsSeparatedByCharactersInSet:
                                 [NSCharacterSet newlineCharacterSet]];
    for (NSString* line in lines) {
        std::string s = std::string([line UTF8String]);
        // Strip trailing CR / whitespace and skip blank lines.
        while (!s.empty() && (s.back() == '\r' || s.back() == ' ' || s.back() == '\t')) s.pop_back();
        if (s.empty()) continue;
        data.strands.push_back(NodeUtils::CompressNodes(s));
    }
    if (data.strands.empty()) return out;
    [out addObject:SubModelImportDataToDict(data)];
    return out;
}

- (BOOL)exportModelToXmodelFile:(NSString*)modelName path:(NSString*)path {
    if (!_context || !_context->HasModelManager() || !modelName || !path) return NO;
    if (path.length == 0) return NO;
    Model* m = _context->GetModelManager()[std::string(modelName.UTF8String)];
    if (!m) return NO;
    if (m->GetDisplayAs() == DisplayAsType::ModelGroup) return NO;
    ObtainAccessToURL([path UTF8String], true);
    XmlSerializer serializer;
    pugi::xml_document doc = serializer.SerializeModel(m, /*includeGroups*/ true);
    return doc.save_file(path.UTF8String) ? YES : NO;
}

- (NSInteger)makeStartChannelValidForModel:(NSString*)modelName {
    if (!_context || !_context->HasModelManager() || !modelName) return 0;
    auto& mgr = _context->GetModelManager();
    Model* m = mgr[std::string(modelName.UTF8String)];
    if (!m || m->GetDisplayAs() == DisplayAsType::ModelGroup) return 0;
    _context->AbortRender(5000);
    NSInteger touched = 0;
    if (!m->CouldComputeStartChannel || !m->IsValidStartChannelString()) {
        m->SetControllerName(NO_CONTROLLER);
        touched = 1;
    }
    mgr.RecalcStartChannels();
    if (touched) _context->MarkLayoutModelDirty(std::string(modelName.UTF8String));
    return touched;
}

- (NSInteger)makeAllStartChannelsValid {
    if (!_context || !_context->HasModelManager()) return 0;
    auto& mgr = _context->GetModelManager();
    _context->AbortRender(5000);
    NSInteger touched = 0;
    for (const auto& it : mgr) {
        Model* m = it.second;
        if (!m || m->GetDisplayAs() == DisplayAsType::ModelGroup) continue;
        if (!m->CouldComputeStartChannel || !m->IsValidStartChannelString()) {
            m->SetControllerName(NO_CONTROLLER);
            _context->MarkLayoutModelDirty(m->GetName());
            ++touched;
        }
    }
    mgr.RecalcStartChannels();
    return touched;
}

- (NSInteger)makeAllStartChannelsNotOverlapping {
    if (!_context || !_context->HasModelManager()) return 0;
    auto& mgr = _context->GetModelManager();
    _context->AbortRender(5000);
    NSInteger touched = 0;
    for (const auto& it : mgr) {
        Model* m = it.second;
        if (!m || m->GetDisplayAs() == DisplayAsType::ModelGroup) continue;
        if (mgr.IsModelOverlapping(m)) {
            m->SetControllerName(NO_CONTROLLER);
            _context->MarkLayoutModelDirty(m->GetName());
            ++touched;
        }
    }
    mgr.RecalcStartChannels();
    return touched;
}

- (nullable NSString*)addSubModelToModel:(NSString*)parentName
                                    name:(NSString*)submodelName {
    if (!_context || !_context->HasModelManager()) return nil;
    if (!parentName || !submodelName) return nil;
    Model* parent = _context->GetModelManager()[std::string(parentName.UTF8String)];
    if (!parent) return nil;
    std::string std_name = Model::SafeModelName(submodelName.UTF8String);
    if (std_name.empty()) return nil;
    if (parent->GetSubModel(std_name)) return nil;
    _context->AbortRender(5000);
    // Defaults: horizontal, ranges, "Default" buffer style.
    // Mirrors what the desktop SubModelsDialog creates for a
    // brand-new submodel before the user edits its ranges.
    auto* sm = new SubModel(parent, std_name, false, true, "Default");
    parent->AddSubmodel(sm);
    sm->AddDefaultBuffer("1-1");
    sm->Setup();
    parent->IncrementChangeCount();
    _context->MarkLayoutModelDirty(std::string(parentName.UTF8String));
    return [NSString stringWithUTF8String:std_name.c_str()];
}

- (NSInteger)nodeCountForModel:(NSString*)modelName {
    if (!_context || !_context->HasModelManager() || !modelName) return 0;
    Model* m = _context->GetModelManager()[std::string(modelName.UTF8String)];
    if (!m) return 0;
    return (NSInteger)m->GetNodeCount();
}

- (nullable NSDictionary*)nodeLayoutForModel:(NSString*)modelName {
    if (!_context || !_context->HasModelManager() || !modelName) return nil;
    Model* m = _context->GetModelManager()[std::string(modelName.UTF8String)];
    if (!m) return nil;
    if (m->GetDisplayAs() == DisplayAsType::ModelGroup || m->GetDisplayAs() == DisplayAsType::SubModel) return nil;

    int nodes = (int)m->GetNodeCount();

    int bufWi = 0, bufHi = 0;
    std::vector<NodeBaseClassPtr> nodeList;
    m->InitRenderBufferNodes("Per Preview", "2D", "None", nodeList, bufWi, bufHi, 0);
    if (nodes > (int)nodeList.size()) nodes = (int)nodeList.size();

    float minx = std::numeric_limits<float>::max();
    float miny = std::numeric_limits<float>::max();
    float maxx = std::numeric_limits<float>::lowest();
    float maxy = std::numeric_limits<float>::lowest();
    for (int i = 0; i < nodes; ++i) {
        for (const auto& c : nodeList[i]->Coords) {
            if (c.screenX < minx) minx = c.screenX;
            if (c.screenX > maxx) maxx = c.screenX;
            if (c.screenY < miny) miny = c.screenY;
            if (c.screenY > maxy) maxy = c.screenY;
        }
    }
    if (nodes == 0) { minx = miny = 0; maxx = maxy = 0; }

    float width = maxx - minx;
    float height = maxy - miny;
    bool flipY = (m->GetDisplayAs() != DisplayAsType::Icicles);

    OutputManager& om = _context->GetOutputManager();

    NSMutableArray* outNodes = [NSMutableArray arrayWithCapacity:nodes];
    for (int i = 0; i < nodes; ++i) {
        if (nodeList[i]->Coords.empty()) continue;
        const auto& c = nodeList[i]->Coords[0];
        float x = c.screenX - minx;
        float y = c.screenY - miny;
        if (flipY) y = height - y;

        std::string chan = m->GetChannelInStartChannelFormat(&om, nodeList[i]->ActChan + 1);

        [outNodes addObject:@{
            @"node":    @(m->GetNodeNumber((size_t)i)),
            @"string":  @(m->GetNodeStringNumber((size_t)i)),
            @"x":       @((double)x),
            @"y":       @((double)y),
            @"channel": [NSString stringWithUTF8String:chan.c_str()],
        }];
    }

    return @{
        @"name":           [NSString stringWithUTF8String:m->GetName().c_str()],
        @"width":          @((double)width),
        @"height":         @((double)height),
        @"supportsWiring": @(m->SupportsWiringView() ? YES : NO),
        @"nodes":          outNodes,
    };
}

- (BOOL)setSubmodelAliasesOnParent:(NSString*)parentName
                          submodel:(NSString*)submodelName
                           aliases:(NSArray<NSString*>*)aliases {
    if (!_context || !_context->HasModelManager()) return NO;
    if (!parentName || !submodelName) return NO;
    Model* parent = _context->GetModelManager()[std::string(parentName.UTF8String)];
    if (!parent) return NO;
    Model* sub = parent->GetSubModel(std::string(submodelName.UTF8String));
    if (!sub) return NO;
    _context->AbortRender(5000);
    std::list<std::string> clean;
    std::set<std::string> seen;
    const std::string selfName = sub->GetName();
    std::string selfLower;
    selfLower.reserve(selfName.size());
    for (char c : selfName) selfLower.push_back(::tolower((unsigned char)c));
    for (NSString* a in aliases) {
        if (![a isKindOfClass:[NSString class]]) continue;
        std::string s = a.UTF8String;
        // trim
        size_t start = s.find_first_not_of(" \t\r\n");
        size_t end   = s.find_last_not_of(" \t\r\n");
        s = (start == std::string::npos) ? std::string() : s.substr(start, end - start + 1);
        if (s.empty()) continue;
        for (auto& c : s) c = ::tolower((unsigned char)c);
        if (s == selfLower) continue;
        if (seen.insert(s).second) clean.push_back(s);
    }
    sub->SetAliases(clean);
    parent->IncrementChangeCount();
    _context->MarkLayoutModelDirty(std::string(parentName.UTF8String));
    return YES;
}

- (NSArray<NSString*>*)submodelAliasesOnParent:(NSString*)parentName
                                      submodel:(NSString*)submodelName {
    NSMutableArray<NSString*>* out = [NSMutableArray array];
    if (!_context || !_context->HasModelManager()) return out;
    if (!parentName || !submodelName) return out;
    Model* parent = _context->GetModelManager()[std::string(parentName.UTF8String)];
    if (!parent) return out;
    Model* sub = parent->GetSubModel(std::string(submodelName.UTF8String));
    if (!sub) return out;
    for (const auto& a : sub->GetAliases()) {
        [out addObject:[NSString stringWithUTF8String:a.c_str()]];
    }
    return out;
}

- (BOOL)deleteSubModelNamed:(NSString*)submodelName
                    onModel:(NSString*)parentName {
    if (!_context || !_context->HasModelManager()) return NO;
    if (!submodelName || !parentName) return NO;
    if (submodelName.length == 0 || parentName.length == 0) return NO;
    Model* parent = _context->GetModelManager()[std::string(parentName.UTF8String)];
    if (!parent) return NO;
    std::string sub = submodelName.UTF8String;
    if (!parent->GetSubModel(sub)) return NO;
    _context->AbortRender(5000);
    parent->RemoveSubModel(sub);
    _context->MarkLayoutModelDirty(std::string(parentName.UTF8String));
    return YES;
}

- (BOOL)setCurve:(BOOL)create onSegment:(NSInteger)segmentIndex forModel:(NSString*)modelName {
    if (!_context || !modelName || modelName.length == 0) return NO;
    Model* m = _context->GetModelManager()[modelName.UTF8String];
    if (!m) return NO;
    if (m->GetModelScreenLocation().IsLocked()) return NO;
    if (!m->SupportsCurves()) return NO;
    _context->AbortRender(5000);
    _context->PushLayoutUndoSnapshotForModel(modelName.UTF8String);
    m->SetCurve(static_cast<int>(segmentIndex), create ? true : false);
    m->Reinitialize();
    _context->MarkLayoutModelDirty(modelName.UTF8String);
    return YES;
}

// MARK: - Layout Editor (Phase J-5, Groups + ViewObjects)

// Desktop parity (LayoutPanel.cpp:1948):
//   group == "All Models"               → all groups
//   group == "Unassigned"               → only model_group == "Unassigned"
//   anything else                       → model_group == active OR "All Previews"
static bool MatchesActiveLayoutGroupForRoster(const std::string& modelGroup,
                                                const std::string& active) {
    if (active == "All Models")  return true;
    if (active == "Unassigned")  return modelGroup == "Unassigned";
    return modelGroup == active || modelGroup == "All Previews";
}

// Resolve a virtual active group to the value that should be
// stored on a newly-created model / group / view object's
// `layoutGroup` field. "All Models" is virtual — newly created
// items fall back to "Default" (matching desktop's
// LayoutPanel.cpp:4912 logic). "Unassigned" round-trips so a
// created-in-Unassigned item actually appears in the Unassigned
// roster.
static std::string LayoutGroupForNewItem(const std::string& active) {
    if (active == "All Models" || active.empty()) return "Default";
    return active;
}

- (NSArray<NSString*>*)modelGroupsInActiveLayoutGroup {
    NSMutableArray<NSString*>* out = [NSMutableArray array];
    if (!_context || !_context->HasModelManager()) return out;
    const std::string active = _context->GetActiveLayoutGroup();
    for (const auto& [name, model] : _context->GetModelManager().GetModels()) {
        if (!model) continue;
        if (model->GetDisplayAs() != DisplayAsType::ModelGroup) continue;
        if (MatchesActiveLayoutGroupForRoster(model->GetLayoutGroup(), active)) {
            [out addObject:[NSString stringWithUTF8String:name.c_str()]];
        }
    }
    return out;
}

- (NSArray<NSDictionary<NSString*, id>*>*)modelGroupsListSummary {
    NSMutableArray<NSDictionary<NSString*, id>*>* out = [NSMutableArray array];
    if (!_context || !_context->HasModelManager()) return out;
    const std::string active = _context->GetActiveLayoutGroup();
    for (const auto& [name, model] : _context->GetModelManager().GetModels()) {
        if (!model) continue;
        if (model->GetDisplayAs() != DisplayAsType::ModelGroup) continue;
        if (!MatchesActiveLayoutGroupForRoster(model->GetLayoutGroup(), active)) continue;
        ModelGroup* g = static_cast<ModelGroup*>(model);
        [out addObject:@{
            @"name":        [NSString stringWithUTF8String:g->GetName().c_str()],
            @"modelCount":  @(g->GetModelCount()),
            @"layoutStyle": [NSString stringWithUTF8String:g->GetLayout().c_str()],
            @"gridSize":    @(g->GetGridSize()),
            @"isFromBase":  @(g->IsFromBase() ? YES : NO),
        }];
    }
    return out;
}

- (nullable NSDictionary<NSString*, id>*)modelGroupLayoutSummary:(NSString*)name {
    if (!_context || !_context->HasModelManager() || !name) return nil;
    Model* m = _context->GetModelManager()[std::string([name UTF8String])];
    if (!m || m->GetDisplayAs() != DisplayAsType::ModelGroup) return nil;
    ModelGroup* g = static_cast<ModelGroup*>(m);

    NSMutableArray<NSString*>* members = [NSMutableArray arrayWithCapacity:g->ModelNames().size()];
    for (const auto& s : g->ModelNames()) {
        [members addObject:[NSString stringWithUTF8String:s.c_str()]];
    }

    // J-10 — Full desktop ModelGroupPanel choice list. The first
    // 4 entries use compact lowercase wire names ("grid",
    // "minimalGrid", "horizontal", "vertical"); the rest round-
    // trip their display label verbatim (desktop's
    // `OnChoiceModelLayoutTypeSelect` falls through to
    // `GetStringSelection`). Order matches the wxs definition so
    // user mental model from desktop transfers.
    NSArray<NSDictionary<NSString*, NSString*>*>* layoutStyleOptions = @[
        @{@"value": @"grid",                          @"label": @"Grid as per preview"},
        @{@"value": @"minimalGrid",                   @"label": @"Minimal Grid"},
        @{@"value": @"horizontal",                    @"label": @"Horizontal Stack"},
        @{@"value": @"vertical",                      @"label": @"Vertical Stack"},
        @{@"value": @"Horizontal Stack - Scaled",     @"label": @"Horizontal Stack - Scaled"},
        @{@"value": @"Vertical Stack - Scaled",       @"label": @"Vertical Stack - Scaled"},
        @{@"value": @"Horizontal Per Model",          @"label": @"Horizontal Per Model"},
        @{@"value": @"Vertical Per Model",            @"label": @"Vertical Per Model"},
        @{@"value": @"Horizontal Per Model/Strand",   @"label": @"Horizontal Per Model/Strand"},
        @{@"value": @"Vertical Per Model/Strand",     @"label": @"Vertical Per Model/Strand"},
        @{@"value": @"Single Line",                   @"label": @"Single Line"},
        @{@"value": @"Overlay - Centered",            @"label": @"Overlay - Centered"},
        @{@"value": @"Overlay - Scaled",              @"label": @"Overlay - Scaled"},
        @{@"value": @"Single Line Model As A Pixel",  @"label": @"Single Line Model As A Pixel"},
        @{@"value": @"Default Model As A Pixel",      @"label": @"Default Model As A Pixel"},
        @{@"value": @"perModelDefault",               @"label": @"Per Model Default"},
    ];

    // J-9 — Default Camera options: "2D" + every named 3D camera
    // the user has saved via ViewpointMgr. Names round-trip
    // verbatim to xmlGroup's `DefaultCamera` attribute.
    NSMutableArray<NSString*>* cameraOptions = [NSMutableArray array];
    [cameraOptions addObject:@"2D"];
    {
        auto& vpm = _context->GetViewpointMgr();
        for (int i = 0; i < vpm.GetNum3DCameras(); ++i) {
            auto* cam = vpm.GetCamera3D(i);
            if (cam) [cameraOptions addObject:[NSString stringWithUTF8String:cam->GetName().c_str()]];
        }
    }

    return @{
        @"name":                [NSString stringWithUTF8String:g->GetName().c_str()],
        @"displayAs":           [NSString stringWithUTF8String:g->GetDisplayAsString().c_str()],
        @"layoutGroup":         [NSString stringWithUTF8String:g->GetLayoutGroup().c_str()],
        @"modelCount":          @(g->GetModelCount()),
        @"models":              members,
        @"defaultCamera":       [NSString stringWithUTF8String:g->GetDefaultCamera().c_str()],
        @"defaultCameraOptions": cameraOptions,
        @"layout":              [NSString stringWithUTF8String:g->GetLayout().c_str()],
        @"layoutStyleOptions":  layoutStyleOptions,
        @"gridSize":            @(g->GetGridSize()),
        @"centerX":             @((double)g->GetCentreX()),
        @"centerY":             @((double)g->GetCentreY()),
        @"centerDefined":       @(g->GetCentreDefined() ? YES : NO),
        @"locked":              @(g->GetBaseObjectScreenLocation().IsLocked() ? YES : NO),
        @"tagColor":            [NSString stringWithUTF8String:g->GetTagColourAsString().c_str()],
    };
}

- (BOOL)setLayoutModelGroupProperty:(NSString*)name
                                key:(NSString*)key
                              value:(id)value {
    if (!_context || !_context->HasModelManager() || !name || !key) return NO;
    Model* m = _context->GetModelManager()[std::string([name UTF8String])];
    if (!m || m->GetDisplayAs() != DisplayAsType::ModelGroup) return NO;
    _context->AbortRender(5000);
    ModelGroup* g = static_cast<ModelGroup*>(m);

    std::string keyStr = [key UTF8String];
    BOOL changed = NO;

    if (keyStr == "layoutGroup") {
        if (![value isKindOfClass:[NSString class]]) return NO;
        std::string newGroup = [(NSString*)value UTF8String];
        if (newGroup != g->GetLayoutGroup()) { g->SetLayoutGroup(newGroup); changed = YES; }
    } else if (keyStr == "locked") {
        if (![value isKindOfClass:[NSNumber class]]) return NO;
        bool v = [(NSNumber*)value boolValue] ? true : false;
        if (v != g->GetBaseObjectScreenLocation().IsLocked()) {
            g->GetBaseObjectScreenLocation().SetLocked(v); changed = YES;
        }
    } else if (keyStr == "defaultCamera") {
        if (![value isKindOfClass:[NSString class]]) return NO;
        std::string newCam = [(NSString*)value UTF8String];
        if (newCam != g->GetDefaultCamera()) { g->SetDefaultCamera(newCam); changed = YES; }
    } else if (keyStr == "layout") {
        if (![value isKindOfClass:[NSString class]]) return NO;
        std::string newLayout = [(NSString*)value UTF8String];
        if (newLayout != g->GetLayout()) { g->SetLayout(newLayout); changed = YES; }
    } else if (keyStr == "gridSize") {
        if (![value isKindOfClass:[NSNumber class]]) return NO;
        int v = [(NSNumber*)value intValue];
        if (v != g->GetGridSize()) { g->SetGridSize(v); changed = YES; }
    } else if (keyStr == "centerX") {
        if (![value isKindOfClass:[NSNumber class]]) return NO;
        float v = (float)[(NSNumber*)value doubleValue];
        if (v != g->GetCentreX()) { g->SetCentreX(v); g->SetCentreDefined(true); changed = YES; }
    } else if (keyStr == "centerY") {
        if (![value isKindOfClass:[NSNumber class]]) return NO;
        float v = (float)[(NSNumber*)value doubleValue];
        if (v != g->GetCentreY()) { g->SetCentreY(v); g->SetCentreDefined(true); changed = YES; }
    } else if (keyStr == "tagColor") {
        if (![value isKindOfClass:[NSString class]]) return NO;
        std::string newCol = ((NSString*)value).UTF8String;
        if (newCol != g->GetTagColourAsString()) {
            g->SetTagColourAsString(newCol);
            changed = YES;
        }
    } else if (keyStr == "members") {
        // J-9 — replace the entire member list (drag-to-reorder or
        // bulk move). Value is NSArray<NSString*>.
        if (![value isKindOfClass:[NSArray class]]) return NO;
        std::vector<std::string> newMembers;
        newMembers.reserve(((NSArray*)value).count);
        for (id entry in (NSArray*)value) {
            if ([entry isKindOfClass:[NSString class]]) {
                newMembers.emplace_back(((NSString*)entry).UTF8String);
            }
        }
        if (newMembers != g->ModelNames()) {
            g->SetModels(newMembers);
            changed = YES;
        }
    } else {
        spdlog::warn("setLayoutModelGroupProperty: unknown key '{}' for group '{}'",
                     keyStr, [name UTF8String]);
        return NO;
    }

    if (changed) {
        _context->MarkLayoutModelDirty(std::string([name UTF8String]));
    }
    return changed;
}

// MARK: - Layout Editor (Phase J-7, Group CRUD)

- (BOOL)addModel:(NSString*)modelName
         toGroup:(NSString*)groupName {
    if (!_context || !_context->HasModelManager() || !modelName || !groupName) return NO;
    Model* g = _context->GetModelManager()[groupName.UTF8String];
    if (!g || g->GetDisplayAs() != DisplayAsType::ModelGroup) return NO;
    auto* grp = static_cast<ModelGroup*>(g);
    Model* m = _context->GetModelManager()[modelName.UTF8String];
    if (!m) return NO;
    // Bail if already a direct member (no need to mark dirty).
    if (grp->DirectlyContainsModel(modelName.UTF8String)) return NO;
    _context->AbortRender(5000);
    grp->AddModel(modelName.UTF8String);
    _context->MarkLayoutModelDirty(groupName.UTF8String);
    return YES;
}

- (BOOL)removeModel:(NSString*)modelName
          fromGroup:(NSString*)groupName {
    if (!_context || !_context->HasModelManager() || !modelName || !groupName) return NO;
    Model* g = _context->GetModelManager()[groupName.UTF8String];
    if (!g || g->GetDisplayAs() != DisplayAsType::ModelGroup) return NO;
    auto* grp = static_cast<ModelGroup*>(g);
    std::string target = modelName.UTF8String;
    const auto& current = grp->ModelNames();
    if (std::find(current.begin(), current.end(), target) == current.end()) {
        return NO;
    }
    _context->AbortRender(5000);
    std::vector<std::string> updated;
    updated.reserve(current.size());
    for (const auto& s : current) {
        if (s != target) updated.push_back(s);
    }
    grp->SetModels(updated);
    _context->MarkLayoutModelDirty(groupName.UTF8String);
    return YES;
}

- (BOOL)createModelGroup:(NSString*)groupName
                  members:(NSArray<NSString*>*)initialMembers {
    if (!_context || !_context->HasModelManager() ||
        !groupName || groupName.length == 0) return NO;
    // J-16 — sanitize via the canonical desktop helper. The
    // SwiftUI sheet should preview the sanitized form so this is
    // defense-in-depth rather than a surprise to the user.
    std::string name = Model::SafeModelName(groupName.UTF8String);
    if (name.empty()) return NO;
    auto& mgr = _context->GetModelManager();
    // Collision check — desktop allows model + group to share a
    // name but it confuses every selection lookup; refuse.
    if (mgr.GetModel(name)) return NO;

    _context->AbortRender(5000);
    auto* grp = new ModelGroup(mgr);
    grp->SetName(name);
    grp->SetLayout("minimalGrid");
    grp->SetGridSize(400);
    grp->SetDefaultCamera("2D");
    grp->SetLayoutGroup(LayoutGroupForNewItem(_context->GetActiveLayoutGroup()));
    grp->SetPreviewSize(_context->GetPreviewWidth(),
                         _context->GetPreviewHeight());

    mgr.AddModel(grp);

    if (initialMembers && initialMembers.count > 0) {
        std::vector<std::string> memberVec;
        memberVec.reserve(initialMembers.count);
        for (NSString* s in initialMembers) {
            if (s.length > 0) memberVec.emplace_back(s.UTF8String);
        }
        grp->SetModels(memberVec);
    }

    _context->MarkGroupCreated(name);
    return YES;
}

- (BOOL)deleteModelGroup:(NSString*)groupName {
    if (!_context || !_context->HasModelManager() ||
        !groupName || groupName.length == 0) return NO;
    Model* g = _context->GetModelManager()[groupName.UTF8String];
    if (!g || g->GetDisplayAs() != DisplayAsType::ModelGroup) return NO;
    std::string name = groupName.UTF8String;
    _context->AbortRender(5000);
    if (!_context->GetModelManager().Delete(name)) return NO;
    _context->MarkGroupDeleted(name);
    return YES;
}

- (BOOL)renameModelGroup:(NSString*)oldName to:(NSString*)newName {
    if (!_context || !_context->HasModelManager()) return NO;
    if (!oldName || !newName) return NO;
    std::string oldStd = oldName.UTF8String;
    // J-16 — sanitize the new name through the same canonical
    // path the desktop uses. Strips illegal characters silently
    // (`, ~ ! ; < > " ' & : | @ / \ \t \r \n`) rather than
    // rejecting them, matching the desktop convention.
    std::string newStd = Model::SafeModelName(newName.UTF8String);
    if (newStd.empty() || oldStd == newStd) return NO;

    auto& mgr = _context->GetModelManager();
    Model* g = mgr[oldStd];
    if (!g || g->GetDisplayAs() != DisplayAsType::ModelGroup) return NO;
    // Refuse collision with an existing model or group.
    if (mgr.GetModel(newStd)) return NO;

    _context->AbortRender(5000);
    // ModelManager::Rename updates the in-memory references
    // (group's own name, plus member-name vectors of every other
    // ModelGroup that contained the old name). It does NOT mark
    // anything dirty for save — we do that below.
    if (!mgr.Rename(oldStd, newStd)) return NO;

    _context->MarkGroupRenamed(oldStd, newStd);
    _context->MarkLayoutModelDirty(newStd);

    // Mark every group that now contains the new name dirty so
    // their on-disk `models` comma-list gets rewritten. We can't
    // tell from Rename's return value which groups were affected,
    // so we walk and check membership. Cheap on realistic show
    // sizes.
    for (const auto& [name, model] : mgr.GetModels()) {
        if (!model) continue;
        if (model->GetDisplayAs() != DisplayAsType::ModelGroup) continue;
        auto* mg = static_cast<ModelGroup*>(model);
        if (mg->DirectlyContainsModel(newStd)) {
            _context->MarkLayoutModelDirty(name);
        }
    }
    return YES;
}

- (NSString*)sanitizedModelName:(NSString*)name {
    if (!name) return @"";
    std::string clean = Model::SafeModelName(name.UTF8String);
    return [NSString stringWithUTF8String:clean.c_str()];
}

- (NSArray<NSString*>*)submodelsForModel:(NSString*)modelName {
    NSMutableArray<NSString*>* out = [NSMutableArray array];
    if (!_context || !_context->HasModelManager() || !modelName) return out;
    Model* m = _context->GetModelManager()[modelName.UTF8String];
    if (!m) return out;
    for (Model* sub : m->GetSubModels()) {
        if (!sub) continue;
        SubModel* sm = dynamic_cast<SubModel*>(sub);
        if (!sm) continue;
        [out addObject:[NSString stringWithUTF8String:sm->GetFullName().c_str()]];
    }
    return out;
}

// J-8 — name of the synthetic "2D Background" pseudo-object that
// always appears at the top of the Objects tab. Used by the
// view-object summary / setter to route through to the active
// layout group's background settings instead of ViewObjectManager.
static NSString* const kBackgroundPseudoObjectName = @"2D Background";

// View-object equivalent of MatchesActiveLayoutGroupForRoster.
// Empty layout_group is treated as Default (legacy storage) for
// the named-preview paths.
static bool MatchesActiveLayoutGroupForViewObject(const std::string& voGroup,
                                                    const std::string& active) {
    if (active == "All Models")  return true;
    if (active == "Unassigned")  return voGroup == "Unassigned";
    return voGroup == active || voGroup == "All Previews" || voGroup.empty();
}

- (NSArray<NSString*>*)viewObjectsInActiveLayoutGroup {
    NSMutableArray<NSString*>* out = [NSMutableArray array];
    if (!_context) return out;
    // J-8 — always surface the 2D Background pseudo-object first
    // so the user can edit it even on a brand-new show with no
    // real view objects.
    [out addObject:kBackgroundPseudoObjectName];
    if (!_context->HasViewObjectManager()) return out;
    const std::string active = _context->GetActiveLayoutGroup();
    ViewObjectManager& vm = _context->GetAllObjects();
    for (auto it = vm.begin(); it != vm.end(); ++it) {
        ViewObject* vo = it->second;
        if (!vo) continue;
        if (MatchesActiveLayoutGroupForViewObject(vo->GetLayoutGroup(), active)) {
            [out addObject:[NSString stringWithUTF8String:vo->GetName().c_str()]];
        }
    }
    return out;
}

- (NSArray<NSDictionary<NSString*, id>*>*)viewObjectsListSummary {
    NSMutableArray<NSDictionary<NSString*, id>*>* out = [NSMutableArray array];
    if (!_context || !_context->HasViewObjectManager()) return out;
    const std::string active = _context->GetActiveLayoutGroup();
    ViewObjectManager& vm = _context->GetAllObjects();
    for (auto it = vm.begin(); it != vm.end(); ++it) {
        ViewObject* vo = it->second;
        if (!vo) continue;
        if (!MatchesActiveLayoutGroupForViewObject(vo->GetLayoutGroup(), active)) continue;
        [out addObject:@{
            @"name":       [NSString stringWithUTF8String:vo->GetName().c_str()],
            @"displayAs":  [NSString stringWithUTF8String:vo->GetDisplayAsString().c_str()],
            @"isFromBase": @(vo->IsFromBase() ? YES : NO),
        }];
    }
    return out;
}

- (nullable NSDictionary<NSString*, id>*)viewObjectLayoutSummary:(NSString*)name {
    if (!_context || !name) return nil;
    // J-8 — synthetic 2D Background pseudo-object. Schema diverges
    // from real view objects: only the four background attributes
    // are meaningful. The "isBackground" sentinel lets the SwiftUI
    // renderer pick a different layout.
    if ([name isEqualToString:kBackgroundPseudoObjectName]) {
        NSString* img = [NSString stringWithUTF8String:_context->GetActiveBackgroundImage().c_str()];
        NSString* group = [NSString stringWithUTF8String:_context->GetActiveLayoutGroup().c_str()];
        return @{
            @"name":          name,
            @"displayAs":     @"2D Background",
            @"isBackground":  @YES,
            @"layoutGroup":   group,
            @"backgroundImage":      img,
            @"backgroundBrightness": @(_context->GetActiveBackgroundBrightness()),
            @"backgroundAlpha":      @(_context->GetActiveBackgroundAlpha()),
            @"scaleBackgroundImage": @(_context->GetActiveScaleBackgroundImage() ? YES : NO),
            // J-19 — layout-group display roll-up (was previously
            // a separate read-only block at the bottom of the
            // Models tab — desktop treats these as preview-level
            // settings, so they belong on the Background object).
            @"previewWidth":         @(_context->GetPreviewWidth()),
            @"previewHeight":        @(_context->GetPreviewHeight()),
            @"display2DCenter0":     @(_context->GetDisplay2DCenter0() ? YES : NO),
            @"display2DGrid":        @(_context->GetDisplay2DGrid() ? YES : NO),
            @"display2DGridSpacing": @((long)_context->GetDisplay2DGridSpacing()),
            @"display2DBoundingBox": @(_context->GetDisplay2DBoundingBox() ? YES : NO),
        };
    }
    if (!_context->HasViewObjectManager()) return nil;
    ViewObject* vo = _context->GetAllObjects().GetViewObject(std::string([name UTF8String]));
    if (!vo) return nil;

    auto& loc = vo->GetObjectScreenLocation();
    glm::vec3 rot = loc.GetRotation();

    NSMutableDictionary<NSString*, id>* out = [@{
        @"name":        [NSString stringWithUTF8String:vo->GetName().c_str()],
        @"displayAs":   [NSString stringWithUTF8String:vo->GetDisplayAsString().c_str()],
        @"layoutGroup": [NSString stringWithUTF8String:vo->GetLayoutGroup().c_str()],
        @"active":      @(vo->IsActive() ? YES : NO),
        @"centerX":     @((double)loc.GetHcenterPos()),
        @"centerY":     @((double)loc.GetVcenterPos()),
        @"centerZ":     @((double)loc.GetDcenterPos()),
        @"width":       @((double)loc.GetMWidth()),
        @"height":      @((double)loc.GetMHeight()),
        @"depth":       @((double)loc.GetMDepth()),
        @"rotateX":     @((double)rot.x),
        @"rotateY":     @((double)rot.y),
        @"rotateZ":     @((double)rot.z),
        @"locked":      @(loc.IsLocked() ? YES : NO),
    } mutableCopy];

    // J-12 — per-type fields. The `typeKind` discriminator lets
    // the SwiftUI renderer pick the right control set without
    // string-matching against `displayAs`.
    switch (vo->GetDisplayAs()) {
        case DisplayAsType::Mesh: {
            auto* m = dynamic_cast<MeshObject*>(vo);
            if (m) {
                out[@"typeKind"]   = @"mesh";
                out[@"objFile"]    = [NSString stringWithUTF8String:m->GetObjFile().c_str()];
                out[@"brightness"] = @(m->GetBrightness());
                out[@"meshOnly"]   = @(m->IsMeshOnly() ? YES : NO);
                // J-14 — Mesh/Image both use BoxedScreenLocation;
                // scaleX/Y/Z are the per-axis stretch factors
                // independent of the boxed width/height/depth.
                if (auto* bsl = dynamic_cast<BoxedScreenLocation*>(&loc)) {
                    out[@"scaleX"] = @((double)bsl->GetScaleX());
                    out[@"scaleY"] = @((double)bsl->GetScaleY());
                    out[@"scaleZ"] = @((double)bsl->GetScaleZ());
                }
            }
            break;
        }
        case DisplayAsType::Image: {
            auto* i = dynamic_cast<ImageObject*>(vo);
            if (i) {
                out[@"typeKind"]     = @"image";
                out[@"imageFile"]    = [NSString stringWithUTF8String:i->GetImageFile().c_str()];
                out[@"brightness"]   = @(i->GetBrightness());
                out[@"transparency"] = @(i->GetTransparency());
                if (auto* bsl = dynamic_cast<BoxedScreenLocation*>(&loc)) {
                    out[@"scaleX"] = @((double)bsl->GetScaleX());
                    out[@"scaleY"] = @((double)bsl->GetScaleY());
                    out[@"scaleZ"] = @((double)bsl->GetScaleZ());
                }
            }
            break;
        }
        case DisplayAsType::Gridlines: {
            auto* g = dynamic_cast<GridlinesObject*>(vo);
            if (g) {
                out[@"typeKind"]    = @"gridlines";
                out[@"gridSpacing"] = @(g->GetGridLineSpacing());
                out[@"gridWidth"]   = @(g->GetGridWidth());
                out[@"gridHeight"]  = @(g->GetGridHeight());
                out[@"gridColor"]   = [NSString stringWithUTF8String:g->GetGridColor().c_str()];
                out[@"hasAxis"]     = @(g->GetHasAxis() ? YES : NO);
                out[@"pointToFront"] = @(g->GetPointToFront() ? YES : NO);
            }
            break;
        }
        case DisplayAsType::Terrain: {
            auto* t = dynamic_cast<TerrainObject*>(vo);
            if (t) {
                out[@"typeKind"]     = @"terrain";
                out[@"imageFile"]    = [NSString stringWithUTF8String:t->GetImageFile().c_str()];
                out[@"brightness"]   = @(t->GetBrightness());
                out[@"transparency"] = @(t->GetTransparency());
                out[@"gridSpacing"]  = @(t->GetSpacing());
                out[@"gridWidth"]    = @(t->GetWidth());
                out[@"gridDepth"]    = @(t->GetDepth());
                out[@"hideGrid"]     = @(t->IsHideGrid() ? YES : NO);
                out[@"hideImage"]    = @(t->IsHideImage() ? YES : NO);
                out[@"gridColor"]    = [NSString stringWithUTF8String:t->GetGridColor().c_str()];
            }
            break;
        }
        case DisplayAsType::Ruler: {
            auto* r = dynamic_cast<RulerObject*>(vo);
            if (r) {
                out[@"typeKind"] = @"ruler";
                out[@"units"]    = @(RulerObject::GetUnits());
                out[@"length"]   = @((double)r->GetLength());
                out[@"unitOptions"] = @[@"m", @"cm", @"mm", @"yd", @"ft", @"in"];
                // J-14 — Ruler is a TwoPointScreenLocation: point 1
                // is the world origin, point 2 is offset by (x2,y2,z2).
                // Surface absolute coords for both so the user
                // doesn't have to do mental math.
                if (auto* tpl = dynamic_cast<TwoPointScreenLocation*>(&loc)) {
                    out[@"twoPoint"] = @YES;
                    out[@"p1X"] = @((double)tpl->GetWorldPos_X());
                    out[@"p1Y"] = @((double)tpl->GetWorldPos_Y());
                    out[@"p1Z"] = @((double)tpl->GetWorldPos_Z());
                    out[@"p2X"] = @((double)(tpl->GetX2() + tpl->GetWorldPos_X()));
                    out[@"p2Y"] = @((double)(tpl->GetY2() + tpl->GetWorldPos_Y()));
                    out[@"p2Z"] = @((double)(tpl->GetZ2() + tpl->GetWorldPos_Z()));
                }
            }
            break;
        }
        default:
            out[@"typeKind"] = @"other";
            break;
    }
    return out;
}

- (NSArray<NSString*>*)availableViewObjectTypes {
    NSMutableArray<NSString*>* out = [NSMutableArray array];
    [out addObject:@"Image"];
    [out addObject:@"Mesh"];
    [out addObject:@"Gridlines"];
    [out addObject:@"Terrain"];
    if (RulerObject::GetRuler() == nullptr) {
        [out addObject:@"Ruler"];
    }
    return out;
}

- (nullable NSString*)createViewObjectWithType:(NSString*)type {
    if (!_context || !_context->HasViewObjectManager() || !type) return nil;
    std::string t = type.UTF8String;
    _context->AbortRender(5000);
    auto& vm = _context->GetAllObjects();
    ViewObject* vo = vm.CreateAndAddObject(t);
    if (!vo) return nil;
    // Default to the active layout group so the new object
    // immediately renders in the current preview. "All Models"
    // is virtual — fall back to "Default" so the object is
    // actually stored against a real preview.
    vo->SetLayoutGroup(LayoutGroupForNewItem(_context->GetActiveLayoutGroup()));
    // Ruler is a TwoPoint object — without initial geometry both
    // endpoints sit at world origin so the line is zero-length and
    // invisible. Match desktop's Add Ruler defaults (100-unit
    // horizontal line at y=100, centred on x=0).
    if (t == "Ruler") {
        auto& loc = vo->GetObjectScreenLocation();
        loc.SetVcenterPos(100);
        loc.SetLeft(-50);
        loc.SetMWidth(100);
    }
    _context->MarkViewObjectCreated(vo->GetName());
    return [NSString stringWithUTF8String:vo->GetName().c_str()];
}

- (BOOL)deleteViewObject:(NSString*)name {
    if (!_context || !_context->HasViewObjectManager() || !name || name.length == 0) return NO;
    if ([name isEqualToString:kBackgroundPseudoObjectName]) return NO;
    std::string nm = name.UTF8String;
    if (!_context->GetAllObjects().GetViewObject(nm)) return NO;
    _context->AbortRender(5000);
    _context->GetAllObjects().Delete(nm);
    _context->MarkViewObjectDeleted(nm);
    return YES;
}

- (BOOL)renameViewObject:(NSString*)oldName to:(NSString*)newName {
    if (!_context || !_context->HasViewObjectManager()) return NO;
    if (!oldName || !newName) return NO;
    if ([oldName isEqualToString:kBackgroundPseudoObjectName]) return NO;
    std::string oldStd = oldName.UTF8String;
    std::string newStd = Model::SafeModelName(newName.UTF8String);
    if (newStd.empty() || oldStd == newStd) return NO;

    auto& vm = _context->GetAllObjects();
    if (!vm.GetViewObject(oldStd)) return NO;
    if (vm.GetViewObject(newStd)) return NO;

    _context->AbortRender(5000);
    vm.Rename(oldStd, newStd);
    // ViewObjectManager::Rename returns `changed` based on cross-
    // reference iteration that's currently commented out on the
    // desktop, so we verify by lookup.
    if (!vm.GetViewObject(newStd)) return NO;

    _context->MarkViewObjectRenamed(oldStd, newStd);
    _context->MarkLayoutViewObjectDirty(newStd);
    return YES;
}

- (nullable NSString*)duplicateViewObject:(NSString*)name {
    if (!_context || !_context->HasViewObjectManager()) return nil;
    if (!name || name.length == 0) return nil;
    if ([name isEqualToString:kBackgroundPseudoObjectName]) return nil;

    auto& vm = _context->GetAllObjects();
    ViewObject* src = vm.GetViewObject(name.UTF8String);
    if (!src) return nil;

    _context->AbortRender(5000);
    // Serialize source through the visitor to produce a fresh
    // <view_object> node, then deserialize into a new object.
    // Per-type attrs (Mesh ObjFile, Image bitmap, terrain heightmap)
    // round-trip via the visitor; no per-type copy code needed here.
    pugi::xml_document doc;
    pugi::xml_node parent = doc.append_child("view_objects");
    parent.append_attribute("type") = "exported";
    XmlSerializingVisitor visitor(parent, /*exporting*/ true);
    src->Accept(visitor);
    pugi::xml_node serObject = parent.first_child();
    if (!serObject) return nil;

    ViewObject* dup = vm.CreateObject(serObject);
    if (!dup) return nil;

    std::string newName = vm.GenerateObjectName(dup->GetName());
    dup->SetName(newName);
    dup->GetBaseObjectScreenLocation().Lock(false);
    // Offset world position so the duplicate doesn't sit on top
    // of the source. Matches desktop's paste-VO offset.
    dup->AddOffset(50.0, 50.0, 0.0);
    vm.AddViewObject(dup);
    _context->MarkViewObjectCreated(newName);
    return [NSString stringWithUTF8String:newName.c_str()];
}

- (BOOL)setLayoutViewObjectProperty:(NSString*)name
                                key:(NSString*)key
                              value:(id)value {
    if (!_context || !name || !key) return NO;
    _context->AbortRender(5000);
    // J-8 — route 2D Background edits to the active-group
    // background settings on iPadRenderContext. These don't ride
    // the regular view-object dirty path; SaveLayoutChanges has
    // a dedicated patcher for `<settings>` / `<layoutGroup>`.
    if ([name isEqualToString:kBackgroundPseudoObjectName]) {
        std::string k = key.UTF8String;
        if (k == "backgroundImage") {
            if (![value isKindOfClass:[NSString class]]) return NO;
            return _context->SetActiveBackgroundImage(((NSString*)value).UTF8String) ? YES : NO;
        } else if (k == "backgroundBrightness") {
            if (![value isKindOfClass:[NSNumber class]]) return NO;
            return _context->SetActiveBackgroundBrightness([(NSNumber*)value intValue]) ? YES : NO;
        } else if (k == "backgroundAlpha") {
            if (![value isKindOfClass:[NSNumber class]]) return NO;
            return _context->SetActiveBackgroundAlpha([(NSNumber*)value intValue]) ? YES : NO;
        } else if (k == "scaleBackgroundImage") {
            if (![value isKindOfClass:[NSNumber class]]) return NO;
            return _context->SetActiveScaleBackgroundImage([(NSNumber*)value boolValue] ? true : false) ? YES : NO;
        }
        spdlog::warn("setLayoutViewObjectProperty: unknown background key '{}'", k);
        return NO;
    }
    if (!_context->HasViewObjectManager()) return NO;
    ViewObject* vo = _context->GetAllObjects().GetViewObject(std::string([name UTF8String]));
    if (!vo) return NO;
    auto& loc = vo->GetObjectScreenLocation();
    std::string k = key.UTF8String;

    auto asDouble = ^double(BOOL* ok) {
        if ([value isKindOfClass:[NSNumber class]]) { *ok = YES; return [(NSNumber*)value doubleValue]; }
        *ok = NO; return 0;
    };
    auto asBool = ^BOOL(BOOL* ok) {
        if ([value isKindOfClass:[NSNumber class]]) { *ok = YES; return [(NSNumber*)value boolValue]; }
        *ok = NO; return NO;
    };
    auto asString = ^NSString*(BOOL* ok) {
        if ([value isKindOfClass:[NSString class]]) { *ok = YES; return (NSString*)value; }
        *ok = NO; return nil;
    };

    BOOL ok = NO;
    BOOL changed = NO;

    if (k == "centerX") {
        double v = asDouble(&ok); if (!ok) return NO;
        if ((float)v != loc.GetHcenterPos()) { vo->SetHcenterPos((float)v); changed = YES; }
    } else if (k == "centerY") {
        double v = asDouble(&ok); if (!ok) return NO;
        if ((float)v != loc.GetVcenterPos()) { vo->SetVcenterPos((float)v); changed = YES; }
    } else if (k == "centerZ") {
        double v = asDouble(&ok); if (!ok) return NO;
        if ((float)v != loc.GetDcenterPos()) { vo->SetDcenterPos((float)v); changed = YES; }
    } else if (k == "width") {
        double v = asDouble(&ok); if (!ok) return NO;
        if ((float)v != loc.GetMWidth()) { vo->SetWidth((float)v); changed = YES; }
    } else if (k == "height") {
        double v = asDouble(&ok); if (!ok) return NO;
        if ((float)v != loc.GetMHeight()) { vo->SetHeight((float)v); changed = YES; }
    } else if (k == "depth") {
        double v = asDouble(&ok); if (!ok) return NO;
        if ((float)v != loc.GetMDepth()) { vo->SetDepth((float)v); changed = YES; }
    } else if (k == "rotateX") {
        double v = asDouble(&ok); if (!ok) return NO;
        if ((float)v != loc.GetRotation().x) { loc.SetRotateX((float)v); changed = YES; }
    } else if (k == "rotateY") {
        double v = asDouble(&ok); if (!ok) return NO;
        if ((float)v != loc.GetRotation().y) { loc.SetRotateY((float)v); changed = YES; }
    } else if (k == "rotateZ") {
        double v = asDouble(&ok); if (!ok) return NO;
        if ((float)v != loc.GetRotation().z) { loc.SetRotateZ((float)v); changed = YES; }
    } else if (k == "locked") {
        BOOL v = asBool(&ok); if (!ok) return NO;
        if ((v ? true : false) != loc.IsLocked()) { loc.SetLocked(v ? true : false); changed = YES; }
    } else if (k == "layoutGroup") {
        NSString* s = asString(&ok); if (!ok) return NO;
        std::string newGroup = s.UTF8String;
        if (newGroup != vo->GetLayoutGroup()) { vo->SetLayoutGroup(newGroup); changed = YES; }
    } else if (k == "active") {
        BOOL v = asBool(&ok); if (!ok) return NO;
        if ((v ? true : false) != vo->IsActive()) { vo->SetActive(v ? true : false); changed = YES; }
    }
    // J-12 — per-type setters. Each branch checks dynamic_cast
    // before writing so a key sent to the wrong type silently
    // no-ops rather than crashing.
    else if (k == "objFile") {
        NSString* s = asString(&ok); if (!ok) return NO;
        if (auto* m = dynamic_cast<MeshObject*>(vo)) {
            std::string p = s.UTF8String;
            if (p != m->GetObjFile()) {
                m->SetObjectFile(p); changed = YES;
            }
        }
    } else if (k == "imageFile") {
        NSString* s = asString(&ok); if (!ok) return NO;
        std::string p = s.UTF8String;
        if (auto* i = dynamic_cast<ImageObject*>(vo)) {
            if (p != i->GetImageFile()) { i->SetImageFile(p); changed = YES; }
        } else if (auto* t = dynamic_cast<TerrainObject*>(vo)) {
            if (p != t->GetImageFile()) { t->SetImageFile(p); changed = YES; }
        }
    } else if (k == "brightness") {
        double v = asDouble(&ok); if (!ok) return NO;
        if (auto* m = dynamic_cast<MeshObject*>(vo)) {
            if ((int)v != m->GetBrightness()) { m->SetBrightness((int)v); changed = YES; }
        } else if (auto* i = dynamic_cast<ImageObject*>(vo)) {
            if ((int)v != i->GetBrightness()) { i->SetBrightness((float)v); changed = YES; }
        } else if (auto* t = dynamic_cast<TerrainObject*>(vo)) {
            if ((int)v != t->GetBrightness()) { t->SetBrightness((float)v); changed = YES; }
        }
    } else if (k == "transparency") {
        double v = asDouble(&ok); if (!ok) return NO;
        if (auto* i = dynamic_cast<ImageObject*>(vo)) {
            if ((int)v != i->GetTransparency()) { i->SetTransparency((int)v); changed = YES; }
        } else if (auto* t = dynamic_cast<TerrainObject*>(vo)) {
            if ((int)v != t->GetTransparency()) { t->SetTransparency((int)v); changed = YES; }
        }
    } else if (k == "gridSpacing") {
        double v = asDouble(&ok); if (!ok) return NO;
        if (auto* g = dynamic_cast<GridlinesObject*>(vo)) {
            if ((int)v != g->GetGridLineSpacing()) { g->SetGridLineSpacing((int)v); changed = YES; }
        } else if (auto* t = dynamic_cast<TerrainObject*>(vo)) {
            if ((int)v != t->GetSpacing()) { t->SetSpacing((int)v); changed = YES; }
        }
    } else if (k == "gridWidth") {
        double v = asDouble(&ok); if (!ok) return NO;
        if (auto* g = dynamic_cast<GridlinesObject*>(vo)) {
            if ((int)v != g->GetGridWidth()) { g->SetGridWidth((int)v); changed = YES; }
        } else if (auto* t = dynamic_cast<TerrainObject*>(vo)) {
            if ((int)v != t->GetWidth()) { t->SetWidth((int)v); changed = YES; }
        }
    } else if (k == "gridHeight") {
        double v = asDouble(&ok); if (!ok) return NO;
        if (auto* g = dynamic_cast<GridlinesObject*>(vo)) {
            if ((int)v != g->GetGridHeight()) { g->SetGridHeight((int)v); changed = YES; }
        }
    } else if (k == "gridDepth") {
        double v = asDouble(&ok); if (!ok) return NO;
        if (auto* t = dynamic_cast<TerrainObject*>(vo)) {
            if ((int)v != t->GetDepth()) { t->SetDepth((int)v); changed = YES; }
        }
    } else if (k == "gridColor") {
        NSString* s = asString(&ok); if (!ok) return NO;
        std::string col = s.UTF8String;
        if (auto* g = dynamic_cast<GridlinesObject*>(vo)) {
            if (col != g->GetGridColor()) { g->SetGridColor(col); changed = YES; }
        } else if (auto* t = dynamic_cast<TerrainObject*>(vo)) {
            if (col != t->GetGridColor()) { t->SetGridColor(col); changed = YES; }
        }
    } else if (k == "hasAxis") {
        BOOL v = asBool(&ok); if (!ok) return NO;
        if (auto* g = dynamic_cast<GridlinesObject*>(vo)) {
            if ((v?true:false) != g->GetHasAxis()) { g->SetHasAxis(v?true:false); changed = YES; }
        }
    } else if (k == "pointToFront") {
        BOOL v = asBool(&ok); if (!ok) return NO;
        if (auto* g = dynamic_cast<GridlinesObject*>(vo)) {
            if ((v?true:false) != g->GetPointToFront()) { g->SetPointToFront(v?true:false); changed = YES; }
        }
    } else if (k == "hideGrid") {
        BOOL v = asBool(&ok); if (!ok) return NO;
        if (auto* t = dynamic_cast<TerrainObject*>(vo)) {
            if ((v?true:false) != t->IsHideGrid()) { t->SetHideGrid(v?true:false); changed = YES; }
        }
    } else if (k == "hideImage") {
        BOOL v = asBool(&ok); if (!ok) return NO;
        if (auto* t = dynamic_cast<TerrainObject*>(vo)) {
            if ((v?true:false) != t->IsHideImage()) { t->SetHideImage(v?true:false); changed = YES; }
        }
    } else if (k == "units") {
        double v = asDouble(&ok); if (!ok) return NO;
        if (auto* r = dynamic_cast<RulerObject*>(vo)) {
            if ((int)v != RulerObject::GetUnits()) { r->SetUnits((int)v); changed = YES; }
        }
    } else if (k == "length") {
        double v = asDouble(&ok); if (!ok) return NO;
        // Mirrors RulerObjectPropertyAdapter::OnPropertyGridChange.
        if (v < 0.01) v = 0.01;
        if (auto* r = dynamic_cast<RulerObject*>(vo)) {
            if ((float)v != r->GetLength()) { r->SetLength((float)v); changed = YES; }
        }
    }
    // J-14 — Mesh-only toggle on MeshObject.
    else if (k == "meshOnly") {
        BOOL v = asBool(&ok); if (!ok) return NO;
        if (auto* m = dynamic_cast<MeshObject*>(vo)) {
            if ((v?true:false) != m->IsMeshOnly()) { m->SetMeshOnly(v?true:false); changed = YES; }
        }
    }
    // J-14 — BoxedScreenLocation scale factors (Mesh + Image).
    else if (k == "scaleX") {
        double v = asDouble(&ok); if (!ok) return NO;
        if (auto* bsl = dynamic_cast<BoxedScreenLocation*>(&loc)) {
            if ((float)v != bsl->GetScaleX()) { bsl->SetScaleX((float)v); changed = YES; }
        }
    } else if (k == "scaleY") {
        double v = asDouble(&ok); if (!ok) return NO;
        if (auto* bsl = dynamic_cast<BoxedScreenLocation*>(&loc)) {
            if ((float)v != bsl->GetScaleY()) { bsl->SetScaleY((float)v); changed = YES; }
        }
    } else if (k == "scaleZ") {
        double v = asDouble(&ok); if (!ok) return NO;
        if (auto* bsl = dynamic_cast<BoxedScreenLocation*>(&loc)) {
            if ((float)v != bsl->GetScaleZ()) { bsl->SetScaleZ((float)v); changed = YES; }
        }
    }
    // J-14 — TwoPointScreenLocation endpoints (Ruler). p1 = world
    // pos; p2 = absolute, stored internally as (x2,y2,z2) offset.
    // Setting p1 keeps point 2 absolute by rebasing the offset.
    else if (k == "p1X" || k == "p1Y" || k == "p1Z" ||
             k == "p2X" || k == "p2Y" || k == "p2Z") {
        double v = asDouble(&ok); if (!ok) return NO;
        auto* tpl = dynamic_cast<TwoPointScreenLocation*>(&loc);
        if (!tpl) return NO;
        if (k == "p1X") {
            float oldAbs = tpl->GetX2() + tpl->GetWorldPos_X();
            if ((float)v != tpl->GetWorldPos_X()) {
                tpl->SetWorldPos_X((float)v);
                tpl->SetX2(oldAbs - (float)v);
                changed = YES;
            }
        } else if (k == "p1Y") {
            float oldAbs = tpl->GetY2() + tpl->GetWorldPos_Y();
            if ((float)v != tpl->GetWorldPos_Y()) {
                tpl->SetWorldPos_Y((float)v);
                tpl->SetY2(oldAbs - (float)v);
                changed = YES;
            }
        } else if (k == "p1Z") {
            float oldAbs = tpl->GetZ2() + tpl->GetWorldPos_Z();
            if ((float)v != tpl->GetWorldPos_Z()) {
                tpl->SetWorldPos_Z((float)v);
                tpl->SetZ2(oldAbs - (float)v);
                changed = YES;
            }
        } else if (k == "p2X") {
            float newOffset = (float)v - tpl->GetWorldPos_X();
            if (newOffset != tpl->GetX2()) { tpl->SetX2(newOffset); changed = YES; }
        } else if (k == "p2Y") {
            float newOffset = (float)v - tpl->GetWorldPos_Y();
            if (newOffset != tpl->GetY2()) { tpl->SetY2(newOffset); changed = YES; }
        } else if (k == "p2Z") {
            float newOffset = (float)v - tpl->GetWorldPos_Z();
            if (newOffset != tpl->GetZ2()) { tpl->SetZ2(newOffset); changed = YES; }
        }
    } else {
        spdlog::warn("setLayoutViewObjectProperty: unknown key '{}' for view object '{}'",
                     k, name.UTF8String);
        return NO;
    }

    if (changed) {
        vo->IncrementChangeCount();
        vo->ReloadModel();
        _context->MarkLayoutViewObjectDirty(name.UTF8String);
    }
    return changed;
}

// MARK: - Layout Editor (Phase J-6, per-type properties)

// Static helpers for building descriptor dictionaries. Each returns
// a fresh NSMutableDictionary so callers can append optional keys
// (help, group, enabled) without going back through a builder.
static NSMutableDictionary* MakeIntProp(NSString* key, NSString* label,
                                         int value, int minV, int maxV) {
    return [@{
        @"key": key, @"label": label, @"kind": @"int",
        @"value": @(value), @"min": @(minV), @"max": @(maxV),
    } mutableCopy];
}
static NSMutableDictionary* MakeDoubleProp(NSString* key, NSString* label,
                                            double value, double minV, double maxV,
                                            double step, int precision) {
    return [@{
        @"key": key, @"label": label, @"kind": @"double",
        @"value": @(value), @"min": @(minV), @"max": @(maxV),
        @"step": @(step), @"precision": @(precision),
    } mutableCopy];
}
static NSMutableDictionary* MakeBoolProp(NSString* key, NSString* label, BOOL value) {
    return [@{
        @"key": key, @"label": label, @"kind": @"bool",
        @"value": @(value ? YES : NO),
    } mutableCopy];
}
static NSMutableDictionary* MakeEnumProp(NSString* key, NSString* label,
                                          int index, NSArray<NSString*>* options) {
    return [@{
        @"key": key, @"label": label, @"kind": @"enum",
        @"value": @(index), @"options": options,
    } mutableCopy];
}
static NSMutableDictionary* MakeStringProp(NSString* key, NSString* label,
                                            NSString* _Nullable value) {
    return [@{
        @"key": key, @"label": label, @"kind": @"string",
        @"value": value ?: @"",
    } mutableCopy];
}

// J-3 (DMX) — a non-editable section divider inside the per-type
// descriptor stream. Renders full-width in the property panel so
// DMX subsections (Color / Shutter / Beam) read as their own
// blocks rather than a flat key list. The key needs to be unique
// so SwiftUI's ForEach doesn't dedupe headers that share a label
// — caller picks a short identifier.
static NSMutableDictionary* MakeHeaderDescriptor(NSString* key, NSString* label) {
    return [@{
        @"key": key, @"label": label, @"kind": @"header",
    } mutableCopy];
}

// J-30 — one-shot action button inside the per-type descriptor
// stream. SwiftUI renders a button with the supplied label;
// tapping it fires `typeCommit(key, @YES)` which the setter
// interprets as "perform this side-effect". Used for the
// Skulltronix preset on DmxSkull; could host other one-shot
// model operations later (e.g. "Reset Defaults").
static NSMutableDictionary* MakeButtonDescriptor(NSString* key, NSString* label) {
    return [@{
        @"key": key, @"label": label, @"kind": @"button",
    } mutableCopy];
}

// Common "Top Left / Top Right / Bottom Left / Bottom Right"
// starting-location combo. Encoded as a 0..3 index matching the
// desktop's `MatrixStart` / `WreathStart` enum order.
static NSArray<NSString*>* StartCornerOptions() {
    return @[@"Top Left", @"Top Right", @"Bottom Left", @"Bottom Right"];
}
static int EncodeStartCorner(const Model* m) {
    return m->GetIsLtoR() ? (m->GetIsBtoT() ? 2 : 0) : (m->GetIsBtoT() ? 3 : 1);
}
static void ApplyStartCorner(Model* m, int idx) {
    m->SetDirection(idx == 0 || idx == 2 ? "L" : "R");
    m->SetStartSide(idx == 0 || idx == 1 ? "T" : "B");
    m->SetIsLtoR(idx == 0 || idx == 2);
    m->SetIsBtoT(idx >= 2);
}

// Per-type builders. Each appends descriptor dictionaries to `out`
// reading the model's current state. Order matches the desktop
// adapter's display order so the iPad UI feels familiar.

static void BuildMatrixProps(MatrixModel* m, NSMutableArray* out) {
    [out addObject:MakeEnumProp(@"MatrixStyle", @"Direction",
                                 m->isVerticalMatrix() ? 1 : 0,
                                 @[@"Horizontal", @"Vertical"])];
    [out addObject:MakeBoolProp(@"AlternateNodes", @"Alternate Nodes",
                                 m->HasAlternateNodes())];
    [out addObject:MakeBoolProp(@"NoZig", @"Don't Zig Zag",
                                 m->IsNoZigZag())];
    [out addObject:MakeIntProp(@"MatrixStringCount", @"# Strings",
                                m->GetNumPhysicalStrings(), 1, 10000)];
    NSString* lpsLabel = m->IsSingleNode() ? @"Lights/String" : @"Nodes/String";
    [out addObject:MakeIntProp(@"MatrixLightCount", lpsLabel,
                                m->GetNodesPerString(), 1, 10000)];
    [out addObject:MakeIntProp(@"MatrixStrandCount", @"Strands/String",
                                m->GetStrandsPerString(), 1, 2500)];
    [out addObject:MakeEnumProp(@"MatrixStart", @"Starting Location",
                                 EncodeStartCorner(m), StartCornerOptions())];
}

static void BuildTreeProps(TreeModel* t, NSMutableArray* out) {
    // J-20 — desktop TreePropertyAdapter overrides AddStyle
    // Properties: emits TreeStyle + the round-tree-only floats +
    // AlternateNodes + NoZig + "Strand Direction" (StrandDir).
    // The matrix "Direction" enum is replaced by StrandDir — we
    // build the tree props manually instead of calling
    // BuildMatrixProps so we don't double up.
    [out addObject:MakeEnumProp(@"TreeStyle", @"Type",
                                 t->GetTreeType(),
                                 @[@"Round", @"Flat", @"Ribbon"])];
    BOOL roundTree = (t->GetTreeType() == 0);
    NSMutableDictionary* d;
    d = MakeIntProp(@"TreeDegrees", @"Degrees",
                     roundTree ? (int)t->GetTreeDegrees() : 180, 1, 360);
    d[@"enabled"] = @(roundTree);
    [out addObject:d];
    d = MakeDoubleProp(@"TreeRotation", @"Rotation",
                        roundTree ? t->GetTreeRotation() : 3.0, -360, 360, 0.1, 2);
    d[@"enabled"] = @(roundTree);
    [out addObject:d];
    d = MakeDoubleProp(@"TreeSpiralRotations", @"Spiral Wraps",
                        roundTree ? t->GetSpiralRotations() : 0.0, -200, 200, 1, 2);
    d[@"enabled"] = @(roundTree);
    [out addObject:d];
    d = MakeDoubleProp(@"TreeBottomTopRatio", @"Bottom/Top Ratio",
                        roundTree ? t->GetBottomTopRatio() : 6.0, -50, 50, 0.5, 2);
    d[@"enabled"] = @(roundTree);
    [out addObject:d];
    d = MakeDoubleProp(@"TreePerspective", @"Perspective",
                        roundTree ? t->GetTreePerspective() * 10.0 : 2.0, 0, 10, 0.1, 2);
    d[@"enabled"] = @(roundTree);
    [out addObject:d];
    // Alternate Nodes / Don't Zig Zag are mutex with each other —
    // mirrored from MatrixPropertyAdapter::AddStyleProperties.
    NSMutableDictionary* an = MakeBoolProp(@"AlternateNodes", @"Alternate Nodes",
                                            t->HasAlternateNodes());
    an[@"enabled"] = @(t->IsNoZigZag() == false);
    [out addObject:an];
    NSMutableDictionary* nz = MakeBoolProp(@"NoZig", @"Don't Zig Zag",
                                            t->IsNoZigZag());
    nz[@"enabled"] = @(t->HasAlternateNodes() == false);
    [out addObject:nz];
    // J-20 — Tree's matrix props (# Strings, Lights/Nodes per
    // String, Strands/String, Starting Location) follow the
    // type-specific block. Strand Direction is emitted in place
    // of Matrix's "Direction".
    [out addObject:MakeIntProp(@"MatrixStringCount", @"# Strings",
                                t->GetNumPhysicalStrings(), 1, 10000)];
    NSString* lpsLabel = t->IsSingleNode() ? @"Lights/String" : @"Nodes/String";
    [out addObject:MakeIntProp(@"MatrixLightCount", lpsLabel,
                                t->GetNodesPerString(), 1, 10000)];
    [out addObject:MakeIntProp(@"MatrixStrandCount", @"Strands/String",
                                t->GetStrandsPerString(), 1, 2500)];
    [out addObject:MakeEnumProp(@"MatrixStart", @"Starting Location",
                                 EncodeStartCorner(t), StartCornerOptions())];
    [out addObject:MakeEnumProp(@"StrandDir", @"Strand Direction",
                                 t->isVerticalMatrix() ? 1 : 0,
                                 @[@"Horizontal", @"Vertical"])];
}

static void BuildSphereProps(SphereModel* s, NSMutableArray* out) {
    // J-20 — desktop SpherePropertyAdapter::AddStyleProperties
    // emits Degrees / Southern Latitude / Northern Latitude
    // BEFORE the matrix props. Order + ranges + labels matched
    // here so the iPad pane lines up.
    [out addObject:MakeIntProp(@"SphereDegrees", @"Degrees",
                                s->GetSphereDegrees(), 45, 360)];
    [out addObject:MakeIntProp(@"SphereStartLatitude", @"Southern Latitude",
                                s->GetStartLatitude(), -89, -1)];
    [out addObject:MakeIntProp(@"SphereEndLatitude", @"Northern Latitude",
                                s->GetEndLatitude(), 1, 89)];
    BuildMatrixProps(s, out);
}

// J-20 — Star has its own 12-entry start-location enum (not the
// matrix corners). Mirrors StarPropertyAdapter.cpp's
// TOP_BOT_LEFT_RIGHT array.
static NSArray<NSString*>* StarStartLocationOptions() {
    return @[
        @"Top Ctr-CCW", @"Top Ctr-CW",
        @"Top Ctr-CCW Inside", @"Top Ctr-CW Inside",
        @"Bottom Ctr-CW", @"Bottom Ctr-CCW",
        @"Bottom Ctr-CW Inside", @"Bottom Ctr-CCW Inside",
        @"Left Bottom-CW", @"Left Bottom-CCW",
        @"Right Bottom-CW", @"Right Bottom-CCW",
    ];
}

static void BuildStarProps(StarModel* s, NSMutableArray* out) {
    [out addObject:MakeIntProp(@"StarStringCount", @"# Strings",
                                s->GetNumStarStrings(), 1, 640)];
    NSString* lpsLabel = s->IsSingleNode() ? @"Lights/String" : @"Nodes/String";
    [out addObject:MakeIntProp(@"StarLightCount", lpsLabel,
                                s->GetNodesPerString(), 1, 10000)];
    [out addObject:MakeIntProp(@"StarStrandCount", @"# Points",
                                s->GetStarPoints(), 1, 250)];
    // Star uses its own start-location vocabulary (12 entries, not
    // the 4-corner matrix scheme used elsewhere).
    NSString* startLoc = [NSString stringWithUTF8String:s->GetStartLocation().c_str()];
    int startIdx = 0;
    NSArray* opts = StarStartLocationOptions();
    for (NSUInteger i = 0; i < opts.count; ++i) {
        if ([opts[i] isEqualToString:startLoc]) { startIdx = (int)i; break; }
    }
    [out addObject:MakeEnumProp(@"StarStart", @"Starting Location",
                                 startIdx, opts)];
    // Layer-size editor (matches desktop's AddLayerSizeProperty).
    NSMutableArray<NSNumber*>* sizes = [NSMutableArray array];
    for (int sz : s->GetLayerSizes()) { [sizes addObject:@(sz)]; }
    [out addObject:@{
        @"key":   @"LayerSizes",
        @"label": @"Layer Sizes",
        @"kind":  @"layerSizes",
        @"value": sizes,
    }];
    [out addObject:MakeDoubleProp(@"StarRatio", @"Outer to Inner Ratio",
                                   s->GetStarRatio(), 0.01, 10, 0.1, 2)];
    if (s->GetNumStrands() > 1) {
        [out addObject:MakeIntProp(@"StarCenterPercent", @"Inner Layer %",
                                    s->GetInnerPercent(), 0, 100)];
    }
}

static void BuildArchesProps(ArchesModel* a, NSMutableArray* out) {
    // J-19 — mirrors desktop's ArchesPropertyAdapter::AddType
    // Properties verbatim. The Layered Arches checkbox MUST be
    // first because flipping it relaods this whole list with
    // different labels and rows. When OFF you get the classic
    // # Arches / Nodes Per Arch / Gap layout; when ON you get a
    // single Nodes count + the per-layer size editor + the
    // hollow / zig-zag flags, the labels shift, and the
    // Starting Location enum expands from 2 to 4 choices.
    bool layered = a->GetLayerSizeCount() != 0;
    [out addObject:MakeBoolProp(@"LayeredArches", @"Layered Arches", layered)];

    if (!layered) {
        [out addObject:MakeIntProp(@"ArchesCount", @"# Arches",
                                    a->GetNumArches(), 1, 100)];
        [out addObject:MakeIntProp(@"ArchesNodes", @"Nodes Per Arch",
                                    a->GetNodesPerArch(), 1, 1000)];
    } else {
        [out addObject:MakeIntProp(@"ArchesNodes", @"Nodes",
                                    a->GetNodesPerArch(), 1, 10000)];
        // Per-layer-size editor. We surface the layer count and
        // the comma-joined size list; SwiftUI renders one TextField
        // per layer. Bridge accepts edits via `LayerSizes` (count
        // + array).
        NSMutableArray<NSNumber*>* sizes = [NSMutableArray array];
        for (int s : a->GetLayerSizes()) {
            [sizes addObject:@(s)];
        }
        [out addObject:@{
            @"key":    @"LayerSizes",
            @"label":  @"Layer Sizes",
            @"kind":   @"layerSizes",
            @"value":  sizes,
        }];
        [out addObject:MakeIntProp(@"ArchesHollow", @"Hollow %",
                                    a->GetHollow(), 0, 95)];
        [out addObject:MakeBoolProp(@"ArchesZigZag", @"Zig-Zag Layers",
                                     a->GetZigZag())];
    }

    [out addObject:MakeIntProp(@"ArchesLights", @"Lights Per Node",
                                a->GetLightsPerNode(), 1, 250)];
    [out addObject:MakeIntProp(@"ArchesArc", @"Arc Degrees",
                                a->GetArc(), 1, 180)];

    auto& threePt = dynamic_cast<ThreePointScreenLocation&>(a->GetModelScreenLocation());
    [out addObject:MakeIntProp(@"ArchesSkew", @"Arch Tilt",
                                threePt.GetAngle(), -180, 180)];

    if (!layered) {
        [out addObject:MakeIntProp(@"ArchesGap", @"Gap Between Arches",
                                    a->GetGap(), 0, 500)];
    }

    if (layered) {
        // 4-choice variant: Green Inside, Green Outside,
        // Blue Inside, Blue Outside. Encoded
        // (IsLtoR ? 0 : 2) + (IsBtoT ? 1 : 0) to match desktop.
        int idx = (a->GetIsLtoR() ? 0 : 2) + (a->GetIsBtoT() ? 1 : 0);
        [out addObject:MakeEnumProp(@"ArchesStart", @"Starting Location",
                                     idx,
                                     @[@"Green Square Inside",
                                       @"Green Square Outside",
                                       @"Blue Square Inside",
                                       @"Blue Square Outside"])];
    } else {
        [out addObject:MakeEnumProp(@"ArchesStart", @"Starting Location",
                                     a->GetIsLtoR() ? 0 : 1,
                                     @[@"Green Square", @"Blue Square"])];
    }
}

static void BuildIciclesProps(IciclesModel* ic, NSMutableArray* out) {
    [out addObject:MakeIntProp(@"IciclesStrings", @"# Strings",
                                ic->GetNumIcicleStrings(), 1, 100)];
    NSString* lpsLabel = ic->IsSingleNode() ? @"Lights/String" : @"Nodes/String";
    [out addObject:MakeIntProp(@"IciclesLights", lpsLabel,
                                ic->GetLightsPerString(), 1, 2000)];
    [out addObject:MakeBoolProp(@"AlternateNodes", @"Alternate Nodes",
                                 ic->HasAlternateNodes())];
    [out addObject:MakeStringProp(@"IciclesDrops", @"Drop Pattern",
                                   [NSString stringWithUTF8String:ic->GetDropPattern().c_str()])];
    [out addObject:MakeEnumProp(@"IciclesStart", @"Starting Location",
                                 ic->GetIsLtoR() ? 0 : 1,
                                 @[@"Green Square", @"Blue Square"])];
}

static void BuildCircleProps(CircleModel* c, NSMutableArray* out) {
    [out addObject:MakeIntProp(@"CircleStringCount", @"# Strings",
                                c->GetNumCircleStrings(), 1, 100)];
    NSString* lpsLabel = c->IsSingleNode() ? @"Lights/String" : @"Nodes/String";
    [out addObject:MakeIntProp(@"CircleLightCount", lpsLabel,
                                c->GetNodesPerString(), 1, 2000)];
    [out addObject:MakeIntProp(@"CircleCenterPercent", @"Center %",
                                c->GetCenterPercent(), 0, 100)];
    // J-20 — Layer-size editor (matches desktop's
    // AddLayerSizeProperty). Circle supports concentric rings.
    NSMutableArray<NSNumber*>* sizes = [NSMutableArray array];
    for (int sz : c->GetLayerSizes()) { [sizes addObject:@(sz)]; }
    [out addObject:@{
        @"key":   @"LayerSizes",
        @"label": @"Layer Sizes",
        @"kind":  @"layerSizes",
        @"value": sizes,
    }];
    int start = c->GetIsLtoR() ? 1 : 0;
    if (c->IsInsideOut()) start += 2;
    if (c->GetIsBtoT())   start += 4;
    [out addObject:MakeEnumProp(@"CircleStart", @"Starting Location",
                                 start,
                                 @[@"Top Outer-CCW",  @"Top Outer-CW",
                                   @"Top Inner-CCW",  @"Top Inner-CW",
                                   @"Bottom Outer-CCW", @"Bottom Outer-CW",
                                   @"Bottom Inner-CCW", @"Bottom Inner-CW"])];
}

static void BuildWreathProps(WreathModel* w, NSMutableArray* out) {
    [out addObject:MakeIntProp(@"WreathStringCount", @"# Strings",
                                w->GetNumWreathStrings(), 1, 640)];
    NSString* lpsLabel = w->IsSingleNode() ? @"Lights/String" : @"Nodes/String";
    [out addObject:MakeIntProp(@"WreathLightCount", lpsLabel,
                                w->GetNodesPerString(), 1, 640)];
    [out addObject:MakeEnumProp(@"WreathStart", @"Starting Location",
                                 EncodeStartCorner(w), StartCornerOptions())];
}

static void BuildSingleLineProps(SingleLineModel* l, NSMutableArray* out) {
    [out addObject:MakeIntProp(@"SingleLineCount", @"# Strings",
                                l->GetNumLines(), 1, 100)];
    NSString* lpsLabel = l->IsSingleNode() ? @"Lights/String" : @"Nodes/String";
    [out addObject:MakeIntProp(@"SingleLineNodes", lpsLabel,
                                l->GetNodesPerString(), 1, 10000)];
    if (!l->IsSingleNode()) {
        [out addObject:MakeIntProp(@"SingleLineLights", @"Lights/Node",
                                    l->GetLightsPerNode(), 1, 300)];
    }
    [out addObject:MakeEnumProp(@"SingleLineStart", @"Starting Location",
                                 l->GetIsLtoR() ? 0 : 1,
                                 @[@"Green Square", @"Blue Square"])];
}

static void BuildCandyCaneProps(CandyCaneModel* c, NSMutableArray* out) {
    [out addObject:MakeIntProp(@"CandyCaneCount", @"# Canes",
                                c->GetNumCanes(), 1, 20)];
    NSString* lpsLabel = c->IsSingleNode() ? @"Lights Per Cane" : @"Nodes Per Cane";
    int lpsVal = c->IsSingleNode() ? c->GetLightsPerNode() : c->GetNodesPerCane();
    [out addObject:MakeIntProp(@"CandyCaneNodes", lpsLabel, lpsVal, 1, 250)];
    if (!c->IsSingleNode()) {
        [out addObject:MakeIntProp(@"CandyCaneLights", @"Lights Per Node",
                                    c->GetLightsPerNode(), 1, 250)];
    }
    [out addObject:MakeDoubleProp(@"CandyCaneHeight", @"Height",
                                   c->GetCandyCaneHeight(), 0.1, 100, 0.1, 2)];
    [out addObject:MakeBoolProp(@"CandyCaneReverse", @"Reverse",
                                 c->IsReverse())];
    [out addObject:MakeBoolProp(@"CandyCaneSticks", @"Sticks",
                                 c->IsSticks())];
    NSMutableDictionary* d = MakeBoolProp(@"AlternateNodes", @"Alternate Nodes",
                                           c->HasAlternateNodes());
    d[@"enabled"] = @(!c->IsSingleNode());
    [out addObject:d];
    [out addObject:MakeEnumProp(@"CandyCaneStart", @"Starting Location",
                                 c->GetIsLtoR() ? 0 : 1,
                                 @[@"Green Square", @"Blue Square"])];
}

static void BuildSpinnerProps(SpinnerModel* s, NSMutableArray* out) {
    // J-20 — mirrors desktop SpinnerPropertyAdapter:
    // # Strings (1..640), Arms/String (1..250, key=FoldCount),
    // Lights/Arm (0..200), Hollow % (0..80), Start Angle
    // (-360..360), Arc (1..360), Starting Location (6-choice),
    // Zig-Zag Start. Alternate Nodes is not in the desktop's
    // type properties — it lives elsewhere — so it's dropped.
    [out addObject:MakeIntProp(@"SpinnerStringCount", @"# Strings",
                                s->GetNumSpinnerStrings(), 1, 640)];
    [out addObject:MakeIntProp(@"FoldCount", @"Arms/String",
                                s->GetArmsPerString(), 1, 250)];
    [out addObject:MakeIntProp(@"SpinnerArmNodeCount", @"Lights/Arm",
                                s->GetNodesPerArm(), 0, 200)];
    [out addObject:MakeIntProp(@"Hollow", @"Hollow %",
                                s->GetHollowPercent(), 0, 80)];
    [out addObject:MakeIntProp(@"StartAngle", @"Start Angle",
                                s->GetStartAngle(), -360, 360)];
    [out addObject:MakeIntProp(@"Arc", @"Arc",
                                s->GetArcAngle(), 1, 360)];
    [out addObject:MakeEnumProp(@"MatrixStart", @"Starting Location",
                                 s->EncodeStartLocation(),
                                 @[@"Center Counter Clockwise",
                                   @"Center Clockwise",
                                   @"End Counter Clockwise",
                                   @"End Clockwise",
                                   @"Center Alternate Counter Clockwise",
                                   @"Center Alternate Clockwise"])];
    NSMutableDictionary* zz = MakeBoolProp(@"ZigZag", @"Zig-Zag Start", s->HasZigZag());
    zz[@"enabled"] = @(s->HasAlternateNodes() == false);
    [out addObject:zz];
}

static void BuildWindowFrameProps(WindowFrameModel* w, NSMutableArray* out) {
    [out addObject:MakeIntProp(@"WindowFrameTopNodes", @"Top Nodes",
                                w->GetTopNodes(), 0, 1000)];
    [out addObject:MakeIntProp(@"WindowFrameSideNodes", @"Side Nodes",
                                w->GetSideNodes(), 1, 1000)];
    [out addObject:MakeIntProp(@"WindowFrameBottomNodes", @"Bottom Nodes",
                                w->GetBottomNodes(), 0, 1000)];
    [out addObject:MakeIntProp(@"WindowFrameRotation", @"Rotation",
                                w->GetRotation(), 0, 3)];
}

static void BuildCubeProps(CubeModel* c, NSMutableArray* out) {
    // J-20 — mirrors desktop CubePropertyAdapter:
    // Starting Location (8 named corners), Direction (6 named
    // styles), Strand Style (3 named), Layers All Start in Same
    // Place toggle, Width/Height/Depth (1..100), # Strings.
    NSArray<NSString*>* starts = @[
        @"Front Bottom Left", @"Front Bottom Right",
        @"Front Top Left",    @"Front Top Right",
        @"Back Bottom Left",  @"Back Bottom Right",
        @"Back Top Left",     @"Back Top Right",
    ];
    NSArray<NSString*>* styles = @[
        @"Vertical Front/Back",  @"Vertical Left/Right",
        @"Horizontal Front/Back",@"Horizontal Left/Right",
        @"Stacked Front/Back",   @"Stacked Left/Right",
    ];
    NSArray<NSString*>* strands = @[
        @"Zig Zag", @"No Zig Zag", @"Aternate Pixel",
    ];
    [out addObject:MakeEnumProp(@"CubeStart", @"Starting Location",
                                 c->GetCubeStartIndex(), starts)];
    [out addObject:MakeEnumProp(@"CubeStyle", @"Direction",
                                 c->GetCubeStyleIndex(), styles)];
    [out addObject:MakeEnumProp(@"StrandPerLine", @"Strand Style",
                                 c->GetStrandStyleIndex(), strands)];
    [out addObject:MakeBoolProp(@"StrandPerLayer",
                                 @"Layers All Start in Same Place",
                                 c->IsStrandPerLayer())];
    [out addObject:MakeIntProp(@"CubeWidth",  @"Width",  c->GetCubeWidth(),  1, 100)];
    [out addObject:MakeIntProp(@"CubeHeight", @"Height", c->GetCubeHeight(), 1, 100)];
    [out addObject:MakeIntProp(@"CubeDepth",  @"Depth",  c->GetCubeDepth(),  1, 100)];
    [out addObject:MakeIntProp(@"CubeStrings", @"# Strings",
                                c->GetCubeStrings(), 1, 1000)];
}

static void BuildChannelBlockProps(ChannelBlockModel* cb, NSMutableArray* out) {
    [out addObject:MakeIntProp(@"ChannelBlockChannels", @"# Channels",
                                cb->GetNumChannels(), 1, 1000)];
}

// J-20 — Image model. Mirrors ImagePropertyAdapter::AddType
// Properties: Image file path + Off Brightness + Read White
// As Alpha toggle. The file-picker affordance for `Image` is
// surfaced as a plain string for now — picking the file via
// the iPad's file picker is a follow-up.
static void BuildImageProps(ImageModel* im, NSMutableArray* out) {
    // J-20.2 — Image file uses the new `imageFile` descriptor
    // kind so SwiftUI renders a path label + folder-button that
    // opens a .fileImporter scoped to image UTTypes.
    [out addObject:@{
        @"key":   @"Image",
        @"label": @"Image",
        @"kind":  @"imageFile",
        @"value": [NSString stringWithUTF8String:im->GetImageFile().c_str()],
    }];
    [out addObject:MakeIntProp(@"OffBrightness", @"Off Brightness",
                                im->GetOffBrightness(), 0, 200)];
    [out addObject:MakeBoolProp(@"WhiteAsAlpha", @"Read White As Alpha",
                                 im->IsWhiteAsAlpha())];
}

// J-20 — Label model. Mirrors LabelPropertyAdapter::AddType
// Properties: Label Text + Font Size + Text Color.
static void BuildLabelProps(LabelModel* lm, NSMutableArray* out) {
    [out addObject:MakeStringProp(@"LabelText", @"Label Text",
        [NSString stringWithUTF8String:lm->GetLabelText().c_str()])];
    [out addObject:MakeIntProp(@"LabelFontSize", @"Font Size",
                                lm->GetLabelFontSize(), 8, 40)];
    xlColor tc = lm->GetLabelTextColor();
    NSString* hex = [NSString stringWithFormat:@"#%02X%02X%02X",
                     tc.red, tc.green, tc.blue];
    [out addObject:@{
        @"key": @"LabelTextColor", @"label": @"Text Color",
        @"kind": @"color", @"value": hex,
    }];
}

// J-21 — MultiPoint. Mirrors MultiPointPropertyAdapter:
// # Lights / # Nodes (read-only), # Strings, optional Indiv
// Start Nodes toggle + per-string node fields, Height.
static void BuildMultiPointProps(MultiPointModel* mp, NSMutableArray* out) {
    NSString* nodesLabel = mp->IsSingleNode() ? @"# Lights" : @"# Nodes";
    NSMutableDictionary* d = MakeIntProp(@"MultiPointNodes", nodesLabel,
                                          mp->GetNumPoints(), 1, 10000);
    d[@"enabled"] = @NO;
    [out addObject:d];
    [out addObject:MakeIntProp(@"MultiPointStrings", @"Strings",
                                mp->GetNumStrings(), 1, 48)];

    // Indiv Start Nodes toggle + per-string fields, only when
    // there's more than one string. Mirrors the desktop's
    // adapter: the toggle defaults to off; turning it on
    // surfaces N spin-able fields, one per string.
    if (mp->GetNumStrings() > 1) {
        bool indiv = mp->HasIndivStartNodes();
        [out addObject:MakeBoolProp(@"ModelIndividualStartNodes",
                                     @"Indiv Start Nodes", indiv)];
        if (indiv) {
            int strings = mp->GetNumStrings();
            int nodeCount = std::max(1, (int)mp->GetNodeCount());
            for (int i = 0; i < strings; ++i) {
                int v = i < mp->GetIndivStartNodesCount() ? mp->GetIndivStartNode(i)
                                                          : mp->ComputeStringStartNode(i);
                if (v < 1) v = 1;
                if (v > nodeCount) v = nodeCount;
                NSString* nm = [NSString stringWithUTF8String:mp->StartNodeAttrName(i).c_str()];
                NSString* key = [NSString stringWithFormat:@"IndivStartNode%d", i];
                [out addObject:MakeIntProp(key, nm, v, 1, nodeCount)];
            }
        }
    }

    [out addObject:MakeDoubleProp(@"ModelHeight", @"Height",
                                   mp->GetModelHeight(), -100, 100, 0.1, 2)];
}

// J-21 — PolyLine. # Lights/Nodes (read-only), Lights/Node,
// Strings, Indiv Start Nodes toggle + per-string fields,
// Starting Location, Alternate Drop Nodes, Height. Segment /
// corner editors still deferred — those need a per-vertex
// editor (segment node-count + corner-style enum per vertex)
// that doesn't yet exist on iPad.
static void BuildPolyLineProps(PolyLineModel* pl, NSMutableArray* out) {
    if (pl->IsSingleNode()) {
        NSMutableDictionary* d = MakeIntProp(@"PolyLineNodes", @"# Lights",
                                              pl->GetTotalLightCount(), 1, 100000);
        d[@"enabled"] = @NO;
        [out addObject:d];
    } else {
        NSMutableDictionary* d = MakeIntProp(@"PolyLineNodes", @"# Nodes",
                                              pl->GetTotalLightCount(), 1, 100000);
        d[@"enabled"] = @NO;
        [out addObject:d];
        [out addObject:MakeIntProp(@"PolyLineLights", @"Lights/Node",
                                    pl->GetLightsPerNode(), 1, 300)];
    }
    [out addObject:MakeIntProp(@"PolyLineStrings", @"Strings",
                                pl->GetNumStrings(), 1, 48)];

    // Indiv Start Nodes (only when > 1 string).
    if (pl->GetNumStrings() > 1) {
        bool indiv = pl->HasIndivStartNodes();
        [out addObject:MakeBoolProp(@"ModelIndividualStartNodes",
                                     @"Start Nodes", indiv)];
        if (indiv) {
            int strings = pl->GetNumStrings();
            int nodeCount = std::max(1, (int)pl->GetNodeCount());
            for (int i = 0; i < strings; ++i) {
                int v = i < pl->GetIndivStartNodesCount() ? pl->GetIndivStartNode(i)
                                                          : pl->ComputeStringStartNode(i);
                if (v < 1) v = 1;
                if (v > nodeCount) v = nodeCount;
                NSString* nm = [NSString stringWithUTF8String:pl->StartNodeAttrName(i).c_str()];
                NSString* key = [NSString stringWithFormat:@"IndivStartNode%d", i];
                [out addObject:MakeIntProp(key, nm, v, 1, nodeCount)];
            }
        }
    }

    [out addObject:MakeEnumProp(@"PolyLineStart", @"Starting Location",
                                 pl->GetIsLtoR() ? 0 : 1,
                                 @[@"Green Square", @"Blue Square"])];
    // J-21.1 — Drop Pattern. Shared with Icicles via the
    // `IciclesDrops` key + `Model::SetDropPattern` setter.
    [out addObject:MakeStringProp(@"IciclesDrops", @"Drop Pattern",
        [NSString stringWithUTF8String:pl->GetDropPattern().c_str()])];
    [out addObject:MakeBoolProp(@"AlternateNodes", @"Alternate Drop Nodes",
                                 pl->HasAlternateNodes())];
    [out addObject:MakeDoubleProp(@"ModelHeight", @"Height",
                                   pl->GetModelHeight(), -100, 100, 0.1, 2)];

    // J-21.1 — Per-segment node counts. Each segment between two
    // PolyPoint vertices gets a `Segment N` row with a spin int.
    // Mirrors desktop's `ModelIndividualSegments.SegmentN` keys.
    // Editing a segment turns off AutoDistribute (matches desktop)
    // so the user's manual sizes are respected.
    int numSegments = pl->GetNumSegments();
    std::vector<int> segSizes = pl->GetSegmentsSizes();
    for (int x = 0; x < numSegments; ++x) {
        NSString* nm  = [NSString stringWithFormat:@"Segment %d", x + 1];
        NSString* key = [NSString stringWithFormat:@"PolySegmentSize%d", x];
        int sz = (x < (int)segSizes.size()) ? segSizes[x] : 1;
        [out addObject:MakeIntProp(key, nm, sz, 1, 100000)];
    }

    // J-21.1 — Corner settings. There are numSegments + 1 corners
    // (one per vertex). Each corner picks how the surrounding
    // segments connect: Leading / Trailing / Neither. Desktop
    // translates the enum into lead/trail offsets at the segment
    // boundaries; bridge mirrors that math in the setter below.
    std::vector<std::string> corners = pl->GetCorners();
    for (int x = 0; x < numSegments + 1; ++x) {
        NSString* nm  = [NSString stringWithFormat:@"Corner %d", x + 1];
        NSString* key = [NSString stringWithFormat:@"PolyCorner%d", x];
        std::string val = (x < (int)corners.size()) ? corners[x] : "Neither";
        int idx = (val == "Leading Segment") ? 0 : (val == "Trailing Segment") ? 1 : 2;
        [out addObject:MakeEnumProp(key, nm, idx,
                                     @[@"Leading Segment",
                                       @"Trailing Segment",
                                       @"Neither"])];
    }
}

// J-3 (DMX) — emit DmxColorAbility property descriptors. Today
// only the RGBW path is fleshed out; CMYW + ColorWheel fall back
// to a placeholder header so the user can at least see what type
// the model is configured for. PWM brightness/gamma fields are
// gated on the controller's PWM-protocol support, which we don't
// expose to the bridge yet — skipped for v1, matching how
// LayoutPanel sees them only when wired to a PWM-capable
// controller.
static void AppendDmxColorProps(DmxColorAbility* color, NSMutableArray* out) {
    if (!color) return;
    using CT = DmxColorAbility::DMX_COLOR_TYPE;
    switch (color->GetColorType()) {
    case CT::DMX_COLOR_RGBW: {
        auto* rgb = static_cast<DmxColorAbilityRGB*>(color);
        [out addObject:MakeHeaderDescriptor(@"DmxColorHeader", @"Color (RGBW)")];
        [out addObject:MakeIntProp(@"DmxRedChannel",   @"Red Channel",
                                     (int)rgb->GetRedChannel(),   0, 512)];
        [out addObject:MakeIntProp(@"DmxGreenChannel", @"Green Channel",
                                     (int)rgb->GetGreenChannel(), 0, 512)];
        [out addObject:MakeIntProp(@"DmxBlueChannel",  @"Blue Channel",
                                     (int)rgb->GetBlueChannel(),  0, 512)];
        [out addObject:MakeIntProp(@"DmxWhiteChannel", @"White Channel",
                                     (int)rgb->GetWhiteChannel(), 0, 512)];
        break;
    }
    case CT::DMX_COLOR_CMYW: {
        auto* cmy = static_cast<DmxColorAbilityCMY*>(color);
        [out addObject:MakeHeaderDescriptor(@"DmxColorHeader", @"Color (CMYW)")];
        [out addObject:MakeIntProp(@"DmxCyanChannel",    @"Cyan Channel",
                                     (int)cmy->GetCyanChannel(),    0, 512)];
        [out addObject:MakeIntProp(@"DmxMagentaChannel", @"Magenta Channel",
                                     (int)cmy->GetMagentaChannel(), 0, 512)];
        [out addObject:MakeIntProp(@"DmxYellowChannel",  @"Yellow Channel",
                                     (int)cmy->GetYellowChannel(),  0, 512)];
        [out addObject:MakeIntProp(@"DmxWhiteChannel",   @"White Channel",
                                     (int)cmy->GetWhiteChannel(),   0, 512)];
        break;
    }
    case CT::DMX_COLOR_WHEEL: {
        auto* wh = static_cast<DmxColorAbilityWheel*>(color);
        [out addObject:MakeHeaderDescriptor(@"DmxColorHeader", @"Color Wheel")];
        [out addObject:MakeIntProp(@"DmxWheelChannel",       @"Wheel Channel",
                                     (int)wh->GetWheelChannel(),   0, 512)];
        [out addObject:MakeIntProp(@"DmxWheelDimmerChannel", @"Dimmer Channel",
                                     (int)wh->GetDimmerChannel(),  0, 512)];
        [out addObject:MakeIntProp(@"DmxWheelDelay",         @"Wheel Delay (ms)",
                                     (int)wh->GetWheelDelay(),     0, 5000)];
        // J-30 — custom wheel-colour list as a clickable
        // descriptor. SwiftUI's `wheelColorList` kind opens an
        // editor sheet that commits the full list back through
        // the `DmxWheelColorList` setter (wholesale replace).
        NSMutableArray* wheelEntries = [NSMutableArray array];
        for (const auto& c : wh->GetWheelColorSettings()) {
            NSString* hex = [NSString stringWithFormat:@"#%02X%02X%02X",
                              c.color.Red(), c.color.Green(), c.color.Blue()];
            [wheelEntries addObject:@{
                @"color":    hex,
                @"dmxValue": @(c.dmxValue),
            }];
        }
        [out addObject:@{
            @"key":   @"DmxWheelColorList",
            @"label": @"Wheel Colours",
            @"kind":  @"wheelColorList",
            @"value": wheelEntries,
        }];
        break;
    }
    case CT::DMX_COLOR_UNUSED:
        break;
    }
}

// J-30 — emit the preset list as a clickable descriptor. The
// SwiftUI `presetList` kind opens an inline editor sheet that
// commits back the full list (wholesale replace through the
// `DmxPresetList` setter). Each entry carries channel /
// value / description.
static void AppendDmxPresetProps(const DmxPresetAbility& preset, NSMutableArray* out) {
    NSMutableArray* entries = [NSMutableArray array];
    for (const auto& p : preset.GetPresetSettings()) {
        [entries addObject:@{
            @"channel":     @(p.DMXChannel),
            @"value":       @(p.DMXValue),
            @"description": [NSString stringWithUTF8String:p.Description.c_str()],
        }];
    }
    [out addObject:@{
        @"key":   @"DmxPresetList",
        @"label": @"Presets",
        @"kind":  @"presetList",
        @"value": entries,
    }];
}

static void AppendDmxShutterProps(const DmxShutterAbility& shutter, NSMutableArray* out) {
    [out addObject:MakeHeaderDescriptor(@"DmxShutterHeader", @"Shutter")];
    [out addObject:MakeIntProp(@"DmxShutterChannel", @"Shutter Channel",
                                 shutter.GetShutterChannel(), 0, 512)];
    [out addObject:MakeIntProp(@"DmxShutterOpen", @"Open Threshold",
                                 shutter.GetShutterThreshold(), -255, 255)];
    [out addObject:MakeIntProp(@"DmxShutterOnValue", @"On Value",
                                 shutter.GetShutterOnValue(), 0, 255)];
}

static void AppendDmxBeamProps(const DmxBeamAbility& beam, NSMutableArray* out) {
    [out addObject:MakeHeaderDescriptor(@"DmxBeamHeader", @"Beam")];
    [out addObject:MakeDoubleProp(@"DmxBeamLength", @"Display Length",
                                    (double)beam.GetBeamLength(), 0.0, 100.0, 0.1, 2)];
    [out addObject:MakeDoubleProp(@"DmxBeamWidth", @"Display Width",
                                    (double)beam.GetBeamWidth(), 0.01, 150.0, 0.1, 2)];
    if (beam.SupportsOrient()) {
        [out addObject:MakeIntProp(@"DmxBeamOrient", @"Orientation",
                                     beam.GetBeamOrient(), 0, 360)];
    }
    if (beam.SupportsYOffset()) {
        [out addObject:MakeDoubleProp(@"DmxBeamYOffset", @"Y Offset",
                                        (double)beam.GetBeamYOffset(), 0.0, 500.0, 1.0, 1)];
    }
}

static void BuildDmxFloodlightProps(DmxFloodlight* fl, NSMutableArray* out) {
    [out addObject:MakeIntProp(@"DmxChannelCount", @"# Channels",
                                 fl->GetDmxChannelCount(), 1, 512)];
    if (fl->HasPresetAbility()) {
        AppendDmxPresetProps(*fl->GetPresetAbility(), out);
    }
    if (fl->HasColorAbility()) {
        AppendDmxColorProps(fl->GetColorAbility(), out);
    }
    if (fl->HasShutterAbility()) {
        AppendDmxShutterProps(*fl->GetShutterAbility(), out);
    }
    if (fl->HasBeamAbility()) {
        AppendDmxBeamProps(*fl->GetBeamAbility(), out);
    }
}

static void AppendDmxDimmerProps(const DmxDimmerAbility& dim, NSMutableArray* out) {
    [out addObject:MakeHeaderDescriptor(@"DmxDimmerHeader", @"Dimmer")];
    [out addObject:MakeIntProp(@"MhDimmerChannel", @"Dimmer Channel",
                                 dim.GetDimmerChannel(), 0, 512)];
}

// J-3 (DMX) — emit a DmxMotor's 10 knobs (channel coarse/fine,
// min/max limit, range of motion, orient zero/home, slew limit,
// reverse, upside down). Key prefix is the motor's `base_name`
// (`PanMotor` / `TiltMotor`) so the setter side can route via the
// shared keys (`PanMotorChannelCoarse`, …, `TiltMotorUpsideDown`).
// Matches desktop's `DmxComponentPropertyHelpers::
// AddMotorProperties` (DmxComponentPropertyHelpers.cpp:191).
static void AppendDmxMotorProps(const DmxMotor& motor, NSMutableArray* out) {
    NSString* base = [NSString stringWithUTF8String:motor.GetName().c_str()];
    NSString* header = ([base isEqualToString:@"PanMotor"] ? @"Pan Motor" :
                         [base isEqualToString:@"TiltMotor"] ? @"Tilt Motor" :
                         base);
    [out addObject:MakeHeaderDescriptor([base stringByAppendingString:@"Header"],
                                          header)];
    [out addObject:MakeIntProp([base stringByAppendingString:@"ChannelCoarse"],
                                 @"Channel (Coarse)",
                                 motor.GetChannelCoarse(), 0, 512)];
    [out addObject:MakeIntProp([base stringByAppendingString:@"ChannelFine"],
                                 @"Channel (Fine)",
                                 motor.GetChannelFine(), 0, 512)];
    [out addObject:MakeIntProp([base stringByAppendingString:@"MinLimit"],
                                 @"Min Limit (deg)",
                                 motor.GetMinLimit(), -180, 0)];
    [out addObject:MakeIntProp([base stringByAppendingString:@"MaxLimit"],
                                 @"Max Limit (deg)",
                                 motor.GetMaxLimit(), 0, 180)];
    [out addObject:MakeDoubleProp([base stringByAppendingString:@"RangeOfMotion"],
                                    @"Range of Motion (deg)",
                                    (double)motor.GetRangeOfMotion(),
                                    0.0, 65535.0, 1.0, 1)];
    [out addObject:MakeIntProp([base stringByAppendingString:@"OrientZero"],
                                 @"Orient to Zero (deg)",
                                 motor.GetOrientZero(), 0, 360)];
    NSString* homeLabel = [base isEqualToString:@"PanMotor"]
        ? @"Orient Forward (deg)"
        : @"Orient Up (deg)";
    [out addObject:MakeIntProp([base stringByAppendingString:@"OrientHome"],
                                 homeLabel,
                                 motor.GetOrientHome(), 0, 360)];
    [out addObject:MakeDoubleProp([base stringByAppendingString:@"SlewLimit"],
                                    @"Slew Limit (deg/sec)",
                                    (double)motor.GetSlewLimit(),
                                    0.0, 500.0, 0.1, 2)];
    [out addObject:MakeBoolProp([base stringByAppendingString:@"Reverse"],
                                  @"Reverse Rotation",
                                  motor.GetReverse() ? YES : NO)];
    [out addObject:MakeBoolProp([base stringByAppendingString:@"UpsideDown"],
                                  @"Upside Down",
                                  motor.GetUpsideDown() ? YES : NO)];
}

static NSArray<NSString*>* DmxMovingHeadStyleOptions() {
    return @[@"Moving Head Top",
             @"Moving Head Side",
             @"Moving Head Bars",
             @"Moving Head Top Bars",
             @"Moving Head Side Bars",
             @"Moving Head 3D"];
}
static NSString* DmxMovingHeadStyleNameForIndex(int idx) {
    // Note: the on-disk style string differs from the display name
    // for the *Bars* entries — "Moving Head TopBars" / "SideBars"
    // (no space) per `DmxMovingHeadPropertyAdapter::
    // DMX_STYLE_NAMES`. The bridge writes the canonical form.
    static NSArray<NSString*>* names = @[@"Moving Head Top",
                                           @"Moving Head Side",
                                           @"Moving Head Bars",
                                           @"Moving Head TopBars",
                                           @"Moving Head SideBars",
                                           @"Moving Head 3D"];
    if (idx < 0 || idx >= (int)names.count) return names[0];
    return names[idx];
}
static NSArray<NSString*>* DmxMovingHeadFixtureOptions() {
    return @[@"MH1", @"MH2", @"MH3", @"MH4",
             @"MH5", @"MH6", @"MH7", @"MH8"];
}
static NSArray<NSString*>* DmxColorTypeOptions() {
    return @[@"RGBW", @"ColorWheel", @"CMYW", @"Unused"];
}
static int DmxColorTypeIndexFor(const DmxModel* dmx) {
    if (!dmx || !dmx->HasColorAbility()) return 3;  // Unused
    switch (dmx->GetColorAbility()->GetColorType()) {
    case DmxColorAbility::DMX_COLOR_TYPE::DMX_COLOR_RGBW:   return 0;
    case DmxColorAbility::DMX_COLOR_TYPE::DMX_COLOR_WHEEL:  return 1;
    case DmxColorAbility::DMX_COLOR_TYPE::DMX_COLOR_CMYW:   return 2;
    case DmxColorAbility::DMX_COLOR_TYPE::DMX_COLOR_UNUSED: return 3;
    }
    return 3;
}

// J-3 (DMX) — DmxMovingHeadAdv basic surface. Shares Pan / Tilt
// motor blocks + Color / Dimmer / Shutter / Beam abilities with
// DmxMovingHead, but adds three mesh files (base / yoke / head)
// and a position-zones collision-avoidance grid that aren't
// editable yet on iPad — those carry "edit on desktop" headers.
// Mesh file pickers + the zone grid editor are DM-2 follow-up
// work (XL).
static void BuildDmxMovingHeadAdvProps(DmxMovingHeadAdv* mh, NSMutableArray* out) {
    [out addObject:MakeEnumProp(@"DmxFixture", @"Fixture",
                                 mh->GetFixtureVal() - 1,
                                 DmxMovingHeadFixtureOptions())];
    [out addObject:MakeIntProp(@"DmxChannelCount", @"# Channels",
                                 mh->GetDmxChannelCount(), 1, 512)];
    if (mh->HasPresetAbility()) {
        AppendDmxPresetProps(*mh->GetPresetAbility(), out);
    }
    if (auto* pan = mh->GetPanMotor()) {
        AppendDmxMotorProps(*pan, out);
    }
    if (auto* tilt = mh->GetTiltMotor()) {
        AppendDmxMotorProps(*tilt, out);
    }
    [out addObject:MakeEnumProp(@"DmxColorType", @"Color Type",
                                 DmxColorTypeIndexFor(mh),
                                 DmxColorTypeOptions())];
    if (mh->HasColorAbility()) {
        AppendDmxColorProps(mh->GetColorAbility(), out);
    }
    if (mh->HasDimmerAbility()) {
        AppendDmxDimmerProps(*mh->GetDimmerAbility(), out);
    }
    if (mh->HasShutterAbility()) {
        AppendDmxShutterProps(*mh->GetShutterAbility(), out);
    }
    if (mh->HasBeamAbility()) {
        AppendDmxBeamProps(*mh->GetBeamAbility(), out);
    }
    // 3D meshes — file picker per slot. Position-zone grid stays
    // desktop-only (the collision-avoidance 2D-grid editor needs
    // its own sheet, queued).
    [out addObject:MakeHeaderDescriptor(@"DmxAdvMeshHeader", @"3D Meshes")];
    Mesh* baseMesh = mh->GetBaseMesh();
    [out addObject:@{
        @"key":   @"AdvBaseMeshFile",
        @"label": @"Base Mesh",
        @"kind":  @"meshFile",
        @"value": baseMesh ? [NSString stringWithUTF8String:baseMesh->GetObjFile().c_str()]
                            : @"",
    }];
    Mesh* yokeMesh = mh->GetYokeMesh();
    [out addObject:@{
        @"key":   @"AdvYokeMeshFile",
        @"label": @"Yoke Mesh",
        @"kind":  @"meshFile",
        @"value": yokeMesh ? [NSString stringWithUTF8String:yokeMesh->GetObjFile().c_str()]
                            : @"",
    }];
    Mesh* headMesh = mh->GetHeadMesh();
    [out addObject:@{
        @"key":   @"AdvHeadMeshFile",
        @"label": @"Head Mesh",
        @"kind":  @"meshFile",
        @"value": headMesh ? [NSString stringWithUTF8String:headMesh->GetObjFile().c_str()]
                            : @"",
    }];
    // J-30 — position-zone list. Each zone gates a channel/value
    // assignment to a Pan × Tilt rectangle; the model writes the
    // zone's `value` to its `channel` whenever the head's
    // current pan/tilt falls inside the zone (collision-
    // avoidance for setups where the beam can't sweep certain
    // directions, e.g. into walls).
    NSMutableArray* zones = [NSMutableArray array];
    for (const auto& z : mh->GetPositionZones()) {
        [zones addObject:@{
            @"panMin":  @(z.pan_min),
            @"panMax":  @(z.pan_max),
            @"tiltMin": @(z.tilt_min),
            @"tiltMax": @(z.tilt_max),
            @"channel": @(z.channel),
            @"value":   @(z.value),
        }];
    }
    [out addObject:@{
        @"key":   @"DmxPositionZoneList",
        @"label": @"Position Zones",
        @"kind":  @"positionZoneList",
        @"value": zones,
    }];
}

// J-3 (DMX) — DmxSkull basic surface. Six servos (Jaw / Pan /
// Tilt / Nod / EyeUD / EyeLR) plus eye-brightness and an
// optional color ability. Servo *enables* (which servos this
// fixture actually has) come from desktop's SkullConfigDialog —
// a checkbox sheet we haven't ported. Per-servo channel / min /
// max / orient are editable inline when the matching `HasXxx`
// bool is set; the four head/jaw/eye meshes stay desktop-only.
static void AppendDmxSkullServoProps(NSString* keyPrefix,
                                       NSString* header,
                                       Servo* servo,
                                       int orient,
                                       NSMutableArray* out) {
    if (!servo) return;
    [out addObject:MakeHeaderDescriptor([keyPrefix stringByAppendingString:@"Header"],
                                          header)];
    [out addObject:MakeIntProp([keyPrefix stringByAppendingString:@"Channel"],
                                 @"Channel",
                                 servo->GetChannel(), 0, 512)];
    [out addObject:MakeIntProp([keyPrefix stringByAppendingString:@"MinLimit"],
                                 @"Min Limit",
                                 servo->GetMinLimit(), 0, 65535)];
    [out addObject:MakeIntProp([keyPrefix stringByAppendingString:@"MaxLimit"],
                                 @"Max Limit",
                                 servo->GetMaxLimit(), 0, 65535)];
    [out addObject:MakeIntProp([keyPrefix stringByAppendingString:@"Orient"],
                                 @"Orient (deg)",
                                 orient, -360, 360)];
}

static void BuildDmxSkullProps(DmxSkull* sk, NSMutableArray* out) {
    [out addObject:MakeIntProp(@"DmxChannelCount", @"# Channels",
                                 sk->GetDmxChannelCount(), 1, 512)];
    if (sk->HasPresetAbility()) {
        AppendDmxPresetProps(*sk->GetPresetAbility(), out);
    }
    [out addObject:MakeBoolProp(@"SkullIs16Bit", @"16-bit Servos",
                                  sk->Is16Bit() ? YES : NO)];
    [out addObject:MakeHeaderDescriptor(@"SkullEnableHeader",
                                          @"Servo Enables")];
    [out addObject:MakeBoolProp(@"SkullHasJaw",   @"Jaw",
                                  sk->HasJaw()   ? YES : NO)];
    [out addObject:MakeBoolProp(@"SkullHasPan",   @"Pan",
                                  sk->HasPan()   ? YES : NO)];
    [out addObject:MakeBoolProp(@"SkullHasTilt",  @"Tilt",
                                  sk->HasTilt()  ? YES : NO)];
    [out addObject:MakeBoolProp(@"SkullHasNod",   @"Nod",
                                  sk->HasNod()   ? YES : NO)];
    [out addObject:MakeBoolProp(@"SkullHasEyeUD", @"Eye Up/Down",
                                  sk->HasEyeUD() ? YES : NO)];
    [out addObject:MakeBoolProp(@"SkullHasEyeLR", @"Eye Left/Right",
                                  sk->HasEyeLR() ? YES : NO)];
    [out addObject:MakeBoolProp(@"SkullHasColor", @"Color",
                                  sk->HasColor() ? YES : NO)];
    [out addObject:MakeButtonDescriptor(@"SkullApplySkulltronix",
                                          @"Apply Skulltronix Preset")];
    if (sk->HasJaw()) {
        AppendDmxSkullServoProps(@"SkullJaw", @"Jaw Servo",
                                  sk->GetJawServo(), sk->GetJawOrient(), out);
    }
    if (sk->HasPan()) {
        AppendDmxSkullServoProps(@"SkullPan", @"Pan Servo",
                                  sk->GetPanServo(), sk->GetPanOrient(), out);
    }
    if (sk->HasTilt()) {
        AppendDmxSkullServoProps(@"SkullTilt", @"Tilt Servo",
                                  sk->GetTiltServo(), sk->GetTiltOrient(), out);
    }
    if (sk->HasNod()) {
        AppendDmxSkullServoProps(@"SkullNod", @"Nod Servo",
                                  sk->GetNodServo(), sk->GetNodOrient(), out);
    }
    if (sk->HasEyeUD()) {
        AppendDmxSkullServoProps(@"SkullEyeUD", @"Eye Up/Down Servo",
                                  sk->GetEyeUDServo(), sk->GetEyeUDOrient(), out);
    }
    if (sk->HasEyeLR()) {
        AppendDmxSkullServoProps(@"SkullEyeLR", @"Eye Left/Right Servo",
                                  sk->GetEyeLRServo(), sk->GetEyeLROrient(), out);
    }
    if (sk->HasColor()) {
        [out addObject:MakeHeaderDescriptor(@"SkullEyeBrightHeader", @"Eyes")];
        [out addObject:MakeIntProp(@"SkullEyeBrightnessChannel",
                                     @"Eye Brightness Channel",
                                     sk->GetEyeBrightnessChannel(), 0, 512)];
        if (sk->HasColorAbility()) {
            AppendDmxColorProps(sk->GetColorAbility(), out);
        }
    }
    [out addObject:MakeHeaderDescriptor(@"SkullMeshHeader", @"3D Meshes")];
    Mesh* headMesh = sk->GetHeadMesh();
    [out addObject:@{
        @"key":   @"SkullHeadMeshFile",
        @"label": @"Head",
        @"kind":  @"meshFile",
        @"value": headMesh ? [NSString stringWithUTF8String:headMesh->GetObjFile().c_str()]
                            : @"",
    }];
    Mesh* jawMesh = sk->GetJawMesh();
    [out addObject:@{
        @"key":   @"SkullJawMeshFile",
        @"label": @"Jaw",
        @"kind":  @"meshFile",
        @"value": jawMesh ? [NSString stringWithUTF8String:jawMesh->GetObjFile().c_str()]
                           : @"",
    }];
    Mesh* eyeLMesh = sk->GetEyeLMesh();
    [out addObject:@{
        @"key":   @"SkullEyeLMeshFile",
        @"label": @"Eye Left",
        @"kind":  @"meshFile",
        @"value": eyeLMesh ? [NSString stringWithUTF8String:eyeLMesh->GetObjFile().c_str()]
                            : @"",
    }];
    Mesh* eyeRMesh = sk->GetEyeRMesh();
    [out addObject:@{
        @"key":   @"SkullEyeRMeshFile",
        @"label": @"Eye Right",
        @"kind":  @"meshFile",
        @"value": eyeRMesh ? [NSString stringWithUTF8String:eyeRMesh->GetObjFile().c_str()]
                            : @"",
    }];
}

// J-3 (DMX) — Servo style enum. Mirrors `Servo::SERVO_STYLE`
// (Servo.h:77). Order is load-bearing — the setter writes back
// the matching string via `Servo::SetStyle(string)`.
static NSArray<NSString*>* DmxServoStyleOptions() {
    return @[@"Translate X", @"Translate Y", @"Translate Z",
             @"Rotate X",    @"Rotate Y",    @"Rotate Z"];
}
static int DmxServoStyleIndexFor(const Servo* s) {
    if (!s) return 0;
    NSString* str = [NSString stringWithUTF8String:s->GetStyle().c_str()];
    NSArray* opts = DmxServoStyleOptions();
    NSUInteger i = [opts indexOfObject:str];
    return (i == NSNotFound) ? 0 : (int)i;
}

// J-3 (DMX) — emit a Servo's editable knobs. Key prefix is
// `<keyPrefix>` (e.g. `Servo0`, `Servo1`); the setter side
// decodes the suffix to dispatch back to the matching setter.
static void AppendDmxServoKnobProps(NSString* keyPrefix,
                                      NSString* headerLabel,
                                      const Servo* s,
                                      NSMutableArray* out) {
    if (!s) return;
    [out addObject:MakeHeaderDescriptor([keyPrefix stringByAppendingString:@"Header"],
                                          headerLabel)];
    [out addObject:MakeIntProp([keyPrefix stringByAppendingString:@"Channel"],
                                 @"Channel",
                                 s->GetChannel(), 0, 512)];
    [out addObject:MakeBoolProp([keyPrefix stringByAppendingString:@"Is16Bit"],
                                  @"16-bit",
                                  s->Is16Bit() ? YES : NO)];
    [out addObject:MakeIntProp([keyPrefix stringByAppendingString:@"MinLimit"],
                                 @"Min Limit",
                                 s->GetMinLimit(), 0, 65535)];
    [out addObject:MakeIntProp([keyPrefix stringByAppendingString:@"MaxLimit"],
                                 @"Max Limit",
                                 s->GetMaxLimit(), 0, 65535)];
    [out addObject:MakeDoubleProp([keyPrefix stringByAppendingString:@"RangeOfMotion"],
                                    @"Range of Motion",
                                    (double)s->GetRangeOfMotion(),
                                    -65535.0, 65535.0, 1.0, 1)];
    [out addObject:MakeEnumProp([keyPrefix stringByAppendingString:@"Style"],
                                  @"Style",
                                  DmxServoStyleIndexFor(s),
                                  DmxServoStyleOptions())];
    [out addObject:MakeIntProp([keyPrefix stringByAppendingString:@"ControllerMin"],
                                 @"Controller Min Pulse (us)",
                                 s->GetControllerMin(), 0, 65535)];
    [out addObject:MakeIntProp([keyPrefix stringByAppendingString:@"ControllerMax"],
                                 @"Controller Max Pulse (us)",
                                 s->GetControllerMax(), 0, 65535)];
    [out addObject:MakeBoolProp([keyPrefix stringByAppendingString:@"ControllerReverse"],
                                  @"Reverse",
                                  s->GetControllerReverse() ? YES : NO)];
}

// J-3 (DMX) — DmxServo basic surface. N servos (1–25) with the
// full per-servo knob set. Static + motion image files stay
// desktop-only (image-picker UX is a separate piece of work).
static void BuildDmxServoProps(DmxServo* ds, NSMutableArray* out) {
    [out addObject:MakeIntProp(@"DmxChannelCount", @"# Channels",
                                 ds->GetDmxChannelCount(), 1, 512)];
    [out addObject:MakeIntProp(@"ServoNumServos", @"# Servos",
                                 ds->GetNumServos(), 1, 25)];
    [out addObject:MakeBoolProp(@"ServoIs16Bit", @"16-bit Servos",
                                  ds->Is16Bit() ? YES : NO)];
    [out addObject:MakeDoubleProp(@"ServoBrightness", @"Brightness (%)",
                                    (double)ds->GetBrightness(),
                                    0.0, 100.0, 1.0, 1)];
    [out addObject:MakeIntProp(@"ServoTransparency", @"Transparency (%)",
                                 ds->GetTransparency(), 0, 100)];
    if (ds->HasPresetAbility()) {
        AppendDmxPresetProps(*ds->GetPresetAbility(), out);
    }
    for (int i = 0; i < ds->GetNumServos(); ++i) {
        NSString* prefix = [NSString stringWithFormat:@"Servo%d", i];
        NSString* hdr = [NSString stringWithFormat:@"Servo %d", i + 1];
        AppendDmxServoKnobProps(prefix, hdr, ds->GetServo(i), out);
        DmxImage* staticImg = ds->GetStaticImage(i);
        [out addObject:@{
            @"key":   [NSString stringWithFormat:@"Servo%dStaticImage", i],
            @"label": @"   Static Image",
            @"kind":  @"imageFile",
            @"value": staticImg ? [NSString stringWithUTF8String:staticImg->GetImageFile().c_str()]
                                : @"",
        }];
        DmxImage* motionImg = ds->GetMotionImage(i);
        [out addObject:@{
            @"key":   [NSString stringWithFormat:@"Servo%dMotionImage", i],
            @"label": @"   Motion Image",
            @"kind":  @"imageFile",
            @"value": motionImg ? [NSString stringWithUTF8String:motionImg->GetImageFile().c_str()]
                                 : @"",
        }];
    }
}

// J-3 (DMX) — DmxServo3d basic surface. Per-servo knobs plus
// counts for static / motion meshes; the mesh-to-servo / servo-
// to-mesh linking matrix is a 2D grid editor that stays
// desktop-only for now (the iPad bridge surfaces the counts
// only — the array itself isn't read back).
static void BuildDmxServo3dProps(DmxServo3d* ds, NSMutableArray* out) {
    [out addObject:MakeIntProp(@"DmxChannelCount", @"# Channels",
                                 ds->GetDmxChannelCount(), 1, 512)];
    [out addObject:MakeIntProp(@"ServoNumServos", @"# Servos",
                                 ds->GetNumServos(), 1, 24)];
    [out addObject:MakeIntProp(@"Servo3dNumStatic", @"# Static Meshes",
                                 ds->GetNumStatic(), 1, 24)];
    [out addObject:MakeIntProp(@"Servo3dNumMotion", @"# Motion Meshes",
                                 ds->GetNumMotion(), 1, 24)];
    [out addObject:MakeBoolProp(@"ServoIs16Bit", @"16-bit Servos",
                                  ds->Is16Bit() ? YES : NO)];
    [out addObject:MakeDoubleProp(@"ServoBrightness", @"Brightness (%)",
                                    (double)ds->GetBrightness(),
                                    0.0, 100.0, 1.0, 1)];
    [out addObject:MakeBoolProp(@"Servo3dShowPivot", @"Show Pivot",
                                  ds->GetShowPivot() ? YES : NO)];
    if (ds->HasPresetAbility()) {
        AppendDmxPresetProps(*ds->GetPresetAbility(), out);
    }
    for (int i = 0; i < ds->GetNumServos(); ++i) {
        NSString* prefix = [NSString stringWithFormat:@"Servo%d", i];
        NSString* hdr = [NSString stringWithFormat:@"Servo %d", i + 1];
        AppendDmxServoKnobProps(prefix, hdr, ds->GetServo(i), out);
    }
    [out addObject:MakeHeaderDescriptor(@"DmxServo3dStaticMeshHeader",
                                          @"Static Meshes")];
    for (int i = 0; i < ds->GetNumStatic(); ++i) {
        Mesh* mesh = ds->GetStaticMesh(i);
        [out addObject:@{
            @"key":   [NSString stringWithFormat:@"Servo3dStatic%dMeshFile", i],
            @"label": [NSString stringWithFormat:@"Static %d", i + 1],
            @"kind":  @"meshFile",
            @"value": mesh ? [NSString stringWithUTF8String:mesh->GetObjFile().c_str()]
                           : @"",
        }];
    }
    [out addObject:MakeHeaderDescriptor(@"DmxServo3dMotionMeshHeader",
                                          @"Motion Meshes")];
    for (int i = 0; i < ds->GetNumMotion(); ++i) {
        Mesh* mesh = ds->GetMotionMesh(i);
        [out addObject:@{
            @"key":   [NSString stringWithFormat:@"Servo3dMotion%dMeshFile", i],
            @"label": [NSString stringWithFormat:@"Motion %d", i + 1],
            @"kind":  @"meshFile",
            @"value": mesh ? [NSString stringWithUTF8String:mesh->GetObjFile().c_str()]
                           : @"",
        }];
    }
    // J-30 — servo / mesh linking. Two parallel enum pickers
    // per servo: which Mesh drives each Servo, and which Servo
    // drives each Mesh. Each picker lists "Mesh 1" .. "Mesh N"
    // where N is the servo count. Desktop's mental model:
    // `servo_links[i] = j` means servo i's motion comes from
    // mesh j; -1 = identity (use i itself). The iPad
    // descriptors flatten -1 to i on emit so the enum can pick
    // any concrete option without a separate "default" entry.
    const int numServos = ds->GetNumServos();
    NSMutableArray<NSString*>* linkOptions = [NSMutableArray array];
    for (int i = 0; i < numServos; ++i) {
        [linkOptions addObject:[NSString stringWithFormat:@"Mesh %d", i + 1]];
    }
    if (numServos > 1) {
        [out addObject:MakeHeaderDescriptor(@"DmxServo3dServoLinkHeader",
                                              @"Servo Linkage")];
        for (int i = 0; i < numServos; ++i) {
            const int raw = ds->GetServoLink(i);
            const int idx = (raw == -1) ? i : raw;
            [out addObject:MakeEnumProp(
                [NSString stringWithFormat:@"Servo3dServo%dLink", i],
                [NSString stringWithFormat:@"Servo %d", i + 1],
                idx,
                linkOptions)];
        }
        [out addObject:MakeHeaderDescriptor(@"DmxServo3dMeshLinkHeader",
                                              @"Mesh Linkage")];
        for (int i = 0; i < numServos; ++i) {
            const int raw = ds->GetMeshLink(i);
            const int idx = (raw == -1) ? i : raw;
            [out addObject:MakeEnumProp(
                [NSString stringWithFormat:@"Servo3dMesh%dLink", i],
                [NSString stringWithFormat:@"Mesh %d", i + 1],
                idx,
                linkOptions)];
        }
    }
}

static void BuildDmxGeneralProps(DmxGeneral* g, NSMutableArray* out) {
    [out addObject:MakeIntProp(@"DmxChannelCount", @"# Channels",
                                 g->GetDmxChannelCount(), 1, 512)];
    if (g->HasPresetAbility()) {
        AppendDmxPresetProps(*g->GetPresetAbility(), out);
    }
    [out addObject:MakeEnumProp(@"DmxColorType", @"Color Type",
                                 DmxColorTypeIndexFor(g),
                                 DmxColorTypeOptions())];
    if (g->HasColorAbility()) {
        AppendDmxColorProps(g->GetColorAbility(), out);
    }
}

static void BuildDmxMovingHeadProps(DmxMovingHead* mh, NSMutableArray* out) {
    [out addObject:MakeEnumProp(@"DmxStyle", @"DMX Style",
                                 mh->GetDmxStyleVal(),
                                 DmxMovingHeadStyleOptions())];
    [out addObject:MakeEnumProp(@"DmxFixture", @"Fixture",
                                 mh->GetFixtureVal() - 1,
                                 DmxMovingHeadFixtureOptions())];
    [out addObject:MakeIntProp(@"DmxChannelCount", @"# Channels",
                                 mh->GetDmxChannelCount(), 1, 512)];
    if (mh->HasPresetAbility()) {
        AppendDmxPresetProps(*mh->GetPresetAbility(), out);
    }
    [out addObject:MakeBoolProp(@"HideBody", @"Hide Body",
                                  mh->GetHideBody() ? YES : NO)];
    if (auto* pan = mh->GetPanMotor()) {
        AppendDmxMotorProps(*pan, out);
    }
    if (auto* tilt = mh->GetTiltMotor()) {
        AppendDmxMotorProps(*tilt, out);
    }
    [out addObject:MakeEnumProp(@"DmxColorType", @"Color Type",
                                 DmxColorTypeIndexFor(mh),
                                 DmxColorTypeOptions())];
    if (mh->HasColorAbility()) {
        AppendDmxColorProps(mh->GetColorAbility(), out);
    }
    if (mh->HasDimmerAbility()) {
        AppendDmxDimmerProps(*mh->GetDimmerAbility(), out);
    }
    if (mh->HasShutterAbility()) {
        AppendDmxShutterProps(*mh->GetShutterAbility(), out);
    }
    if (mh->HasBeamAbility()) {
        AppendDmxBeamProps(*mh->GetBeamAbility(), out);
    }
}

static void BuildCustomProps(CustomModel* cm, NSMutableArray* out) {
    // J-21/J-23 — mirrors CustomPropertyAdapter:
    // Model Data opens the new point/click/drag grid editor;
    // # Strings, optional Indiv Start Nodes, Background Image
    // + Scale + Brightness follow. Matrix dimensions are now
    // editable via the visual editor — surfacing them here as
    // read-only summary rows.
    NSMutableDictionary* d;
    // Open-grid-editor pseudo-descriptor; SwiftUI picks this
    // up via the `customModelData` kind and replaces the row
    // body with an "Edit Grid…" button.
    [out addObject:@{
        @"key":   @"CustomModelData",
        @"label": @"Model Data",
        @"kind":  @"customModelData",
        @"value": @"",
    }];
    d = MakeIntProp(@"CustomWidth", @"Matrix Width",
                     (int)cm->GetCustomWidth(), 1, 10000);
    d[@"enabled"] = @NO; [out addObject:d];
    d = MakeIntProp(@"CustomHeight", @"Matrix Height",
                     (int)cm->GetCustomHeight(), 1, 10000);
    d[@"enabled"] = @NO; [out addObject:d];
    d = MakeIntProp(@"CustomDepth", @"Matrix Depth",
                     (int)cm->GetCustomDepth(), 1, 10000);
    d[@"enabled"] = @NO; [out addObject:d];

    [out addObject:MakeIntProp(@"CustomModelStrings", @"# Strings",
                                cm->GetNumStrings(), 1, 100)];

    if (cm->GetNumStrings() > 1) {
        bool indiv = cm->HasIndivStartNodes();
        [out addObject:MakeBoolProp(@"ModelIndividualStartNodes",
                                     @"Start Nodes", indiv)];
        if (indiv) {
            int strings = cm->GetNumStrings();
            int nodeCount = std::max(1, (int)cm->GetNodeCount());
            for (int i = 0; i < strings; ++i) {
                int v = i < cm->GetIndivStartNodesCount() ? cm->GetIndivStartNode(i)
                                                          : cm->ComputeStringStartNode(i);
                if (v < 1) v = 1;
                if (v > nodeCount) v = nodeCount;
                NSString* nm = [NSString stringWithUTF8String:cm->StartNodeAttrName(i).c_str()];
                NSString* key = [NSString stringWithFormat:@"IndivStartNode%d", i];
                [out addObject:MakeIntProp(key, nm, v, 1, nodeCount)];
            }
        }
    }

    // Background image group — reuse the `imageFile` descriptor
    // kind so users can pick / clear via the same file picker
    // the model Image type uses.
    [out addObject:@{
        @"key":   @"CustomBkgImage",
        @"label": @"Background Image",
        @"kind":  @"imageFile",
        @"value": [NSString stringWithUTF8String:cm->GetCustomBackground().c_str()],
    }];
    [out addObject:MakeIntProp(@"CustomBkgScale", @"Background Scale %",
                                cm->GetCustomBkgScale(), 10, 500)];
    [out addObject:MakeIntProp(@"CustomBkgBrightness", @"Background Brightness %",
                                cm->GetCustomBkgBrightness(), 0, 100)];
}

- (NSArray<NSDictionary*>*)perTypePropertiesForModel:(NSString*)modelName {
    NSMutableArray* out = [NSMutableArray array];
    if (!_context || !_context->HasModelManager() || !modelName) return out;
    Model* m = _context->GetModelManager()[modelName.UTF8String];
    if (!m) return out;

    switch (m->GetDisplayAs()) {
    case DisplayAsType::Tree:
        BuildTreeProps(static_cast<TreeModel*>(m), out);
        break;
    case DisplayAsType::Sphere:
        BuildSphereProps(static_cast<SphereModel*>(m), out);
        break;
    case DisplayAsType::Matrix:
        BuildMatrixProps(static_cast<MatrixModel*>(m), out);
        break;
    case DisplayAsType::Star:
        BuildStarProps(static_cast<StarModel*>(m), out);
        break;
    case DisplayAsType::Arches:
        BuildArchesProps(static_cast<ArchesModel*>(m), out);
        break;
    case DisplayAsType::Icicles:
        BuildIciclesProps(static_cast<IciclesModel*>(m), out);
        break;
    case DisplayAsType::Circle:
        BuildCircleProps(static_cast<CircleModel*>(m), out);
        break;
    case DisplayAsType::Wreath:
        BuildWreathProps(static_cast<WreathModel*>(m), out);
        break;
    case DisplayAsType::SingleLine:
        BuildSingleLineProps(static_cast<SingleLineModel*>(m), out);
        break;
    case DisplayAsType::CandyCanes:
        BuildCandyCaneProps(static_cast<CandyCaneModel*>(m), out);
        break;
    case DisplayAsType::Spinner:
        BuildSpinnerProps(static_cast<SpinnerModel*>(m), out);
        break;
    case DisplayAsType::WindowFrame:
        BuildWindowFrameProps(static_cast<WindowFrameModel*>(m), out);
        break;
    case DisplayAsType::Cube:
        BuildCubeProps(static_cast<CubeModel*>(m), out);
        break;
    case DisplayAsType::ChannelBlock:
        BuildChannelBlockProps(static_cast<ChannelBlockModel*>(m), out);
        break;
    case DisplayAsType::Image:
        BuildImageProps(static_cast<ImageModel*>(m), out);
        break;
    case DisplayAsType::Label:
        BuildLabelProps(static_cast<LabelModel*>(m), out);
        break;
    case DisplayAsType::MultiPoint:
        BuildMultiPointProps(static_cast<MultiPointModel*>(m), out);
        break;
    case DisplayAsType::PolyLine:
        BuildPolyLineProps(static_cast<PolyLineModel*>(m), out);
        break;
    case DisplayAsType::Custom:
        BuildCustomProps(static_cast<CustomModel*>(m), out);
        break;
    case DisplayAsType::DmxFloodlight:
    case DisplayAsType::DmxFloodArea:
        // DmxFloodArea is a DmxFloodlight subclass with only a
        // different DrawModel — the property surface is identical.
        BuildDmxFloodlightProps(static_cast<DmxFloodlight*>(m), out);
        break;
    case DisplayAsType::DmxMovingHead:
        BuildDmxMovingHeadProps(static_cast<DmxMovingHead*>(m), out);
        break;
    case DisplayAsType::DmxMovingHeadAdv:
        BuildDmxMovingHeadAdvProps(static_cast<DmxMovingHeadAdv*>(m), out);
        break;
    case DisplayAsType::DmxGeneral:
        BuildDmxGeneralProps(static_cast<DmxGeneral*>(m), out);
        break;
    case DisplayAsType::DmxSkull:
        BuildDmxSkullProps(static_cast<DmxSkull*>(m), out);
        break;
    case DisplayAsType::DmxServo:
        BuildDmxServoProps(static_cast<DmxServo*>(m), out);
        break;
    case DisplayAsType::DmxServo3d:
        BuildDmxServo3dProps(static_cast<DmxServo3d*>(m), out);
        break;
    default:
        break;
    }
    return out;
}

- (BOOL)setPerTypeProperty:(NSString*)key
                   onModel:(NSString*)modelName
                     value:(id)value {
    if (!_context || !_context->HasModelManager() || !modelName || !key) return NO;
    Model* m = _context->GetModelManager()[modelName.UTF8String];
    if (!m) return NO;
    _context->AbortRender(5000);

    auto asInt = ^int(BOOL* ok) {
        if ([value isKindOfClass:[NSNumber class]]) { *ok = YES; return [(NSNumber*)value intValue]; }
        *ok = NO; return 0;
    };
    auto asDouble = ^double(BOOL* ok) {
        if ([value isKindOfClass:[NSNumber class]]) { *ok = YES; return [(NSNumber*)value doubleValue]; }
        *ok = NO; return 0;
    };
    auto asBool = ^BOOL(BOOL* ok) {
        if ([value isKindOfClass:[NSNumber class]]) { *ok = YES; return [(NSNumber*)value boolValue]; }
        *ok = NO; return NO;
    };
    auto asString = ^std::string(BOOL* ok) {
        if ([value isKindOfClass:[NSString class]]) { *ok = YES; return std::string([(NSString*)value UTF8String]); }
        *ok = NO; return std::string();
    };

    BOOL ok = NO;
    BOOL changed = NO;
    std::string k = key.UTF8String;

    // Matrix family (Matrix / Tree / Sphere) all share these keys
    // because Tree + Sphere extend Matrix. Specialised setters fall
    // through and dispatch after the common block.
    if (k == "MatrixStyle" || k == "StrandDir") {
        // J-20 — both keys map to MatrixModel::SetVertical. Tree
        // uses StrandDir; bare Matrix uses MatrixStyle. Same
        // underlying field.
        int v = asInt(&ok); if (!ok) return NO;
        auto* mat = dynamic_cast<MatrixModel*>(m);
        if (!mat) return NO;
        if (mat->isVerticalMatrix() != (v != 0)) { mat->SetVertical(v != 0); changed = YES; }
    } else if (k == "AlternateNodes") {
        BOOL v = asBool(&ok); if (!ok) return NO;
        if (auto* mat = dynamic_cast<MatrixModel*>(m); mat && mat->HasAlternateNodes() != (v?true:false)) {
            mat->SetAlternateNodes(v); changed = YES;
        } else if (auto* ic = dynamic_cast<IciclesModel*>(m)) {
            if (ic->HasAlternateNodes() != (v?true:false)) { ic->SetAlternateNodes(v); changed = YES; }
        } else if (auto* cc = dynamic_cast<CandyCaneModel*>(m)) {
            if (cc->HasAlternateNodes() != (v?true:false)) { cc->SetAlternateNodes(v); changed = YES; }
        } else if (auto* pl = dynamic_cast<PolyLineModel*>(m)) {
            // J-20.2 — PolyLine shares the AlternateNodes key.
            if (pl->HasAlternateNodes() != (v?true:false)) { pl->SetAlternateNodes(v); changed = YES; }
        }
    } else if (k == "NoZig") {
        BOOL v = asBool(&ok); if (!ok) return NO;
        auto* mat = dynamic_cast<MatrixModel*>(m);
        if (mat && mat->IsNoZigZag() != (v?true:false)) { mat->SetNoZigZag(v); changed = YES; }
    } else if (k == "MatrixStringCount") {
        int v = asInt(&ok); if (!ok) return NO;
        auto* mat = dynamic_cast<MatrixModel*>(m);
        if (mat && mat->GetNumPhysicalStrings() != v) { mat->SetNumMatrixStrings(v); changed = YES; }
    } else if (k == "MatrixLightCount") {
        int v = asInt(&ok); if (!ok) return NO;
        auto* mat = dynamic_cast<MatrixModel*>(m);
        if (mat && mat->GetNodesPerString() != v) { mat->SetNodesPerString(v); changed = YES; }
    } else if (k == "MatrixStrandCount") {
        int v = asInt(&ok); if (!ok) return NO;
        auto* mat = dynamic_cast<MatrixModel*>(m);
        if (mat && mat->GetStrandsPerString() != v) { mat->SetStrandsPerString(v); changed = YES; }
    } else if (k == "MatrixStart" || k == "WreathStart") {
        int v = asInt(&ok); if (!ok) return NO;
        if (auto* sp = dynamic_cast<SpinnerModel*>(m)) {
            // J-20 — Spinner has a 6-choice enum that maps to
            // _alternate + IsLtoR + isBotToTop via Decode.
            sp->DecodeStartLocation(v);
            sp->SetDirection(sp->GetIsLtoR() ? "L" : "R");
            sp->SetStartSide(sp->GetIsBtoT() ? "B" : "T");
        } else {
            ApplyStartCorner(m, v);
        }
        changed = YES;
    }
    // Tree
    else if (k == "TreeStyle") {
        int v = asInt(&ok); if (!ok) return NO;
        auto* t = dynamic_cast<TreeModel*>(m);
        if (t && t->GetTreeType() != v) { t->SetTreeType(v); changed = YES; }
    } else if (k == "TreeDegrees") {
        int v = asInt(&ok); if (!ok) return NO;
        auto* t = dynamic_cast<TreeModel*>(m);
        if (t && (int)t->GetTreeDegrees() != v) { t->SetTreeDegrees(v); changed = YES; }
    } else if (k == "TreeRotation") {
        double v = asDouble(&ok); if (!ok) return NO;
        auto* t = dynamic_cast<TreeModel*>(m);
        if (t && t->GetTreeRotation() != (float)v) { t->SetTreeRotation((float)v); changed = YES; }
    } else if (k == "TreeSpiralRotations") {
        double v = asDouble(&ok); if (!ok) return NO;
        auto* t = dynamic_cast<TreeModel*>(m);
        if (t && t->GetSpiralRotations() != (float)v) { t->SetTreeSpiralRotations((float)v); changed = YES; }
    } else if (k == "TreeBottomTopRatio") {
        double v = asDouble(&ok); if (!ok) return NO;
        auto* t = dynamic_cast<TreeModel*>(m);
        if (t && t->GetBottomTopRatio() != (float)v) { t->SetTreeBottomTopRatio((float)v); changed = YES; }
    } else if (k == "TreePerspective") {
        // Stored ÷10 internally (desktop scales by 10 for display).
        double v = asDouble(&ok); if (!ok) return NO;
        auto* t = dynamic_cast<TreeModel*>(m);
        if (t) { t->SetPerspective((float)(v / 10.0)); changed = YES; }
    }
    // Sphere
    else if (k == "SphereStartLatitude") {
        int v = asInt(&ok); if (!ok) return NO;
        auto* s = dynamic_cast<SphereModel*>(m);
        if (s && s->GetStartLatitude() != v) { s->SetStartLatitude(v); changed = YES; }
    } else if (k == "SphereEndLatitude") {
        int v = asInt(&ok); if (!ok) return NO;
        auto* s = dynamic_cast<SphereModel*>(m);
        if (s && s->GetEndLatitude() != v) { s->SetEndLatitude(v); changed = YES; }
    } else if (k == "SphereDegrees") {
        int v = asInt(&ok); if (!ok) return NO;
        auto* s = dynamic_cast<SphereModel*>(m);
        if (s && s->GetSphereDegrees() != v) { s->SetDegrees(v); changed = YES; }
    }
    // Star
    else if (k == "StarStringCount") {
        int v = asInt(&ok); if (!ok) return NO;
        auto* s = dynamic_cast<StarModel*>(m);
        if (s && s->GetNumStarStrings() != v) { s->SetNumStarStrings(v); changed = YES; }
    } else if (k == "StarLightCount") {
        int v = asInt(&ok); if (!ok) return NO;
        auto* s = dynamic_cast<StarModel*>(m);
        if (s && s->GetNodesPerString() != v) { s->SetStarNodesPerString(v); changed = YES; }
    } else if (k == "StarStrandCount") {
        int v = asInt(&ok); if (!ok) return NO;
        auto* s = dynamic_cast<StarModel*>(m);
        if (s && s->GetStarPoints() != v) { s->SetStarPoints(v); changed = YES; }
    } else if (k == "StarStart") {
        int v = asInt(&ok); if (!ok) return NO;
        auto* s = dynamic_cast<StarModel*>(m);
        // J-20 — Star has 12 named start locations, not the 4
        // matrix corners. Mirrors StarPropertyAdapter.cpp.
        NSArray* opts = StarStartLocationOptions();
        if (s && v >= 0 && v < (int)opts.count) {
            std::string loc = [(NSString*)opts[v] UTF8String];
            s->SetStarStartLocation(loc); changed = YES;
        }
    } else if (k == "StarRatio") {
        double v = asDouble(&ok); if (!ok) return NO;
        auto* s = dynamic_cast<StarModel*>(m);
        if (s && s->GetStarRatio() != (float)v) { s->SetStarRatio((float)v); changed = YES; }
    } else if (k == "StarCenterPercent") {
        int v = asInt(&ok); if (!ok) return NO;
        auto* s = dynamic_cast<StarModel*>(m);
        if (s && s->GetInnerPercent() != v) { s->SetInnerPercent(v); changed = YES; }
    }
    // Arches
    else if (k == "ArchesCount") {
        int v = asInt(&ok); if (!ok) return NO;
        auto* a = dynamic_cast<ArchesModel*>(m);
        if (a && a->GetNumArches() != v) { a->SetNumArches(v); changed = YES; }
    } else if (k == "ArchesNodes") {
        int v = asInt(&ok); if (!ok) return NO;
        auto* a = dynamic_cast<ArchesModel*>(m);
        if (a && a->GetNodesPerArch() != v) { a->SetNodesPerArch(v); changed = YES; }
    } else if (k == "ArchesLights") {
        int v = asInt(&ok); if (!ok) return NO;
        auto* a = dynamic_cast<ArchesModel*>(m);
        if (a && a->GetLightsPerNode() != v) { a->SetLightsPerNode(v); changed = YES; }
    } else if (k == "ArchesArc") {
        int v = asInt(&ok); if (!ok) return NO;
        auto* a = dynamic_cast<ArchesModel*>(m);
        if (a && a->GetArc() != v) { a->SetArc(v); changed = YES; }
    } else if (k == "ArchesHollow") {
        int v = asInt(&ok); if (!ok) return NO;
        auto* a = dynamic_cast<ArchesModel*>(m);
        if (a && a->GetHollow() != v) { a->SetHollow(v); changed = YES; }
    } else if (k == "ArchesZigZag") {
        BOOL v = asBool(&ok); if (!ok) return NO;
        auto* a = dynamic_cast<ArchesModel*>(m);
        if (a && a->GetZigZag() != (v?true:false)) { a->SetZigZag(v); changed = YES; }
    } else if (k == "ArchesGap") {
        int v = asInt(&ok); if (!ok) return NO;
        auto* a = dynamic_cast<ArchesModel*>(m);
        if (a && a->GetGap() != v) { a->SetGap(v); changed = YES; }
    } else if (k == "ArchesSkew") {
        int v = asInt(&ok); if (!ok) return NO;
        auto* a = dynamic_cast<ArchesModel*>(m);
        if (a) {
            auto& threePt = dynamic_cast<ThreePointScreenLocation&>(a->GetModelScreenLocation());
            if (threePt.GetAngle() != v) { threePt.SetAngle(v); changed = YES; }
        }
    } else if (k == "ArchesStart") {
        int v = asInt(&ok); if (!ok) return NO;
        auto* a = dynamic_cast<ArchesModel*>(m);
        if (a) {
            if (a->GetLayerSizeCount() != 0) {
                // 4-choice: (IsLtoR?0:2) + (IsBtoT?1:0)
                a->SetDirection((v == 0 || v == 1) ? "L" : "R");
                a->SetStartSide((v == 0 || v == 2) ? "T" : "B");
                a->SetIsBtoT(v != 0 && v != 2);
            } else {
                a->SetDirection(v == 0 ? "L" : "R");
            }
            changed = YES;
        }
    } else if (k == "LayeredArches") {
        BOOL v = asBool(&ok); if (!ok) return NO;
        auto* a = dynamic_cast<ArchesModel*>(m);
        if (a) {
            if (v) {
                a->SetNumArches(1);
                a->SetLayerSizeCount(1);
                a->SetLayerSize(0, a->GetNodesPerArch());
            } else {
                a->SetLayerSizeCount(0);
            }
            a->OnLayerSizesChange(true);
            changed = YES;
        }
    } else if (k == "LayerSizes") {
        // J-20.7 — Wholesale-replace the layer-size vector on any
        // model that supports the desktop's AddLayerSizeProperty
        // helper (Arches, Star, Circle). Empty array clears layers
        // (same effect as turning the "layered" toggle off).
        // SetLayerSizeCount + SetLayerSize live on the Model base
        // so a generic Model* call is enough; OnLayerSizesChange
        // is the per-type hook (only Arches overrides it today).
        if (![value isKindOfClass:[NSArray class]]) return NO;
        if (m->ModelSupportsLayerSizes()) {
            NSArray* arr = (NSArray*)value;
            m->SetLayerSizeCount((int)arr.count);
            for (NSUInteger i = 0; i < arr.count; ++i) {
                int sz = [arr[i] intValue];
                if (sz < 1) sz = 1;
                m->SetLayerSize(i, sz);
            }
            m->OnLayerSizesChange(true);
            changed = YES;
        }
    }
    // Icicles
    else if (k == "IciclesStrings") {
        int v = asInt(&ok); if (!ok) return NO;
        auto* ic = dynamic_cast<IciclesModel*>(m);
        if (ic && ic->GetNumIcicleStrings() != v) { ic->SetNumIcicleStrings(v); changed = YES; }
    } else if (k == "IciclesLights") {
        int v = asInt(&ok); if (!ok) return NO;
        auto* ic = dynamic_cast<IciclesModel*>(m);
        if (ic && ic->GetLightsPerString() != v) { ic->SetLightsPerString(v); changed = YES; }
    } else if (k == "IciclesDrops") {
        // J-21.1 — Shared key; both IciclesModel and PolyLineModel
        // expose `SetDropPattern`. Dispatch on the live type.
        std::string v = asString(&ok); if (!ok) return NO;
        if (auto* ic = dynamic_cast<IciclesModel*>(m)) {
            if (ic->GetDropPattern() != v) { ic->SetDropPattern(v); changed = YES; }
        } else if (auto* pl = dynamic_cast<PolyLineModel*>(m)) {
            if (pl->GetDropPattern() != v) { pl->SetDropPattern(v); changed = YES; }
        }
    } else if (k == "IciclesStart") {
        int v = asInt(&ok); if (!ok) return NO;
        m->SetDirection(v == 0 ? "L" : "R");
        m->SetIsLtoR(v == 0);
        changed = YES;
    }
    // Circle
    else if (k == "CircleStringCount") {
        int v = asInt(&ok); if (!ok) return NO;
        auto* c = dynamic_cast<CircleModel*>(m);
        if (c && c->GetNumCircleStrings() != v) { c->SetNumCircleStrings(v); changed = YES; }
    } else if (k == "CircleLightCount") {
        int v = asInt(&ok); if (!ok) return NO;
        auto* c = dynamic_cast<CircleModel*>(m);
        if (c && c->GetNodesPerString() != v) { c->SetCircleNodesPerString(v); changed = YES; }
    } else if (k == "CircleCenterPercent") {
        int v = asInt(&ok); if (!ok) return NO;
        auto* c = dynamic_cast<CircleModel*>(m);
        if (c && c->GetCenterPercent() != v) { c->SetCenterPercent(v); changed = YES; }
    } else if (k == "CircleStart") {
        int v = asInt(&ok); if (!ok) return NO;
        auto* c = dynamic_cast<CircleModel*>(m);
        if (c) {
            c->SetDirection(v & 0x1 ? "L" : "R");
            c->SetStartSide(v < 4 ? "T" : "B");
            c->SetIsLtoR(v & 0x1);
            c->SetIsBtoT(v >= 4);
            c->SetInsideOut((v & 0x2) != 0);
            changed = YES;
        }
    }
    // Wreath
    else if (k == "WreathStringCount") {
        int v = asInt(&ok); if (!ok) return NO;
        auto* w = dynamic_cast<WreathModel*>(m);
        if (w && w->GetNumWreathStrings() != v) { w->SetNumWreathStrings(v); changed = YES; }
    } else if (k == "WreathLightCount") {
        int v = asInt(&ok); if (!ok) return NO;
        auto* w = dynamic_cast<WreathModel*>(m);
        if (w && w->GetNodesPerString() != v) { w->SetWreathNodesPerString(v); changed = YES; }
    }
    // SingleLine
    else if (k == "SingleLineCount") {
        int v = asInt(&ok); if (!ok) return NO;
        auto* l = dynamic_cast<SingleLineModel*>(m);
        if (l && l->GetNumLines() != v) { l->SetNumLines(v); changed = YES; }
    } else if (k == "SingleLineNodes") {
        int v = asInt(&ok); if (!ok) return NO;
        auto* l = dynamic_cast<SingleLineModel*>(m);
        if (l && l->GetNodesPerString() != v) { l->SetNodesPerLine(v); changed = YES; }
    } else if (k == "SingleLineLights") {
        int v = asInt(&ok); if (!ok) return NO;
        auto* l = dynamic_cast<SingleLineModel*>(m);
        if (l && l->GetLightsPerNode() != v) { l->SetLightsPerNode(v); changed = YES; }
    } else if (k == "SingleLineStart") {
        int v = asInt(&ok); if (!ok) return NO;
        m->SetDirection(v == 0 ? "L" : "R");
        m->SetIsLtoR(v == 0);
        changed = YES;
    }
    // CandyCane
    else if (k == "CandyCaneCount") {
        int v = asInt(&ok); if (!ok) return NO;
        auto* c = dynamic_cast<CandyCaneModel*>(m);
        if (c && c->GetNumCanes() != v) { c->SetNumCanes(v); changed = YES; }
    } else if (k == "CandyCaneNodes") {
        int v = asInt(&ok); if (!ok) return NO;
        auto* c = dynamic_cast<CandyCaneModel*>(m);
        if (c) {
            if (c->IsSingleNode()) {
                if (c->GetLightsPerNode() != v) { c->SetLightsPerNode(v); changed = YES; }
            } else {
                // CandyCaneModel exposes SetNumCanes but no
                // SetNodesPerCane; it's read from xml at init.
                // Set the wire-format attribute directly via
                // ChangeModelXml — falls back to a no-op if the
                // header doesn't expose the setter.
                // For first cut: skip persisting nodes/cane edits.
            }
        }
    } else if (k == "CandyCaneLights") {
        int v = asInt(&ok); if (!ok) return NO;
        auto* c = dynamic_cast<CandyCaneModel*>(m);
        if (c && c->GetLightsPerNode() != v) { c->SetLightsPerNode(v); changed = YES; }
    } else if (k == "CandyCaneHeight") {
        double v = asDouble(&ok); if (!ok) return NO;
        auto* c = dynamic_cast<CandyCaneModel*>(m);
        if (c && c->GetCandyCaneHeight() != (float)v) {
            // No public setter exposed; future setter wire-up.
        }
    } else if (k == "CandyCaneReverse") {
        BOOL v = asBool(&ok); if (!ok) return NO;
        auto* c = dynamic_cast<CandyCaneModel*>(m);
        if (c && c->IsReverse() != (v?true:false)) { c->SetReverse(v); changed = YES; }
    } else if (k == "CandyCaneSticks") {
        BOOL v = asBool(&ok); if (!ok) return NO;
        auto* c = dynamic_cast<CandyCaneModel*>(m);
        if (c && c->IsSticks() != (v?true:false)) { c->SetSticks(v); changed = YES; }
    } else if (k == "CandyCaneStart") {
        int v = asInt(&ok); if (!ok) return NO;
        m->SetDirection(v == 0 ? "L" : "R");
        m->SetIsLtoR(v == 0);
        changed = YES;
    }
    // Spinner — J-20 keys match desktop SpinnerPropertyAdapter
    else if (k == "SpinnerStringCount") {
        int v = asInt(&ok); if (!ok) return NO;
        auto* s = dynamic_cast<SpinnerModel*>(m);
        if (s && s->GetNumSpinnerStrings() != v) { s->SetNumSpinnerStrings(v); changed = YES; }
    } else if (k == "FoldCount" || k == "SpinnerArmCount") {
        int v = asInt(&ok); if (!ok) return NO;
        auto* s = dynamic_cast<SpinnerModel*>(m);
        if (s && s->GetArmsPerString() != v) { s->SetArmsPerString(v); changed = YES; }
    } else if (k == "SpinnerArmNodeCount" || k == "SpinnerNodesPerArm") {
        int v = asInt(&ok); if (!ok) return NO;
        auto* s = dynamic_cast<SpinnerModel*>(m);
        if (s && s->GetNodesPerArm() != v) { s->SetNodesPerArm(v); changed = YES; }
    } else if (k == "Hollow" || k == "SpinnerHollow") {
        int v = asInt(&ok); if (!ok) return NO;
        auto* s = dynamic_cast<SpinnerModel*>(m);
        if (s && s->GetHollowPercent() != v) { s->SetHollow(v); changed = YES; }
    } else if (k == "Arc" || k == "SpinnerArc") {
        int v = asInt(&ok); if (!ok) return NO;
        auto* s = dynamic_cast<SpinnerModel*>(m);
        if (s && s->GetArcAngle() != v) { s->SetArc(v); changed = YES; }
    } else if (k == "StartAngle" || k == "SpinnerStartAngle") {
        int v = asInt(&ok); if (!ok) return NO;
        auto* s = dynamic_cast<SpinnerModel*>(m);
        if (s && s->GetStartAngle() != v) { s->SetStartAngle(v); changed = YES; }
    } else if (k == "ZigZag" || k == "SpinnerZigZag") {
        BOOL v = asBool(&ok); if (!ok) return NO;
        auto* s = dynamic_cast<SpinnerModel*>(m);
        if (s && s->HasZigZag() != (v?true:false)) { s->SetZigZag(v); changed = YES; }
    } else if (k == "SpinnerAlternate") {
        // Not used by the desktop SpinnerPropertyAdapter today but
        // legacy iPad summaries may still send it; keep the setter
        // so older preferences don't error.
        BOOL v = asBool(&ok); if (!ok) return NO;
        auto* s = dynamic_cast<SpinnerModel*>(m);
        if (s && s->HasAlternateNodes() != (v?true:false)) { s->SetAlternate(v); changed = YES; }
    }
    // WindowFrame
    else if (k == "WindowFrameTopNodes") {
        int v = asInt(&ok); if (!ok) return NO;
        auto* w = dynamic_cast<WindowFrameModel*>(m);
        if (w && w->GetTopNodes() != v) { w->SetTopNodes(v); changed = YES; }
    } else if (k == "WindowFrameSideNodes") {
        int v = asInt(&ok); if (!ok) return NO;
        auto* w = dynamic_cast<WindowFrameModel*>(m);
        if (w && w->GetSideNodes() != v) { w->SetSideNodes(v); changed = YES; }
    } else if (k == "WindowFrameBottomNodes") {
        int v = asInt(&ok); if (!ok) return NO;
        auto* w = dynamic_cast<WindowFrameModel*>(m);
        if (w && w->GetBottomNodes() != v) { w->SetBottomNodes(v); changed = YES; }
    } else if (k == "WindowFrameRotation") {
        int v = asInt(&ok); if (!ok) return NO;
        auto* w = dynamic_cast<WindowFrameModel*>(m);
        if (w && w->GetRotation() != v) { w->SetRotation(v); changed = YES; }
    }
    // Cube
    else if (k == "CubeWidth") {
        int v = asInt(&ok); if (!ok) return NO;
        auto* c = dynamic_cast<CubeModel*>(m);
        if (c && c->GetCubeWidth() != v) { c->SetCubeWidth(v); changed = YES; }
    } else if (k == "CubeHeight") {
        int v = asInt(&ok); if (!ok) return NO;
        auto* c = dynamic_cast<CubeModel*>(m);
        if (c && c->GetCubeHeight() != v) { c->SetCubeHeight(v); changed = YES; }
    } else if (k == "CubeDepth") {
        int v = asInt(&ok); if (!ok) return NO;
        auto* c = dynamic_cast<CubeModel*>(m);
        if (c && c->GetCubeDepth() != v) { c->SetCubeDepth(v); changed = YES; }
    } else if (k == "CubeStrings") {
        int v = asInt(&ok); if (!ok) return NO;
        auto* c = dynamic_cast<CubeModel*>(m);
        if (c && c->GetCubeStrings() != v) { c->SetCubeStrings(v); changed = YES; }
    } else if (k == "CubeStyle" || k == "CubeStyleIdx") {
        int v = asInt(&ok); if (!ok) return NO;
        auto* c = dynamic_cast<CubeModel*>(m);
        if (c && c->GetCubeStyleIndex() != v) { c->SetCubeStyleIndex(v); changed = YES; }
    } else if (k == "CubeStart" || k == "CubeStartIdx") {
        int v = asInt(&ok); if (!ok) return NO;
        auto* c = dynamic_cast<CubeModel*>(m);
        if (c && c->GetCubeStartIndex() != v) { c->SetCubeStartIndex(v); changed = YES; }
    } else if (k == "StrandPerLine") {
        int v = asInt(&ok); if (!ok) return NO;
        auto* c = dynamic_cast<CubeModel*>(m);
        if (c && c->GetStrandStyleIndex() != v) { c->SetStrandStyleIndex(v); changed = YES; }
    } else if (k == "StrandPerLayer") {
        BOOL v = asBool(&ok); if (!ok) return NO;
        auto* c = dynamic_cast<CubeModel*>(m);
        if (c && c->IsStrandPerLayer() != (v?true:false)) {
            c->SetStrandPerLayer(v?true:false); changed = YES;
        }
    }
    // ChannelBlock
    else if (k == "ChannelBlockChannels") {
        int v = asInt(&ok); if (!ok) return NO;
        auto* cb = dynamic_cast<ChannelBlockModel*>(m);
        if (cb && cb->GetNumChannels() != v) { cb->SetNumChannels(v); changed = YES; }
    }
    // J-21 — Custom Model
    else if (k == "CustomModelStrings") {
        int v = asInt(&ok); if (!ok) return NO;
        auto* cm = dynamic_cast<CustomModel*>(m);
        if (cm && cm->GetNumStrings() != v) {
            cm->SetNumStrings(v);
            // Desktop mirrors this: forcing indiv-start-nodes on
            // when strings > 1 + populating per-string defaults.
            cm->SetHasIndivStartNodes(v > 1);
            if (v > 1) {
                cm->SetIndivStartNodesCount(v);
                for (int x = 0; x < v; ++x) {
                    cm->SetIndivStartNode(x, cm->ComputeStringStartNode(x));
                }
            }
            changed = YES;
        }
    } else if (k == "CustomBkgImage") {
        std::string v = asString(&ok); if (!ok) return NO;
        auto* cm = dynamic_cast<CustomModel*>(m);
        if (cm && v != cm->GetCustomBackground()) {
            cm->SetCustomBackground(v);
            changed = YES;
        }
    } else if (k == "CustomBkgScale") {
        int v = asInt(&ok); if (!ok) return NO;
        auto* cm = dynamic_cast<CustomModel*>(m);
        if (cm && cm->GetCustomBkgScale() != v) { cm->SetCustomBkgScale(v); changed = YES; }
    } else if (k == "CustomBkgBrightness") {
        int v = asInt(&ok); if (!ok) return NO;
        auto* cm = dynamic_cast<CustomModel*>(m);
        if (cm && cm->GetCustomBkgBrightness() != v) { cm->SetCustomBkgBrightness(v); changed = YES; }
    }
    // J-20 — Image
    else if (k == "Image") {
        std::string v = asString(&ok); if (!ok) return NO;
        auto* im = dynamic_cast<ImageModel*>(m);
        if (im && v != im->GetImageFile()) {
            im->ClearImageCache();
            im->SetImageFile(v); changed = YES;
        }
    } else if (k == "OffBrightness") {
        int v = asInt(&ok); if (!ok) return NO;
        auto* im = dynamic_cast<ImageModel*>(m);
        if (im && im->GetOffBrightness() != v) { im->SetOffBrightness(v); changed = YES; }
    } else if (k == "WhiteAsAlpha") {
        BOOL v = asBool(&ok); if (!ok) return NO;
        auto* im = dynamic_cast<ImageModel*>(m);
        if (im && im->IsWhiteAsAlpha() != (v?true:false)) {
            im->ClearImageCache();
            im->SetWhiteAsAlpha(v?true:false); changed = YES;
        }
    }
    // J-21 — Indiv Start Nodes (shared between MultiPoint and
    // PolyLine). Toggle flips `_hasIndivNodes`; per-string key
    // `IndivStartNode<N>` writes through `SetIndivStartNode`.
    else if (k == "ModelIndividualStartNodes") {
        BOOL v = asBool(&ok); if (!ok) return NO;
        bool desired = v ? true : false;
        if (m->HasIndivStartNodes() != desired) {
            m->SetHasIndivStartNodes(desired);
            if (desired) {
                int strings = m->GetNumStrings();
                while (m->GetIndivStartNodesCount() < strings) {
                    m->AddIndivStartNode(m->ComputeStringStartNode(m->GetIndivStartNodesCount()));
                }
                if (m->GetIndivStartNodesCount() > strings) {
                    m->SetIndivStartNodesCount(strings);
                }
            }
            changed = YES;
        }
    } else if (k.starts_with("IndivStartNode")) {
        std::string idxStr = k.substr(std::string("IndivStartNode").size());
        char* end = nullptr;
        long idx = std::strtol(idxStr.c_str(), &end, 10);
        if (end == idxStr.c_str()) return NO;
        int v = asInt(&ok); if (!ok) return NO;
        if (idx < 0) return NO;
        while (m->GetIndivStartNodesCount() <= (int)idx) {
            m->AddIndivStartNode(m->ComputeStringStartNode(m->GetIndivStartNodesCount()));
        }
        if (m->GetIndivStartNode((size_t)idx) != v) {
            m->SetIndivStartNode((int)idx, v);
            changed = YES;
        }
    }
    // J-20.2 — MultiPoint
    else if (k == "MultiPointNodes") {
        int v = asInt(&ok); if (!ok) return NO;
        auto* mp = dynamic_cast<MultiPointModel*>(m);
        if (mp && mp->GetNumPoints() != v) { mp->SetNumPoints(v); changed = YES; }
    } else if (k == "MultiPointStrings") {
        int v = asInt(&ok); if (!ok) return NO;
        auto* mp = dynamic_cast<MultiPointModel*>(m);
        if (mp && mp->GetNumStrings() != v) { mp->SetNumStrings(v); changed = YES; }
    }
    // J-20.2 — PolyLine
    else if (k == "PolyLineNodes") {
        int v = asInt(&ok); if (!ok) return NO;
        auto* pl = dynamic_cast<PolyLineModel*>(m);
        if (pl && pl->GetTotalLightCount() != v) { pl->SetTotalLightCount(v); changed = YES; }
    } else if (k == "PolyLineLights") {
        int v = asInt(&ok); if (!ok) return NO;
        auto* pl = dynamic_cast<PolyLineModel*>(m);
        if (pl && pl->GetLightsPerNode() != v) { pl->SetLightsPerNode(v); changed = YES; }
    } else if (k == "PolyLineStrings") {
        int v = asInt(&ok); if (!ok) return NO;
        auto* pl = dynamic_cast<PolyLineModel*>(m);
        if (pl && pl->GetNumStrings() != v) { pl->SetNumStrings(v); changed = YES; }
    } else if (k == "PolyLineStart") {
        int v = asInt(&ok); if (!ok) return NO;
        m->SetDirection(v == 0 ? "L" : "R");
        m->SetIsLtoR(v == 0);
        changed = YES;
    }
    // J-21.1 — Per-segment size. Editing turns off auto-distribute
    // so the user's manual count survives (matches desktop's
    // `SetAutoDistribute(false)` in the segment-edit handler).
    else if (k.starts_with("PolySegmentSize")) {
        std::string idxStr = k.substr(std::string("PolySegmentSize").size());
        char* end = nullptr;
        long idx = std::strtol(idxStr.c_str(), &end, 10);
        if (end == idxStr.c_str()) return NO;
        int v = asInt(&ok); if (!ok) return NO;
        auto* pl = dynamic_cast<PolyLineModel*>(m);
        if (pl && idx >= 0 && idx < pl->GetNumSegments()) {
            pl->SetRawSegmentSize((int)idx, v);
            pl->SetAutoDistribute(false);
            changed = YES;
        }
    }
    // J-21.1 — Per-corner enum. The 3-choice value maps to
    // lead/trail offsets at the segment boundaries: Leading =
    // lead 1.0 / trail 0.0; Trailing = lead 0.0 / trail 1.0;
    // Neither = lead 0.5 / trail 0.5. First / last corners only
    // touch one side. Mirrors PolyLinePropertyAdapter.cpp:171-177.
    else if (k.starts_with("PolyCorner")) {
        std::string idxStr = k.substr(std::string("PolyCorner").size());
        char* end = nullptr;
        long idx = std::strtol(idxStr.c_str(), &end, 10);
        if (end == idxStr.c_str()) return NO;
        int v = asInt(&ok); if (!ok) return NO;
        auto* pl = dynamic_cast<PolyLineModel*>(m);
        if (pl && idx >= 0 && idx <= pl->GetNumSegments()) {
            static const char* CORNER_NAMES[] = {
                "Leading Segment", "Trailing Segment", "Neither"
            };
            if (v < 0 || v > 2) return NO;
            std::string cornerVal = CORNER_NAMES[v];
            pl->SetCornerString((int)idx, cornerVal);
            int numSegments = pl->GetNumSegments();
            float leadOn  = (cornerVal == "Leading Segment")  ? 1.0f
                          : (cornerVal == "Trailing Segment") ? 0.0f
                          :                                      0.5f;
            float trailOn = (cornerVal == "Leading Segment")  ? 0.0f
                          : (cornerVal == "Trailing Segment") ? 1.0f
                          :                                      0.5f;
            if (idx == 0) {
                pl->SetLeadOffset((int)idx, leadOn);
            } else if (idx == numSegments) {
                pl->SetTrailOffset((int)idx - 1, trailOn);
            } else {
                pl->SetTrailOffset((int)idx - 1, trailOn);
                pl->SetLeadOffset((int)idx, leadOn);
            }
            changed = YES;
        }
    }
    // J-20.2 — shared between Multi/PolyLine: Height (key matches
    // desktop "ModelHeight"). Multi/PolyLine both store on Model
    // via SetModelHeight; we route via dynamic_cast so the right
    // setter fires.
    else if (k == "ModelHeight") {
        double v = asDouble(&ok); if (!ok) return NO;
        if (auto* mp = dynamic_cast<MultiPointModel*>(m)) {
            if (mp->GetModelHeight() != (float)v) { mp->SetModelHeight((float)v); changed = YES; }
        } else if (auto* pl = dynamic_cast<PolyLineModel*>(m)) {
            if (pl->GetModelHeight() != (float)v) { pl->SetModelHeight((float)v); changed = YES; }
        }
    }
    // AlternateNodes is shared between Matrix-derived models and
    // PolyLine. Matrix branch caught above; this is the PolyLine
    // fallback.
    else if (k == "PolyLineAlternateNodes") {
        BOOL v = asBool(&ok); if (!ok) return NO;
        auto* pl = dynamic_cast<PolyLineModel*>(m);
        if (pl && pl->HasAlternateNodes() != (v?true:false)) {
            pl->SetAlternateNodes(v?true:false); changed = YES;
        }
    }
    // J-20 — Label
    else if (k == "LabelText") {
        std::string v = asString(&ok); if (!ok) return NO;
        auto* lm = dynamic_cast<LabelModel*>(m);
        if (lm && v != lm->GetLabelText()) { lm->SetLabelText(v); changed = YES; }
    } else if (k == "LabelFontSize") {
        int v = asInt(&ok); if (!ok) return NO;
        auto* lm = dynamic_cast<LabelModel*>(m);
        if (lm && lm->GetLabelFontSize() != v) { lm->SetLabelFontSize(v); changed = YES; }
    } else if (k == "LabelTextColor") {
        std::string v = asString(&ok); if (!ok) return NO;
        auto* lm = dynamic_cast<LabelModel*>(m);
        if (lm) {
            xlColor xc(v);
            if (xc != lm->GetLabelTextColor()) { lm->SetLabelTextColor(xc); changed = YES; }
        }
    }
    // J-3 (DMX) — DMX-model knobs. All keys share the
    // `DmxModel`-as-base dispatch: cast to the relevant ability
    // subobject and update its scalar. Floodlight is the only
    // fixture wired to `BuildXxxProps` today, but the setter is
    // shared so it auto-handles MovingHead / FloodArea / etc.
    // once their builders land.
    else if (k == "DmxChannelCount") {
        int v = asInt(&ok); if (!ok) return NO;
        auto* dmx = dynamic_cast<DmxModel*>(m);
        if (dmx && dmx->GetDmxChannelCount() != v) {
            dmx->UpdateChannelCount(v, /* do_work */ true);
            changed = YES;
        }
    } else if (k == "DmxRedChannel" || k == "DmxGreenChannel"
               || k == "DmxBlueChannel" || k == "DmxWhiteChannel"
               || k == "DmxCyanChannel" || k == "DmxMagentaChannel"
               || k == "DmxYellowChannel") {
        int v = asInt(&ok); if (!ok) return NO;
        auto* dmx = dynamic_cast<DmxModel*>(m);
        if (!dmx || !dmx->HasColorAbility()) return NO;
        using CT = DmxColorAbility::DMX_COLOR_TYPE;
        auto* color = dmx->GetColorAbility();
        if (color->GetColorType() == CT::DMX_COLOR_RGBW) {
            auto* rgb = static_cast<DmxColorAbilityRGB*>(color);
            if (k == "DmxRedChannel"   && (int)rgb->GetRedChannel()   != v) {
                rgb->SetRedChannel((uint32_t)v); changed = YES;
            } else if (k == "DmxGreenChannel" && (int)rgb->GetGreenChannel() != v) {
                rgb->SetGreenChannel((uint32_t)v); changed = YES;
            } else if (k == "DmxBlueChannel"  && (int)rgb->GetBlueChannel()  != v) {
                rgb->SetBlueChannel((uint32_t)v); changed = YES;
            } else if (k == "DmxWhiteChannel" && (int)rgb->GetWhiteChannel() != v) {
                rgb->SetWhiteChannel((uint32_t)v); changed = YES;
            }
        } else if (color->GetColorType() == CT::DMX_COLOR_CMYW) {
            auto* cmy = static_cast<DmxColorAbilityCMY*>(color);
            if (k == "DmxCyanChannel"    && (int)cmy->GetCyanChannel()    != v) {
                cmy->SetCyanChannel(v); changed = YES;
            } else if (k == "DmxMagentaChannel" && (int)cmy->GetMagentaChannel() != v) {
                cmy->SetMagentaChannel(v); changed = YES;
            } else if (k == "DmxYellowChannel"  && (int)cmy->GetYellowChannel()  != v) {
                cmy->SetYellowChannel(v); changed = YES;
            } else if (k == "DmxWhiteChannel"   && (int)cmy->GetWhiteChannel()   != v) {
                cmy->SetWhiteChannel(v); changed = YES;
            }
        }
        // Wheel doesn't share those keys — its setters live below.
    } else if (k == "DmxWheelChannel" || k == "DmxWheelDimmerChannel"
               || k == "DmxWheelDelay") {
        int v = asInt(&ok); if (!ok) return NO;
        auto* dmx = dynamic_cast<DmxModel*>(m);
        if (!dmx || !dmx->HasColorAbility()) return NO;
        if (dmx->GetColorAbility()->GetColorType()
            != DmxColorAbility::DMX_COLOR_TYPE::DMX_COLOR_WHEEL) return NO;
        auto* wh = static_cast<DmxColorAbilityWheel*>(dmx->GetColorAbility());
        if (k == "DmxWheelChannel" && (int)wh->GetWheelChannel() != v) {
            wh->SetWheelChannel((uint32_t)v); changed = YES;
        } else if (k == "DmxWheelDimmerChannel" && (int)wh->GetDimmerChannel() != v) {
            wh->SetDimmerChannel((uint32_t)v); changed = YES;
        } else if (k == "DmxWheelDelay" && (int)wh->GetWheelDelay() != v) {
            wh->SetWheelDelay((uint32_t)v); changed = YES;
        }
    } else if (k == "DmxShutterChannel" || k == "DmxShutterOpen"
               || k == "DmxShutterOnValue") {
        int v = asInt(&ok); if (!ok) return NO;
        auto* dmx = dynamic_cast<DmxModel*>(m);
        if (dmx && dmx->HasShutterAbility()) {
            auto* sh = dmx->GetShutterAbility();
            if (k == "DmxShutterChannel" && sh->GetShutterChannel() != v) {
                sh->SetShutterChannel(v); changed = YES;
            } else if (k == "DmxShutterOpen" && sh->GetShutterThreshold() != v) {
                sh->SetShutterThreshold(v); changed = YES;
            } else if (k == "DmxShutterOnValue" && sh->GetShutterOnValue() != v) {
                sh->SetShutterOnValue(v); changed = YES;
            }
        }
    } else if (k == "DmxBeamLength" || k == "DmxBeamWidth"
               || k == "DmxBeamYOffset") {
        double v = asDouble(&ok); if (!ok) return NO;
        auto* dmx = dynamic_cast<DmxModel*>(m);
        if (dmx && dmx->HasBeamAbility()) {
            auto* b = dmx->GetBeamAbility();
            if (k == "DmxBeamLength"
                && std::fabs((double)b->GetBeamLength() - v) > 1e-4) {
                b->SetBeamLength((float)v); changed = YES;
            } else if (k == "DmxBeamWidth"
                       && std::fabs((double)b->GetBeamWidth() - v) > 1e-4) {
                b->SetBeamWidth((float)v); changed = YES;
            } else if (k == "DmxBeamYOffset"
                       && b->SupportsYOffset()
                       && std::fabs((double)b->GetBeamYOffset() - v) > 1e-4) {
                b->SetBeamYOffset((float)v); changed = YES;
            }
        }
    } else if (k == "DmxBeamOrient") {
        int v = asInt(&ok); if (!ok) return NO;
        auto* dmx = dynamic_cast<DmxModel*>(m);
        if (dmx && dmx->HasBeamAbility()) {
            auto* b = dmx->GetBeamAbility();
            if (b->SupportsOrient() && b->GetBeamOrient() != v) {
                b->SetBeamOrient(v); changed = YES;
            }
        }
    } else if (k == "MhDimmerChannel") {
        int v = asInt(&ok); if (!ok) return NO;
        auto* dmx = dynamic_cast<DmxModel*>(m);
        if (dmx && dmx->HasDimmerAbility()
            && dmx->GetDimmerAbility()->GetDimmerChannel() != v) {
            dmx->GetDimmerAbility()->SetDimmerChannel(v); changed = YES;
        }
    } else if (k == "DmxColorType") {
        int v = asInt(&ok); if (!ok) return NO;
        // Color type maps to `DmxModel::InitColorAbility(int type)`,
        // where type follows the same 0..3 ordering desktop uses
        // (RGBW / Wheel / CMYW / Unused — matches
        // `DMX_COLOR_TYPES_VALUES`).
        auto* dmx = dynamic_cast<DmxModel*>(m);
        if (dmx && DmxColorTypeIndexFor(dmx) != v) {
            dmx->InitColorAbility(v); changed = YES;
        }
    }
    // J-3 (DMX) — MovingHead-specific knobs.
    else if (k == "DmxStyle") {
        int v = asInt(&ok); if (!ok) return NO;
        auto* mh = dynamic_cast<DmxMovingHead*>(m);
        if (mh && mh->GetDmxStyleVal() != v) {
            mh->SetDmxStyleVal(v);
            mh->SetDmxStyle(std::string(
                DmxMovingHeadStyleNameForIndex(v).UTF8String));
            changed = YES;
        }
    } else if (k == "DmxFixture") {
        int v = asInt(&ok); if (!ok) return NO;
        // Shared between DmxMovingHead and DmxMovingHeadAdv via the
        // common `DmxMovingHeadComm` base.
        auto* mhc = dynamic_cast<DmxMovingHeadComm*>(m);
        if (mhc && (mhc->GetFixtureVal() - 1) != v) {
            mhc->SetFixtureVal(v);
            mhc->SetDmxFixture(DmxMovingHeadComm::FixtureIDtoString(v));
            changed = YES;
        }
    } else if (k == "HideBody") {
        BOOL v = asBool(&ok); if (!ok) return NO;
        auto* mh = dynamic_cast<DmxMovingHead*>(m);
        if (mh && mh->GetHideBody() != (bool)v) {
            mh->SetHideBody(v); changed = YES;
        }
    }
    // J-3 (DMX) — DmxSkull-specific knobs.
    else if (k == "SkullIs16Bit") {
        BOOL v = asBool(&ok); if (!ok) return NO;
        auto* sk = dynamic_cast<DmxSkull*>(m);
        if (sk && sk->Is16Bit() != (bool)v) {
            sk->SetIs16Bit(v); changed = YES;
        }
    } else if (k == "SkullHasJaw" || k == "SkullHasPan"
               || k == "SkullHasTilt" || k == "SkullHasNod"
               || k == "SkullHasEyeUD" || k == "SkullHasEyeLR"
               || k == "SkullHasColor") {
        BOOL v = asBool(&ok); if (!ok) return NO;
        auto* sk = dynamic_cast<DmxSkull*>(m);
        if (!sk) return NO;
        if (k == "SkullHasJaw"   && sk->HasJaw()   != (bool)v) {
            sk->SetHasJaw(v);   changed = YES;
        } else if (k == "SkullHasPan"   && sk->HasPan()   != (bool)v) {
            sk->SetHasPan(v);   changed = YES;
        } else if (k == "SkullHasTilt"  && sk->HasTilt()  != (bool)v) {
            sk->SetHasTilt(v);  changed = YES;
        } else if (k == "SkullHasNod"   && sk->HasNod()   != (bool)v) {
            sk->SetHasNod(v);   changed = YES;
        } else if (k == "SkullHasEyeUD" && sk->HasEyeUD() != (bool)v) {
            sk->SetHasEyeUD(v); changed = YES;
        } else if (k == "SkullHasEyeLR" && sk->HasEyeLR() != (bool)v) {
            sk->SetHasEyeLR(v); changed = YES;
        } else if (k == "SkullHasColor" && sk->HasColor() != (bool)v) {
            sk->SetHasColor(v); changed = YES;
        }
    } else if (k == "SkullApplySkulltronix") {
        // One-shot: the bool payload is ignored — taps always
        // apply the preset, which sets the `setup_skulltronix`
        // flag so `InitModel()` (re)assigns the canonical
        // Skulltronix channel layout on the next reinitialise.
        auto* sk = dynamic_cast<DmxSkull*>(m);
        if (sk) {
            sk->SetSkulltronix();
            changed = YES;
        }
    } else if (k == "SkullEyeBrightnessChannel") {
        int v = asInt(&ok); if (!ok) return NO;
        auto* sk = dynamic_cast<DmxSkull*>(m);
        if (sk && sk->GetEyeBrightnessChannel() != v) {
            sk->SetEyeBrightnessChannel(v); changed = YES;
        }
    } else if (k.find("Skull") == 0
               && (k.find("Channel") != std::string::npos
                   || k.find("MinLimit") != std::string::npos
                   || k.find("MaxLimit") != std::string::npos
                   || k.find("Orient") != std::string::npos)) {
        auto* sk = dynamic_cast<DmxSkull*>(m);
        if (!sk) return NO;
        // Decode `Skull<servo><suffix>`. Servo name is whatever
        // sits between the `Skull` prefix and the trailing
        // `Channel`/`MinLimit`/`MaxLimit`/`Orient`. Keeps the
        // per-servo cases compact instead of N × 4 individual
        // string compares.
        Servo* servo = nullptr;
        int* orient = nullptr;
        std::string servoName;
        if      (k.find("SkullJaw")   == 0) { servo = sk->GetJawServo();
                                              servoName = "Jaw"; }
        else if (k.find("SkullPan")   == 0) { servo = sk->GetPanServo();
                                              servoName = "Pan"; }
        else if (k.find("SkullTilt")  == 0) { servo = sk->GetTiltServo();
                                              servoName = "Tilt"; }
        else if (k.find("SkullNod")   == 0) { servo = sk->GetNodServo();
                                              servoName = "Nod"; }
        else if (k.find("SkullEyeUD") == 0) { servo = sk->GetEyeUDServo();
                                              servoName = "EyeUD"; }
        else if (k.find("SkullEyeLR") == 0) { servo = sk->GetEyeLRServo();
                                              servoName = "EyeLR"; }
        else return NO;
        const std::string suffix = k.substr(5 + servoName.size());
        if (suffix == "Channel") {
            int v = asInt(&ok); if (!ok) return NO;
            if (servo && servo->GetChannel() != v) {
                servo->SetChannel(v); changed = YES;
            }
        } else if (suffix == "MinLimit") {
            int v = asInt(&ok); if (!ok) return NO;
            if (servo && servo->GetMinLimit() != v) {
                servo->SetMinLimit(v); changed = YES;
            }
        } else if (suffix == "MaxLimit") {
            int v = asInt(&ok); if (!ok) return NO;
            if (servo && servo->GetMaxLimit() != v) {
                servo->SetMaxLimit(v); changed = YES;
            }
        } else if (suffix == "Orient") {
            int v = asInt(&ok); if (!ok) return NO;
            int cur = 0;
            if      (servoName == "Jaw")   cur = sk->GetJawOrient();
            else if (servoName == "Pan")   cur = sk->GetPanOrient();
            else if (servoName == "Tilt")  cur = sk->GetTiltOrient();
            else if (servoName == "Nod")   cur = sk->GetNodOrient();
            else if (servoName == "EyeUD") cur = sk->GetEyeUDOrient();
            else if (servoName == "EyeLR") cur = sk->GetEyeLROrient();
            if (cur != v) {
                if      (servoName == "Jaw")   sk->SetJawOrient(v);
                else if (servoName == "Pan")   sk->SetPanOrient(v);
                else if (servoName == "Tilt")  sk->SetTiltOrient(v);
                else if (servoName == "Nod")   sk->SetNodOrient(v);
                else if (servoName == "EyeUD") sk->SetEyeUDOrient(v);
                else if (servoName == "EyeLR") sk->SetEyeLROrient(v);
                changed = YES;
            }
        } else {
            return NO;
        }
        (void)orient;  // silence unused-var when default
    }
    // J-30 — wholesale-replace setters for the DMX list editors.
    // Sheet emits an NSArray of NSDictionary entries; the
    // setter clears the existing list and rebuilds it.
    else if (k == "DmxPresetList") {
        NSArray* arr = ([value isKindOfClass:[NSArray class]] ? (NSArray*)value : nil);
        auto* dmx = dynamic_cast<DmxModel*>(m);
        if (!arr || !dmx || !dmx->HasPresetAbility()) return NO;
        auto* preset = dmx->GetPresetAbility();
        while (preset->GetPresetsCount() > 0) preset->PopPreset();
        for (id obj in arr) {
            if (![obj isKindOfClass:[NSDictionary class]]) continue;
            NSDictionary* d = obj;
            uint8_t ch  = (uint8_t)[(NSNumber*)d[@"channel"] intValue];
            uint8_t val = (uint8_t)[(NSNumber*)d[@"value"]   intValue];
            NSString* descNS = (NSString*)d[@"description"] ?: @"";
            std::string desc = std::string([descNS UTF8String]);
            preset->AddPreset(ch, val, desc);
        }
        changed = YES;
    } else if (k == "DmxPositionZoneList") {
        NSArray* arr = ([value isKindOfClass:[NSArray class]] ? (NSArray*)value : nil);
        auto* mha = dynamic_cast<DmxMovingHeadAdv*>(m);
        if (!arr || !mha) return NO;
        std::vector<PositionZone> zones;
        zones.reserve(arr.count);
        for (id obj in arr) {
            if (![obj isKindOfClass:[NSDictionary class]]) continue;
            NSDictionary* d = obj;
            PositionZone z;
            z.pan_min  = [(NSNumber*)d[@"panMin"]  intValue];
            z.pan_max  = [(NSNumber*)d[@"panMax"]  intValue];
            z.tilt_min = [(NSNumber*)d[@"tiltMin"] intValue];
            z.tilt_max = [(NSNumber*)d[@"tiltMax"] intValue];
            z.channel  = [(NSNumber*)d[@"channel"] intValue];
            z.value    = (uint8_t)[(NSNumber*)d[@"value"] intValue];
            zones.push_back(z);
        }
        mha->SetPositionZones(zones);
        changed = YES;
    } else if (k == "DmxWheelColorList") {
        NSArray* arr = ([value isKindOfClass:[NSArray class]] ? (NSArray*)value : nil);
        auto* dmx = dynamic_cast<DmxModel*>(m);
        if (!arr || !dmx || !dmx->HasColorAbility()) return NO;
        if (dmx->GetColorAbility()->GetColorType()
            != DmxColorAbility::DMX_COLOR_TYPE::DMX_COLOR_WHEEL) return NO;
        auto* wh = static_cast<DmxColorAbilityWheel*>(dmx->GetColorAbility());
        wh->ClearColors();
        for (id obj in arr) {
            if (![obj isKindOfClass:[NSDictionary class]]) continue;
            NSDictionary* d = obj;
            NSString* hex = (NSString*)d[@"color"] ?: @"#FFFFFF";
            std::string hexCpp = [hex UTF8String];
            xlColor col(hexCpp);
            uint8_t v = (uint8_t)[(NSNumber*)d[@"dmxValue"] intValue];
            wh->AddWheelColor(col, v);
        }
        changed = YES;
    }
    // J-30 — file-path setters for DMX mesh / image fields.
    // Picker UI emits the picked path through commitPerType, and
    // the matching `Set*File` + `Notify*FileChanged()` calls
    // invalidate the cached mesh / image so the canvas redraws
    // with the new asset.
    else if (k == "AdvBaseMeshFile" || k == "AdvYokeMeshFile" || k == "AdvHeadMeshFile") {
        std::string v = asString(&ok); if (!ok) return NO;
        auto* mha = dynamic_cast<DmxMovingHeadAdv*>(m);
        if (!mha) return NO;
        Mesh* mesh = (k == "AdvBaseMeshFile") ? mha->GetBaseMesh()
                   : (k == "AdvYokeMeshFile") ? mha->GetYokeMesh()
                                              : mha->GetHeadMesh();
        if (mesh && mesh->GetObjFile() != v) {
            mesh->SetObjFile(v);
            mesh->NotifyObjFileChanged();
            changed = YES;
        }
    } else if (k == "SkullHeadMeshFile" || k == "SkullJawMeshFile"
               || k == "SkullEyeLMeshFile" || k == "SkullEyeRMeshFile") {
        std::string v = asString(&ok); if (!ok) return NO;
        auto* sk = dynamic_cast<DmxSkull*>(m);
        if (!sk) return NO;
        Mesh* mesh = (k == "SkullHeadMeshFile") ? sk->GetHeadMesh()
                   : (k == "SkullJawMeshFile")  ? sk->GetJawMesh()
                   : (k == "SkullEyeLMeshFile") ? sk->GetEyeLMesh()
                                                : sk->GetEyeRMesh();
        if (mesh && mesh->GetObjFile() != v) {
            mesh->SetObjFile(v);
            mesh->NotifyObjFileChanged();
            changed = YES;
        }
    } else if (k.length() > 5 && k.substr(0, 5) == "Servo"
               && std::isdigit((unsigned char)k[5])
               && (k.find("StaticImage") != std::string::npos
                   || k.find("MotionImage") != std::string::npos)) {
        std::string v = asString(&ok); if (!ok) return NO;
        auto* ds = dynamic_cast<DmxServo*>(m);
        if (!ds) return NO;
        size_t pos = 5;
        while (pos < k.length() && std::isdigit((unsigned char)k[pos])) ++pos;
        const int idx = std::atoi(k.substr(5, pos - 5).c_str());
        if (idx < 0 || idx >= ds->GetNumServos()) return NO;
        const std::string suffix = k.substr(pos);
        DmxImage* img = (suffix == "StaticImage") ? ds->GetStaticImage(idx)
                                                   : ds->GetMotionImage(idx);
        if (img && img->GetImageFile() != v) {
            img->SetImageFile(v);
            img->NotifyImageFileChanged();
            changed = YES;
        }
    } else if (k.length() > 12 && k.substr(0, 12) == "Servo3dStatic"
               && k.find("MeshFile") != std::string::npos) {
        // "Servo3dStatic" + index + "MeshFile"
        std::string v = asString(&ok); if (!ok) return NO;
        auto* ds3 = dynamic_cast<DmxServo3d*>(m);
        if (!ds3) return NO;
        const size_t prefixLen = std::string("Servo3dStatic").length();
        size_t pos = prefixLen;
        while (pos < k.length() && std::isdigit((unsigned char)k[pos])) ++pos;
        const int idx = std::atoi(k.substr(prefixLen, pos - prefixLen).c_str());
        if (idx < 0 || idx >= ds3->GetNumStatic()) return NO;
        Mesh* mesh = ds3->GetStaticMesh(idx);
        if (mesh && mesh->GetObjFile() != v) {
            mesh->SetObjFile(v);
            mesh->NotifyObjFileChanged();
            changed = YES;
        }
    } else if (k.length() > 12 && k.substr(0, 13) == "Servo3dMotion"
               && k.find("MeshFile") != std::string::npos) {
        std::string v = asString(&ok); if (!ok) return NO;
        auto* ds3 = dynamic_cast<DmxServo3d*>(m);
        if (!ds3) return NO;
        const size_t prefixLen = std::string("Servo3dMotion").length();
        size_t pos = prefixLen;
        while (pos < k.length() && std::isdigit((unsigned char)k[pos])) ++pos;
        const int idx = std::atoi(k.substr(prefixLen, pos - prefixLen).c_str());
        if (idx < 0 || idx >= ds3->GetNumMotion()) return NO;
        Mesh* mesh = ds3->GetMotionMesh(idx);
        if (mesh && mesh->GetObjFile() != v) {
            mesh->SetObjFile(v);
            mesh->NotifyObjFileChanged();
            changed = YES;
        }
    }
    // J-3 (DMX) — DmxServo / DmxServo3d top-level knobs. Per-
    // servo settings come through the `Servo<N>Xxx` decoder
    // further down.
    else if (k == "ServoNumServos") {
        int v = asInt(&ok); if (!ok) return NO;
        if (auto* ds = dynamic_cast<DmxServo*>(m); ds && ds->GetNumServos() != v) {
            ds->SetNumServos(v); changed = YES;
        } else if (auto* ds3 = dynamic_cast<DmxServo3d*>(m); ds3 && ds3->GetNumServos() != v) {
            ds3->SetNumServos(v); changed = YES;
        }
    } else if (k == "Servo3dNumStatic") {
        int v = asInt(&ok); if (!ok) return NO;
        auto* ds3 = dynamic_cast<DmxServo3d*>(m);
        if (ds3 && ds3->GetNumStatic() != v) { ds3->SetNumStatic(v); changed = YES; }
    } else if (k == "Servo3dNumMotion") {
        int v = asInt(&ok); if (!ok) return NO;
        auto* ds3 = dynamic_cast<DmxServo3d*>(m);
        if (ds3 && ds3->GetNumMotion() != v) { ds3->SetNumMotion(v); changed = YES; }
    } else if (k.length() > 12 && k.substr(0, 12) == "Servo3dServo"
               && k.find("Link") != std::string::npos) {
        // "Servo3dServo<idx>Link"
        int v = asInt(&ok); if (!ok) return NO;
        auto* ds3 = dynamic_cast<DmxServo3d*>(m);
        if (!ds3) return NO;
        size_t pos = 12;
        while (pos < k.length() && std::isdigit((unsigned char)k[pos])) ++pos;
        const int idx = std::atoi(k.substr(12, pos - 12).c_str());
        if (idx < 0 || idx >= ds3->GetNumServos()) return NO;
        if (ds3->GetServoLink(idx) != v) {
            ds3->SetServoLink(idx, v); changed = YES;
        }
    } else if (k.length() > 11 && k.substr(0, 11) == "Servo3dMesh"
               && k.find("Link") != std::string::npos) {
        // "Servo3dMesh<idx>Link"
        int v = asInt(&ok); if (!ok) return NO;
        auto* ds3 = dynamic_cast<DmxServo3d*>(m);
        if (!ds3) return NO;
        size_t pos = 11;
        while (pos < k.length() && std::isdigit((unsigned char)k[pos])) ++pos;
        const int idx = std::atoi(k.substr(11, pos - 11).c_str());
        if (idx < 0 || idx >= ds3->GetNumServos()) return NO;
        if (ds3->GetMeshLink(idx) != v) {
            ds3->SetMeshLink(idx, v); changed = YES;
        }
    } else if (k == "Servo3dShowPivot") {
        BOOL v = asBool(&ok); if (!ok) return NO;
        auto* ds3 = dynamic_cast<DmxServo3d*>(m);
        if (ds3 && ds3->GetShowPivot() != (bool)v) {
            ds3->SetShowPivot(v); changed = YES;
        }
    } else if (k == "ServoIs16Bit") {
        BOOL v = asBool(&ok); if (!ok) return NO;
        if (auto* ds = dynamic_cast<DmxServo*>(m); ds && ds->Is16Bit() != (bool)v) {
            ds->SetIs16Bit(v); changed = YES;
        } else if (auto* ds3 = dynamic_cast<DmxServo3d*>(m); ds3 && ds3->Is16Bit() != (bool)v) {
            ds3->SetIs16Bit(v); changed = YES;
        }
    } else if (k == "ServoBrightness") {
        double v = asDouble(&ok); if (!ok) return NO;
        if (auto* ds = dynamic_cast<DmxServo*>(m); ds
            && std::fabs((double)ds->GetBrightness() - v) > 1e-3) {
            ds->SetBrightness((float)v); changed = YES;
        } else if (auto* ds3 = dynamic_cast<DmxServo3d*>(m); ds3
                    && std::fabs((double)ds3->GetBrightness() - v) > 1e-3) {
            ds3->SetBrightness((float)v); changed = YES;
        }
    } else if (k == "ServoTransparency") {
        int v = asInt(&ok); if (!ok) return NO;
        auto* ds = dynamic_cast<DmxServo*>(m);
        if (ds && ds->GetTransparency() != v) {
            ds->SetTransparency(v); changed = YES;
        }
    }
    // Per-servo knobs — `Servo<N>Channel`, `Servo<N>MinLimit`,
    // `Servo<N>MaxLimit`, `Servo<N>RangeOfMotion`,
    // `Servo<N>Style`, `Servo<N>Is16Bit`,
    // `Servo<N>ControllerMin`, `Servo<N>ControllerMax`,
    // `Servo<N>ControllerReverse`. N is parsed from after the
    // `Servo` prefix; the suffix dispatches to the right setter.
    else if (k.length() > 5 && k.substr(0, 5) == "Servo"
             && std::isdigit((unsigned char)k[5])) {
        size_t pos = 5;
        while (pos < k.length() && std::isdigit((unsigned char)k[pos])) ++pos;
        const int idx = std::atoi(k.substr(5, pos - 5).c_str());
        const std::string suffix = k.substr(pos);
        Servo* servo = nullptr;
        if (auto* ds = dynamic_cast<DmxServo*>(m); ds && idx < ds->GetNumServos()) {
            servo = ds->GetServo(idx);
        } else if (auto* ds3 = dynamic_cast<DmxServo3d*>(m); ds3 && idx < ds3->GetNumServos()) {
            servo = ds3->GetServo(idx);
        }
        if (!servo) return NO;
        if (suffix == "Channel") {
            int v = asInt(&ok); if (!ok) return NO;
            if (servo->GetChannel() != v) { servo->SetChannel(v); changed = YES; }
        } else if (suffix == "MinLimit") {
            int v = asInt(&ok); if (!ok) return NO;
            if (servo->GetMinLimit() != v) { servo->SetMinLimit(v); changed = YES; }
        } else if (suffix == "MaxLimit") {
            int v = asInt(&ok); if (!ok) return NO;
            if (servo->GetMaxLimit() != v) { servo->SetMaxLimit(v); changed = YES; }
        } else if (suffix == "RangeOfMotion") {
            double v = asDouble(&ok); if (!ok) return NO;
            if (std::fabs((double)servo->GetRangeOfMotion() - v) > 1e-3) {
                servo->SetRangeOfMotion((float)v); changed = YES;
            }
        } else if (suffix == "Style") {
            int v = asInt(&ok); if (!ok) return NO;
            NSArray* opts = DmxServoStyleOptions();
            if (v >= 0 && v < (int)opts.count) {
                std::string newStyle = [(NSString*)opts[v] UTF8String];
                if (servo->GetStyle() != newStyle) {
                    servo->SetStyle(newStyle); changed = YES;
                }
            }
        } else if (suffix == "Is16Bit") {
            BOOL v = asBool(&ok); if (!ok) return NO;
            if (servo->Is16Bit() != (bool)v) {
                servo->Set16Bit(v); changed = YES;
            }
        } else if (suffix == "ControllerMin") {
            int v = asInt(&ok); if (!ok) return NO;
            if (servo->GetControllerMin() != v) {
                servo->SetControllerMin(v); changed = YES;
            }
        } else if (suffix == "ControllerMax") {
            int v = asInt(&ok); if (!ok) return NO;
            if (servo->GetControllerMax() != v) {
                servo->SetControllerMax(v); changed = YES;
            }
        } else if (suffix == "ControllerReverse") {
            BOOL v = asBool(&ok); if (!ok) return NO;
            if (servo->GetControllerReverse() != (bool)v) {
                servo->SetControllerReverse(v); changed = YES;
            }
        } else {
            return NO;
        }
    }
    // Motor knobs — keyed by motor base_name + suffix.
    // `PanMotorChannelCoarse`, `TiltMotorMinLimit`, …
    //
    // Both DmxMovingHead and DmxMovingHeadAdv expose `GetPanMotor`
    // / `GetTiltMotor` via the shared `DmxMovingHeadComm` base, so
    // the cast goes through that — not the concrete subclass.
    else if (k.find("PanMotor") == 0 || k.find("TiltMotor") == 0) {
        auto* mhc = dynamic_cast<DmxMovingHeadComm*>(m);
        if (!mhc) {
            return NO;
        }
        DmxMotor* motor = (k.find("PanMotor") == 0)
            ? mhc->GetPanMotor() : mhc->GetTiltMotor();
        if (!motor) return NO;
        const std::string base = (k.find("PanMotor") == 0)
            ? "PanMotor" : "TiltMotor";
        const std::string suffix = k.substr(base.size());
        if (suffix == "ChannelCoarse") {
            int v = asInt(&ok); if (!ok) return NO;
            if (motor->GetChannelCoarse() != v) { motor->SetChannelCoarse(v); changed = YES; }
        } else if (suffix == "ChannelFine") {
            int v = asInt(&ok); if (!ok) return NO;
            if (motor->GetChannelFine() != v) { motor->SetChannelFine(v); changed = YES; }
        } else if (suffix == "MinLimit") {
            int v = asInt(&ok); if (!ok) return NO;
            if (motor->GetMinLimit() != v) { motor->SetMinLimit(v); changed = YES; }
        } else if (suffix == "MaxLimit") {
            int v = asInt(&ok); if (!ok) return NO;
            if (motor->GetMaxLimit() != v) { motor->SetMaxLimit(v); changed = YES; }
        } else if (suffix == "RangeOfMotion") {
            double v = asDouble(&ok); if (!ok) return NO;
            if (std::fabs((double)motor->GetRangeOfMotion() - v) > 1e-3) {
                motor->SetRangeOfMOtion((float)v); changed = YES;
            }
        } else if (suffix == "OrientZero") {
            int v = asInt(&ok); if (!ok) return NO;
            if (motor->GetOrientZero() != v) { motor->SetOrientZero(v); changed = YES; }
        } else if (suffix == "OrientHome") {
            int v = asInt(&ok); if (!ok) return NO;
            if (motor->GetOrientHome() != v) { motor->SetOrientHome(v); changed = YES; }
        } else if (suffix == "SlewLimit") {
            double v = asDouble(&ok); if (!ok) return NO;
            if (std::fabs((double)motor->GetSlewLimit() - v) > 1e-3) {
                motor->SetSlewLimit((float)v); changed = YES;
            }
        } else if (suffix == "Reverse") {
            BOOL v = asBool(&ok); if (!ok) return NO;
            if (motor->GetReverse() != (bool)v) { motor->SetReverse(v); changed = YES; }
        } else if (suffix == "UpsideDown") {
            BOOL v = asBool(&ok); if (!ok) return NO;
            if (motor->GetUpsideDown() != (bool)v) { motor->SetUpsideDown(v); changed = YES; }
        } else {
            spdlog::warn("setPerTypeProperty: unknown motor key '{}'", k);
            return NO;
        }
    }
    else {
        spdlog::warn("setPerTypeProperty: unknown key '{}' for model '{}' (type {})",
                     k, modelName.UTF8String, m->GetDisplayAsString());
        return NO;
    }

    if (changed) {
        m->IncrementChangeCount();
        m->Reinitialize();
        _context->MarkLayoutModelDirty(modelName.UTF8String);
    }
    return changed;
}

- (BOOL)hasUnsavedLayoutChanges {
    if (!_context) return NO;
    return _context->HasDirtyLayoutModels() ? YES : NO;
}

- (void)clearDirtyLayoutChanges {
    if (_context) _context->ClearDirtyLayoutModels();
}

- (void)pushLayoutUndoSnapshotForModel:(NSString*)modelName {
    if (!_context || !modelName) return;
    _context->PushLayoutUndoSnapshotForModel(std::string([modelName UTF8String]));
}

- (void)pushLayoutUndoSnapshotForViewObject:(NSString*)objectName {
    if (!_context || !objectName) return;
    if ([objectName isEqualToString:kBackgroundPseudoObjectName]) return;
    _context->PushLayoutUndoSnapshotForViewObject(std::string([objectName UTF8String]));
}

- (void)pushTerrainHeightmapUndoSnapshot:(NSString*)terrainName {
    if (!_context || !terrainName) return;
    _context->PushTerrainHeightmapUndoSnapshot(std::string([terrainName UTF8String]));
}

- (BOOL)undoLastLayoutChange {
    if (!_context) return NO;
    _context->AbortRender(5000);
    return _context->UndoLastLayoutChange() ? YES : NO;
}

- (BOOL)canUndoLayoutChange {
    if (!_context) return NO;
    return _context->CanUndoLayoutChange() ? YES : NO;
}

- (NSDictionary<NSString*, id>*)layoutDisplayState {
    if (!_context) {
        return @{
            @"backgroundImage":      @"",
            @"backgroundBrightness": @100,
            @"backgroundAlpha":      @100,
            @"scaleBackgroundImage": @NO,
            @"display2DGrid":        @NO,
            @"display2DGridSpacing": @100,
            @"display2DBoundingBox": @NO,
            @"display2DCenter0":     @NO,
            @"previewWidth":         @1280,
            @"previewHeight":        @720,
            @"layoutMode3D":         @YES,
        };
    }
    NSString* bg = [NSString stringWithUTF8String:_context->GetActiveBackgroundImage().c_str()];
    return @{
        @"backgroundImage":      bg,
        @"backgroundBrightness": @(_context->GetActiveBackgroundBrightness()),
        @"backgroundAlpha":      @(_context->GetActiveBackgroundAlpha()),
        @"scaleBackgroundImage": @(_context->GetActiveScaleBackgroundImage() ? YES : NO),
        @"display2DGrid":        @(_context->GetDisplay2DGrid() ? YES : NO),
        @"display2DGridSpacing": @((long)_context->GetDisplay2DGridSpacing()),
        @"display2DBoundingBox": @(_context->GetDisplay2DBoundingBox() ? YES : NO),
        @"display2DCenter0":     @(_context->GetDisplay2DCenter0() ? YES : NO),
        @"previewWidth":         @(_context->GetPreviewWidth()),
        @"previewHeight":        @(_context->GetPreviewHeight()),
        @"layoutMode3D":         @(_context->GetLayoutMode3D() ? YES : NO),
    };
}

// MARK: - Effect Editing

- (BOOL)addEffectToRow:(int)rowIndex
                  name:(NSString*)effectName
               startMS:(int)startMS
                 endMS:(int)endMS {
    auto* layer = [self effectLayerForRow:rowIndex];
    if (!layer) return NO;

    std::string name = [effectName UTF8String];
    Effect* e = layer->AddEffect(0, name, "", "", startMS, endMS, 0, false);
    return e != nullptr;
}

- (BOOL)deleteEffectInRow:(int)rowIndex atIndex:(int)effectIndex {
    auto* layer = [self effectLayerForRow:rowIndex];
    if (!layer || effectIndex < 0 || effectIndex >= layer->GetEffectCount()) return NO;

    layer->DeleteEffectByIndex(effectIndex);
    return YES;
}

- (BOOL)moveEffectInRow:(int)rowIndex
                atIndex:(int)effectIndex
              toStartMS:(int)newStartMS
                toEndMS:(int)newEndMS {
    auto* layer = [self effectLayerForRow:rowIndex];
    if (!layer || effectIndex < 0 || effectIndex >= layer->GetEffectCount()) return NO;

    Effect* e = layer->GetEffect(effectIndex);
    if (!e) return NO;

    if (newStartMS < 0) newStartMS = 0;
    if (newEndMS <= newStartMS) return NO;

    // Reject overlap with any other effect on the same layer. Effects
    // are stored in start-time order so neighbour check is enough, but
    // we walk the full layer to also defend against stale indexes.
    for (int i = 0; i < layer->GetEffectCount(); i++) {
        if (i == effectIndex) continue;
        Effect* other = layer->GetEffect(i);
        if (!other) continue;
        int os = other->GetStartTimeMS();
        int oe = other->GetEndTimeMS();
        // Overlap: ranges [newStart,newEnd] and [os,oe] intersect iff
        // newStart < oe AND newEnd > os.
        if (newStartMS < oe && newEndMS > os) {
            return NO;
        }
    }

    e->SetStartTimeMS(newStartMS);
    e->SetEndTimeMS(newEndMS);
    return YES;
}

- (NSArray<NSString*>*)availableEffectNames {
    auto& em = _context->GetEffectManager();
    NSMutableArray* names = [NSMutableArray array];
    for (size_t i = 0; i < em.size(); i++) {
        const std::string& name = em.GetEffectName((int)i);
        if (!name.empty()) {
            [names addObject:[NSString stringWithUTF8String:name.c_str()]];
        }
    }
    return names;
}

// MARK: - dynamicOptions sources

- (NSArray<NSString*>*)_timingTrackNamesWithLayerCount:(int)wantLayerCount
                                       acceptLessThan:(BOOL)acceptLessThan {
    // Mirrors JsonEffectPanel::RepopulateTimingTrackChoices. If
    // acceptLessThan is YES the filter is `layers <= wantLayerCount`
    // (the desktop "not lyric" path); otherwise exact match.
    auto& se = _context->GetSequenceElements();
    NSMutableArray<NSString*>* out = [NSMutableArray array];
    for (int i = 0; i < se.GetElementCount(); i++) {
        Element* el = se.GetElement(i);
        if (!el || el->GetType() != ElementType::ELEMENT_TYPE_TIMING) continue;
        int layers = (int)el->GetEffectLayerCount();
        bool match = acceptLessThan ? (layers <= wantLayerCount)
                                    : (layers == wantLayerCount);
        if (!match) continue;
        [out addObject:[NSString stringWithUTF8String:el->GetName().c_str()]];
    }
    return out;
}

- (NSArray<NSString*>*)timingTrackNames {
    return [self _timingTrackNamesWithLayerCount:1 acceptLessThan:YES];
}

- (NSArray<NSString*>*)lyricTimingTrackNames {
    return [self _timingTrackNamesWithLayerCount:3 acceptLessThan:NO];
}

- (NSArray<NSString*>*)perPreviewCameraNames {
    NSMutableArray<NSString*>* out = [NSMutableArray arrayWithObject:@"2D"];
    if (!_context) return out;
    auto& vm = _context->GetViewpointMgr();
    for (int i = 0; i < vm.GetNum3DCameras(); ++i) {
        if (auto* cam = vm.GetCamera3D(i)) {
            NSString* name = [NSString stringWithUTF8String:cam->GetName().c_str()];
            if (name.length) [out addObject:name];
        }
    }
    return out;
}

- (NSDictionary<NSString*, NSNumber*>*)colorCurveModeSupportForRow:(int)rowIndex
                                                            atIndex:(int)effectIndex {
    if (!_context) return @{};
    EffectLayer* layer = [self effectLayerForRow:rowIndex];
    if (!layer || effectIndex < 0 || effectIndex >= (int)layer->GetEffectCount()) return @{};
    Effect* effect = layer->GetEffect(effectIndex);
    if (!effect) return @{};
    RenderableEffect* fx = _context->GetEffectManager().GetEffect(effect->GetEffectName());
    if (!fx) return @{};
    const SettingsMap& settings = effect->GetSettings();
    bool linear = fx->SupportsLinearColorCurves(settings);
    bool radial = fx->SupportsRadialColorCurves(settings);
    return @{
        @"linear": @(linear),
        @"radial": @(radial),
    };
}

#pragma mark - Palette save / load / import / export

namespace {

// Strip characters that can't safely live in a filename. Desktop's
// equivalent is `RemoveNonAlphanumeric`; we match that shape so the
// filenames we produce round-trip visually with what desktop writes.
NSString* sanitisePaletteName(NSString* raw) {
    if (raw.length == 0) return @"";
    NSMutableCharacterSet* allowed = [NSMutableCharacterSet alphanumericCharacterSet];
    NSMutableString* out = [NSMutableString string];
    for (NSUInteger i = 0; i < raw.length; i++) {
        unichar c = [raw characterAtIndex:i];
        if ([allowed characterIsMember:c]) {
            [out appendFormat:@"%C", c];
        }
    }
    return out;
}

NSString* autogenPaletteName(NSString* paletteDir) {
    NSFileManager* fm = [NSFileManager defaultManager];
    int i = 1;
    while (i < 1000) {
        NSString* candidate = [NSString stringWithFormat:@"PAL%03d.xpalette", i];
        NSString* full = [paletteDir stringByAppendingPathComponent:candidate];
        if (![fm fileExistsAtPath:full]) {
            return candidate;
        }
        i++;
    }
    return @"PAL999.xpalette";
}

// Strip the trailing filename that desktop appends to each loaded
// palette entry (`<palette-string>,<filename>.xpalette`) so our
// list shows the clean palette itself. We track the filename
// separately.
NSString* trimPaletteStringSuffix(NSString* raw) {
    NSRange lastComma = [raw rangeOfString:@"," options:NSBackwardsSearch];
    if (lastComma.location == NSNotFound) return raw;
    NSString* tail = [raw substringFromIndex:lastComma.location + 1];
    if ([tail.lowercaseString hasSuffix:@".xpalette"]) {
        return [raw substringToIndex:lastComma.location + 1]; // keep trailing comma
    }
    return raw;
}

} // namespace

- (NSArray<NSDictionary<NSString*, NSString*>*>*)savedPalettes {
    NSMutableArray<NSDictionary<NSString*, NSString*>*>* out =
        [NSMutableArray array];
    NSMutableSet<NSString*>* seen = [NSMutableSet set];  // dedupe by palette string

    auto scanDir = ^(NSString* dir, BOOL recurse) {
        NSFileManager* fm = [NSFileManager defaultManager];
        BOOL isDir = NO;
        if (![fm fileExistsAtPath:dir isDirectory:&isDir] || !isDir) return;
        NSArray<NSString*>* entries = [fm contentsOfDirectoryAtPath:dir error:nil];
        for (NSString* name in entries) {
            NSString* full = [dir stringByAppendingPathComponent:name];
            BOOL sub = NO;
            [fm fileExistsAtPath:full isDirectory:&sub];
            if (sub) {
                if (recurse) {
                    NSArray* subEntries = [fm contentsOfDirectoryAtPath:full error:nil];
                    for (NSString* subName in subEntries) {
                        if (![subName.lowercaseString hasSuffix:@".xpalette"]) continue;
                        NSString* p = [full stringByAppendingPathComponent:subName];
                        NSString* content = [NSString stringWithContentsOfFile:p
                                                                      encoding:NSUTF8StringEncoding
                                                                         error:nil];
                        if (content.length == 0) continue;
                        NSString* firstLine = [[content componentsSeparatedByString:@"\n"] firstObject];
                        NSString* palette = trimPaletteStringSuffix(firstLine);
                        if (palette.length > 0 && ![seen containsObject:palette]) {
                            [seen addObject:palette];
                            [out addObject:@{@"filename": subName, @"palette": palette}];
                        }
                    }
                }
                continue;
            }
            if (![name.lowercaseString hasSuffix:@".xpalette"]) continue;
            NSString* content = [NSString stringWithContentsOfFile:full
                                                          encoding:NSUTF8StringEncoding
                                                             error:nil];
            if (content.length == 0) continue;
            NSString* firstLine = [[content componentsSeparatedByString:@"\n"] firstObject];
            NSString* palette = trimPaletteStringSuffix(firstLine);
            if (palette.length > 0 && ![seen containsObject:palette]) {
                [seen addObject:palette];
                [out addObject:@{@"filename": name, @"palette": palette}];
            }
        }
    };

    // Show-folder palettes first (user-writable), then bundled.
    NSString* show = [self showFolderPath];
    if (show.length > 0) {
        scanDir([show stringByAppendingPathComponent:@"Palettes"], YES);
    }
    NSString* bundled = [[NSBundle mainBundle] pathForResource:@"palettes" ofType:nil];
    if (bundled.length > 0) {
        scanDir(bundled, YES);
    }
    return out;
}

- (NSString*)savePaletteString:(NSString*)paletteString
                        asName:(NSString*)name {
    if (paletteString.length == 0) return nil;
    NSString* show = [self showFolderPath];
    if (show.length == 0) return nil;

    NSString* paletteDir = [show stringByAppendingPathComponent:@"Palettes"];
    NSFileManager* fm = [NSFileManager defaultManager];
    BOOL isDir = NO;
    if (![fm fileExistsAtPath:paletteDir isDirectory:&isDir] || !isDir) {
        if (![fm createDirectoryAtPath:paletteDir
           withIntermediateDirectories:YES
                            attributes:nil
                                 error:nil]) {
            return nil;
        }
    }

    NSString* filename;
    NSString* sanitised = sanitisePaletteName(name);
    if (sanitised.length == 0) {
        filename = autogenPaletteName(paletteDir);
    } else {
        filename = [sanitised stringByAppendingString:@".xpalette"];
    }
    NSString* full = [paletteDir stringByAppendingPathComponent:filename];

    // Desktop appends the filename as the "identity" trailer after
    // the palette string — preserve that so the files iPad writes
    // are indistinguishable from desktop's.
    NSString* fileContents = [NSString stringWithFormat:@"%@%@",
                              paletteString, filename];
    NSError* err = nil;
    if (![fileContents writeToFile:full
                        atomically:YES
                          encoding:NSUTF8StringEncoding
                             error:&err]) {
        return nil;
    }
    return filename;
}

- (BOOL)deleteSavedPalette:(NSString*)filename {
    if (filename.length == 0) return NO;
    NSString* show = [self showFolderPath];
    if (show.length == 0) return NO;
    NSString* full = [[show stringByAppendingPathComponent:@"Palettes"]
                      stringByAppendingPathComponent:filename];
    NSFileManager* fm = [NSFileManager defaultManager];
    if (![fm fileExistsAtPath:full]) return NO;
    return [fm removeItemAtPath:full error:nil];
}

- (NSString*)currentPaletteStringForRow:(int)rowIndex
                                atIndex:(int)effectIndex {
    if (!_context) return @"";
    EffectLayer* layer = [self effectLayerForRow:rowIndex];
    if (!layer || effectIndex < 0 || effectIndex >= (int)layer->GetEffectCount()) return @"";
    Effect* effect = layer->GetEffect(effectIndex);
    if (!effect) return @"";
    const SettingsMap& settings = effect->GetPaletteMap();
    // Fallback defaults match `ColorPaletteView` so a new / partly-
    // populated palette still serialises as 8 slots.
    static const char* defaults[8] = {
        "#FF0000", "#00FF00", "#0000FF", "#FFFF00",
        "#FFFFFF", "#000000", "#FFA500", "#800080",
    };
    NSMutableString* out = [NSMutableString string];
    for (int i = 0; i < 8; i++) {
        std::string key = "C_BUTTON_Palette" + std::to_string(i + 1);
        std::string v = settings.Get(key, defaults[i]);
        [out appendFormat:@"%s,", v.c_str()];
    }
    return out;
}

- (NSArray<NSString*>*)usedColoursSelectedOnly:(BOOL)selectedOnly {
    NSMutableArray<NSString*>* out = [NSMutableArray array];
    if (!_context) return out;
    std::vector<std::string> used = _context->GetSequenceElements().GetUsedColours(selectedOnly ? true : false);
    for (const auto& c : used) {
        [out addObject:[NSString stringWithUTF8String:c.c_str()]];
    }
    return out;
}

- (int)replaceColourFrom:(NSString*)fromColour to:(NSString*)toColour selectedOnly:(BOOL)selectedOnly {
    if (!_context || fromColour.length == 0 || toColour.length == 0) return 0;
    return _context->GetSequenceElements().ReplaceColours(
        _context.get(),
        std::string([fromColour UTF8String]),
        std::string([toColour UTF8String]),
        selectedOnly ? true : false);
}

// SEQ-2 selected-scope (sync-on-demand): the iPad keeps selection in
// Swift, so before a selectedOnly core op we mirror the given
// (row, effectIndex) pairs into the core Effect Selected flags, run
// the op, then clear them again — the core selection is never left
// dirty. SetSelected does not IncrementChangeCount and the grid draws
// selection from the Swift set, so this has no dirty/render side-effect.
- (void)pushCoreSelectionAtRows:(NSArray<NSNumber*>*)rows effectIndices:(NSArray<NSNumber*>*)indices {
    _context->GetSequenceElements().UnSelectAllEffects();
    NSUInteger n = MIN(rows.count, indices.count);
    for (NSUInteger i = 0; i < n; ++i) {
        EffectLayer* layer = [self effectLayerForRow:[rows[i] intValue]];
        if (!layer) continue;
        int idx = [indices[i] intValue];
        if (idx < 0 || idx >= (int)layer->GetEffectCount()) continue;
        Effect* e = layer->GetEffect(idx);
        if (e) e->SetSelected(EFFECT_SELECTED);
    }
}

- (NSArray<NSString*>*)usedColoursAtRows:(NSArray<NSNumber*>*)rows effectIndices:(NSArray<NSNumber*>*)indices {
    NSMutableArray<NSString*>* out = [NSMutableArray array];
    if (!_context) return out;
    [self pushCoreSelectionAtRows:rows effectIndices:indices];
    std::vector<std::string> used = _context->GetSequenceElements().GetUsedColours(true);
    _context->GetSequenceElements().UnSelectAllEffects();
    for (const auto& c : used) [out addObject:[NSString stringWithUTF8String:c.c_str()]];
    return out;
}

- (int)replaceColourFrom:(NSString*)fromColour to:(NSString*)toColour atRows:(NSArray<NSNumber*>*)rows effectIndices:(NSArray<NSNumber*>*)indices {
    if (!_context || fromColour.length == 0 || toColour.length == 0) return 0;
    [self pushCoreSelectionAtRows:rows effectIndices:indices];
    int n = _context->GetSequenceElements().ReplaceColours(
        _context.get(),
        std::string([fromColour UTF8String]),
        std::string([toColour UTF8String]),
        /*selectedOnly=*/true);
    _context->GetSequenceElements().UnSelectAllEffects();
    return n;
}

// Shift Effects ----------------------------------------------------------

// Collect every EffectLayer reachable from the master-view elements,
// mirroring desktop's `OnMenuItemShiftEffects*` walk (top layers +
// strand node layers + submodel layers). When `includeTiming==NO`,
// timing-track elements are skipped (their layers carry the marks).
- (std::vector<EffectLayer*>)allShiftLayersIncludingTiming:(BOOL)includeTiming {
    std::vector<EffectLayer*> out;
    auto& se = _context->GetSequenceElements();
    for (int elem = 0; elem < (int)se.GetElementCount(MASTER_VIEW); ++elem) {
        Element* ele = se.GetElement(elem, MASTER_VIEW);
        if (!ele) continue;
        bool isTiming = ele->GetType() == ElementType::ELEMENT_TYPE_TIMING;
        if (isTiming && !includeTiming) continue;
        for (int layer = 0; layer < (int)ele->GetEffectLayerCount(); ++layer) {
            if (EffectLayer* el = ele->GetEffectLayer(layer)) out.push_back(el);
        }
        if (ele->GetType() == ElementType::ELEMENT_TYPE_MODEL) {
            ModelElement* me = dynamic_cast<ModelElement*>(ele);
            for (int i = 0; me && i < me->GetStrandCount(); ++i) {
                StrandElement* ste = dynamic_cast<StrandElement*>(me->GetStrand(i));
                for (int k = 0; ste && k < ste->GetNodeLayerCount(); ++k) {
                    if (NodeLayer* nl = ste->GetNodeLayer(k, false)) out.push_back(nl);
                }
            }
            for (int i = 0; me && i < me->GetSubModelAndStrandCount(); ++i) {
                Element* sub = me->GetSubModel(i);
                for (int layer = 0; sub && layer < (int)sub->GetEffectLayerCount(); ++layer) {
                    if (EffectLayer* sel = sub->GetEffectLayer(layer)) out.push_back(sel);
                }
            }
        }
    }
    return out;
}

- (ShiftLayerSnap)snapshotLayer:(EffectLayer*)el {
    ShiftLayerSnap snap;
    snap.layer = el;
    for (int ef = 0; ef < (int)el->GetEffectCount(); ++ef) {
        Effect* e = el->GetEffect(ef);
        if (!e) continue;
        snap.effects.push_back({ e->GetID(), e->GetEffectName(),
                                 e->GetSettingsAsString(), e->GetPaletteAsString(),
                                 e->GetStartTimeMS(), e->GetEndTimeMS(),
                                 e->GetSelected(), e->GetProtected() });
    }
    return snap;
}

// Apply the desktop `ShiftEffectsOnLayer` clamp: iterate backwards so
// deletions are safe; truncate effects whose start crosses 0, delete
// those pushed entirely below 0.
- (void)shiftAllOnLayer:(EffectLayer*)el byMS:(int)ms {
    for (int ef = (int)el->GetEffectCount() - 1; ef >= 0; --ef) {
        Effect* e = el->GetEffect(ef);
        if (!e) continue;
        int start = e->GetStartTimeMS();
        int end = e->GetEndTimeMS();
        if (start + ms < 0) {
            if (end + ms < 0) {
                el->RemoveEffect(ef);
                continue;
            }
            e->SetStartTimeMS(0);
        } else {
            e->SetStartTimeMS(start + ms);
        }
        e->SetEndTimeMS(end + ms);
    }
}

- (int)roundToFrame:(int)ms {
    int frame = [self frameIntervalMS];
    if (frame <= 0) return ms;
    return (ms / frame) * frame;
}

- (NSInteger)shiftAllEffectsByMS:(int)ms includeTiming:(BOOL)includeTiming {
    if (!_context) return 0;
    ms = [self roundToFrame:ms];
    std::vector<EffectLayer*> layers = [self allShiftLayersIncludingTiming:includeTiming];
    if (layers.empty()) return 0;

    std::vector<ShiftLayerSnap> snaps;
    snaps.reserve(layers.size());
    for (EffectLayer* el : layers) snaps.push_back([self snapshotLayer:el]);

    if (ms != 0) {
        for (EffectLayer* el : layers) [self shiftAllOnLayer:el byMS:ms];
        _context->GetSequenceElements().IncrementChangeCount(nullptr);
    }

    NSInteger token = ++_nextShiftToken;
    _shiftSnapshots[token] = std::move(snaps);
    return token;
}

// Selected-scope shift mirroring desktop `ShiftSelectedEffectsOnLayer`:
// negative shifts move left (truncate / delete at 0), positive shifts
// move right; either way a move is rejected when the shifted range
// would clash with an unselected effect on the same layer.
- (void)shiftSelectedOnLayer:(EffectLayer*)el byMS:(int)ms {
    if (ms < 0) {
        std::vector<int> toRemove;
        for (int ef = 0; ef < (int)el->GetEffectCount(); ++ef) {
            Effect* e = el->GetEffect(ef);
            if (!e || !e->GetSelected()) continue;
            bool moved = false;
            int start = e->GetStartTimeMS();
            int end = e->GetEndTimeMS();
            if (start + ms < 0) {
                if (end + ms < 0) {
                    e->SetStartTimeMS(-100);
                    e->SetEndTimeMS(-90);
                    toRemove.insert(toRemove.begin(), ef);
                    continue;
                }
                e->SetStartTimeMS(0);
                moved = true;
            } else {
                auto clashers = el->GetAllEffectsByTime(start + ms, end + ms);
                bool clash = false;
                for (const auto& it : clashers) {
                    if (it->GetID() != e->GetID()) { clash = true; break; }
                }
                if (!clash) {
                    e->SetStartTimeMS(start + ms);
                    moved = true;
                }
            }
            if (moved) e->SetEndTimeMS(end + ms);
        }
        for (int idx : toRemove) el->RemoveEffect(idx);
    } else {
        for (int ef = (int)el->GetEffectCount() - 1; ef >= 0; --ef) {
            Effect* e = el->GetEffect(ef);
            if (!e || !e->GetSelected()) continue;
            int start = e->GetStartTimeMS();
            int end = e->GetEndTimeMS();
            auto clashers = el->GetAllEffectsByTime(start + ms, end + ms);
            bool clash = false;
            for (const auto& it : clashers) {
                if (it->GetID() != e->GetID()) { clash = true; break; }
            }
            if (!clash) {
                e->SetStartTimeMS(start + ms);
                e->SetEndTimeMS(end + ms);
            }
        }
    }
}

- (NSInteger)shiftSelectedEffectsByMS:(int)ms
                               atRows:(NSArray<NSNumber*>*)rows
                        effectIndices:(NSArray<NSNumber*>*)indices {
    if (!_context || rows.count == 0) return 0;
    ms = [self roundToFrame:ms];
    [self pushCoreSelectionAtRows:rows effectIndices:indices];

    // Snapshot every layer that owns a selected effect (whole layer, so
    // a delete near 0 can be restored). Dedupe by layer pointer.
    std::vector<EffectLayer*> layers;
    NSUInteger n = MIN(rows.count, indices.count);
    for (NSUInteger i = 0; i < n; ++i) {
        EffectLayer* el = [self effectLayerForRow:[rows[i] intValue]];
        if (el && std::find(layers.begin(), layers.end(), el) == layers.end()) {
            layers.push_back(el);
        }
    }
    if (layers.empty()) {
        _context->GetSequenceElements().UnSelectAllEffects();
        return 0;
    }

    std::vector<ShiftLayerSnap> snaps;
    snaps.reserve(layers.size());
    for (EffectLayer* el : layers) snaps.push_back([self snapshotLayer:el]);

    if (ms != 0) {
        for (EffectLayer* el : layers) [self shiftSelectedOnLayer:el byMS:ms];
        _context->GetSequenceElements().IncrementChangeCount(nullptr);
    }

    _context->GetSequenceElements().UnSelectAllEffects();
    NSInteger token = ++_nextShiftToken;
    _shiftSnapshots[token] = std::move(snaps);
    return token;
}

- (BOOL)restoreShiftSnapshot:(NSInteger)token {
    if (!_context) return NO;
    auto it = _shiftSnapshots.find(token);
    if (it == _shiftSnapshots.end()) return NO;
    for (const ShiftLayerSnap& ls : it->second) {
        EffectLayer* el = ls.layer;
        el->RemoveAllEffects(nullptr);
        for (const ShiftEffectSnap& es : ls.effects) {
            // Locked effects survive RemoveAllEffects (and the shift's
            // RemoveEffect) keeping their ID; restore their times in
            // place rather than re-adding a duplicate.
            if (Effect* existing = el->GetEffectFromID(es.id)) {
                existing->SetStartTimeMS(es.startMS);
                existing->SetEndTimeMS(es.endMS);
                continue;
            }
            el->AddEffect(es.id, es.name, es.settings, es.palette,
                          es.startMS, es.endMS, es.selected, es.protectd);
        }
    }
    _context->GetSequenceElements().IncrementChangeCount(nullptr);
    _shiftSnapshots.erase(it);
    return YES;
}

- (BOOL)applyPaletteString:(NSString*)paletteString
                     toRow:(int)rowIndex
                   atIndex:(int)effectIndex {
    if (!_context || paletteString.length == 0) return NO;
    EffectLayer* layer = [self effectLayerForRow:rowIndex];
    if (!layer || effectIndex < 0 || effectIndex >= (int)layer->GetEffectCount()) return NO;
    Effect* effect = layer->GetEffect(effectIndex);
    if (!effect) return NO;

    // Palette slot values can themselves contain commas when they
    // hold a ColorCurve blob (`Active=TRUE|Id=…|Values=x=0.000^c=#FF00@FF;…|`).
    // Split on top-level commas only — walk the string tracking
    // whether we're inside an `Active=TRUE|…|` block. Trailing
    // filename trailer (".xpalette") is ignored.
    std::string src([paletteString UTF8String]);
    std::vector<std::string> slots;
    size_t start = 0;
    bool inCurve = false;
    for (size_t i = 0; i < src.size(); ++i) {
        char c = src[i];
        if (!inCurve && c == ',') {
            slots.push_back(src.substr(start, i - start));
            start = i + 1;
        } else if (!inCurve && src.compare(i, 12, "Active=TRUE|") == 0) {
            inCurve = true;
            i += 11;
        } else if (inCurve && c == '|') {
            // A curve ends with "|..." — we treat the trailing `|`
            // that closes the `Values=` section as the curve
            // terminator. The ColorCurve serialiser writes the
            // final `|` after the values block, so when we see
            // `|,` (end-of-curve followed by slot separator) close
            // the curve.
            if (i + 1 < src.size() && src[i + 1] == ',') {
                inCurve = false;
            }
        }
    }
    if (start < src.size()) {
        slots.push_back(src.substr(start));
    }

    int applied = 0;
    for (size_t i = 0; i < slots.size() && applied < 8; ++i) {
        std::string v = slots[i];
        if (v.empty()) continue;
        // Drop the trailer "PAL001.xpalette" if it slipped in.
        if (v.find(".xpalette") != std::string::npos) continue;

        std::string key = "C_BUTTON_Palette" + std::to_string(applied + 1);
        effect->GetPaletteMap()[key] = v;
        applied++;
    }
    effect->PaletteMapUpdated();
    return applied > 0;
}

#pragma mark - Value-curve preset save / load

namespace {

NSString* sanitiseVCName(NSString* raw) {
    if (raw.length == 0) return @"";
    NSMutableString* out = [NSMutableString string];
    for (NSUInteger i = 0; i < raw.length; i++) {
        unichar c = [raw characterAtIndex:i];
        if ((c >= '0' && c <= '9') ||
            (c >= 'a' && c <= 'z') ||
            (c >= 'A' && c <= 'Z')) {
            [out appendFormat:@"%C", c];
        }
    }
    return out;
}

NSString* autogenVCName(NSString* dir) {
    NSFileManager* fm = [NSFileManager defaultManager];
    for (int i = 1; i < 1000; i++) {
        NSString* candidate = [NSString stringWithFormat:@"VC%03d.xvc", i];
        if (![fm fileExistsAtPath:[dir stringByAppendingPathComponent:candidate]]) {
            return candidate;
        }
    }
    return @"VC999.xvc";
}

NSString* autogenCCName(NSString* dir) {
    NSFileManager* fm = [NSFileManager defaultManager];
    for (int i = 1; i < 1000; i++) {
        NSString* candidate = [NSString stringWithFormat:@"CC%03d.xcc", i];
        if (![fm fileExistsAtPath:[dir stringByAppendingPathComponent:candidate]]) {
            return candidate;
        }
    }
    return @"CC999.xcc";
}

// Build the `.xcc` XML document for a serialised ColorCurve, matching
// ColorCurveDialog::OnButtonExportClick byte-for-byte.
std::string buildXccDocument(const std::string& serialised) {
    std::string doc = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n<colorcurve \n";
    doc += "data=\"" + serialised + "\" ";
    doc += "SourceVersion=\"" + xlights_version_string + "\" ";
    doc += " >\n";
    doc += "</colorcurve>";
    return doc;
}

} // namespace

- (NSArray<NSDictionary<NSString*, NSString*>*>*)savedValueCurves {
    NSMutableArray<NSDictionary<NSString*, NSString*>*>* out = [NSMutableArray array];
    NSMutableSet<NSString*>* seen = [NSMutableSet set];

    auto scanDir = ^(NSString* dir, BOOL recurse) {
        NSFileManager* fm = [NSFileManager defaultManager];
        BOOL isDir = NO;
        if (![fm fileExistsAtPath:dir isDirectory:&isDir] || !isDir) return;
        NSArray* entries = [fm contentsOfDirectoryAtPath:dir error:nil];
        for (NSString* name in entries) {
            NSString* full = [dir stringByAppendingPathComponent:name];
            BOOL sub = NO;
            [fm fileExistsAtPath:full isDirectory:&sub];
            if (sub) {
                if (recurse) {
                    NSArray* subEntries = [fm contentsOfDirectoryAtPath:full error:nil];
                    for (NSString* subName in subEntries) {
                        if (![subName.lowercaseString hasSuffix:@".xvc"]) continue;
                        ValueCurve vc("");
                        vc.LoadXVC([[full stringByAppendingPathComponent:subName] UTF8String]);
                        std::string s = vc.Serialise();
                        NSString* serialised = [NSString stringWithUTF8String:s.c_str()];
                        if (serialised.length > 0 && ![seen containsObject:serialised]) {
                            [seen addObject:serialised];
                            [out addObject:@{@"filename": subName, @"serialised": serialised}];
                        }
                    }
                }
                continue;
            }
            if (![name.lowercaseString hasSuffix:@".xvc"]) continue;
            ValueCurve vc("");
            vc.LoadXVC([full UTF8String]);
            std::string s = vc.Serialise();
            NSString* serialised = [NSString stringWithUTF8String:s.c_str()];
            if (serialised.length > 0 && ![seen containsObject:serialised]) {
                [seen addObject:serialised];
                [out addObject:@{@"filename": name, @"serialised": serialised}];
            }
        }
    };

    NSString* show = [self showFolderPath];
    if (show.length > 0) {
        scanDir([show stringByAppendingPathComponent:@"valuecurves"], YES);
    }
    NSString* bundled = [[NSBundle mainBundle] pathForResource:@"valuecurves" ofType:nil];
    if (bundled.length > 0) {
        scanDir(bundled, YES);
    }
    return out;
}

- (NSString*)saveValueCurveSerialised:(NSString*)serialised
                                asName:(NSString*)name {
    if (serialised.length == 0) return nil;
    NSString* show = [self showFolderPath];
    if (show.length == 0) return nil;

    NSString* dir = [show stringByAppendingPathComponent:@"valuecurves"];
    NSFileManager* fm = [NSFileManager defaultManager];
    BOOL isDir = NO;
    if (![fm fileExistsAtPath:dir isDirectory:&isDir] || !isDir) {
        if (![fm createDirectoryAtPath:dir
           withIntermediateDirectories:YES
                            attributes:nil
                                 error:nil]) {
            return nil;
        }
    }

    NSString* sanitised = sanitiseVCName(name);
    NSString* filename = sanitised.length == 0
        ? autogenVCName(dir)
        : [sanitised stringByAppendingString:@".xvc"];
    NSString* full = [dir stringByAppendingPathComponent:filename];

    // Reuse core's `SaveXVC` — it applies the ID / limits
    // normalisation desktop expects, so files round-trip.
    ValueCurve vc([serialised UTF8String]);
    vc.SaveXVC([full UTF8String]);
    if (![fm fileExistsAtPath:full]) return nil;
    return filename;
}

- (BOOL)deleteSavedValueCurve:(NSString*)filename {
    if (filename.length == 0) return NO;
    NSString* show = [self showFolderPath];
    if (show.length == 0) return NO;
    NSString* full = [[show stringByAppendingPathComponent:@"valuecurves"]
                      stringByAppendingPathComponent:filename];
    NSFileManager* fm = [NSFileManager defaultManager];
    if (![fm fileExistsAtPath:full]) return NO;
    return [fm removeItemAtPath:full error:nil];
}

- (NSString*)valueCurveXvcDocument:(NSString*)serialised {
    if (serialised.length == 0) return nil;
    // Route through core SaveXVC (it applies the limit / scale
    // normalisation desktop expects) by writing a temp file then
    // reading it back as the document body.
    NSString* tmp = [NSTemporaryDirectory()
        stringByAppendingPathComponent:[[NSUUID UUID].UUIDString stringByAppendingString:@".xvc"]];
    ValueCurve vc([serialised UTF8String]);
    vc.SaveXVC([tmp UTF8String]);
    NSFileManager* fm = [NSFileManager defaultManager];
    NSString* contents = [NSString stringWithContentsOfFile:tmp
                                                   encoding:NSUTF8StringEncoding
                                                      error:nil];
    [fm removeItemAtPath:tmp error:nil];
    return contents;
}

#pragma mark - Color-curve preset save / load

- (NSArray<NSDictionary<NSString*, NSString*>*>*)savedColorCurves {
    NSMutableArray<NSDictionary<NSString*, NSString*>*>* out = [NSMutableArray array];
    NSMutableSet<NSString*>* seen = [NSMutableSet set];

    auto loadSerialised = ^NSString*(NSString* path) {
        ColorCurve cc;
        cc.SetId("Dummy"); // ColorCurve::IsActive() requires a non-empty id
        cc.LoadXCC([path UTF8String]);
        if (!cc.IsActive()) return nil; // active only if it loaded ok
        std::string s = cc.Serialise();
        return [NSString stringWithUTF8String:s.c_str()];
    };

    auto scanDir = ^(NSString* dir, BOOL recurse) {
        NSFileManager* fm = [NSFileManager defaultManager];
        BOOL isDir = NO;
        if (![fm fileExistsAtPath:dir isDirectory:&isDir] || !isDir) return;
        NSArray* entries = [fm contentsOfDirectoryAtPath:dir error:nil];
        for (NSString* name in entries) {
            NSString* full = [dir stringByAppendingPathComponent:name];
            BOOL sub = NO;
            [fm fileExistsAtPath:full isDirectory:&sub];
            if (sub) {
                if (recurse) {
                    NSArray* subEntries = [fm contentsOfDirectoryAtPath:full error:nil];
                    for (NSString* subName in subEntries) {
                        if (![subName.lowercaseString hasSuffix:@".xcc"]) continue;
                        NSString* serialised = loadSerialised([full stringByAppendingPathComponent:subName]);
                        if (serialised.length > 0 && ![seen containsObject:serialised]) {
                            [seen addObject:serialised];
                            [out addObject:@{@"filename": subName, @"serialised": serialised}];
                        }
                    }
                }
                continue;
            }
            if (![name.lowercaseString hasSuffix:@".xcc"]) continue;
            NSString* serialised = loadSerialised(full);
            if (serialised.length > 0 && ![seen containsObject:serialised]) {
                [seen addObject:serialised];
                [out addObject:@{@"filename": name, @"serialised": serialised}];
            }
        }
    };

    NSString* show = [self showFolderPath];
    if (show.length > 0) {
        scanDir([show stringByAppendingPathComponent:@"colorcurves"], YES);
    }
    NSString* bundled = [[NSBundle mainBundle] pathForResource:@"colorcurves" ofType:nil];
    if (bundled.length > 0) {
        scanDir(bundled, YES);
    }
    return out;
}

- (NSString*)saveColorCurveSerialised:(NSString*)serialised
                               asName:(NSString*)name {
    if (serialised.length == 0) return nil;
    NSString* show = [self showFolderPath];
    if (show.length == 0) return nil;

    NSString* dir = [show stringByAppendingPathComponent:@"colorcurves"];
    NSFileManager* fm = [NSFileManager defaultManager];
    BOOL isDir = NO;
    if (![fm fileExistsAtPath:dir isDirectory:&isDir] || !isDir) {
        if (![fm createDirectoryAtPath:dir
           withIntermediateDirectories:YES
                            attributes:nil
                                 error:nil]) {
            return nil;
        }
    }

    NSString* sanitised = sanitiseVCName(name);
    NSString* filename = sanitised.length == 0
        ? autogenCCName(dir)
        : [sanitised stringByAppendingString:@".xcc"];
    NSString* full = [dir stringByAppendingPathComponent:filename];

    // Normalise through core ColorCurve so the persisted `data=` body
    // matches what desktop writes, then wrap in the same XML envelope.
    ColorCurve cc;
    cc.SetId("Dummy");
    cc.Deserialise([serialised UTF8String]);
    cc.SetActive(true);
    std::string doc = buildXccDocument(cc.Serialise());

    NSData* data = [NSData dataWithBytes:doc.data() length:doc.size()];
    if (![data writeToFile:full atomically:YES]) return nil;
    return filename;
}

- (BOOL)deleteSavedColorCurve:(NSString*)filename {
    if (filename.length == 0) return NO;
    NSString* show = [self showFolderPath];
    if (show.length == 0) return NO;
    NSString* full = [[show stringByAppendingPathComponent:@"colorcurves"]
                      stringByAppendingPathComponent:filename];
    NSFileManager* fm = [NSFileManager defaultManager];
    if (![fm fileExistsAtPath:full]) return NO;
    return [fm removeItemAtPath:full error:nil];
}

- (NSString*)colorCurveXccDocument:(NSString*)serialised {
    if (serialised.length == 0) return nil;
    ColorCurve cc;
    cc.SetId("Dummy");
    cc.Deserialise([serialised UTF8String]);
    cc.SetActive(true);
    std::string doc = buildXccDocument(cc.Serialise());
    return [NSString stringWithUTF8String:doc.c_str()];
}

/// Resolve the target Model for a row's effect, unwrapping ModelGroups
/// the same way desktop does (JsonEffectPanel.cpp:1815-1818). Returns
/// nullptr on any lookup failure.
- (Model*)_targetModelForRow:(int)rowIndex {
    auto* row = _context->GetSequenceElements().GetRowInformation(rowIndex);
    if (!row || !row->element) return nullptr;
    const std::string& modelName = row->element->GetModelName();
    Model* m = _context->GetModelManager()[modelName];
    if (!m) return nullptr;
    if (m->GetDisplayAs() == DisplayAsType::ModelGroup) {
        auto* mg = dynamic_cast<ModelGroup*>(m);
        if (mg) m = mg->GetFirstModel();
    }
    return m;
}

- (NSArray<NSString*>*)_keysOfFaceStateData:(const FaceStateData&)map {
    NSMutableArray<NSString*>* out = [NSMutableArray array];
    for (const auto& pair : map) {
        [out addObject:[NSString stringWithUTF8String:pair.first.c_str()]];
    }
    return out;
}

- (NSArray<NSString*>*)statesForRow:(int)rowIndex atIndex:(int)effectIndex {
    (void)effectIndex;  // state list is model-scoped, not effect-scoped
    Model* m = [self _targetModelForRow:rowIndex];
    if (!m) return @[];
    return [self _keysOfFaceStateData:m->GetStateInfo()];
}

- (NSArray<NSString*>*)facesForRow:(int)rowIndex atIndex:(int)effectIndex {
    (void)effectIndex;
    Model* m = [self _targetModelForRow:rowIndex];
    if (!m) return @[];
    return [self _keysOfFaceStateData:m->GetFaceInfo()];
}

- (NSArray<NSString*>*)modelNodeNamesForRow:(int)rowIndex atIndex:(int)effectIndex {
    (void)effectIndex;
    Model* m = [self _targetModelForRow:rowIndex];
    if (!m) return @[];
    NSMutableArray<NSString*>* out = [NSMutableArray array];
    uint32_t n = m->GetNumChannels();
    for (uint32_t i = 0; i < n; i++) {
        std::string name = m->GetNodeName((size_t)i, /*def*/ false);
        if (name.empty()) continue;
        if (!name.empty() && name[0] == '-') continue;  // desktop skips "-..." names
        [out addObject:[NSString stringWithUTF8String:name.c_str()]];
    }
    return out;
}

- (NSArray<NSString*>*)effectSettingOptionsForRow:(int)rowIndex
                                          atIndex:(int)effectIndex
                                         settingId:(NSString*)settingId {
    auto* layer = [self effectLayerForRow:rowIndex];
    if (!layer || effectIndex < 0 || effectIndex >= layer->GetEffectCount()) return @[];
    Effect* e = layer->GetEffect(effectIndex);
    if (!e) return @[];
    auto& em = _context->GetEffectManager();
    RenderableEffect* re = em.GetEffect(e->GetEffectName());
    if (!re) return @[];
    auto options = re->GetSettingOptions(std::string([settingId UTF8String]));
    NSMutableArray<NSString*>* out = [NSMutableArray arrayWithCapacity:options.size()];
    for (const auto& s : options) {
        [out addObject:[NSString stringWithUTF8String:s.c_str()]];
    }
    return out;
}

- (NSDictionary<NSString*, NSString*>*)effectSettingsForRow:(int)rowIndex atIndex:(int)effectIndex {
    auto* layer = [self effectLayerForRow:rowIndex];
    if (!layer || effectIndex < 0 || effectIndex >= layer->GetEffectCount()) return @{};

    Effect* e = layer->GetEffect(effectIndex);
    if (!e) return @{};

    NSMutableDictionary* dict = [NSMutableDictionary dictionary];
    for (const auto& [key, value] : e->GetSettings()) {
        dict[[NSString stringWithUTF8String:key.c_str()]] =
            [NSString stringWithUTF8String:value.c_str()];
    }
    return dict;
}

- (NSDictionary<NSString*, NSString*>*)effectPaletteForRow:(int)rowIndex atIndex:(int)effectIndex {
    auto* layer = [self effectLayerForRow:rowIndex];
    if (!layer || effectIndex < 0 || effectIndex >= layer->GetEffectCount()) return @{};

    Effect* e = layer->GetEffect(effectIndex);
    if (!e) return @{};

    NSMutableDictionary* dict = [NSMutableDictionary dictionary];
    for (const auto& [key, value] : e->GetPaletteMap()) {
        dict[[NSString stringWithUTF8String:key.c_str()]] =
            [NSString stringWithUTF8String:value.c_str()];
    }
    return dict;
}

// MARK: - Effect Metadata

- (NSString*)metadataJsonForEffectNamed:(NSString*)effectName {
    auto& em = _context->GetEffectManager();
    RenderableEffect* re = em.GetEffect(std::string([effectName UTF8String]));
    if (!re || !re->HasMetadata()) return @"";

    // nlohmann::json::dump() serializes to a compact JSON string.
    std::string dumped = re->GetMetadata().dump();
    return [NSString stringWithUTF8String:dumped.c_str()];
}

- (NSString*)sharedMetadataJsonNamed:(NSString*)name {
    // Shared metadata lives in <resources>/effectmetadata/shared/<Name>.json
    std::string resDir = FileUtils::GetResourcesDir();
    if (resDir.empty()) return @"";

    std::string path = resDir + "/effectmetadata/shared/" + std::string([name UTF8String]) + ".json";
    std::ifstream f(path);
    if (!f.is_open()) return @"";

    std::stringstream ss;
    ss << f.rdbuf();
    return [NSString stringWithUTF8String:ss.str().c_str()];
}

- (NSString*)shaderDynamicPropertiesJsonForPath:(NSString*)shaderPath {
    if (!shaderPath || shaderPath.length == 0) return @"";

    // Mirrors the desktop path: parse the .fs into a ShaderConfig, ask
    // for the JSON-shaped dynamic-property array, return it as a string
    // the Swift-side panel builder can decode exactly like static
    // effect metadata. Caller owns the returned string; we free the
    // config before we return.
    ShaderConfig* cfg = ShaderEffect::ParseShader(
        std::string([shaderPath UTF8String]),
        &_context->GetSequenceElements());
    if (!cfg) return @"";

    std::string dumped = cfg->GetDynamicPropertiesJson().dump();
    delete cfg;
    return [NSString stringWithUTF8String:dumped.c_str()];
}

// MARK: - Effect Setting Read/Write

// C_-prefixed keys live in palette map; everything else in settings map.
static bool isPaletteKey(const std::string& key) {
    return key.size() > 2 && key[0] == 'C' && key[1] == '_';
}

- (NSString*)effectSettingValueForKey:(NSString*)key
                                inRow:(int)rowIndex
                              atIndex:(int)effectIndex {
    auto* layer = [self effectLayerForRow:rowIndex];
    if (!layer || effectIndex < 0 || effectIndex >= layer->GetEffectCount()) return @"";

    Effect* e = layer->GetEffect(effectIndex);
    if (!e) return @"";

    std::string k = [key UTF8String];
    const SettingsMap& map = isPaletteKey(k) ? e->GetPaletteMap() : e->GetSettings();
    if (!map.Contains(k)) return @"";
    return [NSString stringWithUTF8String:map[k].c_str()];
}

- (BOOL)setEffectSettingValue:(NSString*)value
                       forKey:(NSString*)key
                        inRow:(int)rowIndex
                      atIndex:(int)effectIndex {
    auto* layer = [self effectLayerForRow:rowIndex];
    if (!layer || effectIndex < 0 || effectIndex >= layer->GetEffectCount()) return NO;

    Effect* e = layer->GetEffect(effectIndex);
    if (!e) return NO;

    std::string k = [key UTF8String];
    std::string v = [value UTF8String];

    if (isPaletteKey(k)) {
        SettingsMap& map = e->GetPaletteMap();
        if (map.Contains(k) && map[k] == v) return NO;
        map[k] = v;
        // Rebuild the derived `mColors` / `mCC` vectors — the render
        // engine reads from those (via `CopyPalette`), not from
        // `mPaletteMap`. Skipping this step leaves the rendered
        // effect stale: the user's enable/disable toggle flips the
        // map entry but the cached colour vector doesn't change, so
        // nothing visibly updates.
        e->PaletteMapUpdated();
        return YES;
    } else {
        // SetSetting returns true if the value actually changed.
        return e->SetSetting(k, v);
    }
}

- (BOOL)removeEffectSettingForKey:(NSString*)key
                            inRow:(int)rowIndex
                          atIndex:(int)effectIndex {
    auto* layer = [self effectLayerForRow:rowIndex];
    if (!layer || effectIndex < 0 || effectIndex >= layer->GetEffectCount()) return NO;

    Effect* e = layer->GetEffect(effectIndex);
    if (!e) return NO;

    std::string k = [key UTF8String];
    bool palette = isPaletteKey(k);
    SettingsMap& map = palette ? e->GetPaletteMap() : e->GetSettings();
    if (!map.Contains(k)) return NO;
    map.erase(k);
    if (palette) {
        e->PaletteMapUpdated();   // refresh mColors / mCC + IncrementChangeCount
    } else {
        e->IncrementChangeCount();
    }
    return YES;
}

// MARK: - Fade In/Out

static const char* kFadeInKey  = "T_TEXTCTRL_Fadein";
static const char* kFadeOutKey = "T_TEXTCTRL_Fadeout";

- (float)effectFadeInSecondsForRow:(int)rowIndex atIndex:(int)effectIndex {
    auto* layer = [self effectLayerForRow:rowIndex];
    if (!layer || effectIndex < 0 || effectIndex >= layer->GetEffectCount()) return 0.0f;
    Effect* e = layer->GetEffect(effectIndex);
    if (!e) return 0.0f;
    const SettingsMap& map = e->GetSettings();
    if (!map.Contains(kFadeInKey)) return 0.0f;
    return (float)std::strtod(map[kFadeInKey].c_str(), nullptr);
}

- (float)effectFadeOutSecondsForRow:(int)rowIndex atIndex:(int)effectIndex {
    auto* layer = [self effectLayerForRow:rowIndex];
    if (!layer || effectIndex < 0 || effectIndex >= layer->GetEffectCount()) return 0.0f;
    Effect* e = layer->GetEffect(effectIndex);
    if (!e) return 0.0f;
    const SettingsMap& map = e->GetSettings();
    if (!map.Contains(kFadeOutKey)) return 0.0f;
    return (float)std::strtod(map[kFadeOutKey].c_str(), nullptr);
}

- (BOOL)setEffectFadeInSeconds:(float)seconds
                          forRow:(int)rowIndex
                         atIndex:(int)effectIndex {
    auto* layer = [self effectLayerForRow:rowIndex];
    if (!layer || effectIndex < 0 || effectIndex >= layer->GetEffectCount()) return NO;
    Effect* e = layer->GetEffect(effectIndex);
    if (!e) return NO;
    char buf[32];
    std::snprintf(buf, sizeof(buf), "%.2f", seconds);
    return e->SetSetting(kFadeInKey, buf);
}

- (BOOL)setEffectFadeOutSeconds:(float)seconds
                           forRow:(int)rowIndex
                          atIndex:(int)effectIndex {
    auto* layer = [self effectLayerForRow:rowIndex];
    if (!layer || effectIndex < 0 || effectIndex >= layer->GetEffectCount()) return NO;
    Effect* e = layer->GetEffect(effectIndex);
    if (!e) return NO;
    char buf[32];
    std::snprintf(buf, sizeof(buf), "%.2f", seconds);
    return e->SetSetting(kFadeOutKey, buf);
}

// MARK: - Edge Resize

- (BOOL)resizeEffectEdgeInRow:(int)rowIndex
                       atIndex:(int)effectIndex
                          edge:(int)edge
                        toMS:(int)newMS {
    auto* layer = [self effectLayerForRow:rowIndex];
    if (!layer || effectIndex < 0 || effectIndex >= layer->GetEffectCount()) return NO;
    Effect* e = layer->GetEffect(effectIndex);
    if (!e) return NO;

    if (edge == 0) { // left / start
        int maxStart = e->GetEndTimeMS() - 1;
        if (effectIndex > 0) {
            int prevEnd = layer->GetEffect(effectIndex - 1)->GetEndTimeMS();
            if (newMS < prevEnd) newMS = prevEnd;
        }
        if (newMS < 0) newMS = 0;
        if (newMS > maxStart) return NO;
        e->SetStartTimeMS(newMS);
    } else { // right / end
        int minEnd = e->GetStartTimeMS() + 1;
        if (effectIndex + 1 < layer->GetEffectCount()) {
            int nextStart = layer->GetEffect(effectIndex + 1)->GetStartTimeMS();
            if (newMS > nextStart) newMS = nextStart;
        }
        if (newMS < minEnd) return NO;
        e->SetEndTimeMS(newMS);
    }
    return YES;
}

- (void)renderEffectForRow:(int)rowIndex atIndex:(int)effectIndex {
    auto* layer = [self effectLayerForRow:rowIndex];
    if (!layer || effectIndex < 0 || effectIndex >= layer->GetEffectCount()) return;

    Effect* e = layer->GetEffect(effectIndex);
    if (!e) return;

    Element* element = layer->GetParentElement();
    if (!element) return;

    _context->RenderEffectForModel(element->GetModelName(),
                                    e->GetStartTimeMS(),
                                    e->GetEndTimeMS(),
                                    false);
}

- (void)renderRangeForRow:(int)rowIndex
                  startMS:(int)startMS
                    endMS:(int)endMS
                    clear:(BOOL)clear {
    auto* layer = [self effectLayerForRow:rowIndex];
    if (!layer) return;
    Element* element = layer->GetParentElement();
    if (!element) return;
    _context->RenderEffectForModel(element->GetModelName(),
                                    startMS, endMS, clear ? true : false);
}

// MARK: - Lock / Disable

- (BOOL)effectIsLockedInRow:(int)rowIndex atIndex:(int)effectIndex {
    auto* layer = [self effectLayerForRow:rowIndex];
    if (!layer || effectIndex < 0 || effectIndex >= layer->GetEffectCount()) return NO;
    Effect* e = layer->GetEffect(effectIndex);
    return (e && e->IsLocked()) ? YES : NO;
}

- (void)setEffectLocked:(BOOL)locked inRow:(int)rowIndex atIndex:(int)effectIndex {
    auto* layer = [self effectLayerForRow:rowIndex];
    if (!layer || effectIndex < 0 || effectIndex >= layer->GetEffectCount()) return;
    Effect* e = layer->GetEffect(effectIndex);
    if (e) e->SetLocked(locked ? true : false);
}

- (BOOL)effectIsRenderDisabledInRow:(int)rowIndex atIndex:(int)effectIndex {
    auto* layer = [self effectLayerForRow:rowIndex];
    if (!layer || effectIndex < 0 || effectIndex >= layer->GetEffectCount()) return NO;
    Effect* e = layer->GetEffect(effectIndex);
    return (e && e->IsEffectRenderDisabled()) ? YES : NO;
}

- (void)setEffectRenderDisabled:(BOOL)disabled inRow:(int)rowIndex atIndex:(int)effectIndex {
    auto* layer = [self effectLayerForRow:rowIndex];
    if (!layer || effectIndex < 0 || effectIndex >= layer->GetEffectCount()) return;
    Effect* e = layer->GetEffect(effectIndex);
    if (e) e->SetEffectRenderDisabled(disabled ? true : false);
}

// MARK: - Copy / Paste helpers

- (NSString*)effectSettingsStringForRow:(int)rowIndex atIndex:(int)effectIndex {
    auto* layer = [self effectLayerForRow:rowIndex];
    if (!layer || effectIndex < 0 || effectIndex >= layer->GetEffectCount()) return @"";
    Effect* e = layer->GetEffect(effectIndex);
    if (!e) return @"";
    return [NSString stringWithUTF8String:e->GetSettingsAsString().c_str()];
}

- (NSString*)effectPaletteStringForRow:(int)rowIndex atIndex:(int)effectIndex {
    auto* layer = [self effectLayerForRow:rowIndex];
    if (!layer || effectIndex < 0 || effectIndex >= layer->GetEffectCount()) return @"";
    Effect* e = layer->GetEffect(effectIndex);
    if (!e) return @"";
    return [NSString stringWithUTF8String:e->GetPaletteAsString().c_str()];
}

- (NSString*)effectNameForRow:(int)rowIndex atIndex:(int)effectIndex {
    auto* layer = [self effectLayerForRow:rowIndex];
    if (!layer || effectIndex < 0 || effectIndex >= layer->GetEffectCount()) return @"";
    Effect* e = layer->GetEffect(effectIndex);
    if (!e) return @"";
    return [NSString stringWithUTF8String:e->GetEffectName().c_str()];
}

- (NSArray<NSDictionary*>*)copyModelInclSubmodelsAtRow:(int)rowIndex {
    auto& se = _context->GetSequenceElements();
    auto* row = se.GetRowInformation(rowIndex);
    if (!row || !row->element) return @[];
    ModelElement* me = dynamic_cast<ModelElement*>(row->element);
    if (!me) {
        if (auto* sm = dynamic_cast<SubModelElement*>(row->element)) {
            me = sm->GetModelElement();
        }
    }
    if (!me) return @[];

    NSMutableArray<NSDictionary*>* out = [NSMutableArray array];
    int layerRow = 0;
    auto appendLayer = [&](EffectLayer* el) {
        if (!el) return;
        for (int x = 0; x < el->GetEffectCount(); ++x) {
            Effect* ef = el->GetEffect(x);
            if (!ef) continue;
            [out addObject:@{
                @"name": [NSString stringWithUTF8String:ef->GetEffectName().c_str()],
                @"settings": [NSString stringWithUTF8String:ef->GetSettingsAsString().c_str()],
                @"palette": [NSString stringWithUTF8String:ef->GetPaletteAsString().c_str()],
                @"rowOffset": @(layerRow),
                @"startMS": @(ef->GetStartTimeMS()),
                @"endMS": @(ef->GetEndTimeMS()),
            }];
        }
        ++layerRow;
    };

    for (int j = 0; j < (int)me->GetEffectLayerCount(); ++j) {
        appendLayer(me->GetEffectLayer(j));
    }
    for (int s = 0; s < me->GetSubModelCount(); ++s) {
        SubModelElement* sub = me->GetSubModel(s);
        if (!sub) continue;
        for (int j = 0; j < (int)sub->GetEffectLayerCount(); ++j) {
            appendLayer(sub->GetEffectLayer(j));
        }
    }
    return out;
}

- (NSArray<NSDictionary*>*)findSourceEffectsForRow:(int)rowIndex atMS:(int)ms {
    auto& se = _context->GetSequenceElements();
    auto* row = se.GetRowInformation(rowIndex);
    if (!row || !row->element) return @[];
    // Only node / strand rows have a channel range to trace back.
    if (row->strandIndex < 0 || row->nodeIndex < 0) return @[];

    Element* rowElement = row->element;
    int strandIndex = row->strandIndex;
    if (rowElement->GetType() == ElementType::ELEMENT_TYPE_STRAND) {
        if (auto* st = dynamic_cast<StrandElement*>(rowElement)) {
            strandIndex -= st->GetModelElement()->GetSubModelCount();
        }
    }
    Model* rowModel = _context->GetModel(rowElement->GetModelName());
    if (!rowModel) return @[];
    uint8_t chans = rowModel->GetChanCountPerNode();
    uint32_t channel = rowModel->GetChannelForNode(strandIndex, row->nodeIndex);
    if (channel == 0xFFFFFFFF || chans == 0) return @[];

    // Resolve a container element + layer back to a currently-visible
    // (row, effectIndex) so the UI can jump+select. -1/-1 when hidden.
    auto resolveVisible = [&](Element* container, EffectLayer* layer, Effect* ef) -> std::pair<int, int> {
        if (!container || !layer || !ef) return {-1, -1};
        int n = se.GetRowInformationSize();
        for (int i = 0; i < n; ++i) {
            auto* ri = se.GetRowInformation(i);
            if (ri && ri->element == container && ri->layerIndex == layer->GetIndex()
                && ri->strandIndex < 0 && ri->nodeIndex < 0) {
                for (int x = 0; x < layer->GetEffectCount(); ++x) {
                    if (layer->GetEffect(x) == ef) return {i, x};
                }
                return {i, -1};
            }
        }
        return {-1, -1};
    };

    NSMutableArray<NSDictionary*>* out = [NSMutableArray array];
    auto consider = [&](Element* container, EffectLayer* layer, bool matches) {
        if (!matches || !layer) return;
        if (!layer->HasEffectsInTimeRange(ms, ms)) return;
        Effect* ef = layer->GetEffectAtTime(ms);
        if (!ef) return;
        auto vis = resolveVisible(container, layer, ef);
        std::string label = container->GetName() + " · L" + std::to_string(layer->GetLayerNumber())
            + " · " + ef->GetEffectName();
        [out addObject:@{
            @"label": [NSString stringWithUTF8String:label.c_str()],
            @"startMS": @(ef->GetStartTimeMS()),
            @"endMS": @(ef->GetEndTimeMS()),
            @"visibleRow": @(vis.first),
            @"effectIndex": @(vis.second),
        }];
    };

    for (size_t i = 0; i < se.GetElementCount(); ++i) {
        Element* e = se.GetElement(i);
        ModelElement* me = dynamic_cast<ModelElement*>(e);
        if (!me) continue;
        Model* model = _context->GetModel(e->GetModelName());
        if (!model) continue;

        for (auto* layer : me->GetEffectLayers()) {
            consider(me, layer, model->ContainsChannel(channel, channel + chans - 1));
        }
        for (int s = 0; s < me->GetSubModelCount(); ++s) {
            SubModelElement* sm = me->GetSubModel(s);
            if (!sm) continue;
            bool m = model->ContainsChannel(sm->GetName(), channel, channel + chans - 1);
            for (auto* layer : sm->GetEffectLayers()) {
                consider(sm, layer, m);
            }
        }
        for (int s = 0; s < me->GetStrandCount(); ++s) {
            StrandElement* st = me->GetStrand(s);
            if (!st) continue;
            bool m = model->ContainsChannel(st->GetStrand(), channel, channel + chans - 1);
            for (auto* layer : st->GetEffectLayers()) {
                consider(st, layer, m);
            }
        }
    }
    return out;
}

// B20: read/write free-text description. Stored as
// `X_Effect_Description` so it survives `SetSettings(.., keep=true)`
// from randomise / reset / preset-apply.
- (NSString*)effectDescriptionForRow:(int)rowIndex atIndex:(int)effectIndex {
    auto* layer = [self effectLayerForRow:rowIndex];
    if (!layer || effectIndex < 0 || effectIndex >= layer->GetEffectCount()) return @"";
    Effect* e = layer->GetEffect(effectIndex);
    if (!e) return @"";
    const SettingsMap& s = e->GetSettings();
    if (!s.Contains("X_Effect_Description")) return @"";
    return [NSString stringWithUTF8String:s.Get("X_Effect_Description", "").c_str()];
}

- (BOOL)setEffectDescription:(NSString*)description
                        forRow:(int)rowIndex
                       atIndex:(int)effectIndex {
    auto* layer = [self effectLayerForRow:rowIndex];
    if (!layer || effectIndex < 0 || effectIndex >= layer->GetEffectCount()) return NO;
    Effect* e = layer->GetEffect(effectIndex);
    if (!e) return NO;
    std::string v = description ? std::string([description UTF8String]) : std::string();
    SettingsMap& s = e->GetSettings();
    if (v.empty()) {
        s.erase("X_Effect_Description");
    } else {
        s["X_Effect_Description"] = v;
    }
    e->IncrementChangeCount();
    return YES;
}

// B15: replace the entire settings + palette maps of an existing
// effect in one shot. Used by randomize / reset / (future) preset
// apply where the whole property set changes at once. Returns NO
// only on an out-of-range row or effect index; passing a nil /
// empty string for either side clears that side (SetSettings with
// an empty string resets; SetPalette with empty similarly clears
// the palette).
- (BOOL)replaceEffectSettings:(NSString*)settings
                      palette:(NSString*)palette
                        inRow:(int)rowIndex
                      atIndex:(int)effectIndex {
    auto* layer = [self effectLayerForRow:rowIndex];
    if (!layer || effectIndex < 0 || effectIndex >= layer->GetEffectCount()) return NO;
    Effect* e = layer->GetEffect(effectIndex);
    if (!e) return NO;
    std::string st = settings ? std::string([settings UTF8String]) : std::string();
    std::string pal = palette ? std::string([palette UTF8String]) : std::string();
    e->SetSettings(st, true);
    if (!pal.empty()) {
        e->SetPalette(pal);
    }
    e->IncrementChangeCount();
    return YES;
}

- (int)addEffectToRow:(int)rowIndex
                 name:(NSString*)effectName
             settings:(NSString*)settings
              palette:(NSString*)palette
              startMS:(int)startMS
                endMS:(int)endMS {
    auto* layer = [self effectLayerForRow:rowIndex];
    if (!layer) return -1;

    std::string name = [effectName UTF8String];
    std::string st   = settings ? std::string([settings UTF8String]) : std::string();
    std::string pal  = palette  ? std::string([palette  UTF8String]) : std::string();

    Effect* e = layer->AddEffect(0, name, st, pal, startMS, endMS, 0, false);
    if (!e) return -1;
    // AddEffect re-sorts; find the new effect's index by identity.
    for (int i = 0; i < layer->GetEffectCount(); i++) {
        if (layer->GetEffect(i) == e) return i;
    }
    return -1;
}

// MARK: - Controller Output

- (BOOL)startOutput {
    auto& om = _context->GetOutputManager();
    BOOL ok = om.StartOutput();
    // Desktop re-uploads auto-upload-flagged controllers when output
    // is enabled. Mirror that for open-source-firmware controllers;
    // closed-firmware ones are skipped silently (restricted on iPad).
    if (ok) {
        for (auto* c : om.GetControllers()) {
            if (!c || !c->IsActive() || !c->IsAutoUpload()) continue;
            ControllerCaps* caps = ControllerCaps::GetControllerConfig(c);
            if (!caps || !caps->OpenSourceFirmware()) continue;
            NSString* cn = [NSString stringWithUTF8String:c->GetName().c_str()];
            if (caps->SupportsInputOnlyUpload()) {
                [self runUpload:cn input:YES];
            }
            if (caps->SupportsUpload()) {
                [self runUpload:cn input:NO];
            }
        }
    }
    return ok;
}

- (void)stopOutput {
    _context->GetOutputManager().StopOutput();
}

- (NSArray<NSDictionary*>*)globalOutputSettings {
    NSMutableArray<NSDictionary*>* out = [NSMutableArray array];
    if (!_context) return out;
    auto& om = _context->GetOutputManager();

    [out addObject:CtrlBoolProp(@"ControllerSync", @"Controller Sync",
                                  om.IsSyncEnabled() ? YES : NO)];
    if (om.IsSyncEnabled()) {
        [out addObject:CtrlIntProp(@"E131SyncUniverse", @"E1.31 Sync Universe",
                                     om.GetSyncUniverse(), 0, 64000)];
    }
    [out addObject:CtrlIntProp(@"MaxSuppressFrames",
                                 @"Max Duplicate Frames To Suppress",
                                 om.GetSuppressFrames(), 0, 1000)];

    auto const localIPs = ip_utils::GetLocalIPs();
    NSMutableArray<NSString*>* flipOpts = [NSMutableArray array];
    [flipOpts addObject:@""];
    for (const auto& lip : localIPs) {
        [flipOpts addObject:[NSString stringWithUTF8String:lip.c_str()]];
    }
    int flipIdx = IndexOfString(flipOpts, om.GetGlobalForceLocalIP());
    [out addObject:CtrlEnumProp(@"ForceLocalIP", @"Global Force Local IP",
                                  std::max(0, flipIdx), flipOpts)];

    [out addObject:CtrlStringProp(@"GlobalFPPProxy", @"Global FPP Proxy",
                                    [NSString stringWithUTF8String:om.GetGlobalFPPProxy().c_str()],
                                    YES)];
    return out;
}

- (BOOL)setGlobalOutputSetting:(NSString*)key value:(id)value {
    if (!_context || !key) return NO;
    auto& om = _context->GetOutputManager();
    const std::string k = key.UTF8String;
    BOOL changed = NO;

    if (k == "ControllerSync") {
        BOOL v = [(NSNumber*)value boolValue];
        if (om.IsSyncEnabled() != (bool)v) { om.SetSyncEnabled(v); changed = YES; }
    } else if (k == "E131SyncUniverse") {
        int v = [(NSNumber*)value intValue];
        if (om.GetSyncUniverse() != v) { om.SetSyncUniverse(v); changed = YES; }
    } else if (k == "MaxSuppressFrames") {
        int v = [(NSNumber*)value intValue];
        if (om.GetSuppressFrames() != v) { om.SetSuppressFrames(v); changed = YES; }
    } else if (k == "ForceLocalIP") {
        int idx = [(NSNumber*)value intValue];
        auto const localIPs = ip_utils::GetLocalIPs();
        std::string newIP;  // index 0 == "" (no override)
        if (idx > 0) {
            auto it = localIPs.begin();
            std::advance(it, idx - 1);
            if (it != localIPs.end()) newIP = *it;
        }
        if (om.GetGlobalForceLocalIP() != newIP) {
            om.SetGlobalForceLocalIP(newIP); changed = YES;
        }
    } else if (k == "GlobalFPPProxy") {
        if (![value isKindOfClass:[NSString class]]) return NO;
        std::string v = [(NSString*)value UTF8String];
        if (om.GetGlobalFPPProxy() != v) { om.SetGlobalFPPProxy(v); changed = YES; }
    } else {
        return NO;
    }

    if (changed) {
        [self recalcAndMarkControllersDirty];
    }
    return changed;
}

- (BOOL)isOutputting {
    return _context->GetOutputManager().IsOutputting();
}

- (void)outputFrame:(int)frameMS {
    auto& om = _context->GetOutputManager();
    auto& sd = _context->GetSequenceData();
    if (!sd.IsValidData() || !om.IsOutputting()) return;

    int frame = frameMS / sd.FrameTime();
    if (frame < 0 || (unsigned int)frame >= sd.NumFrames()) return;

    om.StartFrame(frameMS);
    auto& fd = sd[frame];
    om.SetManyChannels(0, (unsigned char*)&fd[0], sd.NumChannels());
    om.EndFrame();
}

- (NSInteger)outputCount {
    return _context->GetOutputManager().GetOutputCount();
}

// MARK: - Rendering

- (void)renderAll {
    _context->RenderAll();
}

- (BOOL)isRenderDone {
    return _context->IsRenderDone() ? YES : NO;
}

- (BOOL)wasRenderAborted {
    return _context && _context->WasRenderAborted() ? YES : NO;
}

- (float)renderProgressFraction {
    if (!_context) return 1.0f;
    return _context->GetRenderProgressFraction();
}

- (BOOL)abortRenderAndWait:(NSTimeInterval)timeoutSeconds {
    if (!_context) return YES;
    // Signal every in-flight render job to bail. Workers test the
    // abort flag at their next frame boundary, so this unblocks them
    // within milliseconds for typical sequences.
    _context->AbortRender();
    // Spin-wait on IsRenderDone(). The poll interval is short because
    // we're on the main thread here and want the UI to close promptly,
    // but aborted jobs finish quickly so the expected case is one or
    // two iterations. The timeout is a safety net — we'd rather force
    // a late teardown than hang the app indefinitely on a stuck job.
    NSDate* deadline = [NSDate dateWithTimeIntervalSinceNow:
                        timeoutSeconds > 0 ? timeoutSeconds : 5.0];
    while (!_context->IsRenderDone()) {
        if ([[NSDate date] compare:deadline] == NSOrderedDescending) {
            return NO;
        }
        [NSThread sleepForTimeInterval:0.01];
    }
    return YES;
}

- (void)handleMemoryWarning {
    _context->HandleMemoryWarning();
}

- (void)handleMemoryCritical {
    _context->HandleMemoryCritical();
}

+ (int64_t)availableMemoryMB {
    return (int64_t)(os_proc_available_memory() / (1024 * 1024));
}

- (int)pixelCountAtMS:(int)frameMS {
    auto pixels = _context->GetAllModelPixels(frameMS);
    return (int)pixels.size();
}

- (NSData*)pixelDataAtMS:(int)frameMS {
    auto pixels = _context->GetAllModelPixels(frameMS);
    if (pixels.empty()) return nil;
    return [NSData dataWithBytes:pixels.data() length:pixels.size() * sizeof(iPadRenderContext::PixelData)];
}

// MARK: - Audio Playback

- (AudioManager*)audioManager {
    return _context->GetCurrentMediaManager();
}

- (NSInteger)altTrackCount {
    return _context ? _context->GetAltTrackCount() : 0;
}

- (NSString*)altTrackDisplayNameAtIndex:(NSInteger)index {
    if (!_context) return @"";
    return [NSString stringWithUTF8String:
        _context->GetAltTrackDisplayName((int)index).c_str()];
}

- (NSInteger)activeWaveformTrack {
    return _context ? _context->GetWaveformTrackIndex() : -1;
}

- (void)setActiveWaveformTrack:(NSInteger)index {
    if (_context) _context->SetWaveformTrackIndex((int)index);
}

- (NSString*)altTrackPathAtIndex:(NSInteger)index {
    if (!_context) return @"";
    SequenceFile* sf = _context->GetSequenceFile();
    if (!sf) return @"";
    if (index < 0 || index >= sf->GetAltTrackCount()) return @"";
    return [NSString stringWithUTF8String:sf->GetAltTrack((int)index).path.c_str()];
}

- (NSString*)altTrackShortnameAtIndex:(NSInteger)index {
    if (!_context) return @"";
    SequenceFile* sf = _context->GetSequenceFile();
    if (!sf) return @"";
    if (index < 0 || index >= sf->GetAltTrackCount()) return @"";
    return [NSString stringWithUTF8String:sf->GetAltTrack((int)index).shortname.c_str()];
}

- (BOOL)addAltTrackAtPath:(NSString*)path shortname:(NSString*)shortname {
    if (!_context || !path || path.length == 0) return NO;
    SequenceFile* sf = _context->GetSequenceFile();
    if (!sf) return NO;
    sf->AddAltTrack(_context->GetShowDirectory(),
                     std::string([path UTF8String]),
                     shortname ? std::string([shortname UTF8String]) : std::string());
    _context->GetSequenceElements().IncrementChangeCount(nullptr);
    return YES;
}

- (BOOL)removeAltTrackAtIndex:(NSInteger)index {
    if (!_context) return NO;
    SequenceFile* sf = _context->GetSequenceFile();
    if (!sf) return NO;
    if (index < 0 || index >= sf->GetAltTrackCount()) return NO;
    // If the user removes the alt track currently driving the waveform,
    // fall back to main so the next waveformData call doesn't deref a
    // freed AudioManager.
    int active = _context->GetWaveformTrackIndex();
    if (active == (int)index) _context->SetWaveformTrackIndex(-1);
    else if (active > (int)index) _context->SetWaveformTrackIndex(active - 1);
    sf->RemoveAltTrack((int)index);
    _context->GetSequenceElements().IncrementChangeCount(nullptr);
    return YES;
}

- (BOOL)setAltTrackPathAtIndex:(NSInteger)index path:(NSString*)path {
    if (!_context || !path) return NO;
    SequenceFile* sf = _context->GetSequenceFile();
    if (!sf) return NO;
    if (index < 0 || index >= sf->GetAltTrackCount()) return NO;
    sf->SetAltTrackPath(_context->GetShowDirectory(), (int)index,
                         std::string([path UTF8String]));
    _context->GetSequenceElements().IncrementChangeCount(nullptr);
    return YES;
}

- (BOOL)setAltTrackShortnameAtIndex:(NSInteger)index shortname:(NSString*)shortname {
    if (!_context) return NO;
    SequenceFile* sf = _context->GetSequenceFile();
    if (!sf) return NO;
    if (index < 0 || index >= sf->GetAltTrackCount()) return NO;
    sf->SetAltTrackShortname((int)index,
                              shortname ? std::string([shortname UTF8String]) : std::string());
    _context->GetSequenceElements().IncrementChangeCount(nullptr);
    return YES;
}

- (BOOL)hasAudio {
    auto* am = [self audioManager];
    return am != nullptr && am->IsOk();
}

- (void)audioPlay {
    auto* am = [self audioManager];
    if (am) am->Play();
}

- (void)audioPause {
    auto* am = [self audioManager];
    if (am) am->Pause();
}

- (void)audioStop {
    auto* am = [self audioManager];
    if (am) am->Stop();
}

- (void)audioSeekToMS:(long)positionMS {
    auto* am = [self audioManager];
    if (am) am->Seek(positionMS);
}

- (void)audioPlaySegmentFromMS:(long)positionMS lengthMS:(long)lengthMS {
    auto* am = [self audioManager];
    if (!am || !am->IsOk()) return;
    // Skip when the user is already playing back — the scrub burst
    // would overlap with the live playback engine. The drag-to-scrub
    // flow already pauses on its own through the view model.
    if (am->GetPlayingState() == PLAYING) return;
    if (lengthMS <= 0) return;
    am->Play(positionMS, lengthMS);
}

- (long)audioTellMS {
    auto* am = [self audioManager];
    return am ? am->Tell() : 0;
}

- (int)audioPlayingState {
    auto* am = [self audioManager];
    if (!am) return 2; // STOPPED
    return (int)am->GetPlayingState();
}

- (void)setAudioVolume:(int)volume {
    auto* am = [self audioManager];
    if (am) am->SetVolume(volume);
}

- (int)audioVolume {
    auto* am = [self audioManager];
    return am ? am->GetVolume() : 100;
}

- (void)setAudioPlaybackRate:(float)rate {
    // Static on AudioManager — routes to the current active
    // AVAudioEngineOutput's time-pitch unit on iOS. Safe to call
    // even when no sequence is loaded / no audio manager exists.
    AudioManager::SetPlaybackRate(rate);
}

- (NSData*)waveformDataFromMS:(long)startMS
                         toMS:(long)endMS
                   numSamples:(int)numSamples {
    return [self waveformDataFromMS:startMS toMS:endMS
                         numSamples:numSamples filterType:0
                            lowNote:0 highNote:127];
}

- (NSData*)waveformDataFromMS:(long)startMS
                         toMS:(long)endMS
                   numSamples:(int)numSamples
                   filterType:(int)filterType {
    return [self waveformDataFromMS:startMS toMS:endMS
                         numSamples:numSamples filterType:filterType
                            lowNote:0 highNote:127];
}

- (NSData*)waveformDataFromMS:(long)startMS
                         toMS:(long)endMS
                   numSamples:(int)numSamples
                   filterType:(int)filterType
                      lowNote:(int)lowNote
                     highNote:(int)highNote {
    // B43: route through GetWaveformMedia so an alt-track selection
    // (set via setActiveWaveformTrack:) shows up in the waveform.
    // Playback still uses the main track via [self audioManager].
    auto* am = _context ? _context->GetWaveformMedia() : nullptr;
    if (!am || !am->IsOk() || numSamples <= 0) return nil;

    long rate = am->GetRate();
    long trackSize = am->GetTrackSize();
    long startSample = (startMS * rate) / 1000;
    long endSample = (endMS * rate) / 1000;
    if (startSample < 0) startSample = 0;
    if (endSample > trackSize) endSample = trackSize;
    if (startSample >= endSample) return nil;

    long totalSamples = endSample - startSample;
    long samplesPerBucket = totalSamples / numSamples;
    if (samplesPerBucket < 1) samplesPerBucket = 1;

    // Source-pointer resolution: raw path or filter-specific path.
    // Filter ids map to `AUDIOSAMPLETYPE` (RAW/BASS/TREBLE/ALTO/
    // NONVOCALS in that order, with CUSTOM=5 appended for A9.1's
    // parametric band). `GetFilteredAudioData` may return nullptr when
    // the AudioManager hasn't finished filtering yet (first-time build
    // of that filter) or when the source isn't filter-capable; fall
    // back to raw so the user still sees a waveform.
    float* sourceData = nullptr;
    AUDIOSAMPLETYPE type = AUDIOSAMPLETYPE::RAW;
    int lo = 0, hi = 127;
    switch (filterType) {
        case 1: type = AUDIOSAMPLETYPE::BASS; break;
        case 2: type = AUDIOSAMPLETYPE::TREBLE; break;
        case 3: type = AUDIOSAMPLETYPE::ALTO; break;
        case 4: type = AUDIOSAMPLETYPE::NONVOCALS; break;
        case 5:
            type = AUDIOSAMPLETYPE::CUSTOM;
            lo = std::clamp(lowNote, 0, 127);
            hi = std::clamp(highNote, 0, 127);
            if (hi <= lo) hi = std::min(127, lo + 1);
            break;
        case 6: type = AUDIOSAMPLETYPE::LUFS; break;
        case 7: type = AUDIOSAMPLETYPE::VOCALS; break;
        case 8:  type = AUDIOSAMPLETYPE::STEM_DRUMS; break;
        case 9:  type = AUDIOSAMPLETYPE::STEM_BASS; break;
        case 10: type = AUDIOSAMPLETYPE::STEM_OTHER; break;
        case 11: type = AUDIOSAMPLETYPE::STEM_VOCALS; break;
        default: break;
    }
    if (type != AUDIOSAMPLETYPE::RAW) {
        // Use `EnsureFilteredAudioData` so the cache entry is built on
        // demand for iPad callers — we can't use `SwitchTo` here since
        // it would also overwrite `_pcmdata` / `_data` and change
        // playback. For BASS/TREBLE/etc lowNote=-1 is the "any range"
        // sentinel — the ensure path uses hard-coded defaults per
        // type — while CUSTOM passes explicit lo/hi.
        int qLo = lo, qHi = hi;
        if (type != AUDIOSAMPLETYPE::CUSTOM) {
            qLo = -1; qHi = -1;
        }
        FilteredAudioData* fad = am->EnsureFilteredAudioData(type, qLo, qHi);
        if (fad && fad->data0) {
            sourceData = fad->data0 + startSample;
        }
    }
    if (!sourceData) {
        sourceData = am->GetRawLeftDataPtr(startSample);
        if (!sourceData) return nil;
    }

    // Output: numSamples * 3 floats — {min, max, rms} per bucket. RMS
    // is the A10 peak-vs-RMS overlay data; consumers that only want
    // the peak polygon can ignore the third lane.
    std::vector<float> peaks(numSamples * 3);
    for (int i = 0; i < numSamples; i++) {
        long bucketStart = i * samplesPerBucket;
        long bucketEnd = std::min(bucketStart + samplesPerBucket, totalSamples);
        float mn = 0, mx = 0;
        double sumSq = 0;
        long count = 0;
        for (long s = bucketStart; s < bucketEnd; s++) {
            float v = sourceData[s];
            if (v < mn) mn = v;
            if (v > mx) mx = v;
            sumSq += double(v) * double(v);
            count++;
        }
        peaks[i * 3] = mn;
        peaks[i * 3 + 1] = mx;
        peaks[i * 3 + 2] = count > 0 ? (float)std::sqrt(sumSq / double(count)) : 0.0f;
    }

    return [NSData dataWithBytes:peaks.data() length:peaks.size() * sizeof(float)];
}

- (NSArray<NSNumber*>*)detectOnsetsWithSensitivity:(float)sensitivity {
    auto* am = [self audioManager];
    if (!am || !am->IsOk()) return @[];
    OnsetDetectorOptions opts;
    if (sensitivity > 0) opts.sensitivity = sensitivity;
    std::vector<long> onsets = DetectOnsets(am, opts);
    NSMutableArray<NSNumber*>* arr = [NSMutableArray arrayWithCapacity:onsets.size()];
    for (long ms : onsets) {
        [arr addObject:@(ms)];
    }
    return arr;
}

- (BOOL)ensureSpectrogramComputed {
    auto* am = [self audioManager];
    if (!am || !am->IsOk()) return NO;
    std::string currentHash = am->Hash();
    if (_spectrogram.frames > 0 && _spectrogramAudioHash == currentHash) {
        return YES;
    }
    _spectrogram = ComputeSpectrogram(am);
    _spectrogramAudioHash = currentHash;
    return _spectrogram.frames > 0;
}

- (NSData*)spectrogramBGRAForRangeMS:(long)startMS
                                toMS:(long)endMS
                               width:(int)outWidth
                              height:(int)outHeight {
    if (_spectrogram.frames <= 0) return nil;
    if (outWidth <= 0 || outHeight <= 0) return nil;
    std::vector<uint8_t> buf;
    RenderSpectrogramBGRA(_spectrogram, startMS, endMS, outWidth, outHeight, buf);
    return [NSData dataWithBytes:buf.data() length:buf.size()];
}

- (void)applyPlaybackFilterType:(int)filterType
                          lowNote:(int)lowNote
                         highNote:(int)highNote {
    auto* am = [self audioManager];
    if (!am || !am->IsOk()) return;
    AUDIOSAMPLETYPE type = AUDIOSAMPLETYPE::RAW;
    // `-1` is the "no note-range filtering" sentinel that
    // `EnsureFilteredAudioData` expects for types that don't gate on
    // frequency (RAW, LUFS, NONVOCALS, VOCALS, STEM_*). Only CUSTOM
    // overrides to real MIDI note values.
    int lo = -1, hi = -1;
    switch (filterType) {
        case 1: type = AUDIOSAMPLETYPE::BASS; break;
        case 2: type = AUDIOSAMPLETYPE::TREBLE; break;
        case 3: type = AUDIOSAMPLETYPE::ALTO; break;
        case 4: type = AUDIOSAMPLETYPE::NONVOCALS; break;
        case 5:
            type = AUDIOSAMPLETYPE::CUSTOM;
            lo = std::clamp(lowNote, 0, 127);
            hi = std::clamp(highNote, 0, 127);
            if (hi <= lo) hi = std::min(127, lo + 1);
            break;
        case 6: type = AUDIOSAMPLETYPE::LUFS; break;
        case 7: type = AUDIOSAMPLETYPE::VOCALS; break;
        case 8:  type = AUDIOSAMPLETYPE::STEM_DRUMS; break;
        case 9:  type = AUDIOSAMPLETYPE::STEM_BASS; break;
        case 10: type = AUDIOSAMPLETYPE::STEM_OTHER; break;
        case 11: type = AUDIOSAMPLETYPE::STEM_VOCALS; break;
        default: break;
    }
    // Dispatch off the caller's queue — SwitchTo blocks on the
    // filter cache build, which is multi-second for FIR bandpass
    // filters and longer for stems. Swift callers can fire-and-
    // forget without stalling the main actor.
    dispatch_async(dispatch_get_global_queue(QOS_CLASS_USER_INITIATED, 0), ^{
        am->SwitchTo(type, lo, hi);
    });
}

#pragma mark - A8 stem separation

- (NSArray<NSString*>*)stemModelCandidateRoots {
    NSMutableArray<NSString*>* out = [NSMutableArray array];
    NSString* show = [self showFolderPath];
    if (show.length > 0) [out addObject:show];
    for (NSString* m in [self mediaFolderPaths]) {
        if (m.length > 0) [out addObject:m];
    }
    return out;
}

// Helper: recursively scan `rootDir` for the .mlpackage and lift it
// to `<rootDir>/<modelName>` so the shallow FindModel succeeds on
// subsequent runs. Returns the normalised path, or empty on miss.
static std::string iPadLiftNestedStemModel(const std::string& rootDir) {
    std::error_code ec;
    if (!std::filesystem::exists(rootDir, ec)) return {};
    std::string found;
    for (auto& entry : std::filesystem::recursive_directory_iterator(rootDir, ec)) {
        if (ec) break;
        if (entry.is_directory(ec) &&
            entry.path().filename().string() == AIModelStore::kDemucsModelName) {
            found = entry.path().string();
            break;
        }
    }
    if (found.empty()) return {};
    std::string target = rootDir + "/" + AIModelStore::kDemucsModelName;
    if (found == target) return target;
    std::filesystem::rename(found, target, ec);
    if (ec) return {};
    std::filesystem::path p(found);
    while (p.has_parent_path()) {
        p = p.parent_path();
        if (p.string() == rootDir) break;
        std::error_code rmErr;
        if (!std::filesystem::remove(p, rmErr)) break;
    }
    return target;
}

- (NSString*)findInstalledStemModelPath {
    std::vector<std::string> roots;
    for (NSString* r in [self stemModelCandidateRoots]) {
        roots.push_back([r UTF8String]);
    }
    auto modelDirs = AIModelStore::CandidateModelDirs(roots);
    std::string hit = AIModelStore::FindModel(AIModelStore::kDemucsModelName, modelDirs);
    if (hit.empty()) {
        // Maybe nested from a prior incomplete install — try to lift.
        for (const auto& d : modelDirs) {
            hit = iPadLiftNestedStemModel(d);
            if (!hit.empty()) break;
        }
    }
    if (hit.empty()) return nil;
    return [NSString stringWithUTF8String:hit.c_str()];
}

- (void)installStemModelToRoot:(NSString*)root
                        progress:(void(^)(int pct))progress
                      completion:(void(^)(NSString* installedPath))completion {
    if (root.length == 0) {
        if (completion) dispatch_async(dispatch_get_main_queue(), ^{ completion(nil); });
        return;
    }
    NSString* destDirNS = [root stringByAppendingPathComponent:
        [NSString stringWithUTF8String:AIModelStore::kModelsSubdir]];
    std::string destDir = [destDirNS UTF8String];
    if (!AIModelStore::EnsureDirectory(destDir)) {
        if (completion) dispatch_async(dispatch_get_main_queue(), ^{ completion(nil); });
        return;
    }
    // Security-scoped access for writes.
    ObtainAccessToURL(destDir, /*enforceWritable=*/true);

    NSURL* url = [NSURL URLWithString:
        [NSString stringWithUTF8String:AIModelStore::kDemucsDownloadURL]];
    NSURLSessionConfiguration* cfg = [NSURLSessionConfiguration defaultSessionConfiguration];
    cfg.allowsCellularAccess = NO; // 65 MB download — stay on Wi-Fi by default.
    cfg.waitsForConnectivity = YES;
    NSURLSession* session = [NSURLSession sessionWithConfiguration:cfg];
    NSURLSessionDownloadTask* task =
        [session downloadTaskWithURL:url
                    completionHandler:^(NSURL* tmpLoc, NSURLResponse* resp, NSError* err) {
        if (err || !tmpLoc) {
            NSLog(@"Stem model download failed: %@", err);
            if (completion) dispatch_async(dispatch_get_main_queue(), ^{ completion(nil); });
            return;
        }
        // Move the downloaded tmp file into destDir as the zip.
        NSString* zipPath = [destDirNS stringByAppendingPathComponent:
            @"HTDemucs_SourceSeparation_F32.mlpackage.zip"];
        [[NSFileManager defaultManager] removeItemAtPath:zipPath error:nil];
        NSError* moveErr = nil;
        if (![[NSFileManager defaultManager] moveItemAtURL:tmpLoc
                                                       toURL:[NSURL fileURLWithPath:zipPath]
                                                       error:&moveErr]) {
            NSLog(@"Stem model zip move failed: %@", moveErr);
            if (completion) dispatch_async(dispatch_get_main_queue(), ^{ completion(nil); });
            return;
        }

        // Unzip using minizip. Preserves directory structure —
        // `.mlpackage` is a directory bundle. Progress covers 50..100
        // so the caller sees activity across download (0..50) + unzip.
        unzFile uf = unzOpen([zipPath UTF8String]);
        if (!uf) {
            NSLog(@"unzOpen failed for %@", zipPath);
            if (completion) dispatch_async(dispatch_get_main_queue(), ^{ completion(nil); });
            return;
        }
        unz_global_info gi = {};
        unzGetGlobalInfo(uf, &gi);
        const uLong totalEntries = gi.number_entry;
        uLong idx = 0;
        int ret = unzGoToFirstFile(uf);
        while (ret == UNZ_OK) {
            char nameBuf[1024] = {0};
            unz_file_info fi = {};
            unzGetCurrentFileInfo(uf, &fi, nameBuf, sizeof(nameBuf), nullptr, 0, nullptr, 0);
            std::string entryName = nameBuf;
            std::string outPath = destDir + "/" + entryName;
            // Directory entries end with `/`.
            if (!entryName.empty() && entryName.back() == '/') {
                std::error_code ec;
                std::filesystem::create_directories(outPath, ec);
            } else {
                // Ensure parent dir exists.
                std::error_code ec;
                std::filesystem::create_directories(
                    std::filesystem::path(outPath).parent_path(), ec);
                if (unzOpenCurrentFile(uf) == UNZ_OK) {
                    FILE* out = fopen(outPath.c_str(), "wb");
                    if (out) {
                        std::vector<uint8_t> buf(64 * 1024);
                        int n;
                        while ((n = unzReadCurrentFile(uf, buf.data(), (unsigned)buf.size())) > 0) {
                            fwrite(buf.data(), 1, (size_t)n, out);
                        }
                        fclose(out);
                    }
                    unzCloseCurrentFile(uf);
                }
            }
            idx++;
            if (progress && totalEntries > 0) {
                int pct = 50 + int((idx * 50) / totalEntries);
                if (pct > 100) pct = 100;
                dispatch_async(dispatch_get_main_queue(), ^{ progress(pct); });
            }
            ret = unzGoToNextFile(uf);
        }
        unzClose(uf);
        [[NSFileManager defaultManager] removeItemAtPath:zipPath error:nil];

        // Lift the nested .mlpackage to the canonical path.
        std::string modelPath = iPadLiftNestedStemModel(destDir);
        if (modelPath.empty()) {
            if (completion) dispatch_async(dispatch_get_main_queue(), ^{ completion(nil); });
            return;
        }
        NSString* out = [NSString stringWithUTF8String:modelPath.c_str()];
        if (completion) dispatch_async(dispatch_get_main_queue(), ^{ completion(out); });
    }];
    // Periodic download progress (0..50). Done via a timer-driven
    // poll rather than KVO — NSProgress KVO from ObjC++ would need a
    // subclass for the callback, and a 250 ms tick is plenty for a
    // ~65 MB download.
    if (progress) {
        dispatch_async(dispatch_get_main_queue(), ^{
            // Immediately report 0 so the UI shows the progress bar.
            progress(0);
        });
    }
    // Timer-driven progress poll — fires every 250 ms and reports
    // the download percentage in the 0..50 range (unzip takes 50..100).
    // The block strongly retains `task`; the timer self-cancels as
    // soon as the task transitions to Completed / Canceling.
    dispatch_source_t timer = dispatch_source_create(DISPATCH_SOURCE_TYPE_TIMER,
        0, 0, dispatch_get_global_queue(QOS_CLASS_UTILITY, 0));
    dispatch_source_set_timer(timer,
        dispatch_time(DISPATCH_TIME_NOW, 250 * NSEC_PER_MSEC),
        250 * NSEC_PER_MSEC, 50 * NSEC_PER_MSEC);
    dispatch_source_set_event_handler(timer, ^{
        if (task.state == NSURLSessionTaskStateCompleted ||
            task.state == NSURLSessionTaskStateCanceling) {
            dispatch_source_cancel(timer);
            return;
        }
        double frac = task.progress.fractionCompleted;
        int pct = int(frac * 50.0);
        if (progress) dispatch_async(dispatch_get_main_queue(), ^{ progress(pct); });
    });
    dispatch_resume(timer);
    [task resume];
}

- (void)runStemSeparationAtPath:(NSString*)modelPath
                        progress:(void(^)(int pct))progress
                      completion:(void(^)(BOOL ok))completion {
    auto* am = [self audioManager];
    if (!am || !am->IsOk() || modelPath.length == 0) {
        if (completion) dispatch_async(dispatch_get_main_queue(), ^{ completion(NO); });
        return;
    }
    // Copy the path for use inside the background block.
    NSString* path = [modelPath copy];
    dispatch_async(dispatch_get_global_queue(QOS_CLASS_USER_INITIATED, 0), ^{
        StemOutput stems;
        bool ok = SeparateStems(am, [path UTF8String], stems,
                                  StemSeparatorOptions{},
                                  [progress](int pct) {
                                      if (progress) {
                                          dispatch_async(dispatch_get_main_queue(),
                                                         ^{ progress(pct); });
                                      }
                                  });
        if (ok) {
            am->SetStemData(
                stems.drumsL, stems.drumsR,
                stems.bassL, stems.bassR,
                stems.otherL, stems.otherR,
                stems.vocalsL, stems.vocalsR);
        }
        if (completion) dispatch_async(dispatch_get_main_queue(), ^{ completion(ok ? YES : NO); });
    });
}

- (NSDictionary*)detectChords {
    auto* am = [self audioManager];
    NSMutableDictionary* out = [NSMutableDictionary dictionaryWithCapacity:2];
    out[@"key"] = @"";
    out[@"chords"] = @[];
    if (!am || !am->IsOk()) return out;
    HarmonyAnalysis r = DetectChords(am);
    out[@"key"] = [NSString stringWithUTF8String:r.key.c_str()];
    NSMutableArray* segs = [NSMutableArray arrayWithCapacity:r.chords.size()];
    for (const auto& s : r.chords) {
        [segs addObject:@{
            @"startMS": @(s.startMS),
            @"endMS":   @(s.endMS),
            @"name":    [NSString stringWithUTF8String:s.name.c_str()],
        }];
    }
    out[@"chords"] = segs;
    return out;
}

- (NSData*)detectPitchContour {
    auto* am = [self audioManager];
    if (!am || !am->IsOk()) return nil;
    PitchContour c = DetectPitch(am);
    if (c.samples.empty()) return nil;
    std::vector<float> flat;
    flat.reserve(c.samples.size() * 3);
    for (const auto& s : c.samples) {
        flat.push_back(float(s.timeMS));
        flat.push_back(s.frequency);
        flat.push_back(s.confidence);
    }
    return [NSData dataWithBytes:flat.data()
                           length:flat.size() * sizeof(float)];
}

- (NSDictionary*)detectTempo {
    auto* am = [self audioManager];
    NSMutableDictionary* out = [NSMutableDictionary dictionaryWithCapacity:3];
    if (!am || !am->IsOk()) {
        out[@"bpm"] = @(0);
        out[@"confidence"] = @(0);
        out[@"beats"] = @[];
        return out;
    }
    TempoResult r = DetectTempo(am);
    out[@"bpm"] = @(r.bpm);
    out[@"confidence"] = @(r.confidence);
    NSMutableArray<NSNumber*>* arr = [NSMutableArray arrayWithCapacity:r.beatMS.size()];
    for (long ms : r.beatMS) {
        [arr addObject:@(ms)];
    }
    out[@"beats"] = arr;
    return out;
}

- (NSDictionary*)classifySound {
    auto* am = [self audioManager];
    if (!am || !am->IsOk()) return @{};
    SoundClassification result = ClassifySound(am);
    _lastClassificationTimeStep = result.timeStepSeconds;
    NSMutableDictionary* out = [NSMutableDictionary dictionaryWithCapacity:result.classes.size()];
    for (const auto& c : result.classes) {
        NSMutableArray<NSNumber*>* arr = [NSMutableArray arrayWithCapacity:c.confidence.size()];
        for (float v : c.confidence) {
            [arr addObject:@(v)];
        }
        NSString* key = [NSString stringWithUTF8String:c.name.c_str()];
        out[key] = arr;
    }
    return out;
}

// MARK: - Effect icons (XPM -> BGRA)

namespace {

// Decode "#rrggbb" / "#rrggbbaa" / "None" / "none". Returns true on
// success; colour is returned as 0xAARRGGBB. Named color names (red,
// black, etc.) aren't supported — xLights xpms only use hex and None.
static bool xpmParseColor(const std::string& spec, uint32_t* out) {
    if (spec == "None" || spec == "none") {
        *out = 0x00000000; // fully transparent
        return true;
    }
    if (spec.size() < 7 || spec[0] != '#') return false;
    auto fromHex = [](char c) -> int {
        if (c >= '0' && c <= '9') return c - '0';
        if (c >= 'a' && c <= 'f') return 10 + (c - 'a');
        if (c >= 'A' && c <= 'F') return 10 + (c - 'A');
        return -1;
    };
    if (spec.size() == 7) {
        int r = (fromHex(spec[1]) << 4) | fromHex(spec[2]);
        int g = (fromHex(spec[3]) << 4) | fromHex(spec[4]);
        int b = (fromHex(spec[5]) << 4) | fromHex(spec[6]);
        if (r < 0 || g < 0 || b < 0) return false;
        *out = 0xFF000000u | (uint32_t(r) << 16) | (uint32_t(g) << 8) | uint32_t(b);
        return true;
    }
    if (spec.size() == 9) {
        int r = (fromHex(spec[1]) << 4) | fromHex(spec[2]);
        int g = (fromHex(spec[3]) << 4) | fromHex(spec[4]);
        int b = (fromHex(spec[5]) << 4) | fromHex(spec[6]);
        int a = (fromHex(spec[7]) << 4) | fromHex(spec[8]);
        if (r < 0 || g < 0 || b < 0 || a < 0) return false;
        *out = (uint32_t(a) << 24) | (uint32_t(r) << 16) | (uint32_t(g) << 8) | uint32_t(b);
        return true;
    }
    return false;
}

// Decode an XPM `const char* const*` into BGRA pixel bytes. Returns an
// NSData with width*height*4 bytes, matching CGBitmapContext's expected
// layout for `.bgra8 | .premultipliedFirst` (kCGImageAlphaPremultiplied
// First + byteOrder32Little).
static NSData* xpmToBGRA(const char* const* xpm, int* outW, int* outH) {
    if (!xpm || !xpm[0]) return nil;

    // Header: "width height numColors charsPerPixel [x_hot y_hot]"
    int width = 0, height = 0, numColors = 0, cpp = 0;
    if (sscanf(xpm[0], "%d %d %d %d", &width, &height, &numColors, &cpp) != 4) {
        return nil;
    }
    if (width <= 0 || height <= 0 || numColors <= 0 || cpp <= 0 || cpp > 4) {
        return nil;
    }

    // Build color table keyed on the cpp-character pixel key.
    std::unordered_map<std::string, uint32_t> colorMap;
    colorMap.reserve((size_t)numColors);
    for (int i = 0; i < numColors; i++) {
        const char* line = xpm[1 + i];
        if (!line) return nil;
        std::string l(line);
        if ((int)l.size() < cpp) return nil;
        std::string key = l.substr(0, cpp);
        // After key, XPM has any whitespace, "c", whitespace, color.
        // Find the "c" type token then the value.
        size_t pos = cpp;
        while (pos < l.size() && isspace((unsigned char)l[pos])) pos++;
        // Skip the type char 'c'/'g'/'s'/'m'/'t' and its trailing space.
        // Only 'c' (color) is used by xLights XPMs.
        if (pos >= l.size()) return nil;
        pos++;
        while (pos < l.size() && isspace((unsigned char)l[pos])) pos++;
        std::string color = l.substr(pos);
        uint32_t argb = 0;
        if (!xpmParseColor(color, &argb)) {
            argb = 0x00000000; // unknown -> transparent
        }
        colorMap[key] = argb;
    }

    // Pixels.
    std::vector<uint8_t> bgra((size_t)width * (size_t)height * 4, 0);
    for (int y = 0; y < height; y++) {
        const char* row = xpm[1 + numColors + y];
        if (!row) return nil;
        std::string rowStr(row);
        if ((int)rowStr.size() < width * cpp) return nil;
        for (int x = 0; x < width; x++) {
            std::string key = rowStr.substr((size_t)x * cpp, cpp);
            auto it = colorMap.find(key);
            uint32_t argb = (it != colorMap.end()) ? it->second : 0;
            uint8_t a = (argb >> 24) & 0xFF;
            uint8_t r = (argb >> 16) & 0xFF;
            uint8_t g = (argb >> 8) & 0xFF;
            uint8_t b = argb & 0xFF;
            // Premultiply (Core Graphics with .premultipliedFirst).
            if (a != 255) {
                r = (uint8_t)((uint32_t)r * a / 255);
                g = (uint8_t)((uint32_t)g * a / 255);
                b = (uint8_t)((uint32_t)b * a / 255);
            }
            size_t o = ((size_t)y * (size_t)width + (size_t)x) * 4;
            // byteOrder32Little + .premultipliedFirst: memory layout is
            // B, G, R, A (little-endian read of 0xAARRGGBB).
            bgra[o + 0] = b;
            bgra[o + 1] = g;
            bgra[o + 2] = r;
            bgra[o + 3] = a;
        }
    }

    if (outW) *outW = width;
    if (outH) *outH = height;
    return [NSData dataWithBytes:bgra.data() length:bgra.size()];
}

static int xpmSizeIndexForDesired(int desired) {
    // iconData[0..4] = {16,24,32,48,64}. Round UP so the iPad retina
    // scale still has crisp pixels to sample.
    if (desired <= 16) return 0;
    if (desired <= 24) return 1;
    if (desired <= 32) return 2;
    if (desired <= 48) return 3;
    return 4;
}

} // namespace

- (int)appendEffectBackgroundForRow:(int)rowIndex
                            atIndex:(int)effectIndex
                                 x1:(float)x1
                                 y1:(float)y1
                                 x2:(float)x2
                                 y2:(float)y2
                             bridge:(id)bridge
                          drawRamps:(BOOL)drawRamps {
    if (!bridge || !_context) return 1;
    auto* layer = [self effectLayerForRow:rowIndex];
    if (!layer) return 1;
    if (effectIndex < 0 || effectIndex >= layer->GetEffectCount()) return 1;
    Effect* e = layer->GetEffect(effectIndex);
    if (!e) return 1;
    if (e->GetPaletteSize() == 0) return 1;

    XLGridMetalBridge* b = (XLGridMetalBridge*)bridge;
    auto* accPtr = (xlVertexColorAccumulator*)[b effectBackgroundAccumulator];
    if (!accPtr) return 1;

    RenderableEffect* ef = _context->GetEffectManager()[e->GetEffectIndex()];
    if (!ef) return 1;

    // Mirror EffectsGrid::DrawEffectBackground (EffectsGrid.cpp:6572):
    // channel-block strands get the strand's node mask; single-color /
    // node-single-color models get node 0's mask; everything else
    // passes nullptr (no tint).
    auto* row = _context->GetSequenceElements().GetRowInformation(rowIndex);
    xlColor colorMask = xlColor::NilColor();
    if (row && row->element) {
        Model* m = _context->GetModelManager()[row->element->GetModelName()];
        if (m) {
            if (m->GetDisplayAs() == DisplayAsType::ChannelBlock) {
                auto* se = dynamic_cast<StrandElement*>(row->element);
                colorMask = se ? m->GetNodeMaskColor(se->GetStrand()) : xlWHITE;
            } else {
                const std::string& st = m->GetStringType();
                if (st.rfind("Single Color", 0) == 0 || st == "Node Single Color") {
                    colorMask = m->GetNodeMaskColor(0);
                }
            }
        }
    }
    xlColor* maskPtr = colorMask.IsNilColor() ? nullptr : &colorMask;

    return ef->DrawEffectBackground(e, (int)x1, (int)y1, (int)x2, (int)y2,
                                     *accPtr, maskPtr, drawRamps ? true : false);
}

// MARK: - Media picker helpers

namespace {

NSString* mediaTypeToString(MediaType t) {
    switch (t) {
        case MediaType::Image:      return @"image";
        case MediaType::SVG:        return @"svg";
        case MediaType::Shader:     return @"shader";
        case MediaType::TextFile:   return @"text";
        case MediaType::BinaryFile: return @"binary";
        case MediaType::Video:      return @"video";
        case MediaType::Audio:      return @"audio";
    }
    return @"";
}

std::optional<MediaType> stringToMediaType(NSString* s) {
    if ([s isEqualToString:@"image"])  return MediaType::Image;
    if ([s isEqualToString:@"svg"])    return MediaType::SVG;
    if ([s isEqualToString:@"shader"]) return MediaType::Shader;
    if ([s isEqualToString:@"text"])   return MediaType::TextFile;
    if ([s isEqualToString:@"binary"]) return MediaType::BinaryFile;
    if ([s isEqualToString:@"video"])  return MediaType::Video;
    return std::nullopt;
}

// Locate a cache entry by path scoped to the caller-provided media
// type. Only consults the cache that matches `type`, so we never
// create a stray wrong-type entry — the per-type `Get…` accessors are
// create-on-access, which used to mint an `ImageCacheEntry` for every
// video path the media picker enumerated and then log
// "Error loading image file: …mp4" when the image decoder failed on
// MPEG-4 bytes. `HasMedia` / `HasImage` are non-creating presence
// checks; once we know the path is present, the type-specific
// accessor safely returns the existing entry.
std::shared_ptr<MediaCacheEntry> lookupMediaEntry(SequenceMedia& media,
                                                   const std::string& path,
                                                   MediaType type) {
    switch (type) {
        case MediaType::Image:
            if (!media.HasImage(path)) return nullptr;
            if (auto e = media.GetImage(path))
                return std::static_pointer_cast<MediaCacheEntry>(e);
            return nullptr;
        case MediaType::SVG:
            if (!media.HasMedia(path)) return nullptr;
            if (auto e = media.GetSVG(path))
                return std::static_pointer_cast<MediaCacheEntry>(e);
            return nullptr;
        case MediaType::Shader:
            if (!media.HasMedia(path)) return nullptr;
            if (auto e = media.GetShader(path))
                return std::static_pointer_cast<MediaCacheEntry>(e);
            return nullptr;
        case MediaType::TextFile:
            if (!media.HasMedia(path)) return nullptr;
            if (auto e = media.GetTextFile(path))
                return std::static_pointer_cast<MediaCacheEntry>(e);
            return nullptr;
        case MediaType::BinaryFile:
            if (!media.HasMedia(path)) return nullptr;
            if (auto e = media.GetBinaryFile(path))
                return std::static_pointer_cast<MediaCacheEntry>(e);
            return nullptr;
        case MediaType::Video:
            if (!media.HasMedia(path)) return nullptr;
            if (auto e = media.GetVideo(path))
                return std::static_pointer_cast<MediaCacheEntry>(e);
            return nullptr;
        case MediaType::Audio:
            if (!media.HasMedia(path)) return nullptr;
            if (auto e = media.GetAudio(path))
                return std::static_pointer_cast<MediaCacheEntry>(e);
            return nullptr;
    }
    return nullptr;
}

} // namespace

- (NSArray<NSDictionary<NSString*, NSString*>*>*)mediaPathsInSequence {
    if (!_context) return @[];
    auto& elements = _context->GetSequenceElements();
    auto& media = elements.GetSequenceMedia();
    auto paths = media.GetAllMediaPaths();
    NSMutableArray<NSDictionary<NSString*, NSString*>*>* out =
        [NSMutableArray arrayWithCapacity:paths.size()];
    for (const auto& p : paths) {
        NSString* pathStr = [NSString stringWithUTF8String:p.first.c_str()];
        NSString* typeStr = mediaTypeToString(p.second);
        if (pathStr && typeStr.length) {
            [out addObject:@{@"path": pathStr, @"type": typeStr}];
        }
    }
    return out;
}

// MARK: - Embed / extract (G29)

namespace {

// Called from the Obj-C bridge after a cache mutation that doesn't
// flow through `SequenceElements::IncrementChangeCount` itself
// (embed / extract / remove-unused). Passing nullptr is safe —
// `IncrementChangeCount` only dereferences the `Element*` when the
// caller wants to trigger a dependency re-render for a timing
// element.
inline void bumpSequenceDirty(iPadRenderContext* ctx) {
    if (!ctx || !ctx->IsSequenceLoaded()) return;
    ctx->GetSequenceElements().IncrementChangeCount(nullptr);
}

} // namespace

- (BOOL)embedMediaAtPath:(NSString*)path {
    if (!_context || path.length == 0) return NO;
    auto& media = _context->GetSequenceElements().GetSequenceMedia();
    std::string spath([path UTF8String]);
    if (!media.HasMedia(spath)) return NO;

    // Establish which type the entry is so we can early-out when the
    // caller asks to embed something un-embeddable (video / binary).
    // The inventory is already type-scoped; be defensive anyway.
    auto paths = media.GetAllMediaPaths();
    std::optional<MediaType> type;
    for (const auto& p : paths) {
        if (p.first == spath) { type = p.second; break; }
    }
    if (!type) return NO;
    auto entry = lookupMediaEntry(media, spath, *type);
    if (!entry || !entry->IsEmbeddable() || entry->IsEmbedded()) return NO;

    // Load before embed so the base64 payload is populated from
    // disk. `MediaCacheEntry::Embed` just flips the flag — callers
    // rely on the embeddedData vector being non-empty when Save
    // writes.
    if (!entry->isLoaded()) entry->Load();
    media.EmbedMedia(spath);
    bumpSequenceDirty(_context.get());
    return YES;
}

- (BOOL)extractMediaAtPath:(NSString*)path {
    if (!_context || path.length == 0) return NO;
    auto& media = _context->GetSequenceElements().GetSequenceMedia();
    std::string spath([path UTF8String]);
    if (!media.HasMedia(spath)) return NO;

    auto paths = media.GetAllMediaPaths();
    std::optional<MediaType> type;
    for (const auto& p : paths) {
        if (p.first == spath) { type = p.second; break; }
    }
    if (!type) return NO;
    auto entry = lookupMediaEntry(media, spath, *type);
    if (!entry || !entry->IsEmbedded()) return NO;

    // Resolve the destination so the extracted bytes land next to
    // where the effect expects them. FixFile will pick the first
    // writable location under the show / media folders; falling
    // back to `_filePath` for already-absolute stored paths.
    std::string dest = FileUtils::FixFile("", spath);
    if (dest.empty()) dest = entry->GetFilePath();
    if (dest.empty()) return NO;

    // Desktop's ExtractImageToFile pattern: write the payload out,
    // flip _isEmbedded off, keep the stored path.
    if (!entry->SaveToFile(dest)) return NO;
    media.ExtractMedia(spath);
    bumpSequenceDirty(_context.get());
    return YES;
}

- (int)embedAllMediaOfType:(NSString*)typeFilter {
    if (!_context) return 0;
    auto& media = _context->GetSequenceElements().GetSequenceMedia();
    std::optional<MediaType> onlyType;
    if (typeFilter.length > 0) {
        onlyType = stringToMediaType(typeFilter);
        if (!onlyType) return 0;
    }
    auto paths = media.GetAllMediaPaths();
    int changed = 0;
    for (const auto& p : paths) {
        if (onlyType && p.second != *onlyType) continue;
        auto entry = lookupMediaEntry(media, p.first, p.second);
        if (!entry || !entry->IsEmbeddable() || entry->IsEmbedded()) continue;
        if (!entry->isLoaded()) entry->Load();
        media.EmbedMedia(p.first);
        changed++;
    }
    if (changed > 0) bumpSequenceDirty(_context.get());
    return changed;
}

namespace {

// Walk every effect's settings + palette map and push each value
// into `out`. The media-reference scan uses this to decide which
// cached paths are still in use. False-positive matches (a text
// field with a string that happens to match a cached path) keep
// the entry alive — harmless for cleanup, better than dropping
// something still referenced.
void collectAllEffectSettingValues(iPadRenderContext& ctx,
                                    std::unordered_set<std::string>& out) {
    auto& se = ctx.GetSequenceElements();
    for (size_t i = 0; i < se.GetElementCount(); ++i) {
        Element* el = se.GetElement(i);
        if (!el) continue;
        // All Element types iterate effect layers the same way.
        int nLayers = (int)el->GetEffectLayerCount();
        for (int li = 0; li < nLayers; ++li) {
            EffectLayer* layer = el->GetEffectLayer(li);
            if (!layer) continue;
            int nEffects = (int)layer->GetEffectCount();
            for (int ei = 0; ei < nEffects; ++ei) {
                Effect* eff = layer->GetEffect(ei);
                if (!eff) continue;
                for (const auto& kv : eff->GetSettings()) {
                    out.insert(kv.second);
                }
                for (const auto& kv : eff->GetPaletteMap()) {
                    out.insert(kv.second);
                }
            }
        }
    }
}

// Rewrite every effect's settings + palette-map VALUES equal to
// `oldValue` to `newValue`. Used by the rename path so effects
// tracking the old filename don't end up broken. Returns the
// count of settings touched.
//
// SettingsMap's public `begin()`/`end()` are const-only (they
// return `std::map::const_iterator`), so we can't mutate through a
// range-for. Collect keys in a first pass, then reassign via
// `operator[]` in a second pass.
//
// Per-effect change hooks: settings mutations call
// `Effect::IncrementChangeCount()` (walks up through `EffectLayer`
// to `SequenceElements` and drops the effect's render cache);
// palette mutations go through `PaletteMapUpdated()` which also
// re-derives `mColors`/`mCC` and then calls `IncrementChangeCount`.
// The caller therefore doesn't need a separate sequence-level
// bump when this returns > 0 — the upward propagation handles it.
int rewriteEffectValues(iPadRenderContext& ctx,
                         const std::string& oldValue,
                         const std::string& newValue) {
    if (oldValue == newValue) return 0;
    int changed = 0;
    auto& se = ctx.GetSequenceElements();
    std::vector<std::string> keys;
    for (size_t i = 0; i < se.GetElementCount(); ++i) {
        Element* el = se.GetElement(i);
        if (!el) continue;
        int nLayers = (int)el->GetEffectLayerCount();
        for (int li = 0; li < nLayers; ++li) {
            EffectLayer* layer = el->GetEffectLayer(li);
            if (!layer) continue;
            int nEffects = (int)layer->GetEffectCount();
            for (int ei = 0; ei < nEffects; ++ei) {
                Effect* eff = layer->GetEffect(ei);
                if (!eff) continue;

                keys.clear();
                for (auto it = eff->GetSettings().begin();
                     it != eff->GetSettings().end(); ++it) {
                    if (it->second == oldValue) keys.push_back(it->first);
                }
                bool settingsChanged = !keys.empty();
                for (const auto& k : keys) {
                    eff->GetSettings()[k] = newValue;
                    changed++;
                }
                if (settingsChanged) {
                    // Propagate up the tree (EffectLayer → Element
                    // → SequenceElements) and drop the render
                    // cache so the next frame picks up the new
                    // path.
                    eff->IncrementChangeCount();
                }

                keys.clear();
                for (auto it = eff->GetPaletteMap().begin();
                     it != eff->GetPaletteMap().end(); ++it) {
                    if (it->second == oldValue) keys.push_back(it->first);
                }
                bool paletteChanged = !keys.empty();
                for (const auto& k : keys) {
                    eff->GetPaletteMap()[k] = newValue;
                    changed++;
                }
                if (paletteChanged) {
                    // `PaletteMapUpdated` re-derives `mColors` /
                    // `mCC` and itself calls `IncrementChangeCount`.
                    eff->PaletteMapUpdated();
                }
            }
        }
    }
    return changed;
}

} // namespace

- (BOOL)renameMediaFromPath:(NSString*)oldPath
                      toPath:(NSString*)newPath {
    if (!_context || oldPath.length == 0 || newPath.length == 0) return NO;
    if ([oldPath isEqualToString:newPath]) return NO;
    auto& media = _context->GetSequenceElements().GetSequenceMedia();
    std::string oldStr([oldPath UTF8String]);
    std::string newStr([newPath UTF8String]);

    if (!media.HasMedia(oldStr)) return NO;
    if (media.HasMedia(newStr)) return NO;  // cache-key collision

    // Resolve the entry + its type so we know whether to move on
    // disk. Videos and binaries can't be embedded, so for those
    // the rename always needs a disk move; for images / svgs /
    // shaders / text the disk move is skipped when the entry
    // is embedded (nothing on disk to move).
    auto paths = media.GetAllMediaPaths();
    std::optional<MediaType> type;
    for (const auto& p : paths) {
        if (p.first == oldStr) { type = p.second; break; }
    }
    if (!type) return NO;
    auto entry = lookupMediaEntry(media, oldStr, *type);
    if (!entry) return NO;

    const bool external = !entry->IsEmbedded();
    if (external) {
        // Resolve old + new to absolute on-disk paths. oldResolved
        // must exist (otherwise there's nothing to move); newResolved
        // must NOT exist (otherwise we'd clobber). Both go through
        // FixFile so a show-relative path lands under the show
        // folder.
        std::string oldResolved = FileUtils::FixFile("", oldStr);
        if (oldResolved.empty()) oldResolved = entry->GetFilePath();
        if (oldResolved.empty() || !FileExists(oldResolved)) return NO;

        // For the destination, resolve via FixFile first (handles
        // a relative target under the show folder). If that
        // returns the raw path unchanged (meaning not under
        // show/media) treat the input as absolute-as-given.
        std::string newResolved = FileUtils::FixFile("", newStr);
        if (newResolved.empty()) newResolved = newStr;

        // If the target's parent directory doesn't exist, create
        // it — renaming `Images/foo.png` → `Images/Renamed/bar.png`
        // should succeed even if `Images/Renamed/` hasn't been
        // used yet.
        std::filesystem::path newResPath(newResolved);
        if (newResPath.has_parent_path()) {
            std::error_code ec;
            std::filesystem::create_directories(newResPath.parent_path(), ec);
        }

        if (FileExists(newResolved)) return NO; // disk collision

        ObtainAccessToURL(oldResolved, true);
        ObtainAccessToURL(newResolved, true);

        std::error_code ec;
        std::filesystem::rename(oldResolved, newResolved, ec);
        if (ec) {
            // Fall back to copy + remove for cross-filesystem
            // rename cases (rename fails with EXDEV when the
            // destination is on a different volume).
            ec.clear();
            std::filesystem::copy_file(oldResolved, newResolved,
                std::filesystem::copy_options::overwrite_existing, ec);
            if (ec) return NO;
            std::error_code ec2;
            std::filesystem::remove(oldResolved, ec2);
            // Swallow the remove error — the copy succeeded, so
            // the data is preserved; leaving a stray source is
            // strictly better than losing it.
        }
    }

    // Cache re-key works for every type via the generic helper.
    if (!media.RenameMedia(oldStr, newStr)) {
        // On failure we've already moved the file on disk for
        // externals. Try to roll back to preserve the original
        // state.
        if (external) {
            std::string oldResolved = FileUtils::FixFile("", oldStr);
            std::string newResolved = FileUtils::FixFile("", newStr);
            if (newResolved.empty()) newResolved = newStr;
            if (!oldResolved.empty() && !newResolved.empty()) {
                std::error_code ec;
                std::filesystem::rename(newResolved, oldResolved, ec);
            }
        }
        return NO;
    }

    // Rewrite every effect whose settings reference oldPath.
    // `rewriteEffectValues` propagates the dirty + cache-drop
    // hooks per effect; a final explicit bump covers the
    // no-referencing-effect edge case.
    (void)rewriteEffectValues(*_context, oldStr, newStr);
    bumpSequenceDirty(_context.get());
    return YES;
}

namespace {

// Canonical show-folder subdirectory for a given MediaType. Matches
// what desktop conventionally drops into per type (`Images/`,
// `Shaders/`, `Videos/`) and keeps iPad's relocation parity clean.
const char* canonicalSubdirForType(MediaType t) {
    switch (t) {
        case MediaType::Image:      return "Images";
        case MediaType::SVG:        return "Images";
        case MediaType::Shader:     return "Shaders";
        case MediaType::Video:      return "Videos";
        case MediaType::TextFile:   return "Text";
        case MediaType::BinaryFile: return "Other";
        case MediaType::Audio:      return "Audio";
    }
    return "";
}

} // namespace

- (NSString*)replaceMissingMediaAtPath:(NSString*)storedPath
                        fromSourcePath:(NSString*)sourcePath {
    if (!_context || !_context->IsSequenceLoaded()) return nil;
    if (storedPath.length == 0 || sourcePath.length == 0) return nil;

    auto& media = _context->GetSequenceElements().GetSequenceMedia();
    std::string storedStr([storedPath UTF8String]);
    std::string srcStr([sourcePath UTF8String]);

    if (!media.HasMedia(storedStr)) return nil;

    // Establish the entry's media type so we can pick a target
    // subdirectory when the stored path doesn't suggest one.
    auto paths = media.GetAllMediaPaths();
    std::optional<MediaType> type;
    for (const auto& p : paths) {
        if (p.first == storedStr) { type = p.second; break; }
    }
    if (!type) return nil;

    // Derive the target subdirectory. Preference order:
    //   1. If the stored path is a show-relative form with a
    //      parent directory (e.g. `Images/Snow/flake.png`), reuse
    //      that parent so the replacement lands in the same folder
    //      the effect originally referenced.
    //   2. Otherwise use a canonical per-type subdir (`Images/`,
    //      `Shaders/`, `Videos/`, …).
    std::string subdir;
    {
        std::filesystem::path storedFs(storedStr);
        if (storedFs.is_relative() && storedFs.has_parent_path()) {
            subdir = storedFs.parent_path().string();
        } else {
            subdir = canonicalSubdirForType(*type);
        }
    }

    // Security-scoped access on the source so the copy below can
    // read it (document-picker URLs are sandboxed until
    // `startAccessingSecurityScopedResource` is active on the
    // Swift side; the caller must have already done that).
    ObtainAccessToURL(srcStr, /*enforceWritable=*/false);

    // Copy the picked source into `<showDir>/<subdir>/<basename>`,
    // appending `_N` on collision. Returns the destination absolute
    // path, empty on failure. `reuse=false` because the broken
    // entry's file is missing — there's no matching-byte file to
    // reuse anyway.
    std::string absDest = _context->MoveToShowFolder(srcStr, subdir, /*reuse*/ false);
    if (absDest.empty()) return nil;

    // Convert back to show-relative so the stored path stays
    // portable. `MakeRelativePath` falls through unchanged if the
    // path isn't under the show folder, but `MoveToShowFolder`
    // always places the copy there.
    std::string newStr = _context->MakeRelativePath(absDest);
    if (newStr.empty()) newStr = absDest;

    if (newStr == storedStr) {
        // Common case: the replacement happens to match the stored
        // path exactly (same basename, same parent dir). Just
        // re-read the entry from disk; no settings map rewrite
        // needed.
        (void)media.ReloadMedia(storedStr);
        bumpSequenceDirty(_context.get());
    } else {
        // Different target path — re-key the cache entry, then
        // walk every effect's settings + palette maps to rewrite
        // references. The entry's _resolvedPath cache is stale
        // now, so reload the new key from disk as well.
        if (!media.RenameMedia(storedStr, newStr)) {
            // Rare: either the old path vanished between checks or
            // the new path already exists in another cache. Fall
            // back to leaving the copy in place — the user can
            // manually re-point the effect from the picker.
            return [NSString stringWithUTF8String:absDest.c_str()];
        }
        (void)media.ReloadMedia(newStr);
        (void)rewriteEffectValues(*_context, storedStr, newStr);
        bumpSequenceDirty(_context.get());
    }

    return [NSString stringWithUTF8String:newStr.c_str()];
}

- (NSString*)videoCompatibilityIssueForPath:(NSString*)path {
    if (path.length == 0) return nil;
    // Resolve via FixFile so iCloud / show-relative paths map onto
    // a real on-disk path AVFoundation can open. Obtain security-
    // scoped access before the probe so sandboxed destinations
    // (iCloud Drive) don't trip an access failure that would look
    // like an incompatibility.
    std::string raw([path UTF8String]);
    std::string resolved = FileUtils::FixFile("", raw);
    if (resolved.empty()) resolved = raw;
    ObtainAccessToURL(resolved, false);

    std::string reason = MediaCompatibility::CheckVideoFile(resolved);
    if (reason.empty()) return nil;
    return [NSString stringWithUTF8String:reason.c_str()];
}

namespace {

// iPad-side callbacks for SequenceChecker. Per-check disable flags are
// read back from the render context (populated from the CheckSequence
// sheet's @AppStorage toggles); render-cache "Enabled" matches the
// iPad's lack of that setting; the AVFoundation video probe and
// optional progress block are the remaining overrides.
class iPadSequenceCheckerCallbacks final : public SequenceCheckerCallbacks {
public:
    using ProgressBlock = void (^)(int, NSString*);

    iPadSequenceCheckerCallbacks(ProgressBlock progress,
                                 const iPadRenderContext* ctx)
        : _progress(progress ? [progress copy] : nil), _ctx(ctx) {}

    bool IsCheckOptionDisabled(const std::string& option) const override {
        return _ctx && _ctx->IsCheckOptionDisabled(option);
    }

    std::string CheckVideoCompatibility(const std::string& path) override {
        if (path.empty()) return "";
        std::string resolved = FileUtils::FixFile("", path);
        if (resolved.empty()) resolved = path;
        ObtainAccessToURL(resolved, false);
        return MediaCompatibility::CheckVideoFile(resolved);
    }

    void OnProgress(int percent, const std::string& step) override {
        if (!_progress) return;
        _progress(percent,
                   [NSString stringWithUTF8String:step.c_str()]);
    }

private:
    ProgressBlock _progress;
    const iPadRenderContext* _ctx;
};

XLCheckSequenceSeverity SeverityFor(CheckSequenceReport::ReportIssue::Type t) {
    switch (t) {
        case CheckSequenceReport::ReportIssue::CRITICAL:
            return XLCheckSequenceSeverityCritical;
        case CheckSequenceReport::ReportIssue::WARNING:
            return XLCheckSequenceSeverityWarning;
        case CheckSequenceReport::ReportIssue::INFO:
        default:
            return XLCheckSequenceSeverityInfo;
    }
}

NSString* TrimWhitespacePrefix(const std::string& s) {
    // Desktop's check messages historically lead with `    ERR: ` /
    // `    WARN: ` / `    INFO: ` indents inherited from the wxFile
    // text-mode output. The HTML rendering strips these via
    // `CleanMessage`; mirror that here so the iPad sheet shows
    // tidy strings without re-implementing the cleaner.
    size_t i = 0;
    while (i < s.size() && std::isspace(static_cast<unsigned char>(s[i]))) ++i;
    static constexpr const char* prefixes[] = {"INFO: ", "WARN: ", "ERR: "};
    for (const char* p : prefixes) {
        size_t plen = std::strlen(p);
        if (s.compare(i, plen, p) == 0) { i += plen; break; }
    }
    return [NSString stringWithUTF8String:s.c_str() + i];
}

NSString* OptionalString(const std::string& s) {
    return s.empty() ? nil : [NSString stringWithUTF8String:s.c_str()];
}

}  // namespace

- (void)setCheckSequenceDisabledOptions:(NSArray<NSString*>*)options {
    if (!_context) return;
    std::set<std::string> disabled;
    for (NSString* opt in options) {
        if (opt.length) disabled.insert(opt.UTF8String);
    }
    _context->SetDisabledCheckOptions(disabled);
}

- (NSArray<XLCheckSequenceIssue*>*)runSequenceCheckWithProgress:
    (void (^)(int, NSString*))progress {
    NSMutableArray<XLCheckSequenceIssue*>* issues = [NSMutableArray array];
    if (!_context || !_context->IsSequenceLoaded()) return issues;

    iPadSequenceCheckerCallbacks callbacks(progress, _context.get());
    SequenceChecker checker(_context->GetSequenceElements(),
                             _context->GetModelManager(),
                             _context->GetOutputManager(),
                             _context->GetSequenceFile(),
                             _context->GetShowDirectory(),
                             &callbacks);

    CheckSequenceReport report;
    for (const auto& section : CheckSequenceReport::REPORT_SECTIONS) {
        report.AddSection(section);
    }
    report.SetShowFolder(_context->GetShowDirectory());
    if (auto* sf = _context->GetSequenceFile()) {
        report.SetSequencePath(sf->GetFullPath());
    }
    checker.RunFullCheck(report);

    // Flatten the structured report into the iPad value class. Both
    // sides preserve location data so SwiftUI can offer tap-to-jump
    // when (modelName, effectName, startTimeMS) is populated.
    for (const auto& section : report.GetSections()) {
        NSString* sectionID =
            [NSString stringWithUTF8String:section.id.c_str()];
        NSString* sectionTitle =
            [NSString stringWithUTF8String:section.title.c_str()];
        for (const auto& iss : section.issues) {
            // Skip the per-section "checkdisabled" placeholders that
            // desktop renders as muted footer text — iPad has no
            // equivalent settings UI so they'd be confusing.
            if (iss.category == "checkdisabled") continue;
            XLCheckSequenceIssue* out =
                [[XLCheckSequenceIssue alloc]
                    initWithSeverity:SeverityFor(iss.type)
                            sectionID:sectionID
                         sectionTitle:sectionTitle
                             category:[NSString stringWithUTF8String:iss.category.c_str()]
                              message:TrimWhitespacePrefix(iss.message)
                            modelName:OptionalString(iss.modelName)
                           effectName:OptionalString(iss.effectName)
                          startTimeMS:iss.startTimeMS
                           layerIndex:iss.layerIndex];
            [issues addObject:out];
        }
    }
    return issues;
}

namespace {
std::string toLowerStr(const std::string& s) {
    std::string out(s);
    std::transform(out.begin(), out.end(), out.begin(),
                   [](unsigned char c) { return (char)std::tolower(c); });
    return out;
}

// Find the first "key=value" entry (settings then palette) whose
// lower-cased form contains `needleLower`. Returns "" when nothing
// matches (or when `needleLower` is empty, which the caller treats as
// "any" and never calls here).
std::string matchedSettingEntry(Effect* eff, const std::string& needleLower) {
    for (const auto& kv : eff->GetSettings()) {
        std::string entry = kv.first + "=" + kv.second;
        if (toLowerStr(entry).find(needleLower) != std::string::npos) return entry;
    }
    for (const auto& kv : eff->GetPaletteMap()) {
        std::string entry = kv.first + "=" + kv.second;
        if (toLowerStr(entry).find(needleLower) != std::string::npos) return entry;
    }
    return "";
}

// Append every matching effect on `layer` (belonging to display
// element `elementName` whose parent model is `modelName`) to `out`.
void appendLayerMatches(EffectLayer* layer,
                        const std::string& modelName,
                        const std::string& elementName,
                        const std::string& typeLower,
                        const std::string& settingsLower,
                        NSInteger maxResults,
                        NSMutableArray<XLFindEffectResult*>* out) {
    if (!layer) return;
    int layerNum = layer->GetLayerNumber();
    int nEffects = (int)layer->GetEffectCount();
    for (int ei = 0; ei < nEffects && (NSInteger)out.count < maxResults; ++ei) {
        Effect* eff = layer->GetEffect(ei);
        if (!eff) continue;
        if (!typeLower.empty() && toLowerStr(eff->GetEffectName()) != typeLower) continue;
        std::string matched;
        if (!settingsLower.empty()) {
            matched = matchedSettingEntry(eff, settingsLower);
            if (matched.empty()) continue;
        }
        XLFindEffectResult* r = [[XLFindEffectResult alloc]
            initWithModelName:[NSString stringWithUTF8String:modelName.c_str()]
                  elementName:[NSString stringWithUTF8String:elementName.c_str()]
                   effectName:[NSString stringWithUTF8String:eff->GetEffectName().c_str()]
                   layerIndex:layerNum
                  startTimeMS:eff->GetStartTimeMS()
               matchedSetting:[NSString stringWithUTF8String:matched.c_str()]];
        [out addObject:r];
    }
}
} // namespace

- (NSArray<XLFindEffectResult*>*)findEffectsMatchingType:(NSString*)effectType
                                            settingsText:(NSString*)settingsText
                                             modelFilter:(NSString*)modelFilter
                                              maxResults:(NSInteger)maxResults {
    NSMutableArray<XLFindEffectResult*>* out = [NSMutableArray array];
    if (!_context || !_context->IsSequenceLoaded()) return out;
    if (maxResults <= 0) maxResults = 5000;

    const std::string typeLower = toLowerStr(std::string(effectType.UTF8String));
    const std::string settingsLower = toLowerStr(std::string(settingsText.UTF8String));
    const std::string modelLower = toLowerStr(std::string(modelFilter.UTF8String));

    auto& se = _context->GetSequenceElements();
    for (size_t i = 0; i < se.GetElementCount() && (NSInteger)out.count < maxResults; ++i) {
        Element* el = se.GetElement(i);
        if (!el || el->GetType() == ElementType::ELEMENT_TYPE_TIMING) continue;
        const std::string modelName = el->GetModelName();
        if (!modelLower.empty() && toLowerStr(modelName).find(modelLower) == std::string::npos) continue;

        // The model element's own layers.
        for (int li = 0; li < (int)el->GetEffectLayerCount() && (NSInteger)out.count < maxResults; ++li) {
            appendLayerMatches(el->GetEffectLayer(li), modelName, el->GetName(),
                               typeLower, settingsLower, maxResults, out);
        }

        // Submodels + strands (and their node layers) — mirrors
        // desktop SearchPanel coverage.
        if (el->GetType() == ElementType::ELEMENT_TYPE_MODEL) {
            ModelElement* me = dynamic_cast<ModelElement*>(el);
            if (!me) continue;
            for (int x = 0; x < me->GetSubModelAndStrandCount() && (NSInteger)out.count < maxResults; ++x) {
                SubModelElement* sme = me->GetSubModel(x);
                if (!sme) continue;
                for (int li = 0; li < (int)sme->GetEffectLayerCount() && (NSInteger)out.count < maxResults; ++li) {
                    appendLayerMatches(sme->GetEffectLayer(li), modelName, sme->GetName(),
                                       typeLower, settingsLower, maxResults, out);
                }
                if (StrandElement* str = dynamic_cast<StrandElement*>(sme)) {
                    for (int nl = 0; nl < str->GetNodeLayerCount() && (NSInteger)out.count < maxResults; ++nl) {
                        appendLayerMatches(str->GetNodeEffectLayer(nl), modelName, sme->GetName(),
                                           typeLower, settingsLower, maxResults, out);
                    }
                }
            }
        }
    }
    return out;
}

- (NSArray<NSString*>*)effectTypesInSequence {
    NSMutableArray<NSString*>* out = [NSMutableArray array];
    if (!_context || !_context->IsSequenceLoaded()) return out;
    std::set<std::string> names;
    auto& se = _context->GetSequenceElements();
    for (size_t i = 0; i < se.GetElementCount(); ++i) {
        Element* el = se.GetElement(i);
        if (!el || el->GetType() == ElementType::ELEMENT_TYPE_TIMING) continue;
        for (int li = 0; li < (int)el->GetEffectLayerCount(); ++li) {
            EffectLayer* layer = el->GetEffectLayer(li);
            if (!layer) continue;
            for (int ei = 0; ei < (int)layer->GetEffectCount(); ++ei) {
                if (Effect* eff = layer->GetEffect(ei)) names.insert(eff->GetEffectName());
            }
        }
    }
    for (const auto& n : names) {
        if (!n.empty()) [out addObject:[NSString stringWithUTF8String:n.c_str()]];
    }
    return out;
}

- (NSArray<NSDictionary<NSString*, NSString*>*>*)userLyricDictionaryEntries {
    NSMutableArray<NSDictionary<NSString*, NSString*>*>* out = [NSMutableArray array];
    if (!_context) return out;
    const std::string& showDir = _context->GetShowDirectory();
    if (showDir.empty()) return out;
    std::string path = showDir + "/user_dictionary";
    ObtainAccessToURL(path, /*enforceWritable=*/false);
    std::ifstream in(path);
    if (!in.is_open()) return out;
    std::string line;
    while (std::getline(in, line)) {
        // Strip trailing CR (files authored on the desktop may be CRLF).
        if (!line.empty() && line.back() == '\r') line.pop_back();
        if (line.empty() || line[0] == '#') continue;
        std::istringstream iss(line);
        std::string word;
        if (!(iss >> word)) continue;
        std::string rest, tok;
        bool first = true;
        while (iss >> tok) {
            if (!first) rest += " ";
            rest += tok;
            first = false;
        }
        [out addObject:@{
            @"word": [NSString stringWithUTF8String:word.c_str()],
            @"phonemes": [NSString stringWithUTF8String:rest.c_str()]
        }];
    }
    return out;
}

- (BOOL)saveUserLyricDictionaryEntries:(NSArray<NSDictionary<NSString*, NSString*>*>*)entries {
    if (!_context) return NO;
    const std::string& showDir = _context->GetShowDirectory();
    if (showDir.empty()) return NO;
    std::string path = showDir + "/user_dictionary";
    ObtainAccessToURL(showDir, /*enforceWritable=*/true);
    ObtainAccessToURL(path, /*enforceWritable=*/true);
    std::ofstream f(path, std::ios::trunc);
    if (!f.is_open()) return NO;
    for (NSDictionary<NSString*, NSString*>* e in entries) {
        NSString* w = e[@"word"];
        NSString* p = e[@"phonemes"];
        if (w.length == 0) continue;
        // Desktop LyricUserDictDialog uppercases both the word and the
        // phoneme list before writing; the core lookup uppercases the
        // query word, so the entries must be stored uppercase to match.
        std::string word = std::string([[w uppercaseString] UTF8String]);
        std::string phon = p ? std::string([[p uppercaseString] UTF8String]) : "";
        f << word;
        if (!phon.empty()) f << " " << phon;
        f << "\n";
    }
    f.close();
    // Force the live dictionary to re-read on next use so the edits
    // affect lyric breakdown immediately.
    _context->ReloadPhonemeDictionary();
    return YES;
}

- (int)removeUnusedMedia {
    if (!_context) return 0;
    auto& media = _context->GetSequenceElements().GetSequenceMedia();

    // Collect every value any effect refers to — the "used" set.
    std::unordered_set<std::string> usedValues;
    collectAllEffectSettingValues(*_context, usedValues);

    auto paths = media.GetAllMediaPaths();
    int removed = 0;
    for (const auto& p : paths) {
        if (usedValues.count(p.first) == 0) {
            media.RemoveMedia(p.first);
            removed++;
        }
    }
    if (removed > 0) bumpSequenceDirty(_context.get());
    return removed;
}

- (BOOL)removeMediaAtPath:(NSString*)path {
    // MED-5: forget one embedded/cached media entry by its stored path/value
    // (the same key GetAllMediaPaths / effect settings use), even if still
    // referenced — the user re-sources it or re-imports. Mirrors removeUnusedMedia.
    if (!_context || path.length == 0) return NO;
    auto& media = _context->GetSequenceElements().GetSequenceMedia();
    media.RemoveMedia(std::string([path UTF8String]));
    bumpSequenceDirty(_context.get());
    return YES;
}

namespace {

// Extension → MediaType, wx-free mirror of ManageMediaPanel's
// `MediaTypeFromPath`. Image is the fallback for everything not
// otherwise recognised (jpg / png / gif / webp / bmp / …).
MediaType mediaTypeFromExtension(const std::string& path) {
    std::string ext = std::filesystem::path(path).extension().string();
    if (!ext.empty() && ext.front() == '.') ext.erase(ext.begin());
    std::transform(ext.begin(), ext.end(), ext.begin(),
                   [](unsigned char c) { return std::tolower(c); });
    if (ext == "fs") return MediaType::Shader;
    if (ext == "svg") return MediaType::SVG;
    if (ext == "txt") return MediaType::TextFile;
    if (ext == "gled" || ext == "out" || ext == "csv") return MediaType::BinaryFile;
    if (ext == "avi" || ext == "mp4" || ext == "mkv" || ext == "mov" ||
        ext == "asf" || ext == "flv" || ext == "mpg" || ext == "mpeg" ||
        ext == "m4v" || ext == "wmv") return MediaType::Video;
    if (ext == "mp3" || ext == "ogg" || ext == "m4a" || ext == "wav" ||
        ext == "flac" || ext == "aac" || ext == "wma") return MediaType::Audio;
    return MediaType::Image;
}

// Register `path` in the media cache by forcing a fresh type-scoped
// entry. Returns true when an entry didn't already exist.
bool registerMediaPath(SequenceMedia& media, const std::string& path,
                       MediaType type) {
    if (media.HasMedia(path)) return false;
    std::string resolved = FileUtils::FixFile("", path);
    if (resolved.empty()) resolved = path;
    media.ForceRefreshEntry(path, resolved, type);
    return true;
}

} // namespace

- (BOOL)addMediaAtPath:(NSString*)storedPath {
    if (!_context || !_context->IsSequenceLoaded() || storedPath.length == 0) {
        return NO;
    }
    auto& media = _context->GetSequenceElements().GetSequenceMedia();
    std::string p([storedPath UTF8String]);
    if (!registerMediaPath(media, p, mediaTypeFromExtension(p))) return NO;
    bumpSequenceDirty(_context.get());
    return YES;
}

- (BOOL)reloadMediaAtPath:(NSString*)path {
    if (!_context || path.length == 0) return NO;
    auto& media = _context->GetSequenceElements().GetSequenceMedia();
    return media.ReloadMedia(std::string([path UTF8String])) ? YES : NO;
}

- (int)reloadAllMedia {
    if (!_context) return 0;
    auto& media = _context->GetSequenceElements().GetSequenceMedia();
    int reloaded = 0;
    for (const auto& p : media.GetAllMediaPaths()) {
        if (media.ReloadMedia(p.first)) reloaded++;
    }
    return reloaded;
}

namespace {

// Shared sweep used by the cleanup preview + execute paths. Walks
// the media inventory; for every external (non-embedded) entry whose
// resolved on-disk file lives OUTSIDE the show / media folders,
// computes the show-relative destination it would move to. When
// `execute` is true the file is copied into the show folder and
// every effect reference is rewritten; otherwise nothing is touched.
// `moves` (optional) collects {from, to} pairs for the preview.
int cleanupExternalMedia(iPadRenderContext& ctx, bool execute,
                         NSMutableArray<NSDictionary<NSString*, NSString*>*>* moves) {
    auto& media = ctx.GetSequenceElements().GetSequenceMedia();
    int moved = 0;
    // Snapshot the path list up front — executing mutates the cache.
    auto paths = media.GetAllMediaPaths();
    for (const auto& p : paths) {
        const std::string& stored = p.first;
        auto entry = lookupMediaEntry(media, stored, p.second);
        if (!entry || entry->IsEmbedded()) continue;

        std::string resolved = FileUtils::FixFile("", stored);
        if (resolved.empty()) resolved = entry->GetFilePath();
        if (resolved.empty() || !FileExists(resolved)) continue;
        if (ctx.IsInShowOrMediaFolder(resolved)) continue;

        std::string subdir = canonicalSubdirForType(p.second);
        if (!execute) {
            std::string basename = std::filesystem::path(resolved).filename().string();
            std::string proposed = subdir.empty()
                ? basename : (subdir + "/" + basename);
            if (moves) {
                [moves addObject:@{
                    @"from": [NSString stringWithUTF8String:stored.c_str()],
                    @"to":   [NSString stringWithUTF8String:proposed.c_str()],
                }];
            }
            moved++;
            continue;
        }

        ObtainAccessToURL(resolved, false);
        std::string absDest = ctx.MoveToShowFolder(resolved, subdir, /*reuse*/ true);
        if (absDest.empty()) continue;
        std::string newStr = ctx.MakeRelativePath(absDest);
        if (newStr.empty()) newStr = absDest;
        if (newStr == stored) { moved++; continue; }
        if (!media.RenameMedia(stored, newStr)) continue;
        (void)media.ReloadMedia(newStr);
        (void)rewriteEffectValues(ctx, stored, newStr);
        moved++;
    }
    return moved;
}

} // namespace

- (NSArray<NSDictionary<NSString*, NSString*>*>*)cleanupFileLocationsPreview {
    NSMutableArray<NSDictionary<NSString*, NSString*>*>* moves =
        [NSMutableArray array];
    if (!_context || !_context->IsSequenceLoaded()) return moves;
    (void)cleanupExternalMedia(*_context, /*execute*/ false, moves);
    return moves;
}

- (int)performCleanupFileLocations {
    if (!_context || !_context->IsSequenceLoaded()) return 0;
    int moved = cleanupExternalMedia(*_context, /*execute*/ true, nil);
    if (moved > 0) bumpSequenceDirty(_context.get());
    return moved;
}

- (int)extractAllMediaOfType:(NSString*)typeFilter {
    if (!_context) return 0;
    auto& media = _context->GetSequenceElements().GetSequenceMedia();
    std::optional<MediaType> onlyType;
    if (typeFilter.length > 0) {
        onlyType = stringToMediaType(typeFilter);
        if (!onlyType) return 0;
    }
    auto paths = media.GetAllMediaPaths();
    int changed = 0;
    for (const auto& p : paths) {
        if (onlyType && p.second != *onlyType) continue;
        auto entry = lookupMediaEntry(media, p.first, p.second);
        if (!entry || !entry->IsEmbedded()) continue;
        std::string dest = FileUtils::FixFile("", p.first);
        if (dest.empty()) dest = entry->GetFilePath();
        if (dest.empty()) continue;
        if (!entry->SaveToFile(dest)) continue;
        media.ExtractMedia(p.first);
        changed++;
    }
    if (changed > 0) bumpSequenceDirty(_context.get());
    return changed;
}

- (NSArray<NSDictionary<NSString*, id>*>*)mediaInventoryInSequence {
    if (!_context) return @[];
    auto& media = _context->GetSequenceElements().GetSequenceMedia();
    auto paths = media.GetAllMediaPaths();
    NSMutableArray<NSDictionary<NSString*, id>*>* out =
        [NSMutableArray arrayWithCapacity:paths.size()];

    for (const auto& p : paths) {
        NSString* pathStr = [NSString stringWithUTF8String:p.first.c_str()];
        NSString* typeStr = mediaTypeToString(p.second);
        if (!pathStr || typeStr.length == 0) continue;

        // Per-type entry lookup — use the type-scoped cache so we
        // don't create stray wrong-type entries (same pattern as
        // the thumbnail path).
        auto entry = lookupMediaEntry(media, p.first, p.second);
        if (!entry) continue;

        BOOL embedded = entry->IsEmbedded() ? YES : NO;

        // Resolve via FixFile for the on-disk existence check.
        // `VideoMediaCacheEntry` caches its resolved path; for
        // everything else run FixFile fresh each call (cheap — it
        // hits the FileExists short-circuit when the raw path is
        // already valid).
        std::string resolved;
        if (p.second == MediaType::Video) {
            auto ve = std::static_pointer_cast<VideoMediaCacheEntry>(entry);
            if (!ve->isLoaded()) ve->Load();
            resolved = ve->GetResolvedPath();
        }
        if (resolved.empty()) {
            resolved = FileUtils::FixFile("", p.first);
            if (resolved.empty()) resolved = p.first;
        }

        BOOL broken = NO;
        std::string brokenReason;
        if (!embedded) {
            if (!FileExists(resolved)) {
                broken = YES;
                brokenReason = "missing";
            } else if (p.second == MediaType::Video) {
                // File is present but might still be undecodable —
                // VP9, AV1, ProRes-RAW, etc. Reuse the same
                // AVFoundation probe the per-effect inspector and
                // SequenceChecker use; non-empty reason → roll up
                // into the missing-media banner so the user sees
                // it at sequence load instead of getting silent
                // black frames mid-playback.
                ObtainAccessToURL(resolved, false);
                std::string reason = MediaCompatibility::CheckVideoFile(resolved);
                if (!reason.empty()) {
                    broken = YES;
                    brokenReason = reason;
                }
            }
        }

        int widthPx = 0, heightPx = 0, frameCount = 0;
        if (p.second == MediaType::Image && entry->isLoaded()) {
            auto ie = std::static_pointer_cast<ImageCacheEntry>(entry);
            widthPx = ie->GetImageWidth();
            heightPx = ie->GetImageHeight();
            frameCount = ie->GetImageCount();
        } else if (p.second == MediaType::Video && entry->isLoaded()) {
            auto ve = std::static_pointer_cast<VideoMediaCacheEntry>(entry);
            // Duration-in-ms / frameTime gives an approximate frame
            // count; exact frame count needs the decoder which
            // isn't cheap to open just for a status line.
            int durMS = ve->GetDurationMS();
            if (durMS > 0) {
                int fi = _context->GetSequenceFile()
                    ? _context->GetSequenceFile()->GetFrameMS() : 50;
                if (fi > 0) frameCount = durMS / fi;
            }
        }

        NSString* resolvedStr = resolved.empty()
            ? @"" : [NSString stringWithUTF8String:resolved.c_str()];

        [out addObject:@{
            @"path":         pathStr,
            @"type":         typeStr,
            @"resolvedPath": resolvedStr ?: @"",
            @"isEmbedded":   @(embedded),
            @"isBroken":     @(broken),
            @"brokenReason": brokenReason.empty()
                ? @"" : [NSString stringWithUTF8String:brokenReason.c_str()],
            @"widthPx":      @(widthPx),
            @"heightPx":     @(heightPx),
            @"frameCount":   @(frameCount),
        }];
    }
    return out;
}

- (int)ensureThumbnailPreviewForPath:(NSString*)path
                            mediaType:(NSString*)mediaType
                            maxWidth:(int)maxWidth
                           maxHeight:(int)maxHeight {
    if (!_context || !path) return 0;
    auto typeOpt = stringToMediaType(mediaType);
    if (!typeOpt) return 0;
    auto& media = _context->GetSequenceElements().GetSequenceMedia();
    std::string spath([path UTF8String]);
    auto entry = lookupMediaEntry(media, spath, *typeOpt);
    if (!entry) return 0;
    if (!entry->isLoaded()) {
        entry->Load();
        if (!entry->IsOk()) return 0;
    }
    if (!entry->HasPreview()) {
        if (entry->GetType() == MediaType::Shader) {
            // `MediaCacheEntry::GeneratePreview` is a no-op for shader
            // entries — shader frames come from actually running the
            // shader through the render engine at default params
            // against a 64×64 matrix model. Route through the
            // dedicated path (iPadRenderContext::GenerateShaderPreview
            // mirrors desktop's ShaderPreviewGenerator).
            auto* shaderEntry = static_cast<ShaderMediaCacheEntry*>(entry.get());
            _context->GenerateShaderPreview(shaderEntry);
        } else {
            entry->GeneratePreview(maxWidth, maxHeight);
        }
    }
    return (int)entry->GetPreviewFrameCount();
}

- (NSData*)thumbnailPNGForPath:(NSString*)path
                     mediaType:(NSString*)mediaType
                    frameIndex:(int)frameIndex {
    if (!_context || !path || frameIndex < 0) return nil;
    auto typeOpt = stringToMediaType(mediaType);
    if (!typeOpt) return nil;
    auto& media = _context->GetSequenceElements().GetSequenceMedia();
    std::string spath([path UTF8String]);
    auto entry = lookupMediaEntry(media, spath, *typeOpt);
    if (!entry || !entry->HasPreview()) return nil;
    if ((size_t)frameIndex >= entry->GetPreviewFrameCount()) return nil;
    auto frame = entry->GetPreviewFrame((size_t)frameIndex);
    if (!frame || !frame->IsOk()) return nil;
    std::vector<uint8_t> png;
    if (!frame->SaveAsPNG(png)) return nil;
    return [NSData dataWithBytes:png.data() length:png.size()];
}

- (long)videoDurationMSForPath:(NSString*)path {
    if (!_context || !path) return 0;
    std::string spath([path UTF8String]);
    auto& media = _context->GetSequenceElements().GetSequenceMedia();

    // Prefer the existing VideoMediaCacheEntry — its duration is
    // cached on the entry (populated free-of-charge by
    // `GeneratePreview`'s VideoReader, or lazily by the first
    // `GetDurationMS` call). Non-creating lookup via `HasMedia`
    // first so we don't mint an accidental entry for a non-video
    // path.
    if (media.HasMedia(spath)) {
        if (auto entry = media.GetVideo(spath)) {
            if (!entry->isLoaded()) entry->Load();
            return (long)entry->GetDurationMS();
        }
    }

    // The path has never been referenced by an effect (so the cache
    // never loaded it). Fall back to a fresh FixFile + VideoReader
    // probe. Calling ObtainAccessToURL first activates the ancestor
    // bookmark's security scope so AVFoundation / FFmpeg can read
    // the bytes even when the file lives in iCloud Drive.
    std::string resolved = FileUtils::FixFile("", spath);
    if (resolved.empty()) resolved = spath;
    ObtainAccessToURL(resolved, false);
    long ms = VideoReader::GetVideoLength(resolved);
    return (ms > 0) ? ms : 0;
}

- (long)thumbnailFrameTimeMSForPath:(NSString*)path
                          mediaType:(NSString*)mediaType
                         frameIndex:(int)frameIndex {
    if (!_context || !path || frameIndex < 0) return 0;
    auto typeOpt = stringToMediaType(mediaType);
    if (!typeOpt) return 0;
    auto& media = _context->GetSequenceElements().GetSequenceMedia();
    std::string spath([path UTF8String]);
    auto entry = lookupMediaEntry(media, spath, *typeOpt);
    if (!entry || !entry->HasPreview()) return 0;
    if ((size_t)frameIndex >= entry->GetPreviewFrameCount()) return 0;
    return entry->GetPreviewFrameTime((size_t)frameIndex);
}

- (NSData*)iconBGRAForEffectNamed:(NSString*)effectName
                      desiredSize:(int)desiredSize
                        outputSize:(int*)outputSize {
    if (!_context || !effectName) return nil;
    std::string name([effectName UTF8String]);
    RenderableEffect* fx = _context->GetEffectManager().GetEffect(name);
    if (!fx) return nil;

    // Grab the XPM at the bucket index (same indexing desktop
    // uses in `EffectIconCache`). Effects like `On` bind a single
    // 16-px XPM to every index — we resample below when the
    // native size doesn't match the requested bucket, mirroring
    // desktop's `wxImage::Scale` at cache-build time.
    int idx = xpmSizeIndexForDesired(desiredSize);
    const char* const* xpm = fx->GetIconData(idx);
    int srcW = 0, srcH = 0;
    NSData* srcData = xpmToBGRA(xpm, &srcW, &srcH);
    if (!srcData) return nil;

    // Native size matches the request — no resample needed.
    if (srcW == desiredSize && srcH == desiredSize) {
        if (outputSize) *outputSize = srcW;
        return srcData;
    }

    // Rescale to desiredSize via CoreGraphics. High-quality
    // interpolation gives much better results than the GPU's
    // bilinear filter at display time, especially when scaling
    // 16 → 48+ for effects that only ship a single XPM size.
    int dstW = desiredSize;
    int dstH = desiredSize;
    CGColorSpaceRef cs = CGColorSpaceCreateDeviceRGB();

    // Wrap source bytes in a CGImage.
    CGDataProviderRef provider = CGDataProviderCreateWithCFData(
        (__bridge CFDataRef)srcData);
    CGImageRef srcImage = CGImageCreate(
        srcW, srcH, 8, 32, srcW * 4, cs,
        (uint32_t)kCGImageAlphaPremultipliedFirst | kCGBitmapByteOrder32Little,
        provider, nullptr, true, kCGRenderingIntentDefault);
    CGDataProviderRelease(provider);

    NSMutableData* outData = [NSMutableData dataWithLength:(NSUInteger)dstW * dstH * 4];
    CGContextRef ctx = CGBitmapContextCreate(
        outData.mutableBytes, dstW, dstH, 8, dstW * 4, cs,
        (uint32_t)kCGImageAlphaPremultipliedFirst | kCGBitmapByteOrder32Little);
    CGColorSpaceRelease(cs);
    if (!ctx || !srcImage) {
        if (ctx) CGContextRelease(ctx);
        if (srcImage) CGImageRelease(srcImage);
        return nil;
    }
    CGContextSetInterpolationQuality(ctx, kCGInterpolationHigh);
    CGContextDrawImage(ctx, CGRectMake(0, 0, dstW, dstH), srcImage);
    CGContextRelease(ctx);
    CGImageRelease(srcImage);

    if (outputSize) *outputSize = dstW;
    return outData;
}

// MARK: - Moving Head fixture plumbing (G3 — C7)

namespace {

/// Name of the Moving Head effect as stored in the effect registry.
static constexpr const char* kMovingHeadEffectName = "Moving Head";

/// Keys the Moving Head renderer actually consumes live inside the
/// per-fixture command strings. These are rebuilt by
/// `syncMovingHeadPositionForRow:` from the iPad sliders. Other
/// command keys (Color, Wheel, Dimmer, Path, AutoShutter,
/// IgnorePan, IgnoreTilt) are preserved untouched so desktop-
/// authored colour / path / dimmer settings round-trip intact.
static constexpr std::array<const char*, 6> kMovingHeadPositionCmds = {
    "Pan", "Tilt", "PanOffset", "TiltOffset", "Groupings", "Cycles"
};

/// Parse a packed MH command string into an ordered list of
/// (cmd, value) pairs. Preserves the desktop grammar: commands
/// separated by ';', each one is `<cmd>: <value>` with '@' used
/// as an escaped ';' inside the value (desktop's
/// `UpdateMHSettings` escapes the VC blob the same way).
using MHCommandList = std::vector<std::pair<std::string, std::string>>;
static MHCommandList parseMovingHeadSettings(const std::string& s) {
    MHCommandList out;
    if (s.empty()) return out;
    size_t i = 0;
    while (i < s.size()) {
        size_t semi = s.find(';', i);
        std::string part = s.substr(i, (semi == std::string::npos
                                          ? s.size() - i : semi - i));
        if (!part.empty()) {
            size_t colon = part.find(':');
            if (colon != std::string::npos) {
                std::string cmd = part.substr(0, colon);
                std::string val = part.substr(colon + 1);
                // Strip the leading space desktop always adds
                // after the colon.
                if (!val.empty() && val.front() == ' ') val.erase(val.begin());
                out.emplace_back(std::move(cmd), std::move(val));
            }
        }
        if (semi == std::string::npos) break;
        i = semi + 1;
    }
    return out;
}

static std::string serialiseMovingHeadSettings(const MHCommandList& cmds) {
    std::string out;
    for (size_t i = 0; i < cmds.size(); ++i) {
        if (i > 0) out += ';';
        out += cmds[i].first;
        out += ": ";
        out += cmds[i].second;
    }
    return out;
}

/// Resolve the `(Element*, EffectLayer*, Effect*)` trio for a row
/// / effect index into the live sequence. Returns nullptr triple
/// on any out-of-range hit.
struct EffectLookup {
    Element* element = nullptr;
    EffectLayer* layer = nullptr;
    Effect* effect = nullptr;
    bool ok() const { return effect != nullptr; }
};
static EffectLookup lookupEffect(iPadRenderContext& ctx,
                                  int rowIndex, int effectIndex) {
    EffectLookup out;
    auto& se = ctx.GetSequenceElements();
    auto* rowInfo = se.GetRowInformation(rowIndex);
    if (!rowInfo || !rowInfo->element) return out;
    int layerIndex = rowInfo->layerIndex;
    if (layerIndex < 0
        || layerIndex >= rowInfo->element->GetEffectLayerCount()) {
        return out;
    }
    auto* layer = rowInfo->element->GetEffectLayer(layerIndex);
    if (!layer) return out;
    if (effectIndex < 0
        || effectIndex >= layer->GetEffectCount()) {
        return out;
    }
    auto* eff = layer->GetEffect(effectIndex);
    if (!eff) return out;
    out.element = rowInfo->element;
    out.layer = layer;
    out.effect = eff;
    return out;
}

/// Read the current slider value for a position command. Falls
/// back to the SLIDER key when the TEXTCTRL sibling isn't set
/// (desktop writes both; iPad's float sliders write TEXTCTRL).
static std::string readMHSliderValue(Effect& eff, const std::string& cmd) {
    auto& settings = eff.GetSettings();
    std::string k1 = "E_TEXTCTRL_MH" + cmd;
    if (settings.Contains(k1)) return settings.Get(k1, "");
    std::string k2 = "E_SLIDER_MH" + cmd;
    if (settings.Contains(k2)) return settings.Get(k2, "");
    return "";
}

static std::string readMHValueCurve(Effect& eff, const std::string& cmd) {
    auto& settings = eff.GetSettings();
    std::string key = "E_VALUECURVE_MH" + cmd;
    if (!settings.Contains(key)) return "";
    std::string v = settings.Get(key, "");
    // Desktop includes a VC entry only when it's active.
    if (v.find("Active=TRUE") == std::string::npos) return "";
    return v;
}

/// Default scalar the renderer should see when the slider has no
/// stored value. Matches `MovingHead.json` defaults.
static const char* defaultMHScalar(const std::string& cmd) {
    if (cmd == "Pan")          return "0";
    if (cmd == "Tilt")         return "0";
    if (cmd == "PanOffset")    return "0";
    if (cmd == "TiltOffset")   return "0";
    if (cmd == "Groupings")    return "1";
    if (cmd == "Cycles")       return "0.1";
    return "0";
}

/// True iff the command is one of the six position commands (the
/// slider-backed ones we rewrite from the panel). Used to strip
/// stale position entries before re-appending fresh values, while
/// leaving colour / dimmer / path entries in place.
static bool isMHPositionCommand(const std::string& cmd) {
    if (cmd == "Pan" || cmd == "Tilt"
        || cmd == "PanOffset" || cmd == "TiltOffset"
        || cmd == "Groupings" || cmd == "Cycles"
        || cmd == "Pan VC" || cmd == "Tilt VC"
        || cmd == "PanOffset VC" || cmd == "TiltOffset VC"
        || cmd == "Groupings VC" || cmd == "PathScale VC"
        || cmd == "TimeOffset VC") {
        return true;
    }
    return false;
}

/// Escape a VC blob for embedding inside the command string:
/// desktop uses '@' in place of ';' so the outer parser's
/// split-on-';' still works. See `MovingHeadPanel::AddSetting`.
static std::string escapeForCommand(const std::string& v) {
    std::string out = v;
    std::replace(out.begin(), out.end(), ';', '@');
    return out;
}

/// Comma-separated list of currently-active fixture numbers. Used
/// as the `Heads:` entry which the renderer consumes for
/// fan-offset distribution (`MovingHeadEffect.cpp:179-181`).
static std::string mhHeadsList(Effect& eff) {
    std::string out;
    auto& settings = eff.GetSettings();
    for (int i = 1; i <= 8; ++i) {
        std::string key = std::string("E_TEXTCTRL_MH") + std::to_string(i) + "_Settings";
        if (!settings.Contains(key)) continue;
        std::string v = settings.Get(key, "");
        if (v.empty()) continue;
        if (!out.empty()) out += ",";
        out += std::to_string(i);
    }
    return out;
}

/// Rewrite one fixture's command string: keep every non-position
/// entry as-is, then append fresh Pan / Tilt / offsets / groupings /
/// cycles (+ VC entries when active) + current Heads list.
static void rewriteMovingHeadFixture(Effect& eff, int fixture) {
    auto key = std::string("E_TEXTCTRL_MH") + std::to_string(fixture) + "_Settings";
    auto& settings = eff.GetSettings();
    std::string existing = settings.Contains(key) ? settings.Get(key, "") : "";

    MHCommandList parsed = parseMovingHeadSettings(existing);
    MHCommandList rebuilt;
    rebuilt.reserve(parsed.size());
    // 1. Preserve every non-position command verbatim.
    for (const auto& cmd : parsed) {
        if (!isMHPositionCommand(cmd.first) && cmd.first != "Heads") {
            rebuilt.push_back(cmd);
        }
    }
    // 2. Append fresh position commands (+ VC entries).
    for (const auto* cmd : kMovingHeadPositionCmds) {
        std::string cmdStr(cmd);
        std::string val = readMHSliderValue(eff, cmdStr);
        if (val.empty()) val = defaultMHScalar(cmdStr);
        rebuilt.emplace_back(cmdStr, val);

        std::string vc = readMHValueCurve(eff, cmdStr);
        if (!vc.empty()) {
            rebuilt.emplace_back(cmdStr + " VC", escapeForCommand(vc));
        }
    }
    // 3. Append the heads list (who else is active).
    std::string heads = mhHeadsList(eff);
    if (!heads.empty()) {
        rebuilt.emplace_back("Heads", heads);
    }

    std::string serialised = serialiseMovingHeadSettings(rebuilt);
    if (serialised != existing) {
        settings[key] = SettingValue(serialised);
    }
}

/// Phase C G3+ helper: read the first active fixture's value for
/// the named MH command (e.g. "Color", "Dimmer", "Path"). Returns
/// empty string when no active fixture has the command. Caller
/// promotes this as "the effect's current value" — fixtures that
/// disagree get overwritten on the next set.
static std::string readMHCommandFromActiveFixtures(Effect& eff,
                                                     const std::string& cmdName) {
    auto& settings = eff.GetSettings();
    for (int i = 1; i <= 8; ++i) {
        std::string key = std::string("E_TEXTCTRL_MH") + std::to_string(i) + "_Settings";
        if (!settings.Contains(key)) continue;
        std::string s = settings.Get(key, "");
        if (s.empty()) continue;
        for (auto& [c, v] : parseMovingHeadSettings(s)) {
            if (c == cmdName) return v;
        }
    }
    return "";
}

/// Replace (or insert) `cmdName: value` in every active fixture's
/// command string. If `value` is empty the command is removed
/// (used for Path-clear). Returns true if anything changed.
static bool writeMHCommandToActiveFixtures(Effect& eff,
                                            const std::string& cmdName,
                                            const std::string& value) {
    auto& settings = eff.GetSettings();
    bool any = false;
    for (int i = 1; i <= 8; ++i) {
        std::string key = std::string("E_TEXTCTRL_MH") + std::to_string(i) + "_Settings";
        if (!settings.Contains(key)) continue;
        std::string existing = settings.Get(key, "");
        if (existing.empty()) continue;
        MHCommandList parsed = parseMovingHeadSettings(existing);
        bool found = false;
        MHCommandList rebuilt;
        rebuilt.reserve(parsed.size() + 1);
        for (auto& [c, v] : parsed) {
            if (c == cmdName) {
                if (!value.empty()) {
                    rebuilt.emplace_back(cmdName, value);
                }
                found = true;
            } else {
                rebuilt.emplace_back(c, v);
            }
        }
        if (!found && !value.empty()) {
            rebuilt.emplace_back(cmdName, value);
        }
        std::string serialised = serialiseMovingHeadSettings(rebuilt);
        if (serialised != existing) {
            settings[key] = SettingValue(serialised);
            any = true;
        }
    }
    return any;
}

} // namespace

- (int)movingHeadActiveFixturesForRow:(int)rowIndex
                               atIndex:(int)effectIndex {
    if (!_context) return 0;
    auto look = lookupEffect(*_context, rowIndex, effectIndex);
    if (!look.ok()) return 0;
    if (look.effect->GetEffectName() != kMovingHeadEffectName) return 0;

    int mask = 0;
    auto& settings = look.effect->GetSettings();
    for (int i = 1; i <= 8; ++i) {
        std::string key = std::string("E_TEXTCTRL_MH") + std::to_string(i) + "_Settings";
        if (!settings.Contains(key)) continue;
        if (!settings.Get(key, "").empty()) {
            mask |= (1 << (i - 1));
        }
    }
    return mask;
}

- (BOOL)setMovingHeadFixture:(int)fixture
                        active:(BOOL)active
                        forRow:(int)rowIndex
                       atIndex:(int)effectIndex {
    if (!_context) return NO;
    if (fixture < 1 || fixture > 8) return NO;
    auto look = lookupEffect(*_context, rowIndex, effectIndex);
    if (!look.ok()) return NO;
    if (look.effect->GetEffectName() != kMovingHeadEffectName) return NO;

    auto key = std::string("E_TEXTCTRL_MH") + std::to_string(fixture) + "_Settings";
    auto& settings = look.effect->GetSettings();

    bool changed = false;
    if (active) {
        bool alreadyActive = settings.Contains(key)
            && !settings.Get(key, "").empty();
        if (!alreadyActive) {
            // Seed with a minimal placeholder so the next
            // `rewriteMovingHeadFixture` pass populates it with
            // the current slider values. Can't be fully empty —
            // an empty string reads as "inactive".
            settings[key] = SettingValue("Pan: 0");
            changed = true;
        }
    } else {
        if (settings.Contains(key) && !settings.Get(key, "").empty()) {
            settings[key] = SettingValue("");
            changed = true;
        }
    }

    if (!changed) return NO;

    // The fixture mask changed, so every other active fixture's
    // `Heads:` entry is now stale — rewrite all of them (including
    // the one we just toggled) with fresh position + heads data.
    for (int i = 1; i <= 8; ++i) {
        std::string k = std::string("E_TEXTCTRL_MH") + std::to_string(i) + "_Settings";
        if (!settings.Contains(k)) continue;
        if (settings.Get(k, "").empty()) continue;
        rewriteMovingHeadFixture(*look.effect, i);
    }

    look.effect->IncrementChangeCount();
    return YES;
}

- (NSString*)movingHeadCommand:(NSString*)cmdName
                          forRow:(int)rowIndex
                         atIndex:(int)effectIndex {
    if (!_context || !cmdName) return @"";
    auto look = lookupEffect(*_context, rowIndex, effectIndex);
    if (!look.ok()) return @"";
    if (look.effect->GetEffectName() != kMovingHeadEffectName) return @"";
    std::string v = readMHCommandFromActiveFixtures(*look.effect,
                                                     std::string([cmdName UTF8String]));
    return [NSString stringWithUTF8String:v.c_str()];
}

- (BOOL)setMovingHeadCommand:(NSString*)cmdName
                         value:(NSString*)value
                        forRow:(int)rowIndex
                       atIndex:(int)effectIndex {
    if (!_context || !cmdName) return NO;
    auto look = lookupEffect(*_context, rowIndex, effectIndex);
    if (!look.ok()) return NO;
    if (look.effect->GetEffectName() != kMovingHeadEffectName) return NO;
    std::string cn([cmdName UTF8String]);
    std::string v = value ? std::string([value UTF8String]) : std::string();
    bool changed = writeMHCommandToActiveFixtures(*look.effect, cn, v);
    if (changed) {
        look.effect->IncrementChangeCount();
        // Force a re-render of the active model so the new colour /
        // dimmer takes effect in the preview immediately.
        auto* row = _context->GetSequenceElements().GetRowInformation(rowIndex);
        if (row && row->element) {
            _context->RenderEffectForModel(row->element->GetModelName(),
                                            look.effect->GetStartTimeMS(),
                                            look.effect->GetEndTimeMS(), true);
        }
    }
    return changed ? YES : NO;
}

- (NSArray<NSDictionary*>*)movingHeadWheelColorsForRow:(int)rowIndex
                                               atIndex:(int)effectIndex {
    if (!_context) return nil;
    auto look = lookupEffect(*_context, rowIndex, effectIndex);
    if (!look.ok()) return nil;
    if (look.effect->GetEffectName() != kMovingHeadEffectName) return nil;

    auto* row = _context->GetSequenceElements().GetRowInformation(rowIndex);
    if (!row || !row->element) return nil;
    Model* model = _context->GetModel(row->element->GetModelName());
    if (!model) return nil;

    // The effect's model may be a group of fixtures; the desktop
    // wheel panel sources its slots from the first colour-wheel
    // fixture, so do the same here.
    std::vector<Model*> candidates;
    if (model->GetDisplayAs() == DisplayAsType::ModelGroup) {
        auto* mg = dynamic_cast<ModelGroup*>(model);
        if (mg) {
            for (const auto& it : mg->GetFlatModels(true, false)) {
                if (it->GetDisplayAs() != DisplayAsType::ModelGroup &&
                    it->GetDisplayAs() != DisplayAsType::SubModel) {
                    candidates.push_back(it);
                }
            }
        }
    } else {
        candidates.push_back(model);
    }

    for (Model* m : candidates) {
        auto* dmx = dynamic_cast<DmxModel*>(m);
        if (!dmx || !dmx->HasColorAbility()) continue;
        auto* ca = dmx->GetColorAbility();
        if (ca->GetColorType() != DmxColorAbility::DMX_COLOR_TYPE::DMX_COLOR_WHEEL) continue;
        auto* wh = static_cast<DmxColorAbilityWheel*>(ca);
        NSMutableArray<NSDictionary*>* out = [NSMutableArray array];
        for (const auto& c : wh->GetWheelColorSettings()) {
            HSVValue hsv = c.color.asHSV();
            [out addObject:@{
                @"hex": [NSString stringWithFormat:@"#%02X%02X%02X",
                          c.color.Red(), c.color.Green(), c.color.Blue()],
                @"hue": @(hsv.hue),
                @"sat": @(hsv.saturation),
                @"val": @(hsv.value),
            }];
        }
        return out;
    }
    return nil;
}

// MARK: - DMX state + remap (G8 — C7)

namespace {

static constexpr int kDMXChannelCount = 48;

/// Read a DMX channel slider value (0..255) from the effect
/// settings. Falls back to TEXTCTRL + the stored default when
/// the SLIDER key isn't present.
static int readDMXChannel(Effect& eff, int channel) {
    auto& s = eff.GetSettings();
    std::string sliderKey = "E_SLIDER_DMX" + std::to_string(channel);
    std::string textKey = "E_TEXTCTRL_DMX" + std::to_string(channel);
    int val = 0;
    if (s.Contains(sliderKey)) {
        val = s.GetInt(sliderKey, 0);
    } else if (s.Contains(textKey)) {
        val = s.GetInt(textKey, 0);
    }
    if (val < 0) val = 0;
    if (val > 255) val = 255;
    return val;
}

/// Write a DMX channel slider value through both the slider and
/// text-control sibling keys so the UI stays consistent. Desktop
/// writes both; iPad's JSON-backed sliders store only one of
/// them, but clearing both paths keeps whichever the inspector
/// reads in sync.
static void writeDMXChannel(Effect& eff, int channel, int value) {
    auto& s = eff.GetSettings();
    if (value < 0) value = 0;
    if (value > 255) value = 255;
    std::string valStr = std::to_string(value);
    s[std::string("E_SLIDER_DMX") + std::to_string(channel)] =
        SettingValue(valStr);
    s[std::string("E_TEXTCTRL_DMX") + std::to_string(channel)] =
        SettingValue(valStr);
}

/// Format a DMX byte (0..255) as `#XXXXXX` using the same
/// channel for R/G/B — the storage convention desktop uses in
/// state `s<n>-Color` entries so round-trips are lossless.
static std::string formatDMXColor(int v) {
    if (v < 0) v = 0;
    if (v > 255) v = 255;
    char buf[8];
    std::snprintf(buf, sizeof(buf), "#%02x%02x%02x", v, v, v);
    return std::string(buf);
}

/// Parse a `#RRGGBB` hex string into its red byte. State files
/// encode DMX values in the red channel (see desktop's
/// `DMXPanel.cpp:452`). Returns 0 on parse failure.
static int parseDMXColorRed(const std::string& hex) {
    if (hex.size() < 7 || hex.front() != '#') return 0;
    auto hexDigit = [](char c) -> int {
        if (c >= '0' && c <= '9') return c - '0';
        if (c >= 'a' && c <= 'f') return 10 + (c - 'a');
        if (c >= 'A' && c <= 'F') return 10 + (c - 'A');
        return -1;
    };
    int hi = hexDigit(hex[1]);
    int lo = hexDigit(hex[2]);
    if (hi < 0 || lo < 0) return 0;
    return (hi << 4) | lo;
}

} // namespace

- (BOOL)dmxStateExistsForRow:(int)rowIndex
                      atIndex:(int)effectIndex
                     stateName:(NSString*)stateName {
    if (stateName.length == 0) return NO;
    if (!_context) return NO;
    auto eff = lookupEffect(*_context, rowIndex, effectIndex);
    if (!eff.ok()) return NO;
    Model* m = [self _targetModelForRow:rowIndex];
    if (!m) return NO;
    return m->HasState(std::string([stateName UTF8String])) ? YES : NO;
}

- (BOOL)dmxSaveStateForRow:(int)rowIndex
                    atIndex:(int)effectIndex
                   stateName:(NSString*)stateName
                   overwrite:(BOOL)overwrite {
    if (stateName.length == 0) return NO;
    if (!_context) return NO;
    auto eff = lookupEffect(*_context, rowIndex, effectIndex);
    if (!eff.ok()) return NO;
    Model* m = [self _targetModelForRow:rowIndex];
    if (!m) return NO;
    struct { Effect* effect; Model* model; bool ok() const { return true; } } look = { eff.effect, m };

    std::string name([stateName UTF8String]);
    if (look.model->HasState(name) && !overwrite) {
        return NO;
    }

    std::map<std::string, std::string> attributes;
    attributes["CustomColors"] = "1";
    attributes["Name"] = name;
    attributes["Type"] = "SingleNode";

    uint32_t maxChannels = look.model->GetChanCount();
    if (maxChannels > (uint32_t)kDMXChannelCount) maxChannels = kDMXChannelCount;

    for (int i = 1; i <= kDMXChannelCount; ++i) {
        std::string sKey = "s" + std::to_string(i);
        std::string sNameKey = sKey + "-Name";
        std::string sColorKey = sKey + "-Color";
        if ((uint32_t)i <= maxChannels) {
            int v = readDMXChannel(*look.effect, i);
            attributes[sNameKey] = name;
            attributes[sKey] = "Node " + std::to_string(i);
            attributes[sColorKey] = formatDMXColor(v);
        } else {
            attributes[sNameKey] = "";
            attributes[sKey] = "";
            attributes[sColorKey] = "";
        }
    }

    look.model->AddState(attributes);
    // Persist immediately so the saved state survives show-folder
    // close. SaveModelStates rewrites just the dirty model's
    // <stateInfo> children in xlights_rgbeffects.xml.
    _context->MarkModelStateDirty(look.model->GetName());
    if (!_context->SaveModelStates()) {
        spdlog::warn("XLSequenceDocument: dmxSaveState added '{}' to '{}' in memory, but SaveModelStates failed — change is session-scoped",
                     name, look.model->GetName());
    }
    return YES;
}

- (BOOL)dmxLoadStateForRow:(int)rowIndex
                    atIndex:(int)effectIndex
                   stateName:(NSString*)stateName {
    if (stateName.length == 0) return NO;
    if (!_context) return NO;
    auto eff = lookupEffect(*_context, rowIndex, effectIndex);
    if (!eff.ok()) return NO;
    Model* m = [self _targetModelForRow:rowIndex];
    if (!m) return NO;
    struct { Effect* effect; Model* model; bool ok() const { return true; } } look = { eff.effect, m };

    std::string name([stateName UTF8String]);
    const auto& states = look.model->GetStateInfo();
    auto it = states.find(name);
    if (it == states.end()) return NO;

    // Match desktop validation — only "Custom colour single node"
    // states are shaped correctly for DMX channel reuse.
    auto findOrEmpty = [&](const std::string& k) -> std::string {
        auto jt = it->second.find(k);
        return (jt == it->second.end()) ? std::string() : jt->second;
    };
    if (findOrEmpty("CustomColors") != "1"
        || findOrEmpty("Type") != "SingleNode") {
        return NO;
    }

    uint32_t maxChannels = look.model->GetChanCount();
    if (maxChannels > (uint32_t)kDMXChannelCount) maxChannels = kDMXChannelCount;

    bool changed = false;
    for (int i = 1; i <= (int)maxChannels; ++i) {
        std::string nameKey = "s" + std::to_string(i) + "-Name";
        if (it->second.find(nameKey) == it->second.end()) continue;
        std::string colorKey = "s" + std::to_string(i) + "-Color";
        auto colIt = it->second.find(colorKey);
        if (colIt == it->second.end()) continue;
        int val = parseDMXColorRed(colIt->second);
        writeDMXChannel(*look.effect, i, val);
        changed = true;
    }
    if (changed) {
        look.effect->IncrementChangeCount();
    }
    return changed ? YES : NO;
}

- (BOOL)dmxRemapChannelsForRow:(int)rowIndex
                        atIndex:(int)effectIndex
                         preset:(int)preset {
    if (!_context) return NO;
    auto eff = lookupEffect(*_context, rowIndex, effectIndex);
    if (!eff.ok()) return NO;
    struct { Effect* effect; bool ok() const { return true; } } look = { eff.effect };

    // Snapshot every channel's pre-remap value so we can apply a
    // permutation without stepping on ourselves.
    std::array<int, kDMXChannelCount + 1> before{};  // 1-based
    for (int i = 1; i <= kDMXChannelCount; ++i) {
        before[i] = readDMXChannel(*look.effect, i);
    }

    std::array<int, kDMXChannelCount + 1> after = before;
    switch (preset) {
        case 0: // Shift +1
            for (int i = 1; i <= kDMXChannelCount; ++i) {
                int src = i - 1;
                if (src < 1) src = kDMXChannelCount;
                after[i] = before[src];
            }
            break;
        case 1: // Shift -1
            for (int i = 1; i <= kDMXChannelCount; ++i) {
                int src = i + 1;
                if (src > kDMXChannelCount) src = 1;
                after[i] = before[src];
            }
            break;
        case 2: // Reverse
            for (int i = 1; i <= kDMXChannelCount; ++i) {
                after[i] = before[kDMXChannelCount + 1 - i];
            }
            break;
        case 3: // Invert All
            for (int i = 1; i <= kDMXChannelCount; ++i) {
                after[i] = 255 - before[i];
            }
            break;
        case 4: // Double
            for (int i = 1; i <= kDMXChannelCount; ++i) {
                int v = before[i] * 2;
                after[i] = v > 255 ? 255 : v;
            }
            break;
        case 5: // Half
            for (int i = 1; i <= kDMXChannelCount; ++i) {
                after[i] = before[i] / 2;
            }
            break;
        default:
            return NO;
    }

    bool changed = false;
    for (int i = 1; i <= kDMXChannelCount; ++i) {
        if (after[i] != before[i]) {
            writeDMXChannel(*look.effect, i, after[i]);
            changed = true;
        }
    }
    if (changed) {
        look.effect->IncrementChangeCount();
    }
    return changed ? YES : NO;
}

- (BOOL)dmxRemapChannelsForRow:(int)rowIndex
                        atIndex:(int)effectIndex
                        mapping:(NSArray<NSNumber*>*)mapping {
    if (!_context || mapping.count == 0) return NO;
    auto eff = lookupEffect(*_context, rowIndex, effectIndex);
    if (!eff.ok()) return NO;

    std::array<int, kDMXChannelCount + 1> before{};  // 1-based
    for (int i = 1; i <= kDMXChannelCount; ++i) {
        before[i] = readDMXChannel(*eff.effect, i);
    }

    std::array<int, kDMXChannelCount + 1> after{};
    for (int i = 1; i <= kDMXChannelCount; ++i) {
        int src = (i - 1 < (int)mapping.count) ? mapping[i - 1].intValue : 0;
        after[i] = (src >= 1 && src <= kDMXChannelCount) ? before[src] : 0;
    }

    bool changed = false;
    for (int i = 1; i <= kDMXChannelCount; ++i) {
        if (after[i] != before[i]) {
            writeDMXChannel(*eff.effect, i, after[i]);
            changed = true;
        }
    }
    if (changed) {
        eff.effect->IncrementChangeCount();
    }
    return changed ? YES : NO;
}

- (NSArray<NSNumber*>*)dmxChannelValuesForRow:(int)rowIndex
                                       atIndex:(int)effectIndex {
    NSMutableArray<NSNumber*>* out = [NSMutableArray arrayWithCapacity:kDMXChannelCount + 1];
    [out addObject:@0];  // index 0 unused (channels are 1-based)
    if (!_context) {
        for (int i = 1; i <= kDMXChannelCount; ++i) [out addObject:@0];
        return out;
    }
    auto eff = lookupEffect(*_context, rowIndex, effectIndex);
    for (int i = 1; i <= kDMXChannelCount; ++i) {
        int v = eff.ok() ? readDMXChannel(*eff.effect, i) : 0;
        [out addObject:@(v)];
    }
    return out;
}

- (int)syncMovingHeadPositionForRow:(int)rowIndex
                              atIndex:(int)effectIndex {
    if (!_context) return 0;
    auto look = lookupEffect(*_context, rowIndex, effectIndex);
    if (!look.ok()) return 0;
    if (look.effect->GetEffectName() != kMovingHeadEffectName) return 0;

    int touched = 0;
    auto& settings = look.effect->GetSettings();
    for (int i = 1; i <= 8; ++i) {
        std::string k = std::string("E_TEXTCTRL_MH") + std::to_string(i) + "_Settings";
        if (!settings.Contains(k)) continue;
        if (settings.Get(k, "").empty()) continue;
        rewriteMovingHeadFixture(*look.effect, i);
        touched++;
    }

    if (touched > 0) {
        look.effect->IncrementChangeCount();
    }
    return touched;
}

- (void)bracketColorForState:(XLEffectBracketState)state
                         outR:(CGFloat*)outR
                         outG:(CGFloat*)outG
                         outB:(CGFloat*)outB {
    iPadRenderContext::EffectBracketState s = iPadRenderContext::EffectBracketState::Default;
    switch (state) {
        case XLEffectBracketStateDefault:  s = iPadRenderContext::EffectBracketState::Default; break;
        case XLEffectBracketStateSelected: s = iPadRenderContext::EffectBracketState::Selected; break;
        case XLEffectBracketStateLocked:   s = iPadRenderContext::EffectBracketState::Locked; break;
        case XLEffectBracketStateDisabled: s = iPadRenderContext::EffectBracketState::Disabled; break;
    }
    iPadRenderContext::PaletteColor pc{192, 192, 192};
    if (_context) pc = _context->GetEffectBracketColor(s);
    if (outR) *outR = (CGFloat)pc.r / 255.0;
    if (outG) *outG = (CGFloat)pc.g / 255.0;
    if (outB) *outB = (CGFloat)pc.b / 255.0;
}

#pragma mark - J-31 — Controllers list

static NSDictionary* BuildControllerSummary(const Controller* c) {
    NSString* name      = [NSString stringWithUTF8String:c->GetName().c_str()];
    NSString* type      = [NSString stringWithUTF8String:c->GetType().c_str()];
    // Per-class protocol (DDP / E1.31 / ArtNet for Ethernet, DMX /
    // Renard / Pixelnet for Serial, "" for Null). Surfaced so the
    // sidebar row can show the wire protocol instead of just the
    // generic transport class.
    NSString* protocol  = [NSString stringWithUTF8String:c->GetProtocol().c_str()];
    NSString* ip        = [NSString stringWithUTF8String:c->GetIP().c_str()];
    NSString* universes = [NSString stringWithUTF8String:c->GetColumn3Label().c_str()];
    NSString* channels  = [NSString stringWithUTF8String:c->GetColumn4Label().c_str()];
    NSString* vendor    = [NSString stringWithUTF8String:c->GetVendor().c_str()];
    NSString* model     = [NSString stringWithUTF8String:c->GetModel().c_str()];
    NSString* variant   = [NSString stringWithUTF8String:c->GetVariant().c_str()];
    NSString* active    = [NSString stringWithUTF8String:c->GetColumn8Label().c_str()];
    NSString* desc      = [NSString stringWithUTF8String:c->GetDescription().c_str()];

    // OpenSourceFirmware gate. ControllerCaps may be null when the
    // vendor/model isn't in the iPad's bundled XML, in which case
    // we fail closed and don't surface Upload / Visualize.
    BOOL osf = NO;
    BOOL supportsUpload = NO;
    BOOL supportsInputUpload = NO;
    ControllerCaps* caps = ControllerCaps::GetControllerConfig(c);
    if (caps) {
        osf = caps->OpenSourceFirmware() ? YES : NO;
        supportsUpload = caps->SupportsUpload() ? YES : NO;
        supportsInputUpload = caps->SupportsInputOnlyUpload() ? YES : NO;
    }
    return @{
        @"name":         name,
        @"type":         type,
        @"protocol":     protocol,
        @"ip":           ip,
        @"universes":    universes,
        @"channels":     channels,
        @"vendor":       vendor,
        @"model":        model,
        @"variant":      variant,
        @"active":       active,
        @"autoLayout":   @(c->IsAutoLayout() ? YES : NO),
        @"autoSize":     @(c->IsAutoSize()   ? YES : NO),
        @"description":  desc,
        @"isFromBase":   @(c->IsFromBase() ? YES : NO),
        @"caps.openSourceFirmware":     @(osf),
        @"caps.supportsUpload":         @(supportsUpload),
        @"caps.supportsInputOnlyUpload":@(supportsInputUpload),
    };
}

- (NSArray<NSDictionary*>*)controllersListSummary {
    NSMutableArray<NSDictionary*>* out = [NSMutableArray array];
    if (!_context) return out;
    for (Controller* c : _context->GetOutputManager().GetControllers()) {
        if (!c) continue;
        [out addObject:BuildControllerSummary(c)];
    }
    return out;
}

- (nullable NSDictionary<NSString*, id>*)controllerDetailForName:(NSString*)name {
    if (!_context || !name) return nil;
    Controller* c = _context->GetOutputManager().GetController(name.UTF8String);
    if (!c) return nil;
    NSMutableDictionary* d = [BuildControllerSummary(c) mutableCopy];
    // Detail-only extras the list pane doesn't need.
    d[@"longDescription"] = [NSString stringWithUTF8String:c->GetLongDescription().c_str()];
    d[@"pingDescription"] = [NSString stringWithUTF8String:c->GetPingDescription().c_str()];
    // Construct a `http://<ip>/` URL for the Open action. The
    // long-press menu surfaces this even on non-OSF controllers
    // since plenty of fixtures still have a web UI.
    if (c->GetIP().size() > 0) {
        d[@"url"] = [NSString stringWithFormat:@"http://%s/",
                      c->GetIP().c_str()];
    }
    // FPP proxy (when set) — drives the detail-pane "Open Proxy" button.
    d[@"proxy"] = [NSString stringWithUTF8String:c->GetFPPProxy().c_str()];
    return d;
}

- (NSArray<NSString*>*)modelNamesForController:(NSString*)controllerName {
    NSMutableArray<NSString*>* out = [NSMutableArray array];
    if (!_context || !controllerName || !_context->HasModelManager()) return out;
    const std::string target = controllerName.UTF8String;
    auto& mgr = _context->GetModelManager();
    for (const auto& it : mgr) {
        Model* m = it.second;
        if (!m) continue;
        if (m->GetDisplayAs() == DisplayAsType::ModelGroup) continue;
        if (m->GetDisplayAs() == DisplayAsType::SubModel) continue;
        if (m->GetControllerName() == target) {
            [out addObject:[NSString stringWithUTF8String:m->GetName().c_str()]];
        }
    }
    return out;
}

- (nullable NSString*)controllerNameForFPPHost:(NSString*)host {
    if (!_context || host.length == 0) return nil;
    const std::string ip = host.UTF8String;
    auto matches = _context->GetOutputManager().GetControllers(ip);
    if (matches.empty()) return nil;
    return [NSString stringWithUTF8String:matches.front()->GetName().c_str()];
}

#pragma mark - J-31 — Controllers editable property descriptors

// J-31 — `Active` enum: index→string lookup matching desktop's
// `ACTIVETYPENAMES`. Order is load-bearing.
static NSArray<NSString*>* ControllerActiveOptions() {
    return @[@"Active", @"Inactive", @"xLights Only"];
}
static int EncodeControllerActive(const Controller* c) {
    switch (c->GetActive()) {
        case Controller::ACTIVESTATE::ACTIVE:               return 0;
        case Controller::ACTIVESTATE::INACTIVE:             return 1;
        case Controller::ACTIVESTATE::ACTIVEINXLIGHTSONLY:  return 2;
    }
    return 0;
}

// Encode a string into its index in `options`; returns -1 if
// missing (sheet will show the picker's first option).
static int IndexOfString(NSArray<NSString*>* options, const std::string& v) {
    NSString* needle = [NSString stringWithUTF8String:v.c_str()];
    NSUInteger i = [options indexOfObject:needle];
    return (i == NSNotFound) ? -1 : (int)i;
}

// J-31 — Serial-protocol picker options, mirroring desktop's
// `GetSerialProtocols` filter on `ControllerCaps::GetInputProtocols`.
// When caps are missing the full static list desktop initialises
// in `InitialiseSerialTypes` is returned.
static NSArray<NSString*>* SerialProtocolOptions(const ControllerSerial* ser) {
    NSMutableArray<NSString*>* out = [NSMutableArray array];
    ControllerCaps* caps = ControllerCaps::GetControllerConfig(ser);
    if (caps) {
        for (const auto& proto : caps->GetInputProtocols()) {
            if (proto == "dmx")              [out addObject:@(OUTPUT_DMX)];
            else if (proto == "lor")         [out addObject:@(OUTPUT_LOR)];
            else if (proto == "lor optimised")[out addObject:@(OUTPUT_LOR_OPT)];
            else if (proto == "opendmx")     [out addObject:@(OUTPUT_OPENDMX)];
            else if (proto == "pixelnet")    [out addObject:@(OUTPUT_PIXELNET)];
            else if (proto == "openpixelnet")[out addObject:@(OUTPUT_OPENPIXELNET)];
            else if (proto == "renard")      [out addObject:@(OUTPUT_RENARD)];
            else if (proto == "dlight")      [out addObject:@(OUTPUT_DLIGHT)];
            else if (proto == "generic serial") [out addObject:@(OUTPUT_GENERICSERIAL)];
            else if (proto == "ddp-input") {
                for (const auto& sp : caps->GetSerialProtocols()) {
                    [out addObject:[NSString stringWithUTF8String:sp.c_str()]];
                }
            }
        }
    }
    if (out.count == 0) {
        [out addObjectsFromArray:@[@(OUTPUT_DMX), @(OUTPUT_LOR), @(OUTPUT_LOR_OPT),
                                    @(OUTPUT_OPENDMX), @(OUTPUT_PIXELNET),
                                    @(OUTPUT_OPENPIXELNET), @(OUTPUT_RENARD),
                                    @(OUTPUT_DLIGHT), @(OUTPUT_GENERICSERIAL)]];
    }
    return out;
}

// J-31 — Serial baud-rate picker. Pulled from
// `SerialOutput::GetPossibleBaudRates()` so the list stays in
// sync with whatever desktop offers across rebuilds.
static NSArray<NSString*>* SerialBaudOptions() {
    NSMutableArray<NSString*>* out = [NSMutableArray array];
    for (const auto& v : SerialOutput::GetPossibleBaudRates()) {
        [out addObject:[NSString stringWithUTF8String:v.c_str()]];
    }
    return out;
}

// J-31 — Serial port picker (non-FPP path). Pulled from
// `SerialOutput::GetPossibleSerialPorts()`.
static NSArray<NSString*>* SerialPortOptions() {
    NSMutableArray<NSString*>* out = [NSMutableArray array];
    for (const auto& v : SerialOutput::GetPossibleSerialPorts()) {
        [out addObject:[NSString stringWithUTF8String:v.c_str()]];
    }
    return out;
}

// J-31 — FPP serial port enum (ttyS0-5 / ttyUSB0-5 / ttyACM0-5 /
// ttyAMA0 / i2c-1 / spidev0.0 / spidev0.1). Mirrors desktop
// `ControllerSerialPropertyAdapter::AddProperties` FPP branch
// (lines 122-133).
static NSArray<NSString*>* FppPortOptions() {
    NSMutableArray<NSString*>* out = [NSMutableArray array];
    for (int x = 0; x < 6; ++x) [out addObject:[NSString stringWithFormat:@"ttyS%d", x]];
    for (int x = 0; x < 6; ++x) [out addObject:[NSString stringWithFormat:@"ttyUSB%d", x]];
    for (int x = 0; x < 6; ++x) [out addObject:[NSString stringWithFormat:@"ttyACM%d", x]];
    [out addObject:@"ttyAMA0"];
    [out addObject:@"i2c-1"];
    [out addObject:@"spidev0.0"];
    [out addObject:@"spidev0.1"];
    return out;
}

// J-31 — I2C device address picker (0x00..0x7F).
static NSArray<NSString*>* I2cDeviceOptions() {
    NSMutableArray<NSString*>* out = [NSMutableArray array];
    for (int x = 0; x < 128; ++x) {
        [out addObject:[NSString stringWithFormat:@"0x%02X", x]];
    }
    return out;
}

static bool StringStartsWith(const std::string& s, const std::string& prefix) {
    return s.size() >= prefix.size()
        && s.compare(0, prefix.size(), prefix) == 0;
}

// Build the ethernet-protocol picker options from
// `ControllerCaps::GetInputProtocols()` when present; falls back
// to the desktop's hard-coded protocol set.
static NSArray<NSString*>* EthernetProtocolOptions(const ControllerEthernet* eth) {
    NSMutableArray<NSString*>* out = [NSMutableArray array];
    ControllerCaps* caps = ControllerCaps::GetControllerConfig(eth);
    if (caps) {
        for (const auto& proto : caps->GetInputProtocols()) {
            if (proto == "e131")        [out addObject:@(OUTPUT_E131)];
            else if (proto == "zcpp")   [out addObject:@(OUTPUT_ZCPP)];
            else if (proto == "artnet") [out addObject:@(OUTPUT_ARTNET)];
            else if (proto == "kinet")  [out addObject:@(OUTPUT_KINET)];
            else if (proto == "ddp")    [out addObject:@(OUTPUT_DDP)];
            else if (proto == "opc")    [out addObject:@(OUTPUT_OPC)];
            else if (proto == "player only") [out addObject:@(OUTPUT_PLAYER_ONLY)];
            else if (proto == "twinkly") [out addObject:@(OUTPUT_TWINKLY)];
        }
    }
    if (out.count == 0) {
        // No caps definition for this fixture — surface the
        // common set so the picker isn't blank.
        [out addObjectsFromArray:@[@(OUTPUT_E131), @(OUTPUT_ARTNET),
                                    @(OUTPUT_DDP),  @(OUTPUT_OPC),
                                    @(OUTPUT_ZCPP), @(OUTPUT_KINET),
                                    @(OUTPUT_TWINKLY)]];
    }
    return out;
}

static NSMutableDictionary* CtrlIntProp(NSString* key, NSString* label,
                                          int value, int minV, int maxV) {
    return [@{
        @"key": key, @"label": label, @"kind": @"int",
        @"value": @(value), @"min": @(minV), @"max": @(maxV),
    } mutableCopy];
}
static NSMutableDictionary* CtrlDoubleProp(NSString* key, NSString* label,
                                             double value, double minV, double maxV,
                                             double step, int precision) {
    return [@{
        @"key": key, @"label": label, @"kind": @"double",
        @"value": @(value), @"min": @(minV), @"max": @(maxV),
        @"step": @(step), @"precision": @(precision),
    } mutableCopy];
}
static NSMutableDictionary* CtrlBoolProp(NSString* key, NSString* label, BOOL value) {
    return [@{
        @"key": key, @"label": label, @"kind": @"bool",
        @"value": @(value ? YES : NO),
    } mutableCopy];
}
static NSMutableDictionary* CtrlEnumProp(NSString* key, NSString* label,
                                           int index, NSArray<NSString*>* options) {
    return [@{
        @"key": key, @"label": label, @"kind": @"enum",
        @"value": @(index), @"options": options,
    } mutableCopy];
}
static NSMutableDictionary* CtrlStringProp(NSString* key, NSString* label,
                                             NSString* _Nullable value, BOOL editable) {
    return [@{
        @"key": key, @"label": label, @"kind": @"string",
        @"value": value ?: @"",
        @"enabled": @(editable),
    } mutableCopy];
}
static NSMutableDictionary* CtrlHeader(NSString* key, NSString* label) {
    return [@{
        @"key": key, @"label": label, @"kind": @"header",
    } mutableCopy];
}

static NSArray<NSString*>* StdListToNSArray(const std::list<std::string>& list) {
    NSMutableArray<NSString*>* out = [NSMutableArray array];
    for (const auto& s : list) {
        [out addObject:[NSString stringWithUTF8String:s.c_str()]];
    }
    return out;
}

- (NSArray<NSDictionary*>*)controllerPropertiesForName:(NSString*)name {
    NSMutableArray<NSDictionary*>* out = [NSMutableArray array];
    if (!_context || !name) return out;
    Controller* c = _context->GetOutputManager().GetController(name.UTF8String);
    if (!c) return out;

    // === Base properties ===
    [out addObject:CtrlStringProp(@"Name", @"Name",
                                    [NSString stringWithUTF8String:c->GetName().c_str()],
                                    YES)];
    [out addObject:CtrlStringProp(@"Description", @"Description",
                                    [NSString stringWithUTF8String:c->GetDescription().c_str()],
                                    YES)];
    [out addObject:CtrlIntProp(@"Id", @"Id", c->GetId(), 0, 65535)];
    [out addObject:CtrlEnumProp(@"Active", @"Active",
                                  EncodeControllerActive(c),
                                  ControllerActiveOptions())];
    [out addObject:CtrlBoolProp(@"AutoLayout", @"Auto Layout Models",
                                  c->IsAutoLayout() ? YES : NO)];
    [out addObject:CtrlBoolProp(@"AutoUpload", @"Auto Upload Configuration",
                                  c->IsAutoUpload() ? YES : NO)];
    [out addObject:CtrlBoolProp(@"AutoSize", @"Auto Size",
                                  c->IsAutoSize() ? YES : NO)];
    [out addObject:CtrlBoolProp(@"FullxLightsControl", @"Full xLights Control",
                                  c->IsFullxLightsControl() ? YES : NO)];
    [out addObject:CtrlIntProp(@"DefaultBrightnessUnderFullxLightsControl",
                                 @"Default Port Brightness",
                                 c->GetDefaultBrightnessUnderFullControl(), 0, 100)];
    [out addObject:CtrlDoubleProp(@"DefaultGammaUnderFullxLightsControl",
                                    @"Default Port Gamma",
                                    (double)c->GetDefaultGammaUnderFullControl(),
                                    0.1, 5.0, 0.1, 2)];
    [out addObject:CtrlBoolProp(@"SuppressDuplicates", @"Suppress Duplicate Frames",
                                  c->IsSuppressDuplicateFrames() ? YES : NO)];
    [out addObject:CtrlBoolProp(@"Monitor", @"Monitor",
                                  c->IsMonitoring() ? YES : NO)];

    // === Vendor / Model / Variant cascade ===
    [out addObject:CtrlHeader(@"ControllerVendorHeader", @"Hardware")];
    auto const vendors = ControllerCaps::GetVendors(c->GetType());
    NSArray<NSString*>* vendorOpts = StdListToNSArray(vendors);
    int vendorIdx = IndexOfString(vendorOpts, c->GetVendor());
    [out addObject:CtrlEnumProp(@"Vendor", @"Vendor",
                                  std::max(0, vendorIdx),
                                  vendorOpts)];
    auto const models = ControllerCaps::GetModels(c->GetType(), c->GetVendor());
    NSArray<NSString*>* modelOpts = StdListToNSArray(models);
    if (modelOpts.count > 0) {
        int modelIdx = IndexOfString(modelOpts, c->GetModel());
        [out addObject:CtrlEnumProp(@"Model", @"Model",
                                      std::max(0, modelIdx),
                                      modelOpts)];
    }
    auto const variants = ControllerCaps::GetVariants(
        c->GetType(), c->GetVendor(), c->GetModel());
    NSArray<NSString*>* variantOpts = StdListToNSArray(variants);
    if (variantOpts.count > 0) {
        int variantIdx = IndexOfString(variantOpts, c->GetVariant());
        [out addObject:CtrlEnumProp(@"Variant", @"Variant",
                                      std::max(0, variantIdx),
                                      variantOpts)];
    }

    // === Subclass-specific ===
    if (auto* eth = dynamic_cast<ControllerEthernet*>(c)) {
        [out addObject:CtrlHeader(@"ControllerNetworkHeader", @"Network")];
        const std::string ip = eth->GetIP();
        [out addObject:CtrlBoolProp(@"Multicast", @"Multicast",
                                      ip == "MULTICAST" ? YES : NO)];
        [out addObject:CtrlStringProp(@"IP", @"IP Address",
                                        [NSString stringWithUTF8String:ip.c_str()],
                                        ip != "MULTICAST")];
        [out addObject:CtrlStringProp(@"FPPProxy", @"FPP Proxy IP/Hostname",
                                        [NSString stringWithUTF8String:eth->GetControllerFPPProxy().c_str()],
                                        YES)];
        NSArray<NSString*>* protoOpts = EthernetProtocolOptions(eth);
        int protoIdx = IndexOfString(protoOpts, eth->GetProtocol());
        [out addObject:CtrlEnumProp(@"Protocol", @"Protocol",
                                      std::max(0, protoIdx),
                                      protoOpts)];
        [out addObject:CtrlIntProp(@"Priority", @"Priority",
                                     eth->GetPriority(), 0, 100)];

        // Force Local IP — desktop ethernet adapter enum prop. The
        // empty option ("") means "no override"; the rest are the
        // host's local interface IPs. Index 0 is the empty slot.
        {
            auto const localIPs = ip_utils::GetLocalIPs();
            NSMutableArray<NSString*>* flipOpts = [NSMutableArray array];
            [flipOpts addObject:@""];
            for (const auto& lip : localIPs) {
                [flipOpts addObject:[NSString stringWithUTF8String:lip.c_str()]];
            }
            int flipIdx = IndexOfString(flipOpts, eth->GetControllerForceLocalIP());
            [out addObject:CtrlEnumProp(@"ForceLocalIP", @"Force Local IP",
                                          std::max(0, flipIdx), flipOpts)];
        }

        NSMutableDictionary* managed = CtrlBoolProp(@"Managed", @"Managed",
                                                      eth->IsManaged() ? YES : NO);
        managed[@"enabled"] = @NO;   // matches desktop read-only state
        [out addObject:managed];

        // === Output (per-universe) editing — E1.31 / ArtNet / KiNET ===
        // Mirrors desktop's ControllerEthernetPropertyAdapter universe
        // tree. The controller holds one Output per universe; Universe =
        // first output's number, Universes = output count, IndivSizes =
        // !AllSameSize, and (when uniform) a single Channels field.
        const std::string proto = eth->GetProtocol();
        if (proto == OUTPUT_E131 || proto == OUTPUT_ARTNET || proto == OUTPUT_KINET) {
            [out addObject:CtrlHeader(@"ControllerOutputHeader", @"Output")];
            auto const& outs = eth->GetOutputs();
            int startUniv = outs.empty() ? 1 : outs.front()->GetUniverse();
            const int maxUniv = (proto == OUTPUT_ARTNET) ? 32767 : 64000;
            [out addObject:CtrlIntProp(@"Universe", @"Start Universe",
                                         startUniv, 1, maxUniv)];
            [out addObject:CtrlIntProp(@"Universes", @"Universe Count",
                                         (int)outs.size(), 1, 100000)];
            [out addObject:CtrlBoolProp(@"UniversePerString", @"Universe Per String",
                                          eth->IsUniversePerString() ? YES : NO)];
            const bool indiv = !eth->AllSameSize();
            [out addObject:CtrlBoolProp(@"IndivSizes", @"Individual Sizes",
                                          indiv ? YES : NO)];
            if (!indiv) {
                int chans = outs.empty() ? 510 : outs.front()->GetChannels();
                [out addObject:CtrlIntProp(@"Channels", @"Channels per Universe",
                                             chans, 1, 512)];
            } else {
                // Individual per-universe channel sizes. Key encodes the
                // universe number; the setter looks up the output by it.
                for (const auto& o : outs) {
                    NSString* key = [NSString stringWithFormat:@"Channels/%d",
                                       o->GetUniverse()];
                    NSString* label = [NSString stringWithFormat:@"Universe %d",
                                         o->GetUniverse()];
                    [out addObject:CtrlIntProp(key, label,
                                                 o->GetChannels(), 1, 512)];
                }
            }
        }
    } else if (auto* nul = dynamic_cast<ControllerNull*>(c)) {
        [out addObject:CtrlHeader(@"ControllerNullHeader", @"Output")];
        [out addObject:CtrlIntProp(@"Channels", @"Channels",
                                     nul->GetChannels(), 1, 1000000)];
    } else if (auto* ser = dynamic_cast<ControllerSerial*>(c)) {
        ControllerCaps* serCaps = ControllerCaps::GetControllerConfig(ser);
        const std::string protocol = ser->GetProtocol();
        const int speed = ser->GetSpeed();
        const bool isFPP = (serCaps && serCaps->GetModel() == "FPP");
        [out addObject:CtrlHeader(@"ControllerSerialHeader",
                                    isFPP ? @"FPP Serial" : @"Serial")];

        if (isFPP) {
            // FPP encodes the port as "<ip>:<portName>". Split for
            // the UI so the two halves are independently editable;
            // the setter recomposes on commit.
            std::string port = ser->GetPort();
            std::string ip;
            std::string portStr = port;
            if (const auto colon = port.find(":"); colon != std::string::npos) {
                ip = port.substr(0, colon);
                portStr = port.substr(colon + 1);
            }
            [out addObject:CtrlStringProp(@"IP", @"IP Address",
                                            [NSString stringWithUTF8String:ip.c_str()],
                                            YES)];
            [out addObject:CtrlStringProp(@"FPPProxy", @"FPP Proxy IP/Hostname",
                                            [NSString stringWithUTF8String:ser->GetControllerFPPProxy().c_str()],
                                            YES)];

            NSArray<NSString*>* protoOpts = SerialProtocolOptions(ser);
            int protoIdx = IndexOfString(protoOpts, protocol);
            [out addObject:CtrlEnumProp(@"Protocol", @"Protocol",
                                          std::max(0, protoIdx), protoOpts)];

            NSArray<NSString*>* portOpts = FppPortOptions();
            int portIdx = IndexOfString(portOpts, portStr);
            [out addObject:CtrlEnumProp(@"Port", @"Port",
                                          std::max(0, portIdx), portOpts)];

            if (StringStartsWith(portStr, "tty")) {
                if (protocol != OUTPUT_DMX && protocol != OUTPUT_OPENDMX
                    && protocol != OUTPUT_PIXELNET && protocol != OUTPUT_OPENPIXELNET) {
                    NSArray<NSString*>* speeds = SerialBaudOptions();
                    NSString* speedStr = [NSString stringWithFormat:@"%d", speed];
                    int sidx = IndexOfString(speeds, speedStr.UTF8String);
                    [out addObject:CtrlEnumProp(@"Speed", @"Speed",
                                                  std::max(0, sidx), speeds)];
                }
            } else if (StringStartsWith(portStr, "i2c")) {
                NSArray<NSString*>* i2cs = I2cDeviceOptions();
                NSString* i2cStr = [NSString stringWithFormat:@"0x%02X", speed];
                int iidx = IndexOfString(i2cs, i2cStr.UTF8String);
                [out addObject:CtrlEnumProp(@"I2CDevice", @"I2C Device",
                                              std::max(0, iidx), i2cs)];
            } else if (StringStartsWith(portStr, "spidev")) {
                [out addObject:CtrlIntProp(@"SPISpeed", @"Speed (kHz)",
                                             speed, 0, 999999)];
            }
        } else {
            // Non-FPP serial: system-discovered ports + freeform
            // baud-rate picker. iPads don't have hardware serial
            // ports, but a show config loaded from desktop may
            // already have one defined; surface it as editable so
            // the user can at least adjust the protocol / speed.
            NSArray<NSString*>* portOpts = SerialPortOptions();
            int portIdx = IndexOfString(portOpts, ser->GetPort());
            if (portOpts.count == 0) {
                // Fall back to a freeform string when the host
                // can't enumerate any system serial ports.
                [out addObject:CtrlStringProp(@"Port", @"Port",
                                                [NSString stringWithUTF8String:ser->GetPort().c_str()],
                                                YES)];
            } else {
                [out addObject:CtrlEnumProp(@"Port", @"Port",
                                              std::max(0, portIdx), portOpts)];
            }

            NSArray<NSString*>* protoOpts = SerialProtocolOptions(ser);
            int protoIdx = IndexOfString(protoOpts, protocol);
            [out addObject:CtrlEnumProp(@"Protocol", @"Protocol",
                                          std::max(0, protoIdx), protoOpts)];

            NSArray<NSString*>* speeds = SerialBaudOptions();
            NSString* speedStr = [NSString stringWithFormat:@"%d", speed];
            int sidx = IndexOfString(speeds, speedStr.UTF8String);
            NSMutableDictionary* speedProp = CtrlEnumProp(
                @"Speed", @"Speed",
                std::max(0, sidx), speeds);
            // Some serial protocols fix the baud rate — disable the
            // picker in that case (matches desktop's grey-out).
            SerialOutput* sout = ser->GetSerialOutput();
            if (sout && !sout->AllowsBaudRateSetting()) {
                speedProp[@"enabled"] = @NO;
            }
            [out addObject:speedProp];
        }

        if (protocol == OUTPUT_GENERICSERIAL) {
            [out addObject:CtrlStringProp(@"Prefix", @"Prefix",
                                            [NSString stringWithUTF8String:ser->GetSaveablePreFix().c_str()],
                                            YES)];
            [out addObject:CtrlStringProp(@"Postfix", @"Postfix",
                                            [NSString stringWithUTF8String:ser->GetSaveablePostFix().c_str()],
                                            YES)];
        }

        // Channels — gated to read-only when AutoSize is on,
        // matching desktop's grey-out + tooltip.
        SerialOutput* sout = ser->GetSerialOutput();
        if (sout && sout->GetType() != OUTPUT_LOR_OPT) {
            int maxCh = sout->GetMaxChannels();
            if (serCaps) {
                maxCh = std::min(maxCh, serCaps->GetMaxSerialPortChannels());
            }
            if (maxCh <= 0) maxCh = 1000000;
            NSMutableDictionary* chProp = CtrlIntProp(
                @"Channels", @"Channels",
                ser->GetFirstOutput() ? (int)ser->GetFirstOutput()->GetChannels() : 0,
                1, maxCh);
            if (ser->IsAutoSize()) chProp[@"enabled"] = @NO;
            [out addObject:chProp];
        }
    }

    // === ControllerCaps extra properties ===
    if (ControllerCaps* caps = ControllerCaps::GetControllerConfig(c)) {
        auto const extras = caps->GetExtraPropertyDefs();
        if (!extras.empty()) {
            [out addObject:CtrlHeader(@"ControllerExtraHeader", @"Capabilities")];
            for (const auto& def : extras) {
                NSString* key = [NSString stringWithFormat:@"ControllerExtra.%s",
                                   def.name.c_str()];
                NSString* label = [NSString stringWithUTF8String:def.label.c_str()];
                std::string cur = c->GetExtraProperty(def.name, def.defaultValue);
                if (def.type == "Enum" && !def.values.empty()) {
                    NSMutableArray<NSString*>* options = [NSMutableArray array];
                    for (const auto& v : def.values) {
                        [options addObject:[NSString stringWithUTF8String:v.c_str()]];
                    }
                    int idx = IndexOfString(options, cur);
                    [out addObject:CtrlEnumProp(key, label,
                                                  std::max(0, idx), options)];
                } else {
                    [out addObject:CtrlStringProp(key, label,
                                                    [NSString stringWithUTF8String:cur.c_str()],
                                                    YES)];
                }
            }
        }
    }

    return out;
}

- (BOOL)setControllerProperty:(NSString*)key
                 onController:(NSString*)name
                        value:(id)value {
    if (!_context || !key || !name) return NO;
    Controller* c = _context->GetOutputManager().GetController(name.UTF8String);
    if (!c) return NO;
    const std::string k = key.UTF8String;
    BOOL changed = NO;

    if (k == "Name") {
        if (![value isKindOfClass:[NSString class]]) return NO;
        std::string newName = [(NSString*)value UTF8String];
        // Trim whitespace — matches desktop's
        // `event.GetValue().GetString().Trim(true).Trim(false)`.
        auto trim = [](std::string s) {
            const std::string ws = " \t\r\n";
            const auto a = s.find_first_not_of(ws);
            const auto b = s.find_last_not_of(ws);
            if (a == std::string::npos) return std::string();
            return s.substr(a, b - a + 1);
        };
        newName = trim(newName);
        if (newName.empty() || newName == NO_CONTROLLER) return NO;
        if (newName == c->GetName()) return NO;  // no change
        // Reject if another controller already has this name.
        if (_context->GetOutputManager().GetController(newName) != nullptr) return NO;
        c->SetName(newName);
        changed = YES;
    } else if (k == "Description") {
        if (![value isKindOfClass:[NSString class]]) return NO;
        c->SetDescription([(NSString*)value UTF8String]);
        changed = YES;
    } else if (k == "Id") {
        if (![value isKindOfClass:[NSNumber class]]) return NO;
        int v = [(NSNumber*)value intValue];
        if (c->GetId() != v) { c->SetId(v); changed = YES; }
    } else if (k == "Active") {
        if (![value isKindOfClass:[NSNumber class]]) return NO;
        int idx = [(NSNumber*)value intValue];
        NSArray* opts = ControllerActiveOptions();
        if (idx >= 0 && idx < (int)opts.count) {
            c->SetActive([(NSString*)opts[idx] UTF8String]);
            changed = YES;
        }
    } else if (k == "AutoLayout") {
        BOOL v = [(NSNumber*)value boolValue];
        if (c->IsAutoLayout() != (bool)v) {
            c->SetAutoLayout(v); changed = YES;
        }
    } else if (k == "AutoUpload") {
        BOOL v = [(NSNumber*)value boolValue];
        if (c->IsAutoUpload() != (bool)v) {
            c->SetAutoUpload(v); changed = YES;
        }
    } else if (k == "AutoSize") {
        BOOL v = [(NSNumber*)value boolValue];
        if (c->IsAutoSize() != (bool)v) {
            c->SetAutoSize(v, nullptr); changed = YES;
        }
    } else if (k == "FullxLightsControl") {
        BOOL v = [(NSNumber*)value boolValue];
        if (c->IsFullxLightsControl() != (bool)v) {
            c->SetFullxLightsControl(v); changed = YES;
        }
    } else if (k == "DefaultBrightnessUnderFullxLightsControl") {
        int v = [(NSNumber*)value intValue];
        if (c->GetDefaultBrightnessUnderFullControl() != v) {
            c->SetDefaultBrightnessUnderFullControl(v); changed = YES;
        }
    } else if (k == "DefaultGammaUnderFullxLightsControl") {
        double v = [(NSNumber*)value doubleValue];
        if (std::fabs((double)c->GetDefaultGammaUnderFullControl() - v) > 1e-4) {
            c->SetDefaultGammaUnderFullControl((float)v); changed = YES;
        }
    } else if (k == "SuppressDuplicates") {
        BOOL v = [(NSNumber*)value boolValue];
        if (c->IsSuppressDuplicateFrames() != (bool)v) {
            c->SetSuppressDuplicateFrames(v); changed = YES;
        }
    } else if (k == "Monitor") {
        BOOL v = [(NSNumber*)value boolValue];
        if (c->IsMonitoring() != (bool)v) {
            c->SetMonitoring(v); changed = YES;
        }
    } else if (k == "Vendor") {
        // Vendor change cascades to Model + Variant: desktop resets
        // Model + Variant when vendor changes, picking sensible
        // defaults only when there's exactly one option each.
        int idx = [(NSNumber*)value intValue];
        auto const vendors = ControllerCaps::GetVendors(c->GetType());
        auto it = vendors.begin();
        std::advance(it, idx);
        if (idx >= 0 && it != vendors.end()) {
            c->SetVendor(*it);
            auto models = ControllerCaps::GetModels(c->GetType(), *it);
            if (models.size() == 2) {
                c->SetModel(models.back());
                auto variants = ControllerCaps::GetVariants(
                    c->GetType(), *it, models.front());
                c->SetVariant(variants.size() == 2 ? variants.back() : "");
            } else {
                c->SetModel("");
                c->SetVariant("");
            }
            changed = YES;
        }
    } else if (k == "Model") {
        int idx = [(NSNumber*)value intValue];
        auto const models = ControllerCaps::GetModels(c->GetType(), c->GetVendor());
        auto it = models.begin();
        std::advance(it, idx);
        if (idx >= 0 && it != models.end()) {
            c->SetModel(*it);
            auto variants = ControllerCaps::GetVariants(
                c->GetType(), c->GetVendor(), *it);
            c->SetVariant(variants.empty() ? "" : variants.front());
            ControllerCaps* newCaps = ControllerCaps::GetControllerConfig(c);
            if (newCaps && newCaps->IsPlayerOnly() &&
                c->GetActive() == Controller::ACTIVESTATE::ACTIVE) {
                c->SetActive("xLights Only");
            }
            changed = YES;
        }
    } else if (k == "Variant") {
        int idx = [(NSNumber*)value intValue];
        auto const variants = ControllerCaps::GetVariants(
            c->GetType(), c->GetVendor(), c->GetModel());
        auto it = variants.begin();
        std::advance(it, idx);
        if (idx >= 0 && it != variants.end()) {
            c->SetVariant(*it);
            changed = YES;
        }
    }
    // Ethernet-specific
    else if (k == "Multicast") {
        auto* eth = dynamic_cast<ControllerEthernet*>(c);
        if (!eth) return NO;
        BOOL v = [(NSNumber*)value boolValue];
        if (v && eth->GetIP() != "MULTICAST") {
            eth->SetIP("MULTICAST"); changed = YES;
        } else if (!v && eth->GetIP() == "MULTICAST") {
            eth->SetIP("");  // user must enter a real IP next
            changed = YES;
        }
    } else if (k == "IP") {
        // Shared between Ethernet (direct IP string) and Serial
        // (FPP "<ip>:<port>" composite). Dispatch on the concrete
        // type — Ethernet first since it's the simpler case.
        if (auto* eth = dynamic_cast<ControllerEthernet*>(c)) {
            if (![value isKindOfClass:[NSString class]]) return NO;
            std::string v = [(NSString*)value UTF8String];
            if (eth->GetIP() != v) { eth->SetIP(v); changed = YES; }
        } else if (auto* ser = dynamic_cast<ControllerSerial*>(c)) {
            if (![value isKindOfClass:[NSString class]]) return NO;
            std::string ip = [(NSString*)value UTF8String];
            std::string currentPort = ser->GetPort();
            std::string tail;
            if (const auto colon = currentPort.find(":");
                colon != std::string::npos) {
                tail = currentPort.substr(colon + 1);
            } else {
                tail = currentPort;
            }
            const std::string composed = ip + ":" + tail;
            if (ser->GetPort() != composed) {
                ser->SetPort(composed); changed = YES;
            }
        } else {
            return NO;
        }
    } else if (k == "FPPProxy") {
        if (auto* eth = dynamic_cast<ControllerEthernet*>(c)) {
            if (![value isKindOfClass:[NSString class]]) return NO;
            std::string v = [(NSString*)value UTF8String];
            if (eth->GetControllerFPPProxy() != v) {
                eth->SetFPPProxy(v); changed = YES;
            }
        } else if (auto* ser = dynamic_cast<ControllerSerial*>(c)) {
            if (![value isKindOfClass:[NSString class]]) return NO;
            std::string v = [(NSString*)value UTF8String];
            if (ser->GetControllerFPPProxy() != v) {
                ser->SetFPPProxy(v); changed = YES;
            }
        } else {
            return NO;
        }
    } else if (k == "Protocol") {
        if (auto* eth = dynamic_cast<ControllerEthernet*>(c)) {
            NSArray<NSString*>* opts = EthernetProtocolOptions(eth);
            int idx = [(NSNumber*)value intValue];
            if (idx >= 0 && idx < (int)opts.count) {
                std::string newProto = [opts[idx] UTF8String];
                if (eth->GetProtocol() != newProto) {
                    eth->SetProtocol(newProto); changed = YES;
                }
            }
        } else if (auto* ser = dynamic_cast<ControllerSerial*>(c)) {
            NSArray<NSString*>* opts = SerialProtocolOptions(ser);
            int idx = [(NSNumber*)value intValue];
            if (idx < 0 || idx >= (int)opts.count) return NO;
            std::string newProto = [opts[idx] UTF8String];
            if (ser->GetProtocol() != newProto) {
                ser->SetProtocol(newProto); changed = YES;
            }
        } else {
            return NO;
        }
    } else if (k == "Priority") {
        auto* eth = dynamic_cast<ControllerEthernet*>(c);
        if (!eth) return NO;
        int v = [(NSNumber*)value intValue];
        if (eth->GetPriority() != v) { eth->SetPriority(v); changed = YES; }
    } else if (k == "ForceLocalIP") {
        auto* eth = dynamic_cast<ControllerEthernet*>(c);
        if (!eth) return NO;
        int idx = [(NSNumber*)value intValue];
        auto const localIPs = ip_utils::GetLocalIPs();
        std::string newIP;  // index 0 == "" (no override)
        if (idx > 0) {
            auto it = localIPs.begin();
            std::advance(it, idx - 1);
            if (it != localIPs.end()) newIP = *it;
        }
        if (eth->GetControllerForceLocalIP() != newIP) {
            eth->SetForceLocalIP(newIP); changed = YES;
        }
    } else if (k == "Universe") {
        auto* eth = dynamic_cast<ControllerEthernet*>(c);
        if (!eth) return NO;
        int univ = [(NSNumber*)value intValue];
        for (auto& o : eth->GetOutputs()) { o->SetUniverse(univ++); }
        changed = YES;
    } else if (k == "Universes") {
        auto* eth = dynamic_cast<ControllerEthernet*>(c);
        if (!eth) return NO;
        int want = [(NSNumber*)value intValue];
        if (want < 1) want = 1;
        while ((int)eth->GetOutputCount() < want) { eth->AddOutput(); }
        eth->RemoveTrailingOutputs(want);
        changed = YES;
    } else if (k == "UniversePerString") {
        auto* eth = dynamic_cast<ControllerEthernet*>(c);
        if (!eth) return NO;
        BOOL v = [(NSNumber*)value boolValue];
        if (eth->IsUniversePerString() != (bool)v) {
            eth->SetUniversePerString(v); changed = YES;
        }
    } else if (k == "IndivSizes") {
        auto* eth = dynamic_cast<ControllerEthernet*>(c);
        if (!eth) return NO;
        // `AllSameSize == !IndivSizes`. SetAllSameSize(true) collapses
        // every universe to the first universe's channel count.
        BOOL forceSizes = [(NSNumber*)value boolValue];
        eth->SetAllSameSize(!forceSizes, nullptr);
        changed = YES;
    } else if (k.rfind("Channels/", 0) == 0) {
        auto* eth = dynamic_cast<ControllerEthernet*>(c);
        if (!eth) return NO;
        const int univ = (int)std::strtol(k.c_str() + strlen("Channels/"), nullptr, 10);
        int v = [(NSNumber*)value intValue];
        for (auto& o : eth->GetOutputs()) {
            if (o->GetUniverse() == univ) { o->SetChannels(v); changed = YES; break; }
        }
    }
    // Null-specific
    else if (k == "Channels") {
        // Three controller subclasses surface a `Channels` key —
        // Null, Serial, and Ethernet (uniform per-universe size).
        // They don't conflict because each fixture is one type.
        auto* nul = dynamic_cast<ControllerNull*>(c);
        if (nul) {
            int v = [(NSNumber*)value intValue];
            if (nul->GetChannels() != v) {
                nul->SetChannelSize(v);
                changed = YES;
            }
        } else if (auto* ser = dynamic_cast<ControllerSerial*>(c)) {
            int v = [(NSNumber*)value intValue];
            if (ser->GetChannels() != v) {
                ser->SetChannels(v);
                changed = YES;
            }
        } else if (auto* eth = dynamic_cast<ControllerEthernet*>(c)) {
            int v = [(NSNumber*)value intValue];
            for (auto& o : eth->GetOutputs()) { o->SetChannels(v); }
            changed = YES;
        } else {
            return NO;
        }
    }
    // Serial-specific. `IP` / `FPPProxy` / `Protocol` are handled
    // up in the shared dispatcher because they overlap with
    // Ethernet keys; the remaining knobs (Port / Speed / I2CDevice
    // / SPISpeed / Prefix / Postfix) are unique to Serial.
    else if (k == "Port") {
        auto* ser = dynamic_cast<ControllerSerial*>(c);
        if (!ser) return NO;
        std::string newVal;
        if ([value isKindOfClass:[NSNumber class]]) {
            ControllerCaps* serCaps = ControllerCaps::GetControllerConfig(ser);
            const bool isFPP = (serCaps && serCaps->GetModel() == "FPP");
            NSArray<NSString*>* opts = isFPP ? FppPortOptions()
                                              : SerialPortOptions();
            int idx = [(NSNumber*)value intValue];
            if (idx < 0 || idx >= (int)opts.count) return NO;
            newVal = [opts[idx] UTF8String];
            if (isFPP) {
                // Re-attach the current IP prefix when present.
                std::string current = ser->GetPort();
                if (const auto colon = current.find(":");
                    colon != std::string::npos) {
                    newVal = current.substr(0, colon + 1) + newVal;
                }
            }
        } else if ([value isKindOfClass:[NSString class]]) {
            newVal = [(NSString*)value UTF8String];
        } else {
            return NO;
        }
        if (ser->GetPort() != newVal) {
            ser->SetPort(newVal); changed = YES;
        }
    } else if (k == "Speed") {
        auto* ser = dynamic_cast<ControllerSerial*>(c);
        if (!ser) return NO;
        NSArray<NSString*>* opts = SerialBaudOptions();
        int idx = [(NSNumber*)value intValue];
        if (idx < 0 || idx >= (int)opts.count) return NO;
        int baud = std::atoi([opts[idx] UTF8String]);
        if (ser->GetSpeed() != baud) {
            ser->SetSpeed(baud); changed = YES;
        }
    } else if (k == "I2CDevice") {
        auto* ser = dynamic_cast<ControllerSerial*>(c);
        if (!ser) return NO;
        NSArray<NSString*>* opts = I2cDeviceOptions();
        int idx = [(NSNumber*)value intValue];
        if (idx < 0 || idx >= (int)opts.count) return NO;
        int addr = (int)std::strtoul([opts[idx] UTF8String], nullptr, 16);
        if (ser->GetSpeed() != addr) {
            ser->SetSpeed(addr); changed = YES;
        }
    } else if (k == "SPISpeed") {
        auto* ser = dynamic_cast<ControllerSerial*>(c);
        if (!ser) return NO;
        int v = [(NSNumber*)value intValue];
        if (ser->GetSpeed() != v) {
            ser->SetSpeed(v); changed = YES;
        }
    } else if (k == "Prefix") {
        auto* ser = dynamic_cast<ControllerSerial*>(c);
        if (!ser) return NO;
        if (![value isKindOfClass:[NSString class]]) return NO;
        std::string v = [(NSString*)value UTF8String];
        if (ser->GetSaveablePreFix() != v) {
            ser->SetPrefix(v); changed = YES;
        }
    } else if (k == "Postfix") {
        auto* ser = dynamic_cast<ControllerSerial*>(c);
        if (!ser) return NO;
        if (![value isKindOfClass:[NSString class]]) return NO;
        std::string v = [(NSString*)value UTF8String];
        if (ser->GetSaveablePostFix() != v) {
            ser->SetPostfix(v); changed = YES;
        }
    }
    // ControllerCaps extras — key shape `ControllerExtra.<name>`.
    else if (k.rfind("ControllerExtra.", 0) == 0) {
        ControllerCaps* caps = ControllerCaps::GetControllerConfig(c);
        if (!caps) return NO;
        const std::string propName = k.substr(strlen("ControllerExtra."));
        for (const auto& def : caps->GetExtraPropertyDefs()) {
            if (def.name != propName) continue;
            std::string newVal;
            if (def.type == "Enum") {
                if (![value isKindOfClass:[NSNumber class]]) return NO;
                int idx = [(NSNumber*)value intValue];
                if (idx < 0 || idx >= (int)def.values.size()) return NO;
                newVal = def.values[idx];
            } else {
                if (![value isKindOfClass:[NSString class]]) return NO;
                newVal = [(NSString*)value UTF8String];
            }
            const std::string cur = c->GetExtraProperty(def.name, def.defaultValue);
            if (cur != newVal) {
                c->SetExtraProperty(def.name, newVal);
                changed = YES;
            }
            break;
        }
        if (!changed) return NO;
    } else {
        spdlog::warn("setControllerProperty: unknown key '{}' for controller '{}'",
                     k, name.UTF8String);
        return NO;
    }

    if (changed) {
        [self recalcAndMarkControllersDirty];
    }
    return changed;
}

- (nullable NSString*)controllerAutoSizeUniverseWarning:(NSString*)name {
    if (!_context || !name) return nil;
    auto& om = _context->GetOutputManager();
    Controller* c = om.GetController([name UTF8String]);
    if (!c || !c->IsAutoSize() || c->GetOutputCount() == 0) return nil;
    if (c->GetProtocol() != std::string("E131")) return nil;
    int universeSize = c->GetFirstOutput()->GetChannels();
    if (universeSize == 170 || universeSize == 510 || universeSize == 512) return nil;
    return [NSString stringWithFormat:
        @"The current Universe Size is %d.\n\nFor Auto Size to work correctly, you may "
         "need to update the Universe Size to a common value such as 510 or 512.",
        universeSize];
}

#pragma mark - J-31.3 — Controllers add / delete

- (nullable NSString*)addControllerOfType:(NSString*)type {
    if (!_context || !type) return nil;
    auto& om = _context->GetOutputManager();
    Controller* c = nullptr;
    NSString* t = type;
    if ([t isEqualToString:@"Ethernet"]) {
        c = new ControllerEthernet(&om);
    } else if ([t isEqualToString:@"Serial"]) {
        c = new ControllerSerial(&om);
    } else if ([t isEqualToString:@"Null"]) {
        c = new ControllerNull(&om);
    } else {
        return nil;
    }
    om.AddController(c, -1);
    [self recalcAndMarkControllersDirty];
    return [NSString stringWithUTF8String:c->GetName().c_str()];
}

- (BOOL)deleteController:(NSString*)name {
    if (!_context || !name) return NO;
    auto& om = _context->GetOutputManager();
    if (!om.GetController(name.UTF8String)) return NO;
    // Clear references from models that explicitly assign themselves
    // to this controller (either via `_controllerName` or a
    // "!<name>:<channel>" start channel) before tearing the
    // controller down — leftover references would otherwise leave
    // those models pointing at a missing controller, and the
    // subsequent start-channel recalc can't resolve them.
    if (_context->HasModelManager()) {
        _context->GetModelManager().DeleteController(name.UTF8String);
    }
    om.DeleteController(name.UTF8String);
    // Rework + Recalc (rather than Recalc alone): Rework rewrites
    // every remaining controller's "!Name:###" assignments so the
    // channel range freed by the deleted controller collapses,
    // and Recalc then refreshes each model's cached first channel
    // off the updated strings. Without the Rework pass models on
    // the remaining controllers keep stale channel numbers and the
    // synthetic "1" we just stamped on the unassigned models
    // doesn't get re-numbered into the "No Controller" tail.
    [self reworkAndRecalcStartChannels];
    _context->MarkControllersDirty();
    return YES;
}

- (BOOL)moveController:(NSString*)name toIndex:(int)destIndex {
    if (!_context || !name) return NO;
    auto& om = _context->GetOutputManager();
    Controller* c = om.GetController(name.UTF8String);
    if (!c) return NO;
    auto controllers = om.GetControllers();
    if (destIndex < 0 || destIndex >= (int)controllers.size()) return NO;
    om.MoveController(c, destIndex);
    [self recalcAndMarkControllersDirty];
    return YES;
}

- (BOOL)setControllerActiveState:(NSString*)state
                    onController:(NSString*)name {
    if (!_context || !state || !name) return NO;
    Controller* c = _context->GetOutputManager().GetController(name.UTF8String);
    if (!c) return NO;
    // FromBase controllers must be unlinked before activate/inactivate is meaningful.
    if (c->IsFromBase()) return NO;
    std::string s = [state UTF8String];
    if (s != "Active" && s != "xLights Only" && s != "Inactive") return NO;
    c->SetActive(s);
    c->PostSetActive();
    [self recalcAndMarkControllersDirty];
    return YES;
}

- (BOOL)unlinkControllerFromBase:(NSString*)name {
    if (!_context || !name) return NO;
    Controller* c = _context->GetOutputManager().GetController(name.UTF8String);
    if (!c || !c->IsFromBase()) return NO;
    c->SetFromBase(false);
    _context->MarkControllersDirty();
    return YES;
}

- (BOOL)unlinkModelFromBase:(NSString*)modelName {
    if (!_context || !_context->HasModelManager() || !modelName) return NO;
    auto& mgr = _context->GetModelManager();
    Model* m = mgr[std::string(modelName.UTF8String)];
    // Reject groups — use unlinkGroupFromBase: instead, which
    // routes through the <modelGroups> persistence path.
    if (!m || m->GetDisplayAs() == DisplayAsType::ModelGroup || !m->IsFromBase()) return NO;
    m->SetFromBase(false);
    _context->MarkLayoutModelDirty(std::string(modelName.UTF8String));
    return YES;
}

- (BOOL)unlinkGroupFromBase:(NSString*)groupName {
    if (!_context || !_context->HasModelManager() || !groupName) return NO;
    auto& mgr = _context->GetModelManager();
    Model* m = mgr[std::string(groupName.UTF8String)];
    if (!m || m->GetDisplayAs() != DisplayAsType::ModelGroup) return NO;
    ModelGroup* g = static_cast<ModelGroup*>(m);
    if (!g->IsFromBase()) return NO;
    g->SetFromBase(false);
    _context->MarkLayoutModelDirty(std::string(groupName.UTF8String));
    return YES;
}

#pragma mark - Base Show Directory

- (nullable NSString*)baseShowDirectory {
    if (!_context) return nil;
    const std::string& p = _context->GetOutputManager().GetBaseShowDir();
    if (p.empty()) return nil;
    return [NSString stringWithUTF8String:p.c_str()];
}

- (void)setBaseShowDirectory:(nullable NSString*)path {
    if (!_context) return;
    std::string s = path ? std::string([path UTF8String]) : std::string();
    auto& om = _context->GetOutputManager();
    if (om.GetBaseShowDir() == s) return;
    om.SetBaseShowDir(s);
    // Clearing the path also clears the auto-update flag — matches desktop.
    if (s.empty()) {
        om.SetAutoUpdateFromBaseShowDir(false);
    }
    _context->MarkControllersDirty();
}

- (BOOL)autoUpdateFromBaseShowDirectory {
    if (!_context) return NO;
    return _context->GetOutputManager().IsAutoUpdateFromBaseShowDir() ? YES : NO;
}

- (void)setAutoUpdateFromBaseShowDirectory:(BOOL)enabled {
    if (!_context) return;
    auto& om = _context->GetOutputManager();
    if (om.IsAutoUpdateFromBaseShowDir() == (bool)enabled) return;
    om.SetAutoUpdateFromBaseShowDir(enabled);
    _context->MarkControllersDirty();
}

- (NSDictionary*)updateFromBaseShowDirectory {
    if (!_context) {
        return @{ @"error": @"No show folder loaded.",
                  @"controllersChanged": @NO,
                  @"modelsChanged": @NO,
                  @"objectsChanged": @NO };
    }
    auto& om = _context->GetOutputManager();
    std::string baseDir = om.GetBaseShowDir();
    if (baseDir.empty()) {
        return @{ @"error": @"No base show folder configured.",
                  @"controllersChanged": @NO,
                  @"modelsChanged": @NO,
                  @"objectsChanged": @NO };
    }

    // iPad sandbox: a live security-scoped bookmark is required for any folder we read; reload it before the merge.
    if (!ObtainAccessToURL(baseDir, /*enforceWritable=*/false)) {
        return @{ @"error": @"Cannot access the base show folder. Please reselect it.",
                  @"needsReselect": @YES,
                  @"controllersChanged": @NO,
                  @"modelsChanged": @NO,
                  @"objectsChanged": @NO };
    }
    // Bookmark can outlive the folder it points at.
    std::error_code ec;
    if (!std::filesystem::exists(baseDir, ec)) {
        return @{ @"error": @"The base show folder no longer exists. Please reselect it.",
                  @"needsReselect": @YES,
                  @"controllersChanged": @NO,
                  @"modelsChanged": @NO,
                  @"objectsChanged": @NO };
    }

    // Shared across the three passes so Yes-to-All carries from controllers → models → objects.
    bool acceptAll = false;
    bool rejectAll = false;

    BOOL controllersChanged = NO;
    if (om.MergeFromBase(/*prompt=*/false, acceptAll, rejectAll, nullptr)) {
        controllersChanged = YES;
        [self recalcAndMarkControllersDirty];
    }

    BOOL modelsChanged = NO;
    if (_context->HasModelManager()) {
        if (_context->GetModelManager().MergeFromBase(baseDir, /*prompt=*/false,
                                                       acceptAll, rejectAll)) {
            modelsChanged = YES;
        }
    }

    BOOL objectsChanged = NO;
    if (_context->HasViewObjectManager()) {
        if (_context->GetAllObjects().MergeFromBase(baseDir, /*prompt=*/false,
                                                     acceptAll, rejectAll)) {
            objectsChanged = YES;
        }
    }

    return @{ @"controllersChanged": @(controllersChanged),
              @"modelsChanged": @(modelsChanged),
              @"objectsChanged": @(objectsChanged) };
}

// Desktop fires WORK_CALCULATE_START_CHANNELS via AddASAPWork
// after every model or controller mutation that can shift
// channel ranges; iPad has no work queue so the bridge runs
// the recalc inline at the end of each public mutator. Cheap
// even for big shows (one walk of the model graph) and
// idempotent — extra calls are harmless.
- (void)recalcModelStartChannels {
    if (_context && _context->HasModelManager()) {
        _context->GetModelManager().RecalcStartChannels();
    }
}

// Invariant: any public mutator that can shift a controller's
// channel ranges must pair these two calls before returning. The
// recalc walks the model graph; the dirty flag tells the iPad
// save layer the networks file needs to be rewritten.
- (void)recalcAndMarkControllersDirty {
    if (!_context) return;
    [self recalcModelStartChannels];
    _context->MarkControllersDirty();
}

// `Model::SetControllerName` queues WORK_MODELS_REWORK_STARTCHANNELS
// in addition to WORK_CALCULATE_START_CHANNELS — without that
// rework, a model whose controllerName was just changed keeps
// its old start channel (often outside the new controller's
// range), which makes `UDController::Rescan` silently drop it
// from any port. Visualize sees the model vanish from the new
// controller's wiring AND its old controller's wiring (it gets
// filtered out before either gets a chance to claim it). Call
// both methods from any flow that reassigns a model to a new
// controller / port.
- (void)reworkAndRecalcStartChannels {
    if (!_context || !_context->HasModelManager()) return;
    _context->GetModelManager().ReworkStartChannel();
    _context->GetModelManager().RecalcStartChannels();
}

#pragma mark - J-31.6 — Controller upload

// Minimal UICallbacks impl for upload. Default-yes for prompts
// (the user already authorized by tapping Upload), collects any
// status messages into a log string the bridge returns to
// Swift. File / directory / text input aren't reachable from
// the upload code paths we support today (FPP, WLED,
// ESPixelStick, ESPixelStickV4) — they're stubbed defensively.
class iPadUploadCallbacks : public UICallbacks {
public:
    std::string captured;

    void ShowMessage(const std::string& message,
                     const std::string& /*caption*/) const override {
        const_cast<iPadUploadCallbacks*>(this)->captured
            .append(message).append("\n");
    }
    bool PromptYesNo(const std::string& message,
                     const std::string& /*caption*/) const override {
        const_cast<iPadUploadCallbacks*>(this)->captured
            .append("[auto-yes] ").append(message).append("\n");
        return true;
    }
    std::string PromptForDirectory(const std::string& /*message*/,
                                    const std::string& /*defaultPath*/) const override {
        return "";
    }
    std::string PromptForFile(const std::string& /*message*/,
                               const std::string& /*wildcard*/,
                               const std::string& /*defaultPath*/) const override {
        return "";
    }
    long PromptForNumber(const std::string& /*message*/,
                          const std::string& /*caption*/,
                          long defaultValue,
                          long /*min*/, long /*max*/) const override {
        return defaultValue;
    }
    std::string PromptForText(const std::string& /*message*/,
                                const std::string& /*caption*/,
                                const std::string& defaultValue) const override {
        return defaultValue;
    }
    ProgressToken BeginProgress(const std::string& message,
                                 int /*maximum*/) override {
        captured.append(message).append("\n");
        return 1;
    }
    void UpdateProgress(ProgressToken /*token*/, int /*value*/,
                         const std::string& newMessage) override {
        if (!newMessage.empty()) {
            captured.append(newMessage).append("\n");
        }
    }
    void EndProgress(ProgressToken /*token*/) override {}
};

// Run either input- or output-upload for the named controller.
// Returns the standard `{success, message, log}` shape both
// public methods use.
- (NSDictionary*)runUpload:(NSString*)name input:(BOOL)isInputUpload {
    NSMutableDictionary* result = [@{
        @"success": @NO, @"message": @"", @"log": @"",
    } mutableCopy];
    if (!_context || !name) {
        result[@"message"] = @"Internal error: no render context.";
        return result;
    }
    auto& om = _context->GetOutputManager();
    Controller* c = om.GetController(name.UTF8String);
    if (!c) {
        result[@"message"] = @"Controller not found.";
        return result;
    }
    ControllerCaps* caps = ControllerCaps::GetControllerConfig(c);
    if (!caps) {
        result[@"message"] = @"Controller has no capabilities entry; "
                              @"upload is unavailable.";
        return result;
    }
    if (isInputUpload) {
        if (!caps->SupportsInputOnlyUpload()) {
            result[@"message"] = @"This controller does not support "
                                  @"input upload.";
            return result;
        }
    } else if (!caps->SupportsUpload()) {
        result[@"message"] = @"This controller does not support "
                              @"output upload.";
        return result;
    }
    std::string ip = c->GetResolvedIP(true);
    if (ip.empty() || ip == "MULTICAST") {
        result[@"message"] = @"This controller's IP isn't set (or is "
                              @"MULTICAST). Edit the IP in the "
                              @"property pane before uploading.";
        return result;
    }

    // Pre-flight: recompute model start channels so the upload
    // reflects current model assignments. Matches desktop's
    // `RecalcModels` call inside `UploadInputToController` /
    // `UploadOutputToController`.
    if (_context->HasModelManager()) {
        _context->GetModelManager().RecalcStartChannels();
    }

    iPadUploadCallbacks cbs;
    std::unique_ptr<BaseController> bc(BaseController::CreateBaseController(c, ip));
    if (!bc) {
        result[@"message"] = @"Unable to create a connection for this "
                              @"controller's vendor/model.";
        result[@"log"] = [NSString stringWithUTF8String:cbs.captured.c_str()];
        return result;
    }
    if (!bc->IsConnected()) {
        result[@"message"] = [NSString stringWithFormat:
            @"Could not connect to %s. Verify the IP and that the "
            @"controller is powered on and on the same network.",
            ip.c_str()];
        result[@"log"] = [NSString stringWithUTF8String:cbs.captured.c_str()];
        return result;
    }
    bool ok = false;
    if (isInputUpload) {
        ok = bc->SetInputUniverses(c, &cbs);
    } else {
        if (!_context->HasModelManager()) {
            result[@"message"] = @"Models aren't loaded; "
                                  @"can't compute output channels.";
            result[@"log"] = [NSString stringWithUTF8String:cbs.captured.c_str()];
            return result;
        }
        ok = bc->SetOutputs(&_context->GetModelManager(), &om, c, &cbs);
    }
    result[@"success"] = @(ok);
    NSString* label = isInputUpload ? @"Input" : @"Output";
    if (ok) {
        result[@"message"] = [NSString stringWithFormat:
            @"%@ upload complete.", label];
    } else {
        result[@"message"] = [NSString stringWithFormat:
            @"%@ upload failed. Check the log for details.", label];
    }
    result[@"log"] = [NSString stringWithUTF8String:cbs.captured.c_str()];
    return result;
}

- (NSDictionary*)uploadOutputForController:(NSString*)name {
    return [self runUpload:name input:NO];
}

- (NSDictionary*)uploadInputForController:(NSString*)name {
    return [self runUpload:name input:YES];
}

- (NSDictionary*)validateProxyForController:(NSString*)name {
    NSMutableDictionary* r = [@{
        @"hasProxy": @NO, @"valid": @YES, @"proxy": @"", @"to": @"",
    } mutableCopy];
    if (!_context || !name) return r;
    Controller* c = _context->GetOutputManager().GetController(name.UTF8String);
    if (!c) return r;
    const std::string proxy = c->GetFPPProxy();
    if (proxy.empty()) return r;
    const std::string to = c->GetIP();
    r[@"hasProxy"] = @YES;
    r[@"proxy"] = [NSString stringWithUTF8String:proxy.c_str()];
    r[@"to"]    = [NSString stringWithUTF8String:to.c_str()];
    r[@"valid"] = @(FPP::ValidateProxy(to, proxy) ? YES : NO);
    return r;
}

- (NSString*)pingController:(NSString*)name {
    if (!_context || !name) return @"unknown";
    Controller* c = _context->GetOutputManager().GetController(name.UTF8String);
    if (!c) return @"unknown";
    if (!c->CanPing()) return @"unavailable";
    // Shared core ping — an HTTP reachability probe on non-Windows
    // hosts (CurlManager::HTTPSGet), so it works inside the iOS
    // sandbox without raw ICMP.
    Output::PINGSTATE state = c->Ping();
    switch (state) {
        case Output::PINGSTATE::PING_OK:        return @"ok";
        case Output::PINGSTATE::PING_WEBOK:     return @"webok";
        case Output::PINGSTATE::PING_OPEN:      return @"open";
        case Output::PINGSTATE::PING_OPENED:    return @"open";
        case Output::PINGSTATE::PING_ALLFAILED: return @"failed";
        case Output::PINGSTATE::PING_UNAVAILABLE: return @"unavailable";
        default:                                return @"unknown";
    }
}

- (NSArray<NSDictionary*>*)bulkUploadControllersWithProgress:(void (^)(NSString*, NSInteger, NSInteger))onProgress {
    NSMutableArray<NSDictionary*>* results = [NSMutableArray array];
    if (!_context) return results;
    auto& om = _context->GetOutputManager();

    // Collect open-source-firmware, active controllers that support
    // any upload — matches the single-controller gate. Closed-firmware
    // vendors are skipped silently (restricted/IAP tier on iPad).
    std::vector<std::string> targets;
    for (auto* c : om.GetControllers()) {
        if (!c || !c->IsActive()) continue;
        ControllerCaps* caps = ControllerCaps::GetControllerConfig(c);
        if (!caps || !caps->OpenSourceFirmware()) continue;
        if (!caps->SupportsUpload() && !caps->SupportsInputOnlyUpload()) continue;
        targets.push_back(c->GetName());
    }

    // Recalc start channels once up front so every per-controller
    // upload sees consistent model assignments.
    if (_context->HasModelManager()) {
        _context->GetModelManager().RecalcStartChannels();
    }

    const NSInteger total = (NSInteger)targets.size();
    for (NSInteger i = 0; i < total; ++i) {
        NSString* cn = [NSString stringWithUTF8String:targets[i].c_str()];
        if (onProgress) onProgress(cn, i, total);

        Controller* c = om.GetController(targets[i]);
        ControllerCaps* caps = c ? ControllerCaps::GetControllerConfig(c) : nullptr;
        NSMutableArray<NSString*>* msgs = [NSMutableArray array];
        NSMutableArray<NSString*>* logs = [NSMutableArray array];
        BOOL anyAttempt = NO, anySuccess = NO, anyFailure = NO;
        if (caps && caps->SupportsInputOnlyUpload()) {
            NSDictionary* r = [self runUpload:cn input:YES];
            anyAttempt = YES;
            if ([r[@"success"] boolValue]) anySuccess = YES; else anyFailure = YES;
            [msgs addObject:[NSString stringWithFormat:@"Input: %@", r[@"message"]]];
            if ([r[@"log"] length]) [logs addObject:r[@"log"]];
        }
        if (caps && caps->SupportsUpload()) {
            NSDictionary* r = [self runUpload:cn input:NO];
            anyAttempt = YES;
            if ([r[@"success"] boolValue]) anySuccess = YES; else anyFailure = YES;
            [msgs addObject:[NSString stringWithFormat:@"Output: %@", r[@"message"]]];
            if ([r[@"log"] length]) [logs addObject:r[@"log"]];
        }
        [results addObject:@{
            @"name":    cn,
            @"success": @(anyAttempt && anySuccess && !anyFailure),
            @"message": [msgs componentsJoinedByString:@"\n"],
            @"log":     [logs componentsJoinedByString:@"\n"],
        }];
    }
    return results;
}

- (NSDictionary*)runControllerDiscovery {
    NSMutableArray<NSString*>* addedNames = [NSMutableArray array];
    NSMutableArray<NSDictionary*>* mismatches = [NSMutableArray array];
    NSInteger already = 0;
    if (!_context) {
        return @{@"added": @0, @"already": @0,
                 @"addedNames": addedNames, @"mismatches": mismatches};
    }
    auto& om = _context->GetOutputManager();

    // Construct a single Discovery instance and register every
    // protocol scanner desktop's PrepareAllControllerDiscovery
    // wires up. FPP needs an optional list of forced IPs — we
    // pass an empty list (the iPad has no "FPPConnectForcedIPs"
    // preference today; broadcast discovery is enough for the
    // common case).
    DiscoveryDelegate defaultDelegate;
    Discovery discovery(&om, &defaultDelegate);
    std::list<std::string> emptyForcedAddresses;
    FPP::PrepareDiscovery(discovery, emptyForcedAddresses);
    ArtNetOutput::PrepareDiscovery(discovery);
    TwinklyOutput::PrepareDiscovery(discovery);
    Pixlite16::PrepareDiscovery(discovery);
    DDPOutput::PrepareDiscovery(discovery);
    WLED::PrepareDiscovery(discovery);

    discovery.Discover();

    // Walk each result. Auto-add when there's no conflict;
    // otherwise capture the conflict for the SwiftUI side to
    // resolve via `applyDiscoveryMismatch:action:`. Mismatch
    // classification mirrors desktop's `OnButtonDiscoverClick`
    // dispatch (TabSetup.cpp:1534):
    //   • IP exact match + same name → already.
    //   • IP exact match + different name (single existing
    //     with same protocol) → rename mismatch.
    //   • IP doesn't match + same name + same protocol on an
    //     existing fixture whose IP is a numeric (not hostname)
    //     address → ip-update mismatch.
    //   • Neither → auto-add.
    for (DiscoveredData* d : discovery.GetResults()) {
        if (!d || !d->controller) continue;
        ControllerEthernet* eth = d->controller;

        // Existing controller(s) with this IP?
        auto byIP = om.GetControllers(eth->GetIP());
        if (!byIP.empty()) {
            // IP collision. If the names also match, treat as
            // already-known. If they differ and the protocols
            // match, surface a rename prompt.
            ControllerEthernet* existing =
                dynamic_cast<ControllerEthernet*>(byIP.front());
            if (existing
                && existing->GetName() != eth->GetName()
                && existing->GetProtocol() == eth->GetProtocol()
                && byIP.size() == 1) {
                NSString* mid = [[NSUUID UUID] UUIDString];
                [mismatches addObject:@{
                    @"id":             mid,
                    @"kind":           @"rename",
                    @"existingName":   [NSString stringWithUTF8String:existing->GetName().c_str()],
                    @"existingIP":     [NSString stringWithUTF8String:existing->GetIP().c_str()],
                    @"discoveredName": [NSString stringWithUTF8String:eth->GetName().c_str()],
                }];
            } else {
                ++already;
            }
            continue;
        }

        // No IP match — search for a name+protocol match. When
        // found and the existing IP isn't already a hostname
        // (which would be authoritative), surface an ip-update
        // prompt. Existing hostnames are left alone because
        // desktop treats them as user-managed.
        ControllerEthernet* nameMatch = nullptr;
        for (Controller* itc : om.GetControllers()) {
            auto* other = dynamic_cast<ControllerEthernet*>(itc);
            if (other
                && other->GetName() == eth->GetName()
                && other->GetProtocol() == eth->GetProtocol()) {
                nameMatch = other;
                break;
            }
        }
        if (nameMatch != nullptr) {
            if (ip_utils::IsValidHostname(nameMatch->GetIP())) {
                // Existing IP is a hostname — assume the user
                // configured it explicitly; treat as known.
                ++already;
                continue;
            }
            NSString* mid = [[NSUUID UUID] UUIDString];
            [mismatches addObject:@{
                @"id":            mid,
                @"kind":          @"ip-update",
                @"existingName":  [NSString stringWithUTF8String:nameMatch->GetName().c_str()],
                @"existingIP":    [NSString stringWithUTF8String:nameMatch->GetIP().c_str()],
                @"discoveredIP":  [NSString stringWithUTF8String:eth->GetIP().c_str()],
                @"discoveredName":[NSString stringWithUTF8String:eth->GetName().c_str()],
                @"protocol":      [NSString stringWithUTF8String:eth->GetProtocol().c_str()],
                @"vendor":        [NSString stringWithUTF8String:eth->GetVendor().c_str()],
                @"model":         [NSString stringWithUTF8String:eth->GetModel().c_str()],
                @"variant":       [NSString stringWithUTF8String:eth->GetVariant().c_str()],
            }];
            continue;
        }

        // Auto-add path (no conflict).
        eth->EnsureUniqueId();
        eth->EnsureUniqueName();
        if (d->typeId > 0 && d->typeId < 0x80) {
            eth->SetActive("xLights Only");
            if (eth->GetVendor().empty()) {
                eth->SetVendor("FPP");
            }
        }
        om.AddController(eth);
        [addedNames addObject:[NSString stringWithUTF8String:eth->GetName().c_str()]];
        d->controller = nullptr;  // ownership now belongs to OutputManager
    }
    if (addedNames.count > 0) {
        _context->MarkControllersDirty();
    }
    return @{
        @"added":      @(addedNames.count),
        @"already":    @(already),
        @"addedNames": addedNames,
        @"mismatches": mismatches,
    };
}

- (BOOL)applyDiscoveryMismatch:(NSDictionary*)descriptor
                         action:(NSString*)action {
    if (!_context || !descriptor || !action) return NO;
    auto& om = _context->GetOutputManager();
    NSString* kind = descriptor[@"kind"];
    NSString* existingName = descriptor[@"existingName"];
    if (!kind || !existingName) return NO;

    // "skip" is always a valid no-op — saves the SwiftUI side
    // from special-casing it before iterating.
    if ([action isEqualToString:@"skip"]) return YES;

    if ([kind isEqualToString:@"ip-update"]) {
        NSString* discoveredIP   = descriptor[@"discoveredIP"];
        NSString* discoveredName = descriptor[@"discoveredName"];
        if (!discoveredIP) return NO;

        if ([action isEqualToString:@"update"]) {
            // Find by name; bail if it's been renamed/deleted
            // since discovery ran (rare but possible).
            Controller* c = om.GetController(existingName.UTF8String);
            auto* eth = dynamic_cast<ControllerEthernet*>(c);
            if (!eth) return NO;
            eth->SetIP([discoveredIP UTF8String]);
            [self recalcAndMarkControllersDirty];
            return YES;
        } else if ([action isEqualToString:@"add-new"]) {
            // Reconstruct a ControllerEthernet from the captured
            // scalars; AddController's `EnsureUniqueName` path
            // will resolve any name collision.
            ControllerEthernet* newEth = new ControllerEthernet(&om);
            if (NSString* p = descriptor[@"protocol"]; p.length > 0) {
                newEth->SetProtocol([p UTF8String]);
            }
            newEth->SetIP([discoveredIP UTF8String]);
            if (NSString* v = descriptor[@"vendor"]; v.length > 0) {
                newEth->SetVendor([v UTF8String]);
            }
            if (NSString* m = descriptor[@"model"]; m.length > 0) {
                newEth->SetModel([m UTF8String]);
            }
            if (NSString* v = descriptor[@"variant"]; v.length > 0) {
                newEth->SetVariant([v UTF8String]);
            }
            if (discoveredName.length > 0) {
                newEth->SetName([discoveredName UTF8String]);
            }
            newEth->EnsureUniqueId();
            newEth->EnsureUniqueName();
            om.AddController(newEth);
            [self recalcAndMarkControllersDirty];
            return YES;
        }
        return NO;
    }

    if ([kind isEqualToString:@"rename"]) {
        if (![action isEqualToString:@"rename"]) return NO;
        NSString* discoveredName = descriptor[@"discoveredName"];
        if (discoveredName.length == 0) return NO;
        Controller* c = om.GetController(existingName.UTF8String);
        if (!c) return NO;
        // Refuse if the new name already exists — would silently
        // collapse two distinct controllers.
        if (om.GetController(discoveredName.UTF8String)) return NO;

        const std::string oldName = c->GetName();
        const std::string newName = [discoveredName UTF8String];
        c->SetName(newName);
        // Rewrite every model's controllerName so existing
        // assignments stay valid. Mirrors desktop's `renames`
        // map walk in OnButtonDiscoverClick (TabSetup.cpp:1602).
        if (_context->HasModelManager()) {
            auto& mgr = _context->GetModelManager();
            for (const auto& it : mgr) {
                Model* m = it.second;
                if (m && m->GetControllerName() == oldName) {
                    m->SetControllerName(newName);
                }
            }
        }
        [self recalcAndMarkControllersDirty];
        return YES;
    }
    return NO;
}

#pragma mark - J-32.1 — Controllers Visualize (read-only wiring)

// Build a per-model NSDictionary matching the wiringForController:
// schema. Used for both port-attached models and the
// no-connection bucket — pass `pm == nullptr` for no-connection
// models where only the raw Model is known.
static NSDictionary* BuildWiringModelEntry(UDControllerPortModel* pm,
                                            Model* fallback,
                                            Controller* controller,
                                            const ControllerCaps* caps) {
    Model* m = pm ? pm->GetModel() : fallback;
    NSMutableDictionary* d = [NSMutableDictionary dictionary];
    if (m) {
        d[@"name"]  = [NSString stringWithUTF8String:m->GetName().c_str()];
        d[@"label"] = [NSString stringWithUTF8String:m->GetName().c_str()];
    } else {
        d[@"name"]  = @"";
        d[@"label"] = @"";
    }
    if (pm) {
        d[@"string"]               = @(pm->GetString());
        d[@"startChannel"]         = @((long long)pm->GetStartChannel());
        d[@"endChannel"]           = @((long long)pm->GetEndChannel());
        d[@"channels"]             = @((long long)pm->Channels());
        d[@"smartRemote"]          = @(pm->GetSmartRemote());
        char letter = pm->GetSmartRemoteLetter();
        d[@"smartRemoteLetter"]    = letter ?
            [NSString stringWithFormat:@"%c", letter] : @"";
        d[@"smartRemoteType"]      = [NSString stringWithUTF8String:pm->GetSmartRemoteType().c_str()];
        d[@"universe"]             = @(pm->GetUniverse());
        d[@"universeStartChannel"] = @(pm->GetUniverseStartChannel());
        d[@"protocol"]             = [NSString stringWithUTF8String:pm->GetProtocol().c_str()];

        std::string reason;
        const bool valid = pm->Check(controller, caps, reason);
        d[@"valid"]         = @(valid ? YES : NO);
        d[@"invalidReason"] = [NSString stringWithUTF8String:reason.c_str()];
    } else if (m) {
        d[@"string"]               = @(0);
        d[@"startChannel"]         = @((long long)m->GetFirstChannel() + 1);
        d[@"endChannel"]           = @((long long)m->GetLastChannel() + 1);
        d[@"channels"]             = @((long long)(m->GetLastChannel() - m->GetFirstChannel() + 1));
        d[@"smartRemote"]          = @(m->GetSmartRemote());
        d[@"smartRemoteLetter"]    = @"";
        d[@"smartRemoteType"]      = [NSString stringWithUTF8String:m->GetSmartRemoteType().c_str()];
        d[@"universe"]             = @(0);
        d[@"universeStartChannel"] = @(0);
        d[@"protocol"]             = [NSString stringWithUTF8String:m->GetControllerProtocol().c_str()];
        d[@"valid"]                = @(NO);
        d[@"invalidReason"]        = @"Model is not assigned to a port on this controller";
    }
    return d;
}

static NSString* PortKindFromTypeString(const std::string& type) {
    if (type == "Pixel")            return @"pixel";
    if (type == "Serial")           return @"serial";
    if (type == "PWM")              return @"pwm";
    if (type == "Virtual Matrix")   return @"virtualMatrix";
    if (type == "LED Panel")        return @"ledPanelMatrix";
    return [NSString stringWithUTF8String:type.c_str()];
}

static NSString* PortDisplayName(const std::string& type, int portNum) {
    if (type == "Pixel")           return [NSString stringWithFormat:@"Pixel Port %d", portNum];
    if (type == "Serial")          return [NSString stringWithFormat:@"Serial Port %d", portNum];
    if (type == "PWM")             return [NSString stringWithFormat:@"PWM Port %d", portNum];
    if (type == "Virtual Matrix")  return [NSString stringWithFormat:@"Virtual Matrix %d", portNum];
    if (type == "LED Panel")       return [NSString stringWithFormat:@"LED Panel %d", portNum];
    return [NSString stringWithFormat:@"%s %d", type.c_str(), portNum];
}

static NSDictionary* BuildWiringPortEntry(UDControllerPort* port,
                                           Controller* controller,
                                           const ControllerCaps* caps,
                                           bool isPixel) {
    if (!port) return @{};
    const std::string type = port->GetType();
    NSMutableArray<NSDictionary*>* modelEntries = [NSMutableArray array];
    for (UDControllerPortModel* pm : port->GetModels()) {
        if (!pm) continue;
        [modelEntries addObject:BuildWiringModelEntry(pm, nullptr, controller, caps)];
    }
    // Skip the per-port Check on empty ports — Check reports
    // "Invalid protocol on pixel port N:" for any port whose
    // _protocol is empty, which is always true for ports the
    // user hasn't dropped a model onto yet. The empty case is
    // shown as a "Drop here" target in the UI, not as a
    // hardware-validity error.
    bool valid = true;
    std::string reason;
    if (port->GetModelCount() > 0 && caps) {
        valid = port->Check(controller, isPixel, caps, reason);
    }
    return @{
        @"kind":              PortKindFromTypeString(type),
        @"port":              @(port->GetPort()),
        @"name":              PortDisplayName(type, port->GetPort()),
        @"protocol":          [NSString stringWithUTF8String:port->GetProtocol().c_str()],
        @"valid":             @(valid ? YES : NO),
        @"invalidReason":     [NSString stringWithUTF8String:reason.c_str()],
        @"isSmartRemotePort": @(port->IsSmartRemotePort() ? YES : NO),
        @"smartRemoteCount":  @(port->GetSmartRemoteCount()),
        @"startChannel":      @((long long)port->GetStartChannel()),
        @"endChannel":        @((long long)port->GetEndChannel()),
        @"channels":          @((long long)port->Channels()),
        @"pixels":            @(port->Pixels()),
        @"models":            modelEntries,
    };
}

// Placeholder for a port that exists in caps but has no models
// today. Built directly so we don't auto-create a UDControllerPort
// in `_pixelPorts` — those empty entries pollute `UDController::
// Check`'s "Pixel ports only support a single protocol" check,
// which compares each port's protocol against the first non-
// empty one. An empty placeholder protocol ("" != "ws2811")
// would spuriously trigger that error.
static NSDictionary* BuildEmptyPortEntry(NSString* kind,
                                          int portNum,
                                          const std::string& typeStr) {
    return @{
        @"kind":              kind,
        @"port":              @(portNum),
        @"name":              PortDisplayName(typeStr, portNum),
        @"protocol":          @"",
        @"valid":             @YES,
        @"invalidReason":     @"",
        @"isSmartRemotePort": @NO,
        @"smartRemoteCount":  @0,
        @"startChannel":      @0,
        @"endChannel":        @0,
        @"channels":          @0,
        @"pixels":            @0,
        @"models":            @[],
    };
}

- (nullable NSDictionary*)wiringForController:(NSString*)name {
    if (!_context || !name || !_context->HasModelManager()) return nil;
    auto& om = _context->GetOutputManager();
    auto& mm = _context->GetModelManager();
    Controller* c = om.GetController(name.UTF8String);
    if (!c) return nil;

    UDController cud(c, &om, &mm, /*eliminateOverlaps=*/false);
    ControllerCaps* caps = ControllerCaps::GetControllerConfig(c);

    // Run the controller-level Check FIRST, before we touch any
    // `GetController*Port` lookups. The auto-create path in
    // those getters silently inserts empty `UDControllerPort`
    // entries with `_protocol == ""`, which then break
    // `UDController::Check`'s "single protocol per port-kind"
    // detector (it compares each entry's protocol against the
    // first non-empty one). Snapshotting `Check` while
    // `_pixelPorts` / `_serialPorts` only hold model-bearing
    // entries gives us the validity decision the desktop dialog
    // would show.
    std::string checkReason;
    const bool overallValid = cud.Check(caps, checkReason);

    NSMutableArray<NSDictionary*>* ports = [NSMutableArray array];

    // Caps holds the hardware's actual port count; show every
    // one (even empty) so the user can drag onto them. Fall
    // back to the UDController's max when caps are missing.
    const int capsPixel  = caps ? caps->GetMaxPixelPort()  : 0;
    const int capsSerial = caps ? caps->GetMaxSerialPort() : 0;
    const int hiPixel  = std::max(capsPixel,  cud.GetMaxPixelPort());
    const int hiSerial = std::max(capsSerial, cud.GetMaxSerialPort());
    for (int p = 1; p <= hiPixel; ++p) {
        if (cud.HasPixelPort(p)) {
            [ports addObject:BuildWiringPortEntry(cud.GetControllerPixelPort(p), c, caps, /*pixel*/ true)];
        } else {
            [ports addObject:BuildEmptyPortEntry(@"pixel", p, "Pixel")];
        }
    }
    for (int p = 1; p <= hiSerial; ++p) {
        if (cud.HasSerialPort(p)) {
            [ports addObject:BuildWiringPortEntry(cud.GetControllerSerialPort(p), c, caps, /*pixel*/ false)];
        } else {
            [ports addObject:BuildEmptyPortEntry(@"serial", p, "Serial")];
        }
    }
    // PWM / Virtual Matrix / LED Panel Matrix don't have caps-
    // side max counts the same way; only show ports that exist.
    for (int p = 1; p <= cud.GetMaxPWMPort(); ++p) {
        if (cud.HasPWMPort(p)) {
            [ports addObject:BuildWiringPortEntry(cud.GetControllerPWMPort(p), c, caps, false)];
        }
    }
    for (int p = 1; p <= cud.GetMaxVirtualMatrixPort(); ++p) {
        if (cud.HasVirtualMatrixPort(p)) {
            [ports addObject:BuildWiringPortEntry(cud.GetControllerVirtualMatrixPort(p), c, caps, false)];
        }
    }
    for (int p = 1; p <= cud.GetMaxLEDPanelMatrixPort(); ++p) {
        if (cud.HasLEDPanelMatrixPort(p)) {
            [ports addObject:BuildWiringPortEntry(cud.GetControllerLEDPanelMatrixPort(p), c, caps, false)];
        }
    }

    NSMutableArray<NSDictionary*>* noConn = [NSMutableArray array];
    for (Model* m : cud.GetNoConnectionModels()) {
        if (!m) continue;
        [noConn addObject:BuildWiringModelEntry(nullptr, m, c, caps)];
    }

    // Drag source: every concrete model in the show that ISN'T
    // already on a port of this controller (and isn't already in
    // the no-connection bucket). Mirrors desktop's right-side
    // "Models" pane in ControllerModelDialog. Excludes model
    // groups + submodels — only top-level models map cleanly to
    // a controller port.
    std::set<std::string> assigned;
    for (NSDictionary* p in ports) {
        for (NSDictionary* m in p[@"models"]) {
            if (NSString* n = m[@"name"]) {
                assigned.insert(n.UTF8String);
            }
        }
    }
    for (NSDictionary* m in noConn) {
        if (NSString* n = m[@"name"]) {
            assigned.insert(n.UTF8String);
        }
    }
    NSMutableArray<NSDictionary*>* available = [NSMutableArray array];
    for (const auto& [name, m] : mm.GetModels()) {
        if (!m) continue;
        if (m->GetDisplayAs() == DisplayAsType::ModelGroup) continue;
        if (m->GetDisplayAs() == DisplayAsType::SubModel)   continue;
        if (assigned.count(m->GetName())) continue;
        NSMutableDictionary* d = [NSMutableDictionary dictionary];
        d[@"name"]            = [NSString stringWithUTF8String:m->GetName().c_str()];
        d[@"channels"]        = @((long long)(m->GetLastChannel() - m->GetFirstChannel() + 1));
        d[@"controllerName"]  = [NSString stringWithUTF8String:m->GetControllerName().c_str()];
        d[@"controllerPort"]  = @(m->GetControllerPort(1));
        d[@"protocol"]        = [NSString stringWithUTF8String:m->GetControllerProtocol().c_str()];
        [available addObject:d];
    }

    long long totalModels = 0;
    long long totalChannels = 0;
    for (NSDictionary* p in ports) {
        totalModels   += [p[@"models"] count];
        totalChannels += [p[@"channels"] longLongValue];
    }

    return @{
        @"name":         [NSString stringWithUTF8String:c->GetName().c_str()],
        @"ip":           [NSString stringWithUTF8String:c->GetIP().c_str()],
        @"vendor":       [NSString stringWithUTF8String:c->GetVendor().c_str()],
        @"model":        [NSString stringWithUTF8String:c->GetModel().c_str()],
        @"variant":      [NSString stringWithUTF8String:c->GetVariant().c_str()],
        @"valid":        @(overallValid ? YES : NO),
        @"errorMessage": [NSString stringWithUTF8String:checkReason.c_str()],
        @"ports":        ports,
        @"noConnection": noConn,
        @"availableModels": available,
        @"totals": @{
            @"models":      @(totalModels),
            @"channels":    @(totalChannels),
            @"pixelPorts":  @(cud.GetMaxPixelPort()),
            @"serialPorts": @(cud.GetMaxSerialPort()),
        },
    };
}

#pragma mark - J-32.2 — Visualize: per-port protocol picker

- (NSArray<NSString*>*)availableProtocolsForController:(NSString*)name
                                                  kind:(NSString*)kind {
    NSMutableArray<NSString*>* out = [NSMutableArray array];
    if (!_context || !name || !kind) return out;
    Controller* c = _context->GetOutputManager().GetController(name.UTF8String);
    if (!c) return out;
    ControllerCaps* caps = ControllerCaps::GetControllerConfig(c);

    std::vector<std::string> protos;
    if ([kind isEqualToString:@"pixel"]) {
        protos = caps
            ? GetAllPixelTypes(caps->GetPixelProtocols(),
                                /*includeSerial*/ false,
                                /*includeArtificial*/ true,
                                /*includeMatrices*/ false)
            : GetAllPixelTypes(/*includeSerial*/ false,
                                /*includeArtificial*/ true,
                                /*includeMatrices*/ true);
    } else if ([kind isEqualToString:@"serial"]) {
        protos = caps
            ? GetAllSerialTypes(caps->GetSerialProtocols())
            : GetAllSerialTypes();
    } else {
        return out;
    }
    for (const auto& p : protos) {
        [out addObject:[NSString stringWithUTF8String:p.c_str()]];
    }
    return out;
}

- (BOOL)setPortProtocolOnController:(NSString*)name
                                kind:(NSString*)kind
                                port:(int)port
                            protocol:(NSString*)protocol {
    if (!_context || !name || !kind || !protocol) return NO;
    if (![kind isEqualToString:@"pixel"] && ![kind isEqualToString:@"serial"]) return NO;
    Controller* c = _context->GetOutputManager().GetController(name.UTF8String);
    if (!c) return NO;

    NSArray<NSString*>* valid =
        [self availableProtocolsForController:name kind:kind];
    if (![valid containsObject:protocol]) return NO;

    const std::string proto = protocol.UTF8String;
    auto& om = _context->GetOutputManager();
    auto& mm = _context->GetModelManager();
    UDController cud(c, &om, &mm, /*eliminateOverlaps=*/false);
    ControllerCaps* caps = ControllerCaps::GetControllerConfig(c);

    // Hardware that can't mix protocols across ports of the same
    // kind has to apply the new protocol everywhere of that kind.
    // Falcon F16 et al. are the common case here.
    const bool applyToAllPorts =
        caps && !caps->SupportsMultipleSimultaneousOutputProtocols();

    bool changed = false;
    auto applyToPort = [&](UDControllerPort* p) {
        if (!p) return;
        for (UDControllerPortModel* pm : p->GetModels()) {
            if (!pm || !pm->GetModel()) continue;
            if (pm->GetModel()->GetControllerProtocol() != proto) {
                pm->GetModel()->SetControllerProtocol(proto);
                _context->MarkLayoutModelDirty(pm->GetModel()->GetName());
                changed = true;
            }
        }
    };

    if ([kind isEqualToString:@"pixel"]) {
        if (applyToAllPorts) {
            for (int p = 1; p <= cud.GetMaxPixelPort(); ++p) {
                applyToPort(cud.GetControllerPixelPort(p));
            }
        } else {
            applyToPort(cud.GetControllerPixelPort(port));
        }
    } else {
        if (applyToAllPorts) {
            for (int p = 1; p <= cud.GetMaxSerialPort(); ++p) {
                applyToPort(cud.GetControllerSerialPort(p));
            }
        } else {
            applyToPort(cud.GetControllerSerialPort(port));
        }
    }

    if (changed) {
        [self recalcModelStartChannels];
    }
    return changed;
}

#pragma mark - J-32.3 — Visualize: per-model controller-connection reader

- (nullable NSDictionary*)controllerConnectionForModel:(NSString*)modelName {
    if (!_context || !modelName || !_context->HasModelManager()) return nil;
    Model* m = _context->GetModelManager()[modelName.UTF8String];
    if (!m) return nil;
    ControllerConnection& cc = m->GetCtrlConn();

    NSMutableArray<NSString*>* colorOrderOptions = [NSMutableArray array];
    for (const auto& s : Model::CONTROLLER_COLORORDER) {
        [colorOrderOptions addObject:[NSString stringWithUTF8String:s.c_str()]];
    }
    const std::string curOrder = cc.GetColorOrder();
    int curOrderIdx = -1;
    for (int i = 0; i < (int)Model::CONTROLLER_COLORORDER.size(); ++i) {
        if (Model::CONTROLLER_COLORORDER[i] == curOrder) { curOrderIdx = i; break; }
    }

    NSMutableArray<NSString*>* srTypeOptions = [NSMutableArray array];
    for (const auto& s : cc.GetSmartRemoteTypes()) {
        [srTypeOptions addObject:[NSString stringWithUTF8String:s.c_str()]];
    }

    return @{
        @"brightnessActive": @(cc.IsPropertySet(ControllerConnection::BRIGHTNESS_ACTIVE) ? YES : NO),
        @"brightness":       @(cc.GetBrightness()),
        @"gammaActive":      @(cc.IsPropertySet(ControllerConnection::GAMMA_ACTIVE) ? YES : NO),
        @"gamma":            @(cc.GetGamma()),
        @"colorOrderActive": @(cc.IsPropertySet(ControllerConnection::COLOR_ORDER_ACTIVE) ? YES : NO),
        @"colorOrderIndex":  @(curOrderIdx),
        @"colorOrder":       [NSString stringWithUTF8String:curOrder.c_str()],
        @"colorOrderOptions": colorOrderOptions,
        @"groupCountActive": @(cc.IsPropertySet(ControllerConnection::GROUP_COUNT_ACTIVE) ? YES : NO),
        @"groupCount":       @(cc.GetGroupCount()),
        @"startNullsActive": @(cc.IsPropertySet(ControllerConnection::START_NULLS_ACTIVE) ? YES : NO),
        @"startNulls":       @(cc.GetStartNulls()),
        @"endNullsActive":   @(cc.IsPropertySet(ControllerConnection::END_NULLS_ACTIVE) ? YES : NO),
        @"endNulls":         @(cc.GetEndNulls()),
        @"dmxChannel":       @(cc.GetDMXChannel()),
        @"useSmartRemote":   @(cc.IsPropertySet(ControllerConnection::USE_SMART_REMOTE) ? YES : NO),
        @"smartRemote":      @(cc.GetSmartRemote()),
        @"smartRemoteType":  [NSString stringWithUTF8String:cc.GetSmartRemoteType().c_str()],
        @"smartRemoteTypeOptions": srTypeOptions,
        @"srMaxCascade":     @(cc.GetSRMaxCascade()),
        @"srCascadeOnPort":  @(cc.GetSRCascadeOnPort() ? YES : NO),
    };
}

#pragma mark - J-32.5 — Visualize: drag-drop model assignment

// Find the UDControllerPort matching kind + port number. Returns
// nullptr when kind is unknown or the port isn't reachable.
static UDControllerPort* GetUDPortForKind(UDController& cud,
                                           NSString* portKind,
                                           int port) {
    if ([portKind isEqualToString:@"pixel"])         return cud.GetControllerPixelPort(port);
    if ([portKind isEqualToString:@"serial"])        return cud.GetControllerSerialPort(port);
    if ([portKind isEqualToString:@"pwm"])           return cud.GetControllerPWMPort(port);
    if ([portKind isEqualToString:@"virtualMatrix"]) return cud.GetControllerVirtualMatrixPort(port);
    if ([portKind isEqualToString:@"ledPanelMatrix"]) return cud.GetControllerLEDPanelMatrixPort(port);
    return nullptr;
}

- (BOOL)assignModelToController:(NSString*)modelName
                  controllerName:(NSString*)controllerName
                            kind:(NSString*)portKind
                            port:(int)port
                      afterModel:(nullable NSString*)afterModelName
                     smartRemote:(int)smartRemote {
    if (!_context || !modelName || !controllerName || !portKind) return NO;
    if (!_context->HasModelManager()) return NO;
    auto& om = _context->GetOutputManager();
    auto& mm = _context->GetModelManager();

    Controller* c = om.GetController(controllerName.UTF8String);
    if (!c) return NO;
    Model* m = mm[modelName.UTF8String];
    if (!m) return NO;

    _context->AbortRender(5000);

    // Build the wiring tree BEFORE we mutate so we can read the
    // current chain on the destination port + look up the
    // "after" model and the model that's currently chained
    // immediately after it. UDController is re-built below by
    // RecalcStartChannels, so this snapshot is single-use.
    UDController cud(c, &om, &mm, /*eliminateOverlaps=*/false);
    UDControllerPort* dstPort = GetUDPortForKind(cud, portKind, port);
    if (!dstPort) return NO;
    ControllerCaps* caps = ControllerCaps::GetControllerConfig(c);

    Model* afterModel = nullptr;
    if (afterModelName && afterModelName.length > 0) {
        afterModel = mm[afterModelName.UTF8String];
    }

    // Whoever is currently chained AFTER the target needs to
    // re-resolve their chain head to point at the newly-inserted
    // model. Capture before we mutate.
    Model* afterNext = (afterModel != nullptr)
        ? dstPort->GetModelAfter(afterModel)
        : nullptr;

    // 1) Controller + port assignment.
    m->SetControllerName(c->GetName());
    m->SetControllerPort(port);

    // 2) Protocol — empty port picks a sensible default from
    //    caps; non-empty port inherits the existing chain's
    //    protocol so the user doesn't end up with a port mixing
    //    ws2811 and apa102.
    if ([portKind isEqualToString:@"pixel"]) {
        if (dstPort->GetModelCount() == 0) {
            if (caps && !caps->GetPixelProtocols().empty()) {
                const auto pxs = caps->GetPixelProtocols();
                if (std::find(pxs.begin(), pxs.end(),
                              m->GetControllerProtocol()) == pxs.end()) {
                    m->SetControllerProtocol(pxs.front());
                }
            } else if (!m->IsPixelProtocol()) {
                m->SetControllerProtocol("ws2811");
            }
        } else if (afterModel) {
            m->SetControllerProtocol(afterModel->GetControllerProtocol());
        }
        if (caps && !caps->SupportsSmartRemotes()) {
            m->SetSmartRemote(0);
        }
    } else if ([portKind isEqualToString:@"serial"]) {
        if (dstPort->GetModelCount() == 0) {
            if (caps && !caps->GetSerialProtocols().empty()) {
                const auto sps = caps->GetSerialProtocols();
                if (std::find(sps.begin(), sps.end(),
                              m->GetControllerProtocol()) == sps.end()) {
                    m->SetControllerProtocol(sps.front());
                }
            } else if (!m->IsSerialProtocol()) {
                m->SetControllerProtocol("dmx");
            }
            if (m->GetControllerDMXChannel() == 0) {
                m->SetControllerDMXChannel(1);
            }
        } else if (afterModel) {
            m->SetControllerProtocol(afterModel->GetControllerProtocol());
        }
        m->SetSmartRemote(0);
    } else if ([portKind isEqualToString:@"pwm"]) {
        m->SetControllerProtocol("PWM");
        m->SetSmartRemote(0);
    } else if ([portKind isEqualToString:@"virtualMatrix"]) {
        m->SetControllerProtocol("Virtual Matrix");
        m->SetSmartRemote(0);
    } else if ([portKind isEqualToString:@"ledPanelMatrix"]) {
        m->SetControllerProtocol("LED Panel Matrix");
        m->SetSmartRemote(0);
    }

    // 3) Smart-remote inheritance. -1 = inherit from afterModel
    //    (or leave the model's own value when there's no chain
    //    target); >= 0 = explicit override.
    if ([portKind isEqualToString:@"pixel"]) {
        if (smartRemote >= 0) {
            m->SetSmartRemote(smartRemote);
        } else if (afterModel) {
            m->SetSmartRemote(afterModel->GetSmartRemote());
        }
    }

    // 4) Chain. Pixel uses modelChain string ("@PrevModel:1");
    //    serial uses controllerDMXChannel arithmetic.
    if ([portKind isEqualToString:@"pixel"]) {
        if (afterModel) {
            // Insert after `afterModel`: it stays where it was;
            // `m` chains off it; whatever was chained off
            // `afterModel` now chains off `m`.
            m->SetModelChain(">" + afterModel->GetName());
            if (afterNext) {
                afterNext->SetModelChain(">" + m->GetName());
            }
        } else if (dstPort->GetModelCount() > 0) {
            // No after-target but port non-empty → append at end.
            if (auto* tail = dstPort->GetLastModel()) {
                if (tail->GetModel() && tail->GetModel() != m) {
                    m->SetModelChain(">" + tail->GetModel()->GetName());
                }
            }
        } else {
            m->SetModelChain("");
        }
    } else if ([portKind isEqualToString:@"serial"]) {
        // Walk forward from `m` (now in the chain at the desired
        // position), pushing each downstream model's DMX channel
        // forward when it would overlap. Matches the desktop's
        // rhs=true serial branch.
        if (afterModel) {
            int nextCh = afterModel->GetControllerDMXChannel() + afterModel->GetChanCount();
            m->SetControllerDMXChannel(nextCh);
            m->SetModelChain("");
            Model* prev = m;
            Model* nxt = afterNext;
            while (nxt) {
                int needCh = prev->GetControllerDMXChannel() + prev->GetChanCount();
                if (nxt->GetControllerDMXChannel() < needCh) {
                    nxt->SetControllerDMXChannel(needCh);
                }
                nxt->SetModelChain("");
                prev = nxt;
                // dstPort is stale once we mutate channels, but
                // GetModelAfter only reads ordered list, which
                // hasn't been re-sorted; safe to keep walking.
                nxt = dstPort->GetModelAfter(nxt);
            }
        }
    }

    _context->MarkLayoutModelDirty(m->GetName());
    if (afterNext) _context->MarkLayoutModelDirty(afterNext->GetName());
    [self reworkAndRecalcStartChannels];
    return YES;
}

- (BOOL)removeModelFromController:(NSString*)modelName {
    if (!_context || !modelName || !_context->HasModelManager()) return NO;
    auto& mm = _context->GetModelManager();
    Model* m = mm[modelName.UTF8String];
    if (!m) return NO;

    _context->AbortRender(5000);

    // Anyone chained off this model loses their anchor — clear
    // their chain so they fall back to whatever start-channel
    // pattern was on them before, mirroring desktop's
    // CONTROLLER_REMOVEPORTMODELS behaviour.
    const std::string victim = m->GetName();
    for (const auto& [_, other] : mm.GetModels()) {
        if (!other || other == m) continue;
        const std::string chain = other->GetModelChain();
        if (!chain.empty() && chain.size() > 1 && chain[0] == '>' &&
            chain.substr(1) == victim) {
            other->SetModelChain("");
            _context->MarkLayoutModelDirty(other->GetName());
        }
    }

    m->SetControllerPort(0);
    m->SetModelChain("");
    if (m->GetControllerName() != "") {
        m->SetControllerName(NO_CONTROLLER);
    }
    _context->MarkLayoutModelDirty(m->GetName());
    [self reworkAndRecalcStartChannels];
    return YES;
}

#pragma mark - J-32.7 — Visualize: wiring export

// Quote a CSV cell when it contains a comma, quote, or newline.
// Mirrors RFC 4180.
static std::string CSVQuote(const std::string& s) {
    bool needsQuote = false;
    for (char ch : s) {
        if (ch == ',' || ch == '"' || ch == '\n' || ch == '\r') {
            needsQuote = true;
            break;
        }
    }
    if (!needsQuote) return s;
    std::string out = "\"";
    for (char ch : s) {
        if (ch == '"') out += "\"\"";
        else out += ch;
    }
    out += "\"";
    return out;
}

- (nullable NSString*)exportWiringCSVForController:(NSString*)name {
    if (!_context || !name || !_context->HasModelManager()) return nil;
    auto& om = _context->GetOutputManager();
    Controller* c = om.GetController(name.UTF8String);
    if (!c) return nil;
    auto& mm = _context->GetModelManager();
    UDController cud(c, &om, &mm, /*eliminateOverlaps=*/false);

    using namespace ExportSettings;
    const SETTINGS settings = static_cast<SETTINGS>(
        SETTINGS_PORT_ABSADDRESS |
        SETTINGS_PORT_CHANNELS |
        SETTINGS_PORT_PIXELS |
        SETTINGS_MODEL_DESCRIPTIONS |
        SETTINGS_MODEL_ABSADDRESS |
        SETTINGS_MODEL_CHANNELS |
        SETTINGS_MODEL_PIXELS);
    int columnSize = 0;
    auto rows = cud.ExportAsCSV(settings, /*brightness=*/100.0f, columnSize);

    std::string out;
    out.reserve(rows.size() * 64);
    for (const auto& row : rows) {
        for (size_t i = 0; i < row.size(); ++i) {
            if (i > 0) out += ',';
            out += CSVQuote(row[i]);
        }
        out += '\n';
    }
    return [NSString stringWithUTF8String:out.c_str()];
}

- (nullable NSString*)exportWiringJSONForController:(NSString*)name {
    if (!_context || !name || !_context->HasModelManager()) return nil;
    auto& om = _context->GetOutputManager();
    Controller* c = om.GetController(name.UTF8String);
    if (!c) return nil;
    auto& mm = _context->GetModelManager();
    UDController cud(c, &om, &mm, /*eliminateOverlaps=*/false);
    return [NSString stringWithUTF8String:cud.ExportAsJSON().c_str()];
}

- (NSDictionary*)portCountsForController:(NSString*)name {
    if (!_context || !name) return @{@"maxPixelPort": @0, @"maxSerialPort": @0};
    Controller* c = _context->GetOutputManager().GetController(name.UTF8String);
    if (!c) return @{@"maxPixelPort": @0, @"maxSerialPort": @0};
    ControllerCaps* caps = ControllerCaps::GetControllerConfig(c);
    if (!caps) return @{@"maxPixelPort": @0, @"maxSerialPort": @0};
    return @{
        @"maxPixelPort":  @(caps->GetMaxPixelPort()),
        @"maxSerialPort": @(caps->GetMaxSerialPort()),
    };
}

- (NSDictionary*)smartRemoteCapabilitiesForController:(NSString*)name {
    if (!_context || !name) {
        return @{@"supportsSmartRemotes": @NO,
                  @"maxRemotes": @0,
                  @"types": @[]};
    }
    Controller* c = _context->GetOutputManager().GetController(name.UTF8String);
    ControllerCaps* caps = c ? ControllerCaps::GetControllerConfig(c) : nullptr;
    if (!caps || !caps->SupportsSmartRemotes()) {
        return @{@"supportsSmartRemotes": @NO,
                  @"maxRemotes": @0,
                  @"types": @[]};
    }
    NSMutableArray<NSString*>* types = [NSMutableArray array];
    for (const auto& s : caps->GetSmartRemoteTypes()) {
        [types addObject:[NSString stringWithUTF8String:s.c_str()]];
    }
    return @{
        @"supportsSmartRemotes": @YES,
        @"maxRemotes":           @(caps->GetSmartRemoteCount()),
        @"types":                types,
    };
}

#pragma mark - FPP Connect (Slice A)

// Iframe-Keychain-backed DiscoveryDelegate for FPP Connect.
// One instance per document (constructed lazily) — shared across
// every FPP* the discovery walk produces. Stored creds live in the
// iOS Keychain under service "xLights/Discovery/<ip>" + account =
// username (typically "admin"). 401 prompts hop to main, run a
// caller-supplied ObjC block, and block the discovery thread on a
// DispatchSemaphore until the user dismisses.
class XLiPadDiscoveryAuthDelegate : public DiscoveryDelegate {
public:
    XLiPadDiscoveryAuthDelegate() = default;
    ~XLiPadDiscoveryAuthDelegate() override = default;

    void SetPromptHandler(XLFPPAuthPromptHandler handler) {
        // ObjC ARC: copy the block so it survives past the calling
        // scope (the caller may release theirs).
        _handler = [handler copy];
    }

    bool PromptForPassword(const std::string& host, std::string& username,
                           std::string& password, bool& savePassword) override {
        XLFPPAuthPromptHandler handler = _handler;
        if (!handler) return false;

        NSString* hostNS = [NSString stringWithUTF8String:host.c_str()];

        dispatch_semaphore_t sem = dispatch_semaphore_create(0);
        __block NSString* userOut = nil;
        __block NSString* pwdOut = nil;
        __block BOOL saveOut = NO;

        XLFPPAuthPromptCompletion completion = ^(NSString* user, NSString* pwd, BOOL save) {
            userOut = [user copy];
            pwdOut = [pwd copy];
            saveOut = save;
            dispatch_semaphore_signal(sem);
        };

        // Always present from the main queue — UIAlertController
        // requires it, and we don't know what thread the FPP
        // class's curl callback ran us on.
        dispatch_async(dispatch_get_main_queue(), ^{
            handler(hostNS, completion);
        });

        dispatch_semaphore_wait(sem, DISPATCH_TIME_FOREVER);

        if (userOut == nil || pwdOut == nil) {
            // User cancelled.
            return false;
        }
        username = userOut.UTF8String;
        password = pwdOut.UTF8String;
        savePassword = (saveOut == YES);
        return true;
    }

    bool GetStoredPassword(const std::string& service, std::string& user, std::string& pwd) override {
        @autoreleasepool {
            NSString* svc = [NSString stringWithFormat:@"xLights/Discovery/%s",
                             service.c_str()];
            NSMutableDictionary* query = [NSMutableDictionary dictionary];
            query[(__bridge id)kSecClass] = (__bridge id)kSecClassGenericPassword;
            query[(__bridge id)kSecAttrService] = svc;
            query[(__bridge id)kSecReturnAttributes] = @YES;
            query[(__bridge id)kSecReturnData] = @YES;
            query[(__bridge id)kSecMatchLimit] = (__bridge id)kSecMatchLimitOne;

            CFTypeRef result = NULL;
            OSStatus status = SecItemCopyMatching((__bridge CFDictionaryRef)query, &result);
            if (status != errSecSuccess || result == NULL) {
                return false;
            }
            NSDictionary* item = (__bridge_transfer NSDictionary*)result;
            NSString* account = item[(__bridge id)kSecAttrAccount];
            NSData* pwdData = item[(__bridge id)kSecValueData];
            if (account == nil || pwdData == nil) return false;
            user = account.UTF8String;
            NSString* pwdStr = [[NSString alloc] initWithData:pwdData
                                                     encoding:NSUTF8StringEncoding];
            if (pwdStr == nil) return false;
            pwd = pwdStr.UTF8String;
            return true;
        }
    }

    bool StorePassword(const std::string& service, const std::string& user, const std::string& pwd) override {
        @autoreleasepool {
            NSString* svc = [NSString stringWithFormat:@"xLights/Discovery/%s",
                             service.c_str()];
            NSString* userNS = [NSString stringWithUTF8String:user.c_str()];
            NSMutableDictionary* baseQuery = [NSMutableDictionary dictionary];
            baseQuery[(__bridge id)kSecClass] = (__bridge id)kSecClassGenericPassword;
            baseQuery[(__bridge id)kSecAttrService] = svc;

            if (pwd.empty()) {
                // Empty password = delete the entry.
                OSStatus status = SecItemDelete((__bridge CFDictionaryRef)baseQuery);
                return status == errSecSuccess || status == errSecItemNotFound;
            }

            NSData* pwdData = [[NSString stringWithUTF8String:pwd.c_str()]
                                dataUsingEncoding:NSUTF8StringEncoding];

            // Try update first (single-account-per-service); fall back
            // to add when no entry exists.
            NSDictionary* update = @{
                (__bridge id)kSecAttrAccount: userNS,
                (__bridge id)kSecValueData: pwdData,
                (__bridge id)kSecAttrAccessible: (__bridge id)kSecAttrAccessibleAfterFirstUnlock,
            };
            OSStatus status = SecItemUpdate((__bridge CFDictionaryRef)baseQuery,
                                            (__bridge CFDictionaryRef)update);
            if (status == errSecItemNotFound) {
                NSMutableDictionary* add = [baseQuery mutableCopy];
                [add addEntriesFromDictionary:update];
                status = SecItemAdd((__bridge CFDictionaryRef)add, NULL);
            }
            return status == errSecSuccess;
        }
    }

    void YieldToUI() override {
        // No-op on iPad — discovery already runs off-main, so we
        // don't need to yield anything.
    }

private:
    XLFPPAuthPromptHandler _handler = nil;
};

namespace {

NSString* fppTypeString(FPP_TYPE t) {
    switch (t) {
    case FPP_TYPE::FPP:          return @"FPP";
    case FPP_TYPE::FALCONV4V5:   return @"FalconV4V5";
    case FPP_TYPE::ESPIXELSTICK: return @"ESPixelStick";
    case FPP_TYPE::GENIUS:       return @"Genius";
    case FPP_TYPE::POWERDMX:     return @"PowerDMX";
    }
    return @"FPP";
}

} // namespace

- (NSArray<NSDictionary*>*)discoverFPPInstances {
    return [self discoverFPPInstancesWithForcedAddresses:@[]];
}

- (NSArray<NSDictionary*>*)discoverFPPInstancesWithForcedAddresses:(NSArray<NSString*>*)forcedIPs {
    for (FPP* f : _fppInstances) delete f;
    _fppInstances.clear();

    NSMutableArray<NSDictionary*>* out = [NSMutableArray array];
    if (!_context) return out;
    auto& om = _context->GetOutputManager();

    // Reuse the same broadcast-ping path desktop's
    // `DiscoverFPPInstances` (DiscoveryHelpers.cpp:96) uses. No forced
    // IPs today — iPad has no `FPPConnectForcedIPs` preference yet.
    // Discovery runs synchronously on the calling thread; Swift wraps
    // in `Task.detached`.
    DiscoveryDelegate defaultDelegate;
    Discovery discovery(&om, &defaultDelegate);
    std::list<std::string> forcedAddresses;
    for (NSString* ip in forcedIPs) {
        if (ip.length > 0) forcedAddresses.push_back(std::string([ip UTF8String]));
    }
    // broadcastPing defaults to true, so broadcast discovery still runs
    // alongside any user-supplied forced IPs (CTL-5 "Add FPP by IP").
    FPP::PrepareDiscovery(discovery, forcedAddresses);
    discovery.Discover();
    FPP::MapToFPPInstances(discovery, _fppInstances, &om);

    _fppInstances.sort(sortByIP);

    // Wire the auth delegate onto every discovered FPP so a 401 from
    // any of them routes back to the registered Swift prompt
    // handler. Lazy-init: the delegate isn't constructed until the
    // first discovery (saves a tiny bit at app launch).
    if (!_fppAuthDelegate) {
        _fppAuthDelegate = std::make_unique<XLiPadDiscoveryAuthDelegate>();
    }
    for (FPP* fpp : _fppInstances) {
        if (fpp) fpp->_authDelegate = _fppAuthDelegate.get();
    }

    for (FPP* fpp : _fppInstances) {
        if (!fpp) continue;
        // iPad scope: FPP + ESPixelStick only (both open-source firmware).
        // Falcon V4/V5, Genius, PowerDMX use proprietary HSEQ-style codecs
        // that need separate per-vendor upload paths — explicitly out of
        // iPad scope (see Slice E in future-controller-upload.md). Without
        // this filter the bridge would route FPP-codec frames to those
        // devices and the upload would silently produce broken output.
        if (fpp->fppType != FPP_TYPE::FPP &&
            fpp->fppType != FPP_TYPE::ESPIXELSTICK) {
            continue;
        }
        NSString* uuid = fpp->uuid.empty()
            ? [NSString stringWithUTF8String:fpp->ipAddress.c_str()]
            : [NSString stringWithUTF8String:fpp->uuid.c_str()];
        NSString* version = fpp->fullVersion.empty()
            ? [NSString stringWithFormat:@"%u.%u.%u",
                fpp->majorVersion, fpp->minorVersion, fpp->patchVersion]
            : [NSString stringWithUTF8String:fpp->fullVersion.c_str()];
        NSMutableArray<NSString*>* playlists = [NSMutableArray array];
        for (const std::string& p : fpp->playlists) {
            [playlists addObject:[NSString stringWithUTF8String:p.c_str()]];
        }

        // Resolve a user-facing cape / hat model name using the same
        // logic as desktop's FPPConnectDialog::PopulateFPPInstanceList
        // (line 685-707). Start with FPP::GetModel mapping; if the
        // OutputManager has a matching ControllerEthernet whose caps
        // report a model, prefer that. Empty result means the FPP has
        // no cape worth offering an upload checkbox for — the SwiftUI
        // sheet hides the Pixel Hat/Cape toggle in that case.
        std::string capeModel = FPP::GetModel(fpp->pixelControllerType);
        auto controllers = om.GetControllers(fpp->ipAddress);
        if (controllers.size() == 1) {
            if (auto* eth = dynamic_cast<ControllerEthernet*>(controllers.front())) {
                if (const ControllerCaps* caps = eth->GetControllerCaps()) {
                    std::string capsModel = caps->GetModel();
                    if (!capsModel.empty()) capeModel = capsModel;
                }
            }
        }
        if (!capeModel.empty() && !fpp->panelSize.empty()) {
            capeModel += " - " + fpp->panelSize;
        }

        [out addObject:@{
            @"ipAddress":           [NSString stringWithUTF8String:fpp->ipAddress.c_str()],
            @"hostName":            [NSString stringWithUTF8String:fpp->hostName.c_str()],
            @"description":         [NSString stringWithUTF8String:fpp->description.c_str()],
            @"platform":            [NSString stringWithUTF8String:fpp->platform.c_str()],
            @"model":               [NSString stringWithUTF8String:fpp->model.c_str()],
            @"mode":                [NSString stringWithUTF8String:fpp->mode.c_str()],
            @"version":             version,
            @"uuid":                uuid,
            @"fppType":             fppTypeString(fpp->fppType),
            @"supportedForFPPConnect": @(fpp->supportedForFPPConnect()),
            @"playlists":           playlists,
            // User-facing cape / hat model name (mirrors desktop's
            // PopulateFPPInstanceList m-string). Empty when the FPP
            // has no cape worth configuring — the SwiftUI sheet hides
            // the Pixel Hat/Cape toggle entirely in that case. Always
            // non-empty for FPPs with a real cape attached (e.g.
            // "K8-Pro", "F32-B", "PiHat - 64x32").
            @"capeModel":           [NSString stringWithUTF8String:capeModel.c_str()],
        }];
    }
    return out;
}

- (void)releaseFPPInstances {
    for (FPP* f : _fppInstances) delete f;
    _fppInstances.clear();
}

- (void)setFPPAuthPromptHandler:(nullable XLFPPAuthPromptHandler)handler {
    if (!_fppAuthDelegate) {
        _fppAuthDelegate = std::make_unique<XLiPadDiscoveryAuthDelegate>();
    }
    _fppAuthDelegate->SetPromptHandler(handler);
}

- (NSDictionary*)applyConfigToFPP:(NSString*)ipAddress
                         settings:(NSDictionary*)settings
                         progress:(nullable id<XLFPPUploadProgress>)progress {
    if (!_context || _fppInstances.empty()) {
        return @{@"ok": @NO, @"cancelled": @NO,
                 @"message": @"FPP instance list is empty — run discovery first."};
    }

    std::string targetIP = ipAddress.UTF8String;
    FPP* target = nullptr;
    for (FPP* f : _fppInstances) {
        if (f && f->ipAddress == targetIP) { target = f; break; }
    }
    if (!target) {
        return @{@"ok": @NO, @"cancelled": @NO,
                 @"message": [NSString stringWithFormat:@"No discovered FPP at %@", ipAddress]};
    }
    if (target->fppType != FPP_TYPE::FPP) {
        // Non-FPP discovered device (ESPixelStick, etc.). It doesn't
        // accept the FPP Models/UDP/Cape config uploads, but it does
        // support an immediate-output upload of the show's pixel
        // config. Mirrors FPPConnectDialog.cpp:1199-1203: when the
        // "Upload Controller" checkbox (uploadCape on iPad) is set and
        // exactly one controller matches the device IP, push it.
        bool uploadCapeNonFPP = [settings[@"uploadCape"] boolValue];
        if (!uploadCapeNonFPP) {
            return @{@"ok": @YES, @"cancelled": @NO, @"message": @""};
        }
        if (!_context->HasModelManager()) {
            return @{@"ok": @NO, @"cancelled": @NO,
                     @"message": @"Models aren't loaded; can't compute output channels."};
        }
        auto& omNF = _context->GetOutputManager();
        auto& mmNF = _context->GetModelManager();
        auto controllersNF = omNF.GetControllers(target->ipAddress);
        if (controllersNF.size() != 1) {
            return @{@"ok": @YES, @"cancelled": @NO, @"message": @""};
        }
        Controller* ctrlNF = controllersNF.front();
        mmNF.RecalcStartChannels();
        iPadUploadCallbacks cbsNF;
        std::unique_ptr<BaseController> bcNF(
            BaseController::CreateBaseController(ctrlNF, target->ipAddress));
        if (!bcNF) {
            return @{@"ok": @NO, @"cancelled": @NO,
                     @"message": @"Unable to create a connection for this device."};
        }
        if (!bcNF->IsConnected()) {
            return @{@"ok": @NO, @"cancelled": @NO,
                     @"message": [NSString stringWithFormat:
                         @"Could not connect to %s.", target->ipAddress.c_str()]};
        }
        bool okNF = bcNF->UploadForImmediateOutput(&mmNF, &omNF, ctrlNF, &cbsNF);
        if (okNF) {
            return @{@"ok": @YES, @"cancelled": @NO, @"message": @""};
        }
        NSString* msgNF = cbsNF.captured.empty()
            ? @"Immediate-output upload failed."
            : [NSString stringWithUTF8String:cbsNF.captured.c_str()];
        return @{@"ok": @NO, @"cancelled": @NO, @"message": msgNF};
    }

    // Mirror the desktop dialog's progress / cancel wiring. Single
    // target here — the multi-target protocol's `setProgress:forIPAddress:`
    // just routes the FPP's 0..1000 scale (downscaled to 0..100) into
    // the per-IP slot the SwiftUI sheet reads.
    NSString* targetIPNS = ipAddress;
    bool cancelledFlag = false;
    target->setProgress({
        [progress, targetIPNS](int val) {
            if (progress) [progress setProgress:val / 10 forIPAddress:targetIPNS];
        },
        [progress, &cancelledFlag]() -> bool {
            if (progress && [progress isCancelled]) cancelledFlag = true;
            return cancelledFlag;
        },
        []() {}
    });
    target->defaultConnectTimeout = 5000;
    target->messages.clear();

    auto& om = _context->GetOutputManager();
    auto& mm = _context->GetModelManager();
    bool cancelled = false;

    // Match the matching ControllerEthernet for outputs uploads. If
    // there isn't exactly one match, we skip the Cape uploads (desktop
    // does the same — `if (c.size() == 1)` gate at line 1174).
    auto controllers = om.GetControllers(target->ipAddress);
    ControllerEthernet* matchedEth = nullptr;
    if (controllers.size() == 1) {
        matchedEth = dynamic_cast<ControllerEthernet*>(controllers.front());
    }

    // Backfill `ranges` so the FPP can advertise sane channel ownership
    // when discovery didn't pre-populate it. Mirrors lines 1140-1147.
    if (matchedEth && target->ranges.empty()) {
        uint32_t sc = matchedEth->GetStartChannel() - 1;
        target->ranges = std::to_string(sc) + "-" +
            std::to_string(sc + matchedEth->GetChannels() - 1);
    }

    bool uploadProxies = [settings[@"uploadProxies"] boolValue];
    bool uploadCape = [settings[@"uploadCape"] boolValue];
    NSString* modelsMode = [settings[@"modelsMode"] isKindOfClass:[NSString class]]
        ? (NSString*)settings[@"modelsMode"] : @"none";
    NSString* udpOutMode = [settings[@"udpOutMode"] isKindOfClass:[NSString class]]
        ? (NSString*)settings[@"udpOutMode"] : @"none";
    NSString* playlist = [settings[@"playlist"] isKindOfClass:[NSString class]]
        ? (NSString*)settings[@"playlist"] : @"";

    // Order mirrors desktop's per-FPP config loop
    // (FPPConnectDialog.cpp:1148-1196): Playlist (setup) → Proxies →
    // UDP Out → Cape → Models. Each upload may set the restart flag;
    // the single `Restart(true)` at the end of this method commits.

    if (playlist.length > 0 && !cancelled && !cancelledFlag) {
        cancelled = target->UploadPlaylist(playlist.UTF8String);
    }

    if (uploadProxies && !cancelled && !cancelledFlag) {
        cancelled = target->UploadControllerProxies(&om);
    }

    if (!cancelled && !cancelledFlag) {
        if ([udpOutMode isEqualToString:@"all"]) {
            // Build the universe file from every controller in the
            // OutputManager. The `udpRanges` map collects the channel
            // ranges that come back; we feed them back into the FPP's
            // own range table (FillRanges + SetNewRanges) so it
            // advertises ownership of the channels it now forwards.
            std::map<int, int> udpRanges;
            auto outputs = target->CreateUniverseFile(om.GetControllers(),
                                                      false, &udpRanges);
            cancelled = target->UploadUDPOut(outputs);
            if (!cancelled && !cancelledFlag) {
                std::map<int, int> rngs(udpRanges);
                target->FillRanges(rngs);
                target->SetNewRanges(rngs);
                target->SetRestartFlag();
            }
        } else if ([udpOutMode isEqualToString:@"proxied"]) {
            cancelled = target->UploadUDPOutputsForProxy(&om);
            if (!cancelled && !cancelledFlag) {
                target->SetRestartFlag();
            }
        }
    }

    if (uploadCape && !cancelled && !cancelledFlag && matchedEth) {
        // Order matches FPPConnectDialog.cpp:1175-1178.
        cancelled = target->UploadPanelOutputs(&mm, &om, matchedEth);
        if (!cancelled && !cancelledFlag) {
            cancelled = target->UploadVirtualMatrixOutputs(&mm, &om, matchedEth);
        }
        if (!cancelled && !cancelledFlag) {
            cancelled = target->UploadPixelOutputs(&mm, &om, matchedEth);
        }
        if (!cancelled && !cancelledFlag) {
            cancelled = target->UploadSerialOutputs(&mm, &om, matchedEth);
        }
        // Push the input universes (bridge mode) so the FPP forwards
        // the show's sACN/ArtNet universes onto its pixel outputs.
        // Mirrors FPPConnectDialog.cpp:1179.
        if (!cancelled && !cancelledFlag) {
            cancelled = target->SetInputUniversesBridge(matchedEth);
        }
    }

    if (!cancelled && !cancelledFlag) {
        if ([modelsMode isEqualToString:@"all"]) {
            // "All": full channel range, plus a virtual display map
            // built from the show's models + objects. Mirrors
            // FPPConnectDialog.cpp:1181-1186.
            auto memoryMaps = target->CreateModelMemoryMap(
                &mm, 0, std::numeric_limits<int32_t>::max());
            cancelled = target->UploadModels(memoryMaps);
            if (!cancelled && !cancelledFlag) {
                std::map<std::string, std::string> virtualDisplayData;
                if (_context->HasViewObjectManager()) {
                    FPP::CreateVirtualDisplayMap(
                        mm, _context->GetAllObjects(),
                        _context->GetPreviewWidth(),
                        _context->GetPreviewHeight(),
                        virtualDisplayData);
                }
                cancelled = target->UploadDisplayMap(virtualDisplayData);
            }
            // Model uploads still require a full restart per the
            // desktop comment at line 1185.
            if (!cancelled && !cancelledFlag) {
                target->SetRestartFlag(true);
            }
        } else if ([modelsMode isEqualToString:@"local"] && matchedEth) {
            // "Local": just the channels owned by the matched
            // controller. No display map upload here either (desktop
            // has it commented out at line 1192).
            auto memoryMaps = target->CreateModelMemoryMap(
                &mm,
                matchedEth->GetStartChannel(),
                matchedEth->GetEndChannel());
            cancelled = target->UploadModels(memoryMaps);
            if (!cancelled && !cancelledFlag) {
                target->SetRestartFlag(true);
            }
        }
    }

    // Conditional restart — Restart(true) means "if needed" (only
    // restarts when one of the uploads set the restart flag). Matches
    // the desktop line 1197 pattern.
    if (!cancelled && !cancelledFlag) {
        target->Restart(true);
    }

    target->setProgress({});

    if (cancelledFlag) {
        return @{@"ok": @NO, @"cancelled": @YES, @"message": @"Configuration cancelled."};
    }
    if (cancelled) {
        NSString* msg = target->messages.empty()
            ? @"Configuration upload failed."
            : [NSString stringWithUTF8String:target->messages.front().c_str()];
        return @{@"ok": @NO, @"cancelled": @NO, @"message": msg};
    }
    return @{@"ok": @YES, @"cancelled": @NO, @"message": @""};
}

- (BOOL)updateChannelRangesForFPP:(NSString*)ipAddress {
    if (!_context || _fppInstances.empty()) return NO;
    std::string targetIP = ipAddress.UTF8String;
    for (FPP* f : _fppInstances) {
        if (f && f->ipAddress == targetIP) {
            f->UpdateChannelRanges();
            return YES;
        }
    }
    return NO;
}

- (BOOL)finalizeFPP:(NSString*)ipAddress
            playlist:(nullable NSString*)playlist {
    if (!_context || _fppInstances.empty()) return NO;
    std::string targetIP = ipAddress.UTF8String;
    FPP* target = nullptr;
    for (FPP* f : _fppInstances) {
        if (f && f->ipAddress == targetIP) { target = f; break; }
    }
    if (!target) return NO;

    bool ok = true;
    if (playlist.length > 0) {
        // Second `UploadPlaylist` pass — commits the just-uploaded
        // sequences into the playlist. Matches FPPConnectDialog.cpp:1430.
        if (target->UploadPlaylist(playlist.UTF8String)) ok = false;
    }
    if (target->Restart(true)) ok = false;
    return ok;
}

- (nullable NSString*)mediaPathForXsq:(NSString*)xsqPath {
    if (!_context || xsqPath.length == 0) return nil;
    std::string xsq = xsqPath.UTF8String;
    if (!FileExists(xsq)) return nil;

    XsqFileInfo info = ScanXsqFile(xsq);
    if (!info.isSequence || info.mediaFile.empty()) return nil;

    const std::string& showDir = _context->GetShowDirectory();

    // Mirror desktop's lookup priority (FPPConnectDialog.cpp:878-895):
    //   1. The path as stored — if it resolves on disk, use it.
    //   2. Walk every configured media folder for a basename match.
    //   3. Fall back to FixFile against the show directory.
    if (FileExists(info.mediaFile)) {
        return [NSString stringWithUTF8String:info.mediaFile.c_str()];
    }

    // Extract the basename treating BOTH '/' and '\' as separators: a
    // sequence authored on Windows stores "B:\Music\song.mp3", and
    // std::filesystem on iOS parses with the POSIX format where '\' is
    // an ordinary character, so .filename() would return the whole
    // string instead of "song.mp3" (mirrors desktop ManageMediaPanel).
    auto sepPos = info.mediaFile.find_last_of("/\\");
    std::string basename = (sepPos == std::string::npos)
                               ? info.mediaFile
                               : info.mediaFile.substr(sepPos + 1);
    for (const auto& folder : _context->GetMediaFolders()) {
        std::string candidate = folder + "/" + basename;
        if (FileExists(candidate)) {
            return [NSString stringWithUTF8String:candidate.c_str()];
        }
    }

    std::string fixed = FileUtils::FixFile(showDir, info.mediaFile);
    if (FileExists(fixed)) {
        return [NSString stringWithUTF8String:fixed.c_str()];
    }

    return nil;
}

- (NSDictionary*)uploadFseq:(NSString*)fseqPath
              toFPPInstances:(NSArray<NSDictionary*>*)targets
                    progress:(nullable id<XLFPPUploadProgress>)progress {
    NSMutableDictionary* outcomes = [NSMutableDictionary dictionary];

    if (!_context || _fppInstances.empty()) {
        return @{@"globalError": @"FPP instance list is empty — run discovery first.",
                 @"outcomes": outcomes};
    }
    if (targets.count == 0) {
        return @{@"outcomes": outcomes};
    }

    // Open the source fseq once — every target reads from this same
    // FSEQFile (frame batches are buffered into local std::vectors
    // before fan-out, so target threads never touch the source
    // simultaneously).
    std::string fseq = fseqPath.UTF8String;
    std::unique_ptr<FSEQFile> seq(FSEQFile::openFSEQFile(fseq));
    if (!seq) {
        return @{@"globalError": [NSString stringWithFormat:@"Could not open fseq: %@", fseqPath],
                 @"outcomes": outcomes};
    }

    // Resolve each target to its FPP* and compute the right codec.
    // Targets that don't resolve (deleted between discover and upload?)
    // are dropped silently; the caller can tell because they won't
    // appear in the result map.
    struct TargetCtx {
        FPP* fpp = nullptr;
        std::string ip;
        std::string media;
        int fseqType = 2;
        NSString* ipNS = nil;       // for progress routing
        bool prepared = false;
        bool finalized = false;
        bool cancelled = false;
        bool failed = false;
        std::string message;
    };
    std::vector<TargetCtx> ctxs;
    ctxs.reserve(targets.count);
    for (NSDictionary* t in targets) {
        NSString* ipNS = t[@"ipAddress"];
        if (![ipNS isKindOfClass:[NSString class]]) continue;
        std::string ip = ipNS.UTF8String;
        FPP* match = nullptr;
        for (FPP* f : _fppInstances) {
            if (f && f->ipAddress == ip) { match = f; break; }
        }
        if (!match || !match->supportedForFPPConnect()) continue;

        TargetCtx ctx;
        ctx.fpp = match;
        ctx.ip = ip;
        ctx.ipNS = ipNS;
        NSString* mediaNS = t[@"mediaPath"];
        if ([mediaNS isKindOfClass:[NSString class]]) {
            ctx.media = mediaNS.UTF8String;
        }
        ctx.fseqType = (match->fppType == FPP_TYPE::ESPIXELSTICK) ? 3 : 2;
        ctxs.push_back(ctx);
    }

    if (ctxs.empty()) {
        return @{@"globalError": @"No targets resolved to a discovered FPP.",
                 @"outcomes": outcomes};
    }

    // Wire per-target progress and cancel — one shared cancel flag,
    // per-target progress routing keyed by IP. Cancellation propagates
    // out of the dispatch_apply via the flag every target's IsCancelled
    // lambda polls.
    bool cancelledFlag = false;
    for (TargetCtx& c : ctxs) {
        FPP* fpp = c.fpp;
        NSString* ipNS = c.ipNS;
        fpp->defaultConnectTimeout = 5000;
        fpp->messages.clear();
        fpp->setProgress({
            [progress, ipNS](int val) {
                if (progress) [progress setProgress:val / 10 forIPAddress:ipNS];
            },
            [progress, &cancelledFlag]() -> bool {
                if (progress && [progress isCancelled]) cancelledFlag = true;
                return cancelledFlag;
            },
            []() {}
        });
    }

    auto pumpCurls = []() {
        // Drain any in-flight transfers (Prepare may have queued
        // capability probes / mediaUpload PUTs; Finalize will queue
        // the bulk fseq transfer; AddFrameToUpload is CPU-only).
        while (CurlManager::INSTANCE.processCurls()) {}
    };

    // Phase 1 — Prepare each target. Mirrors FPPConnectDialog.cpp:1253.
    for (TargetCtx& c : ctxs) {
        if (cancelledFlag) break;
        bool prepFail = c.fpp->PrepareUploadSequence(seq.get(), fseq,
                                                      c.media, c.fseqType);
        if (prepFail) {
            c.failed = true;
            c.message = "PrepareUploadSequence failed.";
        } else {
            c.prepared = true;
        }
    }
    pumpCurls();

    // Phase 2 — frame fan-out. Only feed targets whose Prepare
    // succeeded AND who actually need custom transcoding (some FPP
    // versions accept the original fseq as-is via uploadOrCopyFile
    // and don't need per-frame transcoding).
    // Using `char` not `bool` so we can safely take a raw pointer
    // into it from the dispatch block (std::vector<bool> is the
    // packed specialization with no `data()`).
    std::vector<char> needsFrames(ctxs.size(), 0);
    bool anyNeedsFrames = false;
    for (size_t i = 0; i < ctxs.size(); ++i) {
        if (!ctxs[i].prepared) continue;
        if (ctxs[i].fpp->WillUploadSequence() && ctxs[i].fpp->NeedCustomSequence()) {
            needsFrames[i] = 1;
            anyNeedsFrames = true;
        } else if (ctxs[i].fpp->WillUploadSequence()) {
            // Original-fseq path — FPP class flips progress to 100%
            // internally; nothing for us to feed.
        }
    }

    if (anyNeedsFrames && !cancelledFlag) {
        const uint32_t numFrames = (uint32_t)seq->getNumFrames();
        const uint32_t channels = seq->getMaxChannel() + 1;
        constexpr int FRAMES_TO_BUFFER = 50;
        std::vector<std::vector<uint8_t>> frames(FRAMES_TO_BUFFER);
        for (auto& v : frames) v.resize(channels);

        uint32_t frame = 0;
        while (frame < numFrames && !cancelledFlag) {
            // Read the next batch of frames from the source.
            int lastBuffered = 0;
            uint32_t startFrame = frame;
            while (lastBuffered < FRAMES_TO_BUFFER && frame < numFrames) {
                FSEQFile::FrameData* fd = seq->getFrame(frame);
                if (!fd) {
                    // Skip the frame; subsequent reads keep going.
                    lastBuffered++;
                    frame++;
                    continue;
                }
                if (!fd->readFrame(frames[lastBuffered].data(), channels)) {
                    delete fd;
                    cancelledFlag = false;
                    // Mark every prepared target as failed — a corrupt
                    // source fseq is a batch-level fault.
                    for (TargetCtx& c : ctxs) {
                        if (c.prepared && !c.failed) {
                            c.failed = true;
                            c.message = "FSEQ corrupt at frame " +
                                std::to_string(startFrame + lastBuffered);
                        }
                    }
                    frame = numFrames;  // break outer loop
                    break;
                }
                delete fd;
                lastBuffered++;
                frame++;
            }

            if (cancelledFlag || lastBuffered == 0) break;

            // Fan out across targets in parallel. Each target's
            // transcoder is its own object, so AddFrameToUpload is
            // safe to run concurrently across targets. `dispatch_apply`
            // blocks until every closure returns — matches desktop's
            // `parallel_for(instances, func)` semantics.
            //
            // Block captures default to const, which would make the
            // captured std::vector unmodifiable inside. Reach mutable
            // storage via captured raw pointers: `framesPtr[x]` walks
            // through a non-const std::vector*, and `ctxsPtr[i].fpp`
            // gets a non-const FPP* for AddFrameToUpload.
            const size_t numCtx = ctxs.size();
            std::vector<uint8_t>* framesPtr = frames.data();
            TargetCtx* ctxsPtr = ctxs.data();
            const char* needsFramesPtr = needsFrames.data();
            __block bool batchCancelled = cancelledFlag;
            dispatch_apply(numCtx,
                            dispatch_get_global_queue(QOS_CLASS_USER_INITIATED, 0),
                            ^(size_t i) {
                if (batchCancelled || !needsFramesPtr[i]) return;
                FPP* fpp = ctxsPtr[i].fpp;
                for (int x = 0; x < lastBuffered; ++x) {
                    fpp->AddFrameToUpload(startFrame + x, framesPtr[x].data());
                }
            });
            cancelledFlag = batchCancelled || cancelledFlag;

            // Pump curls between batches so the previous-target's
            // network transfer can drain while the CPU works on the
            // next batch's transcode.
            pumpCurls();
        }
    }

    // Phase 3 — Finalize each target. This is where the bulk fseq
    // upload actually queues onto CurlManager (via uploadOrCopyFile
    // inside FinalizeUploadSequence).
    for (TargetCtx& c : ctxs) {
        if (cancelledFlag) {
            c.cancelled = true;
            continue;
        }
        if (!c.prepared || c.failed) continue;
        bool finFail = c.fpp->FinalizeUploadSequence();
        if (finFail) {
            c.failed = true;
            if (c.message.empty()) {
                if (!c.fpp->messages.empty()) c.message = c.fpp->messages.front();
                else c.message = "Finalize failed.";
            }
        } else {
            c.finalized = true;
        }
    }

    // Phase 4 — drain remaining curl transfers. The bulk fseq upload
    // happens here; without this loop the function would return before
    // any of them complete and the data wouldn't actually land on the
    // FPP.
    pumpCurls();

    // Tear down per-target progress callbacks before returning so any
    // residual curl callbacks don't try to dereference the now-dropped
    // forwarder.
    for (TargetCtx& c : ctxs) {
        c.fpp->setProgress({});
    }

    // Build the result dict.
    for (const TargetCtx& c : ctxs) {
        NSString* msg = @"";
        if (!c.message.empty()) {
            msg = [NSString stringWithUTF8String:c.message.c_str()];
        }
        bool ok = c.prepared && !c.failed && !c.cancelled;
        outcomes[c.ipNS] = @{
            @"ok": @(ok),
            @"cancelled": @(c.cancelled),
            @"message": msg,
        };
    }
    return @{@"outcomes": outcomes,
             @"cancelled": @(cancelledFlag)};
}

#pragma mark - IE-15 Export Models report

- (BOOL)exportModelsReportToPath:(NSString*)path {
    if (!_context || !path || !_context->IsSequenceLoaded() || !_context->HasModelManager()) {
        return NO;
    }
    ObtainAccessToURL(path.UTF8String, /*enforceWritable=*/true);
    return ::ExportModels(std::string(path.UTF8String),
                          _context->GetModelManager(),
                          _context->GetOutputManager())
               ? YES
               : NO;
}

#pragma mark - EFX-1 Export Effects report

- (BOOL)exportEffectsReportToPath:(NSString*)path NS_SWIFT_NAME(exportEffectsReport(toPath:)) {
    if (!_context || !path || !_context->IsSequenceLoaded() || !_context->HasModelManager()) {
        return NO;
    }
    ObtainAccessToURL(path.UTF8String, /*enforceWritable=*/true);
    return ::ExportEffects(std::string(path.UTF8String),
                           _context->GetSequenceElements(),
                           _context->GetModelManager())
               ? YES
               : NO;
}

#pragma mark - Theme-07 Export Controller Connections

- (BOOL)exportControllerConnectionsToPath:(NSString*)path {
    if (!_context || !path || !_context->HasModelManager()) {
        return NO;
    }
    auto& om = _context->GetOutputManager();
    auto& mm = _context->GetModelManager();
    auto controllers = om.GetControllers();
    if (controllers.empty()) return NO;

    ObtainAccessToURL(path.UTF8String, /*enforceWritable=*/true);

    // Include the full export field set. The desktop dialog prompts
    // the user (ExportSettings::GetSettings); on iPad we export
    // everything rather than surface a field-picker.
    ExportSettings::SETTINGS exportsettings =
        ExportSettings::SETTINGS_PORT_ABSADDRESS |
        ExportSettings::SETTINGS_PORT_UNIADDRESS |
        ExportSettings::SETTINGS_PORT_CHANNELS |
        ExportSettings::SETTINGS_PORT_PIXELS |
        ExportSettings::SETTINGS_PORT_CURRENT |
        ExportSettings::SETTINGS_MODEL_DESCRIPTIONS |
        ExportSettings::SETTINGS_MODEL_ABSADDRESS |
        ExportSettings::SETTINGS_MODEL_UNIADDRESS |
        ExportSettings::SETTINGS_MODEL_CHANNELS |
        ExportSettings::SETTINGS_MODEL_PIXELS |
        ExportSettings::SETTINGS_MODEL_CURRENT;

    lxw_workbook* workbook = workbook_new(path.UTF8String);
    if (!workbook) return NO;
    lxw_worksheet* worksheet = workbook_add_worksheet(workbook, NULL);

    lxw_format* header_format = workbook_add_format(workbook);
    format_set_align(header_format, LXW_ALIGN_CENTER);
    format_set_align(header_format, LXW_ALIGN_VERTICAL_CENTER);
    format_set_bold(header_format);
    format_set_bg_color(header_format, LXW_COLOR_YELLOW);

    lxw_format* format = workbook_add_format(workbook);
    lxw_format* first_format = workbook_add_format(workbook);
    format_set_bold(first_format);

    // Smart-remote shading mirrors the desktop's six remote colors.
    auto makeShade = [&](lxw_color_t color) -> lxw_format* {
        lxw_format* f = workbook_add_format(workbook);
        format_set_bg_color(f, color);
        return f;
    };
    lxw_format* sr1_format = makeShade(0xFFC0C0);
    lxw_format* sr2_format = makeShade(0xC0FFC0);
    lxw_format* sr3_format = makeShade(0xC0C0FF);
    lxw_format* sr4_format = makeShade(0xFFFFC0);
    lxw_format* sr5_format = makeShade(0xFFC0FF);
    lxw_format* sr6_format = makeShade(0xC0FFFF);

    int row = 0;
    std::map<int, double> colWidths;
    for (Controller* it : controllers) {
        UDController cud(it, &om, &mm, false);
        int columnSize = 0;
        std::vector<std::vector<std::string>> const lines =
            cud.ExportAsCSV(exportsettings,
                            it->GetDefaultBrightnessUnderFullControl(),
                            columnSize);

        worksheet_merge_range(worksheet, row, 0, row, columnSize,
                              it->GetShortDescription().c_str(), header_format);
        ++row;
        lxw_format* lformat = first_format;

        for (const auto& line : lines) {
            for (int i = 1; i <= columnSize; ++i) {
                worksheet_write_blank(worksheet, row, i, lformat);
            }
            int col = 0;
            for (auto const& column : line) {
                if (column.empty()) continue;
                if (column.find("Remote A") != std::string::npos ||
                    column.find("Remote G") != std::string::npos ||
                    column.find("Remote M") != std::string::npos) {
                    lformat = sr1_format;
                }
                if (column.find("Remote B") != std::string::npos ||
                    column.find("Remote H") != std::string::npos ||
                    column.find("Remote N") != std::string::npos) {
                    lformat = sr2_format;
                }
                if (column.find("Remote C") != std::string::npos ||
                    column.find("Remote I") != std::string::npos ||
                    column.find("Remote O") != std::string::npos) {
                    lformat = sr3_format;
                }
                if (column.find("Remote D") != std::string::npos ||
                    column.find("Remote J") != std::string::npos ||
                    column.find("Remote P") != std::string::npos) {
                    lformat = sr4_format;
                }
                if (column.find("Remote E") != std::string::npos ||
                    column.find("Remote K") != std::string::npos) {
                    lformat = sr5_format;
                }
                if (column.find("Remote F") != std::string::npos ||
                    column.find("Remote L") != std::string::npos) {
                    lformat = sr6_format;
                }
                worksheet_write_string(worksheet, row, col, column.c_str(), lformat);
                double width = column.size() + 1.3;
                if (colWidths[col] < width) {
                    colWidths[col] = width;
                    worksheet_set_column(worksheet, col, col, width, NULL);
                }
                ++col;
            }
            ++row;
            lformat = format;
        }
        row += 2;
    }

    lxw_error error = workbook_close(workbook);
    return error == LXW_NO_ERROR ? YES : NO;
}

#pragma mark - Theme-07 Sort Controllers

- (BOOL)sortControllersByMode:(NSString*)mode {
    if (!_context) return NO;
    auto& om = _context->GetOutputManager();
    std::string m = mode ? mode.UTF8String : "";
    if (m == "name") {
        om.SortControllersbyName();
    } else if (m == "id") {
        om.SortControllersbyID();
    } else if (m == "ip") {
        om.SortControllersbyIP();
    } else if (m == "proxy") {
        om.SortControllersbyFPPProxy();
    } else if (m == "vendor") {
        om.SortControllersbyModel();
    } else if (m == "protocol") {
        om.SortControllersbyProtocal();
    } else {
        return NO;
    }
    return YES;
}

#pragma mark - Song Structure Regions (#6268 — bulk actions)

- (int)songStructureRegionIndexAtTimeMS:(int)timeMS {
    return _context->GetSequenceElements().GetSongStructureManager().GetRegionIndexAtTime(timeMS);
}

- (int)copyEffectsFromRegion:(int)sourceRegionIndex toRegion:(int)targetRegionIndex {
    auto& se = _context->GetSequenceElements();
    auto& ssm = se.GetSongStructureManager();
    if (sourceRegionIndex < 0 || targetRegionIndex < 0 ||
        sourceRegionIndex >= (int)ssm.GetRegionCount() ||
        targetRegionIndex >= (int)ssm.GetRegionCount() ||
        sourceRegionIndex == targetRegionIndex) {
        return 0;
    }

    const SongStructureRegion& sourceRegion = ssm.GetRegion(sourceRegionIndex);
    const SongStructureRegion& targetRegion = ssm.GetRegion(targetRegionIndex);
    int timeOffset = targetRegion.startTimeMS - sourceRegion.startTimeMS;
    double freq = se.GetFrequency();

    UndoManager& undoMgr = se.get_undo_mgr();
    undoMgr.CreateUndoStep();
    _context->AbortRender(5000);

    int effectsCopied = 0;
    for (size_t i = 0; i < se.GetElementCount(); i++) {
        Element* elem = se.GetElement(i);
        if (elem == nullptr || elem->GetType() == ElementType::ELEMENT_TYPE_TIMING) continue;

        for (int layer = 0; layer < (int)elem->GetEffectLayerCount(); layer++) {
            EffectLayer* el = elem->GetEffectLayer(layer);
            if (el == nullptr) continue;

            std::vector<Effect*> sourceEffects = el->GetAllEffectsByTime(
                sourceRegion.startTimeMS, sourceRegion.endTimeMS);

            for (Effect* eff : sourceEffects) {
                int newStartMS = RoundToMultipleOfPeriod(eff->GetStartTimeMS() + timeOffset, freq);
                int newEndMS = RoundToMultipleOfPeriod(eff->GetEndTimeMS() + timeOffset, freq);
                if (newStartMS < 0 || newStartMS >= newEndMS) continue;

                if (el->GetRangeIsClearMS(newStartMS, newEndMS)) {
                    Effect* newEff = el->AddEffect(0,
                        eff->GetEffectName(),
                        eff->GetSettingsAsString(),
                        eff->GetPaletteAsString(),
                        newStartMS, newEndMS, EFFECT_NOT_SELECTED, false);
                    if (newEff != nullptr) {
                        undoMgr.CaptureAddedEffect(elem->GetName(), el->GetIndex(), newEff->GetID());
                        effectsCopied++;
                    }
                }
            }
        }
    }

    if (effectsCopied == 0) {
        undoMgr.CancelLastStep();
    }
    se.IncrementChangeCount(nullptr);
    return effectsCopied;
}

- (int)applyPaletteString:(NSString*)paletteString toRegionAtIndex:(int)regionIndex {
    auto& se = _context->GetSequenceElements();
    auto& ssm = se.GetSongStructureManager();
    if (!paletteString || regionIndex < 0 || regionIndex >= (int)ssm.GetRegionCount()) {
        return 0;
    }

    const SongStructureRegion& region = ssm.GetRegion(regionIndex);
    std::string palette([paletteString UTF8String]);

    UndoManager& undoMgr = se.get_undo_mgr();
    undoMgr.CreateUndoStep();
    _context->AbortRender(5000);

    int effectsModified = 0;
    for (size_t i = 0; i < se.GetElementCount(); i++) {
        Element* elem = se.GetElement(i);
        if (elem == nullptr || elem->GetType() == ElementType::ELEMENT_TYPE_TIMING) continue;

        for (int layer = 0; layer < (int)elem->GetEffectLayerCount(); layer++) {
            EffectLayer* el = elem->GetEffectLayer(layer);
            if (el == nullptr) continue;

            std::vector<Effect*> regionEffects = el->GetAllEffectsByTime(
                region.startTimeMS, region.endTimeMS);
            for (Effect* eff : regionEffects) {
                undoMgr.CaptureModifiedEffect(elem->GetName(), el->GetIndex(), eff);
                eff->SetPalette(palette);
                effectsModified++;
            }
        }
    }

    if (effectsModified == 0) {
        undoMgr.CancelLastStep();
    }
    se.IncrementChangeCount(nullptr);
    return effectsModified;
}

- (int)fillRegionFromTimingMarksWithSourceRow:(int)rowIndex sourceIndex:(int)effectIndex {
    auto& se = _context->GetSequenceElements();
    EffectLayer* el = [self effectLayerForRow:rowIndex];
    if (el == nullptr || effectIndex < 0 || effectIndex >= el->GetEffectCount()) return 0;

    Effect* sourceEffect = el->GetEffect(effectIndex);
    if (sourceEffect == nullptr) return 0;

    auto& ssm = se.GetSongStructureManager();
    const SongStructureRegion* region = ssm.GetRegionAtTime(sourceEffect->GetStartTimeMS());
    if (region == nullptr) return 0;

    // Locate the active timing element's last populated layer — mirrors
    // EffectsGrid::FillRegionFromTimingMarks's GetActiveTimingElement().
    EffectLayer* timingLayer = nullptr;
    for (int i = 0; i < se.GetNumberOfTimingElements(); i++) {
        TimingElement* te = se.GetTimingElement(i);
        if (te == nullptr || !te->GetActive()) continue;
        for (int l = (int)te->GetEffectLayerCount() - 1; l >= 0; l--) {
            EffectLayer* tl = te->GetEffectLayer(l);
            if (tl != nullptr && tl->GetEffectCount() > 0) {
                timingLayer = tl;
                break;
            }
        }
        break;
    }
    if (timingLayer == nullptr) return 0;

    std::string effectName = sourceEffect->GetEffectName();
    std::string settings = sourceEffect->GetSettingsAsString();
    std::string palette = sourceEffect->GetPaletteAsString();
    long sourceStart = sourceEffect->GetStartTimeMS();
    long sourceEnd = sourceEffect->GetEndTimeMS();
    long sourceDuration = sourceEnd - sourceStart;
    double freq = se.GetFrequency();

    UndoManager& undoMgr = se.get_undo_mgr();
    undoMgr.CreateUndoStep();
    _context->AbortRender(5000);

    int effectsCreated = 0;
    for (int i = 0; i < timingLayer->GetEffectCount(); i++) {
        Effect* timingMark = timingLayer->GetEffect(i);
        long markStart = timingMark->GetStartTimeMS();
        long markEnd = timingMark->GetEndTimeMS();

        if (markEnd <= region->startTimeMS || markStart >= region->endTimeMS) continue;
        if (markStart < region->startTimeMS) markStart = region->startTimeMS;
        if (markStart < sourceEnd && markEnd > sourceStart) continue;

        long newStart = RoundToMultipleOfPeriod((int)markStart, freq);
        long newEnd = RoundToMultipleOfPeriod((int)(newStart + sourceDuration), freq);
        if (newEnd > region->endTimeMS) {
            newEnd = RoundToMultipleOfPeriod(region->endTimeMS, freq);
        }
        if (newEnd <= newStart) continue;
        if (el->HasEffectsInTimeRange((int)newStart, (int)newEnd)) continue;

        Effect* newEffect = el->AddEffect(0, effectName, settings, palette,
            (int)newStart, (int)newEnd, EFFECT_NOT_SELECTED, false);
        if (newEffect != nullptr) {
            undoMgr.CaptureAddedEffect(el->GetParentElement()->GetName(),
                el->GetIndex(), newEffect->GetID());
            effectsCreated++;
        }
    }

    if (effectsCreated == 0) {
        undoMgr.CancelLastStep();
    }
    se.IncrementChangeCount(nullptr);
    return effectsCreated;
}

@end

// MARK: - Effect preset library (PRE-1)

namespace {
// Resolve a backslash-separated group path to a group node, or the
// root when `path` is empty. Returns nullptr when a path segment is
// missing or names a leaf preset rather than a group.
EffectPresetGroup* ResolveGroup(EffectPresetManager& mgr, NSString* path) {
    std::string p = path ? std::string([path UTF8String]) : std::string();
    if (p.empty()) {
        return &mgr.GetRoot();
    }
    EffectPresetItem* item = mgr.FindItemByPath(p, '\\');
    if (item == nullptr || !item->IsGroup()) {
        return nullptr;
    }
    return static_cast<EffectPresetGroup*>(item);
}

void CollectPresetTree(const EffectPresetGroup& group,
                       const std::string& prefix,
                       NSMutableArray* out) {
    for (const auto& child : group.GetChildren()) {
        std::string path = prefix.empty() ? child->GetName()
                                          : prefix + "\\" + child->GetName();
        if (child->IsGroup()) {
            [out addObject:@{
                @"path": [NSString stringWithUTF8String:path.c_str()],
                @"isGroup": @YES,
                @"layerCount": @0,
                @"durationMS": @0,
            }];
            CollectPresetTree(static_cast<const EffectPresetGroup&>(*child),
                              path, out);
        } else {
            const auto& preset = static_cast<const EffectPreset&>(*child);
            [out addObject:@{
                @"path": [NSString stringWithUTF8String:path.c_str()],
                @"isGroup": @NO,
                @"layerCount": @(preset.GetLayerCount()),
                @"durationMS": @(preset.GetDurationMS()),
            }];
        }
    }
}

std::vector<std::string> SplitOn(const std::string& s, char sep) {
    std::vector<std::string> parts;
    std::string cur;
    for (char ch : s) {
        if (ch == sep) {
            parts.push_back(cur);
            cur.clear();
        } else {
            cur.push_back(ch);
        }
    }
    parts.push_back(cur);
    return parts;
}
} // namespace

@implementation XLSequenceDocument (EffectPresets)

- (NSArray<NSDictionary*>*)presetTree {
    NSMutableArray* out = [NSMutableArray array];
    CollectPresetTree(_context->GetEffectPresetManager().GetRoot(), "", out);
    return out;
}

- (NSData*)presetThumbnailBGRAAtPath:(NSString*)path outputSize:(int*)outputSize {
    if (outputSize) *outputSize = 0;
    if (!_context || !path) return nil;

    EffectPreset* preset =
        _context->GetEffectPresetManager().FindPresetByPath(std::string([path UTF8String]), '\\');
    if (preset == nullptr) {
        preset = _context->GetBasePresetManager().FindPresetByPath(std::string([path UTF8String]), '\\');
    }
    if (preset == nullptr) return nil;

    const std::string& blob = preset->GetSettings();
    if (blob.empty()) return nil;

    // Pull the first renderable effect line out of the CopyFormat1 blob.
    // The thumbnail only needs a representative effect; multi-row presets
    // collapse to their anchor effect for the still.
    std::string fxName, fxSettings, fxPalette;
    int fxStart = 0, fxEnd = 0;
    for (const std::string& line : SplitOn(blob, '\n')) {
        if (line.empty()) continue;
        std::vector<std::string> f = SplitOn(line, '\t');
        if (f.empty() || f[0] == "CopyFormat1" || f[0] == "CopyFormatAC" ||
            f[0] == "None" || f.size() < 6) {
            continue;
        }
        fxName = f[0];
        fxSettings = f[1];
        fxPalette = f[2];
        fxStart = (int)std::strtol(f[3].c_str(), nullptr, 10);
        fxEnd = (int)std::strtol(f[4].c_str(), nullptr, 10);
        break;
    }
    if (fxName.empty() || fxEnd <= fxStart) return nil;

    // Serialize against the shared single preset model — the same
    // scaffolding the shader-preview path uses, so guard with the same
    // mutex to avoid two renders fighting over the one effect layer.
    static std::mutex s_presetThumbMutex;
    std::scoped_lock lock(s_presetThumbMutex);

    Model* model = _context->GetPresetModel();
    if (!model) return nil;
    SequenceElements& se = _context->GetPresetSequenceElements();
    Element* elem = se.GetElement(model->GetName());
    if (!elem) return nil;

    for (const auto& layer : elem->GetEffectLayers()) {
        layer->DeleteAllEffects();
    }
    if (elem->GetEffectLayerCount() == 0) {
        elem->AddEffectLayer();
    }
    EffectLayer* el = elem->GetEffectLayer(0);
    if (!el) return nil;

    constexpr int frameTimeMs = 50;
    const int spanMs = fxEnd - fxStart;
    // Cap the render span so an absurdly long preset doesn't stall the
    // thumbnail; one second of frames is plenty for a representative still.
    const int cappedSpan = std::min(spanMs, 1000);
    el->AddEffect(0, fxName, fxSettings, fxPalette, 0, cappedSpan, EFFECT_NOT_SELECTED, false);

    const size_t numFrames = std::max<size_t>(1, (size_t)(cappedSpan / frameTimeMs));
    auto frames = _context->RenderEffectToFrames(model,
                                                  _context->GetPresetSequenceData(),
                                                  se, numFrames, frameTimeMs);
    el->DeleteAllEffects();
    if (frames.empty() || !frames.back() || !frames.back()->IsOk()) return nil;

    // Use the last frame as the representative still — by then animated
    // effects have developed past their lead-in blank.
    const xlImage& img = *frames.back();
    int w = img.GetWidth();
    int h = img.GetHeight();
    if (w <= 0 || h <= 0) return nil;

    // Convert RGBA → BGRA premultipliedFirst-compatible byte order so the
    // existing Swift makeCGImage helper renders it correctly.
    NSMutableData* out = [NSMutableData dataWithLength:(NSUInteger)w * h * 4];
    uint8_t* dst = (uint8_t*)out.mutableBytes;
    for (int y = 0; y < h; ++y) {
        for (int x = 0; x < w; ++x) {
            size_t di = ((size_t)y * w + x) * 4;
            dst[di + 0] = img.GetBlue(x, y);
            dst[di + 1] = img.GetGreen(x, y);
            dst[di + 2] = img.GetRed(x, y);
            dst[di + 3] = 255;
        }
    }
    if (outputSize) *outputSize = w;
    return out;
}

- (BOOL)savePresetFromRows:(NSArray<NSNumber*>*)rows
             effectIndices:(NSArray<NSNumber*>*)effectIndices
                 groupPath:(NSString*)groupPath
                      name:(NSString*)name {
    std::string trimmedName = name ? std::string([name UTF8String]) : std::string();
    if (trimmedName.empty()) return NO;
    if (rows.count == 0 || rows.count != effectIndices.count) return NO;

    auto& mgr = _context->GetEffectPresetManager();
    EffectPresetGroup* parent = ResolveGroup(mgr, groupPath);
    if (parent == nullptr) return NO;

    // Anchor rows so the lowest selected grid row becomes row 0 in the
    // serialized blob — desktop ParseLayers / paste expect relative rows.
    int minRow = INT_MAX;
    for (NSNumber* r in rows) {
        minRow = std::min(minRow, r.intValue);
    }
    if (minRow == INT_MAX) return NO;

    std::string effectData;
    int numEffects = 0;
    for (NSUInteger i = 0; i < rows.count; ++i) {
        int rowIndex = rows[i].intValue;
        int effectIndex = effectIndices[i].intValue;
        auto* layer = [self effectLayerForRow:rowIndex];
        if (!layer || effectIndex < 0 || effectIndex >= layer->GetEffectCount()) {
            continue;
        }
        Effect* e = layer->GetEffect(effectIndex);
        if (!e) continue;
        int relRow = rowIndex - minRow;
        effectData += e->GetEffectName() + "\t" + e->GetSettingsAsString() + "\t" +
                      e->GetPaletteAsString() + "\t" +
                      std::to_string(e->GetStartTimeMS()) + "\t" +
                      std::to_string(e->GetEndTimeMS()) + "\t" +
                      std::to_string(relRow) + "\t-1000\tNO_PASTE_BY_CELL\tLAYER:0\n";
        ++numEffects;
    }
    if (numEffects == 0) return NO;

    std::string blob = "CopyFormat1\t0\t" + std::to_string(numEffects) +
                       "\t0\t0\t-1\tNO_PASTE_BY_CELL\n" + effectData;

    mgr.AddPreset(parent, trimmedName, blob,
                  XLIGHTS_RGBEFFECTS_VERSION, xlights_version_string);
    return YES;
}

- (BOOL)presetUsesLayersAtPath:(NSString*)path {
    if (!path) return NO;
    auto& mgr = _context->GetEffectPresetManager();
    EffectPreset* preset = mgr.FindPresetByPath(std::string([path UTF8String]), '\\');
    if (preset == nullptr) return NO;
    return preset->GetSettings().find("\tLAYER:") != std::string::npos ? YES : NO;
}

- (BOOL)applyPresetFromManager:(EffectPresetManager&)mgr
                        atPath:(NSString*)path
                         toRow:(int)rowIndex
                     atStartMS:(int)startMS
                   usingLayers:(BOOL)usingLayers {
    if (!path || rowIndex < 0) return NO;
    EffectPreset* preset = mgr.FindPresetByPath(std::string([path UTF8String]), '\\');
    if (preset == nullptr) return NO;

    const std::string& blob = preset->GetSettings();
    if (blob.empty()) return NO;

    // Parse the CopyFormat1 effect lines into relative-offset records,
    // then lay them out anchored at (rowIndex, startMS) the same way the
    // clipboard paste does.
    struct Rec { std::string name, settings, palette; int relRow; int layer; int start; int end; };
    std::vector<Rec> recs;
    int minStart = INT_MAX;
    int minRow = INT_MAX;
    for (const std::string& line : SplitOn(blob, '\n')) {
        if (line.empty()) continue;
        std::vector<std::string> f = SplitOn(line, '\t');
        if (f.empty() || f[0] == "CopyFormat1" || f[0] == "CopyFormatAC" ||
            f[0] == "None" || f.size() < 6) {
            continue;
        }
        Rec r;
        r.name = f[0];
        r.settings = f[1];
        r.palette = f[2];
        r.start = (int)std::strtol(f[3].c_str(), nullptr, 10);
        r.end = (int)std::strtol(f[4].c_str(), nullptr, 10);
        r.relRow = (int)std::strtol(f[5].c_str(), nullptr, 10);
        r.layer = 0;
        for (size_t fi = f.size(); fi-- > 6;) {
            if (f[fi].rfind("LAYER:", 0) == 0) {
                r.layer = (int)std::strtol(f[fi].c_str() + 6, nullptr, 10);
                break;
            }
        }
        if (r.end <= r.start) continue;
        minStart = std::min(minStart, r.start);
        minRow = std::min(minRow, r.relRow);
        recs.push_back(std::move(r));
    }
    if (recs.empty() || minStart == INT_MAX) return NO;

    bool any = false;

    if (usingLayers) {
        // Stack every effect onto the anchor row's element, on successive
        // effect layers (growing the element as needed). Ignores relRow.
        auto* row = _context->GetSequenceElements().GetRowInformation(rowIndex);
        if (!row || !row->element) return NO;
        Element* element = row->element;
        for (const Rec& r : recs) {
            while ((int)element->GetEffectLayerCount() <= r.layer) {
                element->AddEffectLayer();
            }
            EffectLayer* layer = element->GetEffectLayer(r.layer);
            if (!layer) continue;
            int targetStart = startMS + (r.start - minStart);
            int targetEnd = targetStart + (r.end - r.start);
            if (targetEnd <= targetStart) continue;
            Effect* e = layer->AddEffect(0, r.name, r.settings, r.palette,
                                         targetStart, targetEnd, EFFECT_NOT_SELECTED, false);
            if (e) {
                _context->RenderEffectForModel(element->GetModelName(),
                                               targetStart, targetEnd, false);
                any = true;
            }
        }
        _context->GetSequenceElements().PopulateRowInformation();
        return any ? YES : NO;
    }

    int rowCount = (int)_context->GetSequenceElements().GetRowInformationSize();
    for (const Rec& r : recs) {
        int targetRow = rowIndex + (r.relRow - minRow);
        if (targetRow < 0 || targetRow >= rowCount) continue;
        auto* layer = [self effectLayerForRow:targetRow];
        if (!layer) continue;
        int targetStart = startMS + (r.start - minStart);
        int targetEnd = targetStart + (r.end - r.start);
        if (targetEnd <= targetStart) continue;
        Effect* e = layer->AddEffect(0, r.name, r.settings, r.palette,
                                     targetStart, targetEnd, EFFECT_NOT_SELECTED, false);
        if (e) {
            Element* element = layer->GetParentElement();
            if (element) {
                _context->RenderEffectForModel(element->GetModelName(),
                                               targetStart, targetEnd, false);
            }
            any = true;
        }
    }
    return any ? YES : NO;
}

- (BOOL)applyPresetAtPath:(NSString*)path
                    toRow:(int)rowIndex
                atStartMS:(int)startMS
              usingLayers:(BOOL)usingLayers {
    return [self applyPresetFromManager:_context->GetEffectPresetManager()
                                 atPath:path
                                  toRow:rowIndex
                              atStartMS:startMS
                            usingLayers:usingLayers];
}


- (BOOL)addPresetGroupNamed:(NSString*)name
              inGroupAtPath:(NSString*)parentGroupPath {
    std::string n = name ? std::string([name UTF8String]) : std::string();
    if (n.empty()) return NO;
    auto& mgr = _context->GetEffectPresetManager();
    EffectPresetGroup* parent = ResolveGroup(mgr, parentGroupPath);
    if (parent == nullptr || parent->HasChildNamed(n)) return NO;
    mgr.AddGroup(parent, n);
    return YES;
}

- (BOOL)renamePresetItemAtPath:(NSString*)path to:(NSString*)newName {
    std::string n = newName ? std::string([newName UTF8String]) : std::string();
    if (!path || n.empty()) return NO;
    auto& mgr = _context->GetEffectPresetManager();
    EffectPresetItem* item = mgr.FindItemByPath(std::string([path UTF8String]), '\\');
    if (item == nullptr) return NO;
    EffectPresetGroup* parent = item->GetParent();
    if (parent && parent->FindChildByName(n) != nullptr &&
        parent->FindChildByName(n) != item) {
        return NO; // sibling collision
    }
    mgr.RenameItem(item, n);
    return YES;
}

- (BOOL)deletePresetItemAtPath:(NSString*)path {
    if (!path) return NO;
    auto& mgr = _context->GetEffectPresetManager();
    EffectPresetItem* item = mgr.FindItemByPath(std::string([path UTF8String]), '\\');
    if (item == nullptr) return NO;
    mgr.Remove(item);
    return YES;
}

- (BOOL)movePresetItemFromPath:(NSString*)fromPath
                   toGroupPath:(NSString*)toGroupPath {
    if (!fromPath) return NO;
    auto& mgr = _context->GetEffectPresetManager();
    EffectPresetItem* item = mgr.FindItemByPath(std::string([fromPath UTF8String]), '\\');
    if (item == nullptr) return NO;
    EffectPresetGroup* dest = ResolveGroup(mgr, toGroupPath);
    if (dest == nullptr) return NO;
    // Refuse moving a group into itself or one of its descendants.
    for (EffectPresetGroup* g = dest; g != nullptr; g = g->GetParent()) {
        if (g == item) return NO;
    }
    if (dest->HasChildNamed(item->GetName())) return NO;
    mgr.MoveItem(item, dest);
    return YES;
}

- (BOOL)movePresetItemFromPath:(NSString*)fromPath
                   toGroupPath:(NSString*)toGroupPath
                       toIndex:(NSInteger)toIndex {
    if (!fromPath) return NO;
    auto& mgr = _context->GetEffectPresetManager();
    EffectPresetItem* item = mgr.FindItemByPath(std::string([fromPath UTF8String]), '\\');
    if (item == nullptr) return NO;
    EffectPresetGroup* dest = ResolveGroup(mgr, toGroupPath);
    if (dest == nullptr) return NO;
    // Refuse moving a group into itself or one of its descendants.
    for (EffectPresetGroup* g = dest; g != nullptr; g = g->GetParent()) {
        if (g == item) return NO;
    }
    // A name collision is only a problem when crossing into a different
    // group; within the same group the item is just being reordered.
    if (dest != item->GetParent() && dest->HasChildNamed(item->GetName())) {
        return NO;
    }

    // Translate the destination child index into an insert-after sibling,
    // skipping the moved item itself when it already lives in `dest`.
    const auto& kids = dest->GetChildren();
    EffectPresetItem* insertAfter = nullptr;
    NSInteger seen = 0;
    for (const auto& child : kids) {
        if (child.get() == item) continue;
        if (seen >= toIndex) break;
        insertAfter = child.get();
        ++seen;
    }
    if (insertAfter == item) insertAfter = nullptr;
    mgr.MoveItem(item, dest, insertAfter);
    return YES;
}

- (BOOL)updatePresetAtPath:(NSString*)path
                   fromRow:(int)row
               effectIndex:(int)effectIndex {
    if (!path) return NO;
    auto& mgr = _context->GetEffectPresetManager();
    EffectPreset* preset = mgr.FindPresetByPath(std::string([path UTF8String]), '\\');
    if (preset == nullptr) return NO;

    auto* layer = [self effectLayerForRow:row];
    if (!layer || effectIndex < 0 || effectIndex >= layer->GetEffectCount()) return NO;
    Effect* e = layer->GetEffect(effectIndex);
    if (!e) return NO;

    std::string effectData = e->GetEffectName() + "\t" + e->GetSettingsAsString() + "\t" +
                             e->GetPaletteAsString() + "\t" +
                             std::to_string(e->GetStartTimeMS()) + "\t" +
                             std::to_string(e->GetEndTimeMS()) +
                             "\t0\t-1000\tNO_PASTE_BY_CELL\tLAYER:0\n";
    std::string blob = "CopyFormat1\t0\t1\t0\t0\t-1\tNO_PASTE_BY_CELL\n" + effectData;

    mgr.UpdatePresetSettings(preset, blob, xlights_version_string);
    return YES;
}

- (BOOL)importPresetsFromPath:(NSString*)xmlPath
              intoGroupAtPath:(NSString*)groupPath {
    if (!xmlPath) return NO;
    std::string p = [xmlPath UTF8String];
    ObtainAccessToURL(p, false);
    if (!FileExists(p)) return NO;

    pugi::xml_document doc;
    if (!doc.load_file(p.c_str())) return NO;

    auto& mgr = _context->GetEffectPresetManager();
    EffectPresetGroup* parent = ResolveGroup(mgr, groupPath);
    if (parent == nullptr) return NO;

    // Accept either a bare <effects> fragment or a full rgbeffects doc
    // (<xrgb>/<xlights> with an <effects> child).
    pugi::xml_node effectsNode = doc.child("effects");
    if (!effectsNode) {
        pugi::xml_node root = doc.child("xrgb");
        if (!root) root = doc.child("xlights");
        if (root) effectsNode = root.child("effects");
    }
    if (!effectsNode) return NO;
    mgr.ImportFromXml(effectsNode, parent);
    return YES;
}

- (BOOL)exportPresetsToPath:(NSString*)path {
    if (!path) return NO;
    std::string p = [path UTF8String];
    ObtainAccessToURL(p, true);
    return _context->GetEffectPresetManager().SaveJsonFile(p) ? YES : NO;
}

- (BOOL)savePresets {
    return _context->SaveEffectPresets() ? YES : NO;
}

#pragma mark - Song Structure Regions (#6268)

- (NSArray<NSDictionary*>*)songStructureRegions {
    auto& ssm = _context->GetSequenceElements().GetSongStructureManager();
    NSMutableArray<NSDictionary*>* out = [NSMutableArray array];
    const auto& regions = ssm.GetRegions();
    for (const auto& r : regions) {
        [out addObject:@{
            @"id": @(r.id),
            @"startMS": @(r.startTimeMS),
            @"endMS": @(r.endTimeMS),
            @"name": [NSString stringWithUTF8String:r.name.c_str()],
            @"colorARGB": @(r.colorARGB)
        }];
    }
    return out;
}

- (NSArray<NSString*>*)songStructureViewNames {
    auto& ssm = _context->GetSequenceElements().GetSongStructureManager();
    NSMutableArray<NSString*>* out = [NSMutableArray array];
    for (size_t i = 0; i < ssm.GetViewCount(); ++i) {
        [out addObject:[NSString stringWithUTF8String:ssm.GetViewName(i).c_str()]];
    }
    return out;
}

- (NSInteger)songStructureActiveViewIndex {
    return _context->GetSequenceElements().GetSongStructureManager().GetActiveViewIndex();
}

- (void)setSongStructureActiveViewIndex:(NSInteger)index {
    _context->GetSequenceElements().GetSongStructureManager().SetActiveView((int)index);
    _context->GetSequenceElements().IncrementChangeCount(nullptr);
}

- (NSInteger)addSongStructureView:(NSString*)name {
    auto& ssm = _context->GetSequenceElements().GetSongStructureManager();
    int idx = ssm.AddView(name ? std::string([name UTF8String]) : std::string("View"));
    _context->GetSequenceElements().IncrementChangeCount(nullptr);
    return idx;
}

- (NSInteger)duplicateSongStructureViewAtIndex:(NSInteger)sourceIndex
                                       withName:(NSString*)name {
    auto& ssm = _context->GetSequenceElements().GetSongStructureManager();
    int idx = ssm.DuplicateView((int)sourceIndex,
                                name ? std::string([name UTF8String]) : std::string("Copy"));
    _context->GetSequenceElements().IncrementChangeCount(nullptr);
    return idx;
}

- (void)renameSongStructureViewAtIndex:(NSInteger)index toName:(NSString*)name {
    if (!name) return;
    _context->GetSequenceElements().GetSongStructureManager().RenameView((int)index, std::string([name UTF8String]));
    _context->GetSequenceElements().IncrementChangeCount(nullptr);
}

- (void)deleteSongStructureViewAtIndex:(NSInteger)index {
    _context->GetSequenceElements().GetSongStructureManager().DeleteView((int)index);
    _context->GetSequenceElements().IncrementChangeCount(nullptr);
}

- (void)addSongStructureBoundaryAtMS:(int)timeMS {
    auto& se = _context->GetSequenceElements();
    auto& ssm = se.GetSongStructureManager();
    if (ssm.GetViewCount() == 0) {
        ssm.SetActiveView(ssm.AddView("Default"));
    }
    ssm.AddBoundary(timeMS, se.GetSequenceEnd());
    se.IncrementChangeCount(nullptr);
}

- (void)deleteSongStructureBoundaryNearMS:(int)timeMS {
    auto& se = _context->GetSequenceElements();
    se.GetSongStructureManager().DeleteBoundary(timeMS);
    se.IncrementChangeCount(nullptr);
}

- (void)moveSongStructureBoundaryFromMS:(int)oldTimeMS toMS:(int)newTimeMS {
    auto& se = _context->GetSequenceElements();
    se.GetSongStructureManager().MoveBoundary(oldTimeMS, newTimeMS);
    se.IncrementChangeCount(nullptr);
}

- (int)nearestSongStructureBoundaryToMS:(int)timeMS toleranceMS:(int)toleranceMS {
    return _context->GetSequenceElements().GetSongStructureManager().FindNearestBoundary(timeMS, toleranceMS);
}

- (void)setSongStructureRegionNameForID:(int)regionID name:(NSString*)name {
    auto& se = _context->GetSequenceElements();
    auto& ssm = se.GetSongStructureManager();
    for (size_t i = 0; i < ssm.GetRegionCount(); ++i) {
        if (ssm.GetRegion(i).id == regionID) {
            ssm.SetRegionName(i, name ? std::string([name UTF8String]) : std::string());
            se.IncrementChangeCount(nullptr);
            return;
        }
    }
}

- (void)setSongStructureRegionColorForID:(int)regionID colorARGB:(uint32_t)colorARGB {
    auto& se = _context->GetSequenceElements();
    auto& ssm = se.GetSongStructureManager();
    for (size_t i = 0; i < ssm.GetRegionCount(); ++i) {
        if (ssm.GetRegion(i).id == regionID) {
            ssm.SetRegionColor(i, colorARGB);
            se.IncrementChangeCount(nullptr);
            return;
        }
    }
}

- (uint32_t)songStructurePaletteColorAtIndex:(int)index {
    return SongStructureManager::GetPaletteColor(index);
}

- (NSInteger)createSongRegionsFromTimingRow:(int)rowIndex {
    auto& se = _context->GetSequenceElements();
    auto* row = se.GetRowInformation(rowIndex);
    if (!row || !row->element ||
        row->element->GetType() != ElementType::ELEMENT_TYPE_TIMING) return 0;
    auto* te = dynamic_cast<TimingElement*>(row->element);
    if (!te) return 0;
    EffectLayer* el = te->GetEffectLayer(0);
    if (!el || el->GetEffectCount() == 0) return 0;

    auto& ssm = se.GetSongStructureManager();
    if (ssm.GetViewCount() > 0 || ssm.AnyViewHasRegions()) {
        ssm.Clear();
    }
    int viewIdx = ssm.AddView("Default");
    ssm.SetActiveView(viewIdx);

    int seqEndMS = se.GetSequenceEnd();
    int regionId = 1;
    int colorIdx = 0;
    int lastEndMS = 0;
    std::vector<SongStructureRegion> regions;

    for (int k = 0; k < el->GetEffectCount(); ++k) {
        Effect* eff = el->GetEffect(k);
        int markStart = eff->GetStartTimeMS();
        int markEnd = eff->GetEndTimeMS();
        std::string label = eff->GetEffectName();
        if (label.empty()) {
            label = "Region " + std::to_string(k + 1);
        }
        if (markStart > lastEndMS) {
            regions.emplace_back(regionId++, lastEndMS, markStart, "", 0x40808080);
        }
        uint32_t color = SongStructureManager::GetPaletteColor(colorIdx % SongStructureManager::PALETTE_SIZE);
        regions.emplace_back(regionId++, markStart, markEnd, label, color);
        colorIdx++;
        lastEndMS = markEnd;
    }
    if (lastEndMS < seqEndMS) {
        regions.emplace_back(regionId++, lastEndMS, seqEndMS, "", 0x40808080);
    }

    ssm.SetRegions(regions);
    se.IncrementChangeCount(nullptr);
    return (NSInteger)regions.size();
}

// MARK: From Base presets (#6450)

- (BOOL)hasBasePresets {
    return _context->GetBasePresetManager().GetRoot().GetChildren().empty() ? NO : YES;
}

- (void)reloadBasePresets {
    _context->LoadBasePresets();
}

- (NSArray<NSDictionary*>*)basePresetTree {
    NSMutableArray* out = [NSMutableArray array];
    CollectPresetTree(_context->GetBasePresetManager().GetRoot(), "", out);
    return out;
}

- (BOOL)applyBasePresetAtPath:(NSString*)path
                        toRow:(int)rowIndex
                    atStartMS:(int)startMS
                  usingLayers:(BOOL)usingLayers {
    return [self applyPresetFromManager:_context->GetBasePresetManager()
                                 atPath:path
                                  toRow:rowIndex
                              atStartMS:startMS
                            usingLayers:usingLayers];
}

- (BOOL)basePresetUsesLayersAtPath:(NSString*)path {
    if (!path) return NO;
    EffectPreset* preset = _context->GetBasePresetManager().FindPresetByPath(std::string([path UTF8String]), '\\');
    if (preset == nullptr) return NO;
    return preset->GetSettings().find("\tLAYER:") != std::string::npos ? YES : NO;
}

@end
