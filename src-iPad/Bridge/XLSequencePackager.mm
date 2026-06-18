//
//  XLSequencePackager.mm
//  xLights-iPadLib
//

#import "XLSequencePackager.h"
#import "XLSequenceDocument.h"
#import "iPadRenderContext.h"

#include "render/SequencePackage.h"
#include "render/SequenceFile.h"
#include "render/SequenceElements.h"
#include "models/ModelManager.h"
#include "models/ViewObjectManager.h"

#include <filesystem>
#include <string>
#include <vector>

#include <spdlog/spdlog.h>

static NSString* const kPackageSequenceErrorDomain = @"XLSequencePackagerError";

static NSError* MakePackError(NSInteger code, NSString* message) {
    return [NSError errorWithDomain:kPackageSequenceErrorDomain
                               code:code
                           userInfo:@{ NSLocalizedDescriptionKey: message }];
}

@implementation XLSequencePackager

+ (nullable NSURL*)packageSequenceForDocument:(XLSequenceDocument*)document
                                 excludeAudio:(BOOL)excludeAudio
                                excludeVideos:(BOOL)excludeVideos
                                     warnings:(NSArray<NSString*>* _Nullable * _Nullable)outWarnings
                                        error:(NSError* _Nullable * _Nullable)outError {
    if (!document) {
        if (outError) *outError = MakePackError(1, @"No document supplied");
        return nil;
    }

    iPadRenderContext* ctx = static_cast<iPadRenderContext*>([document renderContext]);
    if (!ctx) {
        if (outError) *outError = MakePackError(2, @"Render context not ready");
        return nil;
    }
    if (!ctx->HasModelManager() || !ctx->HasViewObjectManager()) {
        if (outError) *outError = MakePackError(3, @"Show folder not loaded");
        return nil;
    }
    SequenceFile* seqFile = ctx->GetSequenceFile();
    if (!seqFile || !seqFile->IsOpen()) {
        if (outError) *outError = MakePackError(4, @"No sequence is open");
        return nil;
    }

    std::string showDir = ctx->GetShowDirectory();
    std::string xsqPath = seqFile->GetFullPath();
    std::string mediaFile = seqFile->GetMediaFile();
    std::string xsqStem = std::filesystem::path(xsqPath).stem().string();
    if (xsqStem.empty()) {
        xsqStem = "Sequence";
    }

    NSString* tmpDir = NSTemporaryDirectory();
    NSString* stagingDir = [tmpDir stringByAppendingPathComponent:
                            [NSString stringWithFormat:@"SequencePackage-%@", [NSUUID UUID].UUIDString]];
    NSFileManager* fm = [NSFileManager defaultManager];
    NSError* mkErr = nil;
    if (![fm createDirectoryAtPath:stagingDir
       withIntermediateDirectories:YES
                        attributes:nil
                             error:&mkErr]) {
        if (outError) {
            *outError = MakePackError(5,
                [NSString stringWithFormat:@"Failed to create staging directory: %@",
                    mkErr.localizedDescription ?: @"unknown"]);
        }
        return nil;
    }
    std::filesystem::path outPath =
        std::filesystem::path([stagingDir UTF8String]) / (xsqStem + ".xsqz");

    std::vector<std::string> altAudio;
    for (int i = 0; i < seqFile->GetAltTrackCount(); ++i) {
        const auto& track = seqFile->GetAltTrack(i);
        if (!track.path.empty()) {
            altAudio.push_back(track.path);
        }
    }

    std::vector<std::string> extras;
    const std::string& backgroundImage = ctx->GetBackgroundImage();
    if (!backgroundImage.empty()) {
        extras.push_back(backgroundImage);
    }

    SequencePackOptions opts;
    opts.excludeAudio  = excludeAudio  ? true : false;
    opts.excludeVideos = excludeVideos ? true : false;

    std::vector<std::string> warnings;
    bool ok = SequencePackage::Pack(outPath,
                                    showDir,
                                    xsqPath,
                                    mediaFile,
                                    altAudio,
                                    extras,
                                    ctx->GetSequenceElements().GetSequenceMedia(),
                                    ctx->GetModelManager(),
                                    ctx->GetAllObjects(),
                                    ctx->GetSequenceElements(),
                                    opts,
                                    &warnings,
                                    nullptr);

    if (outWarnings) {
        NSMutableArray<NSString*>* arr = [NSMutableArray arrayWithCapacity:warnings.size()];
        for (const auto& w : warnings) {
            [arr addObject:[NSString stringWithUTF8String:w.c_str()]];
        }
        *outWarnings = arr;
    }

    if (!ok) {
        spdlog::warn("XLSequencePackager: Pack returned false for {}", outPath.string());
        [fm removeItemAtPath:stagingDir error:nil];
        if (outError) *outError = MakePackError(10, @"Failed to write sequence package");
        return nil;
    }

    spdlog::info("XLSequencePackager: packaged {} ({} warnings)",
                 outPath.string(), warnings.size());
    return [NSURL fileURLWithPath:[NSString stringWithUTF8String:outPath.string().c_str()]];
}

@end
