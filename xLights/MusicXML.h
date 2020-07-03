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

#include <wx/xml/xml.h>
#include <string>
#include <list>
#include <map>

class MusicXmlNote
{
public:
    MusicXmlNote(wxXmlNode* node, int s);
    int midi;
    int start;
    int startMS;
    int duration;
    int durationMS;
    bool IsRest() { return (midi == -1); }
    void ApplyTiming(float msPerDuration)
    {
        durationMS = (float)duration * msPerDuration;
        startMS = (float)start * msPerDuration;
    }
    void Dump();
};

class MusicXML
{
    wxXmlDocument _doc;
    std::map<std::string, std::string> _parts;

public:
    MusicXML(std::string file);
    ~MusicXML();
    bool IsOk();
    std::list<std::string> GetTracks();
    std::list<MusicXmlNote> GetNotes(std::string track);
};
