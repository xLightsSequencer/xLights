//(*InternalHeaders(FPPConnectDialog)
#include <wx/intl.h>
#include <wx/string.h>
//*)

#include <wx/regex.h>
#include <wx/volume.h>
#include <wx/progdlg.h>
#include <wx/config.h>
#include <wx/dir.h>

#include "FPPConnectDialog.h"
#include "xLightsMain.h"
#include "FPP.h"
#include "xLightsXmlFile.h"
#include "outputs/Output.h"
#include "outputs/OutputManager.h"
#include "UtilFunctions.h"
#include "../outputs/ControllerEthernet.h"
#include "ControllerCaps.h"

#include <log4cpp/Category.hh>
#include "../xSchedule/wxJSON/jsonreader.h"

#include "../include/spxml-0.5/spxmlparser.hpp"
#include "../include/spxml-0.5/spxmlevent.hpp"
#include "../FSEQFile.h"
#include "../Parallel.h"

//(*IdInit(FPPConnectDialog)
const long FPPConnectDialog::ID_SCROLLEDWINDOW1 = wxNewId();
const long FPPConnectDialog::ID_STATICTEXT1 = wxNewId();
const long FPPConnectDialog::ID_CHOICE_FILTER = wxNewId();
const long FPPConnectDialog::ID_STATICTEXT2 = wxNewId();
const long FPPConnectDialog::ID_CHOICE_FOLDER = wxNewId();
const long FPPConnectDialog::ID_PANEL2 = wxNewId();
const long FPPConnectDialog::ID_PANEL1 = wxNewId();
const long FPPConnectDialog::ID_SPLITTERWINDOW1 = wxNewId();
const long FPPConnectDialog::ID_BUTTON1 = wxNewId();
const long FPPConnectDialog::ID_BUTTON_Upload = wxNewId();
//*)

const long FPPConnectDialog::ID_MNU_SELECTALL = wxNewId();
const long FPPConnectDialog::ID_MNU_SELECTNONE = wxNewId();
const long FPPConnectDialog::ID_MNU_SELECTHIGH = wxNewId();
const long FPPConnectDialog::ID_MNU_DESELECTHIGH = wxNewId();
const long FPPConnectDialog::ID_FPP_INSTANCE_LIST = wxNewId();

BEGIN_EVENT_TABLE(FPPConnectDialog,wxDialog)
	//(*EventTable(FPPConnectDialog)
	//*)
    EVT_TREELIST_ITEM_CONTEXT_MENU(wxID_ANY, FPPConnectDialog::SequenceListPopup)

END_EVENT_TABLE()


static const std::string CHECK_COL = "ID_UPLOAD_";
static const std::string FSEQ_COL = "ID_FSEQTYPE_";
static const std::string MEDIA_COL = "ID_MEDIA_";
static const std::string MODELS_COL = "ID_MODELS_";
static const std::string UDP_COL = "ID_UDPOUT_";
static const std::string PLAYLIST_COL = "ID_PLAYLIST_";
static const std::string UPLOAD_CONTROLLER_COL = "ID_CONTROLLER_";


static inline int case_insensitive_match(std::string s1, std::string s2) {
   //convert s1 and s2 into lower case strings
   transform(s1.begin(), s1.end(), s1.begin(), ::tolower);
   transform(s2.begin(), s2.end(), s2.begin(), ::tolower);
   if(s1.compare(s2) == 0)
      return 1; //The strings are same
   return 0; //not matched
}

FPPConnectDialog::FPPConnectDialog(wxWindow* parent, OutputManager* outputManager, wxWindowID id,const wxPoint& pos,const wxSize& size)
{
    _outputManager = outputManager;

	//(*Initialize(FPPConnectDialog)
	wxButton* cancelButton;
	wxFlexGridSizer* FlexGridSizer1;
	wxFlexGridSizer* FlexGridSizer2;
	wxFlexGridSizer* FlexGridSizer3;
	wxFlexGridSizer* FlexGridSizer4;

	Create(parent, wxID_ANY, _("FPP Upload"), wxDefaultPosition, wxDefaultSize, wxCAPTION|wxDEFAULT_DIALOG_STYLE|wxRESIZE_BORDER|wxCLOSE_BOX|wxMAXIMIZE_BOX, _T("wxID_ANY"));
	FlexGridSizer1 = new wxFlexGridSizer(0, 1, 0, 0);
	FlexGridSizer1->AddGrowableCol(0);
	FlexGridSizer1->AddGrowableRow(0);
	SplitterWindow1 = new wxSplitterWindow(this, ID_SPLITTERWINDOW1, wxDefaultPosition, wxDefaultSize, wxSP_3D|wxSP_3DSASH, _T("ID_SPLITTERWINDOW1"));
	SplitterWindow1->SetMinSize(wxDLG_UNIT(this,wxSize(550,-1)));
	SplitterWindow1->SetMinimumPaneSize(200);
	SplitterWindow1->SetSashGravity(0.5);
	FPPInstanceList = new wxScrolledWindow(SplitterWindow1, ID_SCROLLEDWINDOW1, wxDefaultPosition, wxDefaultSize, wxVSCROLL|wxHSCROLL, _T("ID_SCROLLEDWINDOW1"));
	FPPInstanceList->SetMinSize(wxDLG_UNIT(SplitterWindow1,wxSize(-1,150)));
	FPPInstanceSizer = new wxFlexGridSizer(0, 10, 0, 0);
	FPPInstanceList->SetSizer(FPPInstanceSizer);
	FPPInstanceSizer->Fit(FPPInstanceList);
	FPPInstanceSizer->SetSizeHints(FPPInstanceList);
	Panel1 = new wxPanel(SplitterWindow1, ID_PANEL1, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("ID_PANEL1"));
	FlexGridSizer2 = new wxFlexGridSizer(2, 1, 0, 0);
	FlexGridSizer2->AddGrowableCol(0);
	FlexGridSizer2->AddGrowableRow(1);
	FlexGridSizer3 = new wxFlexGridSizer(0, 4, 0, 0);
	StaticText1 = new wxStaticText(Panel1, ID_STATICTEXT1, _("Filter:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT1"));
	FlexGridSizer3->Add(StaticText1, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxFIXED_MINSIZE, 5);
	ChoiceFilter = new wxChoice(Panel1, ID_CHOICE_FILTER, wxDefaultPosition, wxDefaultSize, 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE_FILTER"));
	ChoiceFilter->SetSelection( ChoiceFilter->Append(_("Recursive Search")) );
	ChoiceFilter->Append(_("Only Current Directory"));
	FlexGridSizer3->Add(ChoiceFilter, 1, wxALL|wxEXPAND, 5);
	StaticText2 = new wxStaticText(Panel1, ID_STATICTEXT2, _("Folder:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT2"));
	FlexGridSizer3->Add(StaticText2, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxFIXED_MINSIZE, 5);
	ChoiceFolder = new wxChoice(Panel1, ID_CHOICE_FOLDER, wxDefaultPosition, wxDefaultSize, 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE_FOLDER"));
	FlexGridSizer3->Add(ChoiceFolder, 1, wxALL|wxEXPAND, 5);
	FlexGridSizer2->Add(FlexGridSizer3, 1, wxEXPAND, 0);
	CheckListBoxHolder = new wxPanel(Panel1, ID_PANEL2, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("ID_PANEL2"));
	CheckListBoxHolder->SetMinSize(wxSize(-1,100));
	FlexGridSizer2->Add(CheckListBoxHolder, 1, wxALL|wxEXPAND, 0);
	Panel1->SetSizer(FlexGridSizer2);
	FlexGridSizer2->Fit(Panel1);
	FlexGridSizer2->SetSizeHints(Panel1);
	SplitterWindow1->SplitHorizontally(FPPInstanceList, Panel1);
	FlexGridSizer1->Add(SplitterWindow1, 1, wxALL|wxEXPAND, 5);
	FlexGridSizer4 = new wxFlexGridSizer(0, 5, 0, 0);
	AddFPPButton = new wxButton(this, ID_BUTTON1, _("Add FPP"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON1"));
	FlexGridSizer4->Add(AddFPPButton, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer4->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	wxSize __SpacerSize_1 = wxDLG_UNIT(this,wxSize(50,-1));
	FlexGridSizer4->Add(__SpacerSize_1.GetWidth(),__SpacerSize_1.GetHeight(),1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	Button_Upload = new wxButton(this, ID_BUTTON_Upload, _("Upload"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON_Upload"));
	FlexGridSizer4->Add(Button_Upload, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	cancelButton = new wxButton(this, wxID_CANCEL, _("Cancel"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("wxID_CANCEL"));
	FlexGridSizer4->Add(cancelButton, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer1->Add(FlexGridSizer4, 1, wxALL|wxEXPAND, 5);
	SetSizer(FlexGridSizer1);
	FlexGridSizer1->Fit(this);
	FlexGridSizer1->SetSizeHints(this);

	Connect(ID_CHOICE_FILTER,wxEVT_COMMAND_CHOICE_SELECTED,(wxObjectEventFunction)&FPPConnectDialog::OnChoiceFilterSelect);
	Connect(ID_CHOICE_FOLDER,wxEVT_COMMAND_CHOICE_SELECTED,(wxObjectEventFunction)&FPPConnectDialog::OnChoiceFolderSelect);
	Connect(ID_BUTTON1,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&FPPConnectDialog::OnAddFPPButtonClick);
	Connect(ID_BUTTON_Upload,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&FPPConnectDialog::OnButton_UploadClick);
	Connect(wxID_ANY,wxEVT_CLOSE_WINDOW,(wxObjectEventFunction)&FPPConnectDialog::OnClose);
	//*)

    
    CheckListBox_Sequences = new wxTreeListCtrl(Panel1, wxID_ANY,
                                                wxDefaultPosition, wxDefaultSize,
                                                wxTL_CHECKBOX | wxTL_MULTIPLE, "ID_TREELISTVIEW_SEQUENCES");
    CheckListBox_Sequences->SetMinSize(wxSize(-1,100));
    CheckListBox_Sequences->AppendColumn("Sequence", wxCOL_WIDTH_AUTOSIZE,
                                         wxALIGN_LEFT,
                                         wxCOL_RESIZABLE | wxCOL_SORTABLE);
    CheckListBox_Sequences->AppendColumn("Modified Date", wxCOL_WIDTH_AUTOSIZE,
                                         wxALIGN_LEFT,
                                         wxCOL_RESIZABLE | wxCOL_SORTABLE);
    CheckListBox_Sequences->AppendColumn("Media", wxCOL_WIDTH_AUTOSIZE,
                                         wxALIGN_LEFT,
                                         wxCOL_RESIZABLE | wxCOL_SORTABLE);
    CheckListBox_Sequences->SetSortColumn(0, true);
    
    
    FlexGridSizer2->Replace(CheckListBoxHolder, CheckListBox_Sequences, true);
    
    CheckListBoxHolder->Destroy();
    
    FlexGridSizer2->Layout();
    FlexGridSizer1->Fit(this);
    FlexGridSizer1->SetSizeHints(this);


    wxProgressDialog prgs("Discovering FPP Instances",
                          "Discovering FPP Instances", 100, parent);
    prgs.Pulse("Discovering FPP Instances");
    prgs.Show();


    std::list<std::string> startAddresses;

    wxConfigBase* config = wxConfigBase::Get();
    wxString force;
    if (config->Read("FPPConnectForcedIPs", &force)) {
        wxArrayString ips = wxSplit(force, '|');
        wxString newForce;
        for (const auto &a : ips) {
            startAddresses.push_back(a);
        }
    }
    FPP::Discover(startAddresses, instances);
    wxString newForce = "";
    for (const auto &a : startAddresses) {
        for (const auto& fpp : instances) {
            if (case_insensitive_match(a, fpp->hostName) || case_insensitive_match(a, fpp->ipAddress)) {
                if (newForce != "") {
                    newForce.append(",");
                }
                newForce.append(a);
            }
        }
    }
    if (newForce != force) {
        config->Write("FPPConnectForcedIPs", newForce);
        config->Flush();
    }

    prgs.Pulse("Checking for mounted media drives");
    CreateDriveList();
    prgs.Update(100);
    prgs.Hide();

    AddInstanceHeader("Upload", "Enable to Upload Files/Configs to this FPP Device.");
    AddInstanceHeader("Location", "Host and IP Address.");
    AddInstanceHeader("Description");
    AddInstanceHeader("Version", "FPP Software Version.");
    AddInstanceHeader("FSEQ Type", "FSEQ File Version. FPP 2.6 required for V2 formats.");
    AddInstanceHeader("Media", "Enable to Upload MP3, MP4 Media Files.");
    AddInstanceHeader("Models", "Enable to Upload Models for Dispaly Testing.");
    AddInstanceHeader("UDP Out", "'All' Uploads All E1.31/DDP Ouput Definitions, 'Proxied' Upload E1.31/DDP Ouput Definitions of Proxied Controllers.");
    AddInstanceHeader("Playlist","Select Playlist to Add Uploaded Sequences Too.");
    AddInstanceHeader("Pixel Hat/Cape", "Display Hat or Hat Attached to FPP Device, If Found.");

    PopulateFPPInstanceList();
    GetFolderList(xLightsFrame::CurrentDir);

    if (config != nullptr) {
        int filterSelect = -1;
        wxString folderSelect = "";
        config->Read("FPPConnectFilterSelection", &filterSelect);
        config->Read("FPPConnectFolderSelection", &folderSelect);
        if (filterSelect != wxNOT_FOUND)
        {
            ChoiceFilter->SetSelection(filterSelect);
        }
        int ifoldSelect = ChoiceFolder->FindString(folderSelect);
        if (ifoldSelect != wxNOT_FOUND)
        {
            ChoiceFolder->SetSelection(ifoldSelect);
        }
        else
        {
            ChoiceFolder->SetSelection(0);
        }
    }

    LoadSequences();

    int h = SplitterWindow1->GetSize().GetHeight();
    h *= 33;
    h /= 100;
    SplitterWindow1->SetSashPosition(h);
    
    SetSizer(FlexGridSizer1);
    FlexGridSizer1->Fit(this);
    FlexGridSizer1->SetSizeHints(this);
}

void FPPConnectDialog::PopulateFPPInstanceList() {
    FPPInstanceList->Freeze();
    //remove all the children from the first upload checkbox on
    wxWindow *w = FPPInstanceList->FindWindow(CHECK_COL + "0");
    while (w) {
        wxWindow *tmp = w->GetNextSibling();
        w->Destroy();
        w = tmp;
    }
    while (FPPInstanceSizer->GetItemCount () > 10) {
        FPPInstanceSizer->Remove(10);
    }

    int row = 0;
    for (const auto& inst : instances) {
        std::string rowStr = std::to_string(row);
        wxCheckBox *CheckBox1 = new wxCheckBox(FPPInstanceList, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, CHECK_COL + rowStr);
        CheckBox1->SetValue(true);
        FPPInstanceSizer->Add(CheckBox1, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 1);
        std::string l = inst->hostName + " - " + inst->ipAddress;
        wxStaticText *label = new wxStaticText(FPPInstanceList, wxID_ANY, l, wxDefaultPosition, wxDefaultSize, 0, _T("ID_LOCATION_" + rowStr));
        FPPInstanceSizer->Add(label, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 1);
        label = new wxStaticText(FPPInstanceList, wxID_ANY, inst->description, wxDefaultPosition, wxDefaultSize, 0, _T("ID_DESCRIPTION_" + rowStr));
        FPPInstanceSizer->Add(label, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 1);
        label = new wxStaticText(FPPInstanceList, wxID_ANY, inst->fullVersion, wxDefaultPosition, wxDefaultSize, 0, _T("ID_VERSION_" + rowStr));
        FPPInstanceSizer->Add(label, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 1);

        //FSEQ Type listbox
        if (inst->IsVersionAtLeast(2, 6)) {
            wxChoice *Choice1 = new wxChoice(FPPInstanceList, wxID_ANY, wxDefaultPosition, wxDefaultSize, 0, 0, 0, wxDefaultValidator, FSEQ_COL + rowStr);
            wxFont font = Choice1->GetFont();
            font.SetPointSize(font.GetPointSize() - 2);
            Choice1->SetFont(font);
            Choice1->Append(_("V1"));
            Choice1->Append(_("V2"));
            Choice1->Append(_("V2 Sparse/zstd"));
            Choice1->Append(_("V2 Sparse/Uncompressed"));
            Choice1->SetSelection(inst->mode == "master" ? 1 : 2);
            FPPInstanceSizer->Add(Choice1, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 0);
        } else {
            label = new wxStaticText(FPPInstanceList, wxID_ANY, "V1", wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATIC_TEXT_FS_" + rowStr));
            FPPInstanceSizer->Add(label, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 1);
        }

        CheckBox1 = new wxCheckBox(FPPInstanceList, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, MEDIA_COL + rowStr);
        CheckBox1->SetValue(inst->mode != "remote");
        FPPInstanceSizer->Add(CheckBox1, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 1);
        if (inst->IsVersionAtLeast(2, 6)) {
            CheckBox1 = new wxCheckBox(FPPInstanceList, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, MODELS_COL + rowStr);
            CheckBox1->SetValue(false);
            FPPInstanceSizer->Add(CheckBox1, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 1);
        } else {
            FPPInstanceSizer->Add(0,0,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 1);
        }
        if (inst->IsVersionAtLeast(2, 0)) {
            wxChoice *Choice1 = new wxChoice(FPPInstanceList, wxID_ANY, wxDefaultPosition, wxDefaultSize, 0, 0, 0, wxDefaultValidator, UDP_COL + rowStr);
            wxFont font = Choice1->GetFont();
            font.SetPointSize(font.GetPointSize() - 2);
            Choice1->SetFont(font);

            Choice1->Append(_("None"));
            Choice1->Append(_("All"));
            Choice1->Append(_("Proxied"));
            Choice1->SetSelection(0);
            FPPInstanceSizer->Add(Choice1, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 1);
        } else {
            FPPInstanceSizer->Add(0,0,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 1);
        }

        //playlist combo box
        if (inst->IsVersionAtLeast(2, 6) && !inst->IsDrive()) {
            wxComboBox *ComboBox1 = new wxComboBox(FPPInstanceList, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, 0, wxTE_PROCESS_ENTER, wxDefaultValidator, PLAYLIST_COL + rowStr);
            std::list<std::string> playlists;
            inst->LoadPlaylists(playlists);
            ComboBox1->Append(_(""));
            for (const auto& pl : playlists) {
                ComboBox1->Append(pl);
            }
            wxFont font = ComboBox1->GetFont();
            font.SetPointSize(font.GetPointSize() - 2);
            ComboBox1->SetFont(font);
            FPPInstanceSizer->Add(ComboBox1, 1, wxALL|wxEXPAND, 0);
        } else {
            FPPInstanceSizer->Add(0,0,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 1);
        }

        std::string m = FPP::GetModel(inst->pixelControllerType);

        if (m != "") {
            std::string desc = m;
            if (inst->panelSize != "") {
                desc += " - " + inst->panelSize;
            }
            CheckBox1 = new wxCheckBox(FPPInstanceList, wxID_ANY, desc, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, UPLOAD_CONTROLLER_COL + rowStr);
            CheckBox1->SetValue(false);
            FPPInstanceSizer->Add(CheckBox1, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 1);
        } else {
            FPPInstanceSizer->Add(0,0,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 1);
        }

        row++;
    }
    ApplySavedHostSettings();

    FPPInstanceList->FitInside();
    FPPInstanceList->SetScrollRate(10, 10);
    FPPInstanceList->ShowScrollbars(wxSHOW_SB_ALWAYS, wxSHOW_SB_ALWAYS);
    FPPInstanceList->Thaw();
}

void FPPConnectDialog::AddInstanceHeader(const std::string &h, const std::string &tt) {
    wxPanel *Panel1 = new wxPanel(FPPInstanceList, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxSIMPLE_BORDER|wxTAB_TRAVERSAL, _T("ID_PANEL1"));
    wxBoxSizer *BoxSizer1 = new wxBoxSizer(wxHORIZONTAL);
    wxStaticText *StaticText3 = new wxStaticText(Panel1, wxID_ANY, h, wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT1"));
    if(!tt.empty())
        StaticText3->SetToolTip ( tt );
    BoxSizer1->Add(StaticText3, 1, wxLEFT|wxRIGHT|wxEXPAND, 5);
    Panel1->SetSizer(BoxSizer1);
    BoxSizer1->Fit(Panel1);
    BoxSizer1->SetSizeHints(Panel1);
    FPPInstanceSizer->Add(Panel1, 1, wxALL|wxEXPAND, 0);
}

void FPPConnectDialog::AddInstanceRow(const FPP &inst) {

}

void FPPConnectDialog::OnPopup(wxCommandEvent &event)
{
    int id = event.GetId();
    wxTreeListItem item = CheckListBox_Sequences->GetFirstItem();
    while (item.IsOk()) {
        bool isChecked = CheckListBox_Sequences->GetCheckedState(item) == wxCHK_CHECKED;
        bool isSelected = CheckListBox_Sequences->IsSelected(item);
        if (id == ID_MNU_SELECTALL && !isChecked) {
            CheckListBox_Sequences->CheckItem(item);
        } else if (id == ID_MNU_SELECTNONE && isChecked) {
            CheckListBox_Sequences->UncheckItem(item);
        } else if (id == ID_MNU_SELECTHIGH && !isChecked && isSelected) {
            CheckListBox_Sequences->CheckItem(item);
        } else if (id == ID_MNU_SELECTHIGH && isChecked && isSelected) {
            CheckListBox_Sequences->UncheckItem(item);
        }
        item = CheckListBox_Sequences->GetNextItem(item);
    }
}

FPPConnectDialog::~FPPConnectDialog()
{
	//(*Destroy(FPPConnectDialog)
	//*)

    for (const auto& a : instances) {
        delete a;
    }
}

void FPPConnectDialog::LoadSequencesFromFolder(wxString dir) const
{
    wxLogNull logNo; //kludge: avoid "error 0" message from wxWidgets
    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    logger_base.info("Scanning folder for sequences for FPP upload: %s", (const char *)dir.c_str());

    const wxString fseqDir = xLightsFrame::FseqDir;

    wxDir directory;
    directory.Open(dir);

    wxString file;
    bool fcont = directory.GetFirst(&file, "*.x*");
    static const int BUFFER_SIZE = 1024*12;
    std::vector<char> buf(BUFFER_SIZE); //12K buffer
    while (fcont) {
        if (file != "xlights_rgbeffects.xml" && file != OutputManager::GetNetworksFileName() && file != "xlights_keybindings.xml" &&
            (file.Lower().EndsWith("xml") || file.Lower().EndsWith("xsq"))) {
            // this could be a sequence file ... lets open it and check
            //just check if <xsequence" is in the first 512 bytes, parsing every XML is way too expensive
            wxFile doc(dir + wxFileName::GetPathSeparator() + file);
            SP_XmlPullParser *parser = new SP_XmlPullParser();
            size_t read = doc.Read(&buf[0], BUFFER_SIZE);
            parser->append(&buf[0], read);
            SP_XmlPullEvent * event = parser->getNext();
            int done = 0;
            int count = 0;
            bool isSequence = false;
            bool isMedia = false;
            std::string mediaName;


            while (!done) {
                if (!event) {
                    size_t read2 = doc.Read(&buf[0], BUFFER_SIZE);
                    if (read2 == 0) {
                        done = true;
                    } else {
                        parser->append(&buf[0], read2);
                    }
                } else {
                    switch (event->getEventType()) {
                        case SP_XmlPullEvent::eEndDocument:
                            done = true;
                            break;
                        case SP_XmlPullEvent::eStartTag: {
                                SP_XmlStartTagEvent * stagEvent = (SP_XmlStartTagEvent*)event;
                                wxString NodeName = wxString::FromAscii(stagEvent->getName());
                                count++;
                                if (NodeName == "xsequence") {
                                    isSequence = true;
                                } else if (NodeName == "mediaFile") {
                                    isMedia = true;
                                } else {
                                    isMedia = false;
                                }
                                if (count == 100) {
                                    //media file will be very early in the file, dont waste time;
                                    done = true;
                                }
                            }
                            break;
                        case SP_XmlPullEvent::eCData:
                            if (isMedia) {
                                SP_XmlCDataEvent * stagEvent = (SP_XmlCDataEvent*)event;
                                mediaName = wxString::FromAscii(stagEvent->getText()).ToStdString();
                                done = true;
                            }
                            break;
                    }
                }
                if (!done) {
                    event = parser->getNext();
                }
            }
            delete parser;

            xLightsFrame* frame = static_cast<xLightsFrame*>(GetParent());

            std::string fseqName = frame->GetFseqDirectory() + wxFileName::GetPathSeparator() + file.substr(0, file.length() - 4) + ".fseq";
            if (isSequence) {
                //need to check for existence of fseq
                if (!wxFile::Exists(fseqName)) {
                    fseqName = dir + wxFileName::GetPathSeparator() + file.substr(0, file.length() - 4) + ".fseq";
                }
                if (!wxFile::Exists(fseqName)) {
                    isSequence = false;
                }
            }
            if (mediaName != "") {
                if (!wxFile::Exists(mediaName)) {
                    wxFileName fn(mediaName);
                    std::string tmn = frame->mediaDirectory + wxFileName::GetPathSeparator() + fn.GetFullName();
                    if (wxFile::Exists(tmn)) {
                        mediaName = tmn;
                    } else {
                        const std::string fixedMN = FixFile(frame->CurrentDir, mediaName);
                        if (!wxFile::Exists(fixedMN)) {
                            logger_base.info("Could not find media: %s  OR   %s   OR   %s", mediaName.c_str(), tmn.c_str(), fixedMN.c_str());
                            mediaName = "";
                        } else {
                            mediaName = fixedMN;
                        }
                    }
                }
            }
            logger_base.debug("XML:  %s   IsSeq:  %d    FSEQ:  %s   Media:  %s", file.ToStdString().c_str(), isSequence, fseqName.c_str(), mediaName.c_str());
            if (isSequence) {
                wxTreeListItem item = CheckListBox_Sequences->AppendItem(CheckListBox_Sequences->GetRootItem(),
                                                                         fseqName);
                
                DisplayDateModified(fseqName, item);
                if (mediaName != "") {
                    CheckListBox_Sequences->SetItemText(item, 2, mediaName);
                }
            }
        }
        fcont = directory.GetNext(&file);
    }
    if (ChoiceFilter->GetSelection() == 0)
    {
        fcont = directory.GetFirst(&file, wxEmptyString, wxDIR_DIRS);
        while (fcont)
        {
            if (file != "Backup")
            {
                LoadSequencesFromFolder(dir + wxFileName::GetPathSeparator() + file);
            }
            fcont = directory.GetNext(&file);
        }
    }
}

void FPPConnectDialog::LoadSequences()
{
    CheckListBox_Sequences->DeleteAllItems();
    xLightsFrame* frame = static_cast<xLightsFrame*>(GetParent());
    wxString freqDir = frame->GetFseqDirectory();

    if (ChoiceFolder->GetSelection() == 0) {
        LoadSequencesFromFolder(xLightsFrame::CurrentDir);
    }
    else {
        const wxString folder = ChoiceFolder->GetString(ChoiceFolder->GetSelection());
        LoadSequencesFromFolder(xLightsFrame::CurrentDir + wxFileName::GetPathSeparator() + folder);
        freqDir = xLightsFrame::CurrentDir + wxFileName::GetPathSeparator() + folder;
    }

    wxDir directory;
    directory.Open(freqDir);

    wxString file;
    bool fcont = directory.GetFirst(&file, "*.?seq");
    while (fcont) {
        wxString v = freqDir + wxFileName::GetPathSeparator() + file;
        
        wxTreeListItem item = CheckListBox_Sequences->GetFirstItem();
        bool found = false;
        while (item.IsOk()) {
            if (v == CheckListBox_Sequences->GetItemText(item)) {
                found = true;
            }
            item = CheckListBox_Sequences->GetNextItem(item);
        }
        if (!found) {
            wxTreeListItem item = CheckListBox_Sequences->AppendItem(CheckListBox_Sequences->GetRootItem(), v);
            DisplayDateModified(v, item);
        }
        fcont = directory.GetNext(&file);
    }

    if (xLightsFrame::CurrentSeqXmlFile != nullptr) {
        wxString curSeq = xLightsFrame::CurrentSeqXmlFile->GetLongPath();
        if (!curSeq.StartsWith(xLightsFrame::CurrentDir)) {
            LoadSequencesFromFolder(curSeq);
        }
        wxTreeListItem item = CheckListBox_Sequences->GetFirstItem();
        while (item.IsOk()) {
            if (curSeq == CheckListBox_Sequences->GetItemText(item)) {
                CheckListBox_Sequences->CheckItem(item);
                break;
            }
            item = CheckListBox_Sequences->GetNextItem(item);
        }
    }

    wxConfigBase* config = wxConfigBase::Get();
    if (config != nullptr) {
        const wxString itcsv = config->Read("FPPConnectSelectedSequences", wxEmptyString);

        if (!itcsv.IsEmpty()) {
            wxArrayString savedUploadItems = wxSplit(itcsv, ',');

            wxTreeListItem item = CheckListBox_Sequences->GetFirstItem();
            while (item.IsOk()) {
                if (savedUploadItems.Index(CheckListBox_Sequences->GetItemText(item)) != wxNOT_FOUND) {
                    CheckListBox_Sequences->CheckItem(item);
                }
                item = CheckListBox_Sequences->GetNextItem(item);
            }
        }
    }

    CheckListBox_Sequences->SetColumnWidth(2, wxCOL_WIDTH_AUTOSIZE);
    CheckListBox_Sequences->SetColumnWidth(1, wxCOL_WIDTH_AUTOSIZE);
    CheckListBox_Sequences->SetColumnWidth(0, wxCOL_WIDTH_AUTOSIZE);
}

void FPPConnectDialog::OnButton_UploadClick(wxCommandEvent& event)
{
    static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    bool cancelled = false;

    std::vector<bool> doUpload(instances.size());
    int row = 0;
    for (row = 0; row < doUpload.size(); ++row) {
        std::string rowStr = std::to_string(row);
        doUpload[row] = GetCheckValue(CHECK_COL + rowStr);
        if (doUpload[row]) {

        }
    }
    row = 0;
    xLightsFrame* frame = static_cast<xLightsFrame*>(GetParent());

    std::list<ControllerEthernet*> outputControllers;
    for (const auto& it : _outputManager->GetControllers())
    {
        auto eth = dynamic_cast<ControllerEthernet*>(it);
        if (eth != nullptr)
        {
            if (eth->GetProtocol() != OUTPUT_ZCPP)
            {
                outputControllers.push_back(eth);
            }
        }
    }

    wxJSONValue outputs = FPP::CreateUniverseFile(outputControllers, false);
    wxProgressDialog prgs("", "", 1001, this, wxPD_CAN_ABORT | wxPD_APP_MODAL | wxPD_AUTO_HIDE);
    for (const auto& inst : instances) {
        inst->progressDialog = &prgs;
        inst->parent = this;
        std::string rowStr = std::to_string(row);
        if (!cancelled && doUpload[row]) {
            std::string playlist = GetChoiceValue(PLAYLIST_COL + rowStr);
            if (playlist != "") {
                cancelled |= inst->UploadPlaylist(playlist);
            }
            if (GetChoiceValueIndex(UDP_COL + rowStr) == 1) {
                cancelled |= inst->UploadUDPOut(outputs);
                inst->SetRestartFlag();
            } else if (GetChoiceValueIndex(UDP_COL + rowStr) == 2) {
                cancelled |= inst->UploadUDPOutputsForProxy(_outputManager);
                inst->SetRestartFlag();
            }
            if (GetCheckValue(UPLOAD_CONTROLLER_COL + rowStr)) {
                auto vendor = FPP::GetVendor(inst->pixelControllerType);
                auto model = FPP::GetModel(inst->pixelControllerType);
                //auto caps = ControllerCaps::GetControllerConfig(vendor, model, "");
                auto c = _outputManager->GetControllers(inst->ipAddress);
                if (c.size() == 1)
                {
                    cancelled |= inst->UploadPixelOutputs(&frame->AllModels, _outputManager, c.front());
                    inst->SetRestartFlag();
                }
            }
        }
        row++;
    }
    
    wxTreeListItem item = CheckListBox_Sequences->GetFirstItem();
    while (item.IsOk()) {
        if (CheckListBox_Sequences->GetCheckedState(item) == wxCHK_CHECKED) {
            std::string fseq = CheckListBox_Sequences->GetItemText(item);
            std::string media = CheckListBox_Sequences->GetItemText(item, 2);

            FSEQFile *seq = FSEQFile::openFSEQFile(fseq);
            if (seq) {
                row = 0;
                for (const auto& inst : instances) {
                    std::string rowStr = std::to_string(row);
                    if (!cancelled && doUpload[row]) {
                        std::string m2 = media;
                        if (!GetCheckValue(MEDIA_COL + rowStr)) {
                            m2 = "";
                        }

                        int fseqType = GetChoiceValueIndex(FSEQ_COL + rowStr);
                        cancelled |= inst->PrepareUploadSequence(*seq,
                                                                fseq, m2,
                                                                fseqType);
                    }
                    row++;
                }
                if (!cancelled) {
                    prgs.SetTitle("Generating FSEQ Files");
                    prgs.Update(0, "Generating " + fseq, &cancelled);
                    prgs.Show();
                    int lastDone = 0;
                    static const int FRAMES_TO_BUFFER = 50;
                    std::vector<std::vector<uint8_t>> frames(FRAMES_TO_BUFFER);
                    for (size_t x = 0; x < frames.size(); x++) {
                        frames[x].resize(seq->getMaxChannel() + 1);
                    }

                    for (size_t frame = 0; frame < seq->getNumFrames() && !cancelled; frame++) {
                        int donePct = frame * 1000 / seq->getNumFrames();
                        if (donePct != lastDone) {
                            lastDone = donePct;
                            cancelled |= !prgs.Update(donePct, "Generating " + fseq, &cancelled);
                            wxYield();
                        }

                        int lastBuffered = 0;
                        size_t startFrame = frame;
                        //Read a bunch of frames so each parallel thread has more info to work with before returning out here
                        while (lastBuffered < FRAMES_TO_BUFFER && frame < seq->getNumFrames()) {
                            FSEQFile::FrameData *f = seq->getFrame(frame);
                            if (f != nullptr)
                            {
                                if (!f->readFrame(&frames[lastBuffered][0], frames[lastBuffered].size()))
                                {
                                    logger_base.error("FPPConnect FSEQ file corrupt.");
                                }
                                delete f;
                            }
                            lastBuffered++;
                            frame++;
                        }
                        frame--;
                        std::function<void(FPP * &, int)> func = [startFrame, lastBuffered, &frames, &doUpload](FPP* &inst, int row) {
                            if (doUpload[row]) {
                                for (int x = 0; x < lastBuffered; x++) {
                                    inst->AddFrameToUpload(startFrame + x, &frames[x][0]);
                                }
                            }
                        };
                        parallel_for(instances, func);
                    }
                }
                row = 0;
                for (const auto &inst : instances) {
                    if (!cancelled && doUpload[row]) {
                        cancelled |= inst->FinalizeUploadSequence();
                    }
                    row++;
                }
            }
            delete seq;
        }
        item = CheckListBox_Sequences->GetNextItem(item);
    }
    row = 0;
    
    wxJSONValue memoryMaps = FPP::CreateModelMemoryMap(&frame->AllModels);
    std::string displayMap = FPP::CreateVirtualDisplayMap(&frame->AllModels, frame->GetDisplay2DCenter0());
    for (const auto& inst : instances) {
        std::string rowStr = std::to_string(row);
        if (!cancelled && doUpload[row]) {
            std::string playlist = GetChoiceValue(PLAYLIST_COL + rowStr);
            if (playlist != "") {
                cancelled |= inst->UploadPlaylist(playlist);
            }
            if (GetCheckValue(MODELS_COL + rowStr)) {
                cancelled |= inst->UploadModels(memoryMaps);
                cancelled |= inst->UploadDisplayMap(displayMap);
                inst->SetRestartFlag();
            }
        }
        row++;
    }
    prgs.Update(1001);
    prgs.Hide();
    if (!cancelled) {
        SaveSettings();
        EndDialog(wxID_CLOSE);
    }
}

void FPPConnectDialog::CreateDriveList()
{
    wxArrayString drives;
#ifdef __WXMSW__
    wxArrayString ud = wxFSVolume::GetVolumes(wxFS_VOL_REMOVABLE | wxFS_VOL_MOUNTED, 0);
    for (const auto &a : ud) {
        if (wxDir::Exists(a + "\\sequences")) {
            drives.push_back(a);
        }
    }
#elif defined(__WXOSX__)
    wxDir d;
    d.Open("/Volumes");
    wxString dir;
    bool fcont = d.GetFirst(&dir, wxEmptyString, wxDIR_DIRS);
    while (fcont)
    {
        if (wxDir::Exists("/Volumes/" + dir + "/sequences")) { //raw USB drive mounted
            drives.push_back("/Volumes/" + dir + "/");
        } else if (wxDir::Exists("/Volumes/" + dir + "/media/sequences")) { // 1.x Mounted via SMB/NFS
            drives.push_back("/Volumes/" + dir + "/media/");
        }
        fcont = d.GetNext(&dir);
    }
#else
    bool done = false;
    wxDir d;
    d.Open("/media");
    wxString dir;
    bool fcont = d.GetFirst(&dir, wxEmptyString, wxDIR_DIRS);
    while (fcont) {
        wxDir d2;
        d2.Open("/media/" + dir);
        wxString dir2;
        bool fcont2 = d2.GetFirst(&dir2, wxEmptyString, wxDIR_DIRS);
        while (fcont2) {
            if (dir2 == "sequences") {
                drives.push_back("/media/" + dir + "/" + dir2);
            } else if (wxDir::Exists("/media/" + dir + "/" + dir2 + "/sequences")) {
                drives.push_back("/media/" + dir + "/" + dir2);
            }
            fcont2 = d2.GetNext(&dir2);
        }
        fcont = d.GetNext(&dir);
    }
#endif

    for (const auto& a : drives) {
        FPP *inst = new FPP();
        inst->hostName = "FPP";
        inst->ipAddress = a;
        inst->minorVersion = 0;
        inst->majorVersion = 2;
        inst->fullVersion = "Unknown";
        inst->mode = "standalone";
        if (wxFile::Exists(a + "/fpp-info.json")) {
            //read version and hostname
            wxJSONValue system;
            wxJSONReader reader;
            wxString str;
            wxFile(a + "/fpp-info.json").ReadAll(&str);
            reader.Parse(str, &system);

            if (!system["hostname"].IsNull()) {
                inst->hostName = system["hostname"].AsString();
            }
            if (!system["type"].IsNull()) {
                inst->platform = system["type"].AsString();
            }
            if (!system["model"].IsNull()) {
                inst->model = system["model"].AsString();
            }
            if (!system["version"].IsNull()) {
                inst->fullVersion = system["version"].AsString();
            }
            if (system["minorVersion"].IsInt()) {
                inst->minorVersion = system["minorVersion"].AsInt();
            }
            if (system["majorVersion"].IsInt()) {
                inst->majorVersion = system["majorVersion"].AsInt();
            }
            if (!system["channelRanges"].IsNull()) {
                inst->ranges = system["channelRanges"].AsString();
            }
            if (!system["HostDescription"].IsNull()) {
                inst->description = system["HostDescription"].AsString();
            }
            if (!system["fppModeString"].IsNull()) {
                inst->mode = system["fppModeString"].AsString();
            }
        }
        instances.push_back(inst);
    }
}

bool FPPConnectDialog::GetCheckValue(const std::string &col) {
    wxWindow *w = FPPInstanceList->FindWindow(col);
    if (w) {
        wxCheckBox *cb = dynamic_cast<wxCheckBox*>(w);
        if (cb) {
            return cb->GetValue();
        }
    }
    return false;
}
int FPPConnectDialog::GetChoiceValueIndex(const std::string &col) {
    wxWindow *w = FPPInstanceList->FindWindow(col);
    if (w) {
        wxItemContainer *cb = dynamic_cast<wxItemContainer*>(w);
        if (cb) {
            return cb->GetSelection();
        }
    }
    return 0;
}

std::string FPPConnectDialog::GetChoiceValue(const std::string &col) {
    wxWindow *w = FPPInstanceList->FindWindow(col);
    if (w) {
        wxComboBox *comboBox = dynamic_cast<wxComboBox*>(w);
        if (comboBox) {
            return comboBox->GetValue();
        }

        wxItemContainer *cb = dynamic_cast<wxItemContainer*>(w);
        if (cb) {
            return cb->GetStringSelection();
        }
    }
    return "";
}
void FPPConnectDialog::SetChoiceValueIndex(const std::string &col, int i) {
    wxWindow *w = FPPInstanceList->FindWindow(col);
    if (w) {
        wxItemContainer *cb = dynamic_cast<wxItemContainer*>(w);
        if (cb) {
            return cb->SetSelection(i);
        }
    }
}
void FPPConnectDialog::SetCheckValue(const std::string &col, bool b) {
    wxWindow *w = FPPInstanceList->FindWindow(col);
    if (w) {
        wxCheckBox *cb = dynamic_cast<wxCheckBox*>(w);
        if (cb) {
            return cb->SetValue(b);
        }
    }

}

void FPPConnectDialog::SaveSettings()
{
    wxString selected = "";
    wxTreeListItem item = CheckListBox_Sequences->GetFirstItem();
    while (item.IsOk()) {
        bool isChecked = CheckListBox_Sequences->GetCheckedState(item) == wxCHK_CHECKED;
        if (isChecked) {
            if (selected != "") {
                selected += ",";
            }
            selected += CheckListBox_Sequences->GetItemText(item);
        }
        item = CheckListBox_Sequences->GetNextItem(item);
    }

    wxConfigBase* config = wxConfigBase::Get();
    config->Write("FPPConnectSelectedSequences", selected);
    config->Write("FPPConnectFilterSelection", ChoiceFilter->GetSelection());
    config->Write("FPPConnectFolderSelection", ChoiceFolder->GetString(ChoiceFolder->GetSelection()));

    int row = 0;
    for (const auto& inst : instances) {
        std::string rowStr = std::to_string(row);
        wxString keyPostfx = inst->ipAddress;
        keyPostfx.Replace(":", "_");
        keyPostfx.Replace("/", "_");
        keyPostfx.Replace("\\", "_");
        keyPostfx.Replace(".", "_");
        config->Write("FPPConnectUpload_" + keyPostfx, GetCheckValue(CHECK_COL + rowStr));
        config->Write("FPPConnectUploadMedia_" + keyPostfx, GetCheckValue(MEDIA_COL + rowStr));
        config->Write("FPPConnectUploadFSEQType_" + keyPostfx, GetChoiceValueIndex(FSEQ_COL + rowStr));
        config->Write("FPPConnectUploadModels_" + keyPostfx, GetCheckValue(MODELS_COL + rowStr));
        config->Write("FPPConnectUploadUDPOut_" + keyPostfx, GetChoiceValueIndex(UDP_COL + rowStr));
        config->Write("FPPConnectUploadPixelOut_" + keyPostfx, GetCheckValue(UPLOAD_CONTROLLER_COL + rowStr));
        row++;
    }

    config->Flush();
}

void FPPConnectDialog::ApplySavedHostSettings()
{
    /*
     static const std::string CHECK_COL = "ID_UPLOAD_";
     static const std::string FSEQ_COL = "ID_FSEQTYPE_";
     static const std::string MEDIA_COL = "ID_MEDIA_";
     static const std::string MODELS_COL = "ID_MODELS_";
     static const std::string UDP_COL = "ID_UDPOUT_";
     static const std::string PLAYLIST_COL = "ID_PLAYLIST_";
     static const std::string UPLOAD_CONTROLLER_COL = "ID_CONTROLLER_";
     */


    wxConfigBase* config = wxConfigBase::Get();
    if (config != nullptr) {
        int row = 0;
        for (const auto& inst : instances) {
            std::string rowStr = std::to_string(row);
            wxString keyPostfx = inst->ipAddress;
            keyPostfx.Replace(":", "_");
            keyPostfx.Replace("/", "_");
            keyPostfx.Replace("\\", "_");
            keyPostfx.Replace(".", "_");

            bool bval;
            int lval;
            if (config->Read("FPPConnectUpload_" + keyPostfx, &bval)) {
                SetCheckValue(CHECK_COL + rowStr, bval);
            }
            if (config->Read("FPPConnectUploadFSEQType_" + keyPostfx, &lval)) {
                SetChoiceValueIndex(FSEQ_COL + rowStr, lval);
            }
            if (config->Read("FPPConnectUploadMedia_" + keyPostfx, &bval)) {
                SetCheckValue(MEDIA_COL + rowStr, bval);
            }
            if (config->Read("FPPConnectUploadModels_" + keyPostfx, &bval)) {
                SetCheckValue(MODELS_COL + rowStr, bval);
            }
            if (config->Read("FPPConnectUploadUDPOut_" + keyPostfx, &lval)) {
                SetChoiceValueIndex(UDP_COL + rowStr, lval);
            }
            if (config->Read("FPPConnectUploadPixelOut_" + keyPostfx, &bval)) {
                SetCheckValue(UPLOAD_CONTROLLER_COL + rowStr, bval);
            }
            row++;
        }
    }
}


void FPPConnectDialog::OnClose(wxCloseEvent& event)
{
    EndDialog(0);
}

void FPPConnectDialog::SequenceListPopup(wxTreeListEvent& event)
{
    wxMenu mnu;
    mnu.Append(ID_MNU_SELECTALL, "Select All");
    mnu.Append(ID_MNU_SELECTNONE, "Clear Selections");
    mnu.Append(ID_MNU_SELECTHIGH, "Select Highlighted");
    mnu.Append(ID_MNU_DESELECTHIGH, "Deselect Highlighted");
    mnu.Connect(wxEVT_MENU, (wxObjectEventFunction)&FPPConnectDialog::OnPopup, nullptr, this);
    PopupMenu(&mnu);
}

void FPPConnectDialog::OnAddFPPButtonClick(wxCommandEvent& event)
{
    wxTextEntryDialog dlg(this, "Find FPP Instance", "Enter IP address or hostname for FPP Instance");
    if (dlg.ShowModal() == wxID_OK && IsIPValidOrHostname(dlg.GetValue().ToStdString())) {
        std::string ipAd = dlg.GetValue().ToStdString();
        int curSize = instances.size();

        wxProgressDialog prgs("Gathering configuration for " + ipAd,
                              "Gathering configuration for " + ipAd, 100, this);
        prgs.Pulse("Gathering configuration for " + ipAd);
        prgs.Show();

        FPP *inst = new FPP(ipAd);
        if (inst->AuthenticateAndUpdateVersions()) {
            bool found = false;
            for (const auto &fpp : instances) {
                if (inst->ipAddress == fpp->ipAddress
                    || inst->ipAddress == fpp->hostName
                    || fpp->hostName == inst->ipAddress) {
                    found = true;
                }
            }
            if (found) {
                delete inst;
            } else {
                instances.push_back(inst);
            }
            std::list<std::string> add;
            add.push_back(ipAd);
            FPP::Probe(add, instances);
            int cur = 0;
            for (const auto &fpp : instances) {
                if (cur >= curSize) {
                    prgs.Pulse("Gathering configuration for " + fpp->hostName + " - " + fpp->ipAddress);
                    fpp->AuthenticateAndUpdateVersions();
                    fpp->probePixelControllerType();
                }
                cur++;
            }

            //it worked, we found some new instances, record this
            wxConfigBase* config = wxConfigBase::Get();
            wxString ip;
            config->Read("FPPConnectForcedIPs", &ip);
            if (!ip.Contains(dlg.GetValue())) {
                if (ip != "") {
                    ip += "|";
                }
                ip += dlg.GetValue();
                config->Write("FPPConnectForcedIPs", ip);
                config->Flush();
            }
            PopulateFPPInstanceList();
        } else {
            delete inst;
        }
        prgs.Hide();
    }
}

void FPPConnectDialog::OnChoiceFolderSelect(wxCommandEvent& event)
{
    LoadSequences();
}

void FPPConnectDialog::OnChoiceFilterSelect(wxCommandEvent& event)
{
    LoadSequences();
}

void FPPConnectDialog::GetFolderList(const wxString& folder)
{
    ChoiceFolder->Append("--Show Folder--");
    wxArrayString subfolders;
    wxDir dir(folder);
    if (!dir.IsOpened())
    {
        return;
    }
    wxString strFile;

    if (dir.GetFirst(&strFile, "*", wxDIR_HIDDEN | wxDIR_DIRS))
        subfolders.Add(strFile);

    while (dir.GetNext(&strFile))
    {
        subfolders.Add(strFile);
    }
    subfolders.Sort();
    for (const auto& subfolder: subfolders)
    {
        if(subfolder.StartsWith("Backup"))
            continue;
        if (subfolder.StartsWith("."))
            continue;
        ChoiceFolder->Append(subfolder);
    }
}


void FPPConnectDialog::DisplayDateModified(std::string const& filePath, wxTreeListItem &item) const
{ 
    if (wxFile::Exists(filePath)) {
        wxDateTime last_modified_time(wxFileModificationTime(filePath));
        CheckListBox_Sequences->SetItemText(item, 1, last_modified_time.Format(wxT("%Y-%m-%d %H:%M:%S")));
    }
}
