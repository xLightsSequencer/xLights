#include "MusicEffect.h"
#include "MusicPanel.h"
#include "../AudioManager.h"
#include "../sequencer/SequenceElements.h"

#include "../sequencer/Effect.h"
#include "../RenderBuffer.h"
#include "../UtilClasses.h"

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

wxPanel *MusicEffect::CreatePanel(wxWindow *parent) {
	return new MusicPanel(parent);
}

void MusicEffect::Render(Effect *effect, const SettingsMap &SettingsMap, RenderBuffer &buffer) {
	Render(buffer,
           SettingsMap.GetInt("TEXTCTRL_Music_Bars", 6),
		   SettingsMap.Get("CHOICE_Music_Type", "Collide"),
		   SettingsMap.GetInt("CHOICE_Music_Sensitivity", 50),
		   SettingsMap.GetBool("CHECKBOX_Music_Scale", false),
 		   SettingsMap.GetBool("CHECKBOX_Music_ScaleNotes", false),
           SettingsMap.GetInt("TEXTCTRL_Music_Offset", 0),
           SettingsMap.GetInt("TEXTCTRL_Music_StartNote", 0),
           SettingsMap.GetInt("TEXTCTRL_Music_EndNote", 127),   
           SettingsMap.Get("CHOICE_Music_Colour", "Distinct")
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
	int _bars;
    int _startNote;
    int _endNote;
	int _type;
    int _colourTreatment;
    int _sensitivity;
	int _offsetx; // skip these many cols at left of model
	bool _scale; // scale out to fill model horizontally
    bool _freqRelative; // scale based on maximum volume within frequency ... otherwise scale on maximum volume across all frequencies
};

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
}

int MusicEffect::DecodeType(const std::string& type)
{
    if (type == "Morphs In")
    {
        return 1;
    }
    else if (type == "Morphs Out")
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
    // default type is volume bars
    return 1;
}

void MusicEffect::Render(RenderBuffer &buffer,
    int bars, const std::string& type,
    int sensitivity, bool scale,
    bool freqrelative, int offsetx,
    int startnote, int endnote,
    const std::string& colourtreatment)
{
    startnote = Normalise(startnote, 0, 127);
    endnote = Normalise(endnote, 0, 127);
    sensitivity = Normalise(sensitivity, 0, 100);
    bars = Normalise(bars, 0, 100);
    offsetx = Normalise(offsetx, 0, 100);

    if (startnote < 0)
    {
        startnote = 0;
    }
    if (endnote < 0)
    {
        endnote = 0;
    }
    if (startnote > 127)
    {
        startnote = 127;
    }
    if (endnote > 127)
    {
        endnote = 127;
    }

    // no point if we have no media
    if (buffer.GetMedia() == NULL)
    {
        return;
    }

    int nType = DecodeType(type);
    int nTreatment = DecodeColourTreatment(colourtreatment);

    // Grab our cache
    MusicRenderCache *cache = (MusicRenderCache*)buffer.infoCache[id];
    if (cache == nullptr) {
        cache = new MusicRenderCache();
        buffer.infoCache[id] = cache;
    }
    int &_bars = cache->_bars;
    int &_startNote = cache->_startNote;
    int &_endNote = cache->_endNote;
    int &_type = cache->_type;
    int &_offsetx = cache->_offsetx;
    bool &_scale = cache->_scale;
    int &_sensitivity = cache->_sensitivity;
    bool &_freqRelative = cache->_freqRelative;
    int& _colourTreatment = cache->_colourTreatment;
    std::vector<std::list<MusicEvent*>*>& _events = cache->_events;

    int actualbars = std::min(bars, std::min(endnote - startnote + 1, buffer.BufferWi - offsetx));
    int notesperbar = (endnote - startnote + 1) / actualbars;
    int actualendnote = startnote + std::min(endnote, actualbars * notesperbar);
    int lightsperbar = 0.5 + (float)(buffer.BufferWi - offsetx) / (float)actualbars;

    // Check for config changes which require us to reset
    if (buffer.needToInit || _bars != bars || _type != nType || _startNote != startnote || _endNote != endnote || _offsetx != offsetx || _scale != scale || _freqRelative != freqrelative || _sensitivity != sensitivity || _colourTreatment != nTreatment)
    {
        buffer.needToInit = false;
        _bars = bars;
        _startNote = startnote;
        _endNote = endnote;
        _colourTreatment = nTreatment;
        _type = nType;
        _offsetx = offsetx;
        _scale = scale;
        _sensitivity = sensitivity;
        _freqRelative = freqrelative;
        cache->ClearEvents();
        // We limit bars to the width of the model less the x offset

        CreateEvents(buffer, _events, _startNote, actualendnote, actualbars, _freqRelative, _sensitivity);
    }

    int per = 1;
    if (_scale)
    {
        per = lightsperbar;
    }

    try
    {
        for (int x = 0; x < _events.size(); x++)
        {
            for (int i = 0; i < per; i++)
            {
                switch (_type)
                {
                case 1:
                    RenderMorph(buffer, (x*per) + i + _offsetx, _bars, _startNote, _endNote, true /* in */, *_events[x], _colourTreatment);
                    break;
                case 2:
                    RenderMorph(buffer, (x*per) + i + _offsetx, _bars, _startNote, _endNote, false /* out */, *_events[x], _colourTreatment);
                    break;
                case 3:
                    RenderCollide(buffer, (x*per) + i + _offsetx, _bars, _startNote, _endNote, true /* collide */, *_events[x], _colourTreatment);
                    break;
                case 4:
                    RenderCollide(buffer, (x*per) + i + _offsetx, _bars, _startNote, _endNote, false /* uncollide */,* _events[x], _colourTreatment);
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

void MusicEffect::CreateEvents(RenderBuffer& buffer, std::vector<std::list<MusicEvent*>*>& events, int startNote, int endNote, int bars, bool freqRelative, int sensitivity)
{
    // must have media
    if (buffer.GetMedia() == NULL)
    {
        return;
    }

    float notesperbar = ((float)endNote - (float)startNote + 1.0) / (float)bars;

    // use notes per bar
    // use multiple notes of data per output string using the maximum of these notes intensities
    std::map<int /*bar*/, std::map<int /*frame*/, float>> data;
    std::map<int /*bar*/, float> max;

    // go through each frame and extract the data i need
    for (int f = buffer.curEffStartPer; f <= buffer.curEffEndPer; f++)
    {
        std::list<float>* pdata = buffer.GetMedia()->GetFrameData(f, FRAMEDATATYPE::FRAMEDATA_VU, "");

        auto pn = pdata->begin();

        // skip to start note
        for (int i = 0; i < startNote; i++)
        {
            ++pn;
        }

        for (int b = 0; b < bars; b++)
        {
            float val = 0.0;
            max[b] = 0.0;
            for (int n = 0; n < (int)notesperbar; n++)
            {
                val = std::max(val, *pn);
                ++pn;
            }
            data[b][f] = val;
            max[b] = std::max(max[b], val);
        }
    }

    for (int b = 0; b < bars; b++)
    {
        events.push_back(new std::list<MusicEvent*>());
        float notesensitivity;
        if (freqRelative)
        {
            notesensitivity = (float)sensitivity / 100.0 * max[b];
        }
        else
        {
            notesensitivity = (float)sensitivity / 100.0;
        }
        int startframe = -1;
        // extract the value for this note
        int frame = 0;
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
                if (frame - startframe > MINIMUMEVENTLENGTH)
                {
                    events[b]->push_back(new MusicEvent(startframe, frame - startframe));
                }
            }
            frame++;
        }
    }
}

void MusicEffect::RenderMorph(RenderBuffer &buffer, int x, int bars, int startNote, int endNote, bool in,std::list<MusicEvent*>& events, int colourTreatment)
{
}

void MusicEffect::RenderCollide(RenderBuffer &buffer, int x, int bars, int startNote, int endNote, bool in, std::list<MusicEvent*>& events, int colourTreatment)
{
    for (auto it = events.begin(); it != events.end(); ++it)
    {
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
                buffer.SetPixel(x, y, c);
                buffer.SetPixel(x, mid - y + mid - 1, c);
            }
        }
    }
}
