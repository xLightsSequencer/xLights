/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include "SuperStarImporter.h"

#include "render/Effect.h"
#include "render/EffectLayer.h"
#include "render/Element.h"
#include "render/FontManager.h"
#include "render/SequenceMedia.h"
#include "utils/Color.h"
#include "utils/xlImage.h"

#include <pugixml.hpp>
#include <spdlog/fmt/fmt.h>

#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <cstring>
#include <map>
#include <string>
#include <string_view>
#include <vector>

namespace SuperStar {

namespace {

// --- small std-only translations of the wx helpers used by the importer ---

inline int ToInt(const std::string& s) {
    return static_cast<int>(std::strtol(s.c_str(), nullptr, 10));
}

inline int ToInt(const char* s) {
    return static_cast<int>(std::strtol(s ? s : "", nullptr, 10));
}

inline double ToDouble(const std::string& s) {
    return std::strtod(s.c_str(), nullptr);
}

// wxStringTokenizer in default mode for non-whitespace delimiters returns
// empty tokens between consecutive delimiters (`wxTOKEN_RET_EMPTY`). For
// whitespace-only delimiters it switches to strtok semantics. We always
// return empties here and let callers that want strtok semantics (whitespace-
// delimited payloads) skip empties explicitly — matches the SuperStar XML
// where image-pixel `,,,` runs encode transparent pixels by position.
std::vector<std::string> Tokenize(const std::string& s, const char* delims) {
    std::vector<std::string> out;
    size_t start = 0;
    while (true) {
        size_t pos = s.find_first_of(delims, start);
        if (pos == std::string::npos) {
            out.push_back(s.substr(start));
            return out;
        }
        out.push_back(s.substr(start, pos - start));
        start = pos + 1;
    }
}

bool Contains(const std::string& haystack, const char* needle) {
    return haystack.find(needle) != std::string::npos;
}

// --- ports of the static helpers in src-ui-wx/import_export/ImportEffects.cpp ---

bool CalcPercentage(std::string& value, double base, bool reverse, int offset) {
    int val = ToInt(value);
    val -= offset;
    val %= static_cast<int>(base);
    if (val < 0)
        return false;
    double half_width = 1.0 / base * 50.0;
    double percent = static_cast<double>(val) / base * 100.0 + half_width;
    if (reverse) {
        percent = 100.0 - percent;
    }
    value = std::to_string(static_cast<int>(percent));
    return true;
}

bool CalcBoundedPercentage(std::string& value, int base, bool reverse, int offset) {
    int val = ToInt(value);
    val -= offset;
    val %= base;
    if (val < 0)
        return false;
    if (val == 0) {
        value = reverse ? "99.9" : "0.0";
    } else if (val == (base - 1)) {
        value = reverse ? "0.0" : "99.9";
    } else {
        return CalcPercentage(value, base, reverse, offset);
    }
    return true;
}

int CalcUnBoundedPercentage(int val, int base) {
    double half_width = 50.0 / base;
    double percent = static_cast<double>(val) / base * 100.0 + half_width;
    return static_cast<int>(percent);
}

xlColor GetColor(const std::string& sRed, const std::string& sGreen, const std::string& sBlue) {
    double red = std::strtod(sRed.c_str(), nullptr);
    red = red / 100.0 * 255.0;
    double green = std::strtod(sGreen.c_str(), nullptr);
    green = green / 100.0 * 255.0;
    double blue = std::strtod(sBlue.c_str(), nullptr);
    blue = blue / 100.0 * 255.0;
    return xlColor(static_cast<uint8_t>(red), static_cast<uint8_t>(green), static_cast<uint8_t>(blue));
}

std::string GetColorString(const std::string& sRed, const std::string& sGreen, const std::string& sBlue) {
    return static_cast<std::string>(GetColor(sRed, sGreen, sBlue));
}

uint8_t ChannelBlend(uint8_t c1, uint8_t c2, double ratio) {
    return static_cast<uint8_t>(c1 + std::floor(ratio * (c2 - c1) + 0.5));
}

EffectLayer* FindOpenLayer(Element* model, int layer_index, int startTimeMS, int endTimeMS, std::vector<bool>& reserved) {
    int index = layer_index - 1;

    EffectLayer* layer = model->GetEffectLayer(index);
    if (layer != nullptr && layer->GetRangeIsClearMS(startTimeMS, endTimeMS)) {
        return layer;
    }

    for (size_t i = 0; i < model->GetEffectLayerCount(); ++i) {
        if (i >= reserved.size() || !reserved[i]) {
            layer = model->GetEffectLayer(i);
            if (layer->GetRangeIsClearMS(startTimeMS, endTimeMS)) {
                return layer;
            }
        }
    }

    layer = model->AddEffectLayer();
    if (model->GetEffectLayerCount() > reserved.size()) {
        reserved.resize(model->GetEffectLayerCount(), false);
    }
    return layer;
}

struct ImageInfo {
    int xOffset = 0;
    int yOffset = 0;
    int width = 0;
    int height = 0;
    double scaleY = 1.0;
    double scaleX = 1.0;
    std::string imageName;

    void Set(int x, int y, int w, int h, const std::string& n) {
        xOffset = x;
        yOffset = y;
        width = w;
        height = h;
        imageName = n;
        scaleX = 1.0;
        scaleY = 1.0;
    }
};

void ScaleImage(xlImage& img, ImageResize type,
                int modelWidth, int modelHeight,
                int numCol, int numRow,
                ImageInfo& imgInfo,
                bool reverse) {
    bool scale = false;

    imgInfo.xOffset = imgInfo.xOffset + (imgInfo.width - numCol) / 2;
    if (reverse) {
        imgInfo.yOffset = imgInfo.yOffset + (imgInfo.height + 0.5) / 2 - numRow / 2;
    } else {
        imgInfo.yOffset = numRow - imgInfo.yOffset - (numRow + imgInfo.height) / 2;
    }

    switch (type) {
    case ImageResize::None:
        return;
    case ImageResize::ExactWidth:
        if (numCol == imgInfo.width) {
            imgInfo.width = modelWidth;
            imgInfo.scaleX = static_cast<double>(modelWidth) / static_cast<double>(numCol);
            imgInfo.xOffset = static_cast<int>(std::round(static_cast<double>(imgInfo.xOffset) * imgInfo.scaleX));
            scale = true;
        }
        break;
    case ImageResize::ExactHeight:
        if (numRow == imgInfo.height) {
            imgInfo.height = modelHeight;
            imgInfo.scaleY = static_cast<double>(modelHeight) / static_cast<double>(numRow);
            imgInfo.yOffset = static_cast<int>(std::round(static_cast<double>(imgInfo.yOffset) * imgInfo.scaleY));
            scale = true;
        }
        break;
    case ImageResize::ExactWidthOrHeight:
        if (numCol == imgInfo.width) {
            imgInfo.width = modelWidth;
            imgInfo.scaleX = static_cast<double>(modelWidth) / static_cast<double>(numCol);
            imgInfo.xOffset = static_cast<int>(std::round(static_cast<double>(imgInfo.xOffset) * imgInfo.scaleX));
            scale = true;
        }
        if (numRow == imgInfo.height) {
            imgInfo.height = modelHeight;
            imgInfo.scaleY = static_cast<double>(modelHeight) / static_cast<double>(numRow);
            imgInfo.yOffset = static_cast<int>(std::round(static_cast<double>(imgInfo.yOffset) * imgInfo.scaleY));
            scale = true;
        }
        break;
    case ImageResize::All: {
        imgInfo.scaleX = static_cast<double>(modelWidth) / static_cast<double>(numCol);
        imgInfo.scaleY = static_cast<double>(modelHeight) / static_cast<double>(numRow);
        int newW = static_cast<int>(std::round(static_cast<double>(imgInfo.width) * imgInfo.scaleX));
        int newH = static_cast<int>(std::round(static_cast<double>(imgInfo.height) * imgInfo.scaleY));
        if (newH != imgInfo.height || newW != imgInfo.width) {
            scale = true;
            imgInfo.height = newH;
            imgInfo.width = newW;
            imgInfo.yOffset = static_cast<int>(std::round(static_cast<double>(imgInfo.yOffset) * imgInfo.scaleY));
            imgInfo.xOffset = static_cast<int>(std::round(static_cast<double>(imgInfo.xOffset) * imgInfo.scaleX));
        }
        break;
    }
    }
    if (scale) {
        img.Rescale(imgInfo.width, imgInfo.height);
    }
}

std::string CreateSceneImage(const std::string& imagePfx, const std::string& postFix,
                             pugi::xml_node element, int numCols, int numRows,
                             bool reverse, bool rotate, const xlColor& color, int y_offset,
                             ImageResize resizeType, int modelWidth, int modelHeight,
                             SequenceMedia* media) {
    xlImage i(numCols, numRows); // zero-initialised → fully transparent
    for (pugi::xml_node e = element.first_child(); e; e = e.next_sibling()) {
        if (std::string_view(e.name()) == "element") {
            int x = e.attribute("ribbonIndex").as_int();
            int y = e.attribute("pixelIndex").as_int() - y_offset;
            if (rotate) {
                std::swap(x, y);
            }
            if (rotate ^ reverse) {
                y = numRows - y;
            }
            if (x < numCols && y >= 0 && y < numRows) {
                i.SetRGBA(x, y, color.Red(), color.Green(), color.Blue(), 255);
            }
        }
    }
    std::string name = imagePfx + "/s" + element.attribute("savedIndex").as_string() + postFix + ".png";
    ImageInfo im;
    im.Set(0, 0, numCols, numRows, name);
    ScaleImage(i, resizeType, modelWidth, modelHeight, numCols, numRows, im, false);
    if (media) {
        media->AddEmbeddedImage(name, i);
    }
    return name;
}

struct Rect {
    int x = 0;
    int y = 0;
    int width = 0;
    int height = 0;
};

bool IsPartOfModel(pugi::xml_node element, int num_rows, int num_columns, bool& isFull, Rect& rect, bool reverse) {
    if (!element)
        return false;

    std::vector<std::vector<bool>> data(num_columns, std::vector<bool>(num_rows));
    int maxCol = -1;
    int maxRow = -1;
    int minCol = 9999999;
    int minRow = 9999999;
    isFull = true;
    for (pugi::xml_node e = element.first_child(); e; e = e.next_sibling()) {
        if (std::string_view(e.name()) == "element") {
            int x = e.attribute("ribbonIndex").as_int();
            int y = e.attribute("pixelIndex").as_int();
            if (reverse) {
                std::swap(x, y);
            }
            if (x < num_columns && y < num_rows) {
                data[x][y] = true;
                if (x > maxCol)
                    maxCol = x;
                if (x < minCol)
                    minCol = x;
                if (y > maxRow)
                    maxRow = y;
                if (y < minRow)
                    minRow = y;
            } else {
                return false;
            }
        }
    }
    isFull = minCol == 0 && minRow == 0 && maxRow == (num_rows - 1) && maxCol == (num_columns - 1);
    bool isRect = true;
    for (int x = minCol; x <= maxCol; x++) {
        for (int y = minRow; y <= maxRow; y++) {
            if (!data[x][y]) {
                isFull = false;
                isRect = false;
            }
        }
    }
    if (isRect) {
        rect.x = minCol;
        rect.y = minRow;
        rect.width = maxCol;
        rect.height = maxRow;
    } else {
        rect.x = -1;
        rect.y = -1;
    }
    return true;
}

// Build "C_BUTTON_PaletteN=<color>,C_CHECKBOX_PaletteN=1" entries for a flowy.
std::string BuildFlowyPalette(const std::string& color_string) {
    std::string palette = "C_BUTTON_Palette1=,";
    int cnt = 1;
    for (auto const& tok : Tokenize(color_string, " ")) {
        // wxStringTokenizer with whitespace delimiters skips empties
        // (strtok mode) — match that here since Tokenize now returns
        // empties uniformly.
        if (tok.empty()) continue;
        if (cnt > 6) break;
        auto rgb = Tokenize(tok, ",");
        if (rgb.size() < 3) continue;
        std::string color = GetColorString(rgb[0], rgb[1], rgb[2]);
        if (cnt > 1) palette += ",";
        palette += "C_BUTTON_Palette" + std::to_string(cnt) + "=" + color;
        palette += ",C_CHECKBOX_Palette" + std::to_string(cnt) + "=1";
        cnt++;
    }
    while (cnt <= 6) {
        if (cnt > 1) palette += ",";
        palette += "C_BUTTON_Palette" + std::to_string(cnt) + "=#000000";
        palette += ",C_CHECKBOX_Palette" + std::to_string(cnt) + "=0";
        cnt++;
    }
    return palette;
}

} // namespace

// --- public free functions --------------------------------------------------

// Pre-process a SuperStar XML buffer in place. Same fixups as the legacy
// FixXMLInputStream: drop a "<pixels=" attribute typo, force-close several
// elements that SuperStar leaves open, and strip <configuration>...</configuration>.
void PreprocessXmlBuffer(std::vector<char>& buffer) {
    if (buffer.empty()) return;
    bool needToClose = false;
    char* buf = buffer.data();
    size_t bufLen = buffer.size();
    auto starts = [&](size_t end, const char* token) {
        size_t tlen = std::strlen(token);
        if (end + 1 < tlen) return false;
        return std::memcmp(&buf[end + 1 - tlen], token, tlen) == 0;
    };
    for (size_t x = 7; x < bufLen; ++x) {
        if (starts(x, "<pixels=")) {
            buf[x - 2] = ' ';
        } else if (buf[x] == ' ' && starts(x - 1, "<timing")) {
            needToClose = true;
        } else if (x > 6 && buf[x] == ' ' && starts(x - 1, "<group")) {
            needToClose = true;
        } else if (x > 12 && buf[x] == ' ' && starts(x - 1, "<imageAction")) {
            needToClose = true;
        } else if (x > 11 && buf[x] == ' ' && starts(x - 1, "<textAction")) {
            needToClose = true;
        } else if (x > 14 && buf[x] == ' ' && starts(x - 1, "<configuration")) {
            needToClose = true;
        } else if (x > 15 && starts(x, "</configuration>")) {
            for (size_t y = x - 15; y <= x; ++y) {
                buf[y] = ' ';
            }
        } else if (buf[x - 1] == '>' && needToClose) {
            if (buf[x - 2] != '/') {
                buf[x - 1] = '/';
                buf[x] = '>';
            }
            needToClose = false;
        }
    }
}

void AdjustAllTimings(pugi::xml_node node, int offsetCentiseconds) {
    if (auto attr = node.attribute("startCentisecond")) {
        attr.set_value(attr.as_int() + offsetCentiseconds);
    }
    if (auto attr = node.attribute("endCentisecond")) {
        attr.set_value(attr.as_int() + offsetCentiseconds);
    }
    std::string nodeName = node.name();
    if (nodeName == "flowy") {
        if (auto attr = node.attribute("startTime")) {
            attr.set_value(attr.as_int() + offsetCentiseconds);
        }
        if (auto attr = node.attribute("endTime")) {
            attr.set_value(attr.as_int() + offsetCentiseconds);
        }
    } else if (nodeName == "state1" || nodeName == "state2") {
        if (auto attr = node.attribute("time")) {
            attr.set_value(attr.as_int() + offsetCentiseconds);
        }
    }
    for (pugi::xml_node chan = node.first_child(); chan; chan = chan.next_sibling()) {
        AdjustAllTimings(chan, offsetCentiseconds);
    }
}

// --- Importer ---------------------------------------------------------------

Importer::Importer(Element* targetModel, SequenceMedia* media, Options options)
    : _model(targetModel), _media(media), _opt(std::move(options)) {
    if (!_opt.imageGroupPrefix.empty()) {
        _imagePrefix = _opt.imageGroupPrefix;
    }
    if (_opt.layerBlend != "Normal" && !_opt.layerBlend.empty()) {
        _blendString = ",T_CHOICE_LayerMethod=" + _opt.layerBlend + ",";
    }
}

bool Importer::PromptForPrefix() {
    if (!_imagePrefix.empty()) return true;
    std::string defGroup = _opt.defaultGroupName.empty() ? std::string("SuperStar") : _opt.defaultGroupName;
    if (!_prefixCallback) {
        _imagePrefix = defGroup;
        return true;
    }
    std::string val;
    if (!_prefixCallback(val)) return false;
    // Trim whitespace from both ends.
    auto first = val.find_first_not_of(" \t\r\n");
    auto last = val.find_last_not_of(" \t\r\n");
    if (first == std::string::npos) {
        val.clear();
    } else {
        val = val.substr(first, last - first + 1);
    }
    if (val.empty()) val = defGroup;
    _imagePrefix = val;
    return true;
}

bool Importer::Run(pugi::xml_document& input_xml, std::string* errorOut) {
    if (_opt.timingOffsetMs != 0) {
        // Centiseconds in the SuperStar XML; spin control gives ms.
        AdjustAllTimings(input_xml.document_element(), _opt.timingOffsetMs / 10);
    }
    double num_rows = 1.0;
    double num_columns = 1.0;
    bool reverse_rows = false;
    bool reverse_xy = false;
    bool layout_defined = false;
    pugi::xml_node input_root = input_xml.document_element();
    EffectLayer* layer = _model->AddEffectLayer();
    std::map<int, ImageInfo> imageInfo;
    std::vector<bool> reserved;

    // --- first pass: gather reserved layers, prompt for prefix, read layout ---
    for (pugi::xml_node e = input_root.first_child(); e; e = e.next_sibling()) {
        std::string ename = e.name();
        if (ename == "imageActions") {
            for (pugi::xml_node element = e.first_child(); element; element = element.next_sibling()) {
                if (std::string_view(element.name()) == "imageAction") {
                    int layer_index = element.attribute("layer").as_int();
                    if (layer_index > 0) layer_index--;
                    if (layer_index >= static_cast<int>(reserved.size())) {
                        reserved.resize(layer_index + 1, false);
                    }
                    reserved[layer_index] = true;
                }
            }
        } else if (ename == "scenes" || ename == "images") {
            for (pugi::xml_node element = e.first_child(); element && _imagePrefix.empty(); element = element.next_sibling()) {
                std::string elemname = element.name();
                if (elemname == "image" || elemname == "scene") {
                    if (!PromptForPrefix()) return false;
                }
            }
        } else if (ename == "layouts") {
            pugi::xml_node element = e.first_child();
            std::string attr = element.attribute("visualizationMode").as_string();
            if (attr == "false") {
                num_columns = element.attribute("nbrOfRibbons").as_double();
                num_rows = 50.0;
                attr = element.attribute("ribbonLength").as_string();
                if (attr == "half") {
                    num_rows /= 2.0;
                    num_columns *= 2.0;
                }
            } else {
                num_rows = static_cast<double>(_opt.ySize);
                num_columns = static_cast<double>(_opt.xSize);
            }
            attr = element.attribute("ribbonOrientation").as_string();
            if (attr == "horizontal") {
                reverse_xy = true;
                std::swap(num_columns, num_rows);
            } else {
                reverse_rows = true;
            }
            layout_defined = true;
        }
    }

    const int x_offset = _opt.xOffset;
    const int y_offset = _opt.yOffset;
    const ImageResize imageResizeType = _opt.imageResize;
    const int modelW = _opt.modelWidth;
    const int modelH = _opt.modelHeight;

    // --- second pass: emit effects ---
    for (pugi::xml_node e = input_root.first_child(); e; e = e.next_sibling()) {
        std::string ename = e.name();

        if (ename == "morphs") {
            if (!layout_defined) {
                if (errorOut) *errorOut = "The layouts section was not found in the SuperStar file!";
                return false;
            }
            for (pugi::xml_node element = e.first_child(); element; element = element.next_sibling()) {
                std::string acceleration = element.attribute("acceleration").as_string();
                std::string attr = element.attribute("layer").as_string();
                double layer_val = ToDouble(attr);
                int layer_index = static_cast<int>(layer_val);
                pugi::xml_node state1 = element.first_child();
                pugi::xml_node state2 = state1.next_sibling();
                pugi::xml_node ramp = state2.next_sibling();
                std::string state1_time = state1.attribute("time").as_string();
                std::string state2_time = state2.attribute("time").as_string();
                std::string ramp_time_ext = ramp.attribute("timeExt").as_string();
                int start_time = ToInt(state1_time) * 10;
                int end_time = ToInt(state2_time) * 10;
                int ramp_time = ToInt(ramp_time_ext) * 10;
                end_time += ramp_time;
                double head_duration = (1.0 - static_cast<double>(ramp_time) / (static_cast<double>(end_time) - static_cast<double>(start_time))) * 100.0;
                std::string settings = "E_CHECKBOX_Morph_End_Link=0,E_CHECKBOX_Morph_Start_Link=0,E_CHECKBOX_ShowHeadAtStart=0,E_NOTEBOOK_Morph=Start,E_SLIDER_MorphAccel=0,E_SLIDER_Morph_Repeat_Count=0,E_SLIDER_Morph_Repeat_Skip=1,E_SLIDER_Morph_Stagger=0";
                settings += acceleration + ",";
                settings += "E_SLIDER_MorphDuration=" + std::to_string(static_cast<int>(head_duration)) + ",";
                attr = state2.attribute("trailLen").as_string();
                settings += "E_SLIDER_MorphEndLength=" + attr + ",";
                attr = state1.attribute("trailLen").as_string();
                settings += "E_SLIDER_MorphStartLength=" + attr + ",";

                attr = (!reverse_xy) ? state2.attribute("x1").as_string() : state2.attribute("y1").as_string();
                if (!CalcPercentage(attr, num_columns, false, x_offset)) continue;
                settings += "E_SLIDER_Morph_End_X1=" + attr + ",";
                attr = (!reverse_xy) ? state2.attribute("x2").as_string() : state2.attribute("y2").as_string();
                if (!CalcPercentage(attr, num_columns, false, x_offset)) continue;
                settings += "E_SLIDER_Morph_End_X2=" + attr + ",";
                attr = (!reverse_xy) ? state2.attribute("y1").as_string() : state2.attribute("x1").as_string();
                if (!CalcPercentage(attr, num_rows, reverse_rows, y_offset)) continue;
                settings += "E_SLIDER_Morph_End_Y1=" + attr + ",";
                attr = (!reverse_xy) ? state2.attribute("y2").as_string() : state2.attribute("x2").as_string();
                if (!CalcPercentage(attr, num_rows, reverse_rows, y_offset)) continue;
                settings += "E_SLIDER_Morph_End_Y2=" + attr + ",";
                attr = (!reverse_xy) ? state1.attribute("x1").as_string() : state1.attribute("y1").as_string();
                if (!CalcPercentage(attr, num_columns, false, x_offset)) continue;
                settings += "E_SLIDER_Morph_Start_X1=" + attr + ",";
                attr = (!reverse_xy) ? state1.attribute("x2").as_string() : state1.attribute("y2").as_string();
                if (!CalcPercentage(attr, num_columns, false, x_offset)) continue;
                settings += "E_SLIDER_Morph_Start_X2=" + attr + ",";
                attr = (!reverse_xy) ? state1.attribute("y1").as_string() : state1.attribute("x1").as_string();
                if (!CalcPercentage(attr, num_rows, reverse_rows, y_offset)) continue;
                settings += "E_SLIDER_Morph_Start_Y1=" + attr + ",";
                attr = (!reverse_xy) ? state1.attribute("y2").as_string() : state1.attribute("x2").as_string();
                if (!CalcPercentage(attr, num_rows, reverse_rows, y_offset)) continue;
                settings += "E_SLIDER_Morph_Start_Y2=" + attr + ",";

                std::string sRed = state1.attribute("red").as_string();
                std::string sGreen = state1.attribute("green").as_string();
                std::string sBlue = state1.attribute("blue").as_string();
                std::string color = GetColorString(sRed, sGreen, sBlue);
                std::string palette = "C_BUTTON_Palette1=" + color + ",";
                sRed = state2.attribute("red").as_string();
                sGreen = state2.attribute("green").as_string();
                sBlue = state2.attribute("blue").as_string();
                color = GetColorString(sRed, sGreen, sBlue);
                palette += "C_BUTTON_Palette2=" + color + ",";
                sRed = ramp.attribute("red1").as_string();
                sGreen = ramp.attribute("green1").as_string();
                sBlue = ramp.attribute("blue1").as_string();
                color = GetColorString(sRed, sGreen, sBlue);
                palette += "C_BUTTON_Palette3=" + color + ",";
                sRed = ramp.attribute("red2").as_string();
                sGreen = ramp.attribute("green2").as_string();
                sBlue = ramp.attribute("blue2").as_string();
                color = GetColorString(sRed, sGreen, sBlue);
                palette += "C_BUTTON_Palette4=" + color + ",";
                palette += "C_BUTTON_Palette5=#FFFFFF,C_BUTTON_Palette6=#000000,C_CHECKBOX_Palette1=1,C_CHECKBOX_Palette2=1,C_CHECKBOX_Palette3=1,C_CHECKBOX_Palette4=1,";
                settings += _blendString;
                while (static_cast<int>(_model->GetEffectLayerCount()) < layer_index) {
                    _model->AddEffectLayer();
                }
                layer = FindOpenLayer(_model, layer_index, start_time, end_time, reserved);
                layer->AddEffect(0, "Morph", settings, palette, start_time, end_time, false, false);
            }

        } else if (ename == "images") {
            for (pugi::xml_node element = e.first_child(); element; element = element.next_sibling()) {
                if (std::string_view(element.name()) != "image") continue;
                for (pugi::xml_node i = element.first_child(); i; i = i.next_sibling()) {
                    if (std::string_view(i.name()) != "pixe") continue;
                    std::string data = i.attribute("s").as_string();
                    int w = element.attribute("width").as_int();
                    int h = element.attribute("height").as_int();
                    int idx = element.attribute("savedIndex").as_int();
                    int xOffsetA = element.attribute("xOffset").as_int();
                    int yOffsetA = element.attribute("yOffset").as_int();

                    xlImage image(w, h);
                    int p = 0;
                    for (auto const& tok : Tokenize(data, ",")) {
                        unsigned int ii = static_cast<unsigned int>(ToInt(tok));
                        unsigned int r = ((ii >> 16) & 0xff) * 255 / 100;
                        unsigned int g = ((ii >> 8) & 0xff) * 255 / 100;
                        unsigned int b = (ii & 0xff) * 255 / 100;
                        uint8_t a = (ii == 0) ? 0 : 255;
                        int px = p % w;
                        int py = p / w;
                        if (py < h) image.SetRGBA(px, py, r, g, b, a);
                        p++;
                    }

                    if (!PromptForPrefix()) return false;
                    std::string fname = _imagePrefix + "/" + std::to_string(idx) + ".png";
                    if (reverse_xy) {
                        image = image.Rotate90(false);
                        imageInfo[idx].Set(yOffsetA, xOffsetA, h, w, fname);
                    } else {
                        imageInfo[idx].Set(xOffsetA, yOffsetA, w, h, fname);
                    }
                    ScaleImage(image, imageResizeType, modelW, modelH,
                               static_cast<int>(num_columns), static_cast<int>(num_rows),
                               imageInfo[idx], reverse_xy);
                    if (_media) _media->AddEmbeddedImage(fname, image);
                }
            }

        } else if (ename == "flowys") {
            for (pugi::xml_node element = e.first_child(); element; element = element.next_sibling()) {
                if (std::string_view(element.name()) != "flowy") continue;
                std::string centerX, centerY;
                int startms = element.attribute("startTime").as_int() * 10;
                int endms = element.attribute("endTime").as_int() * 10;
                std::string type = element.attribute("flowyType").as_string();
                std::string color_string = element.attribute("Colors").as_string();
                std::string palette = BuildFlowyPalette(color_string);

                int layer_index = element.attribute("layer").as_int();
                int acceleration = element.attribute("acceleration").as_int();
                centerX = (!reverse_xy) ? element.attribute("centerX").as_string()
                                        : element.attribute("centerY").as_string();
                if (!CalcPercentage(centerX, num_columns, false, x_offset)) continue;
                centerY = (!reverse_xy) ? element.attribute("centerY").as_string()
                                        : element.attribute("centerX").as_string();
                if (!CalcPercentage(centerY, num_rows, reverse_rows, y_offset)) continue;

                int startAngle = element.attribute("startAngle").as_int();
                int endAngle = element.attribute("endAngle").as_int();
                if (reverse_xy) {
                    startAngle -= 90;
                    endAngle -= 90;
                    if (startAngle < 0) startAngle += 360;
                    if (endAngle < 0) endAngle += 360;
                }
                int revolutions = std::abs(endAngle - startAngle);
                if (revolutions == 0)
                    revolutions = 3; // algorithm needs non-zero value until we figure out a better way
                int startRadius = element.attribute("startRadius").as_int();
                int endRadius = element.attribute("endRadius").as_int();

                if (type == "Spiral") {
                    int tailms = element.attribute("tailTimeLength").as_int() * 10;
                    endms += tailms;
                    double duration = (1.0 - static_cast<double>(tailms) / (static_cast<double>(endms) - static_cast<double>(startms))) * 100.0;
                    int startWidth = element.attribute("startDotSize").as_int();
                    int endWidth = element.attribute("endDotSize").as_int();
                    std::string settings =
                        "E_CHECKBOX_Galaxy_Reverse=" + std::to_string(endAngle < startAngle) +
                        ",E_CHECKBOX_Galaxy_Blend_Edges=1,E_CHECKBOX_Galaxy_Inward=1,E_NOTEBOOK_Galaxy=Start" +
                        ",E_SLIDER_Galaxy_Accel=" + std::to_string(acceleration) +
                        ",E_SLIDER_Galaxy_CenterX=" + centerX +
                        ",E_SLIDER_Galaxy_CenterY=" + centerY +
                        ",E_SLIDER_Galaxy_Duration=" + std::to_string(static_cast<int>(duration)) +
                        ",E_SLIDER_Galaxy_End_Radius=" + std::to_string(endRadius) +
                        ",E_SLIDER_Galaxy_End_Width=" + std::to_string(endWidth) +
                        ",E_SLIDER_Galaxy_Revolutions=" + std::to_string(revolutions) +
                        ",E_SLIDER_Galaxy_Start_Angle=" + std::to_string(startAngle) +
                        ",E_SLIDER_Galaxy_Start_Radius=" + std::to_string(startRadius) +
                        ",E_SLIDER_Galaxy_Start_Width=" + std::to_string(startWidth) +
                        _blendString;
                    layer = FindOpenLayer(_model, layer_index, startms, endms, reserved);
                    layer->AddEffect(0, "Galaxy", settings, palette, startms, endms, false, false);
                } else if (type == "Shockwave") {
                    int startWidth = element.attribute("headWidth").as_int();
                    int endWidth = element.attribute("tailWidth").as_int();
                    std::string settings =
                        "E_CHECKBOX_Shockwave_Blend_Edges=1,E_NOTEBOOK_Shockwave=Position" +
                        std::string(",E_SLIDER_Shockwave_Accel=") + std::to_string(acceleration) +
                        ",E_SLIDER_Shockwave_CenterX=" + centerX +
                        ",E_SLIDER_Shockwave_CenterY=" + centerY +
                        ",E_SLIDER_Shockwave_End_Radius=" + std::to_string(endRadius) +
                        ",E_SLIDER_Shockwave_End_Width=" + std::to_string(endWidth) +
                        ",E_SLIDER_Shockwave_Start_Radius=" + std::to_string(startRadius) +
                        ",E_SLIDER_Shockwave_Start_Width=" + std::to_string(startWidth) +
                        _blendString;
                    layer = FindOpenLayer(_model, layer_index, startms, endms, reserved);
                    layer->AddEffect(0, "Shockwave", settings, palette, startms, endms, false, false);
                } else if (type == "Fan") {
                    int revolutionsPerSecond = element.attribute("revolutionsPerSecond").as_int();
                    int blades = element.attribute("blades").as_int();
                    int blade_width = element.attribute("width").as_int();
                    int elementAngle = element.attribute("elementAngle").as_int();
                    int elementStepAngle = element.attribute("elementStepAngle").as_int();
                    int numElements = elementStepAngle != 0 ? elementAngle / elementStepAngle : 1;
                    numElements = std::clamp(numElements, 1, 4);
                    blades = std::clamp(blades, 1, 16);
                    int tailms = static_cast<int>(static_cast<double>(endms - startms) * (blade_width * 2.0) / 100.0) + 35;
                    endms += tailms;
                    int duration = 100 - static_cast<int>(tailms * 100.0 / (endms - startms));
                    int fanRevolutions = static_cast<int>(static_cast<double>(revolutionsPerSecond) * (static_cast<double>(endms - startms) / 1000.0) * 3.6);
                    std::string settings =
                        "E_CHECKBOX_Fan_Reverse=" + std::to_string(startAngle > endAngle) +
                        ",E_CHECKBOX_Fan_Blend_Edges=1,E_NOTEBOOK_Fan=Position" +
                        ",E_SLIDER_Fan_Accel=" + std::to_string(acceleration) +
                        ",E_SLIDER_Fan_Blade_Angle=" + std::to_string(elementAngle) +
                        ",E_SLIDER_Fan_Blade_Width=" + std::to_string(blade_width) +
                        ",E_SLIDER_Fan_CenterX=" + centerX +
                        ",E_SLIDER_Fan_CenterY=" + centerY +
                        ",E_SLIDER_Fan_Duration=" + std::to_string(duration) +
                        ",E_SLIDER_Fan_Element_Width=100" +
                        ",E_SLIDER_Fan_Num_Blades=" + std::to_string(blades) +
                        ",E_SLIDER_Fan_Num_Elements=" + std::to_string(numElements) +
                        ",E_SLIDER_Fan_End_Radius=" + std::to_string(endRadius) +
                        ",E_SLIDER_Fan_Revolutions=" + std::to_string(fanRevolutions) +
                        ",E_SLIDER_Fan_Start_Angle=" + std::to_string(startAngle) +
                        ",E_SLIDER_Fan_Start_Radius=" + std::to_string(startRadius) +
                        _blendString;
                    layer = FindOpenLayer(_model, layer_index, startms, endms, reserved);
                    layer->AddEffect(0, "Fan", settings, palette, startms, endms, false, false);
                }
            }

        } else if (ename == "scenes") {
            for (pugi::xml_node element = e.first_child(); element; element = element.next_sibling()) {
                if (std::string_view(element.name()) != "scene") continue;
                std::string startms_s = std::string(element.attribute("startCentisecond").as_string()) + "0";
                std::string endms_s = std::string(element.attribute("endCentisecond").as_string()) + "0";
                std::string type = element.attribute("type").as_string();
                int layer_index = element.attribute("layer").as_int();
                xlColor startc = GetColor(element.attribute("red1").as_string(),
                                          element.attribute("green1").as_string(),
                                          element.attribute("blue1").as_string());
                xlColor endc = GetColor(element.attribute("red2").as_string(),
                                        element.attribute("green2").as_string(),
                                        element.attribute("blue2").as_string());
                while (static_cast<int>(_model->GetEffectLayerCount()) < layer_index) {
                    _model->AddEffectLayer();
                }

                int start_time = ToInt(startms_s);
                int end_time = ToInt(endms_s);
                layer = FindOpenLayer(_model, layer_index, start_time, end_time, reserved);
                if (!PromptForPrefix()) return false;

                std::string ru = "0.0";
                std::string rd = "0.0";
                std::string imageName;
                bool isFull = false;
                Rect rect;
                bool isPartOfModel = IsPartOfModel(element, static_cast<int>(num_rows), static_cast<int>(num_columns),
                                                   isFull, rect, reverse_xy);

                if (isPartOfModel && isFull) {
                    std::string palette = "C_BUTTON_Palette1=" + static_cast<std::string>(startc) +
                                          ",C_CHECKBOX_Palette1=1,C_BUTTON_Palette2=" + static_cast<std::string>(endc) +
                                          ",C_CHECKBOX_Palette2=1,";
                    std::string settings = _blendString;
                    if (startc == endc) {
                        layer->AddEffect(0, "On", settings, palette, start_time, end_time, false, false);
                    } else if (startc == xlBLACK) {
                        std::string palette1 = "C_BUTTON_Palette1=" + static_cast<std::string>(endc) +
                                               ",C_CHECKBOX_Palette1=1,C_BUTTON_Palette2=" + static_cast<std::string>(startc) +
                                               ",C_CHECKBOX_Palette2=1";
                        settings += ",E_TEXTCTRL_Eff_On_Start=0";
                        layer->AddEffect(0, "On", settings, palette1, start_time, end_time, false, false);
                    } else if (endc == xlBLACK) {
                        layer->AddEffect(0, "On", "E_TEXTCTRL_Eff_On_End=0", palette, start_time, end_time, false, false);
                    } else {
                        layer->AddEffect(0, "Color Wash", settings, palette, start_time, end_time, false, false);
                    }
                } else if (isPartOfModel && rect.x != -1) {
                    std::string palette = "C_BUTTON_Palette1=" + static_cast<std::string>(startc) +
                                          ",C_CHECKBOX_Palette1=1,C_BUTTON_Palette2=" + static_cast<std::string>(endc) +
                                          ",C_CHECKBOX_Palette2=1";
                    std::string settings = "B_CUSTOM_SubBuffer=";
                    std::string val = std::to_string(rect.x);
                    if (!CalcBoundedPercentage(val, static_cast<int>(num_columns), false, x_offset)) continue;
                    settings += val + "x";
                    val = std::to_string(rect.y);
                    if (!CalcBoundedPercentage(val, static_cast<int>(num_rows), reverse_rows ^ reverse_xy, y_offset)) continue;
                    settings += val + "x";
                    val = std::to_string(rect.width);
                    if (!CalcBoundedPercentage(val, static_cast<int>(num_columns), false, x_offset)) continue;
                    settings += val + "x";
                    val = std::to_string(rect.height);
                    if (!CalcBoundedPercentage(val, static_cast<int>(num_rows), reverse_rows ^ reverse_xy, y_offset)) continue;
                    settings += val;
                    settings += _blendString;
                    layer->AddEffect(0, "Color Wash", settings, palette, start_time, end_time, false, false);
                } else if (isPartOfModel) {
                    if (startc == xlBLACK || endc == xlBLACK || endc == startc) {
                        imageName = CreateSceneImage(_imagePrefix, "", element,
                                                     static_cast<int>(num_columns), static_cast<int>(num_rows),
                                                     false, reverse_xy,
                                                     (startc == xlBLACK) ? endc : startc, y_offset,
                                                     imageResizeType, modelW, modelH, _media);
                        std::string ramp = fmt::format("{:f}", static_cast<double>(end_time - start_time) / 1000.0);
                        if (endc == xlBLACK) rd = ramp;
                        if (startc == xlBLACK) ru = ramp;
                    } else {
                        // Colour-changing scene: build one animated entry, one frame per sequence frame.
                        int time = ToInt(endms_s) - ToInt(startms_s);
                        int ft = _opt.frameTimeMs > 0 ? _opt.frameTimeMs : 50;
                        int numFrames = time / ft;
                        std::string animName = _imagePrefix + "/s" +
                                               element.attribute("savedIndex").as_string() + ".png";
                        std::vector<xlImage> animFrames;
                        animFrames.reserve(numFrames);
                        int nc = static_cast<int>(num_columns);
                        int nr = static_cast<int>(num_rows);
                        for (int x = 0; x < numFrames; x++) {
                            double ratio = static_cast<double>(x) / numFrames;
                            xlColor color;
                            color.Set(ChannelBlend(startc.Red(), endc.Red(), ratio),
                                      ChannelBlend(startc.Green(), endc.Green(), ratio),
                                      ChannelBlend(startc.Blue(), endc.Blue(), ratio));
                            xlImage img(nc, nr);
                            for (pugi::xml_node e2 = element.first_child(); e2; e2 = e2.next_sibling()) {
                                if (std::string_view(e2.name()) == "element") {
                                    int px = e2.attribute("ribbonIndex").as_int();
                                    int py = e2.attribute("pixelIndex").as_int() - y_offset;
                                    if (reverse_xy) std::swap(px, py);
                                    if (px < nc && py >= 0 && py < nr) {
                                        img.SetRGBA(px, py, color.Red(), color.Green(), color.Blue(), 255);
                                    }
                                }
                            }
                            ImageInfo im;
                            im.Set(0, 0, nc, nr, animName);
                            ScaleImage(img, imageResizeType, modelW, modelH, nc, nr, im, reverse_xy);
                            animFrames.push_back(std::move(img));
                        }
                        if (_media) _media->AddEmbeddedImage(animName, animFrames, ft);
                        imageName = animName;
                    }

                    std::string settings = "E_CHECKBOX_Pictures_WrapX=0,E_CHOICE_Pictures_Direction=none,"
                                           "E_SLIDER_PicturesXC=0"
                                           ",E_SLIDER_PicturesYC=0"
                                           ",E_CHOICE_Scaling=Scale To Fit"
                                           ",E_SLIDER_Pictures_StartScale=100"
                                           ",E_SLIDER_Pictures_EndScale=100"
                                           ",E_CHECKBOX_Pictures_PixelOffsets=1"
                                           ",E_TEXTCTRL_Pictures_Speed=1.0"
                                           ",E_TEXTCTRL_Pictures_FrameRateAdj=1.0"
                                           ",E_TEXTCTRL_Pictures_Filename=" +
                                           imageName;
                    if (type == "shimmer") settings += ",E_CHECKBOX_Pictures_Shimmer=1";
                    settings += _blendString;
                    if (ru != "0.0") settings += ",T_TEXTCTRL_Fadein=" + ru;
                    if (rd != "0.0") settings += ",T_TEXTCTRL_Fadeout=" + rd;
                    layer->AddEffect(0, "Pictures", settings, "", start_time, end_time, false, false);
                }
            }

        } else if (ename == "textActions") {
            for (pugi::xml_node element = e.first_child(); element; element = element.next_sibling()) {
                if (std::string_view(element.name()) != "textAction") continue;
                std::string startms_s = std::string(element.attribute("startCentisecond").as_string()) + "0";
                std::string endms_s = std::string(element.attribute("endCentisecond").as_string()) + "0";
                std::string text = element.attribute("text").as_string();
                std::string fontName = element.attribute("fontName").as_string();
                int fontSize = element.attribute("fontCapsHeight").as_int(6);
                int fontCellWidth = element.attribute("fontCellWidth").as_int(6);
                int fontCellHeight = element.attribute("fontCellHeight").as_int(6);
                std::string colorType = element.attribute("colorType").as_string("chooseColor");
                int fCI = element.attribute("firstColorIndex").as_int(0);
                std::string mask = element.attribute("maskType").as_string();
                bool use_xl_font = true;
                std::string xl_font_name = fmt::format("{}-{}x{} {}", fontSize, fontCellWidth, fontCellHeight, fontName);
                std::replace(xl_font_name.begin(), xl_font_name.end(), '_', ' ');
                xlFont* xl_font = FontManager::get_font(xl_font_name);
                if (xl_font == nullptr) {
                    xl_font_name = "Use OS Fonts";
                    use_xl_font = false;
                }

                // SuperStar fonts are not as wide as they are listed.
                fontCellWidth = (fontCellWidth * 2) / 3;
                if (!use_xl_font) fontSize += 4;

                int rotation = element.attribute("rotation").as_int(90);
                if (reverse_xy) rotation -= 90;
                int xStart = element.attribute("xStart").as_int(0);
                int yStart = element.attribute("yStart").as_int(0);
                int xEnd = element.attribute("xEnd").as_int(0);
                int yEnd = element.attribute("yEnd").as_int(0);
                if (reverse_xy) {
                    std::swap(xStart, yStart);
                    std::swap(xEnd, yEnd);
                }

                xlColor color = GetColor(element.attribute("red").as_string(),
                                         element.attribute("green").as_string(),
                                         element.attribute("blue").as_string());

                int layer_index = element.attribute("layer").as_int();
                while (static_cast<int>(_model->GetEffectLayerCount()) < layer_index) {
                    _model->AddEffectLayer();
                }
                int start_time = ToInt(startms_s);
                int end_time = ToInt(endms_s);
                layer = FindOpenLayer(_model, layer_index, start_time, end_time, reserved);
                int lorWidth = static_cast<int>(text.size()) * fontCellWidth;
                int lorHeight = fontSize;

                if (use_xl_font) {
                    lorWidth = FontManager::get_length(xl_font, text) - 2;
                }

                std::string font = "arial " + fmt::format("{}{}", (Contains(fontName, "Bold") ? "bold " : ""), fontSize);
                std::string eff = "normal";

                if (use_xl_font) {
                    if (rotation == 90) {
                        eff = "rotate down 90";
                        lorHeight = lorWidth;
                        lorWidth = fontSize - 2;
                    } else if (rotation == -90 || rotation == 270) {
                        eff = "rotate up 90";
                        lorHeight = lorWidth - 2;
                        lorWidth = fontSize - 2;
                    }
                    if (Contains(fontName, "Vertical")) lorHeight += 4;
                } else {
                    if (Contains(fontName, "Vertical")) {
                        eff = "vert text down";
                        lorWidth = fontCellWidth;
                        lorHeight = static_cast<int>(text.size()) * fontSize;
                    } else if (rotation == 90) {
                        eff = "rotate down 90";
                        lorWidth = fontSize;
                        lorHeight = static_cast<int>(text.size()) * fontCellWidth;
                    } else if (rotation == 270 || rotation == -90) {
                        eff = "rotate up 90";
                        lorWidth = fontSize;
                        lorHeight = static_cast<int>(text.size()) * fontCellWidth;
                    }
                }

                xStart += static_cast<int>(std::round(static_cast<double>(lorWidth) / 2.0));
                xEnd += static_cast<int>(std::round(static_cast<double>(lorWidth) / 2.0));
                yStart += static_cast<int>(std::round(static_cast<double>(lorHeight) / 2.0));
                yEnd += static_cast<int>(std::round(static_cast<double>(lorHeight) / 2.0));

                yStart = -yStart + static_cast<int>(num_rows);
                yEnd = -yEnd + static_cast<int>(num_rows);

                xStart = CalcUnBoundedPercentage(xStart, static_cast<int>(num_columns)) - 50;
                xEnd = CalcUnBoundedPercentage(xEnd, static_cast<int>(num_columns)) - 50;
                yStart = CalcUnBoundedPercentage(yStart, static_cast<int>(num_rows)) - 50;
                yEnd = CalcUnBoundedPercentage(yEnd, static_cast<int>(num_rows)) - 50;

                std::string palette = "C_BUTTON_Palette1=" + static_cast<std::string>(color) +
                                      ",C_CHECKBOX_Palette1=1,C_CHECKBOX_Palette2=0,C_CHECKBOX_Palette3=0,C_CHECKBOX_Palette4=0,C_CHECKBOX_Palette5=0,C_CHECKBOX_Palette6=0,C_CHECKBOX_Palette7=0,C_CHECKBOX_Palette8=0";
                if (use_xl_font) {
                    if (colorType == "rainbow") {
                        const char* colors[] = { "#FF0000", "#FF8000", "#FFFF00", "#00FF00", "#0000FF", "#8000FF" };
                        palette = std::string("C_BUTTON_Palette1=") + colors[fCI];
                        palette += std::string(",C_BUTTON_Palette2=") + colors[(fCI + 1) % 6];
                        palette += std::string(",C_BUTTON_Palette3=") + colors[(fCI + 2) % 6];
                        palette += std::string(",C_BUTTON_Palette4=") + colors[(fCI + 3) % 6];
                        palette += std::string(",C_BUTTON_Palette5=") + colors[(fCI + 4) % 6];
                        palette += std::string(",C_BUTTON_Palette6=") + colors[(fCI + 5) % 6];
                        palette += ",C_CHECKBOX_Palette1=1,C_CHECKBOX_Palette2=1,C_CHECKBOX_Palette3=1,C_CHECKBOX_Palette4=1";
                        palette += ",C_CHECKBOX_Palette5=1,C_CHECKBOX_Palette6=1,C_CHECKBOX_Palette7=0,C_CHECKBOX_Palette8=0";
                    } else if (colorType == "redGreenBlue") {
                        const char* colors[] = { "#FF0000", "#00FF00", "#0000FF" };
                        palette = std::string("C_BUTTON_Palette1=") + colors[fCI];
                        palette += std::string(",C_BUTTON_Palette2=") + colors[(fCI + 1) % 3];
                        palette += std::string(",C_BUTTON_Palette3=") + colors[(fCI + 2) % 3];
                        palette += ",C_CHECKBOX_Palette1=1,C_CHECKBOX_Palette2=1,C_CHECKBOX_Palette3=1,C_CHECKBOX_Palette4=0";
                        palette += ",C_CHECKBOX_Palette5=0,C_CHECKBOX_Palette6=0,C_CHECKBOX_Palette7=0,C_CHECKBOX_Palette8=0";
                    }
                }
                std::string settings =
                    "E_CHECKBOX_TextToCenter=0,E_TEXTCTRL_Text=" + text +
                    ",E_TEXTCTRL_Text_Speed=26,E_CHOICE_Text_Count=none," +
                    "E_CHOICE_Text_Dir=vector,E_CHECKBOX_Text_PixelOffsets=0," +
                    "E_CHOICE_Text_Effect=" + eff + "," +
                    "E_FONTPICKER_Text_Font=" + font + "," +
                    "E_CHOICE_Text_Font=" + xl_font_name + "," +
                    "E_SLIDER_Text_XStart=" + std::to_string(xStart) + "," +
                    "E_SLIDER_Text_YStart=" + std::to_string(yStart) + "," +
                    "E_SLIDER_Text_XEnd=" + std::to_string(xEnd) + "," +
                    "E_SLIDER_Text_YEnd=" + std::to_string(yEnd);
                if (mask == "positiveMask") {
                    if (!_blendString.empty()) settings += _blendString;
                } else if (mask == "negativeMask") {
                    settings += ",T_CHOICE_LayerMethod=1 is Mask";
                } else {
                    settings += _blendString;
                }
                layer->AddEffect(0, "Text", settings, palette, start_time, end_time, false, false);
            }

        } else if (ename == "imageActions") {
            for (pugi::xml_node element = e.first_child(); element; element = element.next_sibling()) {
                if (std::string_view(element.name()) != "imageAction") continue;
                std::string name = element.attribute("name").as_string();
                int idx = element.attribute("imageIndex").as_int();
                int startms = element.attribute("startCentisecond").as_int() * 10;
                int endms = element.attribute("endCentisecond").as_int() * 10;
                int layer_index = element.attribute("layer").as_int();
                int rampDownTime = element.attribute("rampTime").as_int() * 10;
                int rampUpTime = element.attribute("preRampTime").as_int() * 10;
                while (static_cast<int>(_model->GetEffectLayerCount()) <= layer_index) {
                    _model->AddEffectLayer();
                }
                std::string rampUpTimeString = "0";
                if (rampUpTime) {
                    rampUpTimeString = fmt::format("{:f}", static_cast<double>(rampUpTime) / 1000.0);
                }
                std::string rampDownTimeString = "0";
                if (rampDownTime) {
                    rampDownTimeString = fmt::format("{:f}", static_cast<double>(rampDownTime) / 1000.0);
                }

                int startx = element.attribute("xStart").as_int();
                int starty = element.attribute("yStart").as_int();
                int endx = element.attribute("xEnd").as_int();
                int endy = element.attribute("yEnd").as_int();
                if (reverse_xy) {
                    std::swap(startx, starty);
                    std::swap(endx, endy);
                    starty = -starty;
                    endy = -endy;
                }
                ImageInfo& imgInfo = imageInfo[idx];
                int x = imgInfo.xOffset;
                int y = imgInfo.yOffset;

                layer = FindOpenLayer(_model, layer_index, startms, endms, reserved);
                if (endy == starty && endx == startx) {
                    x += static_cast<int>(std::round(static_cast<double>(startx) * imgInfo.scaleX));
                    y -= static_cast<int>(std::round(static_cast<double>(starty) * imgInfo.scaleY));
                    std::string settings =
                        "E_CHECKBOX_Pictures_WrapX=0,E_CHOICE_Pictures_Direction=none,"
                        "E_SLIDER_PicturesXC=" + std::to_string(x) +
                        ",E_SLIDER_PicturesYC=" + std::to_string(y) +
                        ",E_CHOICE_Scaling=No Scaling" +
                        ",X_Effect_Description=" + name +
                        ",E_SLIDER_Pictures_StartScale=100,E_SLIDER_Pictures_EndScale=100" +
                        ",E_CHECKBOX_Pictures_PixelOffsets=1" +
                        ",E_TEXTCTRL_Pictures_Filename=" + imgInfo.imageName +
                        ",E_TEXTCTRL_Pictures_Speed=1.0,E_TEXTCTRL_Pictures_FrameRateAdj=1.0";
                    if (rampUpTimeString != "0") settings += ",T_TEXTCTRL_Fadein=" + rampUpTimeString;
                    if (rampDownTimeString != "0") settings += ",T_TEXTCTRL_Fadeout=" + rampDownTimeString;
                    settings += _blendString;
                    layer->AddEffect(0, "Pictures", settings, "", startms, endms, false, false);
                } else {
                    int sx = x + static_cast<int>(std::round(static_cast<double>(startx) * imgInfo.scaleX));
                    int sy = y - static_cast<int>(std::round(static_cast<double>(starty) * imgInfo.scaleY));
                    int ex = x + static_cast<int>(std::round(static_cast<double>(endx) * imgInfo.scaleX));
                    int ey = y - static_cast<int>(std::round(static_cast<double>(endy) * imgInfo.scaleY));
                    std::string settings =
                        "E_CHECKBOX_Pictures_WrapX=0,E_CHOICE_Pictures_Direction=vector,"
                        "E_SLIDER_PicturesXC=" + std::to_string(sx) +
                        ",E_SLIDER_PicturesYC=" + std::to_string(sy) +
                        ",E_SLIDER_PicturesEndXC=" + std::to_string(ex) +
                        ",E_SLIDER_PicturesEndYC=" + std::to_string(ey) +
                        ",E_CHOICE_Scaling=No Scaling" +
                        ",X_Effect_Description=" + name +
                        ",E_SLIDER_Pictures_StartScale=100,E_SLIDER_Pictures_EndScale=100" +
                        ",E_TEXTCTRL_Pictures_Speed=1.0,E_TEXTCTRL_Pictures_FrameRateAdj=1.0" +
                        ",E_CHECKBOX_Pictures_PixelOffsets=1" +
                        ",E_TEXTCTRL_Pictures_Filename=" + imgInfo.imageName;
                    if (rampUpTimeString != "0") settings += ",T_TEXTCTRL_Fadein=" + rampUpTimeString;
                    if (rampDownTimeString != "0") settings += ",T_TEXTCTRL_Fadeout=" + rampDownTimeString;
                    settings += _blendString;
                    layer->AddEffect(0, "Pictures", settings, "", startms, endms, false, false);
                }
            }
        }
    }
    return true;
}

} // namespace SuperStar
