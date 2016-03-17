#include "PianoEffect.h"

#include "PianoPanel.h"
#include "../AudioManager.h"

#include "../sequencer/Effect.h"
#include "../RenderBuffer.h"
#include "../UtilClasses.h"

#include "../../include/piano.xpm"

PianoEffect::PianoEffect(int id) : RenderableEffect(id, "Piano", piano, piano, piano, piano, piano)
{
    //ctor
}

PianoEffect::~PianoEffect()
{
    //dtor
}

wxPanel *PianoEffect::CreatePanel(wxWindow *parent) {
    return new PianoPanel(parent);
}

void PianoEffect::Render(Effect *effect, const SettingsMap &SettingsMap, RenderBuffer &buffer) {
    RenderPiano(buffer,
		SettingsMap.GetInt("SPINCTRL_Piano_StartMIDI"),
		SettingsMap.GetInt("SPINCTRL_Piano_EndMIDI"),
		SettingsMap.GetBool("CHECKBOX_Piano_ShowSharps")
        );
}

class PianoCache : public EffectRenderCache 
{
public:
	PianoCache() { };
	virtual ~PianoCache() { };

	int _startMidiChannel;
	int _endMidiChannel;
	bool _showSharps;
};

//render piano fx during sequence:
void PianoEffect::RenderPiano(RenderBuffer &buffer, const int startmidi, const int endmidi, const bool sharps)
{
	buffer.drawingContext->Clear();

	PianoCache *cache = (PianoCache*)buffer.infoCache[id];
	if (cache == nullptr) {
		cache = new PianoCache();
		buffer.infoCache[id] = cache;
	}

	int& _startMidiChannel = cache->_startMidiChannel;
	int& _endMidiChannel = cache->_endMidiChannel;
	bool& _showSharps = cache->_showSharps;

	if (_startMidiChannel != startmidi ||
		_endMidiChannel != endmidi ||
		_showSharps != sharps)
	{
		_startMidiChannel = startmidi;
		_endMidiChannel = endmidi;
		_showSharps = sharps;
	}

	std::list<float>* pdata = buffer.GetMedia()->GetFrameData(buffer.curPeriod, FRAMEDATA_NOTES , "");

	ReduceChannels(pdata, _startMidiChannel, _endMidiChannel, _showSharps);

	DrawPiano(buffer, pdata, _showSharps, _startMidiChannel, _endMidiChannel);
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

void PianoEffect::ReduceChannels(std::list<float>* pdata, int start, int end, bool sharps)
{
	auto it = pdata->begin();
	while (it != pdata->end())
	{
		if (!sharps && IsSharp(*it))
		{
			float n = *it - 1.0;
			bool found = false;
			for (auto c = pdata->begin(); c != pdata->end(); ++c)
			{
				if (*c == n)
				{
					// already there
					found = true;
					break;
				}
			}
			if (!found)
			{
				pdata->push_back(n);
			}
			pdata->remove(*it);
			it = pdata->begin();
		}

		if (*it < start || *it > end)
		{
			pdata->remove(*it);
			it = pdata->begin();
		}
	}
}

void PianoEffect::DrawPiano(RenderBuffer &buffer, std::list<float>* pdata, bool sharps, int start, int end)
{
	xlColor wkc, bkc, wkdc, bkdc, kb;

	// count the keys
	int wkeys = 0;
	int bkeys = 0;

	int wkw = buffer.BufferWi / (end - start + 1);
	bool border = false;
	if (wkw > 3)
	{
		border = true;
		wkw--;
	}
	int bkw = wkw;

	// Get the colours
	if (buffer.GetColorCount() > 0)
	{
		buffer.palette.GetColor(0, wkc);
	}
	else
	{
		wkc = xlWHITE;
	}
	if (buffer.GetColorCount() > 1)
	{
		buffer.palette.GetColor(0, bkc);
	}
	else
	{
		bkc = xlBLACK;
	}
	if (buffer.GetColorCount() > 2)
	{
		buffer.palette.GetColor(0, wkdc);
	}
	else
	{
		wkdc = xlMAGENTA;
	}
	if (buffer.GetColorCount() > 3)
	{
		buffer.palette.GetColor(0, bkdc);
	}
	else
	{
		bkdc = xlMAGENTA;
	}
	if (buffer.GetColorCount() > 4)
	{
		buffer.palette.GetColor(0, kb);
	}
	else
	{
		kb = xlLIGHT_GREY;
	}

	// Draw white keys
	int x = 0;
	for (int i = start; i <= end; i++)
	{
		if (!IsSharp(i))
		{
			buffer.DrawBox(x, 0, x + wkw, buffer.BufferHt, wkc, false);
			x += wkw + (border ? 1 : 0);
		}
	}
	// Draw the pressed white keys

	// Draw white key borders
	if (border)
	{
		x = wkw + 1;
		for (int i = 0; i < (end - start + 1); i++)
		{
			buffer.DrawLine(x, 0, x, buffer.BufferHt, kb);
			x += wkw + 1;
		}
	}

	// Draw the black keys
	x = wkw / 2;
	int c = start;
	for (int i = 0; i < (end - start + 1); i++)
	{
		buffer.DrawBox(x, 0, x + wkw, buffer.BufferHt, wkc, false);
		x += wkw + (border ? 1 : 0);
	}

	// Draw the Black key borders
}