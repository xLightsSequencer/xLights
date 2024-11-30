/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include "PlayListItemMicrophone.h"
#include "PlayListItemMicrophonePanel.h"
#include "../../xLights/AudioManager.h"
#include "../../xLights/outputs/OutputManager.h"
#include <wx/notebook.h>
#include <wx/xml/xml.h>

PlayListItemMicrophone::PlayListItemMicrophone(OutputManager* outputManager, wxXmlNode* node) :
    PlayListItem(node) {
    _outputManager = outputManager;
    _sc = 0;
    _startChannel = "1";
    _pixels = 1;
    _duration = 60000;
    _colour = *wxWHITE;
    _mode = "Value 2";
    _device = "";
    _blendMode = APPLYMETHOD::METHOD_OVERWRITEIFBLACK;
    PlayListItemMicrophone::Load(node);
}

PlayListItemMicrophone::~PlayListItemMicrophone() {
}

void PlayListItemMicrophone::Load(wxXmlNode* node) {
    PlayListItem::Load(node);
    _mode = node->GetAttribute("Mode", "");
    _startChannel = node->GetAttribute("StartChannel", "1").ToStdString();
    _pixels = wxAtoi(node->GetAttribute("Pixels", "1"));
    _colour = wxColour(node->GetAttribute("Colour", "WHITE"));
    _duration = wxAtol(node->GetAttribute("Duration", "60000"));
    _blendMode = (APPLYMETHOD)wxAtoi(node->GetAttribute("ApplyMethod", "1"));
    _device = node->GetAttribute("Device", "");
}

PlayListItemMicrophone::PlayListItemMicrophone(OutputManager* outputManager) :
    PlayListItem() {
    _type = "PLIMicrophone";
    _outputManager = outputManager;
    _sc = 0;
    _startChannel = "1";
    _pixels = 1;
    _duration = 60000;
    _colour = *wxWHITE;
    _mode = "Value 2";
    _device = "";
    _blendMode = APPLYMETHOD::METHOD_OVERWRITEIFBLACK;
}

PlayListItem* PlayListItemMicrophone::Copy(const bool isClone) const {
    PlayListItemMicrophone* res = new PlayListItemMicrophone(_outputManager);
    res->_outputManager = _outputManager;
    res->_startChannel = _startChannel;
    res->_pixels = _pixels;
    res->_duration = _duration;
    res->_colour = _colour;
    res->_mode = _mode;
    res->_blendMode = _blendMode;
    res->_device = _device;
    PlayListItem::Copy(res, isClone);

    return res;
}

wxXmlNode* PlayListItemMicrophone::Save() {
    wxXmlNode* node = new wxXmlNode(nullptr, wxXML_ELEMENT_NODE, GetType());

    node->AddAttribute("Mode", _mode);
    node->AddAttribute("StartChannel", _startChannel);
    node->AddAttribute("Pixels", wxString::Format(wxT("%i"), (long)_pixels));
    node->AddAttribute("Colour", _colour.GetAsString());
    node->AddAttribute("Device", _device);
    node->AddAttribute("Duration", wxString::Format(wxT("%i"), (long)_duration));
    node->AddAttribute("ApplyMethod", wxString::Format(wxT("%i"), (int)_blendMode));

    PlayListItem::Save(node);

    return node;
}

void PlayListItemMicrophone::Configure(wxNotebook* notebook) {
    notebook->AddPage(new PlayListItemMicrophonePanel(notebook, _outputManager, this), GetTitle(), true);
}

size_t PlayListItemMicrophone::GetStartChannelAsNumber() {
    if (_sc == 0) {
        _sc = _outputManager->DecodeStartChannel(_startChannel);
    }

    return _sc;
}

std::string PlayListItemMicrophone::GetTitle() const {
    return "Microphone";
}

std::string PlayListItemMicrophone::GetNameNoTime() const {
    if (_name != "")
        return _name;

    return _mode;
}

void PlayListItemMicrophone::Frame(uint8_t* buffer, size_t size, size_t ms, size_t framems, bool outputframe) {
    static int lastValue = 0;
    if (outputframe) {
        long sc = GetStartChannelAsNumber();
        int toset = std::min(_pixels * 3, size - ((size_t)sc - 1));

        int value = 0;

        if (_mode == "Maximum") {
            auto sdl = AudioManager::GetSDLManager()->GetInputSDL(_device);
            if (sdl != nullptr) {
                value = sdl->GetMax(framems);
            }
        }

        if (value == -1)
            value = lastValue;
        lastValue = value;

        wxColour c(_colour.Red() * value / 255, _colour.Green() * value / 255, _colour.Blue() * value / 255);

        // wxASSERT(c.Red() <= _colour.Red());
        // wxASSERT(c.Green() <= _colour.Green());
        // wxASSERT(c.Blue() <= _colour.Blue());

        for (uint8_t* p = buffer + sc - 1; p < buffer + sc - 1 + toset; p += 3) {
            SetPixel(p, c.Red(), c.Green(), c.Blue(), _blendMode);
        }
    }
}

void PlayListItemMicrophone::Start(long stepLengthMS) {
    PlayListItem::Start(stepLengthMS);
    auto sdl = AudioManager::GetSDLManager()->GetInputSDL(_device);
    if (sdl != nullptr) {
        sdl->StartListening();
        sdl->PurgeInput();
    }
}

void PlayListItemMicrophone::Stop() {
    auto sdl = AudioManager::GetSDLManager()->GetInputSDL(_device);
    if (sdl != nullptr) {
        sdl->StopListening();
    }
}

void PlayListItemMicrophone::SetPixel(uint8_t* p, uint8_t r, uint8_t g, uint8_t b, APPLYMETHOD blendMode) {
    uint8_t rgb[3];
    rgb[0] = r;
    rgb[1] = g;
    rgb[2] = b;

    Blend(p, 3, rgb, 3, blendMode);
}
