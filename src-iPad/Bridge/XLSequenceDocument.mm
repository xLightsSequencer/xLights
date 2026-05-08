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
