/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#import "XLLyricsImport.h"

#include "lyrics/LRCParser.h"

#include <string>

@implementation XLLrcLine {
@public
    int _timeMS;
    NSString* _text;
}
- (instancetype)initWithTimeMS:(int)timeMS text:(NSString*)text {
    self = [super init];
    if (self) {
        _timeMS = timeMS;
        _text = [text copy];
    }
    return self;
}
- (void)dealloc {
    [_text release];
    [super dealloc];
}
- (int)timeMS { return _timeMS; }
- (NSString*)text { return _text; }
@end

@implementation XLLyricsImport

+ (NSArray<XLLrcLine*>*)parseLRC:(NSString*)syncedLyrics {
    if (syncedLyrics == nil || syncedLyrics.length == 0) return @[];
    auto pairs = lrc::ParseLRC(std::string(syncedLyrics.UTF8String));
    NSMutableArray<XLLrcLine*>* out = [NSMutableArray arrayWithCapacity:pairs.size()];
    for (const auto& p : pairs) {
        NSString* t = [NSString stringWithUTF8String:p.second.c_str()];
        XLLrcLine* line = [[XLLrcLine alloc] initWithTimeMS:p.first text:t ?: @""];
        [out addObject:line];
        [line release];
    }
    return [[out copy] autorelease];
}

+ (NSString*)sanitizePhraseText:(NSString*)text {
    if (text == nil || text.length == 0) return nil;
    std::string s(text.UTF8String);
    if (!lrc::SanitizePhraseText(s)) return nil;
    return [NSString stringWithUTF8String:s.c_str()];
}

@end
