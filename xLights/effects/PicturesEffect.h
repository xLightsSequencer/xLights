#ifndef PICTURESEFFECT_H
#define PICTURESEFFECT_H

#include "RenderableEffect.h"

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
        virtual void Render(Effect *effect, SettingsMap &settings, RenderBuffer &buffer) override;
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
        virtual std::list<std::string> CheckEffectSettings(const SettingsMap& settings, AudioManager* media, Model* model, Effect* eff) override;
        virtual std::list<std::string> GetFileReferences(const SettingsMap &SettingsMap) override;
        static bool IsPictureFile(std::string filename);
        virtual bool SupportsRenderCache() const override { return true; }

    protected:
        virtual wxPanel *CreatePanel(wxWindow *parent) override;
    private:
        static void LoadPixelsFromTextFile(RenderBuffer &buffer, wxFile& debug, const wxString& filename);
        static void SetTransparentBlackPixel(RenderBuffer &buffer, int x, int y, xlColor c, bool transparentBlack, int transparentBlackLevel);
        static void SetTransparentBlackPixel(RenderBuffer &buffer, int x, int y, xlColor c, bool wrap, bool transparentBlack, int transparentBlackLevel);
};

#endif // PICTURESEFFECT_H
