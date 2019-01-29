#include "xLightsMain.h"
#include "xLightsXmlFile.h"
#include "ModelPreview.h"
#include "HousePreviewPanel.h"
#include "LayoutPanel.h"
#include "osxMacUtils.h"

#define PREVIEWROTATIONFACTOR 3

std::string xLightsFrame::GetSelectedLayoutPanelPreview() const
{
    return layoutPanel->GetCurrentPreview();
}

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
    _housePreviewPanel->GetModelPreview()->SetbackgroundImage(mBackgroundImage);
    UpdatePreview();
}

void xLightsFrame::SetDisplay2DBoundingBox(bool bb)
{
    if (bb != GetDisplay2DBoundingBox())
    {
        SetXmlSetting("Display2DBoundingBox", bb ? "1" : "0");
        UnsavedRgbEffectsChanges = true;
        UpdatePreview();
    }
}

bool xLightsFrame::GetDisplay2DBoundingBox() const
{
    return (GetXmlSetting("Display2DBoundingBox", "0") == "1");
}

void xLightsFrame::SetDisplay2DCenter0(bool bb) {
    if (bb != GetDisplay2DCenter0())
    {
        SetXmlSetting("Display2DCenter0", bb ? "1" : "0");
        UnsavedRgbEffectsChanges = true;
        UpdatePreview();
    }
}
bool xLightsFrame::GetDisplay2DCenter0() const {
    return (GetXmlSetting("Display2DCenter0", "0") == "1");
}

const wxString & xLightsFrame::GetDefaultPreviewBackgroundImage()
{
    return mBackgroundImage;
}

void xLightsFrame::SetPreviewBackgroundBrightness(int i, int a)
{
    if (mBackgroundBrightness != i) {
        mBackgroundBrightness = i;
        SetXmlSetting("backgroundBrightness",wxString::Format("%d",mBackgroundBrightness));
        UnsavedRgbEffectsChanges=true;
    }
    if (mBackgroundAlpha != a) {
        mBackgroundAlpha = a;
        SetXmlSetting("backgroundAlpha",wxString::Format("%d",mBackgroundAlpha));
        UnsavedRgbEffectsChanges=true;
    }
    modelPreview->SetBackgroundBrightness(mBackgroundBrightness, mBackgroundAlpha);
    _housePreviewPanel->GetModelPreview()->SetBackgroundBrightness(mBackgroundBrightness, mBackgroundAlpha);
    UpdatePreview();
}

bool xLightsFrame::GetDefaultPreviewBackgroundScaled()
{
    return mScaleBackgroundImage;
}

void xLightsFrame::SetPreviewBackgroundScaled(bool scaled) {
    if (mScaleBackgroundImage != scaled) {
        mScaleBackgroundImage = scaled;
        SetXmlSetting("scaleImage",wxString::Format("%d",scaled));
        UnsavedRgbEffectsChanges=true;
    }
    modelPreview->SetScaleBackgroundImage(scaled);
    _housePreviewPanel->GetModelPreview()->SetScaleBackgroundImage(scaled);
}

int xLightsFrame::GetDefaultPreviewBackgroundBrightness()
{
    return mBackgroundBrightness;
}
int xLightsFrame::GetDefaultPreviewBackgroundAlpha()
{
    return mBackgroundAlpha;
}
