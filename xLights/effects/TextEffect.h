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
        virtual void SetDefaultParameters(Model *cls) override;
    
        virtual void Render(Effect *effect, const SettingsMap &settings, RenderBuffer &buffer) override;
        virtual bool CanBeRandom() override {return false;}

    protected:
        virtual wxPanel *CreatePanel(wxWindow *parent) override;
    private:
    
        void RenderTextLine(RenderBuffer &buffer,
                            DrawingContext* dc, int idx, const wxString& Line_orig, int dir,
                            bool center, int Effect, int Countdown, bool WantRender, int tspeed,
                            int startx, int starty, int endx, int endy,
                            bool isPixelBased);

};

#endif // TEXTEFFECT_H
