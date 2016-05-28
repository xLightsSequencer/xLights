#ifndef ROTOZOOM_H
#define ROTOZOOM_H

#include <wx/position.h>
#include <string>
#include <list>

class RotoZoomParms
{
    std::string _id;
    bool _active;
    float _rotations;
    float _zooms;
    int _start;
    float _zoomminimum;
    float _zoommaximum;
    int _quality;
    int _xcenter;
    int _ycenter;

    void SetSerialisedValue(std::string k, std::string s);

public:
    float GetRotations() { return _rotations; }
    float GetZooms() { return _zooms; }
    int GetStart() { return _start; }
    float GetZoomMinimum() { return _zoomminimum; }
    float GetZoomMaximum() { return _zoommaximum; }
    float GetQuality() { return _quality; }
    float GetXCenter() { return _xcenter; }
    float GetYCenter() { return _ycenter; }
    void SetRotations(int rotations) { _rotations = rotations; }
    void SetZooms(int zooms) { _zooms = zooms; }
    void SetStart(int start) { _start = start; }
    void SetZoomMinimum(int zoomminimum) { _zoomminimum = zoomminimum; }
    void SetZoomMaximum(int zoommaximum) { _zoommaximum = zoommaximum; }
    void SetQuality(int quality) { _quality = quality; }
    void SetXCenter(int xcenter) { _xcenter = xcenter; }
    void SetYCenter(int ycenter) { _ycenter = ycenter; }
    RotoZoomParms() { RotoZoomParms(""); };
    RotoZoomParms(const std::string& id, float rotations = 10.0f, float zooms = 10.0f, int start = 0, float zoomminimum = 1.0f, float zoommaximum = 20.0f, int quality = 1, int x=50, int y=50);
    void ApplySettings(float rotations, float zooms, int start, float zoomminimum, float zoommaximum, int quality, int x, int y);
    void SetDefault(wxSize size);
    std::string Serialise();
    bool IsOk() { return _id != ""; }
    void Deserialise(std::string s);
    wxPoint GetTransform(float x, float y, float offset, wxSize size);
    void SetActive(bool a) { _active = a; }
    bool IsActive() { return IsOk() && _active; }
    void ToggleActive() { _active = !_active; }
};

#endif