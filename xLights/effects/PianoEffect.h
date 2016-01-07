#ifndef PIANOEFFECT_H
#define PIANOEFFECT_H

#include "RenderableEffect.h"

#include <string>

class PianoEffect : public RenderableEffect
{
    public:
        PianoEffect(int id);
        virtual ~PianoEffect();
        virtual bool CanBeRandom() {return false;}
    
        virtual void SetSequenceElements(SequenceElements *els);
    
        virtual void Render(Effect *effect, const SettingsMap &settings, RenderBuffer &buffer);

    protected:
        virtual wxPanel *CreatePanel(wxWindow *parent);
    private:
    
        void RenderPiano(RenderBuffer &buffer,
                         const std::string & Style, int NumKeys, int NumRows, const std::string & KeyPlacement, bool Clipping,
                         const std::string& CueFilename, const std::string& MapFilename, const std::string& ShapeFilename);

};

#endif // PIANOEFFECT_H
