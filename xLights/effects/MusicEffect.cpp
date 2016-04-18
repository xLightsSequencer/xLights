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
		   SettingsMap.Get("CHOICE_Music_Type", "Morphs In"),
		   SettingsMap.GetInt("CHOICE_Music_Sensitivity", 50),
		   SettingsMap.GetBool("TEXTCTRL_Music_Scale", false),
 		   SettingsMap.GetBool("CHOICE_Music_FreqRelative", false),
           SettingsMap.GetInt("TEXTCTRL_Music_OffsetX", 0),
           SettingsMap.GetInt("TEXTCTRL_Music_StartNote", 0),
           SettingsMap.GetInt("TEXTCTRL_Music_EndNote", 127)   
		);
}

// represents a music event for a note
class MusicEvent
{
	public:
		int _startframe;
		int _duration; // in frames
		xlColor _colour1;
		xlColor _colour2;
		MusicEvent(int startframe, int duration, const xlColor& c1, const xlColor& c2)
		{
			_startframe = startframe;
			_duration = duration;
			_colour1 = c1;
			_colour2 = c2;
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
    int _sensitivity;
	int _offsetx; // skip these many cols at left of model
	bool _scale; // scale out to fill model horizontally
    bool _freqRelative; // scale based on maximum volume within frequency ... otherwise scale on maximum volume across all frequencies
};

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
    int startnote, int endnote)
{
    // no point if we have no media
    if (buffer.GetMedia() == NULL)
    {
        return;
    }

    int nType = DecodeType(type);

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
    std::vector<std::list<MusicEvent*>*>& _events = cache->_events;

    // Check for config changes which require us to reset
    if (buffer.needToInit || _bars != bars || _type != nType || _startNote != startnote || _endNote != endnote || _offsetx != offsetx || _scale != scale || _freqRelative != freqrelative || _sensitivity != sensitivity)
    {
        buffer.needToInit = false;
        _bars = bars;
        _startNote = startnote;
        _endNote = endnote;
        _type = nType;
        _offsetx = offsetx;
        _scale = scale;
        _sensitivity = sensitivity;
        _freqRelative = freqrelative;
        cache->ClearEvents();
        // We limit bars to the width of the model less the x offset
        if (_bars > buffer.BufferWi - _offsetx)
        {
            _bars = buffer.BufferWi - _offsetx;
        }

        CreateEvents(buffer, _events, _startNote, _endNote, _bars, _freqRelative, _sensitivity);
    }

    int n = std::min(_bars, _endNote - _startNote + 1);

    int per = 1;
    if (_scale)
    {
        per = _bars / n;
        if (_events.size() * per != _bars)
        {
            // fewer data points than requested ... this is ok ... just empty columns will exist
            int a = 0;
        }

    }
    else
    {
        if (_events.size() != _bars)
        {
            // fewer data points than requested ... this is ok ... just empty columns will exist
            int a = 0;
        }
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
                    RenderMorph(buffer, (x*(i+1)) + i + _offsetx, _bars, _startNote, _endNote, true /* in */, *_events[x]);
                    break;
                case 2:
                    RenderMorph(buffer, (x*(i+1)) + i + _offsetx, _bars, _startNote, _endNote, false /* out */, *_events[x]);
                    break;
                case 3:
                    RenderCollide(buffer, (x*(i+1)) + i + _offsetx, _bars, _startNote, _endNote, true /* collide */, *_events[x]);
                    break;
                case 4:
                    RenderCollide(buffer, (x*(i+1)) + i + _offsetx, _bars, _startNote, _endNote, false /* uncollide */,* _events[x]);
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

void MusicEffect::CreateEvents(RenderBuffer& buffer, std::vector<std::list<MusicEvent*>*>& events, int startNote, int endNote, int bars, bool freqRelative, int sensitivity)
{
    // must have media
    if (buffer.GetMedia() == NULL)
    {
        return;
    }

    float notesperbar = ((float)endNote - (float)startNote + 1.0) / (float)bars;
    float barspernote = (float)bars / ((float)endNote - (float)startNote + 1.0);

    xlColor c1, c2;
    buffer.palette.GetColor(0, c1);
    if (buffer.GetColorCount() > 1)
    {
        buffer.palette.GetColor(1, c2);
    }
    else
    {
        c2 = xlWHITE;
    }

    if (notesperbar < 1.0)
    {
        // use bars per note
        // use multiple output strings per note of data
        std::map<int /*note*/, std::map<int /* frame */, float>> data;
        std::map<int /* note */, float> max;

        // zero out the maximums
        for (int n = startNote; n <= endNote; n++)
        {
            max[n] = 0.0;
        }

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

            for (int n = startNote; n <= endNote; n++)
            {
                data[n][f] = *pn;
                ++pn;
                max[n] = std::max(max[n], *pn);
            }
        }

        int string = 0;
        for (int n = startNote; n <= endNote; n++)
        {            
            events.push_back(new std::list<MusicEvent*>());
            float notesensitivity;
            if (freqRelative)
            {
                notesensitivity = sensitivity * max[n];
            }
            else
            {
                notesensitivity = sensitivity;
            }
            int startframe = -1;
            // extract the value for this note
            int frame = 0;
            for (auto f = data[n].begin(); f != data[n].end(); ++f)
            {
                if (f->second > notesensitivity)
                {
                    startframe = frame;

                    while (f != data[n].end() && f->second > notesensitivity)
                    {
                        ++f;
                        frame++;
                    }
                    --f;
                    frame--;
                    events[string]->push_back(new MusicEvent(startframe, frame - startframe, c1, c2));
                }
                frame++;
            }
            string++;
        }
    }
    else
    {
        // use notes per bar
        // use multiple notes of data per output string using the maximum of these notes intensities
        std::map<int /*string*/, std::map<int /* frame */, float>> data;
        std::map<int /* string */, float> max;

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
                    events[b]->push_back(new MusicEvent(startframe, frame - startframe, c1, c2));
                }
                frame++;
            }
        }
    }
}

void MusicEffect::RenderMorph(RenderBuffer &buffer, int x, int bars, int startNote, int endNote, bool in,std::list<MusicEvent*>& events)
{
}

void MusicEffect::RenderCollide(RenderBuffer &buffer, int x, int bars, int startNote, int endNote, bool in, std::list<MusicEvent*>& events)
{
    for (auto it = events.begin(); it != events.end(); ++it)
    {
        if ((*it)->IsEventActive(buffer.curPeriod))
        {
            float progress = (*it)->OffsetInDuration(buffer.curPeriod);

            int mid = buffer.BufferHt / 2;
            int leftstart, leftend;
            if (in)
            {
                leftstart = (progress - 0.5) * buffer.BufferHt;
                leftend = progress * buffer.BufferHt;
                if (leftend > mid)
                {
                    leftend = mid;
                }
            }
            else
            {
                leftstart = mid - progress * buffer.BufferHt;
                leftend = mid - (progress + 0.5) * buffer.BufferHt;
                if (leftend > mid)
                {
                    leftend = mid;
                }
            }

            for (int y = leftstart; y < leftend; y++)
            {
                buffer.SetPixel(x, y, (*it)->_colour1);
                buffer.SetPixel(x, mid - y + mid, (*it)->_colour1);
            }
        }
    }
}
