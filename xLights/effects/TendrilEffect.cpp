#include "TendrilEffect.h"
#include "TendrilPanel.h"

#include "../sequencer/Effect.h"
#include "../RenderBuffer.h"
#include "../UtilClasses.h"

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
	while (_nodes.front() != NULL)
	{
		delete _nodes.front();
		_nodes.pop_front();
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
		_nodes.push_back(node);
	}
}

void ATendril::Update(wxPoint* target)
{
	float spring = _spring;
	TendrilNode* node = _nodes.front();
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

void ATendril::Draw(wxGraphicsContext* gc)
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
	while (_tendrils.front() != NULL)
	{
		delete _tendrils.front();
		_tendrils.pop_front();
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
		_tendrils.push_back(new ATendril(friction, size, dampening, tension, aspring, start, colour, thickness, maxx, maxy));
	}
}

#define min(a, b) (((a) < (b)) ? (a) : (b))
#define max(a, b) (((a) > (b)) ? (a) : (b))

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

	wxPoint* current = _tendrils.front()->LastLocation();

	int realminmovex = minmovex;
	if (minmovex < 0)
	{
		realminmovex = -1 * min(current->x, minmovex * -1);
	}
	int realmaxmovex = maxmovex;
	if (maxmovex > 0)
	{
		realmaxmovex = min(maxx - current->x, maxmovex);
	}
	int realminmovey = minmovey;
	if (minmovey < 0)
	{
		realminmovey = -1 * min(current->y, minmovey * -1);
	}
	int realmaxmovey = maxmovey;
	if (maxmovey > 0)
	{
		realmaxmovey = min(maxy - current->y, maxmovey);
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

void Tendril::Draw(wxGraphicsContext* gc)
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

wxPanel *TendrilEffect::CreatePanel(wxWindow *parent) {
    return new TendrilPanel(parent);
}

void TendrilEffect::Render(Effect *effect, const SettingsMap &SettingsMap, RenderBuffer &buffer) {
    Render(buffer,
           SettingsMap.GetInt("TEXTCTRL_Tendril_Movement", 1),
           SettingsMap.GetInt("TEXTCTRL_Tendril_TuneMovement", 10),
           SettingsMap.GetInt("TEXTCTRL_Tendril_Speed", 100),
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
    virtual ~TendrilRenderCache() {};

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

void TendrilEffect::Render(RenderBuffer &buffer, int movement,
                            int tunemovement, int movementSpeed, int thickness,
                           float friction, float dampening,
                           float tension, int trails, int length)
{
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
    xlColour &_colour = cache->_colour;
    Tendril* &_tendril = cache->_tendril;
    xlColour color1;
    buffer.palette.GetColor(0, color1);

    if (_tendril == NULL ||
        _movement != movement ||
        _tunemovement != tunemovement ||
        _thickness != thickness ||
        _friction != friction ||
        _dampening != dampening ||
        _tension != tension ||
        _trails != trails ||
        _length != length ||
        _colour != color1)
    {
        if (_tendril != NULL)
        {
            delete _tendril;
        }
        _thickness = thickness;
        _friction = friction;
        _dampening = dampening;
        _tension = tension;
        _trails = trails;
        _length = length;
        _colour = color1;
        wxPoint start(buffer.BufferWi/2, buffer.BufferHt/2);
        _tendril = new Tendril(_friction, _trails, _length, _dampening, _tension, -1, -1, &start, _colour, _thickness, buffer.BufferWi, buffer.BufferHt);

        if (_movement != movement || _tunemovement != tunemovement)
        {
            switch(movement)
            {
            case 1:
                // random
                // no initialisation
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
                break;
            case 3:
                // circles
                _mv1 = 0; // radians
                _mv2 = min(buffer.BufferWi, buffer.BufferHt) / 2; // radius
                _mv3 = tunemovement * 3;
                if (_mv3 == 0)
                {
                    _mv3 = 1;
                }
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
                break;
            case 5:
                // vertical zig zag
                _mv1 = 0; // current x
                _mv2 = (double)tunemovement * 1.5;
                _mv3 = 1; // direction
                break;
            }
        }
        _movement = movement;
        _tunemovement = tunemovement;
    }

    const double PI  = 3.141592653589793238463;
    int speed = 100 - movementSpeed;
    if (speed <= 0 || buffer.curPeriod % speed == 0)
    {
        switch(_movement)
        {
            case 1:
                _tendril->UpdateRandomMove(_tunemovement);
                break;
            case 2:
                switch(_mv3)
                {
                case 0:
                    _mv1+=max(buffer.BufferWi/_mv4,1);
                    if (_mv1 >= buffer.BufferWi - buffer.BufferWi / _mv4)
                    {
                        _mv3++;
                    }
                    break;
                case 1:
                    _mv2+=max(buffer.BufferHt/_mv4,1);
                    if (_mv2 >= buffer.BufferHt - buffer.BufferHt / _mv4)
                    {
                        _mv3++;
                    }
                    break;
                case 2:
                    _mv1-=max(buffer.BufferWi/_mv4,1);
                    if (_mv1 <= buffer.BufferWi / _mv4)
                    {
                        _mv3++;
                    }
                    break;
                case 3:
                    _mv2-=max(buffer.BufferHt/_mv4,1);
                    if (_mv2 <= buffer.BufferHt / _mv4)
                    {
                        _mv3 = 0;
                    }
                    break;
                }
                _tendril->Update(_mv1, _mv2);
                break;
            case 3:
                _mv1++;
                if (_mv1 > _mv3)
                {
                    _mv1 = 0;
                }
                {
                    int x = (double)_mv2 * cos((double)_mv1 * 2.0 * PI / (double)_mv3) + (double)buffer.BufferWi / 2.0;
                    int y = (double)_mv2 * sin((double)_mv1 * 2.0 * PI / (double)_mv3) + (double)buffer.BufferHt / 2.0;
                    _tendril->Update(x, y);
                }
                break;
            case 4:
                {
                    _mv1 = _mv1 + _mv3;
                    int x = sin(max((double)buffer.BufferHt / (double)_mv2, 0.5) * PI * (double)_mv1 / (double)buffer.BufferHt) * (double)buffer.BufferWi / 2.0 + (double)buffer.BufferWi / 2.0;
                    if (_mv1 >= buffer.BufferHt || _mv1 <= 0)
                    {
                        _mv3 = _mv3 * -1;
                    }
                    if (_mv3 < 0)
                    {
                        x = buffer.BufferWi - x;
                    }
                    _tendril->Update(x, _mv1);
                }
                break;
            case 5:
                {
                    _mv1 = _mv1 + _mv3;
                    int y = sin(max((double)buffer.BufferWi / (double)_mv2, 0.5) * PI * (double)_mv1 / (double)buffer.BufferWi) * (double)buffer.BufferHt / 2.0 + (double)buffer.BufferHt / 2.0;
                    if (_mv1 >= buffer.BufferWi || _mv1 <= 0)
                    {
                        _mv3 = _mv3 * -1;
                    }
                    if (_mv3 < 0)
                    {
                        y = buffer.BufferHt - y;
                    }
                    _tendril->Update(_mv1, y);
                }
                break;
        }
    }

    wxBitmap bmp;
    bmp.Create(buffer.BufferWi, buffer.BufferHt);

    wxMemoryDC dc;
    dc.SelectObject(bmp);
    wxGraphicsContext* gc = wxGraphicsContext::Create(dc);
    _tendril->Draw(gc);
    dc.SelectObject(wxNullBitmap);

    wxImage image = bmp.ConvertToImage();
    bool hasAlpha = image.HasAlpha();

    xlColor c;
    for(int y=0; y<buffer.BufferHt; y++)
    {
        for(int x=0; x< buffer.BufferWi; x++)
        {
            if (hasAlpha)
            {
                c.Set(image.GetRed(x,y),image.GetGreen(x,y),image.GetBlue(x,y),image.GetAlpha(x,y));
            }
            else
            {
                c.Set(image.GetRed(x,y),image.GetGreen(x,y),image.GetBlue(x,y), 255);
            }
            buffer.SetPixel(x, y, c);
        }
    }
}
