#ifndef TENDRILEFFECT_H
#define TENDRILEFFECT_H

#include "RenderableEffect.h"
#include "../RenderBuffer.h"
#include <string>
#include <list>
#include <wx/gdicmn.h>
#include <wx/colour.h>
#include <wx/dcmemory.h>
class wxString;

class TendrilNode
{
    public:
    float x;
    float y;
    float vx;
    float vy;

    TendrilNode(float x_, float y_);
    wxPoint* Point();
};

class ATendril
{
    float _friction;
	size_t _size;
	float _dampening;
	float _tension;
	float _spring;
	xlColor _colour;
	size_t _thickness;
	size_t _width;
	size_t _height;

    std::list<TendrilNode*> _nodes;

	public:

	~ATendril();
	ATendril(float friction, size_t size, float dampening, float tension, float spring, wxPoint* start, xlColor colour, size_t thickness, size_t maxx, size_t maxy);
	void Update(wxPoint* target);
	void Draw(PathDrawingContext* gc);
	wxPoint* LastLocation();
};

class Tendril
{
	std::list<ATendril*> _tendrils;
	int _width;
	int _height;

	public:

	~Tendril();
	Tendril(float friction, int trails, int size, float dampening, float tension, float springbase, float springincr, wxPoint* start, xlColor colour, int thickness, int maxx, int maxy);
	void UpdateRandomMove(int tunemovement);
    void Update(wxPoint* target);
    void Update(int x, int y);
    void Draw(PathDrawingContext* gc);
};

class TendrilEffect : public RenderableEffect
{
    public:
        TendrilEffect(int id);
        virtual ~TendrilEffect();
        virtual void Render(Effect *effect, const SettingsMap &settings, RenderBuffer &buffer) override;
        void Render(RenderBuffer &buffer,
					const std::string& movement, int tunemovement, int movementSpeed, int thickness,
                    float friction, float dampening,
                    float tension, int trails, int length, int xoffset, int yoffset);
#ifdef LINUX
        virtual bool CanRenderOnBackgroundThread(Effect *effect, const SettingsMap &settings, RenderBuffer &buffer) override { return false;}
#endif

    protected:
        virtual wxPanel *CreatePanel(wxWindow *parent) override;
		virtual bool needToAdjustSettings(const std::string &version) override;
		virtual void adjustSettings(const std::string &version, Effect *effect) override;
};

#endif // PICTURESEFFECT_H
