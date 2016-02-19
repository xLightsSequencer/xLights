#include "TransitionEffect.h"
#include "TransitionPanel.h"

#include "../sequencer/Effect.h"
#include "../RenderBuffer.h"
#include "../UtilClasses.h"

#include "../../include/transition-16.xpm"
#include "../../include/transition-24.xpm"
#include "../../include/transition-32.xpm"
#include "../../include/transition-48.xpm"
#include "../../include/transition-64.xpm"

#include <algorithm>

#ifdef __WXMSW__
//#include "wx/msw/debughlp.h"
//wxString s;
//s.Printf("%f -> %f", val, db);
//wxDbgHelpDLL::LogError(s);
#endif

//#define wrdebug(...)

TransitionEffect::TransitionEffect(int id) : RenderableEffect(id, "Transition", transition_16, transition_24, transition_32, transition_48, transition_64)
{
}

TransitionEffect::~TransitionEffect()
{
}

bool TransitionEffect::CanRenderOnBackgroundThread() {
    return true;
}

wxPanel *TransitionEffect::CreatePanel(wxWindow *parent) {
	return new TransitionPanel(parent);
}

void TransitionEffect::Render(Effect *effect, const SettingsMap &SettingsMap, RenderBuffer &buffer) {
	Render(buffer,
		   SettingsMap.GetBool("CHECKBOX_Transition_Mode", TRUE),
		   SettingsMap.Get("CHOICE_Transition_Type", "Wipe"),
		   SettingsMap.GetInt("TEXTCTRL_Transition_Adjust", 50),
		   SettingsMap.GetBool("CHECKBOX_Transition_Reverse", FALSE)
		);
}

class TransitionRenderCache : public EffectRenderCache 
{

public:
    TransitionRenderCache() 
	{
	};
    virtual ~TransitionRenderCache() {};
	bool _mode;
	int _type;
	int _adjust;
	float _lastvalue;
	bool _reverse;
};

int TransitionEffect::DecodeType(std::string type)
{
	if (type == "Wipe")
	{
		return 1;
	}
	else if (type == "Clockwise")
	{
		return 2;
	}
	else if (type == "From Middle")
	{
		return 3;
	}
	else if (type == "Square Explode")
	{
		return 4;
	}
	else if (type == "CircleExplode")
	{
		return 5;
	}
	else if (type == "Blinds")
	{
		return 6;
	}
	else if (type == "Blend")
	{
		return 7;
	}
	else if (type == "Slide Checks")
	{
		return 8;
	}
	else if (type == "Slide Bars")
	{
		return 9;
	}

	// default type is volume bars
	return 1;
}

void TransitionEffect::Render(RenderBuffer &buffer, bool mode, const std::string& type, int adjust, bool reverse)
{
	buffer.drawingContext->Clear();

	int nType = DecodeType(type);

	// Grab our cache
	TransitionRenderCache *cache = (TransitionRenderCache*)buffer.infoCache[id];
	if (cache == nullptr) {
		cache = new TransitionRenderCache();
		buffer.infoCache[id] = cache;
	}
	bool &_mode = cache->_mode;
	int &_type = cache->_type;
	int &_adjust = cache->_adjust;
	bool& _reverse = cache->_reverse;
	float& _lastvalue = cache->_lastvalue;

	// Check for config changes which require us to reset
	if (_mode != mode || _type != nType || _adjust != adjust || _reverse != reverse)
	{
		_mode = mode;
		_type = nType;
		_adjust = adjust;
		_reverse = reverse;
		_lastvalue = -99999;
	}

	try
	{
		switch (_type)
		{
		case 1:
			RenderWipe(buffer, _mode, _lastvalue, _adjust);
			break;
		case 2:
			RenderClockWise(buffer, _mode, _lastvalue, _adjust, _reverse);
			break;
		case 3:
		RenderFromMiddle(buffer, _mode, _lastvalue, _adjust, _reverse);
			break;
		case 4:
		RenderSquareExplode(buffer, _mode, _lastvalue, _adjust, _reverse);
		break;
		case 5:
		RenderCircleExplode(buffer, _mode, _lastvalue, _adjust, _reverse);
			break;
		case 6:
		RenderBlinds(buffer, _mode, _lastvalue, _adjust, _reverse);
			break;
		case 7:
		RenderBlend(buffer, _mode, _lastvalue, _adjust, _reverse);
			break;
		case 8:
		RenderSlideChecks(buffer, _mode, _lastvalue, _adjust, _reverse);
			break;
		case 9:
		RenderSlideBars(buffer, _mode, _lastvalue, _adjust, _reverse);
			break;
		}
	}
	catch (...)
	{
		// This is here to let me catch any exceptions and stop the exception causing the render thread to die
		int a = 0;
	}
}

const double PI = 3.141592653589793238463;

bool TransitionEffect::isLeft(wxPoint aa, float slope, wxPoint test) {
	wxPoint a(0, 0);
	a = wxPoint(aa.x, 0);
	wxPoint b(aa.x + aa.y / slope, aa.y);

	return ((b.x - a.x)*(test.y - a.y) - (b.y - a.y)*(test.x - a.x)) > 0;
}

void TransitionEffect::RenderWipe(RenderBuffer &buffer, bool mode, float& lastvalue, int adjust)
{
	float angle = PI * (float)adjust / 100.0;

	float slope = tan(angle);
	int extrax = abs(buffer.BufferHt / slope);
	if (lastvalue == -99999)
	{
		if (slope < 0)
		{
			lastvalue = 0;
		}
		else
		{
			lastvalue = -1 * extrax;
		}
	}

	xlColor c1;
	xlColor c2;
	if (mode)
	{
		buffer.palette.GetColor(0, c1);
		buffer.palette.GetColor(1, c2);
	}
	else
	{
		buffer.palette.GetColor(1, c1);
		buffer.palette.GetColor(0, c2);
	}

	if (adjust == 0 || adjust == 100)
	{
		float step = (float)buffer.BufferHt / (float)(buffer.curEffEndPer - buffer.curEffStartPer);

		lastvalue = lastvalue + step;
		if (lastvalue >= buffer.BufferHt)
		{
			lastvalue = buffer.BufferHt - 1;
		}

		for (int y = 0; y < (int)lastvalue; y++)
		{
			for (int x = 0; x < buffer.BufferWi; x++)
			{
				buffer.SetPixel(x, y, c1);
			}
		}
		for (int y = (int)lastvalue; y < buffer.BufferHt; y++)
		{
			for (int x = 0; x < buffer.BufferWi; x++)
			{
				buffer.SetPixel(x, y, c2);
			}
		}
	}
	else if (adjust < 50)
	{
		float step = ((float)(buffer.BufferWi + extrax)) / (float)(buffer.curEffEndPer - buffer.curEffStartPer);

		lastvalue = lastvalue + step;
		if (lastvalue >= buffer.BufferWi + extrax)
		{
			lastvalue = buffer.BufferWi + extrax - 1;
		}

		// start top left 0, BufferHt
		// y = slope * x + y'
		
		for (int x = 0; x < buffer.BufferWi; x++)
		{
			for (int y = 0; y < buffer.BufferHt; y++)
			{
				if (isLeft(wxPoint(lastvalue, buffer.BufferHt), slope, wxPoint(x, y)))
				{
					buffer.SetPixel(x, y, c1);
				}
				else
				{
					buffer.SetPixel(x, y, c2);
				}
			}
		}
	}
	else if (adjust == 50)
	{
		float step = (float)buffer.BufferWi / (float)(buffer.curEffEndPer - buffer.curEffStartPer);

		lastvalue = lastvalue + step;
		if (lastvalue >= buffer.BufferWi)
		{
			lastvalue = buffer.BufferWi - 1;
		}

		for (int x = 0; x < (int)lastvalue; x++)
		{
			for (int y = 0; y < buffer.BufferHt; y++)
			{
				buffer.SetPixel(x, y, c1);
			}
		}
		for (int x = (int)lastvalue; x < buffer.BufferWi; x++)
		{
			for (int y = 0; y < buffer.BufferHt; y++)
			{
				buffer.SetPixel(x, y, c2);
			}
		}
	}
	else
	{
		float step = ((float)(buffer.BufferWi + extrax)) / (float)(buffer.curEffEndPer - buffer.curEffStartPer);

		lastvalue = lastvalue + step;
		if (lastvalue >= buffer.BufferWi + extrax)
		{
			lastvalue = buffer.BufferWi + extrax - 1;
		}

		// start bottom left 0, 0
		// y = slope * x + y'

		for (int x = 0; x < buffer.BufferWi; x++)
		{
			for (int y = 0; y < buffer.BufferHt; y++)
			{
				if (isLeft(wxPoint(lastvalue, buffer.BufferHt), slope, wxPoint(x, y)))
				{
					buffer.SetPixel(x, y, c1);
				}
				else
				{
					buffer.SetPixel(x, y, c2);
				}
			}
		}
	}
}

void TransitionEffect::RenderClockWise(RenderBuffer &buffer, bool mode, float& lastvalue, int adjust, bool reverse)
{
	// adjust is the start angle
	if (lastvalue == -99999)
	{
		lastvalue = (float)adjust * 360.0 / 100.0;
	}

	float step = 360 / (float)(buffer.curEffEndPer - buffer.curEffStartPer);

	xlColor c1;
	xlColor c2;
	if (mode)
	{
		buffer.palette.GetColor(0, c1);
		buffer.palette.GetColor(1, c2);
	}
	else
	{
		buffer.palette.GetColor(1, c1);
		buffer.palette.GetColor(0, c2);
	}

	if (reverse)
	{
		step = step * -1.0;
	}
	lastvalue += step;
	if (lastvalue < 0)
	{
		lastvalue += 360;
	}
	else if (lastvalue >= 360)
	{
		lastvalue -= 360;
	}

	float startradians = 2.0 * PI * (float)adjust / 100.0;
	float currentradians = 2.0 * PI * lastvalue / 360.0;

	for (int x = 0; x < buffer.BufferWi; x++)
	{
		for (int y = 0; y < buffer.BufferHt; y++)
		{
			if (x < buffer.BufferWi / 2)
			{
				// left half
				float radianspixel = atan2(x, y);

				bool s_lt_p = (radianspixel - startradians > 0);
				bool c_lt_p = (radianspixel - currentradians > 0);
				bool s_lt_c = (currentradians - startradians > 0);

				if (s_lt_c)
				{
					if (s_lt_p && !c_lt_p)
					{
						// black
						buffer.SetPixel(x, y, c2);
					}
					else
					{
						buffer.SetPixel(x, y, c1);
					}
				}
				else
				{
					if (!s_lt_p && c_lt_p)
					{
						buffer.SetPixel(x, y, c1);
					}
					else
					{
						buffer.SetPixel(x, y, c2);
					}
				}
			}
		}
	}
}

void TransitionEffect::RenderFromMiddle(RenderBuffer &buffer, bool mode, float& lastvalue, int adjust, bool reverse)
{

}
void TransitionEffect::RenderSquareExplode(RenderBuffer &buffer, bool mode, float& lastvalue, int adjust, bool reverse)
{

}
void TransitionEffect::RenderCircleExplode(RenderBuffer &buffer, bool mode, float& lastvalue, int adjust, bool reverse)
{

}
void TransitionEffect::RenderBlinds(RenderBuffer &buffer, bool mode, float& lastvalue, int adjust, bool reverse)
{

}
void TransitionEffect::RenderBlend(RenderBuffer &buffer, bool mode, float& lastvalue, int adjust, bool reverse)
{

}
void TransitionEffect::RenderSlideChecks(RenderBuffer &buffer, bool mode, float& lastvalue, int adjust, bool reverse)
{

}
void TransitionEffect::RenderSlideBars(RenderBuffer &buffer, bool mode, float& lastvalue, int adjust, bool reverse)
{

}
