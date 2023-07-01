#pragma once

/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/smeighan/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/smeighan/xLights/blob/master/License.txt
 **************************************************************/

#include "RenderableEffect.h"

#define PICTURES_XC_MIN -100
#define PICTURES_XC_MAX 100

#define PICTURES_YC_MIN -100
#define PICTURES_YC_MAX 100

#include <string>
class wxString;
class wxFile;

class PicturesEffect : public RenderableEffect
{
    public:
        PicturesEffect(int id);
        virtual ~PicturesEffect();
        virtual bool CanBeRandom() override {return false;}
        virtual void SetDefaultParameters() override;
        virtual void Render(Effect *effect, const SettingsMap &settings, RenderBuffer &buffer) override;
        static void Render(RenderBuffer &buffer,
                           const std::string & dirstr, const std::string &NewPictureName2,
                           float movementSpeed, float frameRateAdj,
                           int xc_adj, int yc_adj,
                           int xce_adj, int yce_adj,
                           int start_scale, int end_scale, const std::string& scale_to_fit,
                           bool pixelOffsets, bool wrap_x, bool shimmer, bool loopGIF, bool suppressGIFBackground, bool transparentBlack, int transparentBlackLevel);

        virtual AssistPanel *GetAssistPanel(wxWindow *parent, xLightsFrame* xl_frame) override;
        virtual bool HasAssistPanel() override { return true; }
        virtual bool needToAdjustSettings(const std::string &version) override;
        virtual void adjustSettings(const std::string &version, Effect *effect, bool removeDefaults = true) override;
        virtual std::list<std::string> CheckEffectSettings(const SettingsMap& settings, AudioManager* media, Model* model, Effect* eff, bool renderCache) override;
        virtual std::list<std::string> GetFileReferences(Model* model, const SettingsMap &SettingsMap) const override;
        virtual bool CleanupFileLocations(xLightsFrame* frame, SettingsMap &SettingsMap) override;
        static bool IsPictureFile(std::string filename);
        virtual bool SupportsRenderCache(const SettingsMap& settings) const override { return true; }

    
        virtual double GetSettingVCMin(const std::string& name) const override {
            if (name == "E_VALUECURVE_PicturesXC")
                return PICTURES_XC_MIN;
            if (name == "E_VALUECURVE_PicturesYC")
                return PICTURES_YC_MIN;
            return RenderableEffect::GetSettingVCMin(name);
        }

        virtual double GetSettingVCMax(const std::string& name) const override {
            if (name == "E_VALUECURVE_PicturesXC")
                return PICTURES_XC_MAX;
            if (name == "E_VALUECURVE_PicturesYC")
                return PICTURES_YC_MAX;
            return RenderableEffect::GetSettingVCMax(name);
        }
    
    protected:
        virtual xlEffectPanel *CreatePanel(wxWindow *parent) override;
    private:
        static void LoadPixelsFromTextFile(RenderBuffer &buffer, wxFile& debug, const wxString& filename);
        static void SetTransparentBlackPixel(RenderBuffer &buffer, int x, int y, xlColor c, bool transparentBlack, int transparentBlackLevel);
        static void SetTransparentBlackPixel(RenderBuffer &buffer, int x, int y, xlColor c, bool wrap, bool transparentBlack, int transparentBlackLevel);
};
