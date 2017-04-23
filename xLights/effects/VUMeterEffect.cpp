#include "VUMeterEffect.h"
#include "VUMeterPanel.h"
#include "../AudioManager.h"
#include "../sequencer/SequenceElements.h"

#include "../sequencer/Effect.h"
#include "../RenderBuffer.h"
#include "../UtilClasses.h"
#include "../models/Model.h"
#include "../SequenceCheck.h"

#include "../../include/vumeter-16.xpm"
#include "../../include/vumeter-24.xpm"
#include "../../include/vumeter-32.xpm"
#include "../../include/vumeter-48.xpm"
#include "../../include/vumeter-64.xpm"

#include <algorithm>

VUMeterEffect::VUMeterEffect(int id) : RenderableEffect(id, "VU Meter", vumeter_16, vumeter_24, vumeter_32, vumeter_48, vumeter_64)
{
}

VUMeterEffect::~VUMeterEffect()
{
}

std::list<std::string> VUMeterEffect::CheckEffectSettings(const SettingsMap& settings, AudioManager* media, Model* model, Effect* eff)
{
    std::list<std::string> res;

    wxString type = settings.Get("E_CHOICE_VUMeter_Type", "Waveform");

    if (media == nullptr && 
        (type == "Spectrogram" ||
         type == "Volume Bars" || 
         type == "Waveform" ||
         type == "On" || 
        type == "Intensity Wave" ||
        type == "Level Pulse" ||
        type == "Level Shape" ||
        type == "Color On" ||
        type == "Note On" ||
        type == "Note Level Pulse" ||
        type == "Timing Event Jump"))
    {
        res.push_back(wxString::Format("    ERR: VU Meter effect '%s' is pointless if there is no music. Model '%s', Start %s", type, model->GetName(), FORMATTIME(eff->GetStartTimeMS())).ToStdString());
    }

    wxString timing = settings.Get("E_CHOICE_VUMeter_TimingTrack", "");

    if (timing == "" &&
        (type == "Timing Event Spike" ||
         type == "Timing Event Sweep" ||
            type == "Timing Event Color" ||
            type == "Timing Event Pulse" ||
            type == "Timing Event Jump 100" ||
            type == "Timing Event Jump"))
    {
        res.push_back(wxString::Format("    ERR: VU Meter effect '%s' needs a timing track. Model '%s', Start %s", type, model->GetName(), FORMATTIME(eff->GetStartTimeMS())).ToStdString());
    }

    return res;
}

wxPanel *VUMeterEffect::CreatePanel(wxWindow *parent) {
	return new VUMeterPanel(parent);
}

void VUMeterEffect::SetPanelStatus(Model* cls)
{
    VUMeterPanel *vp = (VUMeterPanel*)panel;
    if (vp == nullptr)
    {
        return;
    }

    vp->Choice_VUMeter_TimingTrack->Clear();
    if (mSequenceElements == nullptr)
    {
        return;
    }

    // Load the names of the timing tracks
    for (int i = 0; i < mSequenceElements->GetElementCount(); i++)
    {
        Element* e = mSequenceElements->GetElement(i);
        if (e->GetType() == ELEMENT_TYPE_TIMING)
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

void VUMeterEffect::SetDefaultParameters(Model *cls) 
{
    VUMeterPanel *vp = (VUMeterPanel*)panel;
    if (vp == nullptr) {
        return;
    }

    SetSliderValue(vp->Slider_VUMeter_Bars, 6);
    SetChoiceValue(vp->Choice_VUMeter_Type, "Waveform");
    SetSliderValue(vp->Slider_VUMeter_Sensitivity, 70);
    SetChoiceValue(vp->Choice_VUMeter_Shape, "Circle");
    SetCheckBoxValue(vp->CheckBox_VUMeter_SlowDownFalls, true);
    SetSliderValue(vp->Slider_VUMeter_StartNote, 36);
    SetSliderValue(vp->Slider_VUMeter_EndNote, 84);
    SetSliderValue(vp->Slider_VUMeter_XOffset, 0);
    SetSliderValue(vp->Slider_VUMeter_YOffset, 0);
    vp->BitmapButton_VUMeter_YOffsetVC->SetActive(false);
}

void VUMeterEffect::RenameTimingTrack(std::string oldname, std::string newname, Effect* effect)
{
    wxString timing = effect->GetSettings().Get("E_CHOICE_VUMeter_TimingTrack", "");

    if (timing.ToStdString() == oldname)
    {
        effect->GetSettings()["E_CHOICE_VUMeter_TimingTrack"] = wxString(newname);
    }
}

void VUMeterEffect::Render(Effect *effect, const SettingsMap &SettingsMap, RenderBuffer &buffer) {
    float oset = buffer.GetEffectTimeIntervalPosition();
    Render(buffer,
        effect->GetParentEffectLayer()->GetParentElement()->GetSequenceElements(),
        SettingsMap.GetInt("SLIDER_VUMeter_Bars", 6),
        SettingsMap.Get("CHOICE_VUMeter_Type", "Waveform"),
        SettingsMap.Get("CHOICE_VUMeter_TimingTrack", ""),
        SettingsMap.GetInt("SLIDER_VUMeter_Sensitivity", 70),
        SettingsMap.Get("CHOICE_VUMeter_Shape", "Circle"),
        SettingsMap.GetBool("CHECKBOX_VUMeter_SlowDownFalls", TRUE),
        SettingsMap.GetInt("SLIDER_VUMeter_StartNote", 0),
        SettingsMap.GetInt("SLIDER_VUMeter_EndNote", 127),
        SettingsMap.GetInt("SLIDER_VUMeter_XOffset", 0),
        GetValueCurveInt("VUMeter_YOffset", 0, SettingsMap, oset)
    );
}

class VUMeterRenderCache : public EffectRenderCache 
{

public:
    VUMeterRenderCache() 
	{
	};
    virtual ~VUMeterRenderCache() {};
	std::list<int> _timingmarks; // collection of recent timing marks ... used for sweep
	int _lasttimingmark; // last time we saw a timing mark ... used for pulse
	std::list<float> _lastvalues;
	float _lastsize;
    int _colourindex;
};

int VUMeterEffect::DecodeType(std::string type)
{
	if (type == "Spectrogram")
	{
		return 1;
	}
	else if (type == "Volume Bars")
	{
		return 2;
	}
	else if (type == "Waveform")
	{
		return 3;
	}
	else if (type == "Timing Event Spike")
	{
		return 4;
	}
	else if (type == "Timing Event Sweep")
	{
		return 5;
	}
	else if (type == "On")
	{
		return 6;
	}
	else if (type == "Pulse")
	{
		return 7;
	}
	else if (type == "Intensity Wave")
	{
		return 8;
	}
	else if (type == "unused")
	{
		return 9;
	}
	else if (type == "Level Pulse")
	{
		return 10;
	}
	else if (type == "Level Shape")
	{
		return 11;
	}
    else if (type == "Color On")
    {
        return 12;
    }
    else if (type == "Timing Event Color")
    {
        return 13;
    }
    else if (type == "Note On")
    {
        return 14;
    }
    else if (type == "Note Level Pulse")
    {
        return 15;
    }
    else if (type == "Timing Event Jump")
    {
        return 16;
    }
    else if (type == "Timing Event Pulse")
    {
        return 17;
    }
    else if (type == "Timing Event Jump 100")
    {
        return 18;
    }

	// default type is volume bars
	return 2;
}

void VUMeterEffect::Render(RenderBuffer &buffer, SequenceElements *elements, int bars, const std::string& type, const std::string &timingtrack, int sensitivity, const std::string& shape, bool slowdownfalls, int startnote, int endnote, int xoffset, int yoffset)
{
	int nType = DecodeType(type);

	// Grab our cache
	VUMeterRenderCache *cache = (VUMeterRenderCache*)buffer.infoCache[id];
	if (cache == nullptr) {
		cache = new VUMeterRenderCache();
		buffer.infoCache[id] = cache;
	}
	std::list<int>& _timingmarks = cache->_timingmarks;
	int &_lasttimingmark = cache->_lasttimingmark;
	std::list<float>& _lastvalues = cache->_lastvalues;
	float& _lastsize = cache->_lastsize;
    int & _colourindex = cache->_colourindex;


	// Check for config changes which require us to reset
	if (buffer.needToInit)
	{
        buffer.needToInit = false;
        _colourindex = -1;
		_timingmarks.clear();
		_lasttimingmark = -1;
		_lastvalues.clear();
		_lastsize = 0;
        if (timingtrack != "")
        {
            elements->AddRenderDependency(timingtrack, buffer.cur_model);
        }
	}

	// We limit bars to the width of the model in some effects
	int usebars = bars;
	if (usebars > buffer.BufferWi)
	{
		usebars = buffer.BufferWi;
	}

	try
	{
		switch (nType)
		{
		case 1:
			RenderSpectrogramFrame(buffer, bars, _lastvalues, slowdownfalls, startnote, endnote, xoffset);
			break;
		case 2:
			RenderVolumeBarsFrame(buffer, usebars);
			break;
		case 3:
			RenderWaveformFrame(buffer, usebars, yoffset);
			break;
		case 4:
		case 5:
			RenderTimingEventFrame(buffer, usebars, nType, timingtrack, _timingmarks);
			break;
		case 6:
			RenderOnFrame(buffer);
			break;
		case 7:
			RenderPulseFrame(buffer, usebars, timingtrack, _lasttimingmark);
			break;
		case 8:
			RenderIntensityWaveFrame(buffer, usebars);
			break;
		case 10:
			RenderLevelPulseFrame(buffer, usebars, sensitivity, _lasttimingmark);
			break;
		case 11:
			RenderLevelShapeFrame(buffer, shape, _lastsize, sensitivity, slowdownfalls, xoffset, yoffset, usebars);
			break;
        case 12:
            RenderOnColourFrame(buffer);
            break;
        case 13:
            RenderTimingEventColourFrame(buffer, _colourindex, timingtrack);
            break;
        case 14:
            RenderNoteOnFrame(buffer, startnote, endnote);
            break;
        case 15:
            RenderNoteLevelPulseFrame(buffer, usebars, sensitivity, _lasttimingmark, startnote, endnote);
            break;
        case 16:
            RenderTimingEventJumpFrame(buffer, usebars, timingtrack, _lastsize, true);
            break;
        case 17:
            RenderTimingEventPulseFrame(buffer, usebars, timingtrack, _lastsize);
            break;
        case 18:
            RenderTimingEventJumpFrame(buffer, usebars, timingtrack, _lastsize, false);
            break;
        }
	}
	catch (...)
	{
		// This is here to let me catch any exceptions and stop the exception causing the render thread to die
		//int a = 0;
	}
}

void VUMeterEffect::RenderSpectrogramFrame(RenderBuffer &buffer, int usebars, std::list<float>& lastvalues, bool slowdownfalls, int startNote, int endNote, int xoffset)
{
    if (buffer.GetMedia() == nullptr) return;

    int truexoffset = xoffset * buffer.BufferWi / 100;
	std::list<float>* pdata = buffer.GetMedia()->GetFrameData(buffer.curPeriod, FRAMEDATA_VU, "");

	if (pdata != nullptr && pdata->size() != 0)
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
        int cols = 1;
        if (xoffset == 0)
        {
            cols = buffer.BufferWi / usebars;
        }
        if (cols < 1)
        {
            cols = 1;
        }
		std::list<float>::iterator it = lastvalues.begin();

        // skip to our start note
        for (int i = 0; i < startNote; i++)
        {
            ++it;
        }

		int x = truexoffset;

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

void VUMeterEffect::RenderVolumeBarsFrame(RenderBuffer &buffer, int usebars)
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
			std::list<float>* pf = buffer.GetMedia()->GetFrameData(start + i, FRAMEDATA_HIGH, "");
			if (pf != nullptr)
			{
				f = *pf->begin();
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

void VUMeterEffect::RenderWaveformFrame(RenderBuffer &buffer, int usebars, int yoffset)
{
    if (buffer.GetMedia() == nullptr) return;

    int trueyoffset = yoffset * buffer.BufferHt / 2 / 100;
	int start = buffer.curPeriod - usebars;
	int cols = buffer.BufferWi / usebars;
	int x = 0;
	for (int i = 0; i < usebars; i++)
	{
		if (start + i >= 0)
		{
			float fh = 0.0;
			std::list<float>* pf = buffer.GetMedia()->GetFrameData(start + i, FRAMEDATA_HIGH, "");
			if (pf != nullptr)
			{
				fh = *pf->begin();
			}
			float fl = 0.0;
			pf = buffer.GetMedia()->GetFrameData(start + i, FRAMEDATA_LOW, "");
			if (pf != nullptr)
			{
				fl = *pf->begin();
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

void VUMeterEffect::RenderTimingEventFrame(RenderBuffer &buffer, int usebars, int nType, std::string timingtrack, std::list<int> &timingmarks)
{
	if (timingtrack != "")
	{
		Element* t = nullptr;
		for (int i = 0; i < mSequenceElements->GetElementCount(); i++)
		{
			Element* e = mSequenceElements->GetElement(i);
			if (e->GetEffectLayerCount() == 1 && e->GetType() == ELEMENT_TYPE_TIMING && e->GetName() == timingtrack)
			{
				t = e;
				break;
			}
		}

		if (t != nullptr)
		{
			int start = buffer.curPeriod - usebars;
			int cols = buffer.BufferWi / usebars;
			int x = 0;
			for (int i = 0; i < usebars; i++)
			{
				if (start + i >= 0)
				{
					EffectLayer* el = t->GetEffectLayer(0);
					int ms = (start + i)*buffer.frameTimeInMs;
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
							for (int y = 0; y < buffer.BufferHt; y++)
							{
								xlColor color1;
								if (nType == 4)
								{
									buffer.GetMultiColorBlend((double)y / (double)buffer.BufferHt, false, color1);
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
									if (((start + i) > *it) && ((start + i) < *it + 10))
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
	}
}

void VUMeterEffect::RenderOnFrame(RenderBuffer& buffer)
{
    if (buffer.GetMedia() == nullptr) return;
   
    float f = 0.0;
	std::list<float>* pf = buffer.GetMedia()->GetFrameData(buffer.curPeriod, FRAMEDATA_HIGH, "");
	if (pf != nullptr)
	{
		f = *pf->begin();
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

void VUMeterEffect::RenderOnColourFrame(RenderBuffer& buffer)
{
    if (buffer.GetMedia() == nullptr) return;

    float f = 0.0;
    std::list<float>* pf = buffer.GetMedia()->GetFrameData(buffer.curPeriod, FRAMEDATA_HIGH, "");
    if (pf != nullptr)
    {
        f = *pf->begin();
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

void VUMeterEffect::RenderPulseFrame(RenderBuffer &buffer, int fadeframes, std::string timingtrack, int& lasttimingmark)
{
	if (timingtrack != "")
	{
		Element* t = nullptr;
		for (int i = 0; i < mSequenceElements->GetElementCount(); i++)
		{
			Element* e = mSequenceElements->GetElement(i);
			if (e->GetEffectLayerCount() == 1 && e->GetType() == ELEMENT_TYPE_TIMING
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
	}
}

void VUMeterEffect::RenderIntensityWaveFrame(RenderBuffer &buffer, int usebars)
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
			std::list<float>* pf = buffer.GetMedia()->GetFrameData(start + i, FRAMEDATA_HIGH, "");
			if (pf != nullptr)
			{
				f = *pf->begin();
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

void VUMeterEffect::RenderLevelPulseFrame(RenderBuffer &buffer, int fadeframes, int sensitivity, int& lasttimingmark)
{
    if (buffer.GetMedia() == nullptr) return;
    
    float f = 0.0;
	std::list<float>* pf = buffer.GetMedia()->GetFrameData(buffer.curPeriod, FRAMEDATA_HIGH, "");
	if (pf != nullptr)
	{
		f = *pf->begin();
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

void VUMeterEffect::RenderLevelShapeFrame(RenderBuffer& buffer, const std::string& shape, float& lastsize, int scale, bool slowdownfalls, int xoffset, int yoffset, int usebars)
{
    if (buffer.GetMedia() == nullptr) return;

    // star points
    if (usebars > 99) usebars = 99;
    int points = usebars / 25 + 4;

    int truexoffset = xoffset * buffer.BufferWi / 2 / 100;
    int trueyoffset = yoffset * buffer.BufferHt / 2 / 100;
    float scaling = (float)scale / 100.0 * 7.0;

	float f = 0.0;
	std::list<float>* pf = buffer.GetMedia()->GetFrameData(buffer.curPeriod, FRAMEDATA_HIGH, "");
	if (pf != nullptr)
	{
		f = *pf->begin();
	}

	if (shape == "Square")
	{
		int centerx = (buffer.BufferWi / 2.0) + truexoffset;
		int centery = (buffer.BufferHt / 2.0) + trueyoffset;
		float maxside = std::min(buffer.BufferHt / 2.0, buffer.BufferWi / 2.0) * scaling;
		float side = maxside * f;
		if (slowdownfalls)
		{
			if (side < lastsize)
			{
				lastsize = lastsize - std::min(maxside, (float)std::max(buffer.BufferHt / 2.0, buffer.BufferWi / 2.0)) / 20.0;
				if (lastsize < side)
				{
					lastsize = side;
				}
			}
			else
			{
				lastsize = side;
			}
		}
		else
		{
			lastsize = side;
		}
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
	else if (shape == "Filled Square")
	{
		int centerx = (buffer.BufferWi / 2.0) + truexoffset;
		int centery = (buffer.BufferHt / 2.0) + trueyoffset;
		float maxside = std::min(buffer.BufferHt / 2.0, buffer.BufferWi / 2.0) * scaling;
		float side = maxside * f;
		if (slowdownfalls)
		{
			if (side < lastsize)
			{
				lastsize = lastsize - std::min(maxside, (float)std::max(buffer.BufferHt / 2.0, buffer.BufferWi / 2.0)) / 20.0;
				if (lastsize < side)
				{
					lastsize = side;
				}
			}
			else
			{
				lastsize = side;
			}
		}
		else
		{
			lastsize = side;
		}
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
	else if (shape == "Circle")
	{
		float maxradius = std::min(buffer.BufferHt / 2.0, buffer.BufferWi / 2.0) * scaling;
		float radius = maxradius * f;
		if (slowdownfalls)
		{
			if (radius < lastsize)
			{
				lastsize = lastsize - std::min(maxradius, (float)std::max(buffer.BufferHt / 2.0, buffer.BufferWi / 2.0)) / 20.0;
				if (lastsize < radius)
				{
					lastsize = radius;
				}
			}
			else
			{
				lastsize = radius;
			}
		}
		else
		{
			lastsize = radius;
		}

		xlColor color1;
		buffer.palette.GetColor(0, color1);

		int centerx = (buffer.BufferWi / 2.0) + truexoffset;
		int centery = (buffer.BufferHt / 2.0) + trueyoffset;
		color1.alpha = 0.25 * 255;
		DrawCircle(buffer, centerx, centery, lastsize-2, color1);
		DrawCircle(buffer, centerx, centery, lastsize+2, color1);
		color1.alpha = 0.5 * 255;
		DrawCircle(buffer, centerx, centery, lastsize - 1, color1);
		DrawCircle(buffer, centerx, centery, lastsize + 1, color1);
		color1.alpha = 255;
		DrawCircle(buffer, centerx, centery, lastsize, color1);
	}
    else if (shape == "Star")
    {
        float maxradius = std::min(buffer.BufferHt / 2.0, buffer.BufferWi / 2.0) * scaling;
        float radius = maxradius * f;
        if (slowdownfalls)
        {
            if (radius < lastsize)
            {
                lastsize = lastsize - std::min(maxradius, (float)std::max(buffer.BufferHt / 2.0, buffer.BufferWi / 2.0)) / 20.0;
                if (lastsize < radius)
                {
                    lastsize = radius;
                }
            }
            else
            {
                lastsize = radius;
            }
        }
        else
        {
            lastsize = radius;
        }

        xlColor color1;
        buffer.palette.GetColor(0, color1);

        int centerx = (buffer.BufferWi / 2.0) + truexoffset;
        int centery = (buffer.BufferHt / 2.0) + trueyoffset;
        color1.alpha = 0.25 * 255;
        DrawStar(buffer, centerx, centery, lastsize - 2, color1, points);
        DrawStar(buffer, centerx, centery, lastsize + 2, color1, points);
        color1.alpha = 0.5 * 255;
        DrawStar(buffer, centerx, centery, lastsize - 1, color1, points);
        DrawStar(buffer, centerx, centery, lastsize + 1, color1, points);
        color1.alpha = 255;
        DrawStar(buffer, centerx, centery, lastsize, color1, points);
    }
    else if (shape == "Filled Star")
    {
        float maxradius = std::min(buffer.BufferHt / 2.0, buffer.BufferWi / 2.0) * scaling;
        float radius = maxradius * f;
        if (slowdownfalls)
        {
            if (radius < lastsize)
            {
                lastsize = lastsize - std::min(maxradius, (float)std::max(buffer.BufferHt / 2.0, buffer.BufferWi / 2.0)) / 20.0;
                if (lastsize < radius)
                {
                    lastsize = radius;
                }
            }
            else
            {
                lastsize = radius;
            }
        }
        else
        {
            lastsize = radius;
        }
        int centerx = (buffer.BufferWi / 2.0) + truexoffset;
        int centery = (buffer.BufferHt / 2.0) + trueyoffset;

        for (float x = 0; x <= lastsize; x+=0.5f)
        {
            float distance = x / lastsize;
            xlColor color1;
            buffer.GetMultiColorBlend(distance, false, color1);
            DrawStar(buffer, centerx, centery, x, color1, points);
        }
    }
    else if (shape == "Diamond")
    {
        int centerx = (buffer.BufferWi / 2.0) + truexoffset;
        int centery = (buffer.BufferHt / 2.0) + trueyoffset;
        float maxside = std::min(buffer.BufferHt / 2.0, buffer.BufferWi / 2.0) * scaling;
        float side = maxside * f;
        if (slowdownfalls)
        {
            if (side < lastsize)
            {
                lastsize = lastsize - std::min(maxside, (float)std::max(buffer.BufferHt / 2.0, buffer.BufferWi / 2.0)) / 20.0;
                if (lastsize < side)
                {
                    lastsize = side;
                }
            }
            else
            {
                lastsize = side;
            }
        }
        else
        {
            lastsize = side;
        }
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
    else if (shape == "Filled Circle")
	{
		float maxradius = std::min(buffer.BufferHt / 2.0, buffer.BufferWi / 2.0) * scaling;
		float radius = maxradius * f;
		if (slowdownfalls)
		{
			if (radius < lastsize)
			{
				lastsize = lastsize - std::min(maxradius, (float)std::max(buffer.BufferHt / 2.0, buffer.BufferWi / 2.0)) / 20.0;
				if (lastsize < radius)
				{
					lastsize = radius;
				}
			}
			else
			{
				lastsize = radius;
			}
		}
		else
		{
			lastsize = radius;
		}
		int centerx = (buffer.BufferWi / 2.0) + truexoffset;
		int centery = (buffer.BufferHt / 2.0) + trueyoffset;

		for (int x = 0; x <= lastsize; x++)
		{
			float distance = (float)x / lastsize;
			xlColor color1;
			buffer.GetMultiColorBlend(distance, false, color1);
			DrawCircle(buffer, centerx, centery, x, color1);
		}
	}
	else if (shape == "Diamond")
	{
		int centerx = (buffer.BufferWi / 2.0) + truexoffset;
		int centery = (buffer.BufferHt / 2.0) + trueyoffset;
		float maxside = std::min(buffer.BufferHt / 2.0, buffer.BufferWi / 2.0) * scaling;
		float side = maxside * f;
		if (slowdownfalls)
		{
			if (side < lastsize)
			{
				lastsize = lastsize - std::min(maxside, (float)std::max(buffer.BufferHt / 2.0, buffer.BufferWi / 2.0)) / 20.0;
				if (lastsize < side)
				{
					lastsize = side;
				}
			}
			else
			{
				lastsize = side;
			}
		}
		else
		{
			lastsize = side;
		}
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
    else if (shape == "Filled Circle")
	{
		float maxradius = std::min(buffer.BufferHt / 2.0, buffer.BufferWi / 2.0) * scaling;
		float radius = maxradius * f;
		if (slowdownfalls)
		{
			if (radius < lastsize)
			{
				lastsize = lastsize - std::min(maxradius, (float)std::max(buffer.BufferHt / 2.0, buffer.BufferWi / 2.0)) / 20.0;
				if (lastsize < radius)
				{
					lastsize = radius;
				}
			}
			else
			{
				lastsize = radius;
			}
		}
		else
		{
			lastsize = radius;
		}
		int centerx = (buffer.BufferWi / 2.0) + truexoffset;
		int centery = (buffer.BufferHt / 2.0) + trueyoffset;

		for (int x = 0; x <= lastsize; x++)
		{
			float distance = (float)x / lastsize;
			xlColor color1;
			buffer.GetMultiColorBlend(distance, false, color1);
			DrawCircle(buffer, centerx, centery, x, color1);
		}
	}
	else if (shape == "Diamond")
	{
		int centerx = (buffer.BufferWi / 2.0) + truexoffset;
		int centery = (buffer.BufferHt / 2.0) + trueyoffset;
		float maxside = std::min(buffer.BufferHt / 2.0, buffer.BufferWi / 2.0) * scaling;
		float side = maxside * f;
		if (slowdownfalls)
		{
			if (side < lastsize)
			{
				lastsize = lastsize - std::min(maxside, (float)std::max(buffer.BufferHt / 2.0, buffer.BufferWi / 2.0)) / 20.0;
				if (lastsize < side)
				{
					lastsize = side;
				}
			}
			else
			{
				lastsize = side;
			}
		}
		else
		{
			lastsize = side;
		}
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
	else if (shape == "Filled Diamond")
	{
		int centerx = (buffer.BufferWi / 2.0) + truexoffset;
		int centery = (buffer.BufferHt / 2.0) + trueyoffset;
		float maxside = std::min(buffer.BufferHt / 2.0, buffer.BufferWi / 2.0) * scaling;
		float side = maxside * f;
		if (slowdownfalls)
		{
			if (side < lastsize)
			{
				lastsize = lastsize - std::min(maxside, (float)std::max(buffer.BufferHt / 2.0, buffer.BufferWi / 2.0)) / 20.0;
				if (lastsize < side)
				{
					lastsize = side;
				}
			}
			else
			{
				lastsize = side;
			}
		}
		else
		{
			lastsize = side;
		}
		for (int xx = 0; xx <= lastsize; xx++)
		{
			xlColor color1;
			buffer.GetMultiColorBlend(xx / lastsize, false, color1);
			DrawDiamond(buffer, centerx, centery, xx, color1);
		}
	}
}

void VUMeterEffect::RenderTimingEventJumpFrame(RenderBuffer &buffer, int fallframes, std::string timingtrack, float& lastsize, bool useAudioLevel)
{
    if (useAudioLevel && buffer.GetMedia() == nullptr) return;

    if (timingtrack != "")
    {
        Element* t = nullptr;
        for (int i = 0; i < mSequenceElements->GetElementCount(); i++)
        {
            Element* e = mSequenceElements->GetElement(i);
            if (e->GetEffectLayerCount() == 1 && e->GetType() == ELEMENT_TYPE_TIMING
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
                    std::list<float>* pf = buffer.GetMedia()->GetFrameData(buffer.curPeriod, FRAMEDATA_HIGH, "");
                    if (pf != nullptr)
                    {
                        f = *pf->begin();
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
            if (e->GetEffectLayerCount() == 1 && e->GetType() == ELEMENT_TYPE_TIMING
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

void VUMeterEffect::RenderTimingEventColourFrame(RenderBuffer &buffer, int& colourindex, std::string timingtrack)
{
    if (timingtrack != "")
    {
        Element* t = nullptr;
        for (int i = 0; i < mSequenceElements->GetElementCount(); i++)
        {
            Element* e = mSequenceElements->GetElement(i);
            if (e->GetEffectLayerCount() == 1 && e->GetType() == ELEMENT_TYPE_TIMING
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

void VUMeterEffect::RenderNoteOnFrame(RenderBuffer& buffer, int startNote, int endNote)
{
    if (buffer.GetMedia() == nullptr) return;

    std::list<float>* pdata = buffer.GetMedia()->GetFrameData(buffer.curPeriod, FRAMEDATA_VU, "");

    if (pdata != nullptr && pdata->size() != 0)
    {
        int i = 0;
        float level = 0.0;
        for (auto it = pdata->begin(); it != pdata->end(); it++)
        {
            if (i > startNote && i <= endNote)
            {
                level = std::max(*it, level);
            }
            i++;
        }

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

void VUMeterEffect::RenderNoteLevelPulseFrame(RenderBuffer& buffer, int fadeframes, int sensitivity, int& lasttimingmark, int startNote, int endNote)
{
    if (buffer.GetMedia() == nullptr) return;

    std::list<float>* pdata = buffer.GetMedia()->GetFrameData(buffer.curPeriod, FRAMEDATA_VU, "");

    if (pdata != nullptr && pdata->size() != 0)
    {
        int i = 0;
        float level = 0.0;
        for (auto it = pdata->begin(); it != pdata->end(); it++)
        {
            if (i > startNote && i <= endNote)
            {
                level = std::max(*it, level);
            }
            i++;
        }

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
