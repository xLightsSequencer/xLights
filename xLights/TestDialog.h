#ifndef TESTDIALOG_H
#define TESTDIALOG_H

// Need to do these manually due to issues with wxSmith and wxTreeListCtrl
#include <wx/treelist.h>
#include <wx/treectrl.h>
#include <wx/xml/xml.h>
#include <wx/filename.h>

//(*Headers(TestDialog)
#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/splitter.h>
#include <wx/aui/aui.h>
#include <wx/radiobut.h>
#include <wx/slider.h>
#include <wx/panel.h>
#include <wx/button.h>
#include <wx/dialog.h>
#include <wx/timer.h>
//*)

#include <string>
#include <list>
#include "models/ModelManager.h"
#include "xlights_out.h"
#include "SequenceData.h"
typedef SequenceData SeqDataType;

#ifdef __WXOSX__
#include "osxMacUtils.h"
#endif

class TreeController
{
public:
	typedef enum CONTROLLERTYPE
	{
		CT_E131, CT_NULL, CT_DMX, CT_CHANNEL, CT_CONTROLLERROOT, CT_MODELROOT, CT_MODELGROUPROOT, CT_MODEL, CT_NODE, CT_MODELGROUP
	} CONTROLLERTYPE;
	typedef enum PIXELFORMAT
	{
		SINGLE, RGB, RBG, BGR, BRG, GRB, GBR, RGBW
	} PIXELFORMAT;
private:
	std::string _name;
	std::string _modelName;
	std::string _description;
	bool _inactive;
	std::string _ipaddress;
	std::string _comport;
	int _startchannel;
	int _endchannel;
	int _startxlightschannel;
	int _endxlightschannel;
	std::string _universe;
	int _nullcount;
	int _nodeNumber;
	bool _doesNotExist;
	CONTROLLERTYPE _type;
	std::string GenerateName();
	TreeController(CONTROLLERTYPE type, std::string comport, int universe, int startxlightschannel, int channels, bool inactive, bool multiuniversedmx, std::string description);
	int _universes;
	bool _multiuniversedmx;
	wxTreeListItem _treeListItem;
	int _nodes;

public:
	TreeController(wxXmlNode* n, int xlightsstartchannel, int nullcount);
	TreeController(int channel, CONTROLLERTYPE type, int xLightsChannel);
	TreeController(CONTROLLERTYPE type, int start, int end);
	TreeController(CONTROLLERTYPE type, std::string name);
	TreeController(CONTROLLERTYPE type, int xLightsChannel, int node, int channelspernode);
	TreeController* GenerateDMXUniverse(int universeoffset);
	std::string Name() { return _name; };
	bool IsNULL() { return _type == CONTROLLERTYPE::CT_NULL; };
	bool IsChannel() { return _type == CONTROLLERTYPE::CT_CHANNEL; };
	bool Inactive() { return _inactive; };
	bool IsDoesNotExist() { return _doesNotExist; };
	void DoesNotExist() { _doesNotExist = true; };
	int Channels() { return _endchannel - _startchannel + 1; };
	int StartChannel() { return _startchannel; };
	int EndChannel() { return _endchannel; };
	int StartXLightsChannel() { return _startxlightschannel; };
	void SetTreeListItem(wxTreeListItem tli) { _treeListItem = tli; };
	wxTreeListItem GetTreeListItem() { return _treeListItem; };
	int EndXLightsChannel() { return _endxlightschannel; };
	void SetEndXLightsChannel(int modelendchannel) { _endxlightschannel = modelendchannel; GenerateName(); };
	void SetStartXLightsChannel(int modelstartchannel) { _startxlightschannel = modelstartchannel; GenerateName(); };
	void SetNodes(int nodes) { _nodes = nodes; GenerateName(); };
	CONTROLLERTYPE GetType() { return _type; };
	std::string ModelName() { return _modelName; };
	bool ContainsChannel(int ch);
	bool Clickable() { return (!IsNULL() && !IsDoesNotExist() && !Inactive()); };
};

class TestDialog: public wxDialog
{
	enum TestFunctions
	{
		OFF,
		CHASE,
		CHASE2,
		CHASE3,
		CHASE4,
		DIM,
		TWINKLE,
		SHIMMER
	};

	public:

		TestDialog(wxWindow* parent, wxXmlDocument* network, wxFileName networkFile, ModelManager* modelManager, wxWindowID id=wxID_ANY);
		virtual ~TestDialog();
		wxTreeListCtrl* TreeListCtrl_Channels;
		wxTreeListItem  _controllers;
		wxTreeListItem  _modelGroups;
		wxTreeListItem  _models;
		wxFileName _networkFile;
		ModelManager* _modelManager;
		bool _cascading;
		TestFunctions _testFunc;
		int _twinkleRatio;
		int _chaseGrouping;
		bool _checkChannelList;
		xOutput* _xout;
		wxCriticalSection _xoutCriticalSection;
		wxDateTime _starttime;
		SeqDataType _seqData;
		std::map<std::string, std::vector<TreeController*>> _modelLookup;
		std::map<int, std::vector<TreeController*>> _channelLookup;

		//(*Declarations(TestDialog)
		wxRadioButton* RadioButton_Standard_Chase;
		wxRadioButton* RadioButton_Standard_Twinkle5;
		wxSlider* Slider_Standard_Background;
		wxRadioButton* RadioButton_RGB_Chase13;
		wxSlider* Slider_RGB_BG_G;
		wxPanel* Panel5;
		wxSlider* Slider_RGB_BG_R;
		wxRadioButton* RadioButton_RGB_Twinkle5;
		wxRadioButton* RadioButton_RGB_Shimmer;
		wxRadioButton* RadioButton_RGB_Background;
		wxStaticText* StaticText2;
		wxPanel* Panel4;
		wxRadioButton* RadioButton_Standard_Chase14;
		wxStaticText* StaticText6;
		wxButton* Button_Save;
		wxRadioButton* RadioButton_RGB_Twinkle10;
		wxSlider* Slider_RGB_BG_B;
		wxRadioButton* RadioButton_RGB_Twinkle25;
		wxRadioButton* RadioButton_RGBCycle_ABCAll;
		wxPanel* Panel1;
		wxStaticText* StaticText1;
		wxSlider* Slider_RGB_H_G;
		wxStaticText* StaticText3;
		wxRadioButton* RadioButton_Standard_Shimmer;
		wxSlider* Slider_RGB_H_R;
		wxSlider* Slider_Standard_Highlight;
		wxRadioButton* RadioButton_Standard_Chase13;
		wxRadioButton* RadioButton_RGB_Alternate;
		wxRadioButton* RadioButton_RGBCycle_ABC;
		wxRadioButton* RadioButton_RGB_Twinkle50;
		wxStaticText* StaticText5;
		wxRadioButton* RadioButton_RGB_Chase14;
		wxRadioButton* RadioButton_RGBCycle_ABCAllNone;
		wxRadioButton* RadioButton_Standard_Chase15;
		wxStaticText* StatusBar1;
		wxSlider* Slider_RGB_H_B;
		wxAuiNotebook* AuiNotebook1;
		wxRadioButton* RadioButton_Standard_Twinkle10;
		wxRadioButton* RadioButton_RGB_Off;
		wxRadioButton* RadioButton_RGBCycle_Off;
		wxRadioButton* RadioButton_RGB_Chase15;
		wxRadioButton* RadioButton_Standard_Twinkle25;
		wxSlider* Slider_Speed;
		wxPanel* Panel2;
		wxRadioButton* RadioButton_Standard_Background;
		wxRadioButton* RadioButton_Standard_Twinkle50;
		wxRadioButton* RadioButton_RGB_Chase;
		wxSplitterWindow* SplitterWindow1;
		wxStaticText* StaticText4;
		wxRadioButton* RadioButton_RGBCycle_MixedColors;
		wxPanel* Panel_Standard;
		wxRadioButton* RadioButton_Standard_Alternate;
		wxRadioButton* RadioButton_Standard_Off;
		wxTimer Timer1;
		wxButton* Button_Load;
		//*)

	protected:

		wxXmlDocument* _network;
		static const long ID_TREELISTCTRL_Channels;

		//(*Identifiers(TestDialog)
		static const long ID_BUTTON_Load;
		static const long ID_BUTTON_Save;
		static const long ID_PANEL1;
		static const long ID_STATICTEXT2;
		static const long ID_RADIOBUTTON_Standard_Off;
		static const long ID_RADIOBUTTON_Standard_Chase;
		static const long ID_RADIOBUTTON_Standard_Chase13;
		static const long ID_RADIOBUTTON_Standard_Chase14;
		static const long ID_RADIOBUTTON_Standard_Chase15;
		static const long ID_RADIOBUTTON_Standard_Alternate;
		static const long ID_RADIOBUTTON_Standard_Twinke5;
		static const long ID_RADIOBUTTON_Standard_Twinkle10;
		static const long ID_RADIOBUTTON_Standard_Twinkle25;
		static const long ID_RADIOBUTTON_Standard_Twinkle50;
		static const long ID_RADIOBUTTON_Standard_Shimmer;
		static const long ID_RADIOBUTTON_Standard_Background;
		static const long ID_STATICTEXT3;
		static const long ID_SLIDER_Standard_Background;
		static const long ID_STATICTEXT4;
		static const long ID_SLIDER_Standard_Highlight;
		static const long ID_PANEL_Standard;
		static const long ID_STATICTEXT5;
		static const long ID_RADIOBUTTON_RGB_Off;
		static const long ID_RADIOBUTTON_RGB_Chase;
		static const long ID_RADIOBUTTON_RGB_Chase13;
		static const long ID_RADIOBUTTON_RGB_Chase14;
		static const long ID_RADIOBUTTON_RGB_Chase15;
		static const long ID_RADIOBUTTON_RGB_Alternate;
		static const long ID_RADIOBUTTON_RGB_Twinkle5;
		static const long ID_RADIOBUTTON_RGB_Twinkle10;
		static const long ID_RADIOBUTTON_RGB_Twinkle25;
		static const long ID_RADIOBUTTON_RGB_Twinkle50;
		static const long ID_RADIOBUTTON_RGB_Shimmer;
		static const long ID_RADIOBUTTON_RGB_Background;
		static const long ID_SLIDER1;
		static const long ID_SLIDER2;
		static const long ID_SLIDER3;
		static const long ID_SLIDER4;
		static const long ID_SLIDER5;
		static const long ID_SLIDER6;
		static const long ID_PANEL4;
		static const long ID_STATICTEXT6;
		static const long ID_RADIOBUTTON_RGBCycle_Off;
		static const long ID_RADIOBUTTON_RGBCycle_ABC;
		static const long ID_RADIOBUTTON_RGBCycle_ABCAll;
		static const long ID_RADIOBUTTON_RGBCycle_ABCAllNone;
		static const long ID_RADIOBUTTON_RGBCycle_MixedColors;
		static const long ID_PANEL5;
		static const long ID_AUINOTEBOOK1;
		static const long ID_STATICTEXT1;
		static const long ID_SLIDER_Speed;
		static const long ID_PANEL2;
		static const long ID_SPLITTERWINDOW1;
		static const long ID_STATICTEXT7;
		static const long ID_TIMER1;
		//*)

	private:

		//(*Handlers(TestDialog)
		void OnButton_LoadClick(wxCommandEvent& event);
		void OnButton_SaveClick(wxCommandEvent& event);
		void OnSlider_SpeedCmdSliderUpdated(wxScrollEvent& event);
		void OnRadioButton_RGB_OffSelect(wxCommandEvent& event);
		void OnRadioButton_RGB_ChaseSelect(wxCommandEvent& event);
		void OnRadioButton_RGB_Chase13Select(wxCommandEvent& event);
		void OnRadioButton_RGB_Chase14Select(wxCommandEvent& event);
		void OnRadioButton_RGB_Chase15Select(wxCommandEvent& event);
		void OnRadioButton_RGB_AlternateSelect(wxCommandEvent& event);
		void OnRadioButton_RGB_Twinkle5Select(wxCommandEvent& event);
		void OnRadioButton_RGB_Twinkle10Select(wxCommandEvent& event);
		void OnRadioButton_RGB_Twinkle25Select(wxCommandEvent& event);
		void OnRadioButton_RGB_Twinkle50Select(wxCommandEvent& event);
		void OnRadioButton_RGB_ShimmerSelect(wxCommandEvent& event);
		void OnRadioButton_RGB_BackgroundSelect(wxCommandEvent& event);
		void OnRadioButton_RGBCycle_OffSelect(wxCommandEvent& event);
		void OnRadioButton_RGBCycle_ABCSelect(wxCommandEvent& event);
		void OnRadioButton_RGBCycle_ABCAllSelect(wxCommandEvent& event);
		void OnRadioButton_RGBCycle_ABCAllNoneSelect(wxCommandEvent& event);
		void OnRadioButton_RGBCycle_MixedColorsSelect(wxCommandEvent& event);
		void OnRadioButton_Standard_OffSelect(wxCommandEvent& event);
		void OnRadioButton_Standard_ChaseSelect(wxCommandEvent& event);
		void OnRadioButton_Standard_Chase13Select(wxCommandEvent& event);
		void OnRadioButton_Standard_Chase14Select(wxCommandEvent& event);
		void OnRadioButton_Standard_Chase15Select(wxCommandEvent& event);
		void OnRadioButton_Standard_AlternateSelect(wxCommandEvent& event);
		void OnRadioButton_Standard_Twinkle5Select(wxCommandEvent& event);
		void OnRadioButton_Standard_Twinkle10Select(wxCommandEvent& event);
		void OnRadioButton_Standard_Twinkle25Select(wxCommandEvent& event);
		void OnRadioButton_Standard_Twinkle50Select(wxCommandEvent& event);
		void OnRadioButton_Standard_ShimmerSelect(wxCommandEvent& event);
		void OnRadioButton_Standard_BackgroundSelect(wxCommandEvent& event);
		void OnTimer1Trigger(wxTimerEvent& event);
		//*)

		void OnTreeListCtrl1Checkboxtoggled(wxTreeListEvent& event);
		void OnTreeListCtrl1ItemActivated(wxTreeListEvent& event);

		TreeController::CONTROLLERTYPE GetTreeItemType(const wxTreeListItem& item);
		void PopulateControllerTree(wxXmlDocument* network);
		void PopulateModelsTree(ModelManager* modelManager);
		void PopulateModelGroupsTree(ModelManager* modelManager);
		bool CascadeSelected(wxTreeListItem& item, wxCheckBoxState state);
		//void RollUpSelected(const wxTreeListItem& item, wxCheckBoxState state);
		void DestroyTreeControllerData(wxTreeListItem& item);
		void GetTestPresetNames(wxArrayString& PresetNames);
		bool CheckChannel(long chid, wxCheckBoxState state);
		std::list<std::string> GetModelsOnChannels(int start, int end);
		//void CascadeSelectedToModelGroup(std::string modelName, wxCheckBoxState state);
		//void CascadeSelectedToModel(std::string modelName, wxCheckBoxState state);
		void Clear(wxTreeListItem& item);
		void GetCheckedItems(wxArrayInt& chArray);
		bool InitialiseOutputs();
		void OnTimer(long curtime);
		double Rand01() { return (double)rand() / (double)RAND_MAX; };
		void TestButtonsOff();
		wxCheckBoxState RollUpAll(wxTreeListItem start);
		void CascadeModelDoesNotExist();
		void DeactivateNotClickableModels();

#ifdef __WXOSX__
		AppNapSuspender _sleepData;
		void EnableSleepModes()
		{
			_sleepData.resume();
		}
		void DisableSleepModes()
		{
			_sleepData.suspend();
		}
#else
		void EnableSleepModes() {}
		void DisableSleepModes() {}
#endif

		DECLARE_EVENT_TABLE()
};
#endif
