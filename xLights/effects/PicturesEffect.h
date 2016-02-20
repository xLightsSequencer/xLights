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
        virtual bool CanBeRandom() {return false;}

        virtual void Render(Effect *effect, const SettingsMap &settings, RenderBuffer &buffer);

        static void Render(RenderBuffer &buffer,
                           const std::string & dirstr, const std::string &NewPictureName2,
                           float movementSpeed, float frameRateAdj,
                           int xc_adj, int yc_adj,
                           int xce_adj, int yce_adj,
                           int start_scale, int end_scale, bool scale_to_fit,
                           bool pixelOffsets, bool wrap_x);

        virtual AssistPanel *GetAssistPanel(wxWindow *parent, xLightsFrame* xl_frame);
        virtual bool HasAssistPanel() { return true; }
        virtual bool needToAdjustSettings(const std::string &version);
        virtual void adjustSettings(const std::string &version, Effect *effect);

    protected:
        virtual wxPanel *CreatePanel(wxWindow *parent);
    private:
        static void LoadPixelsFromTextFile(RenderBuffer &buffer, wxFile& debug, const wxString& filename);
};

#endif // PICTURESEFFECT_H
