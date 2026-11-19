/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#ifdef __APPLE__

#include "ShowFileWatcher.h"

#import <Foundation/Foundation.h>

#include <memory>

// Pokes the watcher when a coordinated writer (iCloud Drive, the Files app,
// another app) changes something in the show folder. Writers that do not
// coordinate (most other sync clients, network shares) are found by polling.
@interface XLShowFolderPresenter : NSObject <NSFilePresenter>
- (instancetype)initWithFolder:(NSURL*)folder poked:(std::shared_ptr<std::atomic<bool>>)poked;
@end

@implementation XLShowFolderPresenter {
    NSURL* _folder;
    NSOperationQueue* _queue;
    std::shared_ptr<std::atomic<bool>> _poked;
}

- (instancetype)initWithFolder:(NSURL*)folder poked:(std::shared_ptr<std::atomic<bool>>)poked {
    if ((self = [super init])) {
        _folder = folder;
        _poked = std::move(poked);
        _queue = [[NSOperationQueue alloc] init];
        _queue.maxConcurrentOperationCount = 1;
        _queue.name = @"xLights show folder presenter";
    }
    return self;
}

- (NSURL*)presentedItemURL {
    return _folder;
}

- (NSOperationQueue*)presentedItemOperationQueue {
    return _queue;
}

- (void)poke {
    _poked->store(true);
}

- (void)presentedItemDidChange {
    [self poke];
}

// Renders write sequences and fseq files into the same folder all the time;
// only the shared show files are worth a read.
- (void)pokeFor:(NSURL*)url {
    if ([url.lastPathComponent.lowercaseString hasPrefix:@"xlights_"]) {
        [self poke];
    }
}

- (void)presentedSubitemDidChangeAtURL:(NSURL*)url {
    [self pokeFor:url];
}

- (void)presentedSubitemDidAppearAtURL:(NSURL*)url {
    [self pokeFor:url];
}

- (void)presentedSubitemAtURL:(NSURL*)oldURL didMoveToURL:(NSURL*)newURL {
    [self pokeFor:newURL];
}

@end

namespace {

NSURL* URLOf(const std::string& path) {
    NSString* s = [NSString stringWithUTF8String:path.c_str()];
    return s ? [NSURL fileURLWithPath:s] : nil;
}

// An iCloud Drive item that is not downloaded (or not the current version)
// yet: ask for it and report it as not readable, rather than blocking in a
// coordinated read until it arrives.
bool WaitingForDownload(NSURL* url) {
    NSNumber* ubiquitous = nil;
    if (![url getResourceValue:&ubiquitous forKey:NSURLIsUbiquitousItemKey error:nil] || !ubiquitous.boolValue) {
        return false;
    }
    NSString* status = nil;
    [url getResourceValue:&status forKey:NSURLUbiquitousItemDownloadingStatusKey error:nil];
    if (status == nil || [status isEqualToString:NSURLUbiquitousItemDownloadingStatusCurrent]) {
        return false;
    }
    [[NSFileManager defaultManager] startDownloadingUbiquitousItemAtURL:url error:nil];
    return true;
}

template <typename T, typename F>
std::optional<T> CoordinatedReadOf(const std::string& path, F&& read) {
    @autoreleasepool {
        NSURL* url = URLOf(path);
        if (url == nil) {
            return std::nullopt;
        }
        if (WaitingForDownload(url)) {
            return std::nullopt;
        }
        __block std::optional<T> out;
        NSError* error = nil;
        NSFileCoordinator* coordinator = [[NSFileCoordinator alloc] initWithFilePresenter:nil];
        [coordinator coordinateReadingItemAtURL:url
                                        options:NSFileCoordinatorReadingWithoutChanges
                                          error:&error
                                     byAccessor:^(NSURL* newURL) {
                                         out = read(std::string(newURL.path.UTF8String));
                                     }];
        if (error != nil) {
            // Coordination unavailable: read it directly.
            return read(path);
        }
        return out;
    }
}

} // namespace

void ShowFileWatcher::StartPlatform(const std::string& folder) {
    StopPlatform();
    NSURL* url = URLOf(folder);
    if (url == nil) {
        return;
    }
    XLShowFolderPresenter* presenter = [[XLShowFolderPresenter alloc] initWithFolder:url poked:_poked];
    [NSFileCoordinator addFilePresenter:presenter];
    _platform = (__bridge_retained void*)presenter;
}

void ShowFileWatcher::StopPlatform() {
    if (_platform == nullptr) {
        return;
    }
    XLShowFolderPresenter* presenter = (__bridge_transfer XLShowFolderPresenter*)_platform;
    _platform = nullptr;
    [NSFileCoordinator removeFilePresenter:presenter];
}

std::optional<ShowFileFingerprint> ShowFileWatcher::CoordinatedFingerprint(const std::string& path) {
    return CoordinatedReadOf<ShowFileFingerprint>(path, [](const std::string& p) { return ShowFileGuard::FingerprintFile(p); });
}

std::optional<std::pair<std::string, ShowFileFingerprint>> ShowFileWatcher::CoordinatedRead(const std::string& path) {
    using Read = std::pair<std::string, ShowFileFingerprint>;
    return CoordinatedReadOf<Read>(path, [](const std::string& p) { return ShowFileGuard::ReadFile(p); });
}

#endif
