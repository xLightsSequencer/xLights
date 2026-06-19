#pragma once

/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#import <Foundation/Foundation.h>

NS_ASSUME_NONNULL_BEGIN

/// One parsed line from an `.lrc` body — `timeMS` is the absolute
/// position in milliseconds, `text` is the line content (may be empty
/// when the LRC source had a bare timestamp).
@interface XLLrcLine : NSObject
@property (nonatomic, assign, readonly) int timeMS;
@property (nonatomic, copy, readonly) NSString* text;
@end

/// Bridge for B91 LRCLIB synced-lyrics import. The HTTP search itself
/// runs Swift-side (URLSession) — this bridge exposes only the wx-free
/// core helpers (`lrc::ParseLRC`, `lrc::SanitizePhraseText`) so the
/// iPad client and the desktop dialog go through the same parser.
@interface XLLyricsImport : NSObject

/// Parse a `.lrc`-format synced-lyrics body into ordered
/// `(timeMS, text)` lines.
+ (NSArray<XLLrcLine*>*)parseLRC:(NSString*)syncedLyrics
    NS_SWIFT_NAME(parseLRC(_:));

/// Apply the desktop's pre-write text cleanup: replaces common Unicode
/// quotes with ASCII, strips `"`, `<`, `>`, runs
/// `PhonemeDictionary::InsertSpacesAfterPunctuation`. Returns nil if
/// the cleaned result is empty / whitespace-only — caller should drop
/// that line.
+ (nullable NSString*)sanitizePhraseText:(NSString*)text
    NS_SWIFT_NAME(sanitizePhraseText(_:));

@end

NS_ASSUME_NONNULL_END
