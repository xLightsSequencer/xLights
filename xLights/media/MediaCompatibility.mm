/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

// Apple-only implementations of MediaCompatibility check methods.
// The cross-platform code (CheckSequenceMedia, non-Apple stubs) is in MediaCompatibility.cpp.

#ifdef __APPLE__

#include "MediaCompatibility.h"

#import <AVFoundation/AVFoundation.h>
#import <AudioToolbox/AudioToolbox.h>

std::string MediaCompatibility::CheckVideoFile(const std::string& filePath) {
    if (filePath.empty()) return "";

    @autoreleasepool {
        NSURL* url = [NSURL fileURLWithPath:[NSString stringWithUTF8String:filePath.c_str()]];
        AVURLAsset* asset = [AVURLAsset URLAssetWithURL:url options:nil];
        if (!asset) {
            return "Cannot open file";
        }

        // Check if the asset is playable (decodable by AVFoundation)
        if (!asset.playable) {
            return "File format not supported by AVFoundation";
        }

        // Check for video tracks
        NSArray<AVAssetTrack*>* videoTracks = [asset tracksWithMediaType:AVMediaTypeVideo];
        if (videoTracks.count == 0) {
            return "No video tracks found";
        }

        // Try to create an AVAssetReader to verify we can actually decode it
        NSError* error = nil;
        AVAssetReader* reader = [[AVAssetReader alloc] initWithAsset:asset error:&error];
        if (!reader || error) {
            return std::string("Cannot create decoder: ") +
                   (error ? [[error localizedDescription] UTF8String] : "unknown error");
        }

        NSDictionary* outputSettings = @{
            (NSString*)kCVPixelBufferPixelFormatTypeKey: @(kCVPixelFormatType_32BGRA)
        };
        AVAssetReaderTrackOutput* output =
            [[AVAssetReaderTrackOutput alloc] initWithTrack:videoTracks[0]
                                            outputSettings:outputSettings];

        if (![reader canAddOutput:output]) {
            return "Video codec not supported for decoding by AVFoundation";
        }

        [reader addOutput:output];

        if (![reader startReading]) {
            return std::string("Cannot start decoding: ") +
                   (reader.error ? [[reader.error localizedDescription] UTF8String] : "unknown");
        }

        // Try to read one frame to confirm decode actually works
        @try {
            CMSampleBufferRef sample = [output copyNextSampleBuffer];
            if (sample) {
                CFRelease(sample);
            } else {
                if (reader.status == AVAssetReaderStatusFailed) {
                    return std::string("Decode failed: ") +
                           (reader.error ? [[reader.error localizedDescription] UTF8String] : "unknown");
                }
            }
        } @catch (NSException* e) {
            return std::string("Decode error: ") + [[e reason] UTF8String];
        }

        [reader cancelReading];
    }
    return ""; // compatible
}

std::string MediaCompatibility::CheckAudioFile(const std::string& filePath) {
    if (filePath.empty()) return "";

    @autoreleasepool {
        NSURL* url = [NSURL fileURLWithPath:[NSString stringWithUTF8String:filePath.c_str()]];
        CFURLRef cfUrl = (__bridge CFURLRef)url;

        ExtAudioFileRef audioFile = nullptr;
        OSStatus status = ExtAudioFileOpenURL(cfUrl, &audioFile);
        if (status != noErr || !audioFile) {
            switch (status) {
                case kAudioFileUnsupportedFileTypeError:
                    return "Audio file format not supported by AudioToolbox";
                case kAudioFileUnsupportedDataFormatError:
                    return "Audio codec not supported by AudioToolbox";
                case kAudioFileFileNotFoundError:
                    return "Audio file not found";
                case kAudioFilePermissionsError:
                    return "Permission denied reading audio file";
                default:
                    return "Cannot open audio file (error " + std::to_string((int)status) + ")";
            }
        }

        // Verify we can set up a client format (PCM output) — this confirms
        // the codec can actually be decoded, not just that the container is recognized
        AudioStreamBasicDescription clientFormat = {};
        clientFormat.mSampleRate = 44100;
        clientFormat.mFormatID = kAudioFormatLinearPCM;
        clientFormat.mFormatFlags = kAudioFormatFlagIsSignedInteger | kAudioFormatFlagIsPacked;
        clientFormat.mBitsPerChannel = 16;
        clientFormat.mChannelsPerFrame = 2;
        clientFormat.mFramesPerPacket = 1;
        clientFormat.mBytesPerFrame = 4;
        clientFormat.mBytesPerPacket = 4;

        status = ExtAudioFileSetProperty(audioFile,
                                          kExtAudioFileProperty_ClientDataFormat,
                                          sizeof(clientFormat),
                                          &clientFormat);
        ExtAudioFileDispose(audioFile);

        if (status != noErr) {
            return "Audio codec cannot be decoded to PCM by AudioToolbox (error " +
                   std::to_string((int)status) + ")";
        }
    }
    return ""; // compatible
}

#endif // __APPLE__
