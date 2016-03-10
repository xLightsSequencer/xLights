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

void xLightsFrame::SetPreviewBackgroundImage(const wxString &filename) {
    mBackgroundImage = filename;
    SetXmlSetting("backgroundImage",mBackgroundImage);
    modelPreview->SetbackgroundImage(mBackgroundImage);
    sPreview2->SetbackgroundImage(mBackgroundImage);
    UnsavedRgbEffectsChanges=true;
    UpdatePreview();
}

void xLightsFrame::SetPreviewBackgroundBrightness(int i) {
    mBackgroundBrightness = i;
    SetXmlSetting("backgroundBrightness",wxString::Format("%d",mBackgroundBrightness));
    modelPreview->SetBackgroundBrightness(mBackgroundBrightness);
    sPreview2->SetBackgroundBrightness(mBackgroundBrightness);
    UnsavedRgbEffectsChanges=true;
    UpdatePreview();
}

void xLightsFrame::SetPreviewBackgroundScaled(bool scaled) {
    SetXmlSetting("scaleImage",wxString::Format("%d",scaled));
    modelPreview->SetScaleBackgroundImage(scaled);
    sPreview2->SetScaleBackgroundImage(scaled);
    UnsavedRgbEffectsChanges=true;
}
