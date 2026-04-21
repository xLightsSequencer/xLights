#pragma once

#import <Foundation/Foundation.h>
#import <UIKit/UIKit.h>

NS_ASSUME_NONNULL_BEGIN

// ObjC bridge over src-core/render/ColorCurve. Wraps a C++ ColorCurve
// instance so SwiftUI can drive the palette ColorCurve editor
// (G16 — C5) without re-implementing the serialisation format. The
// string stored at `C_BUTTON_Palette1..8` is byte-for-byte identical
// to what desktop writes.
//
// ColorCurve's "timecurve" mode (TC_TIME / TC_RIGHT / ... / TC_CCW)
// controls how the gradient is evaluated at render time — either
// time-swept across the effect duration (default), spatially across
// the buffer, or radially / rotationally. Per-effect availability
// comes from `RenderableEffect::SupportsLinear/RadialColorCurves`
// (exposed via the separate `supportsColorCurveModes…` bridge call
// on `XLSequenceDocument`).
//
// Threading: instances are main-actor-owned; the underlying ColorCurve
// is not thread-safe.
@interface XLColorCurve : NSObject

// Mode values match the TC_* defines in ColorCurve.h so ints can
// round-trip without an enum translation on the Swift side.
typedef NS_ENUM(NSInteger, XLColorCurveMode) {
    XLColorCurveModeTime       NS_SWIFT_NAME(time)       = 0,  // TC_TIME
    XLColorCurveModeRight      NS_SWIFT_NAME(right)      = 1,  // TC_RIGHT
    XLColorCurveModeDown       NS_SWIFT_NAME(down)       = 2,  // TC_DOWN
    XLColorCurveModeLeft       NS_SWIFT_NAME(left)       = 3,  // TC_LEFT
    XLColorCurveModeUp         NS_SWIFT_NAME(up)         = 4,  // TC_UP
    XLColorCurveModeRadialIn   NS_SWIFT_NAME(radialIn)   = 5,  // TC_RADIALIN
    XLColorCurveModeRadialOut  NS_SWIFT_NAME(radialOut)  = 6,  // TC_RADIALOUT
    XLColorCurveModeCW         NS_SWIFT_NAME(cw)         = 7,  // TC_CW
    XLColorCurveModeCCW        NS_SWIFT_NAME(ccw)        = 8,  // TC_CCW
};

// Construction. Empty `serialised` creates an inactive curve with a
// single midpoint at black — same default as desktop. `identifier`
// is the slot id ("Palette1".."Palette8") that Save-to-file uses.
- (instancetype)initWithSerialised:(NSString*)serialised
                        identifier:(NSString*)identifier NS_DESIGNATED_INITIALIZER;

// Round-trip to the serialised string stored at the palette slot's
// `C_BUTTON_Palette<N>` key. Matches ColorCurve::Serialise().
- (nonnull NSString*)serialise;

// Core state
@property (nonatomic) BOOL active;
@property (nonatomic, copy, nonnull) NSString* identifier;
@property (nonatomic) XLColorCurveMode mode;

// Point editing. Each point is {x: 0..1, color: UIColor}. `x` snaps
// to the 1/100 grid (matches `ccSortableColorPoint::Normalise`) so
// the serialised form matches desktop.
//
// Returned as an NSArray of NSDictionary so Swift sees
// `[[String: Any]]`. Keys: @"x" → NSNumber (double), @"color" →
// UIColor.
- (nonnull NSArray<NSDictionary<NSString*, id>*>*)points;

// Add-or-update-at-x. Matches `ColorCurve::SetValueAt` semantics —
// if a point already exists at the normalised x, its colour is
// replaced. Call-site-friendly naming for Swift.
- (void)setPointAtX:(float)x color:(nonnull UIColor*)color
    NS_SWIFT_NAME(setPoint(x:color:));
- (void)deletePointAtX:(float)x
    NS_SWIFT_NAME(deletePoint(x:));

// Horizontal mirror — swaps every point's x for (1 - x). Matches
// ColorCurve::Flip.
- (void)flip;

// Sample the gradient at `offset` ∈ [0, 1]. Returns a UIColor. Used
// by the preview strip in the editor sheet.
- (nonnull UIColor*)colorAtOffset:(float)offset;

@end

NS_ASSUME_NONNULL_END
