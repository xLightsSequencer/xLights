/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/smeighan/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/smeighan/xLights/blob/master/License.txt
 **************************************************************/

#include "VUMeterEffect.h"
#include "VUMeterPanel.h"
#include "../AudioManager.h"
#include "../sequencer/SequenceElements.h"

#include "../sequencer/Effect.h"
#include "../RenderBuffer.h"
#include "../UtilClasses.h"
#include "../models/Model.h"
#include "../UtilFunctions.h"

#include "../../include/vumeter-16.xpm"
#include "../../include/vumeter-24.xpm"
#include "../../include/vumeter-32.xpm"
#include "../../include/vumeter-48.xpm"
#include "../../include/vumeter-64.xpm"

#include <algorithm>

namespace RenderType
{
	enum Enum
	{
		SPECTROGRAM,
		VOLUME_BARS,
		WAVEFORM,
		TIMING_EVENT_SPIKE,
		TIMING_EVENT_SWEEP,
		ON,
		PULSE,
		INTENSITY_WAVE,
		UNUSED,
		LEVEL_PULSE,
		LEVEL_SHAPE,
		COLOR_ON,
		TIMING_EVENT_COLOR,
		NOTE_ON,
		NOTE_LEVEL_PULSE,
		TIMING_EVENT_JUMP,
		TIMING_EVENT_PULSE,
		TIMING_EVENT_JUMP_100,
		TIMING_EVENT_BAR,
		LEVEL_BAR,
		NOTE_LEVEL_BAR,
		LEVEL_PULSE_COLOR,
		TIMING_EVENT_BARS,
        LEVEL_COLOR,
        TIMING_EVENT_PULSE_COLOR,
        SPECTROGRAM_PEAK,
        SPECTROGRAM_CIRCLELINE,
        SPECTROGRAM_LINE,
        FRAME_WAVEFORM,
        TIMING_EVENT_SWEEP2,
        TIMING_EVENT_TIMED_SWEEP,
        TIMING_EVENT_TIMED_SWEEP2,
        TIMING_EVENT_ALTERNATE_TIMED_SWEEP,
        TIMING_EVENT_ALTERNATE_TIMED_SWEEP2,
        LEVEL_JUMP,
        LEVEL_JUMP100,
        NOTE_LEVEL_JUMP,
        NOTE_LEVEL_JUMP100,
        DOMINANT_FREQUENCY_COLOUR,
        DOMINANT_FREQUENCY_COLOUR_GRADIENT
    };
}

namespace ShapeType
{
	enum Enum
	{
		CIRCLE,
		FILLED_CIRCLE,
		SQUARE,
		FILLED_SQUARE,
		DIAMOND,
		FILLED_DIAMOND,
		STAR,
		FILLED_STAR,
		TREE,
		FILLED_TREE,
		CRUCIFIX,
		FILLED_CRUCIFIX,
		PRESENT,
		FILLED_PRESENT,
		CANDY_CANE,
		FILLED_CANDY_CANE,
		SNOWFLAKE,
		HEART,
		FILLED_HEART
	};
}

VUMeterEffect::VUMeterEffect(int id) : RenderableEffect(id, "VU Meter", vumeter_16, vumeter_24, vumeter_32, vumeter_48, vumeter_64)
{
}

VUMeterEffect::~VUMeterEffect()
{
}

std::list<std::string> VUMeterEffect::CheckEffectSettings(const SettingsMap& settings, AudioManager* media, Model* model, Effect* eff, bool renderCache)
{
    std::list<std::string> res;

    wxString type = settings.Get("E_CHOICE_VUMeter_Type", "Waveform");

    if (media == nullptr &&
        (type == "Spectrogram" ||
         type == "Spectrogram Peak" ||
         type == "Volume Bars" ||
         type == "Waveform" ||
         type == "On" ||
        type == "Intensity Wave" ||
            type == "Level Bar" ||
            type == "Note Level Bar" ||
            type == "Level Pulse" ||
            type == "Level Jump" ||
            type == "Level Jump 100" ||
            type == "Level Pulse Color" ||
        type == "Level Shape" ||
        type == "Color On" ||
        type == "Note On" ||
        type == "Note Level Pulse" ||
        type == "Timing Event Jump" ||
            type == "Dominant Frequency Colour" ||
            type == "Dominant Frequency Colour Gradient"
            ))
    {
        res.push_back(wxString::Format("    ERR: VU Meter effect '%s' is pointless if there is no music. Model '%s', Start %s", type, model->GetName(), FORMATTIME(eff->GetStartTimeMS())).ToStdString());
    }

    wxString timing = settings.Get("E_CHOICE_VUMeter_TimingTrack", "");

    if (type == "Timing Event Spike" ||
        type == "Timing Event Sweep" ||
        type == "Timing Event Bar" ||
        type == "Timing Event Bars" ||
        type == "Timing Event Color" ||
        type == "Timing Event Pulse" ||
        type == "Timing Event Jump 100" ||
        type == "Timing Event Jump")
    {
        if (timing == "")
        {
            res.push_back(wxString::Format("    ERR: VU Meter effect '%s' needs a timing track. Model '%s', Start %s", type, model->GetName(), FORMATTIME(eff->GetStartTimeMS())).ToStdString());
        }
        else if (GetTiming(timing) == nullptr)
        {
            res.push_back(wxString::Format("    ERR: VU Meter effect '%s' has unknown timing track (%s). Model '%s', Start %s", type, timing, model->GetName(), FORMATTIME(eff->GetStartTimeMS())).ToStdString());
        }
    }

    return res;
}

bool VUMeterEffect::needToAdjustSettings(const std::string& version)
{
    return IsVersionOlder("2019.16", version);
}

void VUMeterEffect::adjustSettings(const std::string& version, Effect* effect, bool removeDefaults)
{
    SettingsMap &settings = effect->GetSettings();
    if (settings.Contains("E_CHECKBOX_Fireworks_LogarithmicX"))
    {
        settings["E_CHECKBOX_VUMeter_LogarithmicX"] = settings.Get("E_CHECKBOX_Fireworks_LogarithmicX", "0");
        settings.erase("E_CHECKBOX_Fireworks_LogarithmicX");
    }
}

wxPanel *VUMeterEffect::CreatePanel(wxWindow *parent) {
	return new VUMeterPanel(parent);
}

void VUMeterEffect::SetPanelStatus(Model* cls)
{
    VUMeterPanel *vp = static_cast<VUMeterPanel*>(panel);
    if (vp == nullptr)
    {
        return;
    }

    vp->Choice_VUMeter_TimingTrack->Clear();
    if (mSequenceElements == nullptr)
    {
        vp->ValidateWindow();
        return;
    }

    // Load the names of the timing tracks
    for (int i = 0; i < mSequenceElements->GetElementCount(); i++)
    {
        Element* e = mSequenceElements->GetElement(i);
        if (e->GetType() == ElementType::ELEMENT_TYPE_TIMING)
        {
            vp->Choice_VUMeter_TimingTrack->Append(e->GetName());
        }
    }

    // Select the first one
    if (vp->Choice_VUMeter_TimingTrack->GetCount() > 0)
    {
        vp->Choice_VUMeter_TimingTrack->Select(0);
    }

    // Validate the window (includes enabling and disabling controls)
    vp->ValidateWindow();
}

void VUMeterEffect::SetDefaultParameters()
{
    VUMeterPanel *vp = static_cast<VUMeterPanel*>(panel);
    if (vp == nullptr) {
        return;
    }

    SetSliderValue(vp->Slider_VUMeter_Bars, 6);
    SetChoiceValue(vp->Choice_VUMeter_Type, "Waveform");
    SetSliderValue(vp->Slider_VUMeter_Sensitivity, 70);
    SetChoiceValue(vp->Choice_VUMeter_Shape, "Circle");
    SetCheckBoxValue(vp->CheckBox_VUMeter_SlowDownFalls, true);
    SetCheckBoxValue(vp->CheckBox_LogarithmicXAxis, false);
    SetSliderValue(vp->Slider_VUMeter_StartNote, 36);
    SetSliderValue(vp->Slider_VUMeter_EndNote, 84);
    SetSliderValue(vp->Slider_VUMeter_XOffset, 0);
    SetSliderValue(vp->Slider_VUMeter_YOffset, 0);
    SetSliderValue(vp->Slider_VUMeter_Gain, 0);
    vp->BitmapButton_VUMeter_YOffsetVC->SetActive(false);
    vp->BitmapButton_VUMeter_Gain->SetActive(false);
    vp->ValidateWindow();
}

void VUMeterEffect::RenameTimingTrack(std::string oldname, std::string newname, Effect* effect)
{
    wxString timing = effect->GetSettings().Get("E_CHOICE_VUMeter_TimingTrack", "");

    if (timing.ToStdString() == oldname)
    {
        effect->GetSettings()["E_CHOICE_VUMeter_TimingTrack"] = wxString(newname);
    }
}

void VUMeterEffect::Render(Effect *effect, SettingsMap &SettingsMap, RenderBuffer &buffer) {
    float oset = buffer.GetEffectTimeIntervalPosition();
    Render(buffer,
        effect->GetParentEffectLayer()->GetParentElement()->GetSequenceElements(),
        SettingsMap.GetInt("SLIDER_VUMeter_Bars", 6),
        SettingsMap.Get("CHOICE_VUMeter_Type", "Waveform"),
        SettingsMap.Get("CHOICE_VUMeter_TimingTrack", ""),
        SettingsMap.GetInt("SLIDER_VUMeter_Sensitivity", 70),
        SettingsMap.Get("CHOICE_VUMeter_Shape", "Circle"),
        SettingsMap.GetBool("CHECKBOX_VUMeter_SlowDownFalls", true),
        SettingsMap.GetInt("SLIDER_VUMeter_StartNote", 0),
        SettingsMap.GetInt("SLIDER_VUMeter_EndNote", 127),
        SettingsMap.GetInt("SLIDER_VUMeter_XOffset", 0),
        GetValueCurveInt("VUMeter_YOffset", 0, SettingsMap, oset, VUMETER_OFFSET_MIN, VUMETER_OFFSET_MAX, buffer.GetStartTimeMS(), buffer.GetEndTimeMS()),
        GetValueCurveInt("VUMeter_Gain", 0, SettingsMap, oset, VUMETER_GAIN_MIN, VUMETER_GAIN_MAX, buffer.GetStartTimeMS(), buffer.GetEndTimeMS()),
        SettingsMap.GetBool("CHECKBOX_VUMeter_LogarithmicX", false)
        );
}

class VUMeterRenderCache : public EffectRenderCache
{

public:
    VUMeterRenderCache()
	{
        _lastsize = 0;
        _colourindex = 0;
        _lasttimingmark = 0;
        _nCount = 0;
	};
    virtual ~VUMeterRenderCache() {};
	std::list<int> _timingmarks; // collection of recent timing marks ... used for sweep
	int _lasttimingmark; // last time we saw a timing mark ... used for pulse
	std::list<float> _lastvalues;
	std::list<float> _lastpeaks;
    std::list<int> _pausepeakfall;
    std::list<std::vector<wxPoint>> _lineHistory;
	float _lastsize;
    int _colourindex;
    int _nCount;
};

int VUMeterEffect::DecodeType(const std::string& type)
{
	if (type == "Spectrogram")
	{
		return RenderType::SPECTROGRAM;
	}
	else if (type == "Volume Bars")
	{
		return RenderType::VOLUME_BARS;
	}
	else if (type == "Waveform")
	{
		return RenderType::WAVEFORM;
	}
	else if (type == "Timing Event Spike")
	{
		return RenderType::TIMING_EVENT_SPIKE;
	}
	else if (type == "Timing Event Sweep")
	{
		return RenderType::TIMING_EVENT_SWEEP;
	}
    else if (type == "Timing Event Sweep 2")
    {
        return RenderType::TIMING_EVENT_SWEEP2;
    }
    else if (type == "Timing Event Timed Sweep")
    {
        return RenderType::TIMING_EVENT_TIMED_SWEEP;
    }
    else if (type == "Timing Event Timed Sweep 2")
    {
        return RenderType::TIMING_EVENT_TIMED_SWEEP2;
    }
    else if (type == "Timing Event Alternate Timed Sweep")
    {
        return RenderType::TIMING_EVENT_ALTERNATE_TIMED_SWEEP;
    }
    else if (type == "Timing Event Alternate Timed Sweep 2")
    {
        return RenderType::TIMING_EVENT_ALTERNATE_TIMED_SWEEP2;
    }
    else if (type == "On")
	{
		return RenderType::ON;
	}
	else if (type == "Pulse")
	{
		return RenderType::PULSE;
	}
	else if (type == "Intensity Wave")
	{
		return RenderType::INTENSITY_WAVE;
	}
	else if (type == "unused")
	{
		return RenderType::UNUSED;
	}
	else if (type == "Level Pulse")
	{
		return RenderType::LEVEL_PULSE;
	}
    else if (type == "Level Jump")
    {
        return RenderType::LEVEL_JUMP;
    }
    else if (type == "Level Jump 100")
    {
        return RenderType::LEVEL_JUMP100;
    }
    else if (type == "Level Shape")
	{
		return RenderType::LEVEL_SHAPE;
	}
    else if (type == "Color On")
    {
        return RenderType::COLOR_ON;
    }
    else if (type == "Timing Event Color")
    {
        return RenderType::TIMING_EVENT_COLOR;
    }
    else if (type == "Note On")
    {
        return RenderType::NOTE_ON;
    }
    else if (type == "Note Level Pulse")
    {
        return RenderType::NOTE_LEVEL_PULSE;
    }
    else if (type == "Note Level Jump")
    {
        return RenderType::NOTE_LEVEL_JUMP;
    }
    else if (type == "Note Level Jump 100")
    {
        return RenderType::NOTE_LEVEL_JUMP100;
    }
    else if (type == "Timing Event Jump")
    {
        return RenderType::TIMING_EVENT_JUMP;
    }
    else if (type == "Timing Event Pulse")
    {
		return RenderType::TIMING_EVENT_PULSE;
    }
    else if (type == "Timing Event Jump 100")
    {
		return RenderType::TIMING_EVENT_JUMP_100;
    }
    else if (type == "Timing Event Bar")
    {
        return RenderType::TIMING_EVENT_BAR;
    }
    else if (type == "Level Bar")
    {
        return RenderType::LEVEL_BAR;
    }
    else if (type == "Note Level Bar")
    {
        return RenderType::NOTE_LEVEL_BAR;
    }
    else if (type == "Level Pulse Color")
    {
        return RenderType::LEVEL_PULSE_COLOR;
    }
    else if (type == "Timing Event Bars")
    {
        return RenderType::TIMING_EVENT_BARS;
    }
    else if (type == "Timing Event Pulse Color")
    {
        return RenderType::TIMING_EVENT_PULSE_COLOR;
    }
    else if (type == "Level Color")
    {
        return RenderType::LEVEL_COLOR;
    }
    else if (type == "Spectrogram Peak")
    {
        return RenderType::SPECTROGRAM_PEAK;
    }
    else if (type == "Spectrogram Line")
    {
        return RenderType::SPECTROGRAM_LINE;
    }
    else if (type == "Spectrogram Circle Line")
    {
        return RenderType::SPECTROGRAM_CIRCLELINE;
    }
    else if (type == "Frame Waveform")
    {
        return RenderType::FRAME_WAVEFORM;
    }
    else if (type == "Dominant Frequency Colour")
    {
    return RenderType::DOMINANT_FREQUENCY_COLOUR;
     }
    else if (type == "Dominant Frequency Colour Gradient")
    {
    return RenderType::DOMINANT_FREQUENCY_COLOUR_GRADIENT;
     }
     // default type is volume bars
	return RenderType::VOLUME_BARS;
}

int VUMeterEffect::DecodeShape(const std::string& shape)
{
	if (shape == "Circle")
	{
		return ShapeType::CIRCLE;
	}
	else if (shape == "Filled Circle")
	{
		return ShapeType::FILLED_CIRCLE;
	}
	else if (shape == "Square")
	{
		return ShapeType::SQUARE;
	}
	else if (shape == "Filled Square")
	{
		return ShapeType::FILLED_SQUARE;
	}
	else if (shape == "Diamond")
	{
		return ShapeType::DIAMOND;
	}
	else if (shape == "Filled Diamond")
	{
		return ShapeType::FILLED_DIAMOND;
	}
	else if (shape == "Star")
	{
		return ShapeType::STAR;
	}
	else if (shape == "Filled Star")
	{
		return ShapeType::FILLED_STAR;
	}
	else if (shape == "Tree")
	{
		return ShapeType::TREE;
	}
	else if (shape == "Filled Tree")
	{
		return ShapeType::FILLED_TREE;
	}
	else if (shape == "Crucifix")
	{
		return ShapeType::CRUCIFIX;
	}
	else if (shape == "Filled Crucifix")
	{
		return ShapeType::FILLED_CRUCIFIX;
	}
	else if (shape == "Present")
	{
		return ShapeType::PRESENT;
	}
	else if (shape == "Filled Present")
	{
		return ShapeType::FILLED_PRESENT;
	}
	else if (shape == "Candy Cane")
	{
		return ShapeType::CANDY_CANE;;
	}
	else if (shape == "Snowflake")
	{
		return ShapeType::SNOWFLAKE;
	}
	else if (shape == "Heart")
	{
		return ShapeType::HEART;
	}
	else if (shape == "Filled Heart")
	{
		return ShapeType::FILLED_HEART;
	}

	return ShapeType::CIRCLE;
}

void VUMeterEffect::Render(RenderBuffer &buffer, SequenceElements *elements, int bars, const std::string& type, const std::string &timingtrack, int sensitivity, const std::string& shape, bool slowdownfalls, int startnote, int endnote, int xoffset, int yoffset, int gain, bool logarithmicX)
{
    // startnote must be less than or equal to endnote
    if (startnote > endnote)
    {
        int temp = startnote;
        startnote = endnote;
        endnote = temp;
    }

    int nType = DecodeType(type);

	// Grab our cache
	VUMeterRenderCache *cache = static_cast<VUMeterRenderCache*>(buffer.infoCache[id]);
	if (cache == nullptr) {
		cache = new VUMeterRenderCache();
		buffer.infoCache[id] = cache;
	}
	std::list<int>& _timingmarks = cache->_timingmarks;
	int &_lasttimingmark = cache->_lasttimingmark;
	std::list<float>& _lastvalues = cache->_lastvalues;
	std::list<float>& _lastpeaks = cache->_lastpeaks;
	std::list<int>& _pausepeakfall = cache->_pausepeakfall;
    int& _nCount = cache->_nCount;
	float& _lastsize = cache->_lastsize;
    int & _colourindex = cache->_colourindex;
    std::list<std::vector<wxPoint>>& _lineHistory = cache->_lineHistory;

	// Check for config changes which require us to reset
	if (buffer.needToInit)
	{
        buffer.needToInit = false;
        _lineHistory.clear();
        _nCount = 0;
        _colourindex = -1;
		_timingmarks.clear();
		_lasttimingmark = -1;
		_lastvalues.clear();
		_lastpeaks.clear();
        _pausepeakfall.clear();
		_lastsize = 0;
        if (timingtrack != "")
        {
            elements->AddRenderDependency(timingtrack, buffer.cur_model);
        }
	}

	// We limit bars to the width of the model in some effects
	int usebars = bars;
    if (nType == RenderType::TIMING_EVENT_JUMP || nType == RenderType::TIMING_EVENT_PULSE || nType == RenderType::TIMING_EVENT_JUMP_100)
    {
        // dont limit
    }
    else
    {
        if (usebars > buffer.BufferWi)
        {
            usebars = buffer.BufferWi;
        }
    }

	try
	{
		switch (nType)
		{
		case RenderType::SPECTROGRAM:
			RenderSpectrogramFrame(buffer, bars, _lastvalues, _lastpeaks, _pausepeakfall, slowdownfalls, startnote, endnote, xoffset, yoffset, false, 0, false, logarithmicX, false, 1, sensitivity, _lineHistory);
			break;
		case RenderType::SPECTROGRAM_PEAK:
			RenderSpectrogramFrame(buffer, bars, _lastvalues, _lastpeaks, _pausepeakfall, slowdownfalls, startnote, endnote, xoffset, yoffset, true, sensitivity, false, logarithmicX, false, 1, sensitivity, _lineHistory);
			break;
		case RenderType::SPECTROGRAM_LINE:
			RenderSpectrogramFrame(buffer, bars, _lastvalues, _lastpeaks, _pausepeakfall, slowdownfalls, startnote, endnote, xoffset, yoffset, true, sensitivity, true, logarithmicX, false, 1, sensitivity, _lineHistory);
			break;
		case RenderType::SPECTROGRAM_CIRCLELINE:
            RenderSpectrogramFrame(buffer, bars, _lastvalues, _lastpeaks, _pausepeakfall, slowdownfalls, startnote, endnote, xoffset, yoffset, true, sensitivity, true, logarithmicX, true, gain, sensitivity, _lineHistory);
			break;
		case RenderType::VOLUME_BARS:
			RenderVolumeBarsFrame(buffer, usebars, gain);
			break;
		case RenderType::WAVEFORM:
			RenderWaveformFrame(buffer, usebars, yoffset, gain, false);
			break;
		case RenderType::FRAME_WAVEFORM:
			RenderWaveformFrame(buffer, usebars, yoffset, gain, true);
			break;
        case RenderType::TIMING_EVENT_TIMED_SWEEP:
        case RenderType::TIMING_EVENT_TIMED_SWEEP2:
        case RenderType::TIMING_EVENT_ALTERNATE_TIMED_SWEEP:
        case RenderType::TIMING_EVENT_ALTERNATE_TIMED_SWEEP2:
            RenderTimingEventTimedSweepFrame(buffer, usebars, nType, timingtrack, _nCount);
            break;
        case RenderType::TIMING_EVENT_SPIKE:
		case RenderType::TIMING_EVENT_SWEEP:
        case RenderType::TIMING_EVENT_SWEEP2:
            RenderTimingEventFrame(buffer, usebars, nType, timingtrack, _timingmarks);
			break;
		case RenderType::ON:
			RenderOnFrame(buffer, gain);
			break;
		case RenderType::PULSE:
			RenderPulseFrame(buffer, usebars, timingtrack, _lasttimingmark);
			break;
		case RenderType::INTENSITY_WAVE:
			RenderIntensityWaveFrame(buffer, usebars, gain);
			break;
		case RenderType::LEVEL_PULSE:
			RenderLevelPulseFrame(buffer, usebars, sensitivity, _lasttimingmark, gain);
			break;
        case RenderType::LEVEL_JUMP:
            RenderLevelJumpFrame(buffer, usebars, sensitivity, _lasttimingmark, gain, false, _lastsize);
            break;
        case RenderType::LEVEL_JUMP100:
            RenderLevelJumpFrame(buffer, usebars, sensitivity, _lasttimingmark, gain, true, _lastsize);
            break;
        case RenderType::LEVEL_SHAPE:
			RenderLevelShapeFrame(buffer, shape, _lastsize, sensitivity, slowdownfalls, xoffset, yoffset, usebars, gain);
			break;
        case RenderType::COLOR_ON:
            RenderOnColourFrame(buffer, gain);
            break;
        case RenderType::DOMINANT_FREQUENCY_COLOUR:
            RenderDominantFrequencyColour(buffer, sensitivity, startnote, endnote, false);
            break;
        case RenderType::DOMINANT_FREQUENCY_COLOUR_GRADIENT:
            RenderDominantFrequencyColour(buffer, sensitivity, startnote, endnote, true);
            break;
        case RenderType::TIMING_EVENT_COLOR:
            RenderTimingEventColourFrame(buffer, _colourindex, timingtrack);
            break;
        case RenderType::NOTE_ON:
            RenderNoteOnFrame(buffer, startnote, endnote, gain);
            break;
        case RenderType::NOTE_LEVEL_PULSE:
            RenderNoteLevelPulseFrame(buffer, usebars, sensitivity, _lasttimingmark, startnote, endnote, gain);
            break;
        case RenderType::NOTE_LEVEL_JUMP:
            RenderNoteLevelJumpFrame(buffer, usebars, sensitivity, _lasttimingmark, startnote, endnote, gain, false, _lastsize);
            break;
        case RenderType::NOTE_LEVEL_JUMP100:
            RenderNoteLevelJumpFrame(buffer, usebars, sensitivity, _lasttimingmark, startnote, endnote, gain, true, _lastsize);
            break;
        case RenderType::TIMING_EVENT_JUMP:
            RenderTimingEventJumpFrame(buffer, usebars, timingtrack, _lastsize, true, gain);
            break;
        case RenderType::TIMING_EVENT_PULSE:
            RenderTimingEventPulseFrame(buffer, usebars, timingtrack, _lastsize);
            break;
        case RenderType::TIMING_EVENT_JUMP_100:
            RenderTimingEventJumpFrame(buffer, usebars, timingtrack, _lastsize, false, 0);
            break;
        case RenderType::TIMING_EVENT_BAR:
            RenderTimingEventBarFrame(buffer, usebars, timingtrack, _lastsize, _colourindex, false);
            break;
        case RenderType::LEVEL_BAR:
            RenderLevelBarFrame(buffer, usebars, sensitivity, _lastsize, _colourindex, gain);
            break;
        case RenderType::NOTE_LEVEL_BAR:
            RenderNoteLevelBarFrame(buffer, usebars, sensitivity, _lastsize, _colourindex, startnote, endnote, gain);
            break;
        case RenderType::LEVEL_PULSE_COLOR:
            RenderLevelPulseColourFrame(buffer, usebars, sensitivity, _lasttimingmark, _colourindex, gain);
            break;
        case RenderType::TIMING_EVENT_BARS:
            RenderTimingEventBarFrame(buffer, usebars, timingtrack, _lastsize, _colourindex, true);
            break;
        case RenderType::TIMING_EVENT_PULSE_COLOR:
            RenderTimingEventPulseColourFrame(buffer, usebars, timingtrack, _lastsize, _colourindex);
            break;
        case RenderType::LEVEL_COLOR:
            RenderLevelColourFrame(buffer, _colourindex, sensitivity, _lasttimingmark, gain);
            break;
        default:
            wxASSERT(false);
            break;
        }
	}
	catch (...)
	{
		// This is here to let me catch any exceptions and stop the exception causing the render thread to die
		//int a = 0;
	}
}

void VUMeterEffect::RenderSpectrogramFrame(RenderBuffer &buffer, int usebars, std::list<float>& lastvalues, std::list<float>& lastpeaks, std::list<int>& pauseuntilpeakfall, bool slowdownfalls, int startNote, int endNote, int xoffset, int yoffset, bool peak, int peakhold, bool line, bool logarithmicX, bool circle, int gain, int sensitivity, std::list<std::vector<wxPoint>>& lineHistory) const
{
    if (buffer.GetMedia() == nullptr) return;

    int truexoffset = xoffset * buffer.BufferWi / 100;
    int trueyoffset = yoffset * buffer.BufferHt / 100;
	std::list<float> const * const pdata = buffer.GetMedia()->GetFrameData(buffer.curPeriod, FRAMEDATA_VU, "");

    while (lineHistory.size() > sensitivity / 10)
    {
        lineHistory.pop_front();
    }

	if (pdata != nullptr && pdata->size() != 0)
	{
        if (peak)
        {
            if (lastvalues.size() == 0)
            {
                lastvalues = *pdata;
                lastpeaks = *pdata;
                for (auto it = lastvalues.begin(); it != lastvalues.end(); ++it)
                {
                    pauseuntilpeakfall.push_back(0);
                }
            }
            else
            {
                std::list<float>::const_iterator newdata = pdata->cbegin();
                std::list<float>::iterator olddata = lastpeaks.begin();
                auto pause = pauseuntilpeakfall.begin();

                while (olddata != lastpeaks.end())
                {
                    if (*newdata < *olddata)
                    {
                        if (*pause == 0)
                        {
                            *olddata = *olddata - 0.05;
                            if (*olddata < *newdata)
                            {
                                *olddata = *newdata;
                            }
                        }
                        *pause = std::max(*pause - 1, 0);
                    }
                    else
                    {
                        *olddata = *newdata;
                        *pause = peakhold; // frames to pause before peaks drop
                    }

                    ++olddata;
                    ++newdata;
                    ++pause;
                }
            }
        }

		if (slowdownfalls)
		{
			if (lastvalues.size() == 0)
			{
				lastvalues = *pdata;
			}
			else
			{
				std::list<float>::const_iterator newdata = pdata->cbegin();
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

		float per = (float)datapoints / (float)usebars;
        float cols = 1;
        if (xoffset == 0)
        {
            cols = (float)buffer.BufferWi / (float)usebars;
        }
        if (cols < 1)
        {
            cols = 1;
        }
		std::list<float>::iterator it = lastvalues.begin();
		std::list<float>::iterator itpeak = lastpeaks.begin();
        int midiNote = 0;
        // skip to our start note
        for (int i = 0; i < startNote; i++)
        {
            ++it;
            if (peak)
            {
                ++itpeak;
            }
            ++midiNote;
        }

		int x = truexoffset;

        // Peak colour is the last colour selected
        xlColor peakColour = buffer.GetPalette().GetColor(0);
        if (buffer.GetPalette().Size() > 1)
        {
            peakColour = buffer.GetPalette().GetColor(buffer.GetColorCount() - 1);
        }

        xlColor color = buffer.palette.GetColor(0);
        int alpha = 255;
        if (lineHistory.size() > 0)
        {
            buffer.SetAllowAlphaChannel(true);
            for (auto l : lineHistory)
            {
                if (l.size() > 1)
                {
                    alpha -= 255 / (sensitivity / 10);
                    color.SetAlpha(alpha);
                    auto p1 = l.begin();
                    auto p2 = std::next(p1);
                    while (p2 != l.end())
                    {
                        buffer.DrawLine(p1->x, p1->y, p2->x, p2->y, color, true);
                        ++p2;
                        ++p1;
                    }
                }
            }
        }

        color = buffer.palette.GetColor(0);
        int lastColHeight = -1;
        int lastColX = -1;
        float firstVector = -1;
        float lastVector = -1;
        std::vector<wxPoint> linePoints;
        for (int j = 0; j < usebars; j++)
        {
            float f = 0;
            float p = 0;
            int thisper = per;
            if (logarithmicX)
            {
                thisper = LogarithmicScale::GetLogSum(j + 1) - LogarithmicScale::GetLogSum(j);
            }
            for (int k = 0; k < thisper; k++)
            {
                // use the max within the frequency range
                if (*it > f)
                {
                    f = *it;
                }
                ++it;
                ++midiNote;
                if (peak)
                {
                    if (*itpeak > p)
                    {
                        p = *itpeak;
                    }
                    ++itpeak;
                }
                // dont let it go off the end
                if (it == lastvalues.end())
                {
                    --midiNote;
                    --it;
                    if (peak)
                    {
                        --itpeak;
                    }
                }
            }
            f = ApplyGain(f, gain);
            int colheight = buffer.BufferHt * f;
            if (line)
            {
                if (circle)
                {
                    float vector = std::min(buffer.BufferWi, buffer.BufferHt) * f;
                    if (j == 0) firstVector = vector;
                    float angleper = 360.0 / usebars;
                    float angle = angleper / 2.0 + j * angleper;
                    if (j == 0)
                    {
                        int x1 = buffer.BufferWi / 2 + truexoffset + vector * sin(toRadians(angle));
                        int y1 = buffer.BufferHt / 2 + trueyoffset + vector * cos(toRadians(angle));
                        linePoints.push_back(wxPoint(x1, y1));
                    }
                    else
                    {
                        int x1 = buffer.BufferWi /2 + truexoffset + lastVector * sin(toRadians(angle - angleper));
                        int y1 = buffer.BufferHt / 2 + trueyoffset + lastVector * cos(toRadians(angle - angleper));
                        int x2 = buffer.BufferWi / 2 + truexoffset + vector * sin(toRadians(angle));
                        int y2 = buffer.BufferHt / 2 + trueyoffset + vector * cos(toRadians(angle));
                        buffer.DrawLine(x1, y1, x2, y2, color);
                        linePoints.push_back(wxPoint(x2, y2));

                        if (j == usebars - 1)
                        {
                            x1 = buffer.BufferWi / 2 + truexoffset + firstVector * sin(toRadians(angle + angleper));
                            y1 = buffer.BufferHt / 2 + trueyoffset + firstVector * cos(toRadians(angle + angleper));
                            buffer.DrawLine(x2, y2, x1, y1, color);
                            linePoints.push_back(wxPoint(x1, y1));
                        }
                    }
                    lastVector = vector;
                }
                else
                {
                    int mid = cols * j + cols / 2.0;
                    linePoints.push_back(wxPoint(mid, colheight));

                    // draw lines to mid point of each column
                    if (lastColHeight >= 0)
                    {
                        buffer.DrawLine(lastColX, lastColHeight, mid, colheight, color);
                    }
                    else if (j == usebars - 1)
                    {
                        // just draw a horizontal line
                        buffer.DrawLine(0, colheight, cols - 1, colheight, color);
                    }

                    lastColHeight = colheight;
                    lastColX = mid;
                }
            }
            else
            {
                float limit = (j+1) * cols;
                while (x < limit)
                {
                    for (int y = 0; y < buffer.BufferHt; y++)
                    {
                        if (y < colheight)
                        {
                            xlColor color1;
                            // an alternate colouring
                            buffer.GetMultiColorBlend((double)y / (double)buffer.BufferHt, false, color1, peak ? 1 : 0);
                            buffer.SetPixel(x, y, color1);
                        }

                        if (peak)
                        {
                            int peakheight = buffer.BufferHt * p;
                            if (y >= peakheight)
                            {
                                buffer.SetPixel(x, y, peakColour);
                                break;
                            }
                        }
                        else
                        {
                            if (y >= colheight)
                            {
                                break;
                            }
                        }
                    }
                    x++;
                }
            }
		}
        if (linePoints.size() > 0)
        {
            lineHistory.push_back(linePoints);
        }
	}
}

void VUMeterEffect::RenderVolumeBarsFrame(RenderBuffer &buffer, int usebars, int gain)
{
    if (buffer.GetMedia() == nullptr) return;

	int start = buffer.curPeriod - usebars;
	int cols = buffer.BufferWi / usebars;
	int x = 0;
	for (int i = 0; i < usebars; i++)
	{
		if (start + i >= 0)
		{
			float f = 0.0;
			std::list<float> const * const pf = buffer.GetMedia()->GetFrameData(start + i, FRAMEDATA_HIGH, "");
			if (pf != nullptr)
			{
				f = ApplyGain(*pf->cbegin(), gain);
			}
			for (int j = 0; j < cols; j++)
			{
				int colheight = buffer.BufferHt * f;
				for (int y = 0; y < colheight; y++)
				{
					xlColor color1;
					//buffer.GetMultiColorBlend((double)y / (double)colheight, false, color1);
					buffer.GetMultiColorBlend((double)y / (double)buffer.BufferHt, false, color1);
					buffer.SetPixel(x, y, color1);
				}
				x++;
			}
		}
		else
		{
			x += cols;
		}
	}
}

void VUMeterEffect::RenderWaveformFrame(RenderBuffer &buffer, int usebars, int yoffset, int gain, bool frameDetail)
{
    if (buffer.GetMedia() == nullptr) return;

    int trueyoffset = yoffset * buffer.BufferHt / 2 / 100;
    float cols = (float)buffer.BufferWi / usebars;

    if (frameDetail)
    {
        int lasty = (float)trueyoffset + (float)buffer.BufferHt / 2.0;
        int lastx = 0;
        float barms = (float)buffer.frameTimeInMs / usebars;
        float rate = buffer.GetMedia()->GetRate();
        float startMS = buffer.curPeriod * buffer.frameTimeInMs;
        xlColor color = buffer.palette.GetColor(0);
        bool up = true;
        for (int i = 0; i < usebars; i++)
        {
            float min = 0;
            float max = 0;
            int startSample = rate * (startMS + (float)i * barms) / 1000.0;
            int endSample = rate * (startMS + (float)(i+1) * barms) / 1000.0;
            buffer.GetMedia()->GetLeftDataMinMax(startSample, endSample, min, max, AUDIOSAMPLETYPE::RAW);

            int y;
            int x = (float)i * cols + cols / 2;
            if (up)
            {
                max = ApplyGain(max, gain);
                y = (float)trueyoffset + (float)buffer.BufferHt / 2.0 + max * ((float)buffer.BufferHt / 2.0);
            }
            else
            {
                min = ApplyGain(min, gain);
                y = (float)trueyoffset + (float)buffer.BufferHt / 2.0 + min * ((float)buffer.BufferHt / 2.0);
            }

            buffer.DrawLine(lastx, lasty, x, y, color);

            lasty = y;
            lastx = x;

            if (i == usebars - 1)
            {
                buffer.DrawLine(lastx, lasty, buffer.BufferWi - 1, (float)trueyoffset + (float)buffer.BufferHt / 2.0, color);
            }

            up = !up;
        }
    }
    else
    {
        int start = buffer.curPeriod - usebars;
        int x = 0;
        for (int i = 0; i < usebars; i++)
        {
            if (start + i >= 0)
            {
                float fh = 0.0;
                std::list<float> const * pf = buffer.GetMedia()->GetFrameData(start + i, FRAMEDATA_HIGH, "");
                if (pf != nullptr)
                {
                    fh = ApplyGain(*pf->cbegin(), gain);
                }
                float fl = 0.0;
                pf = buffer.GetMedia()->GetFrameData(start + i, FRAMEDATA_LOW, "");
                if (pf != nullptr)
                {
                    fl = ApplyGain(*pf->cbegin(), gain);
                }
                int s = (1.0 - fl) * buffer.BufferHt / 2;
                int e = (1.0 + fh) * buffer.BufferHt / 2;
                if (e < s)
                {
                    e = s;
                }
                if (e > buffer.BufferHt)
                {
                    e = buffer.BufferHt;
                }
                for (int j = 0; j < cols; j++)
                {
                    for (int y = s; y < e; y++)
                    {
                        xlColor color1;
                        //buffer.GetMultiColorBlend((double)y / (double)e, false, color1);
                        buffer.GetMultiColorBlend((double)y / (double)buffer.BufferHt, false, color1);
                        buffer.SetPixel(x, y + trueyoffset, color1);
                    }
                    x++;
                }
            }
            else
            {
                x += cols;
            }
        }
    }
}

void VUMeterEffect::RenderTimingEventFrame(RenderBuffer& buffer, int usebars, int nType, std::string timingtrack, std::list<int>& timingmarks)
{
    EffectLayer* el = GetTiming(timingtrack);

    if (el == nullptr) return;

    int start = buffer.curPeriod - usebars;
    int cols = buffer.BufferWi / usebars;
    int x = 0;
    for (int i = 0; i < usebars; i++)
    {
        if (start + i >= 0)
        {
            int ms = (start + i) * buffer.frameTimeInMs;
            bool effectPresent = false;
            for (int j = 0; j < el->GetEffectCount(); j++)
            {
                if (el->GetEffect(j)->GetStartTimeMS() == ms)
                {
                    effectPresent = true;
                    break;
                }
            }
            if (effectPresent)
            {
                timingmarks.remove(start + i);
                timingmarks.push_back(start + i);
                for (int j = 0; j < cols; j++)
                {
                    xlColor color1;
                    buffer.GetMultiColorBlend((double)j / cols, false, color1);
                    for (int y = 0; y < buffer.BufferHt; y++)
                    {
                        if (nType == RenderType::TIMING_EVENT_SWEEP)
                        {
                            buffer.GetMultiColorBlend((double)y / (double)buffer.BufferHt, false, color1);
                        }
                        else if (nType == RenderType::TIMING_EVENT_SWEEP2)
                        {
                            // use x axis colour
                        }
                        else
                        {
                            buffer.GetMultiColorBlend(0, false, color1);
                        }
                        buffer.SetPixel(x, y, color1);
                    }
                    x++;
                }
            }
            else
            {
                if (nType == 5)
                {
                    // remove any no longer required
                    while (timingmarks.size() != 0 && *timingmarks.begin() < start - 10)
                    {
                        timingmarks.pop_front();
                    }

                    if (timingmarks.size() > 0)
                    {
                        int left = cols;

                        for (std::list<int>::iterator it = timingmarks.begin(); it != timingmarks.end(); ++it)
                        {
                            if (((start + i) > * it) && ((start + i) < *it + 10))
                            {
                                float yt = (10 - (start + i - *it)) / 10.0;
                                if (yt < 0)
                                {
                                    yt = 0;
                                }
                                xlColor color1;
                                buffer.GetMultiColorBlend(1.0 - yt, false, color1);
                                for (int j = 0; j < cols; j++)
                                {
                                    for (int y = 0; y < buffer.BufferHt; y++)
                                    {
                                        buffer.SetPixel(x, y, color1);
                                    }
                                    x++;
                                    left--;
                                }
                            }
                        }
                        x += left;
                    }
                    else
                    {
                        x += cols;
                    }
                }
                else
                {
                    x += cols;
                }
            }
        }
        else
        {
            x += cols;
        }
    }
}

void VUMeterEffect::RenderTimingEventTimedSweepFrame(RenderBuffer& buffer, int usebars, int nType, std::string timingtrack, int& nCount)
{
    Effect* timing = GetCurrentTiming(buffer, timingtrack);

    if (timing == nullptr) return;

    if (buffer.curPeriod * buffer.frameTimeInMs == timing->GetStartTimeMS())
    {
        nCount++;
    }

    // we have a timing mark
    double lengthOfTiming = timing->GetEndTimeMS() - timing->GetStartTimeMS();
    double lengthOfTimingFrames = lengthOfTiming / buffer.frameTimeInMs;
    double distanceToTravel = buffer.BufferWi;
    if (nType == RenderType::TIMING_EVENT_TIMED_SWEEP || nType == RenderType::TIMING_EVENT_ALTERNATE_TIMED_SWEEP)
    {
        distanceToTravel += 2 * usebars;
    }
    else
    {
        distanceToTravel -= usebars;
    }
    double perFrameDistance = distanceToTravel / lengthOfTimingFrames;
    double posInTiming = (buffer.curPeriod * buffer.frameTimeInMs - timing->GetStartTimeMS()) / buffer.frameTimeInMs;
    int startX = perFrameDistance * posInTiming;
    if (nType == RenderType::TIMING_EVENT_TIMED_SWEEP || nType == RenderType::TIMING_EVENT_ALTERNATE_TIMED_SWEEP)
    {
        startX -= usebars;
    }
    for (int x = 0; x < usebars; x++)
    {
        xlColor color1;
        buffer.GetMultiColorBlend((double)x / usebars, false, color1);
        for (int y = 0; y < buffer.BufferHt; y++)
        {
            if ((nType == RenderType::TIMING_EVENT_ALTERNATE_TIMED_SWEEP || nType == RenderType::TIMING_EVENT_ALTERNATE_TIMED_SWEEP2) && nCount % 2 == 0)
            {
                buffer.SetPixel(buffer.BufferWi - (x + startX) - 1, y, color1);
            }
            else
            {
                buffer.SetPixel(x + startX, y, color1);
            }
        }
    }
}

void VUMeterEffect::RenderOnFrame(RenderBuffer& buffer, int gain)
{
    if (buffer.GetMedia() == nullptr) return;

    float f = 0.0;
	std::list<float> const * const pf = buffer.GetMedia()->GetFrameData(buffer.curPeriod, FRAMEDATA_HIGH, "");
	if (pf != nullptr)
	{
		f = ApplyGain(*pf->cbegin(), gain);
	}
	xlColor color1;
	buffer.palette.GetColor(0, color1);
	color1.alpha = f * (float)255;

	for (int x = 0; x < buffer.BufferWi; x++)
	{
		for (int y = 0; y < buffer.BufferHt; y++)
		{
			buffer.SetPixel(x, y, color1);
		}
	}
}

void VUMeterEffect::RenderDominantFrequencyColour(RenderBuffer& buffer, int sensitivity, int startnote, int endnote, bool gradient)
{
    if (buffer.GetMedia() == nullptr) return;

    float sns = (float)sensitivity / 100.0;

    std::list<float> const * const pdata = buffer.GetMedia()->GetFrameData(buffer.curPeriod, FRAMEDATA_VU, "");

    if (pdata != nullptr && pdata->size() != 0)
    {
        int note = -1;
        float max = -1000;
        auto it = pdata->cbegin();
        for (int i = 0; i < std::min((int)pdata->size(), endnote+1); i++)
        {
            if (i >= startnote)
            {
                if (*it > sns && *it > max)
                {
                    max = *it;
                    note = i;
                }
            }
            ++it;
        }

        if (note >= 0)
        {
            xlColor color1;
            if (gradient)
            {
                buffer.GetMultiColorBlend((float)(note - startnote) / (float)(endnote - startnote + 1), false, color1);
            }
            else
            {
                int numcolours = buffer.palette.Size();
                int colour = (float)((note - startnote) * numcolours) / (float)(endnote - startnote + 1);
                color1 = buffer.palette.GetColor(colour);
            }

            for (int x = 0; x < buffer.BufferWi; x++)
            {
                for (int y = 0; y < buffer.BufferHt; y++)
                {
                    buffer.SetPixel(x, y, color1);
                }
            }
        }
    }
}

void VUMeterEffect::RenderOnColourFrame(RenderBuffer& buffer, int gain)
{
    if (buffer.GetMedia() == nullptr) return;

    float f = 0.0;
    std::list<float> const * const pf = buffer.GetMedia()->GetFrameData(buffer.curPeriod, FRAMEDATA_HIGH, "");
    if (pf != nullptr)
    {
        f = ApplyGain(*pf->cbegin(), gain);
    }

    xlColor color1;
    buffer.GetMultiColorBlend(f, false, color1);

    for (int x = 0; x < buffer.BufferWi; x++)
    {
        for (int y = 0; y < buffer.BufferHt; y++)
        {
            buffer.SetPixel(x, y, color1);
        }
    }
}

void VUMeterEffect::RenderPulseFrame(RenderBuffer& buffer, int fadeframes, std::string timingtrack, int& lasttimingmark)
{
    EffectLayer* el = GetTiming(timingtrack);

    if (el == nullptr) return;

    int ms = buffer.curPeriod * buffer.frameTimeInMs;
    bool effectPresent = false;
    for (int j = 0; j < el->GetEffectCount(); j++)
    {
        if (el->GetEffect(j)->GetStartTimeMS() == ms)
        {
            effectPresent = true;
            break;
        }
    }
    if (effectPresent)
    {
        lasttimingmark = buffer.curPeriod;
    }

    float f = 0.0;

    if (lasttimingmark >= 0)
    {
        f = 1.0 - (((float)buffer.curPeriod - (float)lasttimingmark) / (float)fadeframes);
        if (f < 0)
        {
            f = 0;
        }
    }

    if (f > 0.0)
    {
        xlColor color1;
        buffer.palette.GetColor(0, color1);
        color1.alpha = f * (float)255;

        for (int x = 0; x < buffer.BufferWi; x++)
        {
            for (int y = 0; y < buffer.BufferHt; y++)
            {
                buffer.SetPixel(x, y, color1);
            }
        }
    }
}

void VUMeterEffect::RenderIntensityWaveFrame(RenderBuffer &buffer, int usebars, int gain)
{
    if (buffer.GetMedia() == nullptr) return;

	int start = buffer.curPeriod - usebars;
	int cols = buffer.BufferWi / usebars;
	int x = 0;
	for (int i = 0; i < usebars; i++)
	{
		if (start + i >= 0)
		{
			float f = 0.0;
			std::list<float> const * const pf = buffer.GetMedia()->GetFrameData(start + i, FRAMEDATA_HIGH, "");
			if (pf != nullptr)
			{
				f = ApplyGain(*pf->begin(), gain);
			}
			xlColor color1;
			if (buffer.palette.Size() < 2)
			{
				buffer.palette.GetColor(0, color1);
				color1.alpha = f * (float)255;
			}
			else
			{
				buffer.GetMultiColorBlend(1.0 - f, false, color1);
			}
			for (int j = 0; j < cols; j++)
			{
				for (int y = 0; y < buffer.BufferHt; y++)
				{
					buffer.SetPixel(x, y, color1);
				}
				x++;
			}
		}
		else
		{
			x += cols;
		}
	}
}

void VUMeterEffect::RenderLevelPulseFrame(RenderBuffer &buffer, int fadeframes, int sensitivity, int& lasttimingmark, int gain)
{
    if (buffer.GetMedia() == nullptr) return;

    float f = 0.0;
	std::list<float> const * const pf = buffer.GetMedia()->GetFrameData(buffer.curPeriod, FRAMEDATA_HIGH, "");
	if (pf != nullptr)
	{
		f = ApplyGain(*pf->cbegin(), gain);
	}

	if (f > (float)sensitivity / 100.0)
	{
		lasttimingmark = buffer.curPeriod;
	}

	if (fadeframes > 0 && buffer.curPeriod - lasttimingmark < fadeframes)
	{
		float ff = 1.0 - (((float)buffer.curPeriod - (float)lasttimingmark) / (float)fadeframes);
		if (ff < 0)
		{
			ff = 0;
		}

		if (ff > 0.0)
		{
			xlColor color1;
			buffer.palette.GetColor(0, color1);
			color1.alpha = ff * (float)255;

			for (int x = 0; x < buffer.BufferWi; x++)
			{
				for (int y = 0; y < buffer.BufferHt; y++)
				{
					buffer.SetPixel(x, y, color1);
				}
			}
		}
	}
}

void VUMeterEffect::RenderLevelJumpFrame(RenderBuffer& buffer, int fadeframes, int sensitivity, int& lasttimingmark, int gain, bool fullJump, float& lastVal)
{
    if (buffer.GetMedia() == nullptr) return;

    float f = 0.0;
    std::list<float> const * const pf = buffer.GetMedia()->GetFrameData(buffer.curPeriod, FRAMEDATA_HIGH, "");
    if (pf != nullptr)
    {
        f = ApplyGain(*pf->cbegin(), gain);
    }

    if (f > (float)sensitivity / 100.0)
    {
        lasttimingmark = buffer.curPeriod;

        if (fullJump)
        {
            lastVal = 1.0;
        }
        else
        {
            lastVal = f;
        }
    }

    if (fadeframes > 0 && buffer.curPeriod - lasttimingmark < fadeframes)
    {
        float ff = lastVal - (lastVal * (((float)buffer.curPeriod - (float)lasttimingmark)) / (float)fadeframes);
        if (ff < 0)
        {
            ff = 0;
        }

        if (ff > 0.0)
        {
            for (int y = 0; y < ff * (float)buffer.BufferHt; y++)
            {
                xlColor color1;
                buffer.GetMultiColorBlend((float)y / (float)buffer.BufferHt, false, color1);
                for (int x = 0; x < buffer.BufferWi; x++)
                {
                    buffer.SetPixel(x, y, color1);
                }
            }
        }
    }
}

void VUMeterEffect::RenderLevelPulseColourFrame(RenderBuffer &buffer, int fadeframes, int sensitivity, int& lasttimingmark, int& colourindex, int gain)
{
    if (buffer.GetMedia() == nullptr) return;

    float f = 0.0;
    std::list<float> const * const pf = buffer.GetMedia()->GetFrameData(buffer.curPeriod, FRAMEDATA_HIGH, "");
    if (pf != nullptr)
    {
        f = ApplyGain(*pf->begin(), gain);
    }

    if (f > (float)sensitivity / 100.0)
    {
        if (lasttimingmark != buffer.curPeriod - 1)
        {
            colourindex++;
            if (colourindex >= buffer.GetColorCount())
            {
                colourindex = 0;
            }
        }

        lasttimingmark = buffer.curPeriod;
    }

    if (fadeframes > 0 && buffer.curPeriod - lasttimingmark < fadeframes)
    {
        float ff = 1.0 - (((float)buffer.curPeriod - (float)lasttimingmark) / (float)fadeframes);
        if (ff < 0)
        {
            ff = 0;
        }

        if (ff > 0.0)
        {
            xlColor color1;
            buffer.palette.GetColor(colourindex, color1);
            color1.alpha = ff * (float)255;

            for (int x = 0; x < buffer.BufferWi; x++)
            {
                for (int y = 0; y < buffer.BufferHt; y++)
                {
                    buffer.SetPixel(x, y, color1);
                }
            }
        }
    }
}

void VUMeterEffect::RenderLevelColourFrame(RenderBuffer &buffer, int& colourindex, int sensitivity, int& lasttimingmark, int gain)
{
    if (buffer.GetMedia() == nullptr) return;

    float f = 0.0;
    std::list<float> const * const pf = buffer.GetMedia()->GetFrameData(buffer.curPeriod, FRAMEDATA_HIGH, "");
    if (pf != nullptr)
    {
        f = ApplyGain(*pf->begin(), gain);
    }

    if (f > (float)sensitivity / 100.0)
    {
        if (lasttimingmark != buffer.curPeriod - 1)
        {
            colourindex++;
            if (colourindex >= buffer.GetColorCount())
            {
                colourindex = 0;
            }
        }

        lasttimingmark = buffer.curPeriod;
    }

    xlColor color1;
    buffer.palette.GetColor(colourindex, color1);

    for (int x = 0; x < buffer.BufferWi; x++)
    {
        for (int y = 0; y < buffer.BufferHt; y++)
        {
            buffer.SetPixel(x, y, color1);
        }
    }
}

void VUMeterEffect::DrawCircle(RenderBuffer& buffer, int centerx, int centery, float radius, xlColor& color1)
{
	if (radius > 0)
	{
		float radiussquared = radius * radius;

		// fraction used to remove blank spots
		for (float x = centerx - radius; x <= centerx + radius; x = x + 0.5)
		{
			if (x >= 0 && x < buffer.BufferWi)
			{
				float zz = radiussquared - (x - centerx) * (x - centerx);
				if (zz >= 0)
				{
					int y = sqrt(zz);
					if (y + centery >= 0 && y + centery < buffer.BufferHt)
					{
						buffer.SetPixel(x, y + centery, color1);
					}
					if (-y + centery >= 0 && -y + centery < buffer.BufferHt)
					{
						buffer.SetPixel(x, -y + centery, color1);
					}
				}
			}
		}

		// do it again from the y side to ensure the circle is complete
		for (float y = centery - radius; y <= centery + radius; y = y + 0.5)
		{
			if (y >= 0 && y < buffer.BufferHt)
			{
				float zz = radiussquared - (y - centery) * (y - centery);
				if (zz >= 0)
				{
					int x = sqrt(zz);
					if (x + centerx >= 0 && x + centerx < buffer.BufferWi)
					{
						buffer.SetPixel(x + centerx, y, color1);
					}
					if (-x + centerx >= 0 && -x + centerx < buffer.BufferWi)
					{
						buffer.SetPixel(-x + centerx, y, color1);
					}
				}
			}
		}
	}
}

void VUMeterEffect::DrawStar(RenderBuffer& buffer, int centerx, int centery, float radius, xlColor& color1, int points)
{
    double offsetangle = 0.0;
    switch (points)
    {
    case 4:
        break;
    case 5:
        offsetangle = 90.0 - 360.0 / 5;
        break;
    case 6:
        offsetangle = 30.0;
        break;
    case 7:
        offsetangle = 90.0 - 360.0 / 7;
        break;
    default:
        wxASSERT(false);
        break;
    }

    if (radius > 0)
    {
        double InnerRadius = radius / 2.618034;    // divide by golden ratio squared

        double increment = 360.0 / points;

        for (double degrees = 0.0; degrees<361.0; degrees += increment) // 361 because it allows for small rounding errors
        {
            if (degrees > 360.0) degrees = 360.0;

            double radian = (offsetangle + degrees) * (M_PI / 180.0);
            int xouter = radius * cos(radian) + centerx;
            int youter = radius * sin(radian) + centery;

            radian = (offsetangle + degrees + increment / 2.0) * (M_PI / 180.0);
            int xinner = InnerRadius * cos(radian) + centerx;
            int yinner = InnerRadius * sin(radian) + centery;

            buffer.DrawLine(xinner, yinner, xouter, youter, color1);

            radian = (offsetangle + degrees - increment / 2.0) * (M_PI / 180.0);
            xinner = InnerRadius * cos(radian) + centerx;
            yinner = InnerRadius * sin(radian) + centery;

            buffer.DrawLine(xinner, yinner, xouter, youter, color1);
        }
    }
}

void VUMeterEffect::DrawBox(RenderBuffer& buffer, int startx, int endx, int starty, int endy, xlColor& color1)
{
	for (int x = startx; x <= endx; x++)
	{
		if (x >= 0 && x < buffer.BufferWi)
		{
			if (x == startx || x == endx)
			{
				for (int y = starty; y <= endy; y++)
				{
					if (y >= 0 && y < buffer.BufferHt)
					{
						buffer.SetPixel(x, y, color1);
					}
				}
			}
			else
			{
				if (starty >= 0 && starty < buffer.BufferHt)
				{
					buffer.SetPixel(x, starty, color1);
				}
				if (endy >= 0 && endy < buffer.BufferHt)
				{
					buffer.SetPixel(x, endy, color1);
				}
			}
		}
	}
}

void VUMeterEffect::DrawDiamond(RenderBuffer& buffer, int centerx, int centery, int size, xlColor& color1)
{
	for (int x = -1 * size; x <= size; x++)
	{
		if (x + centerx >= 0 && x + centerx < buffer.BufferWi)
		{
			int y = size - abs(x);

			if (y + centery >= 0 && y + centery < buffer.BufferHt)
			{
				buffer.SetPixel(x + centerx, y + centery, color1);
			}
			if (-y + centery >= 0 && -y + centery < buffer.BufferHt)
			{
				buffer.SetPixel(x + centerx, -y + centery, color1);
			}
		}
	}
}

void VUMeterEffect::DrawSnowflake(RenderBuffer &buffer, int xc, int yc, double radius, int sides, xlColor color, double rotation)
{
	double increment = 360.0 / (sides * 2);
	double angle = rotation;

	if (radius >= 0)
	{
		for (int i = 0; i < sides * 2; i++)
		{
			double radian = angle * M_PI / 180.0;

			int x1 = std::round(radius * cos(radian)) + xc;
			int y1 = std::round(radius * sin(radian)) + yc;

			radian = (180 + angle) * M_PI / 180.0;

			int x2 = std::round(radius * cos(radian)) + xc;
			int y2 = std::round(radius * sin(radian)) + yc;

			buffer.DrawLine(x1, y1, x2, y2, color);

			angle += increment;
		}
	}
}

void VUMeterEffect::DrawHeart(RenderBuffer &buffer, int xc, int yc, double radius, xlColor color, int thickness)
{
	double interpolation = 0.75;
	double t = (double)thickness - 1.0 + interpolation;

	for (double x = -2.0; x <= 2.0; x += 0.01f)
	{
		double y1 = std::sqrt(1.0 - (std::abs(x) - 1.0) * (std::abs(x) - 1.0));
		double y2 = std::acos(1.0 - std::abs(x)) - M_PI;

		double r = radius;

		for (double i = 0.0; i < t; i += interpolation)
		{
			if (r >= 0)
			{
				buffer.SetPixel(std::round(x * r / 2.0) + xc, std::round(y1 * r / 2.0) + yc, color);
				buffer.SetPixel(std::round(x * r / 2.0) + xc, std::round(y2 * r / 2.0) + yc, color);
			}
			else
			{
				break;
			}
			r -= interpolation;
		}
	}
}

void VUMeterEffect::DrawTree(RenderBuffer &buffer, int xc, int yc, double radius, xlColor color, int thickness)
{
	struct line
	{
		wxPoint start;
		wxPoint end;

		line(const wxPoint s, const wxPoint e)
		{
			start = s;
			end = e;
		}
	};

	const line points[] = { line(wxPoint(3,0), wxPoint(5,0)),
		line(wxPoint(5,0), wxPoint(5,3)),
		line(wxPoint(3,0), wxPoint(3,3)),
		line(wxPoint(0,3), wxPoint(8,3)),
		line(wxPoint(0,3), wxPoint(2,6)),
		line(wxPoint(8,3), wxPoint(6,6)),
		line(wxPoint(1,6), wxPoint(2,6)),
		line(wxPoint(6,6), wxPoint(7,6)),
		line(wxPoint(1,6), wxPoint(3,9)),
		line(wxPoint(7,6), wxPoint(5,9)),
		line(wxPoint(2,9), wxPoint(3,9)),
		line(wxPoint(5,9), wxPoint(6,9)),
		line(wxPoint(6,9), wxPoint(4,11)),
		line(wxPoint(2,9), wxPoint(4,11))
	};
	int count = sizeof(points) / sizeof(line);

	double interpolation = 0.75;
	double t = (double)thickness - 1.0 + interpolation;

	for (double i = 0; i < t; i += interpolation)
	{
		if (radius >= 0)
		{
			for (int j = 0; j < count; ++j)
			{
				int x1 = std::round(((double)points[j].start.x - 4.0) / 11.0 * radius);
				int y1 = std::round(((double)points[j].start.y - 4.0) / 11.0 * radius);
				int x2 = std::round(((double)points[j].end.x - 4.0) / 11.0 * radius);
				int y2 = std::round(((double)points[j].end.y - 4.0) / 11.0 * radius);
				buffer.DrawLine(xc + x1, yc + y1, xc + x2, yc + y2, color);
			}
		}
		else
		{
			break;
		}
		radius -= interpolation;
	}
}

void VUMeterEffect::DrawCrucifix(RenderBuffer &buffer, int xc, int yc, double radius, xlColor color, int thickness)
{
	struct line
	{
		wxPoint start;
		wxPoint end;

		line(const wxPoint s, const wxPoint e)
		{
			start = s;
			end = e;
		}
	};

	const line points[] = { line(wxPoint(2,0), wxPoint(2,6)),
		line(wxPoint(2,6), wxPoint(0,6)),
		line(wxPoint(0,6), wxPoint(0,7)),
		line(wxPoint(0,7), wxPoint(2,7)),
		line(wxPoint(2,7), wxPoint(2,10)),
		line(wxPoint(2,10), wxPoint(3,10)),
		line(wxPoint(3,10), wxPoint(3,7)),
		line(wxPoint(3,7), wxPoint(5,7)),
		line(wxPoint(5,7), wxPoint(5,6)),
		line(wxPoint(5,6), wxPoint(3,6)),
		line(wxPoint(3,6), wxPoint(3,0)),
		line(wxPoint(3,0), wxPoint(2,0))
	};
	int count = sizeof(points) / sizeof(line);

	double interpolation = 0.75;
	double t = (double)thickness - 1.0 + interpolation;

	for (double i = 0; i < t; i += interpolation)
	{
		if (radius >= 0)
		{
			for (int j = 0; j < count; ++j)
			{
				int x1 = std::round(((double)points[j].start.x - 2.5) / 7.0 * radius);
				int y1 = std::round(((double)points[j].start.y - 6.5) / 10.0 * radius);
				int x2 = std::round(((double)points[j].end.x - 2.5) / 7.0 * radius);
				int y2 = std::round(((double)points[j].end.y - 6.5) / 10.0 * radius);
				buffer.DrawLine(xc + x1, yc + y1, xc + x2, yc + y2, color);
			}
		}
		else
		{
			break;
		}
		radius -= interpolation;
	}
}

void VUMeterEffect::DrawPresent(RenderBuffer &buffer, int xc, int yc, double radius, xlColor color, int thickness)
{
	struct line
	{
		wxPoint start;
		wxPoint end;

		line(const wxPoint s, const wxPoint e)
		{
			start = s;
			end = e;
		}
	};

	const line points[] = { line(wxPoint(0,0), wxPoint(0,9)),
		line(wxPoint(0,9), wxPoint(10,9)),
		line(wxPoint(10,9), wxPoint(10,0)),
		line(wxPoint(10,0), wxPoint(0,0)),
		line(wxPoint(5,0), wxPoint(5,9)),
		line(wxPoint(5,9), wxPoint(2,11)),
		line(wxPoint(2,11), wxPoint(2,9)),
		line(wxPoint(5,9), wxPoint(8,11)),
		line(wxPoint(8,11), wxPoint(8,9))
	};
	int count = sizeof(points) / sizeof(line);

	double interpolation = 0.75;
	double t = (double)thickness - 1.0 + interpolation;

	for (double i = 0; i < t; i += interpolation)
	{
		if (radius >= 0)
		{
			for (int j = 0; j < count; ++j)
			{
				int x1 = std::round(((double)points[j].start.x - 5) / 7.0 * radius);
				int y1 = std::round(((double)points[j].start.y - 5.5) / 10.0 * radius);
				int x2 = std::round(((double)points[j].end.x - 5) / 7.0 * radius);
				int y2 = std::round(((double)points[j].end.y - 5.5) / 10.0 * radius);
				buffer.DrawLine(xc + x1, yc + y1, xc + x2, yc + y2, color);
			}
		}
		else
		{
			break;
		}
		radius -= interpolation;
	}
}

float VUMeterEffect::ApplyGain(float value, int gain) const
{
    float v = (100.0 + gain) * value / 100.0;
    if (v > 1.0) v = 1.0;
    return v;
}

void VUMeterEffect::DrawCandycane(RenderBuffer &buffer, int xc, int yc, double radius, xlColor color, int thickness) const
{
	double originalRadius = radius;
	double interpolation = 0.75;
	double t = (double)thickness - 1.0 + interpolation;
	for (double i = 0; i < t; i += interpolation)
	{
		if (radius >= 0)
		{
			// draw the stick
			int y1 = std::round((double)yc + originalRadius / 6.0);
			int y2 = std::round((double)yc - originalRadius / 2.0);
			int x = std::round((double)xc + radius / 2.0);
			buffer.DrawLine(x, y1, x, y2, color);

			// draw the hook
			double r = radius / 3.0;
			for (double degrees = 0.0; degrees < 180; degrees += 1.0)
			{
				double radian = degrees * (M_PI / 180.0);
				x = std::round((r - interpolation) * buffer.cos(radian) + xc + originalRadius / 6.0);
				int y = std::round((r - interpolation) * buffer.sin(radian) + y1);
				buffer.SetPixel(x, y, color);
			}
		}
		else
		{
			break;
		}
		radius -= interpolation;
	}
}

void VUMeterEffect::RenderLevelShapeFrame(RenderBuffer& buffer, const std::string& shape, float& lastsize, int scale, bool slowdownfalls, int xoffset, int yoffset, int usebars, int gain)
{
    if (buffer.GetMedia() == nullptr) return;

	int nShape = DecodeShape(shape);

    // star points
    if (usebars > 99) usebars = 99;
    int points = usebars / 25 + 4;

    int truexoffset = xoffset * buffer.BufferWi / 2 / 100;
    int trueyoffset = yoffset * buffer.BufferHt / 2 / 100;
    float scaling = (float)scale / 100.0 * 7.0;

	float f = 0.0;
	std::list<float> const * const pf = buffer.GetMedia()->GetFrameData(buffer.curPeriod, FRAMEDATA_HIGH, "");
	if (pf != nullptr)
	{
		f = ApplyGain(*pf->begin(), gain);
	}

	int centerx = (buffer.BufferWi / 2.0) + truexoffset;
	int centery = (buffer.BufferHt / 2.0) + trueyoffset;

	//old "maxSide" and "maxradius" were the same calculation
	float maxSize = std::min(buffer.BufferHt / 2.0, buffer.BufferWi / 2.0) * scaling;
	//old "side" and "radius" were the same calculation
	float size = maxSize * f;

	if (slowdownfalls)
	{
		if (size < lastsize)
		{
			lastsize = lastsize - std::min(maxSize, (float)std::max(buffer.BufferHt / 2.0, buffer.BufferWi / 2.0)) / 20.0;
			if (lastsize < size)
			{
				lastsize = size;
			}
		}
		else
		{
			lastsize = size;
		}
	}
	else
	{
		lastsize = size;
	}

	if (nShape == ShapeType::CIRCLE)
	{
		xlColor color1;
		buffer.palette.GetColor(0, color1);

		color1.alpha = 0.25 * 255;
		DrawCircle(buffer, centerx, centery, lastsize - 2, color1);
		DrawCircle(buffer, centerx, centery, lastsize + 2, color1);
		color1.alpha = 0.5 * 255;
		DrawCircle(buffer, centerx, centery, lastsize - 1, color1);
		DrawCircle(buffer, centerx, centery, lastsize + 1, color1);
		color1.alpha = 255;
		DrawCircle(buffer, centerx, centery, lastsize, color1);
	}
	else if (nShape == ShapeType::FILLED_CIRCLE)
	{
		for (int x = 0; x <= lastsize; x++)
		{
			float distance = (float)x / lastsize;
			xlColor color1;
			buffer.GetMultiColorBlend(distance, false, color1);
			DrawCircle(buffer, centerx, centery, x, color1);
		}
	}
	else if(nShape == ShapeType::SQUARE)
	{
		int startx = (int)(centerx - lastsize / 2.0);
		int endx = (int)(centerx + lastsize / 2.0);
		int starty = (int)(centery - lastsize / 2.0);
		int endy = (int)(centery + lastsize / 2.0);
		xlColor color1;
		buffer.palette.GetColor(0, color1);

		color1.alpha = 0.25 * 255;
		DrawBox(buffer, startx - 2, endx + 2, starty - 2, endy+2, color1);
		DrawBox(buffer, startx + 2, endx - 2, starty + 2, endy - 2, color1);
		color1.alpha = 0.5 * 255;
		DrawBox(buffer, startx - 1, endx + 1, starty - 1, endy + 1, color1);
		DrawBox(buffer, startx + 1, endx - 1, starty + 1, endy - 1, color1);
		color1.alpha = 255;
		DrawBox(buffer, startx, endx, starty, endy, color1);
	}
	else if (nShape == ShapeType::FILLED_SQUARE)
	{
		int startx = (int)(centerx - lastsize / 2.0);
		int endx = (int)(centerx + lastsize / 2.0);
		int starty = (int)(centery - lastsize / 2.0);
		int endy = (int)(centery + lastsize / 2.0);
		for (int x = 0; x <= lastsize / 2.0; x++)
		{
			float distance = x / (lastsize / 2.0);
			xlColor color1;
			buffer.GetMultiColorBlend(distance, false, color1);
			DrawBox(buffer, startx + x, endx - x, starty + x, endy - x, color1);
		}
	}
	else if (nShape == ShapeType::DIAMOND)
	{
		xlColor color1;
		buffer.palette.GetColor(0, color1);
		color1.alpha = 0.25 * 255;
		DrawDiamond(buffer, centerx, centery, lastsize - 2, color1);
		DrawDiamond(buffer, centerx, centery, lastsize + 2, color1);
		color1.alpha = 0.5 * 255;
		DrawDiamond(buffer, centerx, centery, lastsize - 1, color1);
		DrawDiamond(buffer, centerx, centery, lastsize + 1, color1);
		color1.alpha = 255;
		DrawDiamond(buffer, centerx, centery, lastsize, color1);
	}
	else if (nShape == ShapeType::FILLED_DIAMOND)
	{
		for (int xx = 0; xx <= lastsize; xx++)
		{
			xlColor color1;
			buffer.GetMultiColorBlend(xx / lastsize, false, color1);
			DrawDiamond(buffer, centerx, centery, xx, color1);
		}
	}
    else if (nShape == ShapeType::STAR)
    {
        xlColor color1;
        buffer.palette.GetColor(0, color1);
        color1.alpha = 0.25 * 255;
        DrawStar(buffer, centerx, centery, lastsize - 2, color1, points);
        DrawStar(buffer, centerx, centery, lastsize + 2, color1, points);
        color1.alpha = 0.5 * 255;
        DrawStar(buffer, centerx, centery, lastsize - 1, color1, points);
        DrawStar(buffer, centerx, centery, lastsize + 1, color1, points);
        color1.alpha = 255;
        DrawStar(buffer, centerx, centery, lastsize, color1, points);
    }
    else if (nShape == ShapeType::FILLED_STAR)
    {
        for (float x = 0; x <= lastsize; x+=0.5f)
        {
            float distance = x / lastsize;
            xlColor color1;
            buffer.GetMultiColorBlend(distance, false, color1);
            DrawStar(buffer, centerx, centery, x, color1, points);
        }
    }
	else if (nShape == ShapeType::TREE)
	{
		xlColor color1;
		buffer.palette.GetColor(0, color1);
		color1.alpha = 0.25 * 255;
		DrawTree(buffer, centerx, centery, lastsize - 2, color1);
		DrawTree(buffer, centerx, centery, lastsize + 2, color1);
		color1.alpha = 0.5 * 255;
		DrawTree(buffer, centerx, centery, lastsize - 1, color1);
		DrawTree(buffer, centerx, centery, lastsize + 1, color1);
		color1.alpha = 255;
		DrawTree(buffer, centerx, centery, lastsize, color1);
	}
	else if (nShape == ShapeType::FILLED_TREE)
	{
		for (float x = 0; x <= lastsize; x += 0.5f)
		{
			float distance = x / lastsize;
			xlColor color1;
			buffer.GetMultiColorBlend(distance, false, color1);
			DrawTree(buffer, centerx, centery, x, color1);
		}
	}
	else if (nShape == ShapeType::CRUCIFIX)
	{
		xlColor color1;
		buffer.palette.GetColor(0, color1);
		color1.alpha = 0.25 * 255;
		DrawCrucifix(buffer, centerx, centery, lastsize - 2, color1);
		DrawCrucifix(buffer, centerx, centery, lastsize + 2, color1);
		color1.alpha = 0.5 * 255;
		DrawCrucifix(buffer, centerx, centery, lastsize - 1, color1);
		DrawCrucifix(buffer, centerx, centery, lastsize + 1, color1);
		color1.alpha = 255;
		DrawCrucifix(buffer, centerx, centery, lastsize, color1);
	}
	else if (nShape == ShapeType::FILLED_CRUCIFIX)
	{
		for (float x = 0; x <= lastsize; x += 0.5f)
		{
			float distance = x / lastsize;
			xlColor color1;
			buffer.GetMultiColorBlend(distance, false, color1);
			DrawCrucifix(buffer, centerx, centery, x, color1);
		}
	}
    else if (nShape == ShapeType::PRESENT)
    {
        xlColor color1;
        buffer.palette.GetColor(0, color1);
        color1.alpha = 0.25 * 255;
		DrawPresent(buffer, centerx, centery, lastsize - 2, color1);
		DrawPresent(buffer, centerx, centery, lastsize + 2, color1);
        color1.alpha = 0.5 * 255;
        DrawPresent(buffer, centerx, centery, lastsize - 1, color1);
		DrawPresent(buffer, centerx, centery, lastsize + 1, color1);
        color1.alpha = 255;
		DrawPresent(buffer, centerx, centery, lastsize, color1);
    }
    else if (nShape == ShapeType::FILLED_PRESENT)
	{
		for (int x = 0; x <= lastsize; x++)
		{
			float distance = (float)x / lastsize;
			xlColor color1;
			buffer.GetMultiColorBlend(distance, false, color1);
			DrawPresent(buffer, centerx, centery, x, color1);
		}
	}
	else if (nShape == ShapeType::CANDY_CANE)
	{
		xlColor color1;
		buffer.palette.GetColor(0, color1);
		color1.alpha = 0.25 * 255;
		DrawCandycane(buffer, centerx, centery, lastsize - 2, color1);
		DrawCandycane(buffer, centerx, centery, lastsize + 2, color1);
		color1.alpha = 0.5 * 255;
		DrawCandycane(buffer, centerx, centery, lastsize - 1, color1);
		DrawCandycane(buffer, centerx, centery, lastsize + 1, color1);
		color1.alpha = 255;
		DrawCandycane(buffer, centerx, centery, lastsize, color1);
	}
	else if (nShape == ShapeType::SNOWFLAKE)
	{
		xlColor color1;
		buffer.palette.GetColor(0, color1);
		color1.alpha = 0.25 * 255;
		DrawSnowflake(buffer, centerx, centery, lastsize - 2, points, color1);
		DrawSnowflake(buffer, centerx, centery, lastsize + 2, points, color1);
		color1.alpha = 0.5 * 255;
		DrawSnowflake(buffer, centerx, centery, lastsize - 1, points, color1);
		DrawSnowflake(buffer, centerx, centery, lastsize + 1, points, color1);
		color1.alpha = 255;
		DrawSnowflake(buffer, centerx, centery, lastsize, points, color1);
	}
	else if (nShape == ShapeType::HEART)
	{
		xlColor color1;
		buffer.palette.GetColor(0, color1);
		color1.alpha = 0.25 * 255;
		DrawHeart(buffer, centerx, centery, lastsize - 2, color1);
		DrawHeart(buffer, centerx, centery, lastsize + 2, color1);
		color1.alpha = 0.5 * 255;
		DrawHeart(buffer, centerx, centery, lastsize - 1, color1);
		DrawHeart(buffer, centerx, centery, lastsize + 1, color1);
		color1.alpha = 255;
		DrawHeart(buffer, centerx, centery, lastsize, color1, 1);
	}
	else if (nShape == ShapeType::FILLED_HEART)
	{
		for (int x = 0; x <= lastsize; x++)
		{
			float distance = (float)x / lastsize;
			xlColor color1;
			buffer.GetMultiColorBlend(distance, false, color1);
			DrawHeart(buffer, centerx, centery, x, color1);
		}
	}
}

void VUMeterEffect::RenderTimingEventJumpFrame(RenderBuffer &buffer, int fallframes, std::string timingtrack, float& lastsize, bool useAudioLevel, int gain)
{
    if (useAudioLevel && buffer.GetMedia() == nullptr) return;

    if (timingtrack != "")
    {
        Element* t = nullptr;
        for (int i = 0; i < mSequenceElements->GetElementCount(); i++)
        {
            Element* e = mSequenceElements->GetElement(i);
            if (e->GetEffectLayerCount() == 1 && e->GetType() == ElementType::ELEMENT_TYPE_TIMING
                && e->GetName() == timingtrack)
            {
                t = e;
                break;
            }
        }

        if (t != nullptr)
        {
            EffectLayer* el = t->GetEffectLayer(0);
            int ms = buffer.curPeriod*buffer.frameTimeInMs;
            bool effectPresent = false;
            for (int j = 0; j < el->GetEffectCount(); j++)
            {
                if (el->GetEffect(j)->GetStartTimeMS() == ms)
                {
                    effectPresent = true;
                    break;
                }
            }

            if (effectPresent)
            {
                if (useAudioLevel)
                {
                    float f = 0.0;
                    std::list<float> const * const pf = buffer.GetMedia()->GetFrameData(buffer.curPeriod, FRAMEDATA_HIGH, "");
                    if (pf != nullptr)
                    {
                        f = ApplyGain(*pf->cbegin(), gain);
                    }
                    lastsize = f;
                }
                else
                {
                    lastsize = 1.0;
                }
            }
        }

        if (lastsize > 0)
        {
            for (int y = 0; y < buffer.BufferHt * lastsize; y++)
            {
                xlColor color;
                buffer.GetMultiColorBlend((float)y / (float)buffer.BufferHt, false, color);
                for (int x = 0; x < buffer.BufferWi; x++)
                {
                    buffer.SetPixel(x, y, color);
                }
            }

            lastsize -= 1.0 / (float)fallframes;
            if (lastsize < 0)
            {
                lastsize = 0;
            }
        }
    }
}

void VUMeterEffect::RenderTimingEventPulseFrame(RenderBuffer &buffer, int fadeframes, std::string timingtrack, float& lastsize)
{
    if (timingtrack != "")
    {
        Element* t = nullptr;
        for (int i = 0; i < mSequenceElements->GetElementCount(); i++)
        {
            Element* e = mSequenceElements->GetElement(i);
            if (e->GetEffectLayerCount() == 1 && e->GetType() == ElementType::ELEMENT_TYPE_TIMING
                && e->GetName() == timingtrack)
            {
                t = e;
                break;
            }
        }

        if (t != nullptr)
        {
            EffectLayer* el = t->GetEffectLayer(0);
            int ms = buffer.curPeriod*buffer.frameTimeInMs;
            bool effectPresent = false;
            for (int j = 0; j < el->GetEffectCount(); j++)
            {
                if (el->GetEffect(j)->GetStartTimeMS() == ms)
                {
                    effectPresent = true;
                    break;
                }
            }

            if (effectPresent)
            {
                lastsize = fadeframes;
            }
        }

        if (lastsize > 0)
        {
            xlColor color;
            buffer.palette.GetColor(0, color);
            color.alpha = lastsize * 255 / fadeframes;
            for (int y = 0; y < buffer.BufferHt * lastsize; y++)
            {
                for (int x = 0; x < buffer.BufferWi; x++)
                {
                    buffer.SetPixel(x, y, color);
                }
            }

            lastsize--;
        }
    }
}

void VUMeterEffect::RenderTimingEventPulseColourFrame(RenderBuffer &buffer, int fadeframes, std::string timingtrack, float& lastsize, int& colourindex)
{
    if (timingtrack != "")
    {
        Element* t = nullptr;
        for (int i = 0; i < mSequenceElements->GetElementCount(); i++)
        {
            Element* e = mSequenceElements->GetElement(i);
            if (e->GetEffectLayerCount() == 1 && e->GetType() == ElementType::ELEMENT_TYPE_TIMING
                && e->GetName() == timingtrack)
            {
                t = e;
                break;
            }
        }

        if (t != nullptr)
        {
            EffectLayer* el = t->GetEffectLayer(0);
            int ms = buffer.curPeriod*buffer.frameTimeInMs;
            bool effectPresent = false;
            for (int j = 0; j < el->GetEffectCount(); j++)
            {
                if (el->GetEffect(j)->GetStartTimeMS() == ms)
                {
                    effectPresent = true;
                    break;
                }
            }

            if (effectPresent)
            {
                lastsize = fadeframes;
                colourindex++;
                if (colourindex >= buffer.GetColorCount())
                {
                    colourindex = 0;
                }
            }
        }

        if (lastsize > 0)
        {
            xlColor color;
            buffer.palette.GetColor(colourindex, color);
            color.alpha = lastsize * 255 / fadeframes;
            for (int y = 0; y < buffer.BufferHt * lastsize; y++)
            {
                for (int x = 0; x < buffer.BufferWi; x++)
                {
                    buffer.SetPixel(x, y, color);
                }
            }

            lastsize--;
        }
    }
}

void VUMeterEffect::RenderTimingEventColourFrame(RenderBuffer &buffer, int& colourindex, std::string timingtrack)
{
    if (timingtrack != "")
    {
        Element* t = nullptr;
        for (int i = 0; i < mSequenceElements->GetElementCount(); i++)
        {
            Element* e = mSequenceElements->GetElement(i);
            if (e->GetEffectLayerCount() == 1 && e->GetType() == ElementType::ELEMENT_TYPE_TIMING
                && e->GetName() == timingtrack)
            {
                t = e;
                break;
            }
        }

        if (t != nullptr)
        {
            EffectLayer* el = t->GetEffectLayer(0);
            int ms = buffer.curPeriod*buffer.frameTimeInMs;
            bool effectPresent = false;
            for (int j = 0; j < el->GetEffectCount(); j++)
            {
                if (el->GetEffect(j)->GetStartTimeMS() == ms)
                {
                    effectPresent = true;
                    break;
                }
            }

            if (effectPresent)
            {
                colourindex++;
                if (colourindex >= buffer.GetColorCount())
                {
                    colourindex = 0;
                }
            }

            if (colourindex < 0) colourindex = 0;

            xlColor color;
            buffer.palette.GetColor(colourindex, color);

            for (int x = 0; x < buffer.BufferWi; x++)
            {
                for (int y = 0; y < buffer.BufferHt; y++)
                {
                    buffer.SetPixel(x, y, color);
                }
            }
        }
    }
}

void VUMeterEffect::RenderNoteOnFrame(RenderBuffer& buffer, int startNote, int endNote, int gain)
{
    if (buffer.GetMedia() == nullptr) return;

    std::list<float> const * const pdata = buffer.GetMedia()->GetFrameData(buffer.curPeriod, FRAMEDATA_VU, "");

    if (pdata != nullptr && pdata->size() != 0)
    {
        int i = 0;
        float level = 0.0;
        for (const auto& it : *pdata)
        {
            if (i > startNote && i <= endNote)
            {
                level = std::max(it, level);
            }
            i++;
        }

        level = ApplyGain(level, gain);

        xlColor color1;
        buffer.palette.GetColor(0, color1);
        color1.alpha = level * (float)255;

        for (int x = 0; x < buffer.BufferWi; x++)
        {
            for (int y = 0; y < buffer.BufferHt; y++)
            {
                buffer.SetPixel(x, y, color1);
            }
        }
    }
}

void VUMeterEffect::RenderNoteLevelPulseFrame(RenderBuffer& buffer, int fadeframes, int sensitivity, int& lasttimingmark, int startNote, int endNote, int gain)
{
    if (buffer.GetMedia() == nullptr) return;

    std::list<float>const * const pdata = buffer.GetMedia()->GetFrameData(buffer.curPeriod, FRAMEDATA_VU, "");

    if (pdata != nullptr && pdata->size() != 0)
    {
        int i = 0;
        float level = 0.0;
        for (const auto& it : *pdata)
        {
            if (i > startNote && i <= endNote)
            {
                level = std::max(it, level);
            }
            i++;
        }

        level = ApplyGain(level, gain);

        if (level > (float)sensitivity / 100.0)
        {
            lasttimingmark = buffer.curPeriod;
        }

        if (fadeframes > 0 && buffer.curPeriod - lasttimingmark < fadeframes)
        {
            float ff = 1.0 - (((float)buffer.curPeriod - (float)lasttimingmark) / (float)fadeframes);
            if (ff < 0)
            {
                ff = 0;
            }

            if (ff > 0.0)
            {
                xlColor color1;
                buffer.palette.GetColor(0, color1);
                color1.alpha = ff * (float)255;

                for (int x = 0; x < buffer.BufferWi; x++)
                {
                    for (int y = 0; y < buffer.BufferHt; y++)
                    {
                        buffer.SetPixel(x, y, color1);
                    }
                }
            }
        }
    }
}

void VUMeterEffect::RenderNoteLevelJumpFrame(RenderBuffer& buffer, int fadeframes, int sensitivity, int& lasttimingmark, int startNote, int endNote, int gain, bool fullJump, float& lastsize)
{
    if (buffer.GetMedia() == nullptr) return;

    std::list<float> const * const pdata = buffer.GetMedia()->GetFrameData(buffer.curPeriod, FRAMEDATA_VU, "");

    if (pdata != nullptr && pdata->size() != 0)
    {
        int i = 0;
        float level = 0.0;
        for (const auto& it : *pdata)
        {
            if (i > startNote && i <= endNote)
            {
                level = std::max(it, level);
            }
            i++;
        }

        level = ApplyGain(level, gain);

        if (level > (float)sensitivity / 100.0)
        {
            lasttimingmark = buffer.curPeriod;
            if (fullJump)
            {
                lastsize = 1;
            }
            else
            {
                lastsize = level;
            }
        }

        if (fadeframes > 0 && buffer.curPeriod - lasttimingmark < fadeframes)
        {
            float ff = lastsize - ((lastsize * ((float)buffer.curPeriod - (float)lasttimingmark)) / (float)fadeframes);
            if (ff < 0)
            {
                ff = 0;
            }

            if (ff > 0.0)
            {
                for (int y = 0; y < ff * (float)buffer.BufferHt; y++)
                {
                    xlColor color1;
                    buffer.GetMultiColorBlend((float)y / (float)buffer.BufferHt, false, color1);
                    for (int x = 0; x < buffer.BufferWi; x++)
                    {
                        buffer.SetPixel(x, y, color1);
                    }
                }
            }
        }
    }
}

void VUMeterEffect::RenderLevelBarFrame(RenderBuffer &buffer, int bars, int sensitivity, float& lastbar, int& colourindex, int gain)
{
    if (buffer.GetMedia() == nullptr) return;

    std::list<float> const * const pdata = buffer.GetMedia()->GetFrameData(buffer.curPeriod, FRAMEDATA_HIGH, "");

    if (pdata != nullptr && pdata->size() != 0)
    {
        float level = ApplyGain(pdata->front(), gain);

        xlColor color1;
        if (level > (float)sensitivity / 100.0)
        {
            colourindex++;
            if (colourindex >= buffer.GetColorCount())
            {
                colourindex = 0;
            }

            lastbar++;
            if (lastbar > bars) lastbar = 1;
        }
        int bar = lastbar - 1;
        buffer.palette.GetColor(colourindex, color1);

        int startx = buffer.BufferWi / bars * bar;
        int endx = std::ceil(buffer.BufferWi / bars) * (bar + 1);
        if (endx > buffer.BufferWi) endx = buffer.BufferWi;

        if (bar >= 0)
        {
            for (int x = startx; x < endx; ++x)
            {
                for (int y = 0; y < buffer.BufferHt; ++y)
                {
                    buffer.SetPixel(x, y, color1);
                }
            }
        }
    }
}

void VUMeterEffect::RenderTimingEventBarFrame(RenderBuffer &buffer, int bars, std::string timingtrack, float& lastbar, int& colourindex, bool all)
{
    if (timingtrack != "")
    {
        Element* t = nullptr;
        for (int i = 0; i < mSequenceElements->GetElementCount(); i++)
        {
            Element* e = mSequenceElements->GetElement(i);
            if (e->GetEffectLayerCount() == 1 && e->GetType() == ElementType::ELEMENT_TYPE_TIMING
                && e->GetName() == timingtrack)
            {
                t = e;
                break;
            }
        }

        if (t != nullptr)
        {
            EffectLayer* el = t->GetEffectLayer(0);
            int ms = buffer.curPeriod*buffer.frameTimeInMs;
            bool effectPresent = false;
            for (int j = 0; j < el->GetEffectCount(); j++)
            {
                if (el->GetEffect(j)->GetStartTimeMS() == ms)
                {
                    effectPresent = true;
                    break;
                }
            }

            if (effectPresent)
            {
                colourindex++;
                if (colourindex >= buffer.GetColorCount())
                {
                    colourindex = 0;
                }

                lastbar++;
                if (lastbar > bars) lastbar = 1;
            }

            if (colourindex < 0) colourindex = 0;

            int bar = lastbar - 1;
            xlColor color;
            buffer.palette.GetColor(colourindex, color);
            int ci = colourindex;

            if (all)
            {
                for (int i = 0; i < bars ;i++)
                {
                    int startx = buffer.BufferWi / bars * i;
                    int endx = std::ceil(buffer.BufferWi / bars) * (i + 1);
                    if (endx > buffer.BufferWi) endx = buffer.BufferWi;

                    for (int x = startx; x < endx; x++)
                    {
                        for (int y = 0; y < buffer.BufferHt; y++)
                        {
                            buffer.SetPixel(x, y, color);
                        }
                    }

                    ci++;
                    if (ci == buffer.GetColorCount()) ci = 0;
                    buffer.palette.GetColor(ci, color);
                }
            }
            else
            {
                int startx = buffer.BufferWi / bars * bar;
                int endx = std::ceil(buffer.BufferWi / bars) * (bar + 1);
                if (endx > buffer.BufferWi) endx = buffer.BufferWi;

                if (bar >= 0)
                {
                    for (int x = startx; x < endx; x++)
                    {
                        for (int y = 0; y < buffer.BufferHt; y++)
                        {
                            buffer.SetPixel(x, y, color);
                        }
                    }
                }
            }
        }
    }
}

void VUMeterEffect::RenderNoteLevelBarFrame(RenderBuffer &buffer, int bars, int sensitivity, float& lastbar, int& colourindex, int startNote, int endNote, int gain)
{
    if (buffer.GetMedia() == nullptr) return;

    std::list<float> const * const pdata = buffer.GetMedia()->GetFrameData(buffer.curPeriod, FRAMEDATA_VU, "");

    if (pdata != nullptr && pdata->size() != 0)
    {
        int i = 0;
        float level = 0.0;
        for (const auto& it : *pdata)
        {
            if (i > startNote && i <= endNote)
            {
                level = std::max(it, level);
            }
            i++;
        }

        level = ApplyGain(level, gain);

        xlColor color1;
        if (level > (float)sensitivity / 100.0)
        {
            colourindex++;
            if (colourindex >= buffer.GetColorCount())
            {
                colourindex = 0;
            }

            lastbar++;
            if (lastbar > bars) lastbar = 1;
        }

        int bar = lastbar - 1;
        buffer.palette.GetColor(colourindex, color1);

        int startx = buffer.BufferWi / bars * bar;
        int endx = std::ceil(buffer.BufferWi / bars) * (bar + 1);
        if (endx > buffer.BufferWi) endx = buffer.BufferWi;

        if (bar >= 0)
        {
            for (int x = startx; x < endx; ++x)
            {
                for (int y = 0; y < buffer.BufferHt; ++y)
                {
                    buffer.SetPixel(x, y, color1);
                }
            }
        }
    }
}
