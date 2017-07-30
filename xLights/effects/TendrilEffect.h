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

#define TENDRIL_MOVEMENT_MIN 0
#define TENDRIL_MOVEMENT_MAX 20

#define TENDRIL_THICKNESS_MIN 1
#define TENDRIL_THICKNESS_MAX 20

#define TENDRIL_MANUALX_MIN 0
#define TENDRIL_MANUALX_MAX 100

#define TENDRIL_MANUALY_MIN 0
#define TENDRIL_MANUALY_MAX 100

#define TENDRIL_OFFSETX_MIN -100
#define TENDRIL_OFFSETX_MAX 100

#define TENDRIL_OFFSETY_MIN -100
#define TENDRIL_OFFSETY_MAX 100

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
	size_t _thickness;
	int _width;
	int _height;

    std::list<TendrilNode*> _nodes;

	public:

	~ATendril();
	ATendril(float friction, int size, float dampening, float tension, float spring, const wxPoint& start, size_t maxx, size_t maxy);
	void Update(wxPoint* target);
	void Draw(PathDrawingContext* gc, xlColor colour, int thickness);
	wxPoint* LastLocation();
};

class Tendril
{
	std::list<ATendril*> _tendrils;
	int _width;
	int _height;

	public:

	~Tendril();
	Tendril(float friction, int trails, int size, float dampening, float tension, float springbase, float springincr, const wxPoint& start, size_t maxx, size_t maxy);
	void UpdateRandomMove(int tunemovement);
    void Update(wxPoint* target);
    void Update(int x, int y);
    void Draw(PathDrawingContext* gc, xlColor colour, int thickness);
};

class TendrilEffect : public RenderableEffect
{
    public:
        TendrilEffect(int id);
        virtual ~TendrilEffect();
        virtual void SetDefaultParameters(Model *cls) override;
        virtual void Render(Effect *effect, const SettingsMap &settings, RenderBuffer &buffer) override;
#ifdef LINUX
        virtual bool CanRenderOnBackgroundThread(Effect *effect, const SettingsMap &settings, RenderBuffer &buffer) override { return false;}
#endif

    protected:
        virtual wxPanel *CreatePanel(wxWindow *parent) override;
		virtual bool needToAdjustSettings(const std::string &version) override;
		virtual void adjustSettings(const std::string &version, Effect *effect, bool removeDefaults = true) override;
        int EncodeMovement(std::string movement);
        void Render(RenderBuffer &buffer,
            const std::string& movement, int tunemovement, int movementSpeed, int thickness,
            float friction, float dampening,
            float tension, int trails, int length, int xoffset, int yoffset, int manualx, int manualy);
};

#endif // PICTURESEFFECT_H
