#include "xLightsMain.h"
#include "ModelDialog.h" //Cheating to avoid full recompile by adding this in main.h
#include "heartbeat.h"
#include "DrawGLUtils.h"
#include "SaveChangesDialog.h"
#include "models/Model.h"
#include "xLightsXmlFile.h"
#define PREVIEWROTATIONFACTOR 3

#include "LayoutPanel.h"

void xLightsFrame::UpdatePreview()
{
    layoutPanel->UpdatePreview();
}

void xLightsFrame::ShowPreviewTime(long ElapsedMSec)
{
    
}

void xLightsFrame::PreviewOutput(int period)
{
    TimerOutput(period);
    modelPreview->Render(&SeqData[period][0]);
}


void xLightsFrame::SetModelAsPartOfDisplay(wxString& model)
{
    for(wxXmlNode* e=ModelsNode->GetChildren(); e!=NULL; e=e->GetNext() )
    {
        if (e->GetName() == "model")
        {
            if(e->GetAttribute("name")== model)
            {
                e->DeleteAttribute("MyDisplay");
                e->AddAttribute("MyDisplay","1");
            }
        }
    }
}

void xLightsFrame::OnButtonSetBackgroundImageClick(wxCommandEvent& event)
{
    wxString filename = wxFileSelector( "Choose Background Image", CurrentDir, "", "", wxImage::GetImageExtWildcard(), wxFD_OPEN | wxFD_FILE_MUST_EXIST );
    if (!filename.IsEmpty())
    {
        mBackgroundImage = filename;
        SetXmlSetting("backgroundImage",mBackgroundImage);
        modelPreview->SetbackgroundImage(mBackgroundImage);
        sPreview2->SetbackgroundImage(mBackgroundImage);
        UnsavedRgbEffectsChanges=true;
        UpdatePreview();
    }
}

void xLightsFrame::OnSlider_BackgroundBrightnessCmdSliderUpdated(wxScrollEvent& event)
{
    mBackgroundBrightness = layoutPanel->Slider_BackgroundBrightness->GetValue();
    SetXmlSetting("backgroundBrightness",wxString::Format("%d",mBackgroundBrightness));
    modelPreview->SetBackgroundBrightness(mBackgroundBrightness);
    sPreview2->SetBackgroundBrightness(mBackgroundBrightness);
    UnsavedRgbEffectsChanges=true;
    UpdatePreview();
}


void xLightsFrame::OnScaleImageCheckboxClick(wxCommandEvent& event)
{
    SetXmlSetting("scaleImage",wxString::Format("%d",layoutPanel->ScaleImageCheckbox->IsChecked()));
    modelPreview->SetScaleBackgroundImage(layoutPanel->ScaleImageCheckbox->IsChecked());
    sPreview2->SetScaleBackgroundImage(layoutPanel->ScaleImageCheckbox->IsChecked());
    UnsavedRgbEffectsChanges=true;
}
