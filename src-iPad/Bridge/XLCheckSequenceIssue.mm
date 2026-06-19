#import "XLCheckSequenceIssue.h"

@implementation XLCheckSequenceIssue

- (instancetype)initWithSeverity:(XLCheckSequenceSeverity)severity
                         sectionID:(NSString*)sectionID
                      sectionTitle:(NSString*)sectionTitle
                          category:(NSString*)category
                           message:(NSString*)message
                         modelName:(NSString*)modelName
                        effectName:(NSString*)effectName
                       startTimeMS:(NSInteger)startTimeMS
                        layerIndex:(NSInteger)layerIndex {
    self = [super init];
    if (self) {
        _severity = severity;
        _sectionID = [sectionID copy];
        _sectionTitle = [sectionTitle copy];
        _category = [category copy];
        _message = [message copy];
        _modelName = [modelName copy];
        _effectName = [effectName copy];
        _startTimeMS = startTimeMS;
        _layerIndex = layerIndex;
    }
    return self;
}

@end
