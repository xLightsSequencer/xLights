/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#import "XLSuperStarImport.h"
#import "XLSequenceDocument.h"

#include "iPadRenderContext.h"
#include "import_export/SuperStarImporter.h"
#include "models/Model.h"
#include "models/ModelManager.h"
#include "render/Element.h"
#include "render/SequenceElements.h"

#include <pugixml.hpp>
#include <spdlog/spdlog.h>

#include <filesystem>
#include <fstream>
#include <string>
#include <vector>

NSString* const XLSuperStarLayerBlendNormal     = @"Normal";
NSString* const XLSuperStarLayerBlendAverage    = @"Average";
NSString* const XLSuperStarLayerBlend2Reveals1  = @"2 reveals 1";

// XLSequenceDocument exposes the iPadRenderContext via its
// -(void*)renderContext accessor (see XLImportSession.mm:34 for the
// matching helper). The cast is safe because the document owns the
// render context as a unique_ptr<iPadRenderContext>.
static iPadRenderContext* RawRenderContext(XLSequenceDocument* doc) {
    return doc != nil ? (iPadRenderContext*)[doc renderContext] : nullptr;
}

static NSError* MakeError(NSInteger code, NSString* msg) {
    return [NSError errorWithDomain:@"XLSuperStarImport"
                               code:code
                           userInfo:@{ NSLocalizedDescriptionKey: msg }];
}

@implementation XLSuperStarImportOptions
- (instancetype)init {
    self = [super init];
    if (self) {
        _xSize = 12;
        _ySize = 50;
        _xOffset = 0;
        _yOffset = 0;
        _imageResize = XLSuperStarImageResizeNone;
        _layerBlend = [XLSuperStarLayerBlend2Reveals1 copy];
        _timingOffsetMs = 0;
        _imageGroupPrefix = @"";
    }
    return self;
}
- (void)dealloc {
    [_layerBlend release];
    [_imageGroupPrefix release];
    [super dealloc];
}
@end

@implementation XLSuperStarImport

+ (NSArray<NSString*>*)availableTargetModelNamesForDocument:(XLSequenceDocument*)document {
    iPadRenderContext* rc = RawRenderContext(document);
    if (rc == nullptr) return @[];

    NSMutableArray<NSString*>* names = [NSMutableArray array];
    SequenceElements& se = rc->GetSequenceElements();
    for (size_t i = 0; i < se.GetElementCount(); i++) {
        Element* e = se.GetElement(i);
        if (e->GetType() != ElementType::ELEMENT_TYPE_MODEL) continue;
        [names addObject:[NSString stringWithUTF8String:e->GetName().c_str()]];
        ModelElement* me = dynamic_cast<ModelElement*>(e);
        if (me == nullptr) continue;
        for (int x = 0; x < me->GetSubModelAndStrandCount(); x++) {
            SubModelElement* sub = me->GetSubModel(x);
            if (sub == nullptr) continue;
            const std::string& name = sub->GetName();
            if (name.empty()) continue;
            [names addObject:[NSString stringWithUTF8String:sub->GetFullName().c_str()]];
        }
    }
    return [[names copy] autorelease];
}

+ (BOOL)applyImportFromPath:(NSString*)path
            targetModelName:(NSString*)modelName
                    options:(XLSuperStarImportOptions*)options
                   document:(XLSequenceDocument*)document
                      error:(NSError**)outError {
    if (path.length == 0 || modelName.length == 0 || options == nil || document == nil) {
        if (outError) *outError = MakeError(1, @"Invalid arguments to applyImportFromPath:");
        return NO;
    }

    iPadRenderContext* rc = RawRenderContext(document);
    if (rc == nullptr) {
        if (outError) *outError = MakeError(2, @"No active sequence loaded.");
        return NO;
    }

    // Resolve the target full-name to an Element* (top-level model OR
    // submodel/strand). Mirrors xLightsFrame::ImportSuperStar's lookup.
    SequenceElements& se = rc->GetSequenceElements();
    std::string targetName = modelName.UTF8String;
    Element* target = nullptr;
    for (size_t i = 0; i < se.GetElementCount(); i++) {
        Element* candidate = se.GetElement(i);
        if (candidate->GetType() != ElementType::ELEMENT_TYPE_MODEL) continue;
        if (candidate->GetName() == targetName) {
            target = candidate;
            break;
        }
        ModelElement* me = dynamic_cast<ModelElement*>(candidate);
        if (me == nullptr) continue;
        for (int x = 0; x < me->GetSubModelAndStrandCount(); x++) {
            SubModelElement* sub = me->GetSubModel(x);
            if (sub != nullptr && sub->GetFullName() == targetName) {
                target = sub;
                break;
            }
        }
        if (target != nullptr) break;
    }
    if (target == nullptr) {
        if (outError) *outError = MakeError(3,
            [NSString stringWithFormat:@"Target model '%@' not found in active sequence.", modelName]);
        return NO;
    }

    Model* targetModel = rc->GetModelManager()[target->GetFullName()];
    int bw = 1, bh = 1;
    if (targetModel != nullptr) {
        targetModel->GetBufferSize("Default", "2D", "None", bw, bh, 0);
    }

    // Read the file into a buffer and apply the FixXMLInputStream-equivalent
    // preprocessor before pugixml parses it.
    std::vector<char> xmlBuffer;
    {
        std::ifstream fs(path.UTF8String, std::ios::binary);
        if (!fs) {
            if (outError) *outError = MakeError(4,
                [NSString stringWithFormat:@"Could not open SuperStar file '%@'.", path]);
            return NO;
        }
        fs.seekg(0, std::ios::end);
        std::streamsize sz = fs.tellg();
        fs.seekg(0, std::ios::beg);
        if (sz > 0) {
            xmlBuffer.resize(static_cast<size_t>(sz));
            fs.read(xmlBuffer.data(), sz);
        }
    }
    if (xmlBuffer.empty()) {
        if (outError) *outError = MakeError(5,
            [NSString stringWithFormat:@"SuperStar file '%@' is empty.", path]);
        return NO;
    }
    SuperStar::PreprocessXmlBuffer(xmlBuffer);

    pugi::xml_document doc;
    pugi::xml_parse_result pr = doc.load_buffer(xmlBuffer.data(), xmlBuffer.size());
    if (!pr) {
        if (outError) *outError = MakeError(6,
            [NSString stringWithFormat:@"Could not parse SuperStar XML: %s",
                                       pr.description()]);
        return NO;
    }

    // Use the file basename (stem) as the default group / image-prefix
    // when the caller didn't override it.
    std::string defaultGroup = std::filesystem::path(path.UTF8String).stem().string();

    SuperStar::Options opt;
    opt.xSize = options.xSize;
    opt.ySize = options.ySize;
    opt.xOffset = options.xOffset;
    opt.yOffset = options.yOffset;
    opt.imageResize = static_cast<SuperStar::ImageResize>(options.imageResize);
    opt.layerBlend = options.layerBlend.length > 0 ? std::string(options.layerBlend.UTF8String)
                                                    : std::string("Normal");
    opt.timingOffsetMs = options.timingOffsetMs;
    opt.frameTimeMs = rc->GetSequenceData().FrameTime();
    opt.modelWidth = bw;
    opt.modelHeight = bh;
    opt.defaultGroupName = defaultGroup;
    opt.imageGroupPrefix = options.imageGroupPrefix.length > 0
        ? std::string(options.imageGroupPrefix.UTF8String)
        : defaultGroup;

    SuperStar::Importer importer(target, &se.GetSequenceMedia(), opt);
    // No prompt callback — imageGroupPrefix is always pre-resolved on iPad.

    std::string err;
    bool ok = importer.Run(doc, &err);
    if (!ok) {
        if (outError) {
            NSString* msg = err.empty()
                ? @"SuperStar import failed."
                : [NSString stringWithUTF8String:err.c_str()];
            *outError = MakeError(7, msg);
        }
        return NO;
    }

    rc->MarkRgbEffectsChanged();
    return YES;
}

@end
