#import "XLFindEffectResult.h"

@implementation XLFindEffectResult

- (instancetype)initWithModelName:(NSString*)modelName
                      elementName:(NSString*)elementName
                       effectName:(NSString*)effectName
                       layerIndex:(NSInteger)layerIndex
                      startTimeMS:(NSInteger)startTimeMS
                   matchedSetting:(NSString*)matchedSetting {
    self = [super init];
    if (self) {
        _modelName = [modelName copy];
        _elementName = [elementName copy];
        _effectName = [effectName copy];
        _layerIndex = layerIndex;
        _startTimeMS = startTimeMS;
        _matchedSetting = [matchedSetting copy];
    }
    return self;
}

@end
