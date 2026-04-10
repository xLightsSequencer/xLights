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

#include <vector>

#include "RenderableEffect.h"
#include "../utils/xlPoint.h"

struct KaleidoscopeEdge
{
    xlPoint _p1;
    xlPoint _p2;
    KaleidoscopeEdge(const xlPoint &p1, const xlPoint& p2) { _p1 = p1; _p2 = p2; }
};

struct KaleidoscopeVertex {
    double x;
    double y;
    KaleidoscopeVertex() :
        x(0), y(0) {
    }
    KaleidoscopeVertex(double x_, double y_) :
        x(x_), y(y_) {
    }
};

struct KaleidoscopeTriangle {
    KaleidoscopeVertex v[3];
};

class KaleidoscopeEffect : public RenderableEffect
{
    public:
        KaleidoscopeEffect(int id);
        virtual ~KaleidoscopeEffect();
        virtual bool CanBeRandom() override {return false;}
        virtual void Render(Effect *effect, const SettingsMap &settings, RenderBuffer &buffer) override;
        virtual bool SupportsLinearColorCurves(const SettingsMap &SettingsMap) const override { return false; }
        virtual std::list<std::string> CheckEffectSettings(const SettingsMap& settings, AudioManager* media, Model* model, Effect* eff, bool renderCache) override;

        // Cached from Kaleidoscope.json by OnMetadataLoaded().
        static std::string sTypeDefault;
        static int sXDefault;
        static int sXMin;
        static int sXMax;
        static int sYDefault;
        static int sYMin;
        static int sYMax;
        static int sSizeDefault;
        static int sSizeMin;
        static int sSizeMax;
        static int sRotationDefault;
        static int sRotationMin;
        static int sRotationMax;

    protected:
        virtual void OnMetadataLoaded() override;
        bool KaleidoscopeDone(const std::vector<std::vector<bool>>& current);
        std::pair<int, int> GetSourceLocation(int x, int y, const KaleidoscopeEdge& edge, int width, int height);
        void RenderNew(const std::string& type, int xCentre, int yCentre, int size, int rotation, RenderBuffer& buffer);
        static KaleidoscopeTriangle ComputeTriangle(const std::string& type, double cx, double cy, double size, double rotRad);
        static std::pair<int, int> MapToSourceTriangle(double px, double py, const KaleidoscopeTriangle& tri, int maxIter);
        static std::pair<int, int> MapToSourceNewSquare(double px, double py, double cx, double cy, double halfSize, double rotRad);
        static void ReflectPointAcrossLine(double& px, double& py, double lx1, double ly1, double lx2, double ly2);
        static double SignedDist(double px, double py, double lx1, double ly1, double lx2, double ly2);
        static double ReflectCoord(double v, double halfSize);
};
