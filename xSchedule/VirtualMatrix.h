#ifndef VIRTUALMATRIX_H
#define VIRTUALMATRIX_H

#include <string>
#include <wx/wx.h>
#include "PlayList/PlayerWindow.h"

class wxXmlNode;
class OutputManager;

typedef enum {VM_NORMAL, VM_90, VM_270 } VMROTATION;

class VirtualMatrix 
{
    OutputManager* _outputManager;
    int _changeCount;
    int _lastSavedChangeCount;
    std::string _name;
	size_t _width;
	size_t _height;
    bool _topMost;
    wxSize _size;
    wxPoint _location;
    VMROTATION _rotation;
    std::string _startChannel;
    wxImage* _image;
    wxImageResizeQuality _quality;
    PlayerWindow* _window;
    bool _suppress;

public:

		static VMROTATION EncodeRotation(const std::string rotation);
		static std::string DecodeRotation(VMROTATION rotation);
        static wxImageResizeQuality EncodeScalingQuality(const std::string quality);
        static std::string DecodeScalingQuality(wxImageResizeQuality quality);

        VirtualMatrix(OutputManager* outputManager, wxXmlNode* n);
        VirtualMatrix(OutputManager* outputManager, int width, int height, bool topMost, VMROTATION rotation, wxImageResizeQuality quality, const std::string& startChannel, const std::string& name, wxSize size, wxPoint loc);
        VirtualMatrix(OutputManager* outputManager, int width, int height, bool topMost, const std::string& rotation, const std::string& quality, const std::string& startChannel, const std::string& name, wxSize size, wxPoint loc);
        VirtualMatrix(OutputManager* outputManager);
        virtual ~VirtualMatrix() {}
        void Frame(wxByte*buffer, size_t size);
        void Start();
        void Stop();
        void Suppress(bool suppress);
        std::string GetStartChannel() const { return _startChannel; }
        long GetStartChannelAsNumber() const;
        size_t GetChannels() const { return _width * _height * 3; }
        void SetStartChannel(const std::string& startChannel) { if (startChannel != _startChannel) { _startChannel = startChannel; _changeCount++; } }
        std::string GetName() const { return _name; }
        void SetName(const std::string& name) { if (name != _name) { _name = name; _changeCount++; } }
        size_t GetWidth() const { return _width; }
        size_t GetHeight() const { return _height; }
        wxSize GetSize() const { return _size; }
        wxPoint GetLocation() const { return _location; }
        bool GetTopMost() const { return _topMost; }
        std::string GetRotation() const { return DecodeRotation(_rotation); }
        std::string GetScalingQuality() const { return DecodeScalingQuality(_quality); }
        bool IsDirty() const { return _lastSavedChangeCount != _changeCount; }
        void ClearDirty() { _lastSavedChangeCount = _changeCount; }
        wxXmlNode* Save();
        void SetWidth(const size_t width) { if (width != _width) { _width = width; _changeCount++; } }
        void SetHeight(const size_t height) { if (height != _height) { _height = height; _changeCount++; } }
        void SetTopMost(const bool topMost) { if (topMost != _topMost) { _topMost = topMost; _changeCount++; } }
        void SetScalingQuality(const wxImageResizeQuality quality) { if (quality != _quality) { _quality = quality; _changeCount++; } }
        void SetRotation(const VMROTATION rotation) { if (rotation != _rotation) { _rotation = rotation; _changeCount++; } }
        void SetLocation(const wxPoint location) { if (location != _location) { _location = location; _changeCount++; } }
        void SetSize(const wxSize size) { if (size != _size) { _size = size; _changeCount++; } }
};

#endif
