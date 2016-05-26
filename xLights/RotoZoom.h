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
    float _zoommaximum;
    int _xcenter;
    int _ycenter;

    void SetSerialisedValue(std::string k, std::string s);

public:
    float GetRotations() { return _rotations; }
    float GetZooms() { return _zooms; }
    float GetZoomMaximum() { return _zoommaximum; }
    float GetXCenter() { return _xcenter; }
    float GetYCenter() { return _ycenter; }
    RotoZoomParms() { RotoZoomParms(""); };
    RotoZoomParms(const std::string& id, float rotations = 1.0f, float zooms = 1.0f, float zoommaximum = 2.0f, int x=1, int y=1);
    void ApplySettings(float rotations, float zooms, float zoommaximum, int x, int y);
    void SetDefault(wxSize size);
    std::string Serialise();
    bool IsOk() { return _id != ""; }
    void Deserialise(std::string s);
    wxPoint GetTransform(int x, int y, float offset);
    void SetActive(bool a) { _active = a; }
    bool IsActive() { return IsOk() && _active; }
    void ToggleActive() { _active = !_active; }
};

#endif