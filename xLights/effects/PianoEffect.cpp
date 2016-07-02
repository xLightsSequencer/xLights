#include "PianoEffect.h"

#include "PianoPanel.h"
#include "../AudioManager.h"

#include "../sequencer/Effect.h"
#include "../RenderBuffer.h"
#include "../UtilClasses.h"
#include "../xLightsMain.h"
#include "../xLightsXmlFile.h"

#include <vector>

#include "../../include/piano-16.xpm"
#include "../../include/piano-64.xpm"

#include <log4cpp/Category.hh>
#include "../MIDI/MidiFile.h"

PianoEffect::PianoEffect(int id) : RenderableEffect(id, "Piano", piano_16, piano_64, piano_64, piano_64, piano_64)
{
    //ctor
	_panel = NULL;
}

PianoEffect::~PianoEffect()
{
    //dtor
}

void PianoEffect::SetPanelStatus(Model *cls)
{
    PianoPanel *fp = (PianoPanel*)panel;
    if (fp == nullptr)
    {
        return;
    }

    // Tell our panel the media file in case we need to do polyphonic transcription
    xLightsFrame* frame = (xLightsFrame*)fp->GetParent();
    if (frame != NULL && frame->CurrentSeqXmlFile != NULL)
        ((PianoPanel*)_panel)->SetAudio(frame->CurrentSeqXmlFile->GetMedia());

    if (mSequenceElements == nullptr)
    {
        return;
    }

    // Load the names of the timing tracks
    std::list<std::string> timingtracks;
    for (size_t i = 0; i < mSequenceElements->GetElementCount(); i++)
    {
        Element* e = mSequenceElements->GetElement(i);
        if (e->GetEffectLayerCount() == 1 && e->GetType() == "timing")
        {
            timingtracks.push_back(e->GetName());
        }
    }

    // Select the first one
    fp->SetTimingTrack(timingtracks);
    fp->TimingTrackValidateWindow();
}

void PianoEffect::adjustSettings(const std::string &version, Effect *effect)
{
    // give the base class a chance to adjust any settings
    if (RenderableEffect::needToAdjustSettings(version))
    {
        RenderableEffect::adjustSettings(version, effect);
    }

    SettingsMap &settings = effect->GetSettings();
    std::string file = settings["E_TEXTCTRL_Piano_File"];

    if (file != "")
    {
        if (!wxFile::Exists(file))
        {
            settings["E_TEXTCTRL_Piano_File"] = xLightsXmlFile::FixFile("", file);
        }
    }
}

wxPanel *PianoEffect::CreatePanel(wxWindow *parent) {
    _panel = new PianoPanel(parent);
	return _panel;
}

bool PianoEffect::CanRenderOnBackgroundThread(Effect *effect, const SettingsMap &settings, RenderBuffer &buffer) {
    if (settings.Get("CHOICE_Piano_Notes_Source", "Polyphonic Transcription") == "Polyphonic Transcription"
        && buffer.GetMedia() != nullptr) {
        //if the Polyphonic data isn't there, we need to flip to the main thread to get that done
        return buffer.GetMedia()->IsPolyphonicTranscriptionDone();
    }
    return true;
}

void PianoEffect::SetDefaultParameters(Model *cls) {
    PianoPanel *pp = (PianoPanel*)panel;
    if (pp == nullptr) {
        return;
    }

    pp->BitmapButton_Piano_ScaleVC->SetActive(false);

    SetChoiceValue(pp->Choice_Piano_Type, "True Piano");
    SetSpinValue(pp->SpinCtrl_Piano_StartMIDI, 60);
    SetSpinValue(pp->SpinCtrl_Piano_EndMIDI, 72);
    SetCheckBoxValue(pp->CheckBox_Piano_ShowSharps, true);
    SetSliderValue(pp->Slider_Piano_Scale, 100);
    SetSliderValue(pp->Slider_Piano_XOffset, 0);
}

void PianoEffect::Render(Effect *effect, const SettingsMap &SettingsMap, RenderBuffer &buffer) {
    float oset = buffer.GetEffectTimeIntervalPosition();
    RenderPiano(buffer,
        effect->GetParentEffectLayer()->GetParentElement()->GetSequenceElements(),
        SettingsMap.GetInt("SPINCTRL_Piano_StartMIDI"),
		SettingsMap.GetInt("SPINCTRL_Piano_EndMIDI"),
		SettingsMap.GetBool("CHECKBOX_Piano_ShowSharps"),
		std::string(SettingsMap.Get("CHOICE_Piano_Type", "True Piano")),
		std::string(SettingsMap.Get("CHOICE_Piano_Notes_Source", "Polyphonic Transcription")),
		std::string(SettingsMap.Get("TEXTCTRL_Piano_File", "")),
		SettingsMap.GetInt("SLIDER_Piano_MIDI_Start"),
		SettingsMap.GetInt("SLIDER_Piano_MIDI_Speed"),
		GetValueCurveInt("Piano_Scale", 100, SettingsMap, oset),
		std::string(SettingsMap.Get("CHOICE_Piano_MIDITrack_APPLYLAST", "")),
        SettingsMap.GetInt("SLIDER_Piano_XOffset", 0)
        );
}

class PianoCache : public EffectRenderCache
{
public:
	PianoCache() { };
	virtual ~PianoCache() { };

	std::string _timingsource;
	std::string _file;
	int _MIDIStartAdjust;
	int _MIDISpeedAdjust;
	std::map<int, std::list<float>> _timings;
	std::string _MIDItrack;
};

//render piano fx during sequence:
void PianoEffect::RenderPiano(RenderBuffer &buffer, SequenceElements *elements, const int startmidi, const int endmidi, const bool sharps, const std::string type, std::string timingsource, std::string file, int MIDIAdjustStart, int MIDIAdjustSpeed, int scale, std::string MIDITrack, int xoffset)
{
	PianoCache *cache = (PianoCache*)buffer.infoCache[id];
	if (cache == nullptr) {
		cache = new PianoCache();
		buffer.infoCache[id] = cache;
	}

	std::string& _timingsource = cache->_timingsource;
	std::string& _file = cache->_file;
	int& _MIDISpeedAdjust = cache->_MIDISpeedAdjust;
	int& _MIDIStartAdjust = cache->_MIDIStartAdjust;
	std::map<int, std::list<float>>& _timings = cache->_timings;
	std::string& _MIDITrack = cache->_MIDItrack;

	if (buffer.needToInit)
	{
        buffer.needToInit = false;
		if (timingsource == "Audacity Timing File" && (_timingsource != timingsource || _file != file))
		{
			// need to load timings
			_timings.clear();
			if (wxFile::Exists(file))
			{
				_timings = LoadAudacityFile(file, buffer.frameTimeInMs);
			}
        }
        else if (timingsource == "MIDI File" && (_timingsource != timingsource || _file != file || _MIDITrack != MIDITrack || _MIDISpeedAdjust != MIDIAdjustSpeed || _MIDIStartAdjust != MIDIAdjustStart))
        {
            // need to load timings
            _timings.clear();
            if (wxFile::Exists(file))
            {
                _timings = LoadMIDIFile(file, buffer.frameTimeInMs, MIDIAdjustSpeed, MIDIAdjustStart * 10, MIDITrack);
            }
        }
        else if (timingsource == "Timing Track" && (_timingsource != timingsource || _MIDITrack != MIDITrack))
        {
            _timings.clear();
            _timings = LoadTimingTrack(MIDITrack, buffer.frameTimeInMs);
            elements->AddRenderDependency(MIDITrack, buffer.cur_model);
        }

		_timingsource = timingsource;
		_MIDISpeedAdjust = MIDIAdjustSpeed;
		_MIDIStartAdjust = MIDIAdjustStart;
		_file = file;
		_MIDITrack = MIDITrack;
	}

    int em = endmidi;

    // end midi must not be less than start midi
    if (em < startmidi)
    {
        em = startmidi;
    }

	if (em - startmidi + 1 > buffer.BufferWi)
	{
		em = startmidi + buffer.BufferWi - 1;
	}

    std::list<float> def;
	std::list<float>* pdata = NULL;

	if (_timingsource == "Polyphonic Transcription")
	{
		if (buffer.GetMedia() != NULL)
		{
			pdata = buffer.GetMedia()->GetFrameData(buffer.curPeriod, FRAMEDATA_NOTES, "");
		}
	}
	else
	{
		int time = buffer.curPeriod * buffer.frameTimeInMs;
		if (_timings.find(time) != _timings.end())
		{
			pdata = &_timings[time];
		}
	}

    if (pdata == NULL)
    {
        pdata = &def;
    }

	ReduceChannels(pdata, startmidi, em, sharps);

	if (type == "True Piano")
	{
		DrawTruePiano(buffer, pdata, sharps, startmidi, em, scale, xoffset);
	}
	else if (type == "Bars")
	{
		DrawBarsPiano(buffer, pdata, sharps, startmidi, em, scale, xoffset);
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

void PianoEffect::DrawTruePiano(RenderBuffer &buffer, std::list<float>* pdata, bool sharps, int start, int end, int scale, int xoffset)
{
    int truexoffset = xoffset * buffer.BufferWi / 100;
	xlColor wkcolour, bkcolour, wkdcolour, bkdcolour, kbcolour;

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
	int maxx = wkcount * fwkw;
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
	int x = truexoffset;
	for (int i = start; i <= end; i++)
	{
		if (!IsSharp(i))
		{
			if (KeyDown(pdata, i))
			{
				buffer.DrawBox(x, 0, x + wkw, buffer.BufferHt * scale / 100, wkdcolour, false);
			}
			else
			{
				buffer.DrawBox(x, 0, x + wkw, buffer.BufferHt *scale / 100, wkcolour, false);
			}
			x += fwkw;
		}
	}

	// Draw white key borders
	if (border)
	{
		x = fwkw + truexoffset;
		for (int i = 0; i < wkcount; i++)
		{
			buffer.DrawLine(x, 0, x, buffer.BufferHt * scale / 100, kbcolour);
			x += fwkw;
		}
	}

	if (sharps)
	{
		#define BKADJUSTMENTWIDTH(a) (int)std::round(0.3 / 2.0 * (float)a)
		// Draw the black keys
		if (IsSharp(start))
		{
			x = -1 * fwkw / 2 + truexoffset;
		}
		else if (IsSharp(start + 1))
		{
			x = fwkw / 2 + truexoffset;
		}
		else
		{
			x = fwkw + fwkw / 2 + truexoffset;
		}
		for (i = start; i <= end; i++)
		{
			if (IsSharp(i))
			{
				if (KeyDown(pdata, i))
				{
					buffer.DrawBox(x + BKADJUSTMENTWIDTH(fwkw), buffer.BufferHt * scale / 200, std::min(maxx, x + fwkw - BKADJUSTMENTWIDTH(fwkw)), buffer.BufferHt * scale / 100, bkdcolour, false);
				}
				else
				{
					buffer.DrawBox(x + BKADJUSTMENTWIDTH(fwkw), buffer.BufferHt * scale / 200, std::min(maxx, x + fwkw - BKADJUSTMENTWIDTH(fwkw)), buffer.BufferHt * scale / 100, bkcolour, false);
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
}

void PianoEffect::DrawBarsPiano(RenderBuffer &buffer, std::list<float>* pdata, bool sharps, int start, int end, int scale, int xoffset)
{
    int truexoffset = xoffset * buffer.BufferWi / 100;
    xlColor wkcolour, bkcolour, wkdcolour, bkdcolour;

	// count the keys
	int kcount = -1;
	if (sharps)
	{
		kcount = end - start + 1;
	}
	else
	{
		for (int i = start; i <= end; i++)
		{
			if (!IsSharp(i))
			{
				kcount++;
			}
		}
	}
	int fwkw = buffer.BufferWi / kcount;

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

	// Draw keys
	int x = 0 + truexoffset;
	int wkh = buffer.BufferHt;
	if (sharps)
	{
		wkh = buffer.BufferHt * 2.0 * scale / 300.0;
	}
	int bkb = buffer.BufferHt * scale / 300.0;
	for (int i = start; i <= end; i++)
	{
		if (!IsSharp(i))
		{
			if (KeyDown(pdata, i))
			{
				buffer.DrawBox(x, 0, x + fwkw - 1, wkh, wkdcolour, false);
			}
			else
			{
				buffer.DrawBox(x, 0, x + fwkw - 1, wkh, wkcolour, false);
			}
			x += fwkw;
		}
		else
		{
			if (sharps)
			{
				if (KeyDown(pdata, i))
				{
					buffer.DrawBox(x, bkb, x + fwkw - 1, buffer.BufferHt * scale / 100, bkdcolour, false);
				}
				else
				{
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
	while (s.Len() != 0)
	{
		int end = s.First('\t');
		if (end > 0)
		{
			res.push_back(wxAtof(s.SubString(0, end - 1)));
			s = s.Right(s.Len() - end - 1);
		}
		else
		{
			res.push_back(wxAtof(s));
			s = "";
		}
	}

	return res;
}

int PianoEffect::LowerTS(float t, int intervalMS)
{
	int res = t * 1000;
	res = res - (res % intervalMS);
    if (t * 1000 - res > intervalMS / 2)
    {
        res += intervalMS;
    }
	return res;
}
int PianoEffect::UpperTS(float t, int intervalMS)
{
	int res = t * 1000;
	res = res + (intervalMS - res % intervalMS);
	return res;
}

std::map<int, std::list<float>> PianoEffect::LoadAudacityFile(std::string file, int intervalMS)
{
    log4cpp::Category &logger_pianodata = log4cpp::Category::getInstance(std::string("log_pianodata"));
    std::map<int, std::list<float>> res;

    logger_pianodata.debug("Processing audacity file " + file);
    logger_pianodata.debug("Interval %d.", intervalMS);
    logger_pianodata.debug("Start,End,midinote");

	wxTextFile f(file);

	if (f.Open())
	{
		while (!f.Eof())
		{
			wxString l = f.GetNextLine();
			std::vector<float> components = Parse(l);
			if (components.size() != 3)
			{
				// this is a problem ... there should be 3 floating point numbers
                logger_pianodata.warn("Invalid data in audacity file - 3 tab separated floating point values expected: '" + l + "'");
				break;
			}
			else
			{
                logger_pianodata.debug("%f,%f,%f", components[0], components[1], components[2]);
                int start = LowerTS(components[0], intervalMS);
				int end = UpperTS(components[1], intervalMS);
				for (int i = start; i <= end; i += intervalMS)
				{
					if (res.find(i) == res.end())
					{
						std::list<float> f;
						f.push_back(components[2]);
						res[i] = f;
					}
					else
					{
                        bool found = false;
                        for (auto it = res[i].begin(); it != res[i].end(); ++it)
                        {
                            if (*it == components[2])
                            {
                                found = true;
                                break;
                            }
                        }
                        if (!found)
                        {
                            res[i].push_back(components[2]);
                        }
                    }
				}
			}
		}

        if (logger_pianodata.isDebugEnabled())
        {
            logger_pianodata.debug("Piano data calculated:");
            logger_pianodata.debug("Time MS, Keys");
            for (auto it = res.begin(); it != res.end(); ++it)
            {
                std::string keys = "";
                for (auto it2 = it->second.begin(); it2 != it->second.end(); ++it2)
                {
                    keys += " " + std::string(wxString::Format("%f", *it2).c_str());
                }
                logger_pianodata.debug("%d,%s", it->first, keys.c_str());
            }
        }
    }

	return res;
}

std::map<int, std::list<float>> PianoEffect::LoadMIDIFile(std::string file, int intervalMS, int speedAdjust, int startAdjustMS, std::string track)
{
    log4cpp::Category &logger_pianodata = log4cpp::Category::getInstance(std::string("log_pianodata"));
    std::map<int, std::list<float>> res;

	float speedadjust;
	if (speedAdjust < 0)
	{
		speedadjust = speedAdjust / 200.0 + 1;
	}
	else
	{
		speedadjust = (speedAdjust + 100.0) / 100.0;
	}

	int notestate[128];
	for (int i = 0; i <= 127; i++)
	{
		notestate[i] = 0;
	}

    logger_pianodata.debug("Processing midi file " + file);
    logger_pianodata.debug("Interval %d.", intervalMS);
    logger_pianodata.debug("SpeedAdjust %d.", speedAdjust);
    logger_pianodata.debug("StartAdjustMS %d.", startAdjustMS);

	MidiFile midifile;
	float lasttime = -1;
	if (midifile.read(file) != 0)
	{
		int ntrack = 0;
		if (track == "All" || track == "")
		{
			midifile.joinTracks();
		}
		else
		{
			ntrack = wxAtoi(track) - 1;
			if (ntrack >= midifile.getNumTracks())
			{
				ntrack = 0;
			}
		}
		midifile.doTimeAnalysis();

        logger_pianodata.debug("Processing midi track %d.", ntrack);
        logger_pianodata.debug("Event,time(s),adjustedtime(s),isnote,isnoteon,isnoteoff,midinote");

		// process each event
		for (int i = 0; i < midifile.getNumEvents(ntrack); i++)
		{
			MidiEvent e = midifile.getEvent(ntrack, i);

            float time = startAdjustMS + midifile.getTimeInSeconds(ntrack, i) * speedadjust;

            if (logger_pianodata.isDebugEnabled())
            {
                logger_pianodata.debug("%d,%f,%f,%d,%d,%d,%d", i, midifile.getTimeInSeconds(ntrack, i), time, e.isNote(), e.isNoteOn(), e.isNoteOff(), e.getKeyNumber());
            }
			if (time != lasttime)
			{
				if (lasttime >= 0)
				{
					// we can update things now
					int start = LowerTS(lasttime, intervalMS);
					int end = UpperTS(time, intervalMS);

					for (int j = start; j < end; j += intervalMS)
					{
						std::list<float> f;
						for (int k = 0; k <= 127; k++)
						{
							if (notestate[k] > 0)
							{
								f.push_back(k);
							}
						}
						res[j] = f;
					}
				}

				lasttime = time;
			}
			if (e.isNote())
			{
				if (e.isNoteOn())
				{
					notestate[e.getKeyNumber()]++;
				}
				else if (e.isNoteOff())
				{
					notestate[e.getKeyNumber()]--;
					if (notestate[e.getKeyNumber()] < 0)
					{
						// this should never happen
					}
				}
			}
		}

        if (logger_pianodata.isDebugEnabled())
        {
            logger_pianodata.debug("Piano data calculated:");
            logger_pianodata.debug("Time MS, Keys");
            for (auto it = res.begin(); it != res.end(); ++it)
            {
                std::string keys = "";
                for (auto it2 = it->second.begin(); it2 != it->second.end(); ++it2)
                {
                    keys += " " + std::string(wxString::Format("%f", *it2).c_str());
                }
                logger_pianodata.debug("%d,%s", it->first, keys.c_str());
            }
        }
	}
	else
	{
        logger_pianodata.warn("Invalid MIDI file " + file);
	}

	return res;
}

std::list<std::string> PianoEffect::ExtractNotes(std::string& label)
{
    std::string n = label;
    std::transform(n.begin(), n.end(), n.begin(), ::toupper);

    std::list<std::string> res;

    string s = "";
    for (auto it = n.begin(); it != n.end(); ++it)
    {
        if (*it == ':' || *it == ' ' || *it == ';' || *it == ',')
        {
            if (s != "")
            {
                res.push_back(s);
                s = "";
            }
        }
        else
        {
            if ((*it >= 'A' && *it <= 'G') ||
                (*it == '#') ||
                (*it >= '0' && *it <= '9'))
            {
                s += *it;
            }
        }
    }

    if (s != "")
    {
        res.push_back(s);
    }

    return res;
}

int PianoEffect::ConvertNote(std::string& note)
{
    int number = -1;
    std::string n = note;
    int nletter = -1;
    std::transform(n.begin(), n.end(), n.begin(), ::toupper);

    switch (n[0])
    {
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
    default:
        number = wxAtoi(n);
        if (number < 0) number = 0;
        if (number > 127) number = 127;
        return number;
    }

    int octave = 4;
    n = n.substr(1);
    if (n != "")
    {
        if (n[0] == '#')
        {
            nletter++;
            n = n.substr(1);
        }
        else if (n[0] == 'B')
        {
            nletter--;
            n = n.substr(1);
        }
    }

    if (n != "")
    {
        octave = wxAtoi(n);
    }

    number = 12 + (octave * 12) + nletter;
    if (number < 0) number = 0;
    if (number > 127) number = 127;
    return number;
}

std::map<int, std::list<float>> PianoEffect::LoadTimingTrack(std::string track, int intervalMS)
{
    log4cpp::Category &logger_pianodata = log4cpp::Category::getInstance(std::string("log_pianodata"));
    std::map<int, std::list<float>> res;

    logger_pianodata.debug("Loading timings from timing track " + track);

    if (mSequenceElements == nullptr)
    {
        logger_pianodata.debug("No timing tracks found.");
        return res;
    }

    // Load the names of the timing tracks
    Element* t = NULL;
    for (size_t i = 0; i < mSequenceElements->GetElementCount(); i++)
    {
        Element* e = mSequenceElements->GetElement(i);
        if (e->GetEffectLayerCount() == 1 && e->GetType() == "timing")
        {
            if (e->GetName() == track)
            {
                t = e;
                break;
            }
        }
    }

    if (t == NULL)
    {
        logger_pianodata.debug("Timing track not found.");
        return res;
    }

    EffectLayer* el = t->GetEffectLayer(0);
    for (int j = 0; j < el->GetEffectCount(); j++)
    {
        std::list<float> notes;
        int starttime = el->GetEffect(j)->GetStartTimeMS();
        int endtime = el->GetEffect(j)->GetEndTimeMS();
        std::string label = el->GetEffect(j)->GetEffectName();
        std::list<std::string> notelabels = ExtractNotes(label);
        for (auto s = notelabels.begin(); s != notelabels.end(); ++s)
        {
            float n = (float)ConvertNote(*s);
            if (n >= 0)
            {
                notes.push_back(n);
            }
        }
        for (int i = starttime; i < endtime; i += intervalMS)
        {
            res[i] = notes;
        }
    }

    return res;
}
