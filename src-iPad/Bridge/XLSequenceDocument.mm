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
#include "utils/UtilFunctions.h"
#include "utils/string_utils.h"
#include "lyrics/PhonemeDictionary.h"
#include "lyrics/LyricBreakdown.h"
#include "render/SequenceViewManager.h"
#include "effects/RenderableEffect.h"
#include "effects/EffectManager.h"
#include "effects/ShaderEffect.h"
#include "graphics/xlGraphicsAccumulators.h"
#include "media/AudioManager.h"
#include "media/OnsetDetector.h"
#include "media/SoundClassifier.h"
#include "media/TempoDetector.h"
#include "media/PitchDetector.h"
#include "media/ChordDetector.h"
#include "media/Spectrogram.h"
#include "media/AIModelStore.h"
#include "media/StemSeparator.h"
#include "../../dependencies/libxlsxwriter/third_party/minizip/unzip.h"
#include <filesystem>
#include "media/MediaCompatibility.h"
#include "media/VideoReader.h"
#include "render/ValueCurve.h"
#include "models/Model.h"
#include "models/ModelManager.h"
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
#include "models/ArchesModel.h"
#include "models/ThreePointScreenLocation.h"
#include "models/ControllerConnection.h"
#include "controllers/ControllerCaps.h"
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
#include "utils/FileUtils.h"
#include "utils/ExternalHooks.h"
#include "utils/xlImage.h"
#include "xLightsVersion.h"
#include "globals.h"
#include "diagnostics/CheckSequenceReport.h"
#include "diagnostics/SequenceChecker.h"

#import "XLCheckSequenceIssue.h"

#include <nlohmann/json.hpp>
#include <spdlog/spdlog.h>

#include <algorithm>
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

// Private helpers. extrasFor: takes a C++ Model* so it lives here
// (not in the ObjC++-free header). Declared up front so it can be
// called from any method below without ordering concerns.
@interface XLSequenceDocument ()
- (NSDictionary<NSString*, id>*)extrasFor:(Model*)m;
- (NSDictionary<NSString*, id>*)controllerConnectionFor:(Model*)m;
- (std::string)joinIndexedNames:(NSArray<NSString*>*)names;
@end

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
}

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

- (void)clearUndoHistory {
    if (!_context) return;
    _context->GetSequenceElements().get_undo_mgr().Clear();
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
    NSMutableArray<NSString*>* out = [NSMutableArray array];
    [out addObject:@"Default"];
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
    return @{
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
    };
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
        case ModelScreenLocation::MSLTOOL::TOOL_TRANSLATE: return @"translate";
        case ModelScreenLocation::MSLTOOL::TOOL_SCALE:     return @"scale";
        case ModelScreenLocation::MSLTOOL::TOOL_ROTATE:    return @"rotate";
        case ModelScreenLocation::MSLTOOL::TOOL_XY_TRANS:  return @"xy_trans";
        case ModelScreenLocation::MSLTOOL::TOOL_ELEVATE:   return @"elevate";
        default:                                            return @"none";
    }
}

- (BOOL)setAxisTool:(NSString*)tool forModel:(NSString*)modelName {
    if (!_context || !modelName || modelName.length == 0 || !tool) return NO;
    Model* m = _context->GetModelManager()[modelName.UTF8String];
    if (!m) return NO;
    ModelScreenLocation::MSLTOOL mslTool;
    if      ([tool isEqualToString:@"translate"]) mslTool = ModelScreenLocation::MSLTOOL::TOOL_TRANSLATE;
    else if ([tool isEqualToString:@"scale"])     mslTool = ModelScreenLocation::MSLTOOL::TOOL_SCALE;
    else if ([tool isEqualToString:@"rotate"])    mslTool = ModelScreenLocation::MSLTOOL::TOOL_ROTATE;
    else if ([tool isEqualToString:@"xy_trans"])  mslTool = ModelScreenLocation::MSLTOOL::TOOL_XY_TRANS;
    else if ([tool isEqualToString:@"elevate"])   mslTool = ModelScreenLocation::MSLTOOL::TOOL_ELEVATE;
    else                                          return NO;
    m->GetModelScreenLocation().SetAxisTool(mslTool);
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
    return YES;
}

- (NSArray<NSString*>*)availableModelTypesForCreation {
    // Curated for first-cut iPad Add-Model. Each name matches a
    // case in `ModelManager::CreateDefaultModel`. Strings here are
    // the literal type tags the factory expects; SwiftUI shows
    // friendlier labels.
    return @[
        @"Arches",
        @"Candy Canes",
        @"Channel Block",
        @"Circle",
        @"Cube",
        @"Custom",
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
        @"Wreath",
    ];
}

- (BOOL)deleteModel:(NSString*)modelName {
    if (!_context || !modelName || modelName.length == 0) return NO;
    _context->AbortRender(5000);
    return _context->GetModelManager().Delete(modelName.UTF8String) ? YES : NO;
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

- (NSArray<NSString*>*)modelGroupsInActiveLayoutGroup {
    NSMutableArray<NSString*>* out = [NSMutableArray array];
    if (!_context || !_context->HasModelManager()) return out;
    const std::string active = _context->GetActiveLayoutGroup();
    for (const auto& [name, model] : _context->GetModelManager().GetModels()) {
        if (!model) continue;
        if (model->GetDisplayAs() != DisplayAsType::ModelGroup) continue;
        const std::string& g = model->GetLayoutGroup();
        if (g == active || g == "All Previews") {
            [out addObject:[NSString stringWithUTF8String:name.c_str()]];
        }
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
    std::string active = _context->GetActiveLayoutGroup();
    grp->SetLayoutGroup(active.empty() ? "Default" : active);
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
        const std::string& g = vo->GetLayoutGroup();
        if (g == active || g == "All Previews" || g.empty()) {
            [out addObject:[NSString stringWithUTF8String:vo->GetName().c_str()]];
        }
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
    // immediately renders in the current preview.
    vo->SetLayoutGroup(_context->GetActiveLayoutGroup());
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
    } else {
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
    return _context->GetOutputManager().StartOutput();
}

- (void)stopOutput {
    _context->GetOutputManager().StopOutput();
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

// iPad-side callbacks for SequenceChecker. The base class defaults
// (no per-check disable, render-cache "Enabled") match the iPad's
// lack of equivalent settings UI; the AVFoundation video probe and
// optional progress block are the only real overrides.
class iPadSequenceCheckerCallbacks final : public SequenceCheckerCallbacks {
public:
    using ProgressBlock = void (^)(int, NSString*);

    explicit iPadSequenceCheckerCallbacks(ProgressBlock progress)
        : _progress(progress ? [progress copy] : nil) {}

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

- (NSArray<XLCheckSequenceIssue*>*)runSequenceCheckWithProgress:
    (void (^)(int, NSString*))progress {
    NSMutableArray<XLCheckSequenceIssue*>* issues = [NSMutableArray array];
    if (!_context || !_context->IsSequenceLoaded()) return issues;

    iPadSequenceCheckerCallbacks callbacks(progress);
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

@end
