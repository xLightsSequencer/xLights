#pragma once

/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include "RenderableEffect.h"
#include "../render/RenderBuffer.h"
#include <string>
#include <list>
#include "../utils/xlPoint.h"

class TendrilNode
{
    public:
    float x;
    float y;
    float vx;
    float vy;

    TendrilNode(float x_, float y_);
    xlPoint Point();
};

class ATendril
{
    float _friction;
	size_t _size;
	float _dampening;
	float _tension;
	float _spring;
	size_t _thickness;
    int _lastWidth = -1;
    int _lastHeight = -1;

    std::list<TendrilNode*> _nodes;

	public:

	~ATendril();
	ATendril(float friction, int size, float dampening, float tension, float spring, const xlPoint& start);
    void Update(const xlPoint& target, int tunemovement, int width, int height);
	void Draw(RenderBuffer& buffer, xlColor colour, int thickness);
	xlPoint LastLocation();
};

class Tendril
{
	std::list<ATendril*> _tendrils;

	public:

	~Tendril();
	Tendril(float friction, int trails, int size, float dampening, float tension, float springbase, float springincr, const xlPoint& start);
	void UpdateRandomMove(int tunemovement, int width, int height);
    void Update(const xlPoint& target, int tunemovement, size_t width, size_t height);
    void Update(int x, int y, int tunemovement, size_t width, size_t height);
    void Draw(RenderBuffer& buffer, xlColor colour, int thickness);
};

class TendrilEffect : public RenderableEffect
{
public:
    TendrilEffect(int id);
    virtual ~TendrilEffect();
    virtual void Render(Effect* effect, const SettingsMap& settings, RenderBuffer& buffer) override;
    virtual bool AppropriateOnNodes() const override
    {
        return false;
    }
    virtual bool SupportsRenderCache(const SettingsMap& settings) const override
    {
        return true;
    }

    // Cached from Tendril.json by OnMetadataLoaded().
    static std::string sMovementDefault;
    static int sTuneMovementDefault;
    static int sTuneMovementMin;
    static int sTuneMovementMax;
    static int sThicknessDefault;
    static int sThicknessMin;
    static int sThicknessMax;
    static int sFrictionDefault;
    static int sDampeningDefault;
    static int sTensionDefault;
    static int sTrailsDefault;
    static int sLengthDefault;
    static int sSpeedDefault;
    static int sXOffsetDefault;
    static int sXOffsetMin;
    static int sXOffsetMax;
    static int sYOffsetDefault;
    static int sYOffsetMin;
    static int sYOffsetMax;
    static int sManualXDefault;
    static int sManualXMin;
    static int sManualXMax;
    static int sManualYDefault;
    static int sManualYMin;
    static int sManualYMax;

protected:
    virtual void OnMetadataLoaded() override;
    int EncodeMovement(std::string movement);
    void Render(RenderBuffer& buffer,
                const std::string& movement, int tunemovement, int movementSpeed, int thickness,
                float friction, float dampening,
                float tension, int trails, int length, int xoffset, int yoffset, int manualx, int manualy);
};
