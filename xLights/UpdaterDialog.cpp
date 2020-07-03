/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/smeighan/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/smeighan/xLights/blob/master/License.txt
 **************************************************************/
 
//(*InternalHeaders(UpdaterDialog)
#include <wx/string.h>
#include <wx/intl.h>
//*)

#include <wx/config.h>

#include "UpdaterDialog.h"

#include <log4cpp/Category.hh>

//(*IdInit(UpdaterDialog)
const long UpdaterDialog::ID_UPDATELABEL = wxNewId();
const long UpdaterDialog::ID_BUTTONUPDOWN = wxNewId();
const long UpdaterDialog::ID_BUTTON2 = wxNewId();
const long UpdaterDialog::ID_BUTTON3 = wxNewId();
//*)

BEGIN_EVENT_TABLE(UpdaterDialog,wxDialog)
	//(*EventTable(UpdaterDialog)
	//*)
END_EVENT_TABLE()

UpdaterDialog::UpdaterDialog(wxWindow* parent,wxWindowID id)
{
	//(*Initialize(UpdaterDialog)
	wxBoxSizer* BoxSizer2;
	wxBoxSizer* BoxSizer1;

	Create(parent, wxID_ANY, _("xLights Update Available"), wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE, _T("wxID_ANY"));
	BoxSizer1 = new wxBoxSizer(wxVERTICAL);
	StaticTextUpdateLabel = new wxStaticText(this, ID_UPDATELABEL, _("You are running xLights version xxx\nWhereas the most recent release is xxx"), wxDefaultPosition, wxDefaultSize, wxALIGN_CENTRE, _T("ID_UPDATELABEL"));
	BoxSizer1->Add(StaticTextUpdateLabel, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	BoxSizer2 = new wxBoxSizer(wxHORIZONTAL);
	ButtonUpDownload = new wxButton(this, ID_BUTTONUPDOWN, _("Download new release"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTONUPDOWN"));
	BoxSizer2->Add(ButtonUpDownload, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	ButtonUpdateIgnore = new wxButton(this, ID_BUTTON2, _("Ignore this version"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON2"));
	BoxSizer2->Add(ButtonUpdateIgnore, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	ButtonUpdateSkip = new wxButton(this, ID_BUTTON3, _("Skip this time"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON3"));
	ButtonUpdateSkip->SetDefault();
	BoxSizer2->Add(ButtonUpdateSkip, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	BoxSizer1->Add(BoxSizer2, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	SetSizer(BoxSizer1);
	BoxSizer1->Fit(this);
	BoxSizer1->SetSizeHints(this);
	Center();

	Connect(ID_BUTTONUPDOWN,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&UpdaterDialog::OnButtonDownloadNewRelease);
	Connect(ID_BUTTON2,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&UpdaterDialog::OnButtonUpdateIgnoreClick);
	Connect(ID_BUTTON3,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&UpdaterDialog::OnButtonUpdateSkipClick);
	//*)
}

UpdaterDialog::~UpdaterDialog()
{
	//(*Destroy(UpdaterDialog)
	//*)
}

void UpdaterDialog::OnButtonDownloadNewRelease(wxCommandEvent& event)
{
    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    logger_base.debug("User has chosen to upgrade to version %s. URL: %s",
        (const char *)urlVersion.c_str(), (const char *)downloadUrl.c_str());
    wxLaunchDefaultBrowser(downloadUrl);
    wxConfigBase* config = wxConfigBase::Get();
    config->Write("SkipVersion","");
    EndDialog(wxID_OK);
}

void UpdaterDialog::OnButtonUpdateSkipClick(wxCommandEvent& event)
{
    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    logger_base.debug("User has chosen to skip upgrade to version %s.", (const char *)urlVersion.c_str());
    wxConfigBase* config = wxConfigBase::Get();
    config->Write("SkipVersion","");
    EndDialog(wxID_CANCEL);
}

void UpdaterDialog::OnButtonUpdateIgnoreClick(wxCommandEvent& event)
{
    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    logger_base.debug("User has chosen to ignore upgrade to version %s.", (const char *)urlVersion.c_str());
    wxConfigBase* config = wxConfigBase::Get();
    config->Write("SkipVersion", urlVersion);
    EndDialog(wxID_CLOSE);
}
