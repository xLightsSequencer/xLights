#ifndef TEXTEFFECT_H
#define TEXTEFFECT_H

#include "RenderableEffect.h"

class wxString;
class DrawingContext;

class TextEffect : public RenderableEffect
{
    public:
        TextEffect(int id);
        virtual ~TextEffect();
        virtual void SetDefaultParameters(ModelClass *cls);
    
        virtual bool CanRenderOnBackgroundThread();
        virtual void Render(Effect *effect, const SettingsMap &settings, RenderBuffer &buffer);
        virtual bool CanBeRandom() {return false;}

    protected:
        virtual wxPanel *CreatePanel(wxWindow *parent);
    private:
    
        void RenderTextLine(RenderBuffer &buffer,
                            DrawingContext* dc, int idx, const wxString& Line_orig, int dir,
                            bool center, int Effect, int Countdown, bool WantRender, int tspeed,
                            int startx, int starty, int endx, int endy,
                            bool isPixelBased);

};

#endif // TEXTEFFECT_H
