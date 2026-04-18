#pragma once

#import <Foundation/Foundation.h>

// ObjC bridge over src-core/render/ValueCurve. Wraps a C++ ValueCurve
// instance and exposes every field the editor needs to Swift. All
// serialisation routes through ValueCurve::Serialise / ::Deserialise so
// iPad-authored curves round-trip byte-for-byte with desktop.
//
// Threading: instances are main-actor-owned; the underlying ValueCurve is
// not thread-safe and must be mutated from one thread at a time.
@interface XLValueCurve : NSObject

// Construction
- (instancetype)init;
// If `serialised` is empty or nil the curve starts inactive with sane
// defaults (Min=0, Max=100, Type="Flat").
- (instancetype)initWithSerialised:(NSString*)serialised NS_DESIGNATED_INITIALIZER;
- (instancetype)initWithSerialised:(NSString*)serialised
                            forId:(NSString*)identifier
                              min:(double)min
                              max:(double)max
                          divisor:(double)divisor;

// Round-trip to the pipe-delimited string format stored in the effect's
// settings map (key `<prefix>VALUECURVE_<id>`).
- (nonnull NSString*)serialise;

// Core state
@property (nonatomic) BOOL active;
@property (nonatomic, copy, nonnull) NSString* identifier;
@property (nonatomic, copy, nonnull) NSString* type;
@property (nonatomic) double minValue;
@property (nonatomic) double maxValue;
@property (nonatomic) double divisor;

// Parameters — set/get via these as the underlying ValueCurve's
// SetParameterN clamps to the type's range and re-renders custom points.
@property (nonatomic) double parameter1;
@property (nonatomic) double parameter2;
@property (nonatomic) double parameter3;
@property (nonatomic) double parameter4;

// Optional fields (empty string / NO / 0 = unset)
@property (nonatomic, copy, nonnull) NSString* timingTrack;
@property (nonatomic, copy, nonnull) NSString* audioTrack;
@property (nonatomic, copy, nonnull) NSString* filterText;
@property (nonatomic) BOOL filterIsRegex;
@property (nonatomic) int timeOffset;
@property (nonatomic) BOOL wrap;
@property (nonatomic) BOOL realValues;

// Sample the curve at `offset` ∈ [0, 1]. Returns a value in [0, 1] (the
// "raw" 0..1 value, before scaling to min/max). For the preview strip.
- (double)valueAtOffset:(double)offset;

// Custom-point editing (type == "Custom"). Returns a flat
// [x1, y1, x2, y2, ...] array so Swift doesn't have to reach into a
// C++ std::list. Names are NS_SWIFT_NAME'd so the Swift call sites read
// naturally.
- (nonnull NSArray<NSNumber*>*)customPoints;
- (void)setCustomPoints:(nonnull NSArray<NSNumber*>*)points;
- (void)addCustomPointAtX:(double)x y:(double)y
    NS_SWIFT_NAME(addCustomPoint(x:y:));
- (void)deleteCustomPointAtX:(double)x
    NS_SWIFT_NAME(deleteCustomPoint(x:));

// Per-type parameter range lookup. `parm` is 1..4. Returns 0..100 default
// if the type doesn't override the range.
+ (void)rangeForParameter:(int)parm
                     type:(nonnull NSString*)type
                  outLow:(nonnull double*)low
                 outHigh:(nonnull double*)high;

// Full list of curve-type strings exactly as used in the serialised
// `Type=` field. Sourced from ValueCurve::SetType's switch so the order
// matches desktop's picker.
+ (nonnull NSArray<NSString*>*)availableTypes;

// Convenience: does this type write `Values=` (Custom / Random)?
+ (BOOL)typeHasCustomPoints:(nonnull NSString*)type;

@end
