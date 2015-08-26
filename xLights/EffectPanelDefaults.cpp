

#include "EffectsPanel.h"

#include "SequenceElements.h"
#include "ModelClass.h"


void EnableControl(wxWindow *w, int id, bool e) {
    wxWindow *c = w->FindWindowById(id);
    if (c) c->Enable(e);
}

void SetCheckboxValue(wxWindow *w, int id, bool b) {
    wxCheckBox *c = (wxCheckBox*)w->FindWindowById(id);
    c->SetValue(b);
    wxCommandEvent evt(wxEVT_COMMAND_CHECKBOX_CLICKED, id);
    evt.SetEventObject(c);
    evt.SetInt(b);
    c->ProcessWindowEvent(evt);
}

void EffectsPanel::SetDefaultEffectValues(ModelClass *cls, const wxString &name) {
    //NOTE: the settings loop after this section does not initialize controls.
    //For controls that have been added recently, an older version of the XML file will cause initial settings to be incorrect.
    //A loop needs to be added to initialize the wx controls to a predictable value.
    //For now, a few recently added controls are explicitly initialized here:
    //(not sure if there will be side effects to using a full loop) -DJ
    
    if ("Color Wash" == name) {
        SetCheckboxValue(this, ID_CHECKBOX_ColorWash_EntireModel, true);
    } else if ("Text" == name) {
        SetCheckboxValue(this, ID_CHECKBOX_TextToCenter1, false);
        SetCheckboxValue(this, ID_CHECKBOX_TextToCenter2, false);
        SetCheckboxValue(this, ID_CHECKBOX_TextToCenter3, false);
        SetCheckboxValue(this, ID_CHECKBOX_TextToCenter4, false);
    } else if ("Faces" == name ) {
        Choice_Faces_TimingTrack->Clear();
        for (int i = 0; i < mSequenceElements->GetElementCount(); i++) {
            if (mSequenceElements->GetElement(i)->GetEffectLayerCount() == 3
                && mSequenceElements->GetElement(i)->GetType() == "timing") {
                Choice_Faces_TimingTrack->Append(mSequenceElements->GetElement(i)->GetName());
            }
        }
    }
    Face_FaceDefinitonChoice->Clear();
    bool addRender = true;
    if (cls != nullptr) {
        for (std::map<wxString, std::map<wxString, wxString> >::iterator it = cls->faceInfo.begin(); it != cls->faceInfo.end(); it++) {
            Face_FaceDefinitonChoice->Append(it->first);
            if (it->first == "Coro" || it->first == "SingleNode" || it->first == "NodeRange") {
                addRender = false;
            }
        }
    }
    if (Face_FaceDefinitonChoice->GetCount() == 0) {
        Face_FaceDefinitonChoice->Append("Default");
        addRender = false;
    }
    if (addRender) {
        Face_FaceDefinitonChoice->Append("Rendered");
    }
    Face_FaceDefinitonChoice->SetSelection(0);
}


void EffectsPanel::OnChoicePicturesDirectionSelect(wxCommandEvent& event)
{
    bool enable = "vector" == Choice_Pictures_Direction->GetStringSelection();
    EnableControl(Choice_Pictures_Direction->GetParent(), IDD_TEXTCTRL_PicturesEndXC, enable);
    EnableControl(Choice_Pictures_Direction->GetParent(), IDD_TEXTCTRL_PicturesEndYC, enable);
    EnableControl(Choice_Pictures_Direction->GetParent(), ID_SLIDER_PicturesEndXC, enable);
    EnableControl(Choice_Pictures_Direction->GetParent(), ID_SLIDER_PicturesEndYC, enable);
    PictureEndPositionPanel->Enable(enable);
}


void EffectsPanel::OnColorWashEntireModelClicked(wxCommandEvent& event)
{
    bool on = ((wxCheckBox*)event.GetEventObject())->GetValue();
    EnableControl(this, IDD_NOTEBOOK_ColorWashRect, !on);
}

void EffectsPanel::OnChoice_Morph_QuickSetSelect(wxCommandEvent& event)
{
    wxString quickset_choice = Choice_Morph_QuickSet->GetStringSelection();
    if( quickset_choice == "Full Sweep Up" )
    {
        TextCtrl_Morph_Start_X1->SetValue("0");
        TextCtrl_Morph_Start_X2->SetValue("100");
        TextCtrl_Morph_End_X1->SetValue("0");
        TextCtrl_Morph_End_X2->SetValue("100");
        TextCtrl_Morph_Start_Y1->SetValue("0");
        TextCtrl_Morph_Start_Y2->SetValue("0");
        TextCtrl_Morph_End_Y1->SetValue("100");
        TextCtrl_Morph_End_Y2->SetValue("100");
    }
    else if( quickset_choice == "Full Sweep Down" )
    {
        TextCtrl_Morph_Start_X1->SetValue("0");
        TextCtrl_Morph_Start_X2->SetValue("100");
        TextCtrl_Morph_End_X1->SetValue("0");
        TextCtrl_Morph_End_X2->SetValue("100");
        TextCtrl_Morph_Start_Y1->SetValue("100");
        TextCtrl_Morph_Start_Y2->SetValue("100");
        TextCtrl_Morph_End_Y1->SetValue("0");
        TextCtrl_Morph_End_Y2->SetValue("0");
    }
    else if( quickset_choice == "Full Sweep Left" )
    {
        TextCtrl_Morph_Start_X1->SetValue("100");
        TextCtrl_Morph_Start_X2->SetValue("100");
        TextCtrl_Morph_End_X1->SetValue("0");
        TextCtrl_Morph_End_X2->SetValue("0");
        TextCtrl_Morph_Start_Y1->SetValue("0");
        TextCtrl_Morph_Start_Y2->SetValue("100");
        TextCtrl_Morph_End_Y1->SetValue("0");
        TextCtrl_Morph_End_Y2->SetValue("100");
    }
    else if( quickset_choice == "Full Sweep Right" )
    {
        TextCtrl_Morph_Start_X1->SetValue("0");
        TextCtrl_Morph_Start_X2->SetValue("0");
        TextCtrl_Morph_End_X1->SetValue("100");
        TextCtrl_Morph_End_X2->SetValue("100");
        TextCtrl_Morph_Start_Y1->SetValue("0");
        TextCtrl_Morph_Start_Y2->SetValue("100");
        TextCtrl_Morph_End_Y1->SetValue("0");
        TextCtrl_Morph_End_Y2->SetValue("100");
    }
    else if( quickset_choice == "Single Sweep Up" )
    {
        TextCtrl_Morph_Start_X1->SetValue("0");
        TextCtrl_Morph_Start_X2->SetValue("0");
        TextCtrl_Morph_End_X1->SetValue("0");
        TextCtrl_Morph_End_X2->SetValue("0");
        TextCtrl_Morph_Start_Y1->SetValue("0");
        TextCtrl_Morph_Start_Y2->SetValue("0");
        TextCtrl_Morph_End_Y1->SetValue("100");
        TextCtrl_Morph_End_Y2->SetValue("100");
    }
    else if( quickset_choice == "Single Sweep Down" )
    {
        TextCtrl_Morph_Start_X1->SetValue("0");
        TextCtrl_Morph_Start_X2->SetValue("0");
        TextCtrl_Morph_End_X1->SetValue("0");
        TextCtrl_Morph_End_X2->SetValue("0");
        TextCtrl_Morph_Start_Y1->SetValue("100");
        TextCtrl_Morph_Start_Y2->SetValue("100");
        TextCtrl_Morph_End_Y1->SetValue("0");
        TextCtrl_Morph_End_Y2->SetValue("0");
    }
    else if( quickset_choice == "Single Sweep Left" )
    {
        TextCtrl_Morph_Start_X1->SetValue("100");
        TextCtrl_Morph_Start_X2->SetValue("100");
        TextCtrl_Morph_End_X1->SetValue("0");
        TextCtrl_Morph_End_X2->SetValue("0");
        TextCtrl_Morph_Start_Y1->SetValue("0");
        TextCtrl_Morph_Start_Y2->SetValue("0");
        TextCtrl_Morph_End_Y1->SetValue("0");
        TextCtrl_Morph_End_Y2->SetValue("0");
    }
    else if( quickset_choice == "Single Sweep Right" )
    {
        TextCtrl_Morph_Start_X1->SetValue("0");
        TextCtrl_Morph_Start_X2->SetValue("0");
        TextCtrl_Morph_End_X1->SetValue("100");
        TextCtrl_Morph_End_X2->SetValue("100");
        TextCtrl_Morph_Start_Y1->SetValue("0");
        TextCtrl_Morph_Start_Y2->SetValue("0");
        TextCtrl_Morph_End_Y1->SetValue("0");
        TextCtrl_Morph_End_Y2->SetValue("0");
    }
    Choice_Morph_QuickSet->SetSelection(0);
}

void EffectsPanel::OnMouthMovementTypeSelected(wxCommandEvent& event)
{
    if (event.GetId() == IDD_RADIOBUTTON_Faces_Phoneme) {
        Choice_Faces_Phoneme->Enable();
        Choice_Faces_TimingTrack->Disable();
    } else {
        Choice_Faces_Phoneme->Disable();
        Choice_Faces_TimingTrack->Enable();
    }
}

