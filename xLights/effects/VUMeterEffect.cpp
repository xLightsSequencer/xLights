#include "VUMeterEffect.h"
#include "VUMeterPanel.h"
#include "../AudioManager.h"

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

void VUMeterEffect::Render(Effect *effect, const SettingsMap &SettingsMap, RenderBuffer &buffer) {
    Render(buffer,
           SettingsMap.GetInt("TEXTCTRL_VUMeter_Bars", 5)
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
};

void VUMeterEffect::Render(RenderBuffer &buffer, int bars)
{
    buffer.drawingContext->Clear();

	VUMeterRenderCache *cache = (VUMeterRenderCache*)buffer.infoCache[id];
    if (cache == nullptr) {
        cache = new VUMeterRenderCache();
        buffer.infoCache[id] = cache;
    }

	int &_bars = cache->_bars;
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

    if (_bars != bars)
    {
        _bars = bars;
    }

	std::list<float>* pdata = buffer.GetMedia()->GetFrameData(buffer.curPeriod, FRAMEDATA_VU, "");

	int per = pdata->size() / _bars;
	int cols = buffer.BufferWi / _bars;
	
	std::list<float>::iterator it = pdata->begin();
	int x = 0;

	for (int j = 0; j < _bars; j++)
	{
		for (int k = 0; k < cols; k++)
		{
			for (int y = 0; y < buffer.BufferHt; y++)
			{
				if (y < buffer.BufferHt * (*it))
				{
					buffer.SetPixel(x, y, color1);
				}
			}
			x++;
		}
		++it;
	}
}
