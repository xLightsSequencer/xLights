/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/smeighan/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/smeighan/xLights/blob/master/License.txt
 **************************************************************/

#include <vector>

#include "../../include/piano-16.xpm"
#include "../../include/piano-64.xpm"

#include "PianoEffect.h"
#include "PianoPanel.h"
#include "../RenderBuffer.h"
#include "../UtilClasses.h"
#include "../UtilFunctions.h"
#include "../sequencer/Effect.h"
#include "../xLightsXmlFile.h"
#include "models/Model.h"

#include <log4cpp/Category.hh>

PianoEffect::PianoEffect(int id) :
    RenderableEffect(id, "Piano", piano_16, piano_64, piano_64, piano_64, piano_64)
{
    // ctor
    _panel = nullptr;
}

PianoEffect::~PianoEffect()
{
    // dtor
}

std::list<std::string> PianoEffect::CheckEffectSettings(const SettingsMap& settings, AudioManager* media, Model* model, Effect* eff, bool renderCache)
{
    std::list<std::string> res;

    if (settings.Get("E_CHOICE_Piano_MIDITrack_APPLYLAST", "") == "") {
        res.push_back(wxString::Format("    ERR: Piano effect needs a timing track. Model '%s', Start %s", model->GetName(), FORMATTIME(eff->GetStartTimeMS())).ToStdString());
    } else {
        std::map<int, std::list<std::pair<float, float>>> timings = LoadTimingTrack(settings.Get("E_CHOICE_Piano_MIDITrack_APPLYLAST", ""), 50, false);
        if (timings.size() == 0) {
            res.push_back(wxString::Format("    ERR: Piano effect timing track '%s' has no notes. Model '%s', Start %s", settings.Get("E_CHOICE_Piano_MIDITrack_APPLYLAST", ""), model->GetName(), FORMATTIME(eff->GetStartTimeMS())).ToStdString());
        }
    }

    return res;
}

void PianoEffect::SetPanelStatus(Model* cls)
{
    SetPanelTimingTracks();
}

void PianoEffect::SetPanelTimingTracks()
{
    PianoPanel* fp = (PianoPanel*)panel;
    if (fp == nullptr) {
        return;
    }

    if (mSequenceElements == nullptr) {
        return;
    }

    // Load the names of the timing tracks
    std::string timingtracks = GetTimingTracks(1);
    wxCommandEvent event(EVT_SETTIMINGTRACKS);
    event.SetString(timingtracks);
    wxPostEvent(fp, event);
}

void PianoEffect::adjustSettings(const std::string& version, Effect* effect, bool removeDefaults)
{
    // give the base class a chance to adjust any settings
    if (RenderableEffect::needToAdjustSettings(version)) {
        RenderableEffect::adjustSettings(version, effect, removeDefaults);
    }

    if (IsVersionOlder("2016.45", version)) {
        SettingsMap& settings = effect->GetSettings();
        wxString oldsettings = settings.Get("E_CHOICE_Piano_Notes_Source", "newsettings");

        if (oldsettings != "newsettings") {
            if (oldsettings == "Timing Track") {
                DisplayWarning("Piano effect has changed. Old settings have been removed but you should be ok.");
            } else {
                DisplayWarning("Piano effect has changed. Old settings have been removed. Please create a notes timing track using 'import notes' by right clicking on the timing track in the sequencer and then adjust piano settings.");
            }

            // strip out old settings
            settings.erase("E_CHOICE_Piano_Notes_Source");
            settings.erase("E_TEXTCTRL_Piano_File");
            settings.erase("E_SLIDER_Piano_MIDI_Start");
            settings.erase("E_SLIDER_Piano_MIDI_Speed");
        }
    }
}

xlEffectPanel* PianoEffect::CreatePanel(wxWindow* parent)
{
    _panel = new PianoPanel(parent);
    return _panel;
}

void PianoEffect::SetDefaultParameters()
{
    PianoPanel* pp = (PianoPanel*)panel;
    if (pp == nullptr) {
        return;
    }

    pp->BitmapButton_Piano_ScaleVC->SetActive(false);

    SetChoiceValue(pp->Choice_Piano_Type, "True Piano");
    SetSpinValue(pp->SpinCtrl_Piano_StartMIDI, 60);
    SetSpinValue(pp->SpinCtrl_Piano_EndMIDI, 72);
    SetCheckBoxValue(pp->CheckBox_Piano_ShowSharps, true);
    SetCheckBoxValue(pp->CheckBox_FadeNotes, false);
    SetSliderValue(pp->Slider_Piano_Scale, 100);
    SetSliderValue(pp->Slider_Piano_XOffset, 0);

    SetPanelTimingTracks();
}

void PianoEffect::RenameTimingTrack(std::string oldname, std::string newname, Effect* effect)
{
    wxString timing = effect->GetSettings().Get("E_CHOICE_Piano_MIDITrack_APPLYLAST", "");

    if (timing.ToStdString() == oldname) {
        effect->GetSettings()["E_CHOICE_Piano_MIDITrack_APPLYLAST"] = wxString(newname);
    }

    SetPanelTimingTracks();
}

void PianoEffect::Render(Effect* effect, const SettingsMap& SettingsMap, RenderBuffer& buffer)
{
    float oset = buffer.GetEffectTimeIntervalPosition();
    RenderPiano(buffer,
                effect->GetParentEffectLayer()->GetParentElement()->GetSequenceElements(),
                SettingsMap.GetInt("SPINCTRL_Piano_StartMIDI"),
                SettingsMap.GetInt("SPINCTRL_Piano_EndMIDI"),
                SettingsMap.GetBool("CHECKBOX_Piano_ShowSharps"),
                std::string(SettingsMap.Get("CHOICE_Piano_Type", "True Piano")),
                GetValueCurveInt("Piano_Scale", 100, SettingsMap, oset, PIANO_SCALE_MIN, PIANO_SCALE_MAX, buffer.GetStartTimeMS(), buffer.GetEndTimeMS()),
                std::string(SettingsMap.Get("CHOICE_Piano_MIDITrack_APPLYLAST", "")),
                SettingsMap.GetInt("SLIDER_Piano_XOffset", 0),
                SettingsMap.GetBool("CHECKBOX_Piano_FadeNotes", false));
}

class PianoCache : public EffectRenderCache
{
public:
    PianoCache(){};
    virtual ~PianoCache(){};

    // frame, {note, fade}
    std::map<int, std::list<std::pair<float, float>>> _timings;
    std::string _MIDItrack;
};

// render piano fx during sequence:
void PianoEffect::RenderPiano(RenderBuffer& buffer, SequenceElements* elements, const int startmidi, const int endmidi, const bool sharps, const std::string type, int scale, std::string MIDITrack, int xoffset, bool fadeNotes)
{
    PianoCache* cache = (PianoCache*)buffer.infoCache[id];
    if (cache == nullptr) {
        cache = new PianoCache();
        buffer.infoCache[id] = cache;
    }

    std::map<int, std::list<std::pair<float, float>>>& _timings = cache->_timings;
    std::string& _MIDITrack = cache->_MIDItrack;

    if (buffer.needToInit) {
        // just in case the timing tracks have changed
        SetPanelTimingTracks();

        buffer.needToInit = false;
        if (_MIDITrack != MIDITrack) {
            _timings.clear();
            _timings = LoadTimingTrack(MIDITrack, buffer.frameTimeInMs, fadeNotes);
            elements->AddRenderDependency(MIDITrack, buffer.cur_model);
        }

        _MIDITrack = MIDITrack;

        if (_MIDITrack != "") {
            elements->AddRenderDependency(_MIDITrack, buffer.cur_model);
        }
    }

    int em = endmidi;

    // end midi must not be less than start midi
    if (em < startmidi) {
        em = startmidi;
    }

    if (em - startmidi + 1 > buffer.BufferWi) {
        em = startmidi + buffer.BufferWi - 1;
    }

    std::list<std::pair<float, float>> def;
    std::list<std::pair<float, float>>* pdata = nullptr;

    int time = buffer.curPeriod * buffer.frameTimeInMs;
    if (_timings.find(time) != _timings.end()) {
        pdata = &_timings[time];
    }

    if (pdata == nullptr) {
        pdata = &def;
    }

    ReduceChannels(pdata, startmidi, em, sharps);

    if (type == "True Piano") {
        DrawTruePiano(buffer, pdata, sharps, startmidi, em, scale, xoffset, fadeNotes);
    } else if (type == "Bars") {
        DrawBarsPiano(buffer, pdata, sharps, startmidi, em, scale, xoffset, fadeNotes);
    }
}

bool PianoEffect::IsSharp(float f)
{
    int x = (int)f % 12;
    // 0 is C
    return (x == 1 ||
            x == 3 ||
            x == 6 ||
            x == 8 ||
            x == 10);
}

void PianoEffect::ReduceChannels(std::list<std::pair<float, float>>* pdata, int start, int end, bool sharps)
{
    auto it = pdata->begin();
    while (it != pdata->end()) {
        if (!sharps && IsSharp((*it).first)) {
            float n = (*it).first - 1.0;
            bool found = false;
            for (const auto& c : *pdata) {
                if (c.first == n) {
                    // already there
                    found = true;
                    break;
                }
            }
            if (!found) {
                pdata->push_back({ n, 0.0 });
            }
            pdata->remove(*it);
            it = pdata->begin();
        } else if ((*it).first < start || (*it).first > end) {
            pdata->remove(*it);
            it = pdata->begin();
        } else {
            ++it;
        }
    }
}

float PianoEffect::GetKeyBrightness(std::list<std::pair<float, float>>* pdata, int ch)
{
    for (const auto& it : *pdata) {
        if (ch == (int)(it.first)) {
            return it.second;
        }
    }

    return 0.0;
}

bool PianoEffect::KeyDown(std::list<std::pair<float, float>>* pdata, int ch)
{
    for (const auto& it : *pdata) {
        if (ch == (int)(it.first)) {
            return true;
        }
    }

    return false;
}

void PianoEffect::DrawTruePiano(RenderBuffer& buffer, std::list<std::pair<float, float>>* pdata, bool sharps, int start, int end, int scale, int xoffset, bool fadeNotes)
{
    int truexoffset = xoffset * buffer.BufferWi / 100;
    xlColor wkcolour, bkcolour, wkdcolour, bkdcolour, kbcolour;

    int sharpstart = -1;
    int i = start;
    while (sharpstart == -1 && i <= end) {
        if (IsSharp(i)) {
            sharpstart = i;
        }
        ++i;
    }

    int sharpend = -1;
    i = end;
    while (sharpend == -1 && i >= start) {
        if (IsSharp(i)) {
            sharpend = i;
        }
        --i;
    }

    int whitestart = -1;
    i = start;
    while (whitestart == -1 && i <= end) {
        if (!IsSharp(i)) {
            whitestart = i;
        }
        ++i;
    }

    int whiteend = -1;
    i = end;
    while (whiteend == -1 && i >= start) {
        if (!IsSharp(i)) {
            whiteend = i;
        }
        --i;
    }

    int wkcount = 0;
    if (whitestart != -1 && whiteend != -1) {
        for (i = whitestart; i <= whiteend; ++i) {
            if (!IsSharp(i)) {
                ++wkcount;
            }
        }
    }
    if (wkcount == 0)
        wkcount = 1; // avoid a divide by zero error

    float fwkw = (float)buffer.BufferWi / (float)wkcount;
    float wkw = fwkw;
    float maxx = (float)wkcount * fwkw;
    bool border = false;
    if (wkw > 3) {
        border = true;
        --wkw;
    }

    // Get the colours
    if (buffer.GetColorCount() > 0) {
        buffer.palette.GetColor(0, wkcolour);
    } else {
        wkcolour = xlWHITE;
    }
    if (buffer.GetColorCount() > 1) {
        buffer.palette.GetColor(1, bkcolour);
    } else {
        bkcolour = xlBLACK;
    }
    if (buffer.GetColorCount() > 2) {
        buffer.palette.GetColor(2, wkdcolour);
    } else {
        wkdcolour = xlMAGENTA;
    }
    if (buffer.GetColorCount() > 3) {
        buffer.palette.GetColor(3, bkdcolour);
    } else {
        bkdcolour = xlMAGENTA;
    }
    if (buffer.GetColorCount() > 4) {
        buffer.palette.GetColor(4, kbcolour);
    } else {
        kbcolour = xlLIGHT_GREY;
    }

    // Draw white keys
    float x = truexoffset;
    for (i = start; i <= end; ++i) {
        if (!IsSharp(i)) {
            if (KeyDown(pdata, i)) {
                if (fadeNotes) {
                    wkdcolour.alpha = GetKeyBrightness(pdata, i) * 255;
                    wkdcolour = wkdcolour.AlphaBlend(wkcolour);
                    buffer.DrawBox(x, 0, x + wkw, buffer.BufferHt * scale / 100, wkdcolour, false, true);
                } else {
                    buffer.DrawBox(x, 0, x + wkw, buffer.BufferHt * scale / 100, wkdcolour, false);
                }
            } else {
                buffer.DrawBox(x, 0, x + wkw, buffer.BufferHt * scale / 100, wkcolour, false);
            }
            x += fwkw;
        }
    }

    // Draw white key borders
    if (border) {
        x = fwkw + truexoffset;
        for (int j = 0; j < wkcount; ++j) {
            buffer.DrawLine(x, 0, x, buffer.BufferHt * scale / 100, kbcolour);
            x += fwkw;
        }
    }

    if (sharps) {
#define BKADJUSTMENTWIDTH(a) (int)std::round(0.3 / 2.0 * (float)a)
        // Draw the black keys
        if (IsSharp(start)) {
            x = -1.0 * fwkw / 2.0 + truexoffset;
        } else if (IsSharp(start + 1)) {
            x = fwkw / 2.0 + truexoffset;
        } else {
            x = fwkw + fwkw / 2.0 + truexoffset;
        }
        for (i = start; i <= end; ++i) {
            if (IsSharp(i)) {
                if (KeyDown(pdata, i)) {
                    if (fadeNotes) {
                        bkdcolour.alpha = GetKeyBrightness(pdata, i) * 255;
                        bkdcolour = bkdcolour.AlphaBlend(bkcolour);
                        buffer.DrawBox(x + BKADJUSTMENTWIDTH(fwkw), buffer.BufferHt * scale / 200, std::min(maxx, x + fwkw - BKADJUSTMENTWIDTH(fwkw)), buffer.BufferHt * scale / 100, bkdcolour, false, true);
                    } else {
                        buffer.DrawBox(x + BKADJUSTMENTWIDTH(fwkw), buffer.BufferHt * scale / 200, std::min(maxx, x + fwkw - BKADJUSTMENTWIDTH(fwkw)), buffer.BufferHt * scale / 100, bkdcolour, false);
                    }
                } else {
                    buffer.DrawBox(x + BKADJUSTMENTWIDTH(fwkw), buffer.BufferHt * scale / 200, std::min(maxx, x + fwkw - BKADJUSTMENTWIDTH(fwkw)), buffer.BufferHt * scale / 100, bkcolour, false);
                }
                if (!IsSharp(i + 1) && !IsSharp(i + 2)) {
                    x += fwkw + fwkw;
                } else {
                    x += fwkw;
                }
            }
        }
    }
}

void PianoEffect::DrawBarsPiano(RenderBuffer& buffer, std::list<std::pair<float, float>>* pdata, bool sharps, int start, int end, int scale, int xoffset, bool fadeNotes)
{
    int truexoffset = xoffset * buffer.BufferWi / 100;
    xlColor wkcolour, bkcolour, wkdcolour, bkdcolour;

    // count the keys
    int kcount = -1;
    if (sharps) {
        kcount = end - start + 1;
    } else {
        for (int i = start; i <= end; ++i) {
            if (!IsSharp(i)) {
                ++kcount;
            }
        }
    }

    if (kcount == 0)
        kcount = 1; // avoid divide by zero error
    float fwkw = (float)buffer.BufferWi / (float)kcount;

    // Get the colours
    if (buffer.GetColorCount() > 0) {
        buffer.palette.GetColor(0, wkcolour);
    } else {
        wkcolour = xlWHITE;
    }
    if (buffer.GetColorCount() > 1) {
        buffer.palette.GetColor(1, bkcolour);
    } else {
        bkcolour = xlBLACK;
    }
    if (buffer.GetColorCount() > 2) {
        buffer.palette.GetColor(2, wkdcolour);
    } else {
        wkdcolour = xlMAGENTA;
    }
    if (buffer.GetColorCount() > 3) {
        buffer.palette.GetColor(3, bkdcolour);
    } else {
        bkdcolour = xlMAGENTA;
    }

    // Draw keys
    float x = 0.0 + truexoffset;
    int wkh = buffer.BufferHt;
    if (sharps) {
        wkh = buffer.BufferHt * 2.0 * scale / 300.0;
    }
    int bkb = buffer.BufferHt * scale / 300.0;
    for (int i = start; i <= end; ++i) {
        if (!IsSharp(i)) {
            if (KeyDown(pdata, i)) {
                if (fadeNotes) {
                    wkdcolour.alpha = GetKeyBrightness(pdata, i) * 255;
                    wkdcolour = wkdcolour.AlphaBlend(wkcolour);
                    buffer.DrawBox(x, 0, x + fwkw - 1, wkh, wkdcolour, true, true);
                } else {
                    buffer.DrawBox(x, 0, x + fwkw - 1, wkh, wkdcolour, false);
                }
            } else {
                buffer.DrawBox(x, 0, x + fwkw - 1, wkh, wkcolour, false);
            }
            x += fwkw;
        } else {
            if (sharps) {
                if (KeyDown(pdata, i)) {
                    if (fadeNotes) {
                        bkdcolour.alpha = GetKeyBrightness(pdata, i) * 255;
                        bkdcolour = bkdcolour.AlphaBlend(bkcolour);
                        buffer.DrawBox(x, bkb, x + fwkw - 1, buffer.BufferHt * scale / 100, bkdcolour, false, true);
                    } else {
                        buffer.DrawBox(x, bkb, x + fwkw - 1, buffer.BufferHt * scale / 100, bkdcolour, false);
                    }
                } else {
                    buffer.DrawBox(x, bkb, x + fwkw - 1, buffer.BufferHt * scale / 100, bkcolour, false);
                }
                x += fwkw;
            }
        }
    }
}

std::vector<float> PianoEffect::Parse(wxString& l)
{
    std::vector<float> res;
    wxString s = l;
    while (s.Len() != 0) {
        int end = s.First('\t');
        if (end > 0) {
            res.push_back(wxAtof(s.SubString(0, end - 1)));
            s = s.Right(s.Len() - end - 1);
        } else {
            res.push_back(wxAtof(s));
            s = "";
        }
    }

    return res;
}

std::list<std::string> PianoEffect::ExtractNotes(const std::string& label)
{
    std::string n = label;
    std::transform(n.begin(), n.end(), n.begin(), ::toupper);

    std::list<std::string> res;

    std::string s = "";
    for (const auto& it : n) {
        if (it == ':' || it == ' ' || it == ';' || it == ',') {
            if (s != "") {
                res.push_back(s);
                s = "";
            }
        } else {
            if ((it >= 'A' && it <= 'G') ||
                (it == '#') ||
                (it >= '0' && it <= '9')) {
                s += it;
            }
        }
    }

    if (s != "") {
        res.push_back(s);
    }

    return res;
}

int PianoEffect::ConvertNote(const std::string& note)
{
    std::string n = note;
    int nletter;
    std::transform(n.begin(), n.end(), n.begin(), ::toupper);

    switch (n[0]) {
    case 'A':
        nletter = 9;
        break;
    case 'B':
        nletter = 11;
        break;
    case 'C':
        nletter = 0;
        break;
    case 'D':
        nletter = 2;
        break;
    case 'E':
        nletter = 4;
        break;
    case 'F':
        nletter = 5;
        break;
    case 'G':
        nletter = 7;
        break;
    default: {
        int number = wxAtoi(n);
        if (number < 0)
            number = 0;
        if (number > 127)
            number = 127;
        return number;
    }
    }

    n = n.substr(1);
    int sharp = 0;
    if (n.find('#') != std::string::npos) {
        sharp = 1;
    } else if (n.find('B') != std::string::npos) {
        sharp = -1;
    }

    int octave = 4;

    if (n != "") {
        if (n[0] == '#') {
            n = n.substr(1);
        } else if (n[0] == 'B') {
            n = n.substr(1);
        }
    }

    if (n != "") {
        octave = wxAtoi(n);
    }

    int number = 12 + (octave * 12) + nletter + sharp;
    if (number < 0)
        number = 0;
    if (number > 127)
        number = 127;
    return number;
}

std::tuple<int, int, int>* FindTracker(std::list<std::tuple<int, int, int>>& tracker, int note)
{
    for (auto& it : tracker) {
        if (std::get<0>(it) == note)
            return &it;
    }

    return nullptr;
}

std::map<int, std::list<std::pair<float, float>>> PianoEffect::LoadTimingTrack(const std::string& track, int intervalMS, bool fadeNotes)
{
    static log4cpp::Category& logger_pianodata = log4cpp::Category::getInstance(std::string("log_pianodata"));
    std::map<int, std::list<std::pair<float, float>>> res;

    logger_pianodata.debug("Loading timings from timing track " + track);

    if (mSequenceElements == nullptr) {
        logger_pianodata.debug("No timing tracks found.");
        return res;
    }

    // Load the names of the timing tracks
    EffectLayer* el = GetTiming(track);

    if (el == nullptr) {
        logger_pianodata.debug("Timing track not found.");
        return res;
    }

    int last = 0;
    for (int j = 0; j < el->GetEffectCount(); ++j) {
        std::list<std::pair<float, float>> notes;
        int starttime = el->GetEffect(j)->GetStartTimeMS();
        int endtime = el->GetEffect(j)->GetEndTimeMS();
        last = std::max(last, endtime);
        std::string label = el->GetEffect(j)->GetEffectName();
        std::list<std::string> notelabels = ExtractNotes(label);
        for (const auto& s : notelabels) {
            float n = (float)ConvertNote(s);
            if (n >= 0) {
                notes.push_back({ n, 1.0 });
            }
        }
        for (int i = starttime; i < endtime; i += intervalMS) {
            res[i] = notes;
        }
    }

    if (fadeNotes) {
        // now we need to work out the fades - not super thrilled with the complexity of this but it works on the data structures we already had built.
        
        // int note, start frame, frames
        std::list<std::tuple<int, int, int>> tracker;
        for (const auto& it : res) {
            std::list<int> notes;
            // go through the current frame notes and add/update the tracker
            for (auto& it2 : it.second) {
                notes.push_back(it2.first);

                auto tuple = FindTracker(tracker, it2.first);

                if (tuple == nullptr) {
                    // note is new
                    tracker.push_back({ it2.first, it.first, 1 });
                } else {
                    // note is existing
                    std::get<2>(*tuple) = std::get<2>(*tuple) + 1;
                }
            }

            for (auto& it2 : tracker) {
                if (std::find(begin(notes), end(notes), std::get<0>(it2)) == end(notes)) {
                    // this tracker item is done so we need to go back and update fade
                    int startFrame = std::get<1>(it2);
                    int endFrame = it.first;
                    for (int f = startFrame; f < endFrame; f += intervalMS) {
                        for (auto& it3 : res[f]) {
                            if (it3.first == std::get<0>(it2)) {
                                it3.second = 1.0 - (float)(f - startFrame) / (float)(endFrame - startFrame);
                            }
                        }
                    }
                    std::get<0>(it2) = -1;
                }
            }

            // std::eraseif
            std::remove_if(tracker.begin(), tracker.end(), [](std::tuple<int, int, int> t) { return std::get<0>(t) == -1; });
        }

        // clean up anything left in the tracker
        for (auto& it2 : tracker) {
            // this tracker item is done so we need to go back and update fade
            int startFrame = std::get<1>(it2);
            int endFrame = last;
            for (int f = startFrame; f < endFrame; f += intervalMS) {
                for (auto& it3 : res[f]) {
                    if (it3.first == std::get<0>(it2)) {
                        it3.second = 1.0 - (float)(f - startFrame) / (float)(endFrame - startFrame);
                    }
                }
            }
        }
    }

    return res;
}
