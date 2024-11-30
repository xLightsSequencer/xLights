/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include "VirtualMatrix.h"
#include "RGBEffects.h"
#include "ScheduleOptions.h"
#include "xScheduleApp.h"
#include "../xLights/outputs/OutputManager.h"
#include <wx/string.h>
#include <wx/wx.h>
#include <wx/xml/xml.h>

extern "C" {
#include <libswscale/swscale.h>
}

#include <log4cpp/Category.hh>

VirtualMatrix::VirtualMatrix(OutputManager* outputManager, int width, int height, bool topMost, VMROTATION rotation, VMPIXELCHANNELS pixelChannels, wxImageResizeQuality quality, int swsQuality, const std::string& startChannel, const std::string& name, wxSize size, wxPoint loc, bool useMatrixSize, int matrixMultiplier, const std::string& fromModel) {
    _suppress = false;
    _outputManager = outputManager;
    _lastSavedChangeCount = 0;
    _changeCount = 1;
    _name = name;
    _width = width;
    _height = height;
    _topMost = topMost;
    _useMatrixSize = useMatrixSize;
    _matrixMultiplier = matrixMultiplier;
    _rotation = rotation;
    _pixelChannels = pixelChannels;
    _quality = quality;
    _swsQuality = swsQuality;
    _size = size;
    _location = loc;
    _startChannel = startChannel;
    _fromModel = fromModel;
    _window = nullptr;
}

VirtualMatrix::VirtualMatrix(OutputManager* outputManager, ScheduleOptions* options) {
    _suppress = false;
    _outputManager = outputManager;
    _lastSavedChangeCount = 0;
    _changeCount = 1;
    _name = "";
    _width = 32;
    _height = 16;
    _topMost = true;
    _useMatrixSize = false;
    _matrixMultiplier = 1;
    _rotation = VMROTATION::VM_NORMAL;
    _pixelChannels = VMPIXELCHANNELS::RGB;
    _quality = wxIMAGE_QUALITY_HIGH;
    _swsQuality = -1;
    _size = options->GetDefaultVideoSize();
    _location = options->GetDefaultVideoPos();
    _startChannel = "1";
    _fromModel = "";
    _window = nullptr;
}

VirtualMatrix::VirtualMatrix(OutputManager* outputManager, int width, int height, bool topMost, const std::string& rotation, const std::string& pixelChannels, const std::string& quality, const std::string& startChannel, const std::string& name, wxSize size, wxPoint loc, bool useMatrixSize, int matrixMultiplier, const std::string& fromModel) {
    _suppress = false;
    _outputManager = outputManager;
    _lastSavedChangeCount = 0;
    _changeCount = 1;
    _name = name;
    _width = width;
    _height = height;
    _topMost = topMost;
    _useMatrixSize = useMatrixSize;
    _matrixMultiplier = matrixMultiplier;
    _rotation = EncodeRotation(rotation);
    _pixelChannels = EncodePixelChannels(pixelChannels);
    _quality = EncodeScalingQuality(quality, _swsQuality);
    _size = size;
    _location = loc;
    _startChannel = startChannel;
    _fromModel = fromModel;
    _window = nullptr;
}

VirtualMatrix::VirtualMatrix(OutputManager* outputManager, wxXmlNode* n) {
    _suppress = false;
    _outputManager = outputManager;
    _lastSavedChangeCount = 0;
    _changeCount = 0;
    _name = n->GetAttribute("Name", "");
    _width = wxAtoi(n->GetAttribute("Width", "32"));
    _height = wxAtoi(n->GetAttribute("Height", "16"));
    _topMost = (n->GetAttribute("TopMost", "TRUE") == "TRUE");
    _useMatrixSize = (n->GetAttribute("UseMatrixSize", "FALSE") == "TRUE");
    _rotation = EncodeRotation(n->GetAttribute("Rotation", "None").ToStdString());
    _pixelChannels = EncodePixelChannels(n->GetAttribute("PixelChannels", "RGB").ToStdString());
    _quality = EncodeScalingQuality(n->GetAttribute("Quality", "Bilinear").ToStdString(), _swsQuality);
    _size = wxSize(wxAtoi(n->GetAttribute("WW", "300")), wxAtoi(n->GetAttribute("WH", "300")));
    _matrixMultiplier = wxAtoi(n->GetAttribute("MatrixMultiplier", "1"));
    _location = wxPoint(wxAtoi(n->GetAttribute("X", "0")), wxAtoi(n->GetAttribute("Y", "0")));
    _startChannel = n->GetAttribute("StartChannel", "1");
    _fromModel = n->GetAttribute("FromModel", "");
    _window = nullptr;
}

wxXmlNode* VirtualMatrix::Save() {
    wxXmlNode* res = new wxXmlNode(nullptr, wxXML_ELEMENT_NODE, "VMatrix");

    res->AddAttribute("Name", _name);
    res->AddAttribute("Width", wxString::Format(wxT("%i"), (long)_width));
    res->AddAttribute("Height", wxString::Format(wxT("%i"), (long)_height));
    if (!_topMost) {
        res->AddAttribute("TopMost", "FALSE");
    }
    if (_useMatrixSize) {
        res->AddAttribute("UseMatrixSize", "TRUE");
    }
    res->AddAttribute("Rotation", DecodeRotation(_rotation));
    res->AddAttribute("PixelChannels", DecodePixelChannels(_pixelChannels));
    res->AddAttribute("Quality", DecodeScalingQuality(_quality, _swsQuality));
    res->AddAttribute("WW", wxString::Format(wxT("%i"), (long)_size.GetWidth()));
    res->AddAttribute("WH", wxString::Format(wxT("%i"), (long)_size.GetHeight()));
    res->AddAttribute("X", wxString::Format(wxT("%i"), (long)_location.x));
    res->AddAttribute("Y", wxString::Format(wxT("%i"), (long)_location.y));
    res->AddAttribute("MatrixMultiplier", wxString::Format(wxT("%d"), _matrixMultiplier));
    res->AddAttribute("StartChannel", _startChannel);
    res->AddAttribute("FromModel", _fromModel);
    return res;
}

VMROTATION VirtualMatrix::EncodeRotation(const std::string rotation) {
    if (wxString(rotation).Lower() == "none") {
        return VMROTATION::VM_NORMAL;
    } else if (wxString(rotation).Lower() == "90 cw") {
        return VMROTATION::VM_90;
    } else if (wxString(rotation).Lower() == "flip vertical") {
        return VMROTATION::VM_FLIP_VERTICAL;
    } else if (wxString(rotation).Lower() == "flip horizontal") {
        return VMROTATION::VM_FLIP_HORIZONTAL;
    } else {
        return VMROTATION::VM_270;
    }
}

std::string VirtualMatrix::DecodeRotation(VMROTATION rotation) {
    if (rotation == VMROTATION::VM_NORMAL) {
        return "None";
    } else if (rotation == VMROTATION::VM_90) {
        return "90 CW";
    } else if (rotation == VMROTATION::VM_FLIP_HORIZONTAL) {
        return "Flip Horizontal";
    } else if (rotation == VMROTATION::VM_FLIP_VERTICAL) {
        return "Flip Vertical";
    } else {
        return "90 CCW";
    }
}

VMPIXELCHANNELS VirtualMatrix::EncodePixelChannels(const std::string rotation) {
    if (wxString(rotation).Lower() == "rgbw") {
        return VMPIXELCHANNELS::RGBW;
    } else {
        return VMPIXELCHANNELS::RGB;
    }
}

std::string VirtualMatrix::DecodePixelChannels(VMPIXELCHANNELS pixelChannels) {
    if (pixelChannels == VMPIXELCHANNELS::RGBW) {
        return "RGBW";
    } else {
        return "RGB";
    }
}

wxImageResizeQuality VirtualMatrix::EncodeScalingQuality(const std::string quality, int& swsQuality) {
    swsQuality = -1;
    if (wxString(quality).Lower() == "normal") {
        return wxIMAGE_QUALITY_NORMAL;
    } else if (wxString(quality).Lower() == "bicubic") {
        return wxIMAGE_QUALITY_BICUBIC;
    } else if (wxString(quality).Lower() == "bilinear") {
        return wxIMAGE_QUALITY_BILINEAR;
    } else if (wxString(quality).Lower() == "box average") {
        return wxIMAGE_QUALITY_BOX_AVERAGE;
    } else if (wxString(quality).Lower() == "high") {
        return wxIMAGE_QUALITY_HIGH;
    } else if (wxString(quality).Lower() == "fast bilinear") {
        swsQuality = SWS_FAST_BILINEAR;
        return wxIMAGE_QUALITY_HIGH;
    } else if (wxString(quality).Lower() == "sws bilinear") {
        swsQuality = SWS_BILINEAR;
        return wxIMAGE_QUALITY_HIGH;
    } else if (wxString(quality).Lower() == "sws bicubic") {
        swsQuality = SWS_BICUBIC;
        return wxIMAGE_QUALITY_HIGH;
    } else if (wxString(quality).Lower() == "x") {
        swsQuality = SWS_X;
        return wxIMAGE_QUALITY_HIGH;
    } else if (wxString(quality).Lower() == "point") {
        swsQuality = SWS_POINT;
        return wxIMAGE_QUALITY_HIGH;
    } else if (wxString(quality).Lower() == "area") {
        swsQuality = SWS_AREA;
        return wxIMAGE_QUALITY_HIGH;
    } else if (wxString(quality).Lower() == "bicublin") {
        swsQuality = SWS_BICUBLIN;
        return wxIMAGE_QUALITY_HIGH;
    } else if (wxString(quality).Lower() == "gauss") {
        swsQuality = SWS_GAUSS;
        return wxIMAGE_QUALITY_HIGH;
    } else if (wxString(quality).Lower() == "sinc") {
        swsQuality = SWS_SINC;
        return wxIMAGE_QUALITY_HIGH;
    } else if (wxString(quality).Lower() == "lanczos") {
        swsQuality = SWS_LANCZOS;
        return wxIMAGE_QUALITY_HIGH;
    } else if (wxString(quality).Lower() == "spline") {
        swsQuality = SWS_SPLINE;
        return wxIMAGE_QUALITY_HIGH;
    }

    return wxIMAGE_QUALITY_NORMAL;
}

std::string VirtualMatrix::DecodeScalingQuality(wxImageResizeQuality quality, int swsQuality) {
    if (swsQuality < 0) {
        switch (quality) {
        case wxIMAGE_QUALITY_NORMAL:
            return "Normal";
        case wxIMAGE_QUALITY_BICUBIC:
            return "Bicubic";
        case wxIMAGE_QUALITY_BILINEAR:
            return "Bilinear";
        case wxIMAGE_QUALITY_BOX_AVERAGE:
            return "Box Average";
        case wxIMAGE_QUALITY_HIGH:
            return "High";
        default:
            break;
        }
    } else {
        switch (swsQuality) {
        case SWS_FAST_BILINEAR:
            return "Fast Bilinear";
        case SWS_BILINEAR:
            return "SWS Bilinear";
        case SWS_BICUBIC:
            return "SWS Bicubic";
        case SWS_X:
            return "X";
        case SWS_POINT:
            return "Point";
        case SWS_AREA:
            return "Area";
        case SWS_BICUBLIN:
            return "Bicublin";
        case SWS_GAUSS:
            return "Gauss";
        case SWS_SINC:
            return "Sinc";
        case SWS_LANCZOS:
            return "Lanczos";
        case SWS_SPLINE:
            return "Spline";
        default:
            break;
        }
    }
    return "Normal";
}

void VirtualMatrix::AllOff() {
    if (!_image.IsOk())
        return;
    if (_window == nullptr)
        return;

    _image.Clear();

    if (_rotation == VMROTATION::VM_NORMAL) {
        _window->SetImage(_image);
    } else if (_rotation == VMROTATION::VM_FLIP_HORIZONTAL) {
        wxImage rot = _image.Mirror(true);
        _window->SetImage(rot);
    } else if (_rotation == VMROTATION::VM_FLIP_VERTICAL) {
        wxImage rot = _image.Mirror(false);
        _window->SetImage(rot);
    } else if (_rotation == VMROTATION::VM_90) {
        wxImage rot = _image.Rotate90();
        _window->SetImage(rot);
    } else {
        wxImage rot = _image.Rotate90(false);
        _window->SetImage(rot);
    }
}

void VirtualMatrix::Frame(uint8_t* buffer, size_t size) {
    if (!_image.IsOk())
        return;
    if (_window == nullptr)
        return;

    // If there is no width or height there is nothing to draw
    if (_width == 0 || _height == 0)
        return;

    long sc = _outputManager->DecodeStartChannel(_startChannel);

    if (_fromModel == "") {
        const int channelsPerPixel = GetPixelChannelsCount();
        size_t end = _width * _height * channelsPerPixel < size - (sc - 1) ? _width * _height * channelsPerPixel : size - (sc - 1);

        for (size_t i = 0; i < end; i += channelsPerPixel) {
            uint8_t* pb = buffer + (sc - 1) + i;
            uint8_t r = *pb;
            uint8_t g = 0;
            if (i + 1 < end) {
                g = *(pb + 1);
            }
            uint8_t b = 0;
            if (i + 2 < end) {
                b = *(pb + 2);
            }
            if (channelsPerPixel > 3) {
                uint8_t w = 0;
                if (i + 3 < end) {
                    w = *(pb + 3);
                }
                if (w != 0) {
                    r = g = b = w;
                }
            }
            _image.SetRGB((i / channelsPerPixel) % _width, i / channelsPerPixel / _width, r, g, b);
        }
    } else {
        const int channelsPerPixel = 3; // this only supports RGB pixels

        if (_displayAs == "Custom") {
            long y = 0;
            for (const auto& row : _customModelData[0]) {
                long x = 0;
                for (const auto& col : row) {
                    if (col >= 0) {
                        long ch = sc - 1 + ((col - 1) * channelsPerPixel);
                        _image.SetRGB(x, y, *(buffer + ch), *(buffer + ch + 1), *(buffer + ch + 2));
                    }
                    ++x;
                }
                ++y;
            }
        } else {
            long ch = sc - 1;
            long nodesPerStrand = _nodes / _strandsPerString;
            for (long n = 0; n < _strings * _nodes; ++n) {
                long string = n / _nodes;
                long strand = (n % _nodes) / nodesPerStrand;
                long nodeInStrand = (n % _nodes) % nodesPerStrand;
                long x = 0;
                long y = 0;

                if (_startLocation == MMSTARTLOCATION::TOP_LEFT) {
                    if (_orientation == MMORIENTATION::HORIZONTAL) {
                        // y = _height - (string * _strandsPerString + strand) - 1;
                        y = string * _strandsPerString + strand;
                        if (strand % 2 == 0) {
                            x = nodeInStrand;
                        } else {
                            x = nodesPerStrand - nodeInStrand - 1;
                        }
                    } else {
                        x = string * _strandsPerString + strand;
                        if (strand % 2 == 0) {
                            y = nodeInStrand;
                        } else {
                            y = nodesPerStrand - nodeInStrand - 1;
                        }
                    }
                } else if (_startLocation == MMSTARTLOCATION::TOP_RIGHT) {
                    if (_orientation == MMORIENTATION::HORIZONTAL) {
                        y = string * _strandsPerString + strand;
                        if (strand % 2 == 0) {
                            x = nodesPerStrand - nodeInStrand - 1;
                        } else {
                            x = nodeInStrand;
                        }
                    } else {
                        // This is drawing wrong
                        x = _width - (string * _strandsPerString + strand) - 1;
                        // x = string * _strandsPerString + strand;
                        if (strand % 2 == 0) {
                            y = nodeInStrand;
                        } else {
                            y = nodesPerStrand - nodeInStrand - 1;
                        }
                    }
                } else if (_startLocation == MMSTARTLOCATION::BOTTOM_LEFT) {
                    if (_orientation == MMORIENTATION::HORIZONTAL) {
                        y = _height - (string * _strandsPerString + strand) - 1;
                        if (strand % 2 == 0) {
                            x = nodeInStrand;
                        } else {
                            x = nodesPerStrand - nodeInStrand - 1;
                        }
                    } else {
                        // This is drawing wrong
                        // x = _width - (string * _strandsPerString + strand) - 1;
                        x = string * _strandsPerString + strand;
                        if (strand % 2 == 0) {
                            y = _height - nodeInStrand - 1;
                        } else {
                            y = _height - nodesPerStrand + nodeInStrand;
                        }
                    }
                } else if (_startLocation == MMSTARTLOCATION::BOTTOM_RIGHT) {
                    if (_orientation == MMORIENTATION::HORIZONTAL) {
                        y = _height - (string * _strandsPerString + strand) - 1;
                        if (strand % 2 == 0) {
                            x = nodesPerStrand - nodeInStrand - 1;
                        } else {
                            x = nodeInStrand;
                        }
                    } else {
                        x = _width - (string * _strandsPerString + strand) - 1;
                        if (strand % 2 == 0) {
                            y = _height - nodeInStrand - 1;
                        } else {
                            y = _height - nodesPerStrand + nodeInStrand;
                        }
                    }
                }

                _image.SetRGB(x, y, *(buffer + ch), *(buffer + ch + 1), *(buffer + ch + 2));
                ch += channelsPerPixel;
            }
        }
    }

    if (_rotation == VMROTATION::VM_NORMAL) {
        _window->SetImage(_image);
    } else if (_rotation == VMROTATION::VM_FLIP_HORIZONTAL) {
        wxImage rot = _image.Mirror(true);
        _window->SetImage(rot);
    } else if (_rotation == VMROTATION::VM_FLIP_VERTICAL) {
        wxImage rot = _image.Mirror(false);
        _window->SetImage(rot);
    } else if (_rotation == VMROTATION::VM_90) {
        wxImage rot = _image.Rotate90();
        _window->SetImage(rot);
    } else {
        wxImage rot = _image.Rotate90(false);
        _window->SetImage(rot);
    }
}

void VirtualMatrix::Start() {
    static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    logger_base.debug("Virtual matrix started %s.", (const char*)_name.c_str());

    if (_fromModel != "") {
        _rotation = VMROTATION::VM_NORMAL;
        RGBEffects effects;
        _modelNode = new wxXmlNode(*effects.GetModel(_fromModel));

        if (_modelNode != nullptr) {
            _displayAs = _modelNode->GetAttribute("DisplayAs");
            if (_displayAs == "Custom") {
                _width = wxAtoi(_modelNode->GetAttribute("parm1", "0"));
                _height = wxAtoi(_modelNode->GetAttribute("parm2", "0"));
                _customModelData = MatrixMapper::ParseCustomModelDataFromXml(_modelNode);
            } else {
                _strings = wxAtol(_modelNode->GetAttribute("parm1", "0"));
                _nodes = wxAtol(_modelNode->GetAttribute("parm2", "0"));
                _strandsPerString = wxAtol(_modelNode->GetAttribute("parm3", "1"));
                if (_displayAs == "Horiz Matrix") {
                    _width = _nodes / _strandsPerString;
                    _height = _strings * _strandsPerString;
                    _orientation = MMORIENTATION::HORIZONTAL;
                } else if (_displayAs == "Vert Matrix") {
                    _height = _nodes / _strandsPerString;
                    _width = _strings * _strandsPerString;
                    _orientation = MMORIENTATION::VERTICAL;
                }
                std::string startSide = _modelNode->GetAttribute("StartSide", "B");
                std::string dir = _modelNode->GetAttribute("Dir", "L");
                if (startSide == "B" && dir == "L") {
                    _startLocation = MMSTARTLOCATION::BOTTOM_LEFT;
                } else if (startSide == "B" && dir == "R") {
                    _startLocation = MMSTARTLOCATION::BOTTOM_RIGHT;
                } else if (startSide == "T" && dir == "L") {
                    _startLocation = MMSTARTLOCATION::TOP_LEFT;
                } else {
                    _startLocation = MMSTARTLOCATION::TOP_RIGHT;
                }
            }
            _startChannel = _modelNode->GetAttribute("StartChannel", "1");
        } else {
            _width = 0;
            _height = 0;
        }
    }

    if (_useMatrixSize) {
        _size = wxSize(_width * _matrixMultiplier, _height * _matrixMultiplier);
    }

    // create the window
    if (_window == nullptr) {
        _window = new PlayerWindow(wxGetApp().GetTopWindow(), _topMost, _quality, _swsQuality, wxID_ANY, _location, _size);
    } else {
        _window->Move(_location);
        _window->SetSize(_size);
    }

    if (_suppress) {
        _window->Hide();
    }

    _image = wxImage(_width, _height);
}

void VirtualMatrix::Stop() {
    static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    logger_base.debug("Virtual matrix stopped %s.", (const char*)_name.c_str());

    // destroy the window
    if (_window != nullptr) {
        _window->Close();
        delete _window;
        _window = nullptr;
    }

    if (_fromModel != "") {
        if (_modelNode != nullptr) {
            delete _modelNode;
            _modelNode = nullptr;
        }
    }
}

void VirtualMatrix::Suppress(bool suppress) {
    _suppress = suppress;

    if (_window != nullptr) {
        if (suppress) {
            _window->Hide();
        } else {
            _window->Show();
        }
    }
}

long VirtualMatrix::GetStartChannelAsNumber() const {
    return _outputManager->DecodeStartChannel(_startChannel);
}

int VirtualMatrix::GetPixelChannelsCount() const {
    return (_pixelChannels == VMPIXELCHANNELS::RGBW) ? 4 : 3;
}
