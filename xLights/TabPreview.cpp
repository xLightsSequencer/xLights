#include "xLightsMain.h"
#include "heartbeat.h"
#include "DrawGLUtils.h"
#include "SaveChangesDialog.h"
#include "models/Model.h"
#include "xLightsXmlFile.h"
#define PREVIEWROTATIONFACTOR 3

#include "LayoutPanel.h"
#include "osxMacUtils.h"

void xLightsFrame::UpdatePreview()
{
    layoutPanel->UpdatePreview();
}

void xLightsFrame::RefreshLayout()
{
    layoutPanel->RefreshLayout();
}

void xLightsFrame::RenderLayout()
{
    layoutPanel->RenderLayout();
}

void xLightsFrame::ShowPreviewTime(long ElapsedMSec)
{

}

void xLightsFrame::PreviewOutput(int period)
{
    TimerOutput(period);
    modelPreview->Render(&SeqData[period][0]);
}

void xLightsFrame::SetStoredLayoutGroup(const std::string &group)
{
    if (mStoredLayoutGroup != group) {
        mStoredLayoutGroup = group;
        SetXmlSetting("storedLayoutGroup",mStoredLayoutGroup);
        UnsavedRgbEffectsChanges=true;
    }
}

void xLightsFrame::SetPreviewBackgroundImage(const wxString &filename)
{
    ObtainAccessToURL(filename.ToStdString());
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

void xLightsFrame::SetPreviewBackgroundBrightness(int i)
{
    if (mBackgroundBrightness != i) {
        mBackgroundBrightness = i;
        SetXmlSetting("backgroundBrightness",wxString::Format("%d",mBackgroundBrightness));
        UnsavedRgbEffectsChanges=true;
    }
    modelPreview->SetBackgroundBrightness(mBackgroundBrightness);
    sPreview2->SetBackgroundBrightness(mBackgroundBrightness);
    UpdatePreview();
}

bool xLightsFrame::GetDefaultPreviewBackgroundScaled()
{
    return sPreview2->GetScaleBackgroundImage();
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

int xLightsFrame::GetDefaultPreviewBackgroundBrightness()
{
    return sPreview2->GetBackgroundBrightness();
}
