#include "VUMeterEffect.h"
#include "VUMeterPanel.h"
#include "../AudioManager.h"
#include "../sequencer/SequenceElements.h"

#include "../sequencer/Effect.h"
#include "../RenderBuffer.h"
#include "../UtilClasses.h"

#include "../../include/vumeter-16.xpm"
#include "../../include/vumeter-24.xpm"
#include "../../include/vumeter-32.xpm"
#include "../../include/vumeter-48.xpm"
#include "../../include/vumeter-64.xpm"

#define wrdebug(...)

VUMeterEffect::VUMeterEffect(int id) : RenderableEffect(id, "VU Meter", vumeter_16, vumeter_24, vumeter_32, vumeter_48, vumeter_64)
{
}

VUMeterEffect::~VUMeterEffect()
{
}

bool VUMeterEffect::CanRenderOnBackgroundThread() {
    return true;
}

wxPanel *VUMeterEffect::CreatePanel(wxWindow *parent) {
	return new VUMeterPanel(parent);
}

void VUMeterEffect::SetDefaultParameters(Model *cls) 
{
	VUMeterPanel *fp = (VUMeterPanel*)panel;
	if (fp == nullptr) 
	{
		return;
	}

	fp->Choice_VUMeter_TimingTrack->Clear();
	if (mSequenceElements == nullptr) 
	{
		return;
	}

	// Load the names of the timing tracks
	for (int i = 0; i < mSequenceElements->GetElementCount(); i++)
	{
		Element* e = mSequenceElements->GetElement(i);
		if (e->GetEffectLayerCount() == 1 && e->GetType() == "timing") 
		{
			fp->Choice_VUMeter_TimingTrack->Append(e->GetName());
		}
	}

	// Select the first one
	if (fp->Choice_VUMeter_TimingTrack->GetCount() > 0)
	{
		fp->Choice_VUMeter_TimingTrack->Select(0);
	}

	// Validate the window (includes enabling and disabling controls)
	fp->ValidateWindow();
}

void VUMeterEffect::Render(Effect *effect, const SettingsMap &SettingsMap, RenderBuffer &buffer) {
    Render(buffer,
           SettingsMap.GetInt("TEXTCTRL_VUMeter_Bars", 6),
		   SettingsMap.Get("CHOICE_VUMeter_Type", "Waveform"),
		   SettingsMap.Get("CHOICE_VUMeter_TimingTrack", ""),
		   SettingsMap.GetInt("TEXTCTRL_VUMeter_Sensitivity", 70)
		);
}

class VUMeterRenderCache : public EffectRenderCache 
{

public:
    VUMeterRenderCache() 
	{
	};
    virtual ~VUMeterRenderCache() {};
	int _bars;
	int _type;
	std::string _timingtrack;
	std::list<int> _timingmarks; // collection of recent timing marks ... used for sweep
	int _lasttimingmark; // last time we saw a timing mark ... used for pulse
	std::list<float> _lastvalues;
	int _sensitivity;
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
	else if (type == "Spectrogram with Gravity")
	{
		return 9;
	}
	else if (type == "Level Pulse")
	{
		return 10;
	}

	// default type is volume bars
	return 2;
}

void VUMeterEffect::Render(RenderBuffer &buffer, int bars, const std::string& type, const std::string &timingtrack, int sensitivity)
{
	buffer.drawingContext->Clear();

	// no point if we have no media
	if (buffer.GetMedia() == NULL)
	{
		return;
	}

	int nType = DecodeType(type);

	// Grab our cache
	VUMeterRenderCache *cache = (VUMeterRenderCache*)buffer.infoCache[id];
	if (cache == nullptr) {
		cache = new VUMeterRenderCache();
		buffer.infoCache[id] = cache;
	}
	int &_bars = cache->_bars;
	int &_type = cache->_type;
	std::string& _timingtrack = cache->_timingtrack;
	std::list<int>& _timingmarks = cache->_timingmarks;
	int &_lasttimingmark = cache->_lasttimingmark;
	std::list<float>& _lastvalues = cache->_lastvalues;
	int& _sensitivity = cache->_sensitivity;

	// Check for config changes which require us to reset
	if (_bars != bars || _type != nType || _timingtrack != timingtrack || _sensitivity != sensitivity)
	{
		_bars = bars;
		_type = nType;
		_timingtrack = timingtrack;
		_timingmarks.clear();
		_lasttimingmark = -1;
		_lastvalues.clear();
		_sensitivity = sensitivity;
	}

	// We limit bars to the width of the model
	int usebars = _bars;
	if (usebars > buffer.BufferWi)
	{
		usebars = buffer.BufferWi;
	}

	try
	{
		switch (_type)
		{
		case 1:
			RenderSpectrogramFrame(buffer, usebars, _lastvalues, false);
			break;
		case 2:
			RenderVolumeBarsFrame(buffer, usebars);
			break;
		case 3:
			RenderWaveformFrame(buffer, usebars);
			break;
		case 4:
		case 5:
			RenderTimingEventFrame(buffer, usebars, nType, _timingtrack, _timingmarks);
			break;
		case 6:
			RenderOnFrame(buffer);
			break;
		case 7:
			RenderPulseFrame(buffer, usebars, _timingtrack, _lasttimingmark);
			break;
		case 8:
			RenderIntensityWaveFrame(buffer, usebars);
			break;
		case 9:
			RenderSpectrogramFrame(buffer, usebars, _lastvalues, true);
			break;
		case 10:
			RenderLevelPulseFrame(buffer, usebars, _sensitivity, _lasttimingmark);
			break;
		}
	}
	catch (...)
	{
		// This is here to let me catch any exceptions and stop the exception causing the render thread to die
		int a = 0;
	}
}

void VUMeterEffect::RenderSpectrogramFrame(RenderBuffer &buffer, int usebars, std::list<float>& lastvalues, bool gravity)
{
	std::list<float>* pdata = buffer.GetMedia()->GetFrameData(buffer.curPeriod, FRAMEDATA_VU, "");

	if (pdata != NULL && pdata->size() != 0)
	{
		if (gravity)
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

		if (usebars > pdata->size())
		{
			usebars = pdata->size();
		}

		int per = pdata->size() / usebars;
		int cols = buffer.BufferWi / usebars;

		std::list<float>::iterator it = lastvalues.begin();
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
			}
			for (int k = 0; k < cols; k++)
			{
				for (int y = 0; y < buffer.BufferHt; y++)
				{
					if (y < buffer.BufferHt * f)
					{
						xlColor color1;
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
	int start = buffer.curPeriod - usebars;
	int cols = buffer.BufferWi / usebars;
	int x = 0;
	for (int i = 0; i < usebars; i++)
	{
		if (start + i >= 0)
		{
			float f = 0.0;
			std::list<float>* pf = buffer.GetMedia()->GetFrameData(start + i, FRAMEDATA_HIGH, "");
			if (pf != NULL)
			{
				f = *pf->begin();
			}
			for (int j = 0; j < cols; j++)
			{
				for (int y = 0; y < buffer.BufferHt * f; y++)
				{
					xlColor color1;
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

void VUMeterEffect::RenderWaveformFrame(RenderBuffer &buffer, int usebars)
{
	int start = buffer.curPeriod - usebars;
	int cols = buffer.BufferWi / usebars;
	int x = 0;
	for (int i = 0; i < usebars; i++)
	{
		if (start + i >= 0)
		{
			float fh = 0.0;
			std::list<float>* pf = buffer.GetMedia()->GetFrameData(start + i, FRAMEDATA_HIGH, "");
			if (pf != NULL)
			{
				fh = *pf->begin();
			}
			float fl = 0.0;
			pf = buffer.GetMedia()->GetFrameData(start + i, FRAMEDATA_LOW, "");
			if (pf != NULL)
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

void VUMeterEffect::RenderTimingEventFrame(RenderBuffer &buffer, int usebars, int nType, std::string timingtrack, std::list<int> &timingmarks)
{
	if (timingtrack != "")
	{
		Element* t = NULL;
		for (int i = 0; i < mSequenceElements->GetElementCount(); i++)
		{
			Element* e = mSequenceElements->GetElement(i);
			if (e->GetEffectLayerCount() == 1 && e->GetType() == "timing" && e->GetName() == timingtrack)
			{
				t = e;
				break;
			}
		}

		if (t != NULL)
		{
			int start = buffer.curPeriod - usebars;
			int cols = buffer.BufferWi / usebars;
			int x = 0;
			for (int i = 0; i < usebars; i++)
			{
				if (start + i >= 0)
				{
					EffectLayer* el = t->GetEffectLayer(0);
					int ms = (start + i)*buffer.GetMedia()->GetFrameInterval();
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
	float f = 0.0;
	std::list<float>* pf = buffer.GetMedia()->GetFrameData(buffer.curPeriod, FRAMEDATA_HIGH, "");
	if (pf != NULL)
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

void VUMeterEffect::RenderPulseFrame(RenderBuffer &buffer, int fadeframes, std::string timingtrack, int& lasttimingmark)
{
	if (timingtrack != "")
	{
		Element* t = NULL;
		for (int i = 0; i < mSequenceElements->GetElementCount(); i++)
		{
			Element* e = mSequenceElements->GetElement(i);
			if (e->GetEffectLayerCount() == 1 && e->GetType() == "timing" && e->GetName() == timingtrack)
			{
				t = e;
				break;
			}
		}

		if (t != NULL)
		{
			EffectLayer* el = t->GetEffectLayer(0);
			int ms = buffer.curPeriod*buffer.GetMedia()->GetFrameInterval();
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
	int start = buffer.curPeriod - usebars;
	int cols = buffer.BufferWi / usebars;
	int x = 0;
	for (int i = 0; i < usebars; i++)
	{
		if (start + i >= 0)
		{
			float f = 0.0;
			std::list<float>* pf = buffer.GetMedia()->GetFrameData(start + i, FRAMEDATA_HIGH, "");
			if (pf != NULL)
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
	float f = 0.0;
	std::list<float>* pf = buffer.GetMedia()->GetFrameData(buffer.curPeriod, FRAMEDATA_HIGH, "");
	if (pf != NULL)
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

