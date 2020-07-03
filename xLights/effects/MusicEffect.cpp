/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/smeighan/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/smeighan/xLights/blob/master/License.txt
 **************************************************************/

#include "MusicPanel.h"
#include "MusicEffect.h"
#include "../AudioManager.h"
#include "../sequencer/SequenceElements.h"

#include "../sequencer/Effect.h"
#include "../RenderBuffer.h"
#include "../UtilClasses.h"
#include "../models/Model.h"
#include "../UtilFunctions.h"

#include "../../include/music-16.xpm"
#include "../../include/music-24.xpm"
#include "../../include/music-32.xpm"
#include "../../include/music-48.xpm"
#include "../../include/music-64.xpm"

#include <algorithm>

//#define wrdebug(...)

MusicEffect::MusicEffect(int id) : RenderableEffect(id, "Music Effect", music_16, music_24, music_32, music_48, music_64)
{
}

MusicEffect::~MusicEffect()
{
}

std::list<std::string> MusicEffect::CheckEffectSettings(const SettingsMap& settings, AudioManager* media, Model* model, Effect* eff, bool renderCache)
{
    std::list<std::string> res;

    if (media == nullptr)
    {
        res.push_back(wxString::Format("    ERR: Music effect is pointless if there is no music. Model '%s', Start %s", model->GetName(), FORMATTIME(eff->GetStartTimeMS())).ToStdString());
    }

    return res;
}

wxPanel *MusicEffect::CreatePanel(wxWindow *parent) {
	return new MusicPanel(parent);
}

bool MusicEffect::needToAdjustSettings(const std::string &version)
{
    return IsVersionOlder("2016.27", version);
}

void MusicEffect::adjustSettings(const std::string &version, Effect *effect, bool removeDefaults)
{
    // give the base class a chance to adjust any settings
    if (RenderableEffect::needToAdjustSettings(version))
    {
        RenderableEffect::adjustSettings(version, effect, removeDefaults);
    }

    SettingsMap &settings = effect->GetSettings();
    if (settings.Contains("E_CHECKBOX_Music_ScaleNotes"))
    {
        bool loop = settings.GetBool("E_CHECKBOX_Music_ScaleNotes", false);
        if (loop)
        {
            settings["E_CHOICE_Music_Scaling"] = "Individual Notes";
            settings.erase("E_CHECKBOX_Music_ScaleNotes");
        }
        else
        {
            settings["E_CHOICE_Music_Scaling"] = "All Notes";
            settings.erase("E_CHECKBOX_Music_ScaleNotes");
        }
    }
}

void MusicEffect::SetDefaultParameters() {
    MusicPanel *mp = (MusicPanel*)panel;
    if (mp == nullptr) {
        return;
    }

    mp->BitmapButton_Music_OffsetVC->SetActive(false);

    SetSliderValue(mp->Slider_Music_Bars, 20);
    SetChoiceValue(mp->Choice_Music_Type, "Morph");
    SetSliderValue(mp->Slider_Music_Sensitivity, 50);
    SetCheckBoxValue(mp->CheckBox_Music_Scale, false);
    SetChoiceValue(mp->Choice_Music_Scaling, "None");
    SetSliderValue(mp->Slider_Music_Offset, 0);
    SetSliderValue(mp->Slider_Music_StartNote, 60);
    SetSliderValue(mp->Slider_Music_EndNote, 80);
    SetChoiceValue(mp->Choice_Music_Colour, "Distinct");
    SetCheckBoxValue(mp->CheckBox_Music_Fade, false);
    SetCheckBoxValue(mp->CheckBox_Music_LogarithmicXAxis, false);
}

void MusicEffect::Render(Effect *effect, SettingsMap &SettingsMap, RenderBuffer &buffer) {
    float oset = buffer.GetEffectTimeIntervalPosition();
    Render(buffer,
        SettingsMap.GetInt("SLIDER_Music_Bars", 20),
        SettingsMap.Get("CHOICE_Music_Type", "Morph"),
        SettingsMap.GetInt("SLIDER_Music_Sensitivity", 50),
        SettingsMap.GetBool("CHECKBOX_Music_Scale", false),
        std::string(SettingsMap.Get("CHOICE_Music_Scaling", "None")),
        GetValueCurveInt("Music_Offset", 0, SettingsMap, oset, MUSIC_OFFSET_MIN, MUSIC_OFFSET_MAX, buffer.GetStartTimeMS(), buffer.GetEndTimeMS()),
        SettingsMap.GetInt("SLIDER_Music_StartNote", 60),
        SettingsMap.GetInt("SLIDER_Music_EndNote", 80),
        SettingsMap.Get("CHOICE_Music_Colour", "Distinct"),
        SettingsMap.GetBool("CHECKBOX_Music_Fade", false),
        SettingsMap.GetBool("CHECKBOX_Music_LogarithmicX", false)
    );
}

// represents a music event for a note
class MusicEvent
{
	public:
		int _startframe;
		int _duration; // in frames
		MusicEvent(int startframe, int duration)
		{
			_startframe = startframe;
			_duration = duration;
		}
		bool IsEventActive(int frame)
		{
			return (frame >= _startframe && frame < _startframe + _duration);
		}
		// a value between 0 & 1 representing how far into event the specified frame is
		float OffsetInDuration(int frame)
		{
			if (_duration == 0 || frame >= _startframe + _duration)
			{
				return 1.0;
			}
			if (frame < _startframe)
			{
				return 0.0;
			}

			return ((float)frame - (float)_startframe) / (float)_duration;
		}
};

class MusicRenderCache : public EffectRenderCache
{
public:
	void ClearEvents()
	{
		// delete all our music events
		for(int i = 0; i < _events.size(); i++)
		{
			for (auto it = _events[i]->begin(); it != _events[i]->end(); ++it)
			{
				delete *it;
			}
			_events[i]->clear();
		}
		_events.clear();
	}
    MusicRenderCache()
	{
	};
    virtual ~MusicRenderCache() {
        ClearEvents();
        for (auto it = _events.begin(); it != _events.end(); ++it)
        {
            delete *it;
        }
        _events.clear();
	};
	std::vector<std::list<MusicEvent*>*> _events;
};

int MusicEffect::DecodeScaleNotes(const std::string& scalenotes)
{
    if (scalenotes == "None")
    {
        return 1;
    }
    else if (scalenotes == "Individual Notes")
    {
        return 2;
    }
    else // All Notes
    {
        return 3;
    }
}

int MusicEffect::DecodeColourTreatment(const std::string& colourtreatment)
{
    if (colourtreatment == "Distinct")
    {
        return 1;
    }
    else if (colourtreatment == "Blend")
    {
        return 2;
    }
    else if (colourtreatment == "Cycle")
    {
        return 3;
    }
    return 1;
}

int MusicEffect::DecodeType(const std::string& type)
{
    if (type == "Morph")
    {
        return 1;
    }
    else if (type == "Bounce")
    {
        return 2;
    }
    else if (type == "Collide")
    {
        return 3;
    }
    else if (type == "Seperate")
    {
        return 4;
    }
    else if (type == "On")
    {
        return 5;
    }
    // default type is volume bars
    return 1;
}

void MusicEffect::Render(RenderBuffer &buffer,
    int bars, const std::string& type,
    int sensitivity, bool scale,
    const std::string& scalenotes, int offsetx,
    int startnote, int endnote,
    const std::string& colourtreatment,
    bool fade, bool logarithmicX)
{
    // no point if we have no media
    if (buffer.GetMedia() == nullptr)
    {
        return;
    }

    int nType = DecodeType(type);
    int nTreatment = DecodeColourTreatment(colourtreatment);

    // Grab our cache
    MusicRenderCache *cache = static_cast<MusicRenderCache*>(buffer.infoCache[id]);
    if (cache == nullptr) {
        cache = new MusicRenderCache();
        buffer.infoCache[id] = cache;
    }
    std::vector<std::list<MusicEvent*>*>& _events = cache->_events;

    // end note cant be less than start note
    if (startnote > endnote)
    {
        int temp = startnote;
        startnote = endnote;
        endnote = temp;
    }

    int actualbars = std::min(bars, std::min(endnote - startnote + 1, buffer.BufferWi - offsetx));
    if (actualbars == 0) actualbars = 1; // stop divide by zero error
    int notesperbar = (endnote - startnote + 1) / actualbars;
    int actualendnote = startnote + std::min(endnote, actualbars * notesperbar);
    int lightsperbar = 0.5 + (float)(buffer.BufferWi - offsetx) / (float)actualbars;

    // Check for config changes which require us to reset
    if (buffer.needToInit)
    {
        buffer.needToInit = false;
        cache->ClearEvents();
        // We limit bars to the width of the model less the x offset

        int nScaleNotes = DecodeScaleNotes(scalenotes);
        CreateEvents(buffer, _events, startnote, actualendnote, actualbars, nScaleNotes, sensitivity, logarithmicX);
    }

    int per = 1;
    if (scale)
    {
        per = lightsperbar;
    }

    try
    {
        for (int x = 0; x < _events.size(); x++)
        {
            for (int i = 0; i < per; i++)
            {
                switch (nType)
                {
                case 1:
                    RenderMorph(buffer, (x*per) + i + offsetx, bars, startnote, endnote, *_events[x], nTreatment, false, fade);
                    break;
                case 2:
                    RenderMorph(buffer, (x*per) + i + offsetx, bars, startnote, endnote, *_events[x], nTreatment, true, fade);
                    break;
                case 3:
                    RenderCollide(buffer, (x*per) + i + offsetx, bars, startnote, endnote, true /* collide */, *_events[x], nTreatment, fade);
                    break;
                case 4:
                    RenderCollide(buffer, (x*per) + i + offsetx, bars, startnote, endnote, false /* uncollide */,* _events[x], nTreatment, fade);
                    break;
                case 5:
                    RenderOn(buffer, (x*per) + i + offsetx, bars, startnote, endnote, *_events[x], nTreatment, fade);
                    break;
                }
            }
        }
    }
    catch (...)
    {
        // This is here to let me catch any exceptions and stop the exception causing the render thread to die
        //int a = 0;
    }
}

#define MINIMUMEVENTLENGTH 5

void MusicEffect::CreateEvents(RenderBuffer& buffer, std::vector<std::list<MusicEvent*>*>& events, int startNote, int endNote, int bars, int scalenotes, int sensitivity, bool logarithmicX)
{
    // must have media
    if (buffer.GetMedia() == nullptr)
    {
        return;
    }

    float notesperbar = ((float)endNote - (float)startNote + 1.0) / (float)bars;

    // use notes per bar
    // use multiple notes of data per output string using the maximum of these notes intensities
    std::map<int /*bar*/, std::map<int /*frame*/, float>> data;
    std::map<int /*bar*/, float> max;
    float overallmax = 0.0;

    for (int b = 0; b < bars; b++)
    {
        max[b] = 0.0;
    }

    // go through each frame and extract the data i need
    for (int f = buffer.curEffStartPer; f <= buffer.curEffEndPer; f++)
    {
        std::list<float> const * const pdata = buffer.GetMedia()->GetFrameData(f, FRAMEDATATYPE::FRAMEDATA_VU, "");

        if (pdata != nullptr)
        {
            auto pn = pdata->cbegin();

            // skip to start note
            for (int i = 0; i < startNote && pn != pdata->end(); i++)
            {
                ++pn;
            }

            for (int b = 0; b < bars && pn != pdata->end(); b++)
            {
                float val = 0.0;
                int thisper = static_cast<int>(notesperbar);
                if (logarithmicX)
                {
                    thisper = LogarithmicScale::GetLogSum(b + 1) - LogarithmicScale::GetLogSum(b);
                }
                for (auto n = 0; n < thisper && pn != pdata->end(); n++)
                {
                    val = std::max(val, *pn);
                    ++pn;
                }
                data[b][f] = val;
                max[b] = std::max(max[b], val);
                overallmax = std::max(overallmax, val);
            }
        }
    }

    for (int b = 0; b < bars; b++)
    {
        events.push_back(new std::list<MusicEvent*>());
        float notesensitivity;
        if (scalenotes == 1)
        {
            notesensitivity = (float)sensitivity / 100.0;
        }
        else if (scalenotes == 2)
        {
            notesensitivity = (float)sensitivity / 100.0 * max[b];
        }
        else
        {
            notesensitivity = (float)sensitivity / 100.0 * overallmax;
        }
        int startframe = -1;
        // extract the value for this note
        int frame = buffer.curEffStartPer;
        for (auto f = data[b].begin(); f != data[b].end(); ++f)
        {
            if (f->second > notesensitivity)
            {
                startframe = frame;

                while (f != data[b].end() && f->second > notesensitivity)
                {
                    ++f;
                    frame++;
                }
                --f;
                frame--;
                if (frame - startframe >= MINIMUMEVENTLENGTH)
                {
                    events[b]->push_back(new MusicEvent(startframe, frame - startframe));
                }
            }
            frame++;
        }
    }
}

void MusicEffect::RenderMorph(RenderBuffer &buffer, int x, int bars, int startNote, int endNote, std::list<MusicEvent*>& events, int colourTreatment, bool bounce, bool fade)
{
    bool up = true;
    int event = -1;
    for (auto it = events.begin(); it != events.end(); ++it)
    {
        event++;
        up = !up;
        if ((*it)->IsEventActive(buffer.curPeriod))
        {
            float progress = (*it)->OffsetInDuration(buffer.curPeriod);

            int length = buffer.BufferHt;
            int start = -1 * length + progress * 2 * length + 1;
            int end = start + length;

            for (int y = std::max(0, start); y < std::min(end, buffer.BufferHt); y++)
            {
                xlColor c = xlWHITE;
                float proportion = ((float)end - (float)y) / (float)length;
                if (colourTreatment == 1)
                {
                    // distinct
                    float percolour = 1.0 / (float)buffer.GetColorCount();
                    for (int i = 0; i < buffer.GetColorCount(); i++)
                    {
                        if (proportion <= ((float)i + 1.0)*percolour)
                        {
                            buffer.palette.GetColor(i, c);
                            break;
                        }
                    }
                }
                else if (colourTreatment == 2)
                {
                    // blend
                    buffer.GetMultiColorBlend(proportion, false, c);
                }
                else if (colourTreatment == 3)
                {
                    buffer.palette.GetColor(event % buffer.GetColorCount(), c);
                }

                if (fade)
                {
                    c.alpha = (1.0 -proportion) * 255;
                }

                if (up || !bounce)
                {
                    buffer.SetPixel(x, y, c);
                }
                else
                {
                    buffer.SetPixel(x, buffer.BufferHt - y - 1, c);
                }
            }
        }
    }
}

void MusicEffect::RenderCollide(RenderBuffer &buffer, int x, int bars, int startNote, int endNote, bool in, std::list<MusicEvent*>& events, int colourTreatment, bool fade)
{
    int event = -1;
    for (auto it = events.begin(); it != events.end(); ++it)
    {
        event++;
        if ((*it)->IsEventActive(buffer.curPeriod))
        {
            float progress = (*it)->OffsetInDuration(buffer.curPeriod);

            int mid = buffer.BufferHt / 2;
            int length = buffer.BufferHt;
            int leftstart, leftend;

            if (!in)
            {
                progress = 1.0 - progress;
            }

            leftstart = 0 - mid - 1 + progress * length;
            leftend = leftstart + mid;
            if (leftend > mid)
            {
                leftend = mid;
            }

            int loopstart = leftstart;
            if (loopstart < 0)
            {
                loopstart = 0;
            }

            for (int y = loopstart; y < leftend; y++)
            {
                xlColor c = xlWHITE;
                float proportion = ((float)y - (float)leftstart) / (float)mid;
                if (colourTreatment == 1)
                {
                    // distinct
                    float percolour = 1.0 / (float)buffer.GetColorCount();
                    for (int i = 0; i < buffer.GetColorCount(); i++)
                    {
                        if (proportion <= ((float)i + 1.0)*percolour)
                        {
                            buffer.palette.GetColor(i, c);
                            break;
                        }
                    }
                }
                else if (colourTreatment == 2)
                {
                    // blend
                    buffer.GetMultiColorBlend(proportion, false, c);
                }
                else if (colourTreatment == 3)
                {
                    buffer.palette.GetColor(event % buffer.GetColorCount(), c);
                }

                if (fade)
                {
                    c.alpha = progress * 255;
                }

                buffer.SetPixel(x, y, c);
                buffer.SetPixel(x, mid - y + mid - 1, c);
            }
        }
    }
}

void MusicEffect::RenderOn(RenderBuffer &buffer, int x, int bars, int startNote, int endNote, std::list<MusicEvent*>& events, int colourTreatment, bool fade)
{
    int event = -1;
    for (auto it = events.begin(); it != events.end(); ++it)
    {
        event++;
        if ((*it)->IsEventActive(buffer.curPeriod))
        {
            float progress = (*it)->OffsetInDuration(buffer.curPeriod);


            for (int y = 0; y < buffer.BufferHt; y++)
            {
                xlColor c = xlWHITE;
                float proportion = (float)y / (float)buffer.BufferHt;
                if (colourTreatment == 1)
                {
                    // distinct
                    float percolour = 1.0 / (float)buffer.GetColorCount();
                    for (int i = 0; i < buffer.GetColorCount(); i++)
                    {
                        if (proportion <= ((float)i + 1.0)*percolour)
                        {
                            buffer.palette.GetColor(i, c);
                            break;
                        }
                    }
                }
                else if (colourTreatment == 2)
                {
                    // blend
                    buffer.GetMultiColorBlend(proportion, false, c);
                }
                else if (colourTreatment == 3)
                {
                    buffer.palette.GetColor(event % buffer.GetColorCount(), c);
                }

                if (fade)
                {
                    c.alpha = (1.0 - progress) * 255;
                }

                buffer.SetPixel(x, y, c);
            }
        }
    }
}
