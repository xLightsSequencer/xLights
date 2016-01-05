#include "FacesEffect.h"
#include "FacesPanel.h"
#include "../ModelClass.h"
#include "../sequencer/SequenceElements.h"

#include "../sequencer/Effect.h"
#include "../RenderBuffer.h"
#include "../UtilClasses.h"


FacesEffect::FacesEffect(int id) : RenderableEffect(id, "Faces")
{
    //ctor
}

FacesEffect::~FacesEffect()
{
    //dtor
}

void FacesEffect::SetDefaultParameters(ModelClass *cls) {
    FacesPanel *fp = (FacesPanel*)panel;
    if (fp == nullptr) {
        return;
    }
    fp->Choice_Faces_TimingTrack->Clear();
    fp->Face_FaceDefinitonChoice->Clear();
    if (mSequenceElements == nullptr) {
        return;
    }
    for (int i = 0; i < mSequenceElements->GetElementCount(); i++) {
        if (mSequenceElements->GetElement(i)->GetEffectLayerCount() == 3
            && mSequenceElements->GetElement(i)->GetType() == "timing") {
            fp->Choice_Faces_TimingTrack->Append(mSequenceElements->GetElement(i)->GetName());
        }
    }
    bool addRender = true;
    if (cls != nullptr) {
        for (std::map<wxString, std::map<wxString, wxString> >::iterator it = cls->faceInfo.begin(); it != cls->faceInfo.end(); it++) {
            fp->Face_FaceDefinitonChoice->Append(it->first);
            if (it->second["Type"] == "Coro" || it->second["Type"] == "SingleNode" || it->second["Type"] == "NodeRange") {
                addRender = false;
            }
        }
    }
    if (fp->Face_FaceDefinitonChoice->GetCount() == 0) {
        fp->Face_FaceDefinitonChoice->Append("Default");
        addRender = false;
    }
    if (addRender) {
        fp->Face_FaceDefinitonChoice->Append("Rendered");
    }
    fp->Face_FaceDefinitonChoice->SetSelection(0);
}

wxPanel *FacesEffect::CreatePanel(wxWindow *parent) {
    return new FacesPanel(parent);
}

void FacesEffect::Render(Effect *effect, const SettingsMap &SettingsMap, RenderBuffer &buffer) {
}

