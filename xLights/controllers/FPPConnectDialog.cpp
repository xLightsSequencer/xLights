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

#include <log4cpp/Category.hh>
#include "../xSchedule/wxJSON/jsonreader.h"

#include "../include/spxml-0.5/spxmlparser.hpp"
#include "../include/spxml-0.5/spxmlevent.hpp"
#include "../FSEQFile.h"
#include "../Parallel.h"

//(*IdInit(FPPConnectDialog)
const long FPPConnectDialog::ID_LISTVIEW_Sequences = wxNewId();
const long FPPConnectDialog::ID_BUTTON1 = wxNewId();
const long FPPConnectDialog::ID_BUTTON_Upload = wxNewId();
//*)

const long FPPConnectDialog::ID_MNU_SELECTALL = wxNewId();
const long FPPConnectDialog::ID_MNU_SELECTNONE = wxNewId();
const long FPPConnectDialog::ID_FPP_INSTANCE_LIST = wxNewId();

BEGIN_EVENT_TABLE(FPPConnectDialog,wxDialog)
	//(*EventTable(FPPConnectDialog)
	//*)
END_EVENT_TABLE()


static const int CHECK_COL = 0;
static const int SEQ_COL = 4;
static const int MEDIA_COL = 5;
static const int MODELS_COL = 6;
static const int UDP_COL = 7;
static const int PLAYLIST_COL = 8;
static const int UPLOAD_PIXELS_COL = 9;
//static const int PIXELS_CONTROLLER_COL = 10;


FPPConnectDialog::FPPConnectDialog(wxWindow* parent, OutputManager* outputManager, wxWindowID id,const wxPoint& pos,const wxSize& size)
{
    _outputManager = outputManager;

	//(*Initialize(FPPConnectDialog)
	wxButton* cancelButton;
	wxFlexGridSizer* FlexGridSizer1;
	wxFlexGridSizer* FlexGridSizer4;

	Create(parent, wxID_ANY, _("FPP Upload"), wxDefaultPosition, wxDefaultSize, wxCAPTION|wxDEFAULT_DIALOG_STYLE|wxRESIZE_BORDER|wxCLOSE_BOX|wxMAXIMIZE_BOX, _T("wxID_ANY"));
	FlexGridSizer1 = new wxFlexGridSizer(0, 1, 0, 0);
	FlexGridSizer1->AddGrowableCol(0);
	FlexGridSizer1->AddGrowableRow(1);
	CheckListBox_Sequences = new wxListView(this, ID_LISTVIEW_Sequences, wxDefaultPosition, wxDefaultSize, wxLC_REPORT, wxDefaultValidator, _T("ID_LISTVIEW_Sequences"));
	CheckListBox_Sequences->SetMinSize(wxDLG_UNIT(this,wxSize(-1,200)));
	FlexGridSizer1->Add(CheckListBox_Sequences, 1, wxALL|wxEXPAND, 5);
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
	FlexGridSizer1->Add(FlexGridSizer4, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	SetSizer(FlexGridSizer1);
	FlexGridSizer1->Fit(this);
	FlexGridSizer1->SetSizeHints(this);

	Connect(ID_LISTVIEW_Sequences,wxEVT_COMMAND_LIST_ITEM_RIGHT_CLICK,(wxObjectEventFunction)&FPPConnectDialog::SequenceListPopup);
	Connect(ID_BUTTON1,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&FPPConnectDialog::OnAddFPPButtonClick);
	Connect(ID_BUTTON_Upload,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&FPPConnectDialog::OnButton_UploadClick);
	Connect(wxID_ANY,wxEVT_CLOSE_WINDOW,(wxObjectEventFunction)&FPPConnectDialog::OnClose);
	//*)

    CheckListBox_Sequences->EnableCheckBoxes();

    wxProgressDialog prgs("Discovering FPP Instances",
                          "Discovering FPP Instances", 100, parent);
    prgs.Pulse("Discovering FPP Instances");
    prgs.Show();
    
    FPP::Discover(instances);

    std::list<std::string> startAddresses;
    for (auto &inst : instances) {
        prgs.Pulse("Gathering information from " + inst.ipAddress);
        if (inst.AuthenticateAndUpdateVersions()) {
            startAddresses.push_back(inst.ipAddress);
        }
    }

    wxConfigBase* config = wxConfigBase::Get();
    wxString force;
    if (config->Read("FPPConnectForcedIPs", &force)) {
        wxArrayString ips = wxSplit(force, '|');
        wxString newForce;
        for (auto &a : ips) {
            bool foundAlready = false;
            for (auto &inst : instances) {
                if (a == inst.ipAddress || a == inst.hostName) {
                    foundAlready = true;
                }
            }
            if (!foundAlready) {
                FPP fpp(a);
                prgs.Pulse("Gathering information from " + a);
                if (fpp.AuthenticateAndUpdateVersions()) {
                    //able to contact and authenticate
                    instances.push_back(fpp);
                    startAddresses.push_back(a);
                    if (newForce == "") {
                        newForce = a;
                    } else {
                        newForce += "," + a;
                    }
                }
            }
        }
        if (newForce != force) {
            config->Write("FPPConnectForcedIPs", newForce);
        }
    }
    prgs.Pulse("Probing for multisync instances");
    int curSize = instances.size();
    FPP::Probe(startAddresses, instances);
    std::list<std::string> playlists;
    int cur = 0;
    for (auto &fpp : instances) {
        prgs.Pulse("Gathering configuration for " + fpp.hostName + " - " + fpp.ipAddress);
        if (cur >= curSize) {
            fpp.AuthenticateAndUpdateVersions();
        }
        fpp.LoadPlaylists(playlists);
        fpp.probePixelControllerType();
        cur++;
    }
    prgs.Pulse("Checking for mounted media drives");
    CreateDriveList();
    prgs.Update(100);
    prgs.Hide();

    FPPInstanceList = new wxDataViewListCtrl(this, ID_FPP_INSTANCE_LIST, wxDefaultPosition, wxDefaultSize, 0);
    FPPInstanceList->SetMinSize(wxDLG_UNIT(this,wxSize(500,100)));
    FPPInstanceList->AppendToggleColumn("Upload");
    FPPInstanceList->AppendTextColumn("Location");
    FPPInstanceList->AppendTextColumn("Description");
    FPPInstanceList->AppendTextColumn("Version");


    wxArrayString ftypes;
    ftypes.push_back("V1");
    ftypes.push_back("V2");
    ftypes.push_back("V2 Sparse");
    wxDataViewColumn *seqCol = new wxDataViewColumn("FSEQ Type", new wxDataViewChoiceByIndexRenderer(ftypes), SEQ_COL);
    FPPInstanceList->AppendColumn(seqCol);
    FPPInstanceList->AppendToggleColumn("Media");
    FPPInstanceList->AppendToggleColumn("Models");
    FPPInstanceList->AppendToggleColumn("UDP Out");

    wxArrayString pl;
    pl.push_back("");
    for (const auto &a : playlists) {
        if (pl.Index(a) == -1) {
            pl.push_back(a);
        }
    }
    wxDataViewChoiceRenderer *rend = new wxDataViewChoiceRenderer(pl, wxDATAVIEW_CELL_EDITABLE);
    seqCol = new wxDataViewColumn("Playlist", rend, PLAYLIST_COL);
    FPPInstanceList->AppendColumn(seqCol);
    FPPInstanceList->AppendToggleColumn(" ");
    FPPInstanceList->AppendTextColumn("Pixel Hat/Cap");


    //these aren't working for checkboxes :(
    //Connect(ID_FPP_INSTANCE_LIST,wxEVT_DATAVIEW_ITEM_START_EDITING,(wxObjectEventFunction)&FPPConnectDialog::OnFPPInstanceItemStartEditing);
    Connect(ID_FPP_INSTANCE_LIST,wxEVT_DATAVIEW_ITEM_VALUE_CHANGED,(wxObjectEventFunction)&FPPConnectDialog::OnFPPInstanceItemValueChanged);

    FlexGridSizer1->Insert(0, FPPInstanceList, 1, wxALL|wxEXPAND, 5);
    SetSizer(FlexGridSizer1);
    FlexGridSizer1->Fit(this);
    FlexGridSizer1->SetSizeHints(this);

    PopulateFPPInstanceList();
    LoadSequences();
}

void FPPConnectDialog::PopulateFPPInstanceList() {
    FPPInstanceList->Freeze();
    FPPInstanceList->DeleteAllItems();
    int row = 0;
    for (auto &inst : instances) {
        wxVector< wxVariant > values;
        values.push_back(wxVariant(true));   //upload check box
        values.push_back(wxVariant(inst.hostName + " - " + inst.ipAddress));
        values.push_back(wxVariant(inst.description));
        values.push_back(wxVariant(inst.fullVersion));
        values.push_back(wxVariant(inst.IsVersionAtLeast(2, 6) ? (inst.mode != "master" ? 2 : 1) : 0)); //v2
        values.push_back(wxVariant(inst.mode != "remote"));  //media
        values.push_back(wxVariant(false));  //models
        values.push_back(wxVariant(false));  //UDP
        values.push_back(wxVariant("")); //playlist
        values.push_back(wxVariant(false));  //upload pixels
        values.push_back(wxVariant(inst.PixelContollerDescription()));  //upload pixels
        FPPInstanceList->AppendItem(values, row++);
    }
    for (int x = 0; x< FPPInstanceList->GetColumnCount(); x++) {
        FPPInstanceList->GetColumn(x)->SetWidth(wxCOL_WIDTH_AUTOSIZE);
    }
    ApplySavedHostSettings();
    FPPInstanceList->Thaw();
}

void FPPConnectDialog::OnFPPInstanceItemValueChanged(wxDataViewEvent& event) {
    int col = event.GetColumn();
    int row = FPPInstanceList->ItemToRow(event.GetItem());
    std::list<FPP>::iterator it = instances.begin();
    std::advance(it, row);
    if (col == SEQ_COL) {
        wxVariant val;
        FPPInstanceList->GetValue(val, row, SEQ_COL);
        if (val.GetLong() && !it->IsVersionAtLeast(2, 6)) {
            //not supported
            FPPInstanceList->SetValue(wxVariant(0), row, SEQ_COL);
        }
    } else if (col == PLAYLIST_COL) {
        if (!it->IsVersionAtLeast(2, 6)) {
            FPPInstanceList->SetTextValue("", row, col);
        }
    } else if (col == UPLOAD_PIXELS_COL) {
        if (it->pixelControllerType == "" && FPPInstanceList->GetToggleValue(row, UPLOAD_PIXELS_COL)) {
            FPPInstanceList->SetToggleValue(false, row, UPLOAD_PIXELS_COL);
        }
    }
}

void FPPConnectDialog::OnPopup(wxCommandEvent &event)
{
    int id = event.GetId();
    if (id == ID_MNU_SELECTALL) {
        for (size_t i = 0; i < CheckListBox_Sequences->GetItemCount(); i++) {
            if (!CheckListBox_Sequences->IsItemChecked(i)) {
                CheckListBox_Sequences->CheckItem(i, true);
            }
        }
    } else if (id == ID_MNU_SELECTNONE) {
        for (size_t i = 0; i < CheckListBox_Sequences->GetItemCount(); i++) {
            if (CheckListBox_Sequences->IsItemChecked(i)) {
                CheckListBox_Sequences->CheckItem(i, false);
            }
        }
    }
}

FPPConnectDialog::~FPPConnectDialog()
{
	//(*Destroy(FPPConnectDialog)
	//*)
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
    bool fcont = directory.GetFirst(&file, "*.xml");
    static const int BUFFER_SIZE = 1024*12;
    std::vector<char> buf(BUFFER_SIZE); //12K buffer
    while (fcont) {
        if (file != "xlights_rgbeffects.xml" && file != OutputManager::GetNetworksFileName() && file != "xlights_keybindings.xml") {
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
                    size_t read = doc.Read(&buf[0], BUFFER_SIZE);
                    if (read == 0) {
                        done = true;
                    } else {
                        parser->append(&buf[0], read);
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
                    if (wxFile::Exists(frame->mediaDirectory + wxFileName::GetPathSeparator() + fn.GetFullName())) {
                        mediaName = frame->mediaDirectory + wxFileName::GetPathSeparator() + fn.GetFullName();
                    } else {
                        mediaName = "";
                    }
                }
            }
            
            if (isSequence) {
                long index = CheckListBox_Sequences->GetItemCount();
                CheckListBox_Sequences->InsertItem(index, fseqName);
                if (mediaName != "") {
                    CheckListBox_Sequences->SetItem(index, 1, mediaName);
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
            LoadSequencesFromFolder(dir + wxFileName::GetPathSeparator() + file);
        }
        fcont = directory.GetNext(&file);
    }
}

void FPPConnectDialog::LoadSequences()
{
    CheckListBox_Sequences->ClearAll();
    CheckListBox_Sequences->AppendColumn("Sequence");
    CheckListBox_Sequences->AppendColumn("Media");
    LoadSequencesFromFolder(xLightsFrame::CurrentDir);
    


    xLightsFrame* frame = static_cast<xLightsFrame*>(GetParent());
    wxDir directory;
    directory.Open(frame->GetFseqDirectory());

    wxString file;
    bool fcont = directory.GetFirst(&file, "*.?seq");
    while (fcont) {
        int i = CheckListBox_Sequences->FindItem(0, frame->GetFseqDirectory() + wxFileName::GetPathSeparator() + file, true);
        if (i == -1) {
            wxListItem info;
            info.SetText(frame->GetFseqDirectory() + wxFileName::GetPathSeparator() + file);
            info.SetId(99999);
            CheckListBox_Sequences->InsertItem(info);
        }
        fcont = directory.GetNext(&file);
    }

    if (xLightsFrame::CurrentSeqXmlFile != nullptr) {
        wxString curSeq = xLightsFrame::CurrentSeqXmlFile->GetLongPath();
        if (!curSeq.StartsWith(xLightsFrame::CurrentDir)) {
            LoadSequencesFromFolder(xLightsFrame::CurrentSeqXmlFile->GetLongPath());
        }
        int i = CheckListBox_Sequences->FindItem(0, xLightsFrame::CurrentSeqXmlFile->GetLongPath(), true);
        if (i != -1) {
            CheckListBox_Sequences->CheckItem(i, true);
        }
    }
    
    wxConfigBase* config = wxConfigBase::Get();
    if (config != nullptr) {
        const wxString itcsv = config->Read("FPPConnectSelectedSequences", wxEmptyString);
        
        if (!itcsv.IsEmpty()) {
            wxArrayString savedUploadItems = wxSplit(itcsv, ',');
            
            for (int x = 0; x < CheckListBox_Sequences->GetItemCount(); x++) {
                if (savedUploadItems.Index(CheckListBox_Sequences->GetItemText(x)) != wxNOT_FOUND) {
                    CheckListBox_Sequences->CheckItem(x, true);
                }
            }
        }
    }

    CheckListBox_Sequences->SetColumnWidth(0, wxLIST_AUTOSIZE);
    CheckListBox_Sequences->SetColumnWidth(1, wxLIST_AUTOSIZE);
    Fit();
}

void FPPConnectDialog::OnButton_UploadClick(wxCommandEvent& event)
{
    bool cancelled = false;
    std::vector<bool> doUpload(FPPInstanceList->GetItemCount());
    for (int x = 0; x < FPPInstanceList->GetItemCount(); x++) {
        doUpload[x] = FPPInstanceList->GetToggleValue(x, CHECK_COL);
    }

    for (int fs = 0; fs < CheckListBox_Sequences->GetItemCount(); fs++) {
        if (CheckListBox_Sequences->IsItemChecked(fs)) {
            std::string fseq = CheckListBox_Sequences->GetItemText(fs);
            std::string media = CheckListBox_Sequences->GetItemText(fs, 1);

            FSEQFile *seq = FSEQFile::openFSEQFile(fseq);
            if (seq) {
                int row = 0;
                for (auto &inst : instances) {
                    if (!cancelled && doUpload[row]) {
                        inst.parent = this;
                        std::string m2 = media;
                        if (!FPPInstanceList->GetToggleValue(row, MEDIA_COL)) {
                            m2 = "";
                        }

                        wxVariant var;
                        FPPInstanceList->GetValue(var, row, SEQ_COL);
                        cancelled |= inst.PrepareUploadSequence(*seq,
                                                                fseq, m2,
                                                                var.GetLong());
                    }
                    row++;
                }
                if (!cancelled) {
                    wxProgressDialog prgs("Generating FSEQ Files", "Generating " + fseq, 1000, this, wxPD_CAN_ABORT | wxPD_APP_MODAL);
                    prgs.Show();
                    int lastDone = 0;
                    static const int FRAMES_TO_BUFFER = 50;
                    std::vector<std::vector<uint8_t>> frames(FRAMES_TO_BUFFER);
                    for (int x = 0; x < frames.size(); x++) {
                        frames[x].resize(seq->getMaxChannel() + 1);
                    }
                    
                    for (int frame = 0; frame < seq->getNumFrames() && !cancelled; frame++) {
                        int donePct = frame * 1000 / seq->getNumFrames();
                        if (donePct != lastDone) {
                            lastDone = donePct;
                            cancelled |= !prgs.Update(donePct, "Generating " + fseq, &cancelled);
                            wxYield();
                        }

                        int lastBuffered = 0;
                        int startFrame = frame;
                        //Read a bunch of frames so each parallel thread has more info to work with before returning out here
                        while (lastBuffered < FRAMES_TO_BUFFER && frame < seq->getNumFrames()) {
                            FSEQFile::FrameData *f = seq->getFrame(frame);
                            f->readFrame(&frames[lastBuffered][0]);
                            delete f;
                            lastBuffered++;
                            frame++;
                        }
                        frame--;
                        std::function<void(FPP &, int)> func = [startFrame, lastBuffered, &frames, &doUpload](FPP& inst, int row) {
                            if (doUpload[row]) {
                                for (int x = 0; x < lastBuffered; x++) {
                                    inst.AddFrameToUpload(startFrame + x, &frames[x][0]);
                                }
                            }
                        };
                        parallel_for(instances, func);
                    }
                    prgs.Hide();
                }
                row = 0;
                for (auto &inst : instances) {
                    if (!cancelled && doUpload[row]) {
                        cancelled |= inst.FinalizeUploadSequence();
                    }
                    row++;
                }
            }
            delete seq;
        }
    }
    int row = 0;
    xLightsFrame* frame = static_cast<xLightsFrame*>(GetParent());
    wxJSONValue outputs = FPP::CreateOutputUniverseFile(_outputManager);
    std::string memoryMaps = FPP::CreateModelMemoryMap(&frame->AllModels);
    for (auto &inst : instances) {
        if (!cancelled && doUpload[row]) {
            wxVariant val;
            FPPInstanceList->GetValue(val, row, PLAYLIST_COL);
            if (val.GetString() != "") {
                cancelled |= inst.UploadPlaylist(val.GetString().ToStdString());
            }
            if (FPPInstanceList->GetToggleValue(row, MODELS_COL)) {
                cancelled |= inst.UploadModels(memoryMaps);
                inst.SetRestartFlag();
            }
            if (FPPInstanceList->GetToggleValue(row, UDP_COL)) {
                cancelled |= inst.UploadUDPOut(outputs);
                inst.SetRestartFlag();
            }
            if (FPPInstanceList->GetToggleValue(row, UPLOAD_PIXELS_COL)) {
                cancelled |= inst.UploadPixelOutputs(&frame->AllModels, _outputManager);
                inst.SetRestartFlag();
            }
        }
        row++;
    }

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
    for (auto &a : ud) {
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

    for (auto a : drives) {
        FPP inst;
        inst.hostName = "FPP";
        inst.ipAddress = a;
        inst.minorVersion = 0;
        inst.majorVersion = 2;
        inst.fullVersion = "Unknown";
        inst.mode = "standalone";
        if (wxFile::Exists(a + "/fpp-info.json")) {
            //read version and hostname
            wxJSONValue system;
            wxJSONReader reader;
            wxString str;
            wxFile(a + "/fpp-info.json").ReadAll(&str);
            reader.Parse(str, &system);

            if (!system["hostname"].IsNull()) {
                inst.hostName = system["hostname"].AsString();
            }
            if (!system["type"].IsNull()) {
                inst.platform = system["type"].AsString();
            }
            if (!system["model"].IsNull()) {
                inst.model = system["model"].AsString();
            }
            if (!system["version"].IsNull()) {
                inst.fullVersion = system["version"].AsString();
            }
            if (system["minorVersion"].IsInt()) {
                inst.minorVersion = system["minorVersion"].AsInt();
            }
            if (system["majorVersion"].IsInt()) {
                inst.majorVersion = system["majorVersion"].AsInt();
            }
            if (!system["channelRanges"].IsNull()) {
                inst.ranges = system["channelRanges"].AsString();
            }
            if (!system["HostDescription"].IsNull()) {
                inst.description = system["HostDescription"].AsString();
            }
            if (!system["fppModeString"].IsNull()) {
                inst.mode = system["fppModeString"].AsString();
            }
        }
        instances.push_back(inst);
    }
}


void FPPConnectDialog::SaveSettings()
{
    
    wxString selected = "";
    for (int fs = 0; fs < CheckListBox_Sequences->GetItemCount(); fs++) {
        if (CheckListBox_Sequences->IsItemChecked(fs)) {
            if (selected != "") {
                selected += ",";
            }
            selected += CheckListBox_Sequences->GetItemText(fs);
        }
    }
    wxConfigBase* config = wxConfigBase::Get();
    config->Write("FPPConnectSelectedSequences", selected);
    int row = 0;
    for (auto &inst : instances) {
        wxString keyPostfx = inst.ipAddress;
        keyPostfx.Replace(":", "_");
        keyPostfx.Replace("/", "_");
        keyPostfx.Replace("\\", "_");
        keyPostfx.Replace(".", "_");
        config->Write("FPPConnectUpload_" + keyPostfx, FPPInstanceList->GetToggleValue(row, CHECK_COL));
        config->Write("FPPConnectUploadMedia_" + keyPostfx, FPPInstanceList->GetToggleValue(row, MEDIA_COL));
        wxVariant var;
        FPPInstanceList->GetValue(var, row, SEQ_COL);
        config->Write("FPPConnectUploadFSEQType_" + keyPostfx, var.GetLong());
        config->Write("FPPConnectUploadModels_" + keyPostfx, FPPInstanceList->GetToggleValue(row, MODELS_COL));
        config->Write("FPPConnectUploadUDPOut_" + keyPostfx, FPPInstanceList->GetToggleValue(row, UDP_COL));
        config->Write("FPPConnectUploadPixelOut_" + keyPostfx, FPPInstanceList->GetToggleValue(row, UPLOAD_PIXELS_COL));
        row++;
    }
    
    config->Flush();
}

void FPPConnectDialog::ApplySavedHostSettings()
{
    wxConfigBase* config = wxConfigBase::Get();
    if (config != nullptr) {
        int row = 0;
        for (auto &inst : instances) {
            wxString keyPostfx = inst.ipAddress;
            keyPostfx.Replace(":", "_");
            keyPostfx.Replace("/", "_");
            keyPostfx.Replace("\\", "_");
            keyPostfx.Replace(".", "_");
            
            bool bval;
            int lval;
            if (config->Read("FPPConnectUpload_" + keyPostfx, &bval)) {
                if (FPPInstanceList->GetToggleValue(row, CHECK_COL) != bval) {
                    FPPInstanceList->SetToggleValue(bval, row, CHECK_COL);
                }
            }
            if (config->Read("FPPConnectUploadFSEQType_" + keyPostfx, &lval)) {
                FPPInstanceList->SetValue(wxVariant(lval), row, SEQ_COL);
            }
            if (config->Read("FPPConnectUploadMedia_" + keyPostfx, &bval)) {
                if (FPPInstanceList->GetToggleValue(row, MEDIA_COL) != bval) {
                    FPPInstanceList->SetToggleValue(bval, row, MEDIA_COL);
                }
            }
            if (config->Read("FPPConnectUploadModels_" + keyPostfx, &bval)) {
                if (FPPInstanceList->GetToggleValue(row, MODELS_COL) != bval) {
                    FPPInstanceList->SetToggleValue(bval, row, MODELS_COL);
                }
            }
            if (config->Read("FPPConnectUploadUDPOut_" + keyPostfx, &bval)) {
                if (FPPInstanceList->GetToggleValue(row, UDP_COL) != bval) {
                    FPPInstanceList->SetToggleValue(bval, row, UDP_COL);
                }
            }
            if (config->Read("FPPConnectUploadPixelOut_" + keyPostfx, &bval)) {
                if (FPPInstanceList->GetToggleValue(row, UPLOAD_PIXELS_COL) != bval) {
                    FPPInstanceList->SetToggleValue(bval, row, UPLOAD_PIXELS_COL);
                }
            }
            row++;
        }
    }
}


void FPPConnectDialog::OnClose(wxCloseEvent& event)
{
    EndDialog(0);
}

void FPPConnectDialog::SequenceListPopup(wxListEvent& event)
{
    wxMenu mnu;
    mnu.Append(ID_MNU_SELECTALL, "Select All");
    mnu.Append(ID_MNU_SELECTNONE, "Clear Selections");
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

        FPP inst(ipAd);
        if (inst.AuthenticateAndUpdateVersions()) {
            std::list<std::string> add;
            add.push_back(ipAd);
            FPP::Probe(add, instances);
            int cur = 0;
            for (auto &fpp : instances) {
                prgs.Pulse("Gathering configuration for " + fpp.hostName + " - " + fpp.ipAddress);
                if (cur >= curSize) {
                    fpp.AuthenticateAndUpdateVersions();
                    fpp.probePixelControllerType();
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
        }
        prgs.Hide();
    }
}
