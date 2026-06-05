
/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#ifdef __WXMAC__

#import <Cocoa/Cocoa.h>
#include "CommandPaletteHelper.h"

#include <cmath>
#include <cstdlib>
#include <map>
#include <string>

wxBitmap GetSFSymbolBitmap(const std::string& symbolName, int pointSize, const wxColour& color)
{
    @autoreleasepool {
        NSString* name = [NSString stringWithUTF8String:symbolName.c_str()];
        NSImage* image = [NSImage imageWithSystemSymbolName:name accessibilityDescription:nil];
        if (!image) return wxNullBitmap;

        NSImageSymbolConfiguration* sizeConfig =
            [NSImageSymbolConfiguration configurationWithPointSize:pointSize
                                                           weight:NSFontWeightRegular];

        NSColor* nsColor = [NSColor colorWithSRGBRed:color.Red() / 255.0
                                               green:color.Green() / 255.0
                                                blue:color.Blue() / 255.0
                                               alpha:1.0];

        if (@available(macOS 12.0, *)) {
            NSImageSymbolConfiguration* colorConfig =
                [NSImageSymbolConfiguration configurationWithHierarchicalColor:nsColor];
            sizeConfig = [sizeConfig configurationByApplyingConfiguration:colorConfig];
        }

        image = [image imageWithSymbolConfiguration:sizeConfig];
        if (!image) return wxNullBitmap;

        // For macOS 11 (no hierarchical color), tint the template image manually
        if (@available(macOS 12.0, *)) {
            // Already tinted via config
        } else {
            NSSize sz = image.size;
            NSImage* tinted = [[NSImage alloc] initWithSize:sz];
            [tinted lockFocus];
            [image drawInRect:NSMakeRect(0, 0, sz.width, sz.height)
                     fromRect:NSZeroRect
                    operation:NSCompositingOperationSourceOver
                     fraction:1.0];
            [nsColor set];
            NSRectFillUsingOperation(NSMakeRect(0, 0, sz.width, sz.height),
                                     NSCompositingOperationSourceAtop);
            [tinted unlockFocus];
            image = tinted;
        }

        return wxBitmap(image);
    }
}

// Matches the mapping from feature/native-macos-rebuild
std::string GetSFSymbolForEffect(const std::string& effectName)
{
    static const std::map<std::string, std::string> mapping = {
        {"Off",          "power.circle"},
        {"On",           "lightbulb.fill"},
        {"Adjust",       "slider.horizontal.3"},
        {"Arpeggio",     "music.note.list"},
        {"Bars",         "chart.bar.fill"},
        {"Butterfly",    "bird.fill"},
        {"Candle",       "flame"},
        {"Circles",      "circle.grid.3x3.fill"},
        {"Color Wash",   "paintbrush.fill"},
        {"Curtain",      "rectangle.split.2x1.fill"},
        {"DMX",          "slider.vertical.3"},
        {"Duplicate",    "plus.square.on.square"},
        {"Faces",        "face.smiling.fill"},
        {"Fan",          "fan.fill"},
        {"Fill",         "square.fill"},
        {"Fire",         "flame.fill"},
        {"Fireworks",    "sparkles"},
        {"Galaxy",       "staroflife.fill"},
        {"Garlands",     "leaf.fill"},
        {"Glediator",    "square.grid.3x3.fill"},
        {"Guitar",       "guitars.fill"},
        {"Kaleidoscope", "camera.filters"},
        {"Life",         "heart.fill"},
        {"Lightning",    "bolt.fill"},
        {"Lines",        "line.3.horizontal"},
        {"Liquid",       "drop.fill"},
        {"Marquee",      "text.badge.star"},
        {"Meteors",      "moonphase.waning.crescent"},
        {"Morph",        "arrow.triangle.2.circlepath"},
        {"Moving Head",  "light.beacon.max.fill"},
        {"Music Effect", "music.note"},
        {"Piano",        "pianokeys"},
        {"Pictures",     "photo.fill"},
        {"Pinwheel",     "rotate.3d"},
        {"Plasma",       "waveform"},
        {"Ripple",       "drop.circle.fill"},
        {"Servo",        "gearshape.2.fill"},
        {"Shader",       "paintpalette.fill"},
        {"Shape",        "star.fill"},
        {"Shimmer",      "sparkle"},
        {"Shockwave",    "waveform.circle.fill"},
        {"SingleStrand", "line.diagonal"},
        {"Sketch",       "pencil.tip"},
        {"Snowflakes",   "snowflake"},
        {"Snowstorm",    "cloud.snow.fill"},
        {"Spirals",      "tornado"},
        {"Spirograph",   "circle.circle"},
        {"State",        "switch.2"},
        {"Strobe",       "light.max"},
        {"Tendril",      "leaf.arrow.circlepath"},
        {"Text",         "textformat"},
        {"Tree",         "tree.fill"},
        {"Twinkle",      "sparkles"},
        {"Video",        "video.fill"},
        {"VU Meter",     "chart.bar.fill"},
        {"Warp",         "arrow.up.and.down.and.arrow.left.and.right"},
        {"Wave",         "water.waves"},
    };

    auto it = mapping.find(effectName);
    if (it != mapping.end()) return it->second;
    return "questionmark.square.fill";
}

// Hash-based color generation matching the native rebuild approach
wxColour GetColorForEffect(const std::string& effectName)
{
    unsigned int hash = 0;
    for (char c : effectName) {
        hash = hash * 31u + static_cast<unsigned char>(c);
    }

    double hue = (hash % 360) / 360.0;
    double sat = 0.7;
    double bri = 0.8;

    // HSV to RGB
    int hi = static_cast<int>(hue * 6.0) % 6;
    double f = hue * 6.0 - hi;
    double p = bri * (1.0 - sat);
    double q = bri * (1.0 - f * sat);
    double t = bri * (1.0 - (1.0 - f) * sat);

    double r, g, b;
    switch (hi) {
        case 0: r = bri; g = t;   b = p;   break;
        case 1: r = q;   g = bri; b = p;   break;
        case 2: r = p;   g = bri; b = t;   break;
        case 3: r = p;   g = q;   b = bri; break;
        case 4: r = t;   g = p;   b = bri; break;
        default:r = bri; g = p;   b = q;   break;
    }

    return wxColour(static_cast<unsigned char>(r * 255),
                    static_cast<unsigned char>(g * 255),
                    static_cast<unsigned char>(b * 255));
}

#endif // __WXMAC__
