/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include <wx/wx.h>

#include "LayoutGroup.h"
#include "xLightsMain.h"
#include "models/Model.h"
#include "PreviewPane.h"
#include "ModelPreview.h"

#include <log.h>

LayoutGroup::LayoutGroup(const std::string& name, xLightsFrame* xl)
    : mName(name), mScaleBackgroundImage(false), mBackgroundBrightness(100), mBackgroundAlpha(100), mPreviewHidden(true), mPreviewCreated(false),
    mModelPreview(nullptr), xlights(xl), mMenuItemPreview(nullptr), mPreviewPane(nullptr), id_menu_item(wxNewId()),
    mPosX(-1), mPosY(-1), mPaneWidth(-1), mPaneHeight(-1), ignore_size_and_pos(false)
{
}

LayoutGroup::~LayoutGroup()
{
    previewModels.clear();
}

void LayoutGroup::SetBackgroundImage(const std::string& filename)
{
    if (mBackgroundImage != filename) {
        mBackgroundImage = filename;
        if (mModelPreview != nullptr) {
            mModelPreview->SetbackgroundImage(mBackgroundImage);
            if (!mPreviewHidden) {
                mModelPreview->Refresh();
                mModelPreview->Update();
            }
        }
        xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "SetBackgroundImage");
    }
}

void LayoutGroup::SetBackgroundBrightness(int i, int a)
{
    if (mBackgroundBrightness != i || mBackgroundAlpha != a) {
        mBackgroundBrightness = i;
        mBackgroundAlpha = a;
        if (mModelPreview != nullptr) {
            mModelPreview->SetBackgroundBrightness(mBackgroundBrightness, mBackgroundAlpha);
        }
        xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "SetBackgroundBrightness");
    }
}

void LayoutGroup::SetBackgroundScaled(bool scaled)
{
    if (mScaleBackgroundImage != scaled) {
        mScaleBackgroundImage = scaled;
        if (mModelPreview != nullptr) {
            mModelPreview->SetScaleBackgroundImage(scaled);
        }
        xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "SetBackgroundScaled");
    }
}

void LayoutGroup::SetPreviewPosition(wxPoint point)
{
    if (!ignore_size_and_pos) {
        mPosX = point.x;
        mPosY = point.y;
        xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "SetPreviewPosition");
    }
}

void LayoutGroup::SetPreviewSize(wxSize size_)
{
    if (!ignore_size_and_pos) {
        mPaneWidth = size_.GetWidth();
        mPaneHeight = size_.GetHeight();
        xlights->GetOutputModelManager()->AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "SetPreviewSize");
    }
}

void LayoutGroup::SetFromXml(pugi::xml_node LayoutGroupNode)
{
    mName = LayoutGroupNode.attribute("name").as_string("");
    mBackgroundImage = LayoutGroupNode.attribute("backgroundImage").as_string("");
    mBackgroundBrightness = LayoutGroupNode.attribute("backgroundBrightness").as_int(100);
    mBackgroundAlpha = LayoutGroupNode.attribute("backgroundAlpha").as_int(100);
    mScaleBackgroundImage = LayoutGroupNode.attribute("scaleImage").as_int(0) > 0;
    mPosX = LayoutGroupNode.attribute("PosX").as_int(-1);
    mPosY = LayoutGroupNode.attribute("PosY").as_int(-1);
    mPaneWidth = LayoutGroupNode.attribute("PaneWidth").as_int(-1);
    mPaneHeight = LayoutGroupNode.attribute("PaneHeight").as_int(-1);
}

void LayoutGroup::SetModels(std::vector<Model*> &models)
{
    previewModels.clear();
    for (const auto& it : models) {
        previewModels.push_back(it);
    }
}

void LayoutGroup::ResetPositions()
{
    if( mPreviewPane != nullptr ) {
        ModelPreview* modelPreview = xlights->GetLayoutPreview();
        mPaneWidth = modelPreview->GetVirtualCanvasWidth();
        mPaneHeight = modelPreview->GetVirtualCanvasHeight();
        mPreviewPane->SetSize(modelPreview->GetPosition().x+100, modelPreview->GetPosition().y+100, mPaneWidth, mPaneHeight);
    }
}

void LayoutGroup::SetPreviewActive(bool show)
{
    if( mPreviewCreated ) {
        mPreviewHidden = !show;
        mModelPreview->SetActive(show);
    }
}

const long LayoutGroup::AddToPreviewMenu(wxMenu* preview_menu)
{
    mMenuItemPreview = new wxMenuItem(preview_menu, id_menu_item, mName, wxEmptyString, wxITEM_CHECK);
    preview_menu->Append(mMenuItemPreview);
    return id_menu_item;
}

void LayoutGroup::RemoveFromPreviewMenu(wxMenu* preview_menu)
{
    preview_menu->Destroy(mMenuItemPreview);
    if( mPreviewPane != nullptr ) {
        delete mPreviewPane;
        mPreviewPane = nullptr;
    }
    mModelPreview = nullptr;
    mPreviewCreated = false;
    mMenuItemPreview = nullptr;
}

void LayoutGroup::ShowPreview(bool show)
{
    if( !mPreviewCreated ) {
        ignore_size_and_pos = true;
        ModelPreview* modelPreview = xlights->GetLayoutPreview();
        if (modelPreview == nullptr)
        {
            
            spdlog::warn("LayoutGroup::ShowPreview GetLayoutPreview returned NULL");
            return;
        }

        PreviewPane* preview = new PreviewPane(xlights, wxID_ANY, wxDefaultPosition, wxSize(modelPreview->GetVirtualCanvasWidth(), modelPreview->GetVirtualCanvasHeight()));
        mPreviewPane = preview;
        wxPanel* panel = preview->GetPreviewPanel();
        wxFlexGridSizer* panel_sizer = preview->GetPreviewPanelSizer();
        ModelPreview* new_preview = new ModelPreview(panel, xlights, false);
        new_preview->SetActiveLayoutGroup(GetName());
        new_preview->SetPreviewPane(preview);
        mModelPreview = new_preview;
        panel_sizer->Add(new_preview, 1, wxALL | wxEXPAND, 0);
        preview->SetLayoutGroup(this);

        if( !mPreviewCreated ) {
            if( mPaneWidth == -1 ) {
                mPaneWidth = modelPreview->GetVirtualCanvasWidth();
            }
            if( mPaneHeight == -1 ) {
                mPaneHeight = modelPreview->GetVirtualCanvasHeight();
            }
        }
        preview->SetSize(mPosX, mPosY, mPaneWidth, mPaneHeight);
        xlights->PreviewWindows.push_back(new_preview);
        new_preview->InitializePreview(mBackgroundImage, mBackgroundBrightness, mBackgroundAlpha, xlights->GetDisplay2DCenter0());
        new_preview->SetScaleBackgroundImage(mScaleBackgroundImage);
        new_preview->SetCanvasSize(modelPreview->GetVirtualCanvasWidth(), modelPreview->GetVirtualCanvasHeight());
        mPreviewCreated = true;
        mMenuItemPreview->Check(true);
        ignore_size_and_pos = false;
    }
    SetPreviewActive(show);
    mMenuItemPreview->Check(show);
}
