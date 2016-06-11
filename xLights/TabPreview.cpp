#include "xLightsMain.h"
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

void xLightsFrame::RefreshLayout()
{
    layoutPanel->RefreshLayout();
}

void xLightsFrame::ShowPreviewTime(long ElapsedMSec)
{

}

void xLightsFrame::PreviewOutput(int period)
{
    TimerOutput(period);
    modelPreview->Render(&SeqData[period][0]);
}

void xLightsFrame::SetPreviewBackgroundImage(const wxString &filename) {
    if (mBackgroundImage != filename) {
        mBackgroundImage = filename;
        SetXmlSetting("backgroundImage",mBackgroundImage);
        UnsavedRgbEffectsChanges=true;
    }
    modelPreview->SetbackgroundImage(mBackgroundImage);
    sPreview2->SetbackgroundImage(mBackgroundImage);
    UpdatePreview();
}

const wxString & xLightsFrame::GetDefaultPreviewBackgroundImage()
{
    return sPreview2->GetBackgroundImage();
}

void xLightsFrame::SetPreviewBackgroundBrightness(int i) {
    if (mBackgroundBrightness != i) {
        mBackgroundBrightness = i;
        SetXmlSetting("backgroundBrightness",wxString::Format("%d",mBackgroundBrightness));
        UnsavedRgbEffectsChanges=true;
    }
    modelPreview->SetBackgroundBrightness(mBackgroundBrightness);
    sPreview2->SetBackgroundBrightness(mBackgroundBrightness);
    UpdatePreview();
}

void xLightsFrame::SetPreviewBackgroundScaled(bool scaled) {
    if (mScaleBackgroundImage != scaled) {
        mScaleBackgroundImage = scaled;
        SetXmlSetting("scaleImage",wxString::Format("%d",scaled));
        UnsavedRgbEffectsChanges=true;
    }
    modelPreview->SetScaleBackgroundImage(scaled);
    sPreview2->SetScaleBackgroundImage(scaled);
}
