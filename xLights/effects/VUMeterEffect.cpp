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
	fp->ValidateWindow();
}

void VUMeterEffect::Render(Effect *effect, const SettingsMap &SettingsMap, RenderBuffer &buffer) {
    Render(buffer,
           SettingsMap.GetInt("TEXTCTRL_VUMeter_Bars", 6),
	   	   SettingsMap.GetInt("TEXTCTRL_VUMeter_Type", 1),
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
};

void VUMeterEffect::Render(RenderBuffer &buffer, int bars, int type, const std::string &timingtrack)
{
	buffer.drawingContext->Clear();

	VUMeterRenderCache *cache = (VUMeterRenderCache*)buffer.infoCache[id];
	if (cache == nullptr) {
		cache = new VUMeterRenderCache();
		buffer.infoCache[id] = cache;
	}

	int &_bars = cache->_bars;
	int &_type = cache->_type;
	std::string& _timingtrack = cache->_timingtrack;
	std::list<int>& _timingmarks = cache->_timingmarks;
	xlColor color1;
	xlColor color2;
	xlColor color3;
	xlColor color4;
	xlColor color5;
	buffer.palette.GetColor(0, color1);
	buffer.palette.GetColor(1, color2);
	buffer.palette.GetColor(2, color3);
	buffer.palette.GetColor(3, color4);
	buffer.palette.GetColor(4, color5);

	if (_bars != bars || _type != type || _timingtrack != timingtrack)
	{
		_bars = bars;
		_type = type;
		_timingtrack = timingtrack;
		_timingmarks.clear();
	}

	if (buffer.GetMedia() != NULL)
	{
		switch (_type)
		{
		case 1:
		{
			std::list<float>* pdata = buffer.GetMedia()->GetFrameData(buffer.curPeriod, FRAMEDATA_VU, "");

			if (pdata->size() != 0)
			{
				if (_bars > pdata->size())
				{
					_bars = pdata->size();
				}

				int per = pdata->size() / _bars;
				int cols = buffer.BufferWi / _bars;

				std::list<float>::iterator it = pdata->begin();
				int x = 0;

				for (int j = 0; j < _bars; j++)
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
			int start = buffer.curPeriod - _bars;
			int cols = buffer.BufferWi / _bars;
			int x = 0;
			for (int i = 0; i < _bars; i++)
			{
				if (start + i >= 0)
				{
					float f = *buffer.GetMedia()->GetFrameData(start + i, FRAMEDATA_HIGH, "")->begin();
					for (int j = 0; j < cols; j++)
					{
						for (int y = 0; y <= buffer.BufferHt * f; y++)
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
		break;
		case 3:
		{
			int start = buffer.curPeriod - _bars;
			int cols = buffer.BufferWi / _bars;
			int x = 0;
			for (int i = 0; i < _bars; i++)
			{
				if (start + i >= 0)
				{
					float fh = *buffer.GetMedia()->GetFrameData(start + i, FRAMEDATA_HIGH, "")->begin();
					float fl = *buffer.GetMedia()->GetFrameData(start + i, FRAMEDATA_LOW, "")->begin();
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
					int start = buffer.curPeriod - _bars;
					int cols = buffer.BufferWi / _bars;
					int x = 0;
					for (int i = 0; i < _bars; i++)
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
														color1.alpha = yt * 255;
														buffer.SetPixel(x, y, color1);
														color1.alpha = 255;
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
		}
	}
}
