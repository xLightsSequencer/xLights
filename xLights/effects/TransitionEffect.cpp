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
		   SettingsMap.GetBool("CHECKBOX_Transition_Reveal", TRUE),
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
	else if (type == "Circle Explode")
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
		RenderFromMiddle(buffer, _mode, _lastvalue, _reverse);
			break;
		case 4:
		RenderSquareExplode(buffer, _mode, _lastvalue, _reverse);
		break;
		case 5:
		RenderCircleExplode(buffer, _mode, _lastvalue, _reverse);
			break;
		case 6:
			RenderBlinds(buffer, _mode, _lastvalue, _adjust);
			break;
		case 7:
		RenderBlend(buffer, _mode, _lastvalue, _adjust);
			break;
		case 8:
		RenderSlideChecks(buffer, _mode, _lastvalue, _adjust);
			break;
		case 9:
		RenderSlideBars(buffer, _mode, _lastvalue, _adjust);
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
	// aa contains the current x value and the buffer height
	// test is the point we are testing
	int extrax = std::abs(aa.y / slope);
	wxPoint a(0, 0);
	wxPoint b(0, 0);
	if (slope < 0)
	{
		b = wxPoint(aa.x - extrax, aa.y);
		a = wxPoint(aa.x, 0);
	}
	else
	{
		b = aa;
		a = wxPoint(aa.x - extrax, 0);
	}

	return ((b.x - a.x)*(test.y - a.y) - (b.y - a.y)*(test.x - a.x)) > 0;
}

void TransitionEffect::RenderWipe(RenderBuffer &buffer, bool mode, float& lastvalue, int adjust)
{
	float angle = PI * (float)adjust / 100.0;

	float slope = tan(angle);
    float extrax = std::abs(buffer.BufferHt / slope);
	if (lastvalue == -99999)
	{
		lastvalue = 0;
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

	if (adjust == 0)
	{
		float step = -1 * ((float)buffer.BufferHt / (float)(buffer.curEffEndPer - buffer.curEffStartPer));

		lastvalue = lastvalue + step;
		if (lastvalue >= buffer.BufferHt)
		{
			lastvalue = buffer.BufferHt - 1;
		}

		for (int y = buffer.BufferHt-1; y > (int)lastvalue; y--)
		{
			for (int x = 0; x < buffer.BufferWi; x++)
			{
				buffer.SetPixel(x, y, c2);
			}
		}
		for (int y = (int)lastvalue; y >= 0; y--)
		{
			for (int x = 0; x < buffer.BufferWi; x++)
			{
				buffer.SetPixel(x, y, c1);
			}
		}
	}
	else if (adjust == 100)
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
	// this is like an xor
	if (mode == reverse)
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
			float radianspixel;
			if (x - buffer.BufferWi / 2 == 0 && y - buffer.BufferHt / 2 == 0)
			{
				radianspixel = 0.0;
			}
			else
			{
				radianspixel = atan2(x - buffer.BufferWi / 2, y - buffer.BufferHt / 2);
			}
			if (radianspixel < 0)
			{
				radianspixel += 2 * PI;
			}

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

void TransitionEffect::RenderFromMiddle(RenderBuffer &buffer, bool mode, float& lastvalue, bool reverse)
{
	if (lastvalue == -99999)
	{
		if (reverse)
		{
			lastvalue = buffer.BufferWi / 2;
		}
		else
		{
			lastvalue = 0;
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

	float step = ((float)buffer.BufferWi / 2.0) / (float)(buffer.curEffEndPer - buffer.curEffStartPer);


	if (reverse)
	{
		lastvalue = lastvalue - step;
		if (lastvalue < 0)
		{
			lastvalue = 0;
		}
	}
	else
	{
		lastvalue = lastvalue + step;
		if (lastvalue >= buffer.BufferWi / 2)
		{
			lastvalue = buffer.BufferWi / 2 - 1;
		}
	}

	int x1 = buffer.BufferWi / 2 - lastvalue;
	int x2 = buffer.BufferWi / 2 + lastvalue;
	for (int x = 0; x < buffer.BufferWi; x++)
	{
		xlColor c;
		if (x < x1)
		{
			c = c2;
		}
		else if (x < x2)
		{
			c = c1;
		}
		else
		{
			c = c2;
		}
		for (int y = 0; y < buffer.BufferHt; y++)
		{
			buffer.SetPixel(x, y, c);
		}
	}
}
void TransitionEffect::RenderSquareExplode(RenderBuffer &buffer, bool mode, float& lastvalue, bool reverse)
{
	if (lastvalue == -99999)
	{
		if (reverse)
		{
			lastvalue = std::max(buffer.BufferWi / 2, buffer.BufferHt / 2);
		}
		else
		{
			lastvalue = 0;
		}
	}

	xlColor c1;
	xlColor c2;
	if (mode == reverse)
	{
		buffer.palette.GetColor(0, c1);
		buffer.palette.GetColor(1, c2);
	}
	else
	{
		buffer.palette.GetColor(1, c1);
		buffer.palette.GetColor(0, c2);
	}

	float step = std::max(((float)buffer.BufferWi / 2.0), ((float)buffer.BufferHt / 2.0)) / (float)(buffer.curEffEndPer - buffer.curEffStartPer);

	if (reverse)
	{
		lastvalue = lastvalue - step;
		if (lastvalue < 0)
		{
			lastvalue = 0;
		}
	}
	else
	{
		lastvalue = lastvalue + step;
		if (lastvalue >= std::max(buffer.BufferWi / 2, buffer.BufferHt / 2))
		{
			lastvalue = std::max(buffer.BufferWi / 2, buffer.BufferHt / 2) - 1;
		}
	}

	int x1 = buffer.BufferWi / 2 - lastvalue;
	int x2 = buffer.BufferWi / 2 + lastvalue;
	int y1 = buffer.BufferHt / 2 - lastvalue;
	int y2 = buffer.BufferHt / 2 + lastvalue;
	for (int x = 0; x < buffer.BufferWi; x++)
	{
		for (int y = 0; y < buffer.BufferHt; y++)
		{
			xlColor c;
			if (x < x1 || x > x2 || y < y1 || y > y2)
			{
				c = c1;
			}
			else
			{
				c = c2;
			}
			buffer.SetPixel(x, y, c);
		}
	}
}
void TransitionEffect::RenderCircleExplode(RenderBuffer &buffer, bool mode, float& lastvalue, bool reverse)
{
	// distance from centre
	// sqrt((x - buffer.BufferWi / 2) ^ 2 + (y - buffer.BufferHt / 2) ^ 2);
	float maxradius = sqrt(((buffer.BufferWi / 2) * (buffer.BufferWi / 2)) + ((buffer.BufferHt / 2) * (buffer.BufferHt / 2)));
	if (lastvalue == -99999)
	{
		if (reverse)
		{
			lastvalue = maxradius;
		}
		else
		{
			lastvalue = 0;
		}
	}

	xlColor c1;
	xlColor c2;
	if (mode == reverse)
	{
		buffer.palette.GetColor(1, c1);
		buffer.palette.GetColor(0, c2);
	}
	else
	{
		buffer.palette.GetColor(0, c1);
		buffer.palette.GetColor(1, c2);
	}

	float step = maxradius / (float)(buffer.curEffEndPer - buffer.curEffStartPer);

	if (reverse)
	{
		lastvalue = lastvalue - step;
		if (lastvalue < 0)
		{
			lastvalue = 0;
		}
	}
	else
	{
		lastvalue = lastvalue + step;
		if (lastvalue >= maxradius)
		{
			lastvalue = maxradius;
		}
	}

	for (int x = 0; x < buffer.BufferWi; x++)
	{
		for (int y = 0; y < buffer.BufferHt; y++)
		{
			float radius = sqrt((x - (buffer.BufferWi / 2)) * (x - (buffer.BufferWi / 2)) + (y - (buffer.BufferHt / 2)) * (y - (buffer.BufferHt / 2)));
			xlColor c;
			if (radius < lastvalue)
			{
				c = c1;
			}
			else
			{
				c = c2;
			}
			buffer.SetPixel(x, y, c);
		}
	}
}
void TransitionEffect::RenderBlinds(RenderBuffer &buffer, bool mode, float& lastvalue, int adjust)
{
	if (adjust == 0)
	{
		adjust = 1;
	}
	adjust = (buffer.BufferWi / 2) * adjust / 100;
	if (adjust == 0)
	{
		adjust = 1;
	}

	int per = buffer.BufferWi / adjust;
	if (per < 1)
	{
		per = 1;
	}
	int blinds = buffer.BufferWi / per;
	while (blinds * per < buffer.BufferWi)
	{
		blinds++;
	}

	float step = ((float)per / (float)(buffer.curEffEndPer - buffer.curEffStartPer));

	if (lastvalue == -99999)
	{
		lastvalue = 0;
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

	lastvalue = lastvalue + step;
	if (lastvalue >= per)
	{
		lastvalue = per;
	}

	for (int x = 0; x < per; x++)
	{
		for (int i = 0; i < blinds; i++)
		{
			if (x + (i * per) < buffer.BufferWi)
			{
				if (x <= (int)lastvalue)
				{
					for (int y = 0; y < buffer.BufferWi; y++)
					{
						buffer.SetPixel(x + (i*per), y, c1);
					}
				}
				else
				{
					for (int y = 0; y < buffer.BufferWi; y++)
					{
						buffer.SetPixel(x + (i*per), y, c2);
					}
				}
			}
		}
	}
}
void TransitionEffect::RenderBlend(RenderBuffer &buffer, bool mode, float& lastvalue, int adjust)
{
	int seed = 1234;
	srand(seed);
	int pixels = buffer.BufferWi * buffer.BufferHt;
	if (lastvalue == -99999)
	{
		lastvalue = 0;
	}
	adjust = 10 * adjust / 100;
	if (adjust == 0)
	{
		adjust = 1;
	}
	int actualpixels = pixels / (adjust * adjust);

	float step = ((float)pixels / (adjust*adjust)) / ((float)(buffer.curEffEndPer - buffer.curEffStartPer));

	lastvalue += step;
	if (lastvalue > actualpixels)
	{
		lastvalue = actualpixels;
	}
	int xpixels = buffer.BufferWi / adjust;
	while (xpixels * adjust < buffer.BufferWi)
	{
		xpixels++;
	}
	int ypixels = buffer.BufferHt / adjust;
	while (ypixels * adjust < buffer.BufferHt)
	{
		ypixels++;
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

	// set all the background first
	for (int x = 0; x < buffer.BufferWi; x++)
	{
		for (int y = 0; y < buffer.BufferWi; y++)
		{
			buffer.SetPixel(x, y, c2);
		}
	}

	for (int i = 0; i < lastvalue; i++)
	{
		int j = rand() % actualpixels;

		int x = j % xpixels * adjust;
		int y = j / xpixels * adjust;

		xlColor curcolor;
		buffer.GetPixel(x, y, curcolor);

		if (curcolor == c1)
		{
			i--;
		}
		else
		{
			for (int k = 0; k < adjust; k++)
			{
				for (int l = 0; l < adjust; l++)
				{
					buffer.SetPixel(x + k, y + l, c1);
				}
			}
		}
	}
}
void TransitionEffect::RenderSlideChecks(RenderBuffer &buffer, bool mode, float& lastvalue, int adjust)
{
	if (adjust < 2)
	{
		adjust = 2;
	}
	adjust = (std::max(buffer.BufferWi / 2, buffer.BufferHt /2)) * adjust / 100;
	if (adjust < 2)
	{
		adjust = 2;
	}

	int xper = buffer.BufferWi / adjust;
	if (xper < 1)
	{
		xper = 1;
	}
	int yper = buffer.BufferHt / adjust;
	if (yper < 1)
	{
		yper = 1;
	}
	int xblinds = buffer.BufferWi / xper;
	while (xblinds * xper < buffer.BufferWi)
	{
		xblinds++;
	}
	int yblinds = buffer.BufferHt / yper;
	while (yblinds * yper < buffer.BufferHt)
	{
		yblinds++;
	}

	float step = (((float)xper*2.0) / (float)(buffer.curEffEndPer - buffer.curEffStartPer));

	if (lastvalue == -99999)
	{
		lastvalue = 0;
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

	lastvalue = lastvalue + step;
	if (lastvalue >= xper * 2)
	{
		lastvalue = xper;
	}

	for (int i = 0; i < xblinds; i++)
	{
		if (i % 2 == 0)
		{
			for (int j = 0; j < yblinds; j++)
			{
				for (int x = 0; x < xper; x++)
				{
					for (int y = 0; y < yper; y++)
					{
						if (j % 2 == 0)
						{
							if (x < lastvalue)
							{
								buffer.SetPixel(xper * i + x, yper * j + y, c2);
							}
							else
							{
								buffer.SetPixel(xper * i + x, yper * j + y, c1);
							}
						}
						else
						{
							if (x + xper < lastvalue)
							{
								buffer.SetPixel(xper * i + x, yper * j + y, c2);
							}
							else
							{
								buffer.SetPixel(xper * i + x, yper * j + y, c1);
							}
						}
					}
				}
			}
		}
		else
		{
			for (int j = 0; j < yblinds; j++)
			{
				for (int x = 0; x < xper; x++)
				{
					for (int y = 0; y < yper; y++)
					{
						if (j % 2 == 1)
						{
							if (x >= lastvalue)
							{
								buffer.SetPixel(xper * i + x, yper * j + y, c1);
							}
							else
							{
								buffer.SetPixel(xper * i + x, yper * j + y, c2);
							}
						}
						else
						{
							if (x + xper >= lastvalue)
							{
								buffer.SetPixel(xper * i + x, yper * j + y, c1);
							}
							else
							{
								buffer.SetPixel(xper * i + x, yper * j + y, c2);
							}
						}
					}
				}
			}
		}
	}
}
void TransitionEffect::RenderSlideBars(RenderBuffer &buffer, bool mode, float& lastvalue, int adjust)
{
	if (adjust == 0)
	{
		adjust = 1;
	}
	adjust = (buffer.BufferHt / 2) * adjust / 100;
	if (adjust == 0)
	{
		adjust = 1;
	}

	int per = buffer.BufferHt / adjust;
	if (per < 1)
	{
		per = 1;
	}
	int blinds = buffer.BufferHt / per;
	while (blinds * per < buffer.BufferHt)
	{
		blinds++;
	}

	float step = ((float)buffer.BufferWi / (float)(buffer.curEffEndPer - buffer.curEffStartPer));

	if (lastvalue == -99999)
	{
		lastvalue = 0;
	}
	lastvalue += step;

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

	for (int i = 0; i < blinds; i++)
	{
		if (i % 2 == 0)
		{
			for (int x = 0; x < lastvalue; x++)
			{
				for (int y = i * per; y < (i + 1) * per; y++)
				{
					buffer.SetPixel(x, y, c1);
				}
			}
			for (int x = lastvalue; x < buffer.BufferWi; x++)
			{
				for (int y = i * per; y < (i + 1) * per; y++)
				{
					buffer.SetPixel(x, y, c2);
				}
			}
		}
		else
		{
			for (int x = 0; x < buffer.BufferWi - lastvalue; x++)
			{
				for (int y = i * per; y < (i + 1) * per; y++)
				{
					buffer.SetPixel(x, y, c2);
				}
			}
			for (int x = buffer.BufferWi - lastvalue; x < buffer.BufferWi; x++)
			{
				for (int y = i * per; y < (i + 1) * per; y++)
				{
					buffer.SetPixel(x, y, c1);
				}
			}
		}
	}
}
