/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include "SubModelsDialog.h"
#include "SubModelsPanel.h"
#include "layout/ModelPreview.h"
#include "models/Model.h"
#include "outputs/OutputManager.h"
#include <wx/button.h>
#include <wx/sizer.h>
#include <wx/splitter.h>
#include <wx/msgdlg.h>

// EVT_SMDROP defined once here; SubModelsPanel.cpp includes SubModelsDialog.h
// to get the declaration only.
wxDEFINE_EVENT(EVT_SMDROP, wxCommandEvent);

wxDragResult SubModelTextDropTarget::OnDragOver(wxCoord x, wxCoord y, wxDragResult def)
{
    static int MINSCROLLDELAY = 10;
    static int STARTSCROLLDELAY = 300;
    static int scrollDelay = STARTSCROLLDELAY;
    static wxLongLong lastTime = wxGetUTCTimeMillis();

    if (wxGetUTCTimeMillis() - lastTime < scrollDelay)
    {
        // too soon to scroll again
    }
    else
    {
        if (_type == "SubModel" && _list->GetItemCount() > 0)
        {
            int flags = wxLIST_HITTEST_ONITEM;
            int lastItem = _list->HitTest(wxPoint(x, y), flags, nullptr);

            for (int i = 0; i < _list->GetItemCount(); ++i)
            {
                if (i == lastItem)
                    _list->SetItemState(i, wxLIST_STATE_DROPHILITED, wxLIST_STATE_DROPHILITED);
                else
                    _list->SetItemState(i, 0, wxLIST_STATE_DROPHILITED);
            }

            wxRect rect;
            _list->GetItemRect(0, rect);
            int itemSize = rect.GetHeight();

            if (y < 2 * itemSize)
            {
                if (_list->GetTopItem() > 0)
                {
                    lastTime = wxGetUTCTimeMillis();
                    _list->EnsureVisible(_list->GetTopItem() - 1);
                    scrollDelay = scrollDelay / 2;
                    if (scrollDelay < MINSCROLLDELAY) scrollDelay = MINSCROLLDELAY;
                }
            }
            else if (y > _list->GetRect().GetHeight() - itemSize)
            {
                if (lastItem >= 0 && lastItem < _list->GetItemCount())
                {
                    _list->EnsureVisible(lastItem + 1);
                    lastTime = wxGetUTCTimeMillis();
                    scrollDelay = scrollDelay / 2;
                    if (scrollDelay < MINSCROLLDELAY) scrollDelay = MINSCROLLDELAY;
                }
            }
            else
            {
                scrollDelay = STARTSCROLLDELAY;
            }
        }
    }

    return wxDragMove;
}

bool SubModelTextDropTarget::OnDropText(wxCoord x, wxCoord y, const wxString& data)
{
    if (data == "") return false;

    long mousePos = x;
    mousePos = mousePos << 16;
    mousePos += y;
    wxCommandEvent event(EVT_SMDROP);
    event.SetString(data);
    event.SetExtraLong(mousePos);

    wxArrayString parms = wxSplit(data, ',');

    if (parms[0] == "SubModel")
    {
        if (_type == "SubModel")
        {
            event.SetInt(0);
            wxPostEvent(_owner, event);
            return true;
        }
    }

    return false;
}

BEGIN_EVENT_TABLE(SubModelsDialog, wxDialog)
    EVT_BUTTON(wxID_OK,     SubModelsDialog::OnOK)
    EVT_BUTTON(wxID_CANCEL, SubModelsDialog::OnCancelButton)
    EVT_CLOSE(              SubModelsDialog::OnClose)
END_EVENT_TABLE()

SubModelsDialog::SubModelsDialog(wxWindow* parent, OutputManager* outputManager)
    : wxDialog(parent, wxID_ANY, "SubModel Definition", wxDefaultPosition, wxDefaultSize,
               wxCAPTION | wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER | wxMAXIMIZE_BOX),
      _outputManager(outputManager)
{
    auto* outerSizer = new wxFlexGridSizer(2, 1, 0, 0);
    outerSizer->AddGrowableCol(0);
    outerSizer->AddGrowableRow(0);

    auto* splitter = new wxSplitterWindow(this, wxID_ANY, wxDefaultPosition, wxDefaultSize,
                                          wxSP_3D | wxSP_LIVE_UPDATE);
    splitter->SetSashGravity(0.5);

    _subModelsPanel = new SubModelsPanel(splitter, outputManager);

    auto* previewPanel = new wxPanel(splitter, wxID_ANY);
    auto* previewSizer = new wxFlexGridSizer(1, 1, 0, 0);
    previewSizer->AddGrowableCol(0);
    previewSizer->AddGrowableRow(0);

    _modelPreview = new ModelPreview(previewPanel);
    _modelPreview->SetMinSize(wxSize(150, 150));
    previewSizer->Add(_modelPreview, 1, wxALL | wxEXPAND, 0);
    previewPanel->SetSizer(previewSizer);

    splitter->SplitVertically(_subModelsPanel, previewPanel);
    outerSizer->Add(splitter, 1, wxALL | wxEXPAND, 0);

    auto* btnSizer = new wxStdDialogButtonSizer();
    btnSizer->AddButton(new wxButton(this, wxID_OK));
    btnSizer->AddButton(new wxButton(this, wxID_CANCEL));
    btnSizer->Realize();
    outerSizer->Add(btnSizer, 0, wxALL | wxEXPAND, 5);

    SetSizer(outerSizer);
    outerSizer->Fit(this);
    outerSizer->SetSizeHints(this);
    Center();

    _subModelsPanel->SetModelPreview(_modelPreview);

    SetEscapeId(wxID_CANCEL);
    EnableCloseButton(false);

    _oldOutputToLights = _outputManager->IsOutputting();
    if (_oldOutputToLights) {
        _outputManager->StopOutput();
    }
}

SubModelsDialog::~SubModelsDialog()
{
    if (_oldOutputToLights) {
        _outputManager->StartOutput();
    }
}

void SubModelsDialog::Setup(Model* m)
{
    _subModelsPanel->Setup(m);
}

void SubModelsDialog::Save()
{
    _subModelsPanel->Save();
    ReloadLayout = _subModelsPanel->ReloadLayout;
}

void SubModelsDialog::OnOK(wxCommandEvent&)
{
    EndModal(wxID_OK);
}

void SubModelsDialog::ConfirmClose()
{
    if (wxMessageBox("Are you sure you want to close the SubModels window?",
                     "Are you sure?", wxYES_NO | wxCENTER, this) == wxNO) {
        return;
    }
    EndModal(wxID_CANCEL);
}

void SubModelsDialog::OnClose(wxCloseEvent&)          { ConfirmClose(); }
void SubModelsDialog::OnCancelButton(wxCommandEvent&)  { ConfirmClose(); }
