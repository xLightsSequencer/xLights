#import "XLValueCurve.h"

#include "render/ValueCurve.h"

#include <memory>
#include <string>

@implementation XLValueCurve {
    std::unique_ptr<ValueCurve> _vc;
}

- (instancetype)init {
    return [self initWithSerialised:@""];
}

- (instancetype)initWithSerialised:(NSString*)serialised {
    if ((self = [super init])) {
        _vc = std::make_unique<ValueCurve>();
        if (serialised.length > 0) {
            _vc->Deserialise(std::string([serialised UTF8String]), /*holdminmax*/ false);
        }
    }
    return self;
}

- (instancetype)initWithSerialised:(NSString*)serialised
                            forId:(NSString*)identifier
                              min:(double)min
                              max:(double)max
                          divisor:(double)divisor {
    // Chain through the designated initWithSerialised: then replace the
    // underlying VC with one constructed for the property's min/max/divisor.
    // Mirror JsonEffectPanel's flow: seed with the property scale, then
    // deserialise any stored string (holdminmax=false so ConvertChangedScale
    // fires if the stored string's min/max differs).
    if ((self = [self initWithSerialised:nil])) {
        _vc = std::make_unique<ValueCurve>(std::string([identifier UTF8String]),
                                           (float)min, (float)max,
                                           "Flat", 0.0f, 0.0f, 0.0f, 0.0f,
                                           /*wrap*/ false,
                                           (float)divisor);
        if (serialised.length > 0) {
            _vc->Deserialise(std::string([serialised UTF8String]), /*holdminmax*/ false);
        }
    }
    return self;
}

- (NSString*)serialise {
    std::string s = _vc->Serialise();
    return [NSString stringWithUTF8String:s.c_str()];
}

// MARK: - Core state

- (BOOL)active { return _vc->IsActive() ? YES : NO; }
- (void)setActive:(BOOL)active { _vc->SetActive(active == YES); }

- (NSString*)identifier {
    return [NSString stringWithUTF8String:_vc->GetId().c_str()];
}
- (void)setIdentifier:(NSString*)identifier {
    _vc->SetId(std::string([identifier UTF8String]));
}

- (NSString*)type {
    return [NSString stringWithUTF8String:_vc->GetType().c_str()];
}
- (void)setType:(NSString*)type {
    _vc->SetType(std::string([type UTF8String]));
}

- (double)minValue { return (double)_vc->GetMin(); }
- (double)maxValue { return (double)_vc->GetMax(); }
- (void)setMinValue:(double)v { _vc->SetLimits((float)v, _vc->GetMax()); }
- (void)setMaxValue:(double)v { _vc->SetLimits(_vc->GetMin(), (float)v); }

- (double)divisor { return (double)_vc->GetDivisor(); }
- (void)setDivisor:(double)d { _vc->SetDivisor((float)d); }

// MARK: - Parameters

- (double)parameter1 { return (double)_vc->GetParameter1(); }
- (double)parameter2 { return (double)_vc->GetParameter2(); }
- (double)parameter3 { return (double)_vc->GetParameter3(); }
- (double)parameter4 { return (double)_vc->GetParameter4(); }
- (void)setParameter1:(double)v { _vc->SetParameter1((float)v); }
- (void)setParameter2:(double)v { _vc->SetParameter2((float)v); }
- (void)setParameter3:(double)v { _vc->SetParameter3((float)v); }
- (void)setParameter4:(double)v { _vc->SetParameter4((float)v); }

// MARK: - Optional fields

- (NSString*)timingTrack {
    return [NSString stringWithUTF8String:_vc->GetTimingTrack().c_str()];
}
- (void)setTimingTrack:(NSString*)t {
    _vc->SetTimingTrack(std::string([(t ?: @"") UTF8String]));
}

- (NSString*)audioTrack {
    return [NSString stringWithUTF8String:_vc->GetAudioTrack().c_str()];
}
- (void)setAudioTrack:(NSString*)t {
    _vc->SetAudioTrack(std::string([(t ?: @"") UTF8String]));
}

- (NSString*)filterText {
    return [NSString stringWithUTF8String:_vc->GetFilterLabelText().c_str()];
}
- (void)setFilterText:(NSString*)t {
    _vc->SetFilterLabelText(std::string([(t ?: @"") UTF8String]));
}

- (BOOL)filterIsRegex { return _vc->IsFilterLabelRegex() ? YES : NO; }
- (void)setFilterIsRegex:(BOOL)r { _vc->SetFilterLabelRegex(r == YES); }

- (int)timeOffset { return _vc->GetTimeOffset(); }
- (void)setTimeOffset:(int)o { _vc->SetTimeOffset(o); }

- (BOOL)wrap { return _vc->GetWrap() ? YES : NO; }
- (void)setWrap:(BOOL)w { _vc->SetWrap(w == YES); }

- (BOOL)realValues { return _vc->IsRealValue() ? YES : NO; }
- (void)setRealValues:(BOOL)r {
    // ValueCurve only exposes SetRealValue(=true); to clear we'd need a
    // back-door, but since it's only cleared on construction / deserialise
    // and the editor flow always round-trips through a string, we can
    // re-serialise-with-Active=... if that turns out to be needed. For
    // now: no-op when r==NO; flip it on when r==YES.
    if (r == YES) _vc->SetRealValue();
}

// MARK: - Evaluation

- (double)valueAtOffset:(double)offset {
    // startMS/endMS are only consulted by the Music/Timing-track types;
    // pass (0, 1000) as neutral values — the raw 0..1 fraction is what
    // the preview strip cares about.
    return (double)_vc->GetValueAt((float)offset, 0, 1000);
}

// MARK: - Custom points

- (NSArray<NSNumber*>*)customPoints {
    auto list = _vc->GetPoints();
    NSMutableArray<NSNumber*>* out = [NSMutableArray arrayWithCapacity:list.size() * 2];
    for (const auto& p : list) {
        [out addObject:@((double)p.x)];
        [out addObject:@((double)p.y)];
    }
    return out;
}

- (void)setCustomPoints:(NSArray<NSNumber*>*)points {
    // Drop every existing point then re-insert. Desktop does this by
    // overwriting via SetValueAt for each known x; simpler here to walk
    // both lists.
    auto current = _vc->GetPoints();
    for (const auto& p : current) {
        _vc->DeletePoint(p.x);
    }
    for (NSUInteger i = 0; i + 1 < points.count; i += 2) {
        double x = points[i].doubleValue;
        double y = points[i + 1].doubleValue;
        _vc->SetValueAt((float)x, (float)y, /*force*/ true);
    }
}

- (void)addCustomPointAtX:(double)x y:(double)y {
    _vc->SetValueAt((float)x, (float)y, /*force*/ true);
}

- (void)deleteCustomPointAtX:(double)x {
    _vc->DeletePoint((float)x);
}

// MARK: - Class methods

+ (void)rangeForParameter:(int)parm
                     type:(NSString*)type
                  outLow:(double*)low
                 outHigh:(double*)high {
    float lo = 0, hi = 100;
    ValueCurve::GetRangeParm(parm, std::string([type UTF8String]), lo, hi);
    if (low) *low = (double)lo;
    if (high) *high = (double)hi;
}

+ (NSArray<NSString*>*)availableTypes {
    // Exact strings accepted by ValueCurve::SetType / emitted by
    // Serialise. Order matches the desktop ValueCurveDialog choice.
    return @[
        @"Flat",
        @"Ramp",
        @"Ramp Up/Down",
        @"Ramp Up/Down Hold",
        @"Saw Tooth",
        @"Sine",
        @"Abs Sine",
        @"Decaying Sine",
        @"Square",
        @"Parabolic Up",
        @"Parabolic Down",
        @"Logarithmic Up",
        @"Logarithmic Down",
        @"Exponential Up",
        @"Exponential Down",
        @"Music",
        @"Inverted Music",
        @"Music Trigger Fade",
        @"Timing Track Toggle",
        @"Timing Track Fade Fixed",
        @"Timing Track Fade Proportional",
        @"Random",
        @"Custom"
    ];
}

+ (BOOL)typeHasCustomPoints:(NSString*)type {
    return [type isEqualToString:@"Custom"] || [type isEqualToString:@"Random"];
}

@end
