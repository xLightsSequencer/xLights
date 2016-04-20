#ifndef CONVERTDIALOG_H
#define CONVERTDIALOG_H

class xLightsFrame;
class SP_XmlStartTagEvent;

//(*Headers(ConvertDialog)
#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/textctrl.h>
#include <wx/checkbox.h>
#include <wx/filedlg.h>
#include <wx/choice.h>
#include <wx/button.h>
#include <wx/dialog.h>
//*)

#include <set>

#include "SequenceData.h"
#include "NetInfo.h"
#include "xLightsMain.h"

typedef SequenceData SeqDataType;

class ConvertDialog: public wxDialog
{
    xLightsFrame* _parent;
    wxString msgBuffer;
    wxArrayString FileNames;
    SeqDataType& SeqData;
    NetInfoClass& NetInfo;
    std::set<int> LorTimingList; // contains a list of period numbers, set by ReadLorFile()
    wxString& mediaFilename;
    wxArrayString& ChannelNames;
    wxArrayInt& ChannelColors;
    wxArrayString& ChNames;

    bool mapEmptyChannels();
    bool showChannelMapping();
    bool isSetOffAtEnd();
    bool WriteVixenFile(const wxString& filename);
    void WriteVirFile(const wxString& filename);
    void WriteHLSFile(const wxString& filename);
    //void ReadFalconFile(const wxString& FileName);
    //void WriteFalconPiFile(const wxString& filename);
    //void WriteFalconPiModelFile(const wxString& filename, long numChans, long numPeriods,
    //    SeqDataType *dataBuf, int startAddr, int modelSize);
    void WriteXLightsFile(const wxString& filename);
    void WriteLSPFile(const wxString& filename);
    void WriteLorFile(const wxString& filename);
    //void WriteLcbFile(const wxString& filename, long numChans, long numPeriods, SeqDataType *dataBuf);
    void WriteLcbFile(const wxString& filename);
    void WriteConductorFile(const wxString& filename);
    bool LoadVixenProfile(const wxString& ProfileName, wxArrayInt& VixChannels, wxArrayString& VixChannelNames);
    void ReadConductorFile(const wxString& FileName);
    //void ReadXlightsFile(const wxString& FileName, wxString *mediaFilename = NULL);
    void ReadGlediatorFile(const wxString& FileName);
    void ReadVixFile(const wxString& filename);
    void ReadHLSFile(const wxString& filename);
    void ReadLorFile(const wxString& filename, int LORImportInterval);
    void DoConversion(const wxString& Filename, const wxString& OutputFormat);
    wxString FromAscii(const char *val);
    wxString getAttributeValueSafe(SP_XmlStartTagEvent * stagEvent, const char * name);

	public:

        void PlayerError(const wxString& msg);
        void ConversionError(const wxString& msg);
        void AppendConvertStatus(const wxString &msg, bool flushBuffer = true);
        void SetStatusText(const wxString &msg);
        ConvertDialog(wxWindow* parent, SeqDataType& SeqData_, NetInfoClass& NetInfo, wxString& mediaFilename_, wxArrayString& ChannelNames_, wxArrayInt& ChannelColors_, wxArrayString& ChNames_, wxWindowID id=wxID_ANY,const wxPoint& pos=wxDefaultPosition,const wxSize& size=wxDefaultSize);
		virtual ~ConvertDialog();

		//(*Declarations(ConvertDialog)
		wxStaticText* StaticText10;
		wxStaticText* StaticText9;
		wxTextCtrl* TextCtrlFilename;
		wxChoice* LORImportTimeResolution;
		wxButton* ButtonClose;
		wxStaticText* StaticText2;
		wxFileDialog* FileDialogConvert;
		wxChoice* ChoiceOutputFormat;
		wxStaticText* StaticText6;
		wxStaticText* StaticTextStatus;
		wxStaticText* StaticText8;
		wxButton* ButtonStartConversion;
		wxStaticText* StaticText11;
		wxCheckBox* CheckBoxMapEmptyChannels;
		wxStaticText* StaticText3;
		wxStaticText* StaticText5;
		wxStaticText* StaticText7;
		wxCheckBox* CheckBoxShowChannelMapping;
		wxTextCtrl* TextCtrlConversionStatus;
		wxCheckBox* MapLORChannelsWithNoNetwork;
		wxButton* ButtonChooseFile;
		wxStaticText* StaticText4;
		wxCheckBox* CheckBoxOffAtEnd;
		//*)

	protected:

		//(*Identifiers(ConvertDialog)
		static const long ID_STATICTEXT2;
		static const long ID_STATICTEXT3;
		static const long ID_STATICTEXT6;
		static const long ID_BUTTON_CHOOSE_FILE;
		static const long ID_TEXTCTRL_FILENAME;
		static const long ID_STATICTEXT7;
		static const long ID_CHOICE_OUTPUT_FORMAT;
		static const long ID_STATICTEXT8;
		static const long ID_CHECKBOX_OFF_AT_END;
		static const long ID_STATICTEXT9;
		static const long ID_CHECKBOX_MAP_EMPTY_CHANNELS;
		static const long ID_STATICTEXT10;
		static const long ID_CHECKBOX_LOR_WITH_NO_CHANNELS;
		static const long ID_STATICTEXT11;
		static const long ID_CHECKBOX_ShowChannelMapping;
		static const long ID_STATICTEXT12;
		static const long ID_CHOICE1;
		static const long ID_BUTTON_START_CONVERSION;
		static const long ID_STATICTEXT4;
		static const long ID_TEXTCTRL_CONVERSION_STATUS;
		static const long ID_STATICTEXT_STATUS;
		static const long ID_BUTTON_CLOSE;
		//*)

	private:

		//(*Handlers(ConvertDialog)
		void OnButtonChooseFileClick(wxCommandEvent& event);
		void OnButtonStartConversionClick(wxCommandEvent& event);
		void OnButtonCloseClick(wxCommandEvent& event);
		//*)

		DECLARE_EVENT_TABLE()
};

#endif
