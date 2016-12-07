#include "FPPConnectDialog.h"
#include "xLightsMain.h"
#include <wx/regex.h>
#include "xLightsXmlFile.h"
#include <wx/volume.h>
#include <wx/progdlg.h>
#include <wx/config.h>
#include <wx/sckstrm.h>

//(*InternalHeaders(FPPConnectDialog)
#include <wx/intl.h>
#include <wx/string.h>
//*)

//(*IdInit(FPPConnectDialog)
const long FPPConnectDialog::ID_STATICTEXT1 = wxNewId();
const long FPPConnectDialog::ID_TEXTCTRL_IPAddress = wxNewId();
const long FPPConnectDialog::ID_STATICTEXT2 = wxNewId();
const long FPPConnectDialog::ID_TEXTCTRL_Username = wxNewId();
const long FPPConnectDialog::ID_STATICTEXT3 = wxNewId();
const long FPPConnectDialog::ID_TEXTCTRL_Password = wxNewId();
const long FPPConnectDialog::ID_BUTTON_Console = wxNewId();
const long FPPConnectDialog::ID_PANEL_FTP = wxNewId();
const long FPPConnectDialog::ID_STATICTEXT4 = wxNewId();
const long FPPConnectDialog::ID_CHOICE_Drives = wxNewId();
const long FPPConnectDialog::ID_STATICTEXT5 = wxNewId();
const long FPPConnectDialog::ID_DIRPICKERCTRL1 = wxNewId();
const long FPPConnectDialog::ID_PANEL_USB = wxNewId();
const long FPPConnectDialog::ID_NOTEBOOK_FPP = wxNewId();
const long FPPConnectDialog::ID_CHECKBOX_UploadController = wxNewId();
const long FPPConnectDialog::ID_CHECKBOX1 = wxNewId();
const long FPPConnectDialog::ID_CHECKLISTBOX_Sequences = wxNewId();
const long FPPConnectDialog::ID_BUTTON_Upload = wxNewId();
//*)

const long FPPConnectDialog::ID_MNU_SELECTALL = wxNewId();
const long FPPConnectDialog::ID_MNU_SELECTNONE = wxNewId();

BEGIN_EVENT_TABLE(FPPConnectDialog,wxDialog)
	//(*EventTable(FPPConnectDialog)
	//*)
END_EVENT_TABLE()

FPPConnectDialog::FPPConnectDialog(wxWindow* parent,wxWindowID id,const wxPoint& pos,const wxSize& size)
{
	//(*Initialize(FPPConnectDialog)
	wxFlexGridSizer* FlexGridSizer3;
	wxFlexGridSizer* FlexGridSizer2;
	wxFlexGridSizer* FlexGridSizer1;

	Create(parent, id, _("FPP Upload"), wxDefaultPosition, wxDefaultSize, wxCAPTION|wxDEFAULT_DIALOG_STYLE|wxRESIZE_BORDER|wxCLOSE_BOX|wxMAXIMIZE_BOX, _T("id"));
	SetClientSize(wxDefaultSize);
	Move(wxDefaultPosition);
	FlexGridSizer1 = new wxFlexGridSizer(0, 1, 0, 0);
	FlexGridSizer1->AddGrowableCol(0);
	FlexGridSizer1->AddGrowableRow(3);
	Notebook_FPP = new wxNotebook(this, ID_NOTEBOOK_FPP, wxDefaultPosition, wxDefaultSize, 0, _T("ID_NOTEBOOK_FPP"));
	Panel_FTP = new wxPanel(Notebook_FPP, ID_PANEL_FTP, wxPoint(14,59), wxDefaultSize, wxTAB_TRAVERSAL, _T("ID_PANEL_FTP"));
	FlexGridSizer2 = new wxFlexGridSizer(0, 2, 0, 0);
	FlexGridSizer2->AddGrowableCol(1);
	StaticText1 = new wxStaticText(Panel_FTP, ID_STATICTEXT1, _("IP Address"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT1"));
	FlexGridSizer2->Add(StaticText1, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	TextCtrl_IPAddress = new wxTextCtrl(Panel_FTP, ID_TEXTCTRL_IPAddress, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_TEXTCTRL_IPAddress"));
	TextCtrl_IPAddress->SetMaxLength(15);
	FlexGridSizer2->Add(TextCtrl_IPAddress, 1, wxALL|wxEXPAND, 5);
	StaticText2 = new wxStaticText(Panel_FTP, ID_STATICTEXT2, _("Username"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT2"));
	FlexGridSizer2->Add(StaticText2, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	TextCtr_Username = new wxTextCtrl(Panel_FTP, ID_TEXTCTRL_Username, _("fpp"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_TEXTCTRL_Username"));
	FlexGridSizer2->Add(TextCtr_Username, 1, wxALL|wxEXPAND, 5);
	StaticText3 = new wxStaticText(Panel_FTP, ID_STATICTEXT3, _("Password"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT3"));
	FlexGridSizer2->Add(StaticText3, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	TextCtrl_Password = new wxTextCtrl(Panel_FTP, ID_TEXTCTRL_Password, _("falcon"), wxDefaultPosition, wxDefaultSize, wxTE_PASSWORD, wxDefaultValidator, _T("ID_TEXTCTRL_Password"));
	FlexGridSizer2->Add(TextCtrl_Password, 1, wxALL|wxEXPAND, 5);
	FlexGridSizer2->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	Button_Console = new wxButton(Panel_FTP, ID_BUTTON_Console, _("FPP Console"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON_Console"));
	FlexGridSizer2->Add(Button_Console, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	Panel_FTP->SetSizer(FlexGridSizer2);
	FlexGridSizer2->Fit(Panel_FTP);
	FlexGridSizer2->SetSizeHints(Panel_FTP);
	Panel_USB = new wxPanel(Notebook_FPP, ID_PANEL_USB, wxPoint(39,12), wxDefaultSize, wxTAB_TRAVERSAL, _T("ID_PANEL_USB"));
	FlexGridSizer3 = new wxFlexGridSizer(0, 2, 0, 0);
	FlexGridSizer3->AddGrowableCol(1);
	StaticText4 = new wxStaticText(Panel_USB, ID_STATICTEXT4, _("Drive"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT4"));
	FlexGridSizer3->Add(StaticText4, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	Choice_Drives = new wxChoice(Panel_USB, ID_CHOICE_Drives, wxDefaultPosition, wxDefaultSize, 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE_Drives"));
	FlexGridSizer3->Add(Choice_Drives, 1, wxALL|wxEXPAND, 5);
	StaticText5 = new wxStaticText(Panel_USB, ID_STATICTEXT5, _("FPP Media Directory"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT5"));
	FlexGridSizer3->Add(StaticText5, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	DirPickerCtrl_FPPMedia = new wxDirPickerCtrl(Panel_USB, ID_DIRPICKERCTRL1, wxEmptyString, _("Select FPP Media Directory on a USB Stick"), wxDefaultPosition, wxDefaultSize, wxDIRP_DIR_MUST_EXIST|wxDIRP_USE_TEXTCTRL, wxDefaultValidator, _T("ID_DIRPICKERCTRL1"));
	FlexGridSizer3->Add(DirPickerCtrl_FPPMedia, 1, wxALL|wxEXPAND, 5);
	Panel_USB->SetSizer(FlexGridSizer3);
	FlexGridSizer3->Fit(Panel_USB);
	FlexGridSizer3->SetSizeHints(Panel_USB);
	Notebook_FPP->AddPage(Panel_FTP, _("FTP"), true);
	Notebook_FPP->AddPage(Panel_USB, _("USB"), false);
	FlexGridSizer1->Add(Notebook_FPP, 1, wxALL|wxEXPAND, 5);
	CheckBox_UploadController = new wxCheckBox(this, ID_CHECKBOX_UploadController, _("Upload controller configuration"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX_UploadController"));
	CheckBox_UploadController->SetValue(false);
	FlexGridSizer1->Add(CheckBox_UploadController, 1, wxALL|wxEXPAND, 5);
	CheckBox_UploadModels = new wxCheckBox(this, ID_CHECKBOX1, _("Upload Models"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX1"));
	CheckBox_UploadModels->SetValue(false);
	FlexGridSizer1->Add(CheckBox_UploadModels, 1, wxALL|wxEXPAND, 5);
	CheckListBox_Sequences = new wxCheckListBox(this, ID_CHECKLISTBOX_Sequences, wxDefaultPosition, wxDefaultSize, 0, 0, wxLB_SORT, wxDefaultValidator, _T("ID_CHECKLISTBOX_Sequences"));
	CheckListBox_Sequences->SetMinSize(wxDLG_UNIT(this,wxSize(-1,100)));
	FlexGridSizer1->Add(CheckListBox_Sequences, 1, wxALL|wxEXPAND, 5);
	Button_Upload = new wxButton(this, ID_BUTTON_Upload, _("Upload"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON_Upload"));
	FlexGridSizer1->Add(Button_Upload, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	SetSizer(FlexGridSizer1);
	FlexGridSizer1->Fit(this);
	FlexGridSizer1->SetSizeHints(this);

	Connect(ID_TEXTCTRL_IPAddress,wxEVT_COMMAND_TEXT_UPDATED,(wxObjectEventFunction)&FPPConnectDialog::OnTextCtrl_IPAddressText);
	Connect(ID_TEXTCTRL_Username,wxEVT_COMMAND_TEXT_UPDATED,(wxObjectEventFunction)&FPPConnectDialog::OnTextCtr_UsernameText);
	Connect(ID_TEXTCTRL_Password,wxEVT_COMMAND_TEXT_UPDATED,(wxObjectEventFunction)&FPPConnectDialog::OnTextCtrl_PasswordText);
	Connect(ID_BUTTON_Console,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&FPPConnectDialog::OnButton_ConsoleClick);
	Connect(ID_DIRPICKERCTRL1,wxEVT_COMMAND_DIRPICKER_CHANGED,(wxObjectEventFunction)&FPPConnectDialog::OnFilePickerCtrl_MediaFolderFileChanged);
	Connect(ID_NOTEBOOK_FPP,wxEVT_COMMAND_NOTEBOOK_PAGE_CHANGED,(wxObjectEventFunction)&FPPConnectDialog::OnNotebook_FPPPageChanged);
	Connect(ID_CHECKBOX_UploadController,wxEVT_COMMAND_CHECKBOX_CLICKED,(wxObjectEventFunction)&FPPConnectDialog::OnCheckBox_UploadControllerClick);
	Connect(ID_CHECKLISTBOX_Sequences,wxEVT_COMMAND_CHECKLISTBOX_TOGGLED,(wxObjectEventFunction)&FPPConnectDialog::OnCheckListBox_SequencesToggled);
	Connect(ID_BUTTON_Upload,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&FPPConnectDialog::OnButton_UploadClick);
	//*)

    CheckListBox_Sequences->Connect(ID_CHECKLISTBOX_Sequences, wxEVT_RIGHT_UP, (wxObjectEventFunction)&FPPConnectDialog::OnSequenceRClick, nullptr, this);

#ifdef __WXMSW__
    wxArrayString drives = wxFSVolume::GetVolumes(wxFS_VOL_REMOVABLE | wxFS_VOL_MOUNTED, 0);
    for (auto it = drives.begin(); it != drives.end(); ++it)
    {
        Choice_Drives->AppendString(*it);
    }
    if (Choice_Drives->GetCount() > 0)
    {
        Choice_Drives->SetSelection(0);
    }
    DirPickerCtrl_FPPMedia->Hide();
    StaticText5->Hide();
#else
    Choice_Drives->Hide();
    StaticText4->Hide();
    #ifdef __WXOSX__
        wxDir d;
        d.Open("/Volumes");
        wxString dir;
        bool fcont = d.GetFirst(&dir, wxEmptyString, wxDIR_DIRS);
        while (fcont)
        {
            if (wxDir::Exists("/Volumes/" + dir + "/sequences")) //raw USB drive mounted
            {
                DirPickerCtrl_FPPMedia->SetPath("/Volumes/" + dir + "/");
                break;
            }
            else if (wxDir::Exists("/Volumes/" + dir + "/media/sequences")) // Mounted via SMB/NFS
            {
                DirPickerCtrl_FPPMedia->SetPath("/Volumes/" + dir + "/media/");
                break;
            }
            fcont = d.GetNext(&dir);
        }
    #else
    bool done = false;
    wxDir d;
    d.Open("/media");
    wxString dir;
    bool fcont = d.GetFirst(&dir, wxEmptyString, wxDIR_DIRS);
    while (fcont)
    {
        wxDir d2;
        d2.Open("/media/" + dir);
        wxString dir2;
        bool fcont2 = d2.GetFirst(&dir2, wxEmptyString, wxDIR_DIRS);
        while (fcont2)
        {
            if (wxDir::Exists("/media/" + dir + "/" + dir2 + "/sequences"))
            {
                DirPickerCtrl_FPPMedia->SetPath("/media/" + dir + "/" + dir2);
                done = true;
                break;
            }
            fcont2 = d2.GetNext(&dir2);
        }
        if (done) break;
        fcont = d.GetNext(&dir);
    }
    #endif
#endif

    LoadSequences();

    wxConfigBase* config = wxConfigBase::Get();
    wxString ip;
    config->Read("xLightsPiIP", &ip, "");
    wxString user;
    config->Read("xLightsPiUser", &user, "fpp");
    TextCtrl_IPAddress->SetValue(ip);
    TextCtr_Username->SetValue(user);

    bool usedefaultpwd;
    config->Read("xLightsPiDefaultPassword", &usedefaultpwd, true);

    if (usedefaultpwd)
    {
        if (user == "pi")
        {
            TextCtrl_Password->SetValue("raspberry");
        }
        else if (user == "fpp")
        {
            TextCtrl_Password->SetValue("falcon");
        }
        else
        {
            TextCtrl_Password->SetValue("");
        }
    }
    else
    {
        TextCtrl_Password->SetValue("");
    }

    ValidateWindow();
}

void FPPConnectDialog::OnSequenceRClick(wxMouseEvent& event)
{
    wxMenu mnu;
    mnu.Append(ID_MNU_SELECTALL, "Select All");
    mnu.Append(ID_MNU_SELECTNONE, "Clear Selections");
    mnu.Connect(wxEVT_MENU, (wxObjectEventFunction)&FPPConnectDialog::OnPopup, nullptr, this);
    PopupMenu(&mnu);
}

void FPPConnectDialog::OnPopup(wxCommandEvent &event)
{
    int id = event.GetId();

    if (id == ID_MNU_SELECTALL)
    {
        for (size_t i = 0; i < CheckListBox_Sequences->GetCount(); i++)
        {
            CheckListBox_Sequences->Check(i, true);
        }
    }
    else if (id == ID_MNU_SELECTNONE)
    {
        wxArrayInt sel;
        CheckListBox_Sequences->GetCheckedItems(sel);
        for (auto it = sel.begin(); it != sel.end(); ++it)
        {
            CheckListBox_Sequences->Check(*it, false);
        }
    }
    ValidateWindow();
}

FPPConnectDialog::~FPPConnectDialog()
{
	//(*Destroy(FPPConnectDialog)
	//*)
}

void FPPConnectDialog::LoadSequencesFromFolder(wxString dir)
{
    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    logger_base.info("Scanning folder for sequences for FPP upload: %s", (const char *)dir.c_str());

    wxDir directory;
    directory.Open(dir);

    wxString file;
    bool fcont = directory.GetFirst(&file, "*.xml");

    while (fcont)
    {
        if (file != "xlights_rgbeffects.xml" && file != "xlights_networks.xml" && file != "xlights_keybindings.xml")
        {
            // this could be a sequence file ... lets open it and check
            wxXmlDocument doc(xLightsFrame::CurrentDir + "/" + file);
            if (doc.IsOk())
            {
                wxXmlNode* root = doc.GetRoot();
                if (root->GetName() == "xsequence")
                {
                    // now check the fseq file exists
                    wxFileName fn(dir + "/" + file);
                    wxString fseq = dir + "/" + fn.GetName() + ".fseq";

                    if (wxFile::Exists(fseq))
                    {
                        // ok it is a sequence so we need to add it
                        CheckListBox_Sequences->AppendString(fn.GetLongPath());
                    }
                }
            }
        }

        fcont = directory.GetNext(&file);
    }

    fcont = directory.GetFirst(&file, wxEmptyString, wxDIR_DIRS);
    while (fcont)
    {
        if (file != "Backup")
        {
            LoadSequencesFromFolder(dir + "/" + file);
        }
        fcont = directory.GetNext(&file);
    }
}

void FPPConnectDialog::LoadSequences()
{
    CheckListBox_Sequences->Clear();

    LoadSequencesFromFolder(xLightsFrame::CurrentDir);

    if (xLightsFrame::CurrentSeqXmlFile != nullptr)
    {
        wxString curSeq = xLightsFrame::CurrentSeqXmlFile->GetLongPath();
        if (!curSeq.StartsWith(xLightsFrame::CurrentDir))
        {
            LoadSequencesFromFolder(xLightsFrame::CurrentSeqXmlFile->GetLongPath());
        }

        int i = CheckListBox_Sequences->FindString(xLightsFrame::CurrentSeqXmlFile->GetLongPath(), true);
        CheckListBox_Sequences->Check(i);
    }

    Fit();
}

bool FPPConnectDialog::IsValidIP(wxString ip)
{
    if (ip == "") {
        return false;
    }
    wxRegEx regxIPAddr("^(([0-9]{1}|[0-9]{2}|[0-1][0-9]{2}|2[0-4][0-9]|25[0-5])\\.){3}([0-9]{1}|[0-9]{2}|[0-1][0-9]{2}|2[0-4][0-9]|25[0-5])$");
    if (regxIPAddr.Matches(ip)) {
        return true;
    }

    wxIPV4address addr;
    addr.Hostname(ip);
    wxString ipAddr = addr.IPAddress();
    return ipAddr != "0.0.0.0" && ipAddr != "";
}

void FPPConnectDialog::ValidateWindow()
{
    if (wxFile::Exists(xLightsFrame::CurrentDir + "/universes"))
    {
        CheckBox_UploadController->Enable();
    }
    else
    {
        CheckBox_UploadController->Disable();
        CheckBox_UploadController->SetValue(false);
    }
    if (wxFile::Exists(xLightsFrame::CurrentDir + "/channelmemorymaps"))
    {
        CheckBox_UploadModels->Enable();
    }
    else
    {
        CheckBox_UploadModels->Disable();
        CheckBox_UploadModels->SetValue(false);
    }


    if (Notebook_FPP->GetSelection() == 0)
    {
        wxArrayInt tmp;
        CheckListBox_Sequences->GetCheckedItems(tmp);
        if (TextCtr_Username->GetValue() != "" && IsValidIP(TextCtrl_IPAddress->GetValue()) && (CheckBox_UploadController->IsChecked() || tmp.size() > 0))
        {
            Button_Upload->Enable();
        }
        else
        {
            Button_Upload->Disable();
        }
    }
    else
    {
        #ifdef __WXMSW__
        wxArrayInt tmp;
        CheckListBox_Sequences->GetCheckedItems(tmp);
        if (Choice_Drives->GetCount() > 0 && (CheckBox_UploadController->IsChecked() || tmp.size() > 0))
        {
            Button_Upload->Enable();
        }
        else
        {
            Button_Upload->Disable();
        }
#else
        if (wxDir::Exists(DirPickerCtrl_FPPMedia->GetPath()))
        {
            wxDir d;
            d.Open(DirPickerCtrl_FPPMedia->GetPath());
            if (d.HasSubDirs("sequences")) {
                Button_Upload->Enable();
            }
            else
            {
                Button_Upload->Disable();
            }
        }
        else
        {
            Button_Upload->Disable();
        }
#endif
    }

    if (IsValidIP(TextCtrl_IPAddress->GetValue()))
    {
        Button_Console->Enable();
    }
    else
    {
        Button_Console->Disable();
    }
}

bool FPPConnectDialog::FTPUpload()
{
    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    bool cancelled = false;

    wxFTP ftp;
    //ftp.SetPassive(false);
    ftp.SetFlags(wxSOCKET_NOWAIT_WRITE);

    // if you don't use these lines anonymous login will be used
    ftp.SetUser(TextCtr_Username->GetValue());
    ftp.SetPassword(TextCtrl_Password->GetValue());
    if (!ftp.Connect(TextCtrl_IPAddress->GetValue()))
    {
        logger_base.warn("Could not connect to FPP using address '%s'.", (const char *)TextCtrl_IPAddress->GetValue().c_str());
        wxMessageBox("Could not connect to FPP using address '%s'.", (const char *)TextCtrl_IPAddress->GetValue().c_str());
        return true;
    }

    ftp.ChDir("/home/fpp/media");

    if (CheckBox_UploadController->IsChecked())
    {
        ftp.SetAscii();
        cancelled = UploadFile(ftp, (xLightsFrame::CurrentDir + "/universes").ToStdString(), ".", true);
    }

    if (!cancelled && CheckBox_UploadModels->IsChecked())
    {
        ftp.SetAscii();
        cancelled = UploadFile(ftp, (xLightsFrame::CurrentDir + "/channelmemorymaps").ToStdString(), ".", true);
    }


    if (!cancelled)
    {
        ftp.SetBinary();

        wxArrayInt sel;
        CheckListBox_Sequences->GetCheckedItems(sel);
        for (auto it = sel.begin(); it != sel.end() && !cancelled; ++it)
        {
            wxString file = CheckListBox_Sequences->GetString(*it);
            wxString media = "";

            wxXmlDocument doc(file);
            if (doc.IsOk())
            {
                wxXmlNode* root = doc.GetRoot();
                if (root->GetName() == "xsequence")
                {
                    for (auto n = root->GetChildren(); n != nullptr; n = n->GetNext())
                    {
                        if (n->GetName() == "head")
                        {
                            for (auto n1 = n->GetChildren(); n1 != nullptr; n1 = n1->GetNext())
                            {
                                if (n1->GetName() == "mediaFile")
                                {
                                    media = n1->GetNodeContent();
                                    break;
                                }
                            }
                            break;
                        }
                    }
                }
            }

            wxFileName fn(file);
            wxString fseq = fn.GetPath() + "/" + fn.GetName() + ".fseq";
            if (wxFile::Exists(fseq))
            {
                ftp.ChDir("/home/fpp/media");
                cancelled = UploadFile(ftp, fseq.ToStdString(), "sequences", false);
            }

            if (!cancelled && media != "")
            {
                media = xLightsXmlFile::FixFile("", media);
                ftp.ChDir("/home/fpp/media");
                cancelled = UploadFile(ftp, media.ToStdString(), "music", false);
            }
        }
    }

    // gracefully close the connection to the server
    ftp.Close();

    return cancelled;
}

bool FPPConnectDialog::USBUpload()
{
    bool cancelled = false;

    wxArrayInt sel;
    CheckListBox_Sequences->GetCheckedItems(sel);


#ifdef __WXMSW__
    wxString tgtdir = Choice_Drives->GetStringSelection();
#else
    wxString tgtdir = DirPickerCtrl_FPPMedia->GetPath();
#endif

    if (!wxDir::Exists(tgtdir))
    {
        wxMessageBox("USB Drive " + tgtdir + " does not have the expected media folder.", "Error", 4 | wxCENTRE, this);
        return true;
    }
    wxProgressDialog progress("File Copy", "", 1000, this, wxPD_CAN_ABORT | wxPD_APP_MODAL | wxPD_AUTO_HIDE | wxPD_REMAINING_TIME);
    progress.Show();

    int total = CheckBox_UploadController->IsChecked() ? 1 : 0;
    total += sel.size();

    int count = 0;
    if (CheckBox_UploadController->IsChecked())
    {
        cancelled = CopyFile(std::string(xLightsFrame::CurrentDir + "/universes"), std::string(tgtdir + "/universes"), true, progress, 0, 1000 / total);
        count++;
    }

    for (auto it = sel.begin(); it != sel.end() && !cancelled; ++it)
    {
        wxString file = CheckListBox_Sequences->GetString(*it);
        wxString media = "";

        wxXmlDocument doc(file);
        if (doc.IsOk())
        {
            wxXmlNode* root = doc.GetRoot();
            if (root->GetName() == "xsequence")
            {
                for (auto n = root->GetChildren(); n != nullptr; n = n->GetNext())
                {
                    if (n->GetName() == "head")
                    {
                        for (auto n1 = n->GetChildren(); n1 != nullptr; n1 = n1->GetNext())
                        {
                            if (n1->GetName() == "mediaFile")
                            {
                                media = n1->GetNodeContent();
                                break;
                            }
                        }
                        break;
                    }
                }
            }
        }
        wxFileName fn(file);
        wxString fseq = fn.GetPath() + "/" + fn.GetName() + ".fseq";

        int start = count * 1000 / total;
        count++;
        int end = count * 1000 / total;
        int mid = end;
        if (media != "") {
            mid = end  - (end - start) / 5;
        }

        if (!cancelled && wxFile::Exists(fseq))
        {
            cancelled = CopyFile(std::string(fseq), std::string(tgtdir + "/sequences/" + fn.GetName() + ".fseq"), false, progress, start, mid);
        }

        if (!cancelled && media != "")
        {
            media = xLightsXmlFile::FixFile("", media);
            wxFileName fnmedia(media);
            cancelled = CopyFile(std::string(media), std::string(tgtdir + "/music/" + fnmedia.GetName() + "." + fnmedia.GetExt()), false, progress, mid, end);
        }
    }

    return cancelled;
}

void FPPConnectDialog::OnButton_UploadClick(wxCommandEvent& event)
{
    bool cancelled = false;

    SetCursor(wxCURSOR_WAIT);

    Button_Upload->Disable();
    TextCtr_Username->Disable();
    TextCtrl_IPAddress->Disable();
    TextCtrl_Password->Disable();
    CheckListBox_Sequences->Disable();
    Choice_Drives->Disable();
    CheckBox_UploadController->Disable();

    if (Notebook_FPP->GetSelection() == 0)
    {
        cancelled = FTPUpload();
    }
    else
    {
        cancelled  = USBUpload();
    }

    Button_Upload->Enable();
    TextCtr_Username->Enable();
    TextCtrl_IPAddress->Enable();
    TextCtrl_Password->Enable();
    CheckListBox_Sequences->Enable();
    Choice_Drives->Enable();
    CheckBox_UploadController->Enable();

    SetCursor(wxCURSOR_ARROW);

    // remember user name and ip address
    wxConfigBase* config = wxConfigBase::Get();
    config->Write("xLightsPiIP", TextCtrl_IPAddress->GetValue());
    config->Write("xLightsPiUser", TextCtr_Username->GetValue());

    // deliberately not saving password ... while I dont think the password is sensitive others might

    if ((TextCtr_Username->GetValue() == "pi" && TextCtrl_Password->GetValue() == "raspberry") ||
        (TextCtr_Username->GetValue() == "fpp" && TextCtrl_Password->GetValue() == "falcon"))
    {
        config->Write("xLightsPiDefaultPassword", true);
    }
    else
    {
        config->Write("xLightsPiDefaultPassword", false);
    }

    if (!cancelled)
    {
        EndDialog(0);
    }
}

bool FPPConnectDialog::DoCopyFile(const std::string& source, const std::string& target, wxProgressDialog &progress, int start, int end)
{
    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    bool cancelled = false;

    progress.Update(start, source, &cancelled);

    wxFile in;
    in.Open(source);

    if (in.IsOpened())
    {
        wxFile out;
        out.Open(target, wxFile::write);

        if (out.IsOpened())
        {
            wxFileOffset length = in.Length();
            wxFileOffset done = 0;

            uint8_t buffer[8192]; // 8KB at a time
            while (!in.Eof() && !cancelled)
            {
                ssize_t read = in.Read(&buffer[0], sizeof(buffer));
                size_t written = out.Write(&buffer[0], read);
                while (written < read) {
                    written += out.Write(&buffer[written], read - written);
                }
                done += read;

                int prgs = start + (done * (end - start)) / length;
                progress.Update(prgs, wxEmptyString, &cancelled);
                if (!cancelled)
                {
                    cancelled = progress.WasCancelled();
                }
            }
            if (in.Eof())
            {
                progress.Update(end, wxEmptyString);
                logger_base.info("   Copy of file %s done.", (const char *)source.c_str());
            }
            else
            {
                progress.Update(end, wxEmptyString);
                logger_base.warn("   Copy of file %s cancelled.", (const char *)source.c_str());
            }
            in.Close();
            out.Close();

            if (cancelled && wxFile::Exists(target))
            {
                wxRemoveFile(target);
            }
        }
        else
        {
            progress.Update(end, wxEmptyString);
            logger_base.warn("   Copy of file %s failed ... target file %s could not be opened.", (const char *)source.c_str(), (const char *)target.c_str());
        }
    }
    else
    {
        progress.Update(end, wxEmptyString);
        logger_base.warn("   Copy of file %s failed ... file could not be opened.", (const char *)source.c_str());
    }
    return cancelled;
}

bool FPPConnectDialog::CopyFile(std::string source, std::string target, bool backup, wxProgressDialog &progress, int start, int end)
{
    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));

    bool cancelled = false;

    if (backup && wxFile::Exists(target))
    {
        wxDateTime dt = wxDateTime::Now();
        wxString tgtfile = target + "_" + dt.Format("%Y%m%d_%H%M%S");
        logger_base.info("Backing up %s to %s", (const char *)source.c_str(), (const char *)tgtfile.c_str());

        cancelled = DoCopyFile(source, std::string(tgtfile.c_str()), progress, start, end);
    }

    if (!cancelled)
    {
        logger_base.info("Copying file to USB %s to %s", (const char *)source.c_str(), (const char *)target.c_str());
        cancelled = DoCopyFile(source, target, progress, start, end);
    }

    return cancelled;
}

class MySocketOutputStream : public wxSocketOutputStream {
public:
    MySocketOutputStream(wxSocketBase &tmp, MySocketOutputStream *s) : wxSocketOutputStream(tmp) {
        s->m_o_socket->SetFlags(wxSOCKET_NOWAIT_WRITE);
    }
};

bool FPPConnectDialog::UploadFile(wxFTP& ftp, std::string file, std::string folder, bool backup)
{
    wxASSERT(ftp.IsConnected());
    wxASSERT(ftp.IsOk());

    wxProgressDialog progress("FTP Upload", wxString(file.c_str()), 100, this, wxPD_CAN_ABORT | wxPD_APP_MODAL | wxPD_AUTO_HIDE);
    progress.Show();

    bool cancelled = false;

    progress.Update(0, wxEmptyString, &cancelled);

    wxFileName fn(wxString(file.c_str()));
    wxString ext = fn.GetExt();
    if (ext != "") ext = "." + ext;
    wxString basefile = fn.GetName() + ext;

    wxLogNull logNo; //kludge: avoid "error 0" message from wxWidgets
    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));

    //logger_base.info("FTP current directory %s.", (const char *)ftp.Pwd().c_str());

    if (backup)
    {
        int size = ftp.GetFileSize((folder + "/" + basefile).c_str());
        if (size == -1)
        {
            // file not there so no need to backup
        }
        else
        {
            wxDateTime dt = wxDateTime::Now();
            wxString tgtfile = wxString((folder + "/" + basefile).c_str()) + "_" + dt.Format("%Y%m%d_%H%M%S");
            logger_base.info("FTP Backing up file %s to %s.", (const char *)(folder + "/" + basefile).c_str(), (const char *)tgtfile.c_str());
            ftp.Rename((folder + "/" + basefile).c_str(), tgtfile);
            if (!cancelled)
            {
                cancelled = progress.WasCancelled();
            }
        }
    }

    wxFile in;
    in.Open(wxString(file.c_str()));
    if (in.IsOpened())
    {
        logger_base.info("FTP Uploading file %s to %s.", (const char *)file.c_str(), (const char *)(folder + "/" + basefile).c_str());
        wxFileOffset length = in.Length();
        wxFileOffset done = 0;
        wxSocketOutputStream *out = dynamic_cast<wxSocketOutputStream*>(ftp.GetOutputStream((folder + "/" + basefile).c_str()));
        wxSocketBase sock;
        MySocketOutputStream sout(sock, (MySocketOutputStream*)out);
        if (out)
        {
            uint8_t buffer[8192]; // 8KB at a time
            int lastDone = 0;
            while (!in.Eof() && !cancelled)
            {
                ssize_t read = in.Read(&buffer[0], sizeof(buffer));
                done += read;

                int bufPos = 0;
                while (read) {
                    out->Write(&buffer[bufPos], read);
                    ssize_t written = out->LastWrite();
                    bufPos += written;
                    read -= written;
                }
                ssize_t donePct = done * 100;
                donePct = donePct / length;
                if (donePct != lastDone) {
                    lastDone = donePct;
                    cancelled = !progress.Update(donePct, wxEmptyString, &cancelled);
                    wxYield();
                }
            }
            if (in.Eof())
            {
                progress.Update(100, wxEmptyString, &cancelled);
                logger_base.info("   FTP Upload of file %s done.", (const char *)file.c_str());
            }
            else
            {
                progress.Update(100, wxEmptyString, &cancelled);
                logger_base.warn("   FTP Upload of file %s cancelled.", (const char *)file.c_str());
            }
            in.Close();
            out->Close();
            delete out;
            if (ftp.GetFileSize((folder + "/" + basefile).c_str()) != length)
            {
                logger_base.warn("   FTP Upload of file %s failed. Source size (%d) != Destination Size (%d)",(const char *)file.c_str(),length,ftp.GetFileSize((folder + "/" + basefile).c_str()));
            }
        }
        else
        {
            wxMessageBox("FTP Upload of file failed to create the target file.");
            progress.Update(100, wxEmptyString, &cancelled);
            logger_base.error("   FTP Upload of file %s failed as file could not be created on FPP.", (const char *)file.c_str());
        }
    }
    else
    {
        wxMessageBox("FTP Upload of file failed to open the file.");
        progress.Update(100, wxEmptyString, &cancelled);
        logger_base.error("   FTP Upload of file %s failed as file could not be opened.", (const char *)file.c_str());
    }

    if (!cancelled)
    {
        cancelled = progress.WasCancelled();
    }

    return cancelled;
}

void FPPConnectDialog::OnCheckListBox_SequencesToggled(wxCommandEvent& event)
{
    ValidateWindow();
}

void FPPConnectDialog::OnCheckBox_UploadControllerClick(wxCommandEvent& event)
{
    ValidateWindow();
}

void FPPConnectDialog::OnTextCtrl_IPAddressText(wxCommandEvent& event)
{
    ValidateWindow();
}

void FPPConnectDialog::OnTextCtr_UsernameText(wxCommandEvent& event)
{
    ValidateWindow();
}

void FPPConnectDialog::OnTextCtrl_PasswordText(wxCommandEvent& event)
{
    ValidateWindow();
}

void FPPConnectDialog::OnNotebook_FPPPageChanged(wxNotebookEvent& event)
{
    ValidateWindow();
}

void FPPConnectDialog::OnButton_ConsoleClick(wxCommandEvent& event)
{
    ::wxLaunchDefaultBrowser(TextCtrl_IPAddress->GetValue());
}

void FPPConnectDialog::OnFilePickerCtrl_MediaFolderFileChanged(wxFileDirPickerEvent& event)
{
    ValidateWindow();
}
