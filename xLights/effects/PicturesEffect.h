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
                           bool pixelOffsets, bool wrap_x);

    protected:
        virtual wxPanel *CreatePanel(wxWindow *parent);
    private:
        static void LoadPixelsFromTextFile(RenderBuffer &buffer, wxFile& debug, const wxString& filename);
};

#endif // PICTURESEFFECT_H
