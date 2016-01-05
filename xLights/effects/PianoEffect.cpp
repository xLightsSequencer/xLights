#include "PianoEffect.h"

#include "PianoPanel.h"
#include <wx/filename.h>
#include "../sequencer/SequenceElements.h"

#include "../sequencer/Effect.h"
#include "../RenderBuffer.h"
#include "../UtilClasses.h"

PianoEffect::PianoEffect(int id) : RenderableEffect(id, "Piano")
{
    //ctor
}

PianoEffect::~PianoEffect()
{
    //dtor
}


void PianoEffect::SetSequenceElements(SequenceElements *els) {
    mSequenceElements = els;
    if (panel == nullptr) {
        return;
    }
    wxFileName fn(els->GetFileName());
    
    ((PianoPanel*)panel)->defaultDir = fn.GetPath();
}

wxPanel *PianoEffect::CreatePanel(wxWindow *parent) {
    return new PianoPanel(parent);
}

void PianoEffect::Render(Effect *effect, const SettingsMap &SettingsMap, RenderBuffer &buffer) {
}
