#include "FPPConnectDialog.h"
#include "xLightsMain.h"
#include <wx/regex.h>
#include "xLightsXmlFile.h"
#include <wx/volume.h>
#include <wx/progdlg.h>

//(*InternalHeaders(FPPConnectDialog)
#include <wx/intl.h>
#include <wx/string.h>
#include <wx/confbase.h>
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
const long FPPConnectDialog::ID_PANEL_USB = wxNewId();
const long FPPConnectDialog::ID_NOTEBOOK_FPP = wxNewId();
const long FPPConnectDialog::ID_CHECKBOX_UploadController = wxNewId();
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
	FlexGridSizer1->AddGrowableRow(2);
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
	TextCtr_Username = new wxTextCtrl(Panel_FTP, ID_TEXTCTRL_Username, _("pi"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_TEXTCTRL_Username"));
	FlexGridSizer2->Add(TextCtr_Username, 1, wxALL|wxEXPAND, 5);
	StaticText3 = new wxStaticText(Panel_FTP, ID_STATICTEXT3, _("Password"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT3"));
	FlexGridSizer2->Add(StaticText3, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	TextCtrl_Password = new wxTextCtrl(Panel_FTP, ID_TEXTCTRL_Password, _("raspberry"), wxDefaultPosition, wxDefaultSize, wxTE_PASSWORD, wxDefaultValidator, _T("ID_TEXTCTRL_Password"));
	FlexGridSizer2->Add(TextCtrl_Password, 1, wxALL|wxEXPAND, 5);
	FlexGridSizer2->Add(0,0,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	Button_Console = new wxButton(Panel_FTP, ID_BUTTON_Console, _("FPP Console"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON_Console"));
	FlexGridSizer2->Add(Button_Console, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	Panel_FTP->SetSizer(FlexGridSizer2);
	FlexGridSizer2->Fit(Panel_FTP);
	FlexGridSizer2->SetSizeHints(Panel_FTP);
	Panel_USB = new wxPanel(Notebook_FPP, ID_PANEL_USB, wxPoint(39,12), wxDefaultSize, wxTAB_TRAVERSAL, _T("ID_PANEL_USB"));
	FlexGridSizer3 = new wxFlexGridSizer(0, 2, 0, 0);
	FlexGridSizer3->AddGrowableCol(1);
	StaticText4 = new wxStaticText(Panel_USB, ID_STATICTEXT4, _("Drive"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT4"));
	FlexGridSizer3->Add(StaticText4, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	Choice_Drives = new wxChoice(Panel_USB, ID_CHOICE_Drives, wxDefaultPosition, wxDefaultSize, 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE_Drives"));
	FlexGridSizer3->Add(Choice_Drives, 1, wxALL|wxEXPAND, 5);
	Panel_USB->SetSizer(FlexGridSizer3);
	FlexGridSizer3->Fit(Panel_USB);
	FlexGridSizer3->SetSizeHints(Panel_USB);
	Notebook_FPP->AddPage(Panel_FTP, _("FTP"), true);
	Notebook_FPP->AddPage(Panel_USB, _("USB"), false);
	FlexGridSizer1->Add(Notebook_FPP, 1, wxALL|wxEXPAND, 5);
	CheckBox_UploadController = new wxCheckBox(this, ID_CHECKBOX_UploadController, _("Upload controller configuration"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX_UploadController"));
	CheckBox_UploadController->SetValue(true);
	FlexGridSizer1->Add(CheckBox_UploadController, 1, wxALL|wxEXPAND, 5);
	CheckListBox_Sequences = new wxCheckListBox(this, ID_CHECKLISTBOX_Sequences, wxDefaultPosition, wxDefaultSize, 0, 0, 0, wxDefaultValidator, _T("ID_CHECKLISTBOX_Sequences"));
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
#else
    // Dan ... not sure how to do the equivalent on Mac/Linux
    Notebook_FPP->RemovePage(1);
#endif

    LoadSequences();

    wxConfigBase* config = wxConfigBase::Get();
    wxString ip;
    config->Read("xLightsPiIP", &ip, "");
    wxString user;
    config->Read("xLightsPiUser", &user, "pi");
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

        int i = CheckListBox_Sequences->FindString(xLightsFrame::CurrentSeqXmlFile->GetLongPath());
        CheckListBox_Sequences->Check(i);
    }

    Fit();
}

bool FPPConnectDialog::IsValidIP(wxString ip)
{
    wxRegEx regxIPAddr("^(([0-9]{1}|[0-9]{2}|[0-1][0-9]{2}|2[0-4][0-9]|25[0-5])\\.){3}([0-9]{1}|[0-9]{2}|[0-1][0-9]{2}|2[0-4][0-9]|25[0-5])$");

    return regxIPAddr.Matches(ip);
}

void FPPConnectDialog::ValidateWindow()
{
    if (wxFile::Exists(xLightsFrame::CurrentDir + "/universes"))
    {
        CheckBox_UploadController->Enable();
    }
    else
    {
        CheckBox_UploadController->SetValue(false);
        CheckBox_UploadController->Disable();
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
    bool cancelled = false;

    wxFTP ftp;
    //ftp.SetPassive(false);

    // if you don't use these lines anonymous login will be used
    ftp.SetUser(TextCtr_Username->GetValue());
    ftp.SetPassword(TextCtrl_Password->GetValue());
    if (!ftp.Connect(TextCtrl_IPAddress->GetValue()))
    {
        wxMessageBox("Count not connect to FPP");
        return true;
    }

    ftp.ChDir("/home/fpp/media");

    if (CheckBox_UploadController->IsChecked())
    {
        ftp.SetAscii();
        cancelled = UploadFile(ftp, (xLightsFrame::CurrentDir + "/universes").ToStdString(), ".", true);

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

    int steps = 0;
    if (CheckBox_UploadController->IsChecked()) steps++;
    steps += 2 * sel.size();
    int done = 0;

    wxProgressDialog progress("USB Copy", "", 100, this, wxPD_CAN_ABORT | wxPD_APP_MODAL | wxPD_AUTO_HIDE);
    progress.ShowModal();

    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));
#ifdef __WXMSW__
    wxString tgtdir = Choice_Drives->GetStringSelection();

    if (!wxDir::Exists(tgtdir))
    {
        wxMessageBox("USB Drive " + Choice_Drives->GetStringSelection() + " does not have the expected media folder.", "Error", 4 | wxCENTRE, this);
        return true;
    }

    if (CheckBox_UploadController->IsChecked())
    {
        progress.Update(0, "universes", &cancelled);

        if (wxFile::Exists(tgtdir + "universes"))
        {
            wxDateTime dt = wxDateTime::Now();
            wxString tgtfile = tgtdir + "universes_" + dt.Format("%Y%m%d_%H%M%S");
            logger_base.info("Backing up %s to %s", (const char *)(tgtdir + "/universes").c_str(), (const char *)tgtfile.c_str());
            wxCopyFile(tgtdir + "/universes", tgtfile);
            if (!cancelled)
            {
                cancelled = progress.WasCancelled();
            }
        }

        logger_base.info("Copying file to USB %s to %s", (const char *)(xLightsFrame::CurrentDir + "/universes").c_str(), (const char *)(tgtdir + "universes").c_str());
        wxCopyFile(xLightsFrame::CurrentDir + "/universes", tgtdir + "universes");
        done++;
        progress.Update(done * 100 / steps, wxEmptyString, &cancelled);
        if (!cancelled)
        {
            cancelled = progress.WasCancelled();
        }
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

        if (!cancelled && wxFile::Exists(fseq))
        {
            logger_base.info("Copying file to USB %s to %s", (const char *)fseq.c_str(), (const char *)(tgtdir + "sequences/" + fn.GetName() + ".fseq").c_str());
            progress.Update(done * 100 / steps, fseq, &cancelled);
            wxCopyFile(fseq, tgtdir + "sequences/" + fn.GetName() + ".fseq");
            if (!cancelled)
            {
                cancelled = progress.WasCancelled();
            }
        }
        done++;
        progress.Update(done * 100 / steps, wxEmptyString, &cancelled);

        if (!cancelled && media != "")
        {
            media = xLightsXmlFile::FixFile("", media);
            wxFileName fnmedia(media);
            progress.Update(done * 100 / steps, media, &cancelled);
            logger_base.info("Copying file to USB %s to %s", (const char *)media.c_str(), (const char *)(tgtdir + "music/" + fnmedia.GetName() + "." + fnmedia.GetExt()).c_str());
            wxCopyFile(media, tgtdir + "music/" + fnmedia.GetName() + "." + fnmedia.GetExt());
            if (!cancelled)
            {
                cancelled = progress.WasCancelled();
            }
        }
        done++;
        progress.Update(done * 100 / steps, wxEmptyString, &cancelled);
    }
    progress.Update(100, wxEmptyString, &cancelled);
#endif

    if (!cancelled)
    {
        cancelled = progress.WasCancelled();
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

bool FPPConnectDialog::UploadFile(wxFTP& ftp, std::string file, std::string folder, bool backup)
{
    wxASSERT(ftp.IsConnected());
    wxASSERT(ftp.IsOk());

    wxProgressDialog progress("FTP Upload", wxString(file.c_str()), 100, this, wxPD_CAN_ABORT | wxPD_APP_MODAL | wxPD_AUTO_HIDE);
    progress.ShowModal();

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
        wxOutputStream *out = ftp.GetOutputStream((folder + "/" + basefile).c_str());
        if (out)
        {
            byte buffer[8192]; // 8KB at a time
            while (!in.Eof() && !cancelled)
            {
                ssize_t read = in.Read(&buffer[0], sizeof(buffer));
                out->Write(&buffer[0], read);
                done += read;
                progress.Update((done * 100) / length, wxEmptyString, &cancelled);
                if (!cancelled)
                {
                    cancelled = progress.WasCancelled();
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
