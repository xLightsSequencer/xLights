#import "XLColorCurve.h"

#include "render/ColorCurve.h"
#include "utils/Color.h"

#include <memory>
#include <string>

namespace {

// ColorCurve stores colours as xlColor ("#RRGGBB" round-trippable).
// Convert to / from UIColor (sRGB, no alpha — palette curves are
// fully opaque by design).
UIColor* uiColorFromXl(const xlColor& c) {
    return [UIColor colorWithRed:(CGFloat)c.Red() / 255.0
                           green:(CGFloat)c.Green() / 255.0
                            blue:(CGFloat)c.Blue() / 255.0
                           alpha:1.0];
}

xlColor xlColorFromUI(UIColor* u) {
    // Always convert via sRGB before reading components. UIColor's
    // -getRed:green:blue:alpha: reports values in the source color
    // space; on Display-P3 iPads the Color the SwiftUI ColorPicker
    // hands back lives in extended-sRGB, and getRed dumps extended-
    // range values directly. That mangles the exact #RRGGBB hex the
    // user typed (e.g. #FF0000 round-trips as a slightly different
    // red). CGColorCreateCopyByMatchingToColorSpace clamps + maps
    // to bona fide sRGB before we read the bytes.
    CGFloat r = 0, g = 0, b = 0;
    CGColorSpaceRef srgb = CGColorSpaceCreateWithName(kCGColorSpaceSRGB);
    CGColorRef cg = CGColorCreateCopyByMatchingToColorSpace(
        srgb, kCGRenderingIntentDefault, u.CGColor, nullptr);
    CGColorSpaceRelease(srgb);
    if (cg) {
        const CGFloat* comps = CGColorGetComponents(cg);
        if (comps) {
            r = comps[0]; g = comps[1]; b = comps[2];
        }
        CGColorRelease(cg);
    } else {
        // Conversion failed (unusual) — fall back to a device-space
        // read so we still return *something* rather than black.
        CGFloat a = 0;
        [u getRed:&r green:&g blue:&b alpha:&a];
    }
    auto clamp = [](CGFloat v) -> uint8_t {
        if (v < 0) v = 0;
        if (v > 1) v = 1;
        return (uint8_t)std::lround(v * 255.0);
    };
    return xlColor(clamp(r), clamp(g), clamp(b));
}

} // namespace

@implementation XLColorCurve {
    std::unique_ptr<ColorCurve> _cc;
}

- (instancetype)initWithSerialised:(NSString*)serialised
                        identifier:(NSString*)identifier {
    if ((self = [super init])) {
        _cc = std::make_unique<ColorCurve>();
        if (serialised.length > 0) {
            _cc->Deserialise(std::string([serialised UTF8String]));
        }
        // Desktop sets Id from the palette slot ("Palette1".."Palette8")
        // so the "Save as preset" path has something to name the file.
        if (identifier.length > 0) {
            _cc->SetId(std::string([identifier UTF8String]));
        }
    }
    return self;
}

- (NSString*)serialise {
    std::string s = _cc->Serialise();
    return [NSString stringWithUTF8String:s.c_str()];
}

// MARK: - Core state

- (BOOL)active { return _cc->IsActive() ? YES : NO; }
- (void)setActive:(BOOL)active { _cc->SetActive(active == YES); }

- (NSString*)identifier {
    return [NSString stringWithUTF8String:_cc->GetId().c_str()];
}
- (void)setIdentifier:(NSString*)identifier {
    _cc->SetId(identifier ? std::string([identifier UTF8String]) : std::string());
}

- (XLColorCurveMode)mode {
    return (XLColorCurveMode)_cc->GetTimeCurve();
}
- (void)setMode:(XLColorCurveMode)mode {
    // `NextTimeCurve` / `SetValidTimeCurve` on ColorCurve take
    // support flags; we don't have them here. The editor is
    // responsible for only offering valid modes, so we set the raw
    // value directly via a minimal re-serialise through the public
    // surface.
    // ColorCurve has no direct setter — write via Serialise /
    // Deserialise round trip: replace the Timecurve= token.
    std::string s = _cc->Serialise();
    std::string marker = "Timecurve=";
    auto pos = s.find(marker);
    if (pos != std::string::npos) {
        auto end = s.find('|', pos);
        s.replace(pos, (end == std::string::npos ? s.size() : end) - pos,
                  marker + std::to_string((int)mode));
    } else if (_cc->IsActive() && (int)mode != 0 /* TC_TIME is default */) {
        // Insert after the Id= token so ordering matches what
        // ColorCurve::Serialise produces on the next mutation.
        std::string idMarker = "Id=";
        auto idPos = s.find(idMarker);
        if (idPos != std::string::npos) {
            auto idEnd = s.find('|', idPos);
            if (idEnd != std::string::npos) {
                s.insert(idEnd + 1, marker + std::to_string((int)mode) + "|");
            }
        }
    }
    _cc->Deserialise(s);
}

// MARK: - Points

- (NSArray<NSDictionary<NSString*, id>*>*)points {
    NSMutableArray* out = [NSMutableArray array];
    for (const auto& p : _cc->GetPoints()) {
        [out addObject:@{
            @"x": @((double)p.x),
            @"color": uiColorFromXl(p.color),
        }];
    }
    return out;
}

- (void)setPointAtX:(float)x color:(UIColor*)color {
    _cc->SetValueAt(x, xlColorFromUI(color));
}

- (void)deletePointAtX:(float)x {
    _cc->DeletePoint(x);
}

- (void)flip {
    _cc->Flip();
}

// MARK: - Sampling

- (UIColor*)colorAtOffset:(float)offset {
    return uiColorFromXl(_cc->GetValueAt(offset));
}

@end
