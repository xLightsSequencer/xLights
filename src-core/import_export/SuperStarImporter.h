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

#include <functional>
#include <string>
#include <vector>

namespace pugi {
class xml_document;
class xml_node;
} // namespace pugi

class Element;
class EffectLayer;
class SequenceMedia;

namespace SuperStar {

// Image rescaling strategy. Values match the position in the desktop
// SuperStar import dialog's Image Resizing dropdown.
enum class ImageResize {
    None = 0,
    ExactWidth = 1,
    ExactHeight = 2,
    ExactWidthOrHeight = 3,
    All = 4,
};

struct Options {
    int xSize = 12;
    int ySize = 50;
    int xOffset = 0;
    int yOffset = 0;
    ImageResize imageResize = ImageResize::None;
    // Layer blend choice — the literal string used as T_CHOICE_LayerMethod.
    // "Normal" (or empty) means no override is emitted. Other accepted values
    // include "Average" and "2 reveals 1".
    std::string layerBlend = "2 reveals 1";
    // Timing offset in milliseconds applied to every centisecond /
    // time / startTime / endTime / timeExt attribute before parsing.
    // Run() handles the conversion to centiseconds and the recursive
    // walk; callers must NOT pre-adjust the document.
    int timingOffsetMs = 0;
    // Frame time for animated scene generation (ms per frame).
    int frameTimeMs = 50;
    // Target model buffer dimensions (Model::GetBufferSize "Default"/"2D"/"None").
    int modelWidth = 1;
    int modelHeight = 1;
    // Default group/prefix name suggested when prompting for image group.
    std::string defaultGroupName;
    // If non-empty, used as the image-group prefix without prompting.
    std::string imageGroupPrefix;
};

// Pre-process a raw SuperStar XML buffer in place. Replaces the desktop's
// FixXMLInputStream — closes self-closing elements SuperStar leaves open and
// patches the handful of malformed sequences pugixml chokes on.
void PreprocessXmlBuffer(std::vector<char>& buffer);

// Apply timing offset to centisecond / time / startTime / endTime attributes
// throughout the document. offset is in centiseconds.
void AdjustAllTimings(pugi::xml_node node, int offsetCentiseconds);

// Callback invoked when an image-group prefix is needed and
// Options::imageGroupPrefix is empty. Returns true with `prefix` filled in to
// continue, false to abort the import.
using PrefixPromptCallback = std::function<bool(std::string& prefix)>;

class Importer {
public:
    Importer(Element* targetModel, SequenceMedia* media, Options options);

    void SetPrefixPromptCallback(PrefixPromptCallback cb) {
        _prefixCallback = std::move(cb);
    }

    // Run the import. Returns false on prefix-prompt cancel or fatal parse
    // failure (missing <layouts>); effects already added prior to a failure
    // remain in place, matching desktop behaviour. If `errorOut` is non-null,
    // a human-readable message is written there on fatal failure.
    bool Run(pugi::xml_document& doc, std::string* errorOut = nullptr);

private:
    bool PromptForPrefix();

    Element* _model;
    SequenceMedia* _media;
    Options _opt;
    PrefixPromptCallback _prefixCallback;
    std::string _imagePrefix;
    std::string _blendString; // "" or ",T_CHOICE_LayerMethod=...,"
};

} // namespace SuperStar
