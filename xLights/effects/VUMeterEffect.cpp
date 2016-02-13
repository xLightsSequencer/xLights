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

void VUMeterEffect::SetDefaultParameters(Model *cls) {
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
	for (int i = 0; i < mSequenceElements->GetElementCount(); i++) 
	{
		Element* e = mSequenceElements->GetElement(i);
		if (e->GetEffectLayerCount() == 1 && e->GetType() == "timing") 
		{
			fp->Choice_VUMeter_TimingTrack->Append(e->GetName());
		}
	}
	if (fp->Choice_VUMeter_TimingTrack->GetCount() > 0)
	{
		fp->Choice_VUMeter_TimingTrack->Select(0);
	}
	fp->ValidateWindow();
}

void VUMeterEffect::Render(Effect *effect, const SettingsMap &SettingsMap, RenderBuffer &buffer) {
    Render(buffer,
           SettingsMap.GetInt("TEXTCTRL_VUMeter_Bars", 6),
		   SettingsMap.Get("CHOICE_VUMeter_Type", "Waveform"),
		   SettingsMap.Get("CHOICE_VUMeter_TimingTrack", "")
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
	std::list<int> _timingmarks;
	int _lasttimingmark;
};

void VUMeterEffect::Render(RenderBuffer &buffer, int bars, const std::string& type, const std::string &timingtrack)
{
	buffer.drawingContext->Clear();

	// no point if we have no media
	if (buffer.GetMedia() == NULL)
	{
		return;
	}

	int nType = 2;
	if (type == "Chromagram")
	{
		nType = 1;
	}
	else if (type == "Volume Bars")
	{
		nType = 2;
	}
	else if (type == "Waveform")
	{
		nType = 3;
	}
	else if (type == "Timing Event Spike")
	{
		nType = 4;
	}
	else if (type == "Timing Event Sweep")
	{
		nType = 5;
	}
	else if (type == "On")
	{
		nType = 6;
	}
	else if (type == "Pulse")
	{
		nType = 7;
	}

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

	if (_bars != bars || _type != nType || _timingtrack != timingtrack)
	{
		_bars = bars;
		_type = nType;
		_timingtrack = timingtrack;
		_timingmarks.clear();
		_lasttimingmark = -1;
	}

	int usebars = _bars;
	if (usebars > buffer.BufferWi)
	{
		usebars = buffer.BufferWi;
	}

	if (buffer.GetMedia() != NULL)
	{
		switch (_type)
		{
		case 1:
		{
			std::list<float>* pdata = buffer.GetMedia()->GetFrameData(buffer.curPeriod, FRAMEDATA_VU, "");

			if (pdata != NULL && pdata->size() != 0)
			{
				if (usebars > pdata->size())
				{
					usebars = pdata->size();
				}

				int per = pdata->size() / usebars;
				int cols = buffer.BufferWi / usebars;

				std::list<float>::iterator it = pdata->begin();
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
						for (int y = 0; y <= buffer.BufferHt; y++)
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
		break;
		case 2:
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
						for (int y = 0; y <= buffer.BufferHt * f; y++)
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
		break;
		case 3:
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
					for (int j = 0; j < cols; j++)
					{
						for (int y = s; y <= e; y++)
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
		break;
		case 4:
		case 5:
		{
			if (_timingtrack != "")
			{
				Element* t = NULL;
				for (int i = 0; i < mSequenceElements->GetElementCount(); i++)
				{
					Element* e = mSequenceElements->GetElement(i);
					if (e->GetEffectLayerCount() == 1 && e->GetType() == "timing" && e->GetName() == _timingtrack)
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
								_timingmarks.remove(start+i);
								_timingmarks.push_back(start+i);
								for (int j = 0; j < cols; j++)
								{
									for (int y = 0; y < buffer.BufferHt; y++)
									{
										xlColor color1;
										if (_type == 4)
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
								if (_type == 5)
								{
									// remove any no longer required
									while (_timingmarks.size() != 0 && *_timingmarks.begin() < start - 10)
									{
										_timingmarks.pop_front();
									}

									if (_timingmarks.size() > 0)
									{
										int left = cols;

										for (std::list<int>::iterator it = _timingmarks.begin(); it != _timingmarks.end(); ++it)
										{
											if (((start + i) > *it) && ((start + i) < *it + 10))
											{
												float yt = (10 - (start + i - *it)) / 10.0;
												if (yt < 0)
												{
													yt = 0;
												}
												for (int j = 0; j < cols; j++)
												{
													for (int y = 0; y < buffer.BufferHt; y++)
													{
														xlColor color1;
														buffer.GetMultiColorBlend(1.0-yt, false, color1);
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
		break;
		// On
		case 6:
		{
			float f = 0.0;
			std::list<float>* pf = buffer.GetMedia()->GetFrameData(buffer.curPeriod, FRAMEDATA_HIGH, "");
			if (pf != NULL)
			{
				f = *pf->begin();
			}
			xlColor color1;
			buffer.palette.GetColor(0, color1);
			color1.alpha = f * 255;

			for (int x = 0; x < buffer.BufferWi; x++)
			{
				for (int y = 0; y < buffer.BufferHt; y++)
				{
					buffer.SetPixel(x, y, color1);
				}
			}
		}
		break;
		// Pulse
		case 7:
		{
			if (_timingtrack != "")
			{
				Element* t = NULL;
				for (int i = 0; i < mSequenceElements->GetElementCount(); i++)
				{
					Element* e = mSequenceElements->GetElement(i);
					if (e->GetEffectLayerCount() == 1 && e->GetType() == "timing" && e->GetName() == _timingtrack)
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
						_lasttimingmark = buffer.curPeriod;
					}

					float f = 0.0;
					
					if (_lasttimingmark >= 0)
					{
						f = 1.0 - (((float)buffer.curPeriod - (float)_lasttimingmark) / (float)usebars);
						if (f < 0)
						{
							f = 0;
						}
					}

					if (f > 0.0)
					{
						xlColor color1;
						buffer.palette.GetColor(0, color1);
						color1.alpha = f * 255;

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
		break;
		}
	}
}
