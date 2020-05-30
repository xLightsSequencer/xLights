#pragma once

/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/smeighan/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/smeighan/xLights/blob/master/License.txt
 **************************************************************/

class xLightsFrame;
class SP_XmlStartTagEvent;

//(*Headers(ConvertDialog)
#include <wx/button.h>
#include <wx/checkbox.h>
#include <wx/choice.h>
#include <wx/dialog.h>
#include <wx/filedlg.h>
#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/textctrl.h>
//*)

#include <set>

#include "SequenceData.h"

class OutputManager;

typedef SequenceData SeqDataType;

class ConvertDialog: public wxDialog
{
    xLightsFrame* _parent;
    wxString msgBuffer;
    wxArrayString FileNames;
    SeqDataType& SeqData;
    OutputManager* _outputManager;
    std::set<int> LorTimingList; // contains a list of period numbers, set by ReadLorFile()
    std::string& mediaFilename;
    wxArrayString& ChannelNames;
    wxArrayInt& ChannelColors;
    wxArrayString& ChNames;

    bool mapEmptyChannels();
    bool showChannelMapping();
    bool isSetOffAtEnd();
    bool WriteLedBlinkyFile(const wxString& filename);
    bool WriteVixenFile(const wxString& filename);
    void WriteVirFile(const wxString& filename) const;
    void WriteHLSFile(const wxString& filename) const;
    void WriteXLightsFile(const wxString& filename) const;
    void WriteLSPFile(const wxString& filename) const;
    void WriteLorFile(const wxString& filename);
    void WriteLcbFile(const wxString& filename) const;
    void WriteConductorFile(const wxString& filename) const;
    bool LoadVixenProfile(const wxString& ProfileName, wxArrayInt& VixChannels, wxArrayString& VixChannelNames);
    void ReadConductorFile(const wxString& FileName);
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
        ConvertDialog(wxWindow* parent, SeqDataType& SeqData_, OutputManager* outputManager_, std::string& mediaFilename_, wxArrayString& ChannelNames_, wxArrayInt& ChannelColors_, wxArrayString& ChNames_, wxWindowID id=wxID_ANY,const wxPoint& pos=wxDefaultPosition,const wxSize& size=wxDefaultSize);
		virtual ~ConvertDialog();

		//(*Declarations(ConvertDialog)
		wxButton* ButtonChooseFile;
		wxButton* ButtonClose;
		wxButton* ButtonStartConversion;
		wxCheckBox* CheckBoxMapEmptyChannels;
		wxCheckBox* CheckBoxOffAtEnd;
		wxCheckBox* CheckBoxShowChannelMapping;
		wxCheckBox* MapLORChannelsWithNoNetwork;
		wxChoice* ChoiceOutputFormat;
		wxChoice* LORImportTimeResolution;
		wxFileDialog* FileDialogConvert;
		wxStaticText* StaticText10;
		wxStaticText* StaticText11;
		wxStaticText* StaticText2;
		wxStaticText* StaticText3;
		wxStaticText* StaticText4;
		wxStaticText* StaticText5;
		wxStaticText* StaticText6;
		wxStaticText* StaticText7;
		wxStaticText* StaticText8;
		wxStaticText* StaticText9;
		wxStaticText* StaticTextStatus;
		wxTextCtrl* TextCtrlConversionStatus;
		wxTextCtrl* TextCtrlFilename;
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
