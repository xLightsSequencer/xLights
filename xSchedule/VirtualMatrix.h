#pragma once

/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/smeighan/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/smeighan/xLights/blob/master/License.txt
 **************************************************************/

#include <string>
#include <wx/wx.h>
#include "PlayList/PlayerWindow.h"

class wxXmlNode;
class OutputManager;
class ScheduleOptions;

typedef enum {VM_NORMAL, VM_90, VM_270, VM_FLIP_HORIZONTAL, VM_FLIP_VERTICAL } VMROTATION;
typedef enum {RGB, RGBW } VMPIXELCHANNELS;

class VirtualMatrix 
{
    OutputManager* _outputManager;
    int _changeCount;
    int _lastSavedChangeCount;
    std::string _name;
	size_t _width;
	size_t _height;
    bool _topMost;
    bool _useMatrixSize;
    int _matrixMultiplier;
    wxSize _size;
    wxPoint _location;
    VMROTATION _rotation;
    VMPIXELCHANNELS _pixelChannels;
    std::string _startChannel;
    wxImage _image;
    wxImageResizeQuality _quality;
    int _swsQuality;
    PlayerWindow* _window;
    bool _suppress;

public:

		static VMROTATION EncodeRotation(const std::string rotation);
		static std::string DecodeRotation(VMROTATION rotation);
        static VMPIXELCHANNELS EncodePixelChannels(const std::string pixelChannels);
        static std::string DecodePixelChannels(VMPIXELCHANNELS pixelChannels);
        static wxImageResizeQuality EncodeScalingQuality(const std::string quality, int& swsQuality);
        static std::string DecodeScalingQuality(wxImageResizeQuality quality, int swsQuality);

        VirtualMatrix(OutputManager* outputManager, wxXmlNode* n);
        VirtualMatrix(OutputManager* outputManager, int width, int height, bool topMost, VMROTATION rotation, VMPIXELCHANNELS pixelChannels, wxImageResizeQuality quality, int swsQuality, const std::string& startChannel, const std::string& name, wxSize size, wxPoint loc, bool useMatrixSize, int matrixMultiplier);
        VirtualMatrix(OutputManager* outputManager, int width, int height, bool topMost, const std::string& rotation, const std::string& pixelChannels, const std::string& quality, const std::string& startChannel, const std::string& name, wxSize size, wxPoint loc, bool useMatrixSize, int matrixMultiplier);
        VirtualMatrix(OutputManager* outputManager, ScheduleOptions* options);
        virtual ~VirtualMatrix() {}
        void Frame(uint8_t*buffer, size_t size);
        void AllOff();
        void Start();
        void Stop();
        void Suppress(bool suppress);
        std::string GetStartChannel() const { return _startChannel; }
        long GetStartChannelAsNumber() const;
        int GetPixelChannelsCount() const;
        size_t GetChannels() const { return _width * _height * GetPixelChannelsCount(); }
        void SetStartChannel(const std::string& startChannel) { if (startChannel != _startChannel) { _startChannel = startChannel; _changeCount++; } }
        std::string GetName() const { return _name; }
        void SetName(const std::string& name) { if (name != _name) { _name = name; _changeCount++; } }
        size_t GetWidth() const { return _width; }
        size_t GetHeight() const { return _height; }
        wxSize GetSize() const { return _size; }
        wxPoint GetLocation() const { return _location; }
        bool GetTopMost() const { return _topMost; }
        bool GetUseMatrixSize() const { return _useMatrixSize; }
        int GetMatrixMultiplier() const { return _matrixMultiplier; }
        std::string GetRotation() const { return DecodeRotation(_rotation); }
        std::string GetPixelChannels() const { return DecodePixelChannels(_pixelChannels); }
        std::string GetScalingQuality() const { return DecodeScalingQuality(_quality, _swsQuality); }
        bool IsDirty() const { return _lastSavedChangeCount != _changeCount; }
        void ClearDirty() { _lastSavedChangeCount = _changeCount; }
        wxXmlNode* Save();
        void SetWidth(const size_t width) { if (width != _width) { _width = width; _changeCount++; } }
        void SetHeight(const size_t height) { if (height != _height) { _height = height; _changeCount++; } }
        void SetTopMost(const bool topMost) { if (topMost != _topMost) { _topMost = topMost; _changeCount++; } }
        void SetUseMatrixSize(const bool useMatrixSize) { if (useMatrixSize != _useMatrixSize) { _useMatrixSize = useMatrixSize; _changeCount++; } }
        void SetMatrixMultiplier(const int matrixMultiplier) { if (matrixMultiplier != _matrixMultiplier) { _matrixMultiplier = matrixMultiplier; _changeCount++; } }
        void SetScalingQuality(const wxImageResizeQuality quality, int swsQuality) { if (quality != _quality || swsQuality != _swsQuality) { _quality = quality; _swsQuality = swsQuality; _changeCount++; } }
        void SetRotation(const VMROTATION rotation) { if (rotation != _rotation) { _rotation = rotation; _changeCount++; } }
        void SetLocation(const wxPoint location) { if (location != _location) { _location = location; _changeCount++; } }
        void SetSize(const wxSize size) { if (size != _size) { _size = size; _changeCount++; } }
};

