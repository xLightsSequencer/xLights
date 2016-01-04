#include "GlediatorEffect.h"
#include "GlediatorPanel.h"

#include "../sequencer/SequenceElements.h"

#include <wx/filename.h>

GlediatorEffect::GlediatorEffect(int id) : RenderableEffect(id, "Glediator")
{
    //ctor
}

GlediatorEffect::~GlediatorEffect()
{
    //dtor
}

void GlediatorEffect::SetSequenceElements(SequenceElements *els) {
    mSequenceElements = els;
    if (panel == nullptr) {
        return;
    }
    wxFileName fn(els->GetFileName());
    
    ((GlediatorPanel*)panel)->defaultDir = fn.GetPath();
}

wxPanel *GlediatorEffect::CreatePanel(wxWindow *parent) {
    return new GlediatorPanel(parent);
}
