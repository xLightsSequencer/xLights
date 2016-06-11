
#include <wx/wx.h>
#include <wx/xml/xml.h>

#include "LayoutGroup.h"
#include "xLightsMain.h"
#include "models/Model.h"
#include "PreviewPane.h"

LayoutGroup::LayoutGroup(const std::string & name, xLightsFrame* xl, wxXmlNode *node, wxString bkImage)
: mName(name), mPreviewHidden(true), mPreviewCreated(false), mModelPreview(nullptr), xlights(xl), LayoutGroupXml(node), id_menu_item(wxNewId())
{
    if( bkImage != "" ) {
        SetBackgroundImage( bkImage );
    }
    SetFromXml(node);
}

LayoutGroup::~LayoutGroup()
{
    previewModels.clear();
    for (auto it = xlights->PreviewWindows.begin(); it != xlights->PreviewWindows.end(); it++) {
        if( *it == mModelPreview ) {
            xlights->PreviewWindows.erase(it);
            delete mModelPreview;
            mModelPreview = nullptr;
            break;
        }
    }
}

void LayoutGroup::SetBackgroundImage(const wxString &filename)
{
    if (mBackgroundImage != filename) {
        mBackgroundImage = filename;
        LayoutGroupXml->DeleteAttribute("backgroundImage");
        LayoutGroupXml->AddAttribute("backgroundImage", mBackgroundImage);
        if( mModelPreview != nullptr ) {
            mModelPreview->SetbackgroundImage(mBackgroundImage);
            if( !mPreviewHidden ) {
                mModelPreview->Refresh();
                mModelPreview->Update();
            }
        }
    }
}

void LayoutGroup::SetFromXml(wxXmlNode* LayoutGroupNode)
{
    LayoutGroupXml = LayoutGroupNode;
    mName=LayoutGroupNode->GetAttribute("name").ToStdString();
    mBackgroundImage=LayoutGroupNode->GetAttribute("backgroundImage").ToStdString();
}

wxXmlNode* LayoutGroup::GetLayoutGroupXml() const {
    return this->LayoutGroupXml;
}

void LayoutGroup::SetModels(std::vector<Model*> &models)
{
    previewModels.clear();
    for (auto it = models.begin(); it != models.end(); it++) {
        previewModels.push_back(*it);
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

void LayoutGroup::ShowHidePreview()
{
    if( !mPreviewCreated ) {
        ModelPreview* modelPreview = xlights->GetLayoutPreview();
        PreviewPane* preview = new PreviewPane(xlights, wxID_ANY, wxDefaultPosition, wxSize(modelPreview->GetVirtualCanvasWidth(), modelPreview->GetVirtualCanvasHeight()));
        wxPanel* panel = preview->GetPreviewPanel();
        wxFlexGridSizer* panel_sizer = preview->GetPreviewPanelSizer();
        ModelPreview* new_preview = new ModelPreview(panel, GetModels(), false);
        new_preview->SetPreviewPane(preview);
        mModelPreview = new_preview;
        panel_sizer->Add(new_preview, 1, wxALL | wxEXPAND, 0);
        preview->SetLayoutGroup(this);

        xlights->PreviewWindows.push_back(new_preview);
        new_preview->InitializePreview(mBackgroundImage,modelPreview->GetBackgroundBrightness());
        new_preview->SetScaleBackgroundImage(modelPreview->GetScaleBackgroundImage());
        new_preview->SetCanvasSize(modelPreview->GetVirtualCanvasWidth(),modelPreview->GetVirtualCanvasHeight());
        new_preview->SetVirtualCanvasSize(modelPreview->GetVirtualCanvasWidth(), modelPreview->GetVirtualCanvasHeight());
        preview->SetSize(modelPreview->GetVirtualCanvasWidth(),modelPreview->GetVirtualCanvasHeight());
        mPreviewCreated = true;
        SetPreviewActive(true);
        mMenuItemPreview->Check(true);
    } else {
        bool show = mMenuItemPreview->IsChecked();
        SetPreviewActive(show);
        mMenuItemPreview->Check(show);
    }

}

