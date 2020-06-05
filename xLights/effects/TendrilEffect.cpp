/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/smeighan/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/smeighan/xLights/blob/master/License.txt
 **************************************************************/

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

void TendrilEffect::adjustSettings(const std::string &version, Effect *effect, bool removeDefaults)
{
	SettingsMap &settings = effect->GetSettings();
	int movement = settings.GetInt("E_SLIDER_Tendril_Movement", -1);

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
        case 8:
            settings["E_CHOICE_Tendril_Movement"] = "Vert. Zig Zag Return";
            break;
        case 9:
            settings["E_CHOICE_Tendril_Movement"] = "Horiz. Zig Zag Return";
            break;
        }
	}

	// also give the base class a chance to adjust any settings
	if (RenderableEffect::needToAdjustSettings(version))
	{
		RenderableEffect::adjustSettings(version, effect, removeDefaults);
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
		if (p != nullptr)
		{
			delete p;
		}
	}
}

ATendril::ATendril(float friction, int size, float dampening, float tension, float spring, const wxPoint& start, size_t maxx, size_t maxy)
{
    _width = maxx;
    _height = maxy;
    _size = 60;
    if (size > 0)
    {
		_size = size;
	}
	_dampening = 0.25f;
	if (dampening >= 0)
	{
		_dampening = dampening;
	}
	_tension = 0.98f;
	if (tension >= 0)
	{
		_tension = tension;
	}
	_spring = 0;
	if (spring >= 0)
	{
		_spring = spring;
	}
	_friction = 0.5f;
	if (friction >= 0)
	{
		_friction = friction + ((float)rand())/(float)RAND_MAX * 0.01f - 0.005f;
	}
	else
	{
		_friction = _friction + ((float)rand())/(float)RAND_MAX * 0.01f - 0.005f;
	}

    _nodes.clear();
	for (size_t i = 0; i < _size; i++)
	{
		TendrilNode* node = new TendrilNode(start.x, start.y);
		if (node != nullptr)
		{
			_nodes.push_back(node);
		}
	}
}

void ATendril::Update(wxPoint* target)
{
	float spring = _spring;
	TendrilNode* node = _nodes.front();
	if (node != nullptr)
	{
		node->vx += (target->x - node->x) * spring;
		node->vy += (target->y - node->y) * spring;

		TendrilNode* prev = nullptr;
		for (std::list<TendrilNode*>::const_iterator ci = _nodes.begin(); ci != _nodes.end(); ++ci)
		{
			node = *ci;
			if (prev != nullptr)
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

void ATendril::Draw(PathDrawingContext* gc, xlColor colour, int thickness)
{
    wxColor c(colour);
    wxPen pen(c, thickness);
    gc->SetPen(pen);

    wxGraphicsPath path = gc->CreatePath();
    path.MoveToPoint(_nodes.front()->x, _nodes.front()->y);

    std::list<TendrilNode*>::const_iterator ci = _nodes.begin();
    ++ci; // move to second node

    std::list<TendrilNode*>::const_iterator ci_second_last = _nodes.end();
    --ci_second_last;
    --ci_second_last;

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

    TendrilNode* a = *ci;
    TendrilNode* b = *(++ci);
    path.AddQuadCurveToPoint(a->x, a->y, b->x, b->y);
    gc->StrokePath(path);
}

wxPoint* ATendril::LastLocation()
{
    TendrilNode *last = _nodes.back();
    if (last != nullptr)
    {
        return last->Point();
    }
    else
    {
        return nullptr;
    }
}

Tendril::~Tendril()
{
	while (_tendrils.size() != 0)
	{
		ATendril* p = _tendrils.front();
		_tendrils.pop_front();
		if (p != nullptr)
		{
			delete p;
		}
	}
}

Tendril::Tendril(float friction, int trails, int size, float dampening, float tension, float springbase, float springincr, const wxPoint& start, size_t maxx, size_t maxy)
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
		ATendril* at = new ATendril(friction, size, dampening, tension, aspring, start, maxx, maxy);
		if (at != nullptr)
		{
			_tendrils.push_back(at);
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
	if (t != nullptr)
	{
		wxPoint* current = t->LastLocation();

		if (current != nullptr)
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

void Tendril::Draw(PathDrawingContext* gc, xlColor colour, int thickness)
{
	for (std::list<ATendril*>::const_iterator ci = _tendrils.begin(); ci != _tendrils.end(); ++ci)
	{
		(*ci)->Draw(gc, colour, thickness);
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

void TendrilEffect::SetDefaultParameters() {
    TendrilPanel *tp = (TendrilPanel*)panel;
    if (tp == nullptr) {
        return;
    }

    tp->BitmapButton_Tendril_ManualXVC->SetActive(false);
    tp->BitmapButton_Tendril_ManualYVC->SetActive(false);
    tp->BitmapButton_Tendril_ThicknessVC->SetActive(false);
    tp->BitmapButton_Tendril_TuneMovementVC->SetActive(false);
    tp->BitmapButton_Tendril_XOffsetVC->SetActive(false);
    tp->BitmapButton_Tendril_YOffsetVC->SetActive(false);

    SetChoiceValue(tp->Choice_Tendril_Movement, "Random");
    SetSliderValue(tp->Slider_Tendril_TuneMovement, 10);
    SetSliderValue(tp->Slider_Tendril_Speed, 10);
    SetSliderValue(tp->Slider_Tendril_Thickness, 1);
    SetSliderValue(tp->Slider_Tendril_Friction, 10);
    SetSliderValue(tp->Slider_Tendril_Dampening, 10);
    SetSliderValue(tp->Slider_Tendril_Tension, 20);
    SetSliderValue(tp->Slider_Tendril_Trails, 1);
    SetSliderValue(tp->Slider_Tendril_Length, 60);
    SetSliderValue(tp->Slider_Tendril_XOffset, 0);
    SetSliderValue(tp->Slider_Tendril_YOffset, 0);
    SetSliderValue(tp->Slider_Tendril_ManualX, 0);
    SetSliderValue(tp->Slider_Tendril_ManualY, 0);
}

void TendrilEffect::Render(Effect *effect, SettingsMap &SettingsMap, RenderBuffer &buffer) {
    float oset = buffer.GetEffectTimeIntervalPosition();
    Render(buffer,
        SettingsMap.Get("CHOICE_Tendril_Movement", "Random"),
        GetValueCurveInt("Tendril_TuneMovement", 10, SettingsMap, oset, 0, 20, buffer.GetStartTimeMS(), buffer.GetEndTimeMS()),
        SettingsMap.GetInt("TEXTCTRL_Tendril_Speed", 10),
        GetValueCurveInt("Tendril_Thickness", 1, SettingsMap, oset, 1, 20, buffer.GetStartTimeMS(), buffer.GetEndTimeMS()),
        SettingsMap.GetFloat("TEXTCTRL_Tendril_Friction", 10) / 20 * 0.2 + 0.4, // 0.4->0.6 but on screen 0-20: def 0.5
        SettingsMap.GetFloat("TEXTCTRL_Tendril_Dampening", 10) / 20 * 0.5, // 0->0.5 but on screen 0-20: def 0.25
        SettingsMap.GetFloat("TEXTCTRL_Tendril_Tension", 20) / 39 * 0.039 + 0.96, // 0.960->0.999 but on screen 0->39: def 0.980
        SettingsMap.GetInt("TEXTCTRL_Tendril_Trails", 1),
        SettingsMap.GetInt("TEXTCTRL_Tendril_Length", 60),
        GetValueCurveInt("Tendril_XOffset", 0, SettingsMap, oset, -100, 100, buffer.GetStartTimeMS(), buffer.GetEndTimeMS()),
        GetValueCurveInt("Tendril_YOffset", 0, SettingsMap, oset, -100, 100, buffer.GetStartTimeMS(), buffer.GetEndTimeMS()),
        GetValueCurveInt("Tendril_ManualX", 0, SettingsMap, oset, 0, 100, buffer.GetStartTimeMS(), buffer.GetEndTimeMS()),
        GetValueCurveInt("Tendril_ManualY", 0, SettingsMap, oset, 0, 100, buffer.GetStartTimeMS(), buffer.GetEndTimeMS())
    );
}

class TendrilRenderCache : public EffectRenderCache {
public:
    TendrilRenderCache() { _tendril = nullptr; };
    virtual ~TendrilRenderCache() {
		if (_tendril != nullptr)
		{
			delete _tendril;
			_tendril = nullptr;
		}
	};

    int _mv1;
    int _mv2;
    int _mv3;
    int _mv4;
    Tendril* _tendril;
};

int TendrilEffect::EncodeMovement(std::string movement)
{
    if (movement == "Random")
    {
        return 1;
    }
    else if (movement == "Square")
    {
        return 2;
    }
    else if (movement == "Circle")
    {
        return 3;
    }
    else if (movement == "Horizontal Zig Zag")
    {
        return 4;
    }
    else if (movement == "Vertical Zig Zag")
    {
        return 5;
    }
    else if (movement == "Music Line")
    {
        return 6;
    }
    else if (movement == "Music Circle")
    {
        return 7;
    }
    else if (movement == "Vert. Zig Zag Return")
    {
        return 8;
    }
    else if (movement == "Horiz. Zig Zag Return")
    {
        return 9;
    }
    else if (movement == "Manual")
    {
        return 10;
    }

    return 1;
}

void TendrilEffect::Render(RenderBuffer &buffer, const std::string& movement,
    int tunemovement, int movementSpeed, int thickness,
    float friction, float dampening,
    float tension, int trails, int length, int xoffset, int yoffset, int manualx, int manualy)
{
    float oset = buffer.GetEffectTimeIntervalPosition();
    buffer.GetPathDrawingContext()->Clear();

    if (friction < 0.4f)
    {
        friction = 0.4f;
    }
    if (friction > 0.6f)
    {
        friction = 0.6f;
    }
    if (dampening < 0.0f)
    {
        dampening = 0.0f;
    }
    if (dampening > 0.5f)
    {
        dampening = 0.5f;
    }
    if (tension < 0.96f)
    {
        tension = 0.96f;
    }
    if (tension > 0.999f)
    {
        tension = 0.999f;
    }
    TendrilRenderCache *cache = (TendrilRenderCache*)buffer.infoCache[id];
    if (cache == nullptr) {
        cache = new TendrilRenderCache();
        buffer.infoCache[id] = cache;
    }

    int &_mv1 = cache->_mv1;
    int &_mv2 = cache->_mv2;
    int &_mv3 = cache->_mv3;
    int &_mv4 = cache->_mv4;
    Tendril* &_tendril = cache->_tendril;
    xlColor colour;
    buffer.GetMultiColorBlend(oset, false, colour);

    int nMovement = EncodeMovement(movement);

    int truexoffset = xoffset * buffer.BufferWi / 100;
    int trueyoffset = yoffset * buffer.BufferHt / 100;

    if (_tendril == nullptr || buffer.needToInit)
    {
        buffer.needToInit = false;
        wxPoint startmiddle(buffer.BufferWi / 2 + truexoffset / 2, buffer.BufferHt / 2 + trueyoffset / 2);
        wxPoint startmiddletop(buffer.BufferWi / 2 + truexoffset / 2, buffer.BufferHt + trueyoffset);
        wxPoint startmiddlebottom(buffer.BufferWi / 2 + truexoffset / 2, 0 + trueyoffset);
        wxPoint startbottomleft(0 + truexoffset, 0 + trueyoffset);
        wxPoint starttopleft(0 + truexoffset, buffer.BufferHt + trueyoffset);
        wxPoint startmiddleleft(0 + truexoffset, buffer.BufferHt / 2 + trueyoffset / 2);

        if (_tendril != nullptr)
        {
            delete _tendril;
            _tendril = nullptr;
        }

        switch (nMovement)
        {
        case 1:
            // random
            _tendril = new Tendril(friction, trails, length, dampening, tension, -1, -1, startmiddle, buffer.ModelBufferWi, buffer.ModelBufferHt);
            break;
        case 2:
            // corners
            _mv1 = 0 + truexoffset; // current x
            _mv2 = 0 + trueyoffset; // current y
            _mv3 = 0; // corner
            _mv4 = tunemovement; // movement amount
            if (_mv4 == 0)
            {
                _mv4 = 1;
            }
            _tendril = new Tendril(friction, trails, length, dampening, tension, -1, -1, startbottomleft, buffer.ModelBufferWi, buffer.ModelBufferHt);
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
            _tendril = new Tendril(friction, trails, length, dampening, tension, -1, -1, startmiddle, buffer.ModelBufferWi, buffer.ModelBufferHt);
            break;
        case 4:
            // horizontal zig zag
            _mv1 = 0 + trueyoffset; // current y
            _mv2 = (double)tunemovement * 1.5;
            if (_mv2 == 0)
            {
                _mv2 = 1;
            }
            _mv3 = 1; // direction
            _tendril = new Tendril(friction, trails, length, dampening, tension, -1, -1, startmiddlebottom, buffer.ModelBufferWi, buffer.ModelBufferHt);
            break;
        case 5:
            // vertical zig zag
            _mv1 = 0 + truexoffset; // current x
            _mv2 = (double)tunemovement * 1.5;
            _mv3 = 1; // direction
            _tendril = new Tendril(friction, trails, length, dampening, tension, -1, -1, startmiddleleft, buffer.ModelBufferWi, buffer.ModelBufferHt);
            break;
        case 6:
            // line movement based on music
            _mv1 = 0 + truexoffset; // current x
            _mv3 = tunemovement; // direction
            if (_mv3 < 1)
            {
                _mv3 = 1;
            }
            _tendril = new Tendril(friction, trails, length, dampening, tension, -1, -1, startbottomleft, buffer.ModelBufferWi, buffer.ModelBufferHt);
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
            _tendril = new Tendril(friction, trails, length, dampening, tension, -1, -1, startmiddle, buffer.ModelBufferWi, buffer.ModelBufferHt);
            break;
        case 9:
            // horizontal zig zag return
            _mv1 = 0; // current y
            _mv2 = (double)tunemovement * 1.5;
            if (_mv2 == 0)
            {
                _mv2 = 1;
            }
            _mv3 = 1; // direction
            _tendril = new Tendril(friction, trails, length, dampening, tension, -1, -1, startmiddlebottom, buffer.ModelBufferWi, buffer.ModelBufferHt);
            break;
        case 8:
            // vertical zig zag return
            _mv1 = 0; // current x
            _mv2 = (double)tunemovement * 1.5;
            _mv3 = 1; // direction
            _tendril = new Tendril(friction, trails, length, dampening, tension, -1, -1, startmiddleleft, buffer.ModelBufferWi, buffer.ModelBufferHt);
            break;
        case 10:
            _tendril = new Tendril(friction, trails, length, dampening, tension, -1, -1, wxPoint(manualx * buffer.BufferWi / 100, manualy * buffer.BufferHt / 100), buffer.ModelBufferWi, buffer.ModelBufferHt);
            break;
        }
    }

    // these are sensitive to the current buffer size
    switch (nMovement)
    {
    case 3:
        // circles
        _mv2 = std::min(buffer.BufferWi, buffer.BufferHt) / 2; // radius
    case 7:
        // circle movement based on music
        _mv2 = std::min(buffer.BufferWi, buffer.BufferHt) / 2; // max radius
    }

    const double PI = 3.141592653589793238463;
    int speed = 10 - movementSpeed;
    if (speed <= 0 || buffer.curPeriod % speed == 0)
    {
        switch (nMovement)
        {
        case 1:
            // random
            if (_tendril != nullptr)
            {
                _tendril->UpdateRandomMove(tunemovement);
            }
            break;
        case 2:
            // corners
            _mv4 = tunemovement; // movement amount
            switch (_mv3)
            {
            case 0:
                if (_mv4 == 0) _mv4 = 1;
                _mv1 += std::max(buffer.BufferWi / _mv4, 1);
                if (_mv1 >= buffer.BufferWi + truexoffset - buffer.BufferWi / _mv4)
                {
                    _mv3++;
                }
                break;
            case 1:
                if (_mv4 == 0) _mv4 = 1;
                _mv2 += std::max(buffer.BufferHt / _mv4, 1);
                if (_mv2 >= buffer.BufferHt + trueyoffset - buffer.BufferHt / _mv4)
                {
                    _mv3++;
                }
                break;
            case 2:
                if (_mv4 == 0) _mv4 = 1;
                _mv1 -= std::max(buffer.BufferWi / _mv4, 1);
                if (_mv1 <= truexoffset + buffer.BufferWi / _mv4)
                {
                    _mv3++;
                }
                break;
            case 3:
                if (_mv4 == 0) _mv4 = 1;
                _mv2 -= std::max(buffer.BufferHt / _mv4, 1);
                if (_mv2 <= trueyoffset + buffer.BufferHt / _mv4)
                {
                    _mv3 = 0;
                }
                break;
            }
            if (_tendril != nullptr)
            {
                _tendril->Update(_mv1, _mv2);
            }
            break;
        case 3:
        {
            // circles
            _mv3 = tunemovement * 3;
            _mv1 = _mv1 + _mv3;
            if (_mv3 > 360)
            {
                _mv3 = 0;
            }
            int x = sin((double)_mv1 / 360.0 * PI * 2.0) * (double)_mv2 + (double)buffer.BufferWi / 2.0 + truexoffset / 2;
            int y = cos((double)_mv1 / 360.0 * PI * 2.0) * (double)_mv2 + (double)buffer.BufferHt / 2.0 + trueyoffset / 2;
            if (_tendril != nullptr)
            {
                _tendril->Update(x, y);
            }
        }
        break;
        case 4:
        {
            // horizontal zig zag
            _mv2 = (double)tunemovement * 1.5;
            if (_mv2 == 0)
            {
                _mv2 = 1;
            }
            _mv1 = _mv1 + _mv3;
            int x = truexoffset + sin(std::max((double)buffer.BufferHt / (double)_mv2, 0.5) * PI * (double)_mv1 / (double)buffer.BufferHt) * (double)buffer.BufferWi / 2.0 + (double)buffer.BufferWi / 2.0;
            if (_mv1 >= trueyoffset + buffer.BufferHt || _mv1 <= 0 + trueyoffset)
            {
                _mv3 = _mv3 * -1;
            }
            if (_mv3 < 0)
            {
                x = buffer.BufferWi + truexoffset + truexoffset - x;
            }
            if (_tendril != nullptr)
            {
                _tendril->Update(x, _mv1);
            }
        }
        break;
        case 5:
        {
            // vertical zig zag
            _mv2 = (double)tunemovement * 1.5;
            _mv1 = _mv1 + _mv3;
            int y = trueyoffset + sin(std::max((double)buffer.BufferWi / (double)_mv2, 0.5) * PI * (double)_mv1 / (double)buffer.BufferWi) * (double)buffer.BufferHt / 2.0 + (double)buffer.BufferHt / 2.0;
            if (_mv1 >= truexoffset + buffer.BufferWi || _mv1 <= 0 + truexoffset)
            {
                _mv3 = _mv3 * -1;
            }
            if (_mv3 < 0)
            {
                y = buffer.BufferHt + trueyoffset + trueyoffset - y;
            }
            if (_tendril != nullptr)
            {
                _tendril->Update(_mv1, y);
            }
        }
        break;
        case 6:
        {
            // line movement based on music
            float f = 0.1f;
            if (buffer.GetMedia() != nullptr)
            {
                std::list<float> const * const p = buffer.GetMedia()->GetFrameData(buffer.curPeriod, FRAMEDATA_HIGH, "");
                if (p != nullptr)
                {
                    f = *p->cbegin();
                }
            }

            _mv1 = _mv1 + _mv3;
            if ((_mv1 < 0 + truexoffset && _mv3 < 0) || (_mv1 > buffer.BufferWi + truexoffset && _mv3 > 0))
            {
                _mv3 = _mv3 * -1;
            }

            if (_tendril != nullptr)
            {
                _tendril->Update(_mv1, trueyoffset + buffer.BufferHt * f);
            }
        }
        break;
        case 7:
        {
            // circle movement based on music
            _mv3 = tunemovement * 3;
            if (_mv3 < 1)
            {
                _mv3 = 1;
            }
            float f = 0.1f;
            if (buffer.GetMedia() != nullptr)
            {
                const std::list<float>* p = buffer.GetMedia()->GetFrameData(buffer.curPeriod, FRAMEDATA_HIGH, "");
                if (p != nullptr)
                {
                    f = *p->begin();
                }
            }

            _mv1 = _mv1 + _mv3;
            if (_mv3 > 360)
            {
                _mv3 = 0;
            }
            int x = sin((double)_mv1 / 360.0 * PI * 2.0) * (double)_mv2 * f * 2 + (double)buffer.BufferWi / 2.0 + truexoffset / 2;
            int y = cos((double)_mv1 / 360.0 * PI * 2.0) * (double)_mv2 * f * 2 + (double)buffer.BufferHt / 2.0 + trueyoffset / 2;
            if (_tendril != nullptr)
            {
                _tendril->Update(x, y);
            }
        }
        break;
        case 9:
        {
            // vert zig zag return
            _mv2 = (double)tunemovement * 1.5;
            if (_mv2 == 0)
            {
                _mv2 = 1;
            }
            _mv1 = _mv1 + _mv3;
            int x = buffer.BufferWi / 2 + truexoffset / 2;
            if (_mv3 > 0)
            {
                x = truexoffset + sin(std::max((double)buffer.BufferHt / (double)_mv2, 0.5) * PI * (double)_mv1 / (double)buffer.BufferHt) * (double)buffer.BufferWi / 2.0 + (double)buffer.BufferWi / 2.0;
            }
            if (_mv1 >= buffer.BufferHt || _mv1 <= 0)
            {
                _mv3 = _mv3 * -1;
            }
            if (_tendril != nullptr)
            {
                _tendril->Update(x, _mv1 + trueyoffset);
            }
        }
        break;
        case 8:
        {
            // horizontal zig zag return
            _mv2 = (double)tunemovement * 1.5;
            _mv1 = _mv1 + _mv3;
            int y = buffer.BufferHt / 2 + trueyoffset / 2;
            if (_mv3 > 0)
            {
                y = trueyoffset + sin(std::max((double)buffer.BufferWi / (double)_mv2, 0.5) * PI * (double)_mv1 / (double)buffer.BufferWi) * (double)buffer.BufferHt / 2.0 + (double)buffer.BufferHt / 2.0;
            }
            if (_mv1 >= buffer.BufferWi || _mv1 <= 0)
            {
                _mv3 = _mv3 * -1;
            }
            if (_tendril != nullptr)
            {
                _tendril->Update(_mv1 + truexoffset, y);
            }
        }
        break;
        case 10:
        {
            // manual
            if (_tendril != nullptr)
            {
                _tendril->Update(manualx * buffer.BufferWi / 100 + truexoffset, manualy * buffer.BufferHt / 100 + trueyoffset);
            }
        }
        break;
        }
    }

    if (_tendril != nullptr)
    {
        _tendril->Draw(buffer.GetPathDrawingContext(), colour, thickness);
    }
    wxImage * image = buffer.GetPathDrawingContext()->FlushAndGetImage();
    bool hasAlpha = image->HasAlpha();

    xlColor c;
    for (int y = 0; y < buffer.BufferHt; y++)
    {
        for (int x = 0; x < buffer.BufferWi; x++)
        {
            if (hasAlpha)
            {
                c.Set(image->GetRed(x, y), image->GetGreen(x, y), image->GetBlue(x, y), image->GetAlpha(x, y));
            }
            else
            {
                c.Set(image->GetRed(x, y), image->GetGreen(x, y), image->GetBlue(x, y), 255);
            }
            buffer.SetPixel(x, y, c);
        }
    }
}
