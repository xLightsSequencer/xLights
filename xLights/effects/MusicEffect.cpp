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

#ifdef __WXMSW__
//#include "wx/msw/debughlp.h"
//wxString s;
//s.Printf("%f -> %f", val, db);
//wxDbgHelpDLL::LogError(s);
#endif

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

void MusicEffect::SetDefaultParameters(Model *cls) 
{
	// Validate the window (includes enabling and disabling controls)
	fp->ValidateWindow();
}

void MusicEffect::Render(Effect *effect, const SettingsMap &SettingsMap, RenderBuffer &buffer) {
	Render(buffer,
           SettingsMap.GetInt("TEXTCTRL_Music_Bars", 6),
		   SettingsMap.Get("CHOICE_Music_Type", "Waveform"),
		   SettingsMap.Get("CHOICE_Music_TimingTrack", ""),
		   SettingsMap.GetInt("TEXTCTRL_Music_Sensitivity", 70),
 		   SettingsMap.Get("CHOICE_Music_Shape", "Circle"),
		   SettingsMap.GetBool("CHECKBOX_Music_SlowDownFalls", TRUE),
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
} 

class MusicRenderCache : public EffectRenderCache 
{
public:
	void ClearEvents()
	{
		// delete all our music events
		for(int i = 0; i < _events.size(); i++)
		{
			for (auto it = _events[i].begin(); it != _events[i].end(); ++it)
			{
				delete it;
			}
			_events[i].clear();
		}
		_events.clear();
	}
    MusicRenderCache() 
	{
	};
    virtual ~MusicRenderCache() {
	};
	std::vector<std::list<MusicEvent*>> _events;
	int _bars;
    int _startNote;
    int _endNote;
	int _type;
	int _offsetx; // skip these many cols at left of model
	bool _scale; // scale out to fill model horizontally
	bool _freqRelative // scale based on maximum volume within frequency ... otherwise scale on maximum volume across all frequencies
};

int MusicEffect::DecodeType(std::string type)
{
	if (type == "Morphs In")
	{
		return 1;
	}
	else if (type == "Morphs Out")
	{
		return 2;
	}

	// default type is volume bars
	return 1;
}

void MusicEffect::Render(RenderBuffer &buffer, int bars, const std::string& type, const std::string &timingtrack, int sensitivity, const std::string& shape, bool slowdownfalls, int startnote, int endnote)
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
	bool &_freqRelative = cache->_freqRelative;
	std::vector<std::list<MusicEvent*>>& _events = cache->_events;

	// Check for config changes which require us to reset
	if (_bars != bars || _type != nType || _startNote != startnote || _endNote != endnote || _offsetx != offsetx || _scale != scale || _freqRelative != freqRelative)
	{
		_bars = bars;
        _startNote = startnote;
        _endNote = endnote;
		_type = nType;
		_offsetx = offsetx;
		_scale = scale;
		_freqRelative = freqRelative;
		_events.ClearEvents();
		// We limit bars to the width of the model less the x offset
		if (_bars > buffer.BufferWi - _offsetx)
		{
			_bars = buffer.BufferWi - _offsetx;
		}
		
		CreateEvents(buffer, _events, _startNote, _endNote, _bars, _freqRelative);
	}

	int n = std::min(_bars, _endNote - _startNote + 1);
	
	int per = 1;
	if (_scale)
	{
		per = _bars / n;
	}
	
	try
	{
		for (int x = 0; x < n; x++)
		{
			for (int i = 0; i < per; i++)
			{
				switch (_type)
				{
				case 1:
					RenderMorph(buffer, x + _offsetx, _events, _bars, _startNote, _endNote, true /* in */, _events[x]);
					break;
				case 2:
					RenderMorph(buffer, x + _offsetx, _events, _bars, _startNote, _endNote, false /* out */, _events[x]);
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

void MusicEffect::RenderMorph(RenderBuffer &buffer, int x, int bars, int startNote, int endNote, bool in,std::list<MusicEvent*>& events)
{
	std::list<float>* pdata = buffer.GetMedia()->GetFrameData(buffer.curPeriod, FRAMEDATA_VU, "");

	if (pdata != NULL && pdata->size() != 0)
	{
		if (slowdownfalls)
		{
			if (lastvalues.size() == 0)
			{
				lastvalues = *pdata;
			}
			else
			{
				std::list<float>::iterator newdata = pdata->begin();
				std::list<float>::iterator olddata = lastvalues.begin();

				while (olddata != lastvalues.end())
				{
					if (*newdata < *olddata)
					{
						*olddata = *olddata - 0.05;
						if (*olddata < *newdata)
						{
							*olddata = *newdata;
						}
					}
					else
					{
						*olddata = *newdata;
					}

					++olddata;
					++newdata;
				}
			}
		}
		else
		{
			lastvalues = *pdata;
		}

        int datapoints = std::min((int)pdata->size(), endNote - startNote + 1);

		if (usebars > datapoints)
		{
			usebars = datapoints;
		}

		int per = datapoints / usebars;
		int cols = buffer.BufferWi / usebars;

		std::list<float>::iterator it = lastvalues.begin();

        // skip to our start note
        for (int i = 0; i < startNote; i++)
        {
            ++it;
        }

		int x = 0;

		for (int j = 0; j < usebars; j++)
		{
			float f = 0;
			for (int k = 0; k < per; k++)
			{
				// use the max within the frequency range
				if (*it > f)
				{
					f = *it;
				}
				++it;
                // dont let it go off the end
                if (it == lastvalues.end())
                {
                    --it;
                }
			}
			for (int k = 0; k < cols; k++)
			{
				for (int y = 0; y < buffer.BufferHt; y++)
				{
					int colheight = buffer.BufferHt * f;
					if (y < colheight)
					{
						xlColor color1;
						// an alternate colouring
						//buffer.GetMultiColorBlend((double)y / (double)colheight, false, color1);
						buffer.GetMultiColorBlend((double)y / (double)buffer.BufferHt, false, color1);
						buffer.SetPixel(x, y, color1);
					}
					else
					{
						break;
					}
				}
				x++;
			}
		}
	}
}
