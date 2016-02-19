#include "TendrilEffect.h"
#include "TendrilPanel.h"

#include "../sequencer/Effect.h"
#include "../RenderBuffer.h"
#include "../UtilClasses.h"
#include "../AudioManager.h"

#include <wx/graphics.h>
#if wxUSE_GRAPHICS_CONTEXT == 0
  #error Please refer to README.windows to make necessary changes to wxWidgets setup.h file.
  #error You will also need to rebuild wxWidgets once the change is made.
#endif

#include "../../include/tendril-16.xpm"
#include "../../include/tendril-24.xpm"
#include "../../include/tendril-32.xpm"
#include "../../include/tendril-48.xpm"
#include "../../include/tendril-64.xpm"

#define wrdebug(...)

bool TendrilEffect::needToAdjustSettings(const std::string &version)
{
	return IsVersionOlder("2016.8", version);
}

void TendrilEffect::adjustSettings(const std::string &version, Effect *effect)
{
	SettingsMap &settings = effect->GetSettings();
	int movement = settings.GetInt("SLIDER_Tendril_Movement", -1);

	if (movement != -1)
	{
		settings.erase("E_SLIDER_Tendril_Movement");
		switch (movement)
		{
		case 1:
			settings["E_CHOICE_Tendril_Movement"] = "Random";
			break;
		case 2:
			settings["E_CHOICE_Tendril_Movement"] = "Square";
			break;
		case 3:
			settings["E_CHOICE_Tendril_Movement"] = "Circle";
			break;
		case 4:
			settings["E_CHOICE_Tendril_Movement"] = "Horizontal Zig Zag";
			break;
		case 5:
			settings["E_CHOICE_Tendril_Movement"] = "Vertical Zig Zag";
			break;
		case 6:
			settings["E_CHOICE_Tendril_Movement"] = "Music Line";
			break;
		case 7:
			settings["E_CHOICE_Tendril_Movement"] = "Music Circle";
			break;
		}
	}

	// also give the base class a chance to adjust any settings
	if (RenderableEffect::needToAdjustSettings(version))
	{
		RenderableEffect::adjustSettings(version, effect);
	}
}

TendrilNode::TendrilNode(float x_, float y_)
{
    x = x_;
    y = y_;
    vx = 0;
    vy = 0;
}

// Note callers of this function have to delete the returned wxPoint
wxPoint* TendrilNode::Point()
{
    return new wxPoint(rint(x) ,rint(y));
}

ATendril::~ATendril()
{
	while (_nodes.size() != 0)
	{
		TendrilNode* p = _nodes.front();
		_nodes.pop_front();
		if (p != NULL)
		{
			delete p;
		}
	}
}

ATendril::ATendril(float friction, int size, float dampening, float tension, float spring, wxPoint* start, xlColor colour, int thickness, int maxx, int maxy)
{
    _width = maxx;
    _height = maxy;
    _size = 60;
    if (size > 0)
    {
		_size = size;
	}
	_thickness = 2;
	if (thickness > 0)
	{
		_thickness = thickness;
	}
	_colour = xlWHITE;
	if (colour != xlWHITE)
	{
		_colour = colour;
	}
	_dampening = 0.25;
	if (dampening >= 0)
	{
		_dampening = dampening;
	}
	_tension = 0.98;
	if (tension >= 0)
	{
		_tension = tension;
	}
	_spring = 0;
	if (spring >= 0)
	{
		_spring = spring;
	}
	_friction = 0.5;
	if (friction >= 0)
	{
		_friction = friction + ((float)rand())/(float)RAND_MAX * 0.01f - 0.005f;
	}
	else
	{
		_friction = _friction + ((float)rand())/(float)RAND_MAX * 0.01f - 0.005f;
	}

    _nodes.clear();
	for (int i = 0; i < _size; i++)
	{
		TendrilNode* node = new TendrilNode(start->x, start->y);
		if (node != NULL)
		{
			_nodes.push_back(node);
		}
	}
}

void ATendril::Update(wxPoint* target)
{
	float spring = _spring;
	TendrilNode* node = _nodes.front();
	if (node != NULL)
	{
		node->vx += (target->x - node->x) * spring;
		node->vy += (target->y - node->y) * spring;

		TendrilNode* prev = NULL;
		for (std::list<TendrilNode*>::const_iterator ci = _nodes.begin(); ci != _nodes.end(); ++ci)
		{
			node = *ci;
			if (prev != NULL)
			{
				node->vx += (prev->x - node->x) * spring;
				node->vy += (prev->y - node->y) * spring;
				node->vx += prev->vx * _dampening;
				node->vy += prev->vy * _dampening;
			}
			node->vx *= _friction;
			node->vy *= _friction;
			node->x += node->vx;
			node->y += node->vy;
			if (node->x < -1 * _width)
			{
				node->x = -1 * _width;
			}
			if (node->x > 2 * _width)
			{
				node->x = 2 * _width;
			}
			if (node->y < -1 * _height)
			{
				node->y = -1 * _height;
			}
			if (node->y > 2 * _height)
			{
				node->y = 2 * _height;
			}
			prev = node;
			spring *= _tension;
		}
	}
}

void ATendril::Draw(DrawingContext* gc)
{
    wxColor c(_colour);
    wxPen pen(c, _thickness);
    gc->SetPen(pen);

    wxGraphicsPath path = gc->CreatePath();
    path.MoveToPoint(_nodes.front()->x, _nodes.front()->y);

    std::list<TendrilNode*>::const_iterator ci = _nodes.begin();
    ++ci; // move to second node

    std::list<TendrilNode*>::const_iterator ci_second_last = _nodes.end();
    --ci_second_last;
    --ci_second_last;

    TendrilNode* a;
    TendrilNode* b;
    for(;ci != ci_second_last; ++ci)
    {
        TendrilNode* a = *ci;
        std::list<TendrilNode*>::const_iterator cinext = ci;
        ++cinext;
        TendrilNode* b = *cinext;
        float x = (a->x + b->x) * 0.5;
        float y = (a->y + b->y) * 0.5;
        path.AddQuadCurveToPoint(a->x, a->y, x, y);
    }

    a = *ci;
    b = *(++ci);
    path.AddQuadCurveToPoint(a->x, a->y, b->x, b->y);
    gc->StrokePath(path);
}

wxPoint* ATendril::LastLocation()
{
    TendrilNode *last = _nodes.back();
    if (last != NULL)
    {
        return last->Point();
    }
    else
    {
        return NULL;
    }
}

Tendril::~Tendril()
{
	while (_tendrils.size() != 0)
	{
		ATendril* p = _tendrils.front();
		_tendrils.pop_front();
		if (p != NULL)
		{
			delete p;
		}
	}
}

Tendril::Tendril(float friction, int trails, int size, float dampening, float tension, float springbase, float springincr, wxPoint* start, xlColor colour, int thickness, int maxx, int maxy)
{
    _width = maxx;
    _height = maxy;
	float sb = 0.45f;
	if (springbase >= 0)
	{
		sb = springbase;
	}
	float si = 0.025f;
	if (springincr >= 0)
	{
		si = springincr;
	}
	int t = 10;
	if (trails > 0)
	{
		t = trails;
	}

	_tendrils.clear();
	for (int i = 0; i < t; i++)
	{
		float aspring = sb + si * ((float)i / (float)t);
		ATendril* t = new ATendril(friction, size, dampening, tension, aspring, start, colour, thickness, maxx, maxy);
		if (t != NULL)
		{
			_tendrils.push_back(t);
		}
	}
}

void Tendril::UpdateRandomMove(int tunemovement)
{
    if (tunemovement < 1)
    {
        tunemovement = 1;
    }

	int minx = -1 * _width / 4;
	int miny = -1 * _height / 4;
	int maxx = _width + _width / 4;
	int maxy = _height + _height / 4;
	int minmovex = -1 * _width * 2 * tunemovement / 20;
	int minmovey = -1 * _height * 2 * tunemovement / 20;
	int maxmovex = _width * 2 * tunemovement / 20;
	int maxmovey = _height * 2 * tunemovement / 20;

	ATendril* t = _tendrils.front();
	if (t != NULL)
	{
		wxPoint* current = t->LastLocation();

		if (current != NULL)
		{
			int realminmovex = minmovex;
			if (minmovex < 0)
			{
				realminmovex = -1 * std::min(current->x, minmovex * -1);
			}
			int realmaxmovex = maxmovex;
			if (maxmovex > 0)
			{
				realmaxmovex = std::min(maxx - current->x, maxmovex);
			}
			int realminmovey = minmovey;
			if (minmovey < 0)
			{
				realminmovey = -1 * std::min(current->y, minmovey * -1);
			}
			int realmaxmovey = maxmovey;
			if (maxmovey > 0)
			{
				realmaxmovey = std::min(maxy - current->y, maxmovey);
			}

			int xmove = -1 * realminmovex + realmaxmovex;
			int ymove = -1 * realminmovey + realmaxmovey;
			int x = 0;
			if (xmove > 0)
			{
				x = (rand() % xmove) + realminmovex;
			}
			int y = 0;
			if (ymove > 0)
			{
				y = (rand() % ymove) + realminmovey;
			}

			current->x = current->x + x;
			current->y = current->y + y;

			if (current->x < minx)
			{
				current->x = minx;
			}
			if (current->x > maxx)
			{
				current->x = maxx;
			}
			if (current->y < miny)
			{
				current->y = miny;
			}
			if (current->y > maxy)
			{
				current->y = maxy;
			}
			Update(current);
			delete current;
		}
#ifdef _DEBUG
		else
		{
			int a = 0;
		}
#endif
	}
#ifdef _DEBUG
	else
	{
		int a = 0;
	}
#endif
}

void Tendril::Update(wxPoint* target)
{
    for (std::list<ATendril*>::const_iterator ci = _tendrils.begin(); ci != _tendrils.end(); ++ci)
    {
        (*ci)->Update(target);
    }
}

void Tendril::Update(int x, int y)
{
    wxPoint pt(x,y);
    Update(&pt);
}

void Tendril::Draw(DrawingContext* gc)
{
	for (std::list<ATendril*>::const_iterator ci = _tendrils.begin(); ci != _tendrils.end(); ++ci)
	{
		(*ci)->Draw(gc);
	}
}

TendrilEffect::TendrilEffect(int id) : RenderableEffect(id, "Tendril", tendril_16, tendril_24, tendril_32, tendril_48, tendril_64)
{
}

TendrilEffect::~TendrilEffect()
{
}
bool TendrilEffect::CanRenderOnBackgroundThread() {
    return true;
}
wxPanel *TendrilEffect::CreatePanel(wxWindow *parent) {
    return new TendrilPanel(parent);
}

void TendrilEffect::Render(Effect *effect, const SettingsMap &SettingsMap, RenderBuffer &buffer) {
    Render(buffer,
		   SettingsMap.Get("CHOICE_Tendril_Movement", "Random"),
           SettingsMap.GetInt("TEXTCTRL_Tendril_TuneMovement", 10),
           SettingsMap.GetInt("TEXTCTRL_Tendril_Speed", 10),
           SettingsMap.GetInt("TEXTCTRL_Tendril_Thickness", 1),
           SettingsMap.GetFloat("TEXTCTRL_Tendril_Friction", 10) / 20 * 0.2 + 0.4, // 0.4->0.6 but on screen 0-20: def 0.5
           SettingsMap.GetFloat("TEXTCTRL_Tendril_Dampening", 10) / 20 * 0.5, // 0->0.5 but on screen 0-20: def 0.25
           SettingsMap.GetFloat("TEXTCTRL_Tendril_Tension", 20) / 39 * 0.039 + 0.96, // 0.960->0.999 but on screen 0->39: def 0.980
           SettingsMap.GetInt("TEXTCTRL_Tendril_Trails", 1),
           SettingsMap.GetInt("TEXTCTRL_Tendril_Length", 60)
           );
}

class TendrilRenderCache : public EffectRenderCache {
public:
    TendrilRenderCache() { _tendril = NULL; };
    virtual ~TendrilRenderCache() {
		if (_tendril != NULL)
		{
			delete _tendril;
			_tendril = NULL;
		}
	};

    int _mv1;
    int _mv2;
    int _mv3;
    int _mv4;
	int _thickness;
    int _movement;
    int _tunemovement;
    float _friction;
    float _dampening;
    float _tension;
    int _trails;
    int _length;
    xlColor _colour;
    Tendril* _tendril;
};

void TendrilEffect::Render(RenderBuffer &buffer, const std::string& movement,
                            int tunemovement, int movementSpeed, int thickness,
                           float friction, float dampening,
                           float tension, int trails, int length)
{
    buffer.drawingContext->Clear();

    if (friction < 0.4)
    {
        friction = 0.4;
    }
    if (friction > 0.6)
    {
        friction = 0.6;
    }
    if (dampening < 0)
    {
        dampening = 0;
    }
    if (dampening > 0.5)
    {
        dampening = 0.5;
    }
    if (tension < 0.96)
    {
        tension = 0.96;
    }
    if (tension > 0.999)
    {
        tension = 0.999;
    }
    TendrilRenderCache *cache = (TendrilRenderCache*)buffer.infoCache[id];
    if (cache == nullptr) {
        cache = new TendrilRenderCache();
        buffer.infoCache[id] = cache;
    }

    int &_movement = cache->_movement;
    int &_tunemovement = cache->_tunemovement;
    int &_mv1 = cache->_mv1;
    int &_mv2 = cache->_mv2;
    int &_mv3 = cache->_mv3;
    int &_mv4 = cache->_mv4;
    int &_thickness = cache->_thickness;
    int &_trails = cache->_trails;
    int &_length = cache->_length;
    float &_friction = cache->_friction;
    float &_dampening = cache->_dampening;
    float &_tension = cache->_tension;
    xlColor &_colour = cache->_colour;
    Tendril* &_tendril = cache->_tendril;
    xlColor color1;
    buffer.palette.GetColor(0, color1);

	int nMovement = 1;
	if (movement == "Random")
	{
		nMovement = 1;
	}
	else if (movement == "Square")
	{
		nMovement = 2;
	}
	else if (movement == "Circle")
	{
		nMovement = 3;
	}
	else if (movement == "Horizontal Zig Zag")
	{
		nMovement = 4;
	}
	else if (movement == "Vertical Zig Zag")
	{
		nMovement = 5;
	}
	else if (movement == "Music Line")
	{
		nMovement = 6;
	}
	else if (movement == "Music Circle")
	{
		nMovement = 7;
	}

    if (_tendril == NULL ||
        _movement != nMovement ||
        _tunemovement != tunemovement ||
        _thickness != thickness ||
        _friction != friction ||
        _dampening != dampening ||
        _tension != tension ||
        _trails != trails ||
        _length != length ||
        _colour != color1)
    {
        _thickness = thickness;
        _friction = friction;
        _dampening = dampening;
        _tension = tension;
        _trails = trails;
        _length = length;
        _colour = color1;
		wxPoint startmiddle(buffer.BufferWi / 2, buffer.BufferHt / 2);
		wxPoint startmiddletop(buffer.BufferWi / 2, buffer.BufferHt);
		wxPoint startmiddlebottom(buffer.BufferWi / 2, 0);
		wxPoint startbottomleft(0, 0);
		wxPoint starttopleft(0, buffer.BufferHt);
		wxPoint startmiddleleft(0, buffer.BufferHt / 2);
		if (_tendril != NULL)
		{
			delete _tendril;
			_tendril = NULL;
		}

        if (_movement != nMovement || _tunemovement != tunemovement)
        {
            switch(nMovement)
            {
            case 1:
                // random
				_tendril = new Tendril(_friction, _trails, _length, _dampening, _tension, -1, -1, &startmiddle, _colour, _thickness, buffer.BufferWi, buffer.BufferHt);
				break;
            case 2:
                // corners
                _mv1 = 0; // current x
                _mv2 = 0; // current y
                _mv3 = 0; // corner
                _mv4 = tunemovement; // movement amount
                if (_mv4 == 0)
                {
                    _mv4 = 1;
                }
				_tendril = new Tendril(_friction, _trails, _length, _dampening, _tension, -1, -1, &startbottomleft, _colour, _thickness, buffer.BufferWi, buffer.BufferHt);
				break;
            case 3:
                // circles
                _mv1 = 0; // radians
                _mv2 = std::min(buffer.BufferWi, buffer.BufferHt) / 2; // radius
                _mv3 = tunemovement * 3;
                if (_mv3 == 0)
                {
                    _mv3 = 1;
                }
				_tendril = new Tendril(_friction, _trails, _length, _dampening, _tension, -1, -1, &startmiddle, _colour, _thickness, buffer.BufferWi, buffer.BufferHt);
				break;
            case 4:
                // horizontal zig zag
                _mv1 = 0; // current y
                _mv2 = (double)tunemovement * 1.5;
                if (_mv2 == 0)
                {
                    _mv2 = 1;
                }
                _mv3 = 1; // direction
				_tendril = new Tendril(_friction, _trails, _length, _dampening, _tension, -1, -1, &startmiddlebottom, _colour, _thickness, buffer.BufferWi, buffer.BufferHt);
				break;
            case 5:
                // vertical zig zag
                _mv1 = 0; // current x
                _mv2 = (double)tunemovement * 1.5;
                _mv3 = 1; // direction
				_tendril = new Tendril(_friction, _trails, _length, _dampening, _tension, -1, -1, &startmiddleleft, _colour, _thickness, buffer.BufferWi, buffer.BufferHt);
				break;
			case 6:
				// line movement based on music
				_mv1 = 0; // current x
				_mv3 = tunemovement; // direction
				if (_mv3 < 1)
				{
					_mv3 = 1;
				}
				_tendril = new Tendril(_friction, _trails, _length, _dampening, _tension, -1, -1, &startbottomleft, _colour, _thickness, buffer.BufferWi, buffer.BufferHt);
				break;
			case 7:
				// circle movement based on music
				_mv1 = 0; // radians
				_mv2 = std::min(buffer.BufferWi, buffer.BufferHt) / 2; // max radius
				_mv3 = tunemovement * 3;
				if (_mv3 < 1)
				{
					_mv3 = 1;
				}
				_tendril = new Tendril(_friction, _trails, _length, _dampening, _tension, -1, -1, &startmiddle, _colour, _thickness, buffer.BufferWi, buffer.BufferHt);
				break;
            }
        }
        _movement = nMovement;
        _tunemovement = tunemovement;
    }

    const double PI  = 3.141592653589793238463;
    int speed = 10 - movementSpeed;
	if (speed <= 0 || buffer.curPeriod % speed == 0)
	{
		switch (_movement)
		{
		case 1:
			if (_tendril != NULL)
			{
				_tendril->UpdateRandomMove(_tunemovement);
			}
			break;
		case 2:
			switch (_mv3)
			{
			case 0:
				_mv1 += std::max(buffer.BufferWi / _mv4, 1);
				if (_mv1 >= buffer.BufferWi - buffer.BufferWi / _mv4)
				{
					_mv3++;
				}
				break;
			case 1:
				_mv2 += std::max(buffer.BufferHt / _mv4, 1);
				if (_mv2 >= buffer.BufferHt - buffer.BufferHt / _mv4)
				{
					_mv3++;
				}
				break;
			case 2:
				_mv1 -= std::max(buffer.BufferWi / _mv4, 1);
				if (_mv1 <= buffer.BufferWi / _mv4)
				{
					_mv3++;
				}
				break;
			case 3:
				_mv2 -= std::max(buffer.BufferHt / _mv4, 1);
				if (_mv2 <= buffer.BufferHt / _mv4)
				{
					_mv3 = 0;
				}
				break;
			}
			if (_tendril != NULL)
			{
				_tendril->Update(_mv1, _mv2);
			}
			break;
		case 3:
		{
			_mv1 = _mv1 + _mv3;
			if (_mv3 > 360)
			{
				_mv3 = 0;
			}
			int x = sin((double)_mv1 / 360.0 * PI * 2.0) * (double)_mv2 + (double)buffer.BufferWi / 2.0;
			int y = cos((double)_mv1 / 360.0 * PI * 2.0) * (double)_mv2 + (double)buffer.BufferHt / 2.0;
			if (_tendril != NULL)
			{
				_tendril->Update(x, y);
			}
		}
			break;
		case 4:
		{
			_mv1 = _mv1 + _mv3;
			int x = sin(std::max((double)buffer.BufferHt / (double)_mv2, 0.5) * PI * (double)_mv1 / (double)buffer.BufferHt) * (double)buffer.BufferWi / 2.0 + (double)buffer.BufferWi / 2.0;
			if (_mv1 >= buffer.BufferHt || _mv1 <= 0)
			{
				_mv3 = _mv3 * -1;
			}
			if (_mv3 < 0)
			{
				x = buffer.BufferWi - x;
			}
			if (_tendril != NULL)
			{
				_tendril->Update(x, _mv1);
			}
		}
		break;
		case 5:
		{
			_mv1 = _mv1 + _mv3;
			int y = sin(std::max((double)buffer.BufferWi / (double)_mv2, 0.5) * PI * (double)_mv1 / (double)buffer.BufferWi) * (double)buffer.BufferHt / 2.0 + (double)buffer.BufferHt / 2.0;
			if (_mv1 >= buffer.BufferWi || _mv1 <= 0)
			{
				_mv3 = _mv3 * -1;
			}
			if (_mv3 < 0)
			{
				y = buffer.BufferHt - y;
			}
			if (_tendril != NULL)
			{
				_tendril->Update(_mv1, y);
			}
		}
		break;
		case 6:
		{
			float f = 0.1;
			if (buffer.GetMedia() != NULL)
			{
				std::list<float>* p = buffer.GetMedia()->GetFrameData(buffer.curPeriod, FRAMEDATA_HIGH, "");
				if (p != NULL)
				{
					f = *p->begin();
				}
			}

			_mv1 = _mv1 + _mv3;
			if (_mv1 < 0 || _mv1 > buffer.BufferWi)
			{
				_mv3 = _mv3 * -1;
			}

			if (_tendril != NULL)
			{
				_tendril->Update(_mv1, buffer.BufferHt * f);
			}
		}
		break;
		case 7:
		{
			float f = 0.1;
			if (buffer.GetMedia() != NULL)
			{
				std::list<float>* p = buffer.GetMedia()->GetFrameData(buffer.curPeriod, FRAMEDATA_HIGH, "");
				if (p != NULL)
				{
					f = *p->begin();
				}
			}

			_mv1 = _mv1 + _mv3;
			if (_mv3 > 360)
			{
				_mv3 = 0;
			}
			int x = sin((double)_mv1 / 360.0 * PI * 2.0) * (double)_mv2 * f * 2 + (double)buffer.BufferWi / 2.0;
			int y = cos((double)_mv1 / 360.0 * PI * 2.0) * (double)_mv2 * f * 2 + (double)buffer.BufferHt / 2.0;
			if (_tendril != NULL)
			{
				_tendril->Update(x, y);
			}
		}
		break;
		}
	}

	if (_tendril != NULL)
	{
		_tendril->Draw(buffer.drawingContext);
	}
    wxImage * image = buffer.drawingContext->FlushAndGetImage();
    bool hasAlpha = image->HasAlpha();

    xlColor c;
    for(int y=0; y<buffer.BufferHt; y++)
    {
        for(int x=0; x< buffer.BufferWi; x++)
        {
            if (hasAlpha)
            {
                c.Set(image->GetRed(x,y),image->GetGreen(x,y),image->GetBlue(x,y),image->GetAlpha(x,y));
            }
            else
            {
                c.Set(image->GetRed(x,y),image->GetGreen(x,y),image->GetBlue(x,y), 255);
            }
            buffer.SetPixel(x, y, c);
        }
    }
}
