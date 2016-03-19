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
		else if (*it < start || *it > end)
		{
			pdata->remove(*it);
			it = pdata->begin();
		}
		else
		{
			it++;
		}
	}
}

bool PianoEffect::KeyDown(std::list<float>* pdata, int ch)
{
	for (auto it = pdata->begin(); it != pdata->end(); ++it)
	{
		if (ch == (int)(*it))
		{
			return true;
		}
	}

	return false;
}

void PianoEffect::DrawPiano(RenderBuffer &buffer, std::list<float>* pdata, bool sharps, int start, int end)
{
	xlColor wkcolour, bkcolour, wkdcolour, bkdcolour, kbcolour;

	// count the keys
	int wkeys = 0;
	int bkeys = 0;

	int sharpstart = -1;
	int i = start;
	while (sharpstart == -1 && i <= end)
	{
		if (IsSharp(i))
		{
			sharpstart = i;
		}
		i++;
	}

	int sharpend = -1;
	i = end;
	while (sharpend == -1 && i >= start)
	{
		if (IsSharp(i))
		{
			sharpend = i;
		}
		i--;
	}

	int whitestart = -1;
	i = start;
	while (whitestart == -1 && i <= end)
	{
		if (!IsSharp(i))
		{
			whitestart = i;
		}
		i++;
	}

	int whiteend = -1;
	i = end;
	while (whiteend == -1 && i >= start)
	{
		if (!IsSharp(i))
		{
			whiteend = i;
		}
		i--;
	}

	int wkcount = 0;
	if (whitestart != -1 && whiteend != -1)
	{
		for (i = whitestart; i <= whiteend; i++)
		{
			if (!IsSharp(i))
			{
				wkcount++;
			}
		}
	}

	int fwkw = buffer.BufferWi / wkcount;
	int wkw = fwkw;
	bool border = false;
	if (wkw > 3)
	{
		border = true;
		wkw--;
	}

	// Get the colours
	if (buffer.GetColorCount() > 0)
	{
		buffer.palette.GetColor(0, wkcolour);
	}
	else
	{
		wkcolour = xlWHITE;
	}
	if (buffer.GetColorCount() > 1)
	{
		buffer.palette.GetColor(1, bkcolour);
	}
	else
	{
		bkcolour = xlBLACK;
	}
	if (buffer.GetColorCount() > 2)
	{
		buffer.palette.GetColor(2, wkdcolour);
	}
	else
	{
		wkdcolour = xlMAGENTA;
	}
	if (buffer.GetColorCount() > 3)
	{
		buffer.palette.GetColor(3, bkdcolour);
	}
	else
	{
		bkdcolour = xlMAGENTA;
	}
	if (buffer.GetColorCount() > 4)
	{
		buffer.palette.GetColor(4, kbcolour);
	}
	else
	{
		kbcolour = xlLIGHT_GREY;
	}

	// Draw white keys
	int x = 0;
	for (int i = start; i <= end; i++)
	{
		if (!IsSharp(i))
		{
			if (KeyDown(pdata, i))
			{
				buffer.DrawBox(x, 0, x + wkw, buffer.BufferHt, wkdcolour, false);
			}
			else
			{
				buffer.DrawBox(x, 0, x + wkw, buffer.BufferHt, wkcolour, false);
			}
			x += fwkw;
		}
	}
	// Draw the pressed white keys

	// Draw white key borders
	if (border)
	{
		x = fwkw;
		for (int i = 0; i < wkcount; i++)
		{
			buffer.DrawLine(x, 0, x, buffer.BufferHt, kbcolour);
			x += fwkw;
		}
	}

#define BKADJUSTMENTWIDTH(a) (int)(0.3 / 2.0 * (float)a)
	// Draw the black keys
	if (IsSharp(start))
	{
		x = -1 * fwkw / 2;
	}
	else if (IsSharp(start + 1))
	{
		x = fwkw / 2;
	}
	else
	{
		x = fwkw + fwkw / 2;
	}
	for (i = start; i <= end; i++)
	{
		if (IsSharp(i))
		{
			if (KeyDown(pdata, i))
			{
				buffer.DrawBox(x + BKADJUSTMENTWIDTH(fwkw), buffer.BufferHt / 2, x + fwkw - BKADJUSTMENTWIDTH(fwkw), buffer.BufferHt, bkdcolour, false);
			}
			else
			{
				buffer.DrawBox(x + BKADJUSTMENTWIDTH(fwkw), buffer.BufferHt / 2, x + fwkw - BKADJUSTMENTWIDTH(fwkw), buffer.BufferHt, bkcolour, false);
			}
			if (!IsSharp(i + 1) && !IsSharp(i + 2))
			{
				x += fwkw + fwkw;
			}
			else
			{
				x += fwkw;
			}
		}
	}
}