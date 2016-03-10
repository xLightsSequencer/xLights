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
#include <wx/slider.h>
#include <wx/panel.h>
#include <wx/button.h>
#include <wx/dialog.h>
//*)

#include <string>
#include <list>
#include "models/ModelManager.h"

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
	CONTROLLERTYPE _type;
	std::string GenerateName();
	TreeController(CONTROLLERTYPE type, std::string comport, int universe, int startxlightschannel, int channels, bool inactive, bool multiuniversedmx, std::string description);
	int _universes;
	bool _multiuniversedmx;
	wxTreeListItem* _treeListItem;
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
	int Channels() { return _endchannel - _startchannel + 1; };
	int StartChannel() { return _startchannel; };
	int EndChannel() { return _endchannel; };
	int StartXLightsChannel() { return _startxlightschannel; };
	void SetTreeListItem(wxTreeListItem* tli) { _treeListItem = tli; };
	wxTreeListItem* GetTreeListItem() { return _treeListItem; };
	int EndXLightsChannel() { return _endxlightschannel; };
	void SetEndXLightsChannel(int modelendchannel) { _endxlightschannel = modelendchannel; GenerateName(); };
	void SetStartXLightsChannel(int modelstartchannel) { _startxlightschannel = modelstartchannel; GenerateName(); };
	void SetNodes(int nodes) { _nodes = nodes; GenerateName(); };
	CONTROLLERTYPE GetType() { return _type; };
	std::string ModelName() { return _modelName; };
	bool ContainsChannel(int ch);
};

class TestDialog: public wxDialog
{
	public:

		TestDialog(wxWindow* parent, wxXmlDocument* network, wxFileName networkFile, ModelManager* modelManager, wxWindowID id=wxID_ANY);
		virtual ~TestDialog();
		wxTreeListCtrl* TreeListCtrl_Channels;
		wxTreeListItem  _controllers;
		wxTreeListItem  _modelGroups;
		wxTreeListItem  _models;
		wxFileName _networkFile;
		ModelManager* _modelManager;
		bool _modelCopiedOnRollup;

		//(*Declarations(TestDialog)
		wxPanel* Panel5;
		wxPanel* Panel4;
		wxButton* Button_Save;
		wxPanel* Panel1;
		wxStaticText* StaticText1;
		wxPanel* Panel3;
		wxAuiNotebook* AuiNotebook1;
		wxSlider* Slider_Speed;
		wxPanel* Panel2;
		wxSplitterWindow* SplitterWindow1;
		wxButton* Button_Load;
		//*)

	protected:

		wxXmlDocument* _network;
		static const long ID_TREELISTCTRL_Channels;

		//(*Identifiers(TestDialog)
		static const long ID_BUTTON_Load;
		static const long ID_BUTTON_Save;
		static const long ID_PANEL1;
		static const long ID_PANEL3;
		static const long ID_PANEL4;
		static const long ID_PANEL5;
		static const long ID_AUINOTEBOOK1;
		static const long ID_STATICTEXT1;
		static const long ID_SLIDER_Speed;
		static const long ID_PANEL2;
		static const long ID_SPLITTERWINDOW1;
		//*)

	private:

		//(*Handlers(TestDialog)
		void OnButton_LoadClick(wxCommandEvent& event);
		void OnButton_SaveClick(wxCommandEvent& event);
		void OnSlider_SpeedCmdSliderUpdated(wxScrollEvent& event);
		//*)

		void OnTreeListCtrl1Checkboxtoggled(wxTreeListEvent& event);
		void OnTreeListCtrl1ItemActivated(wxTreeListEvent& event);

		TreeController::CONTROLLERTYPE GetTreeItemType(const wxTreeListItem& item);
		void PopulateControllerTree(wxXmlDocument* network);
		void PopulateModelsTree(ModelManager* modelManager);
		void PopulateModelGroupsTree(ModelManager* modelManager);
		bool CascadeSelected(wxTreeListItem& item, wxCheckBoxState state);
		void RollUpSelected(const wxTreeListItem& item, wxCheckBoxState state);
		void DestroyTreeControllerData(wxTreeListItem& item);
		void GetTestPresetNames(wxArrayString& PresetNames);
		bool CheckChannel(long chid, wxCheckBoxState state);
		std::list<std::string> GetModelsOnChannels(int start, int end);
		void CascadeSelectedToModelGroup(std::string modelName, wxCheckBoxState state);
		void CascadeSelectedToModel(std::string modelName, wxCheckBoxState state);
		void Clear(wxTreeListItem& item);

		DECLARE_EVENT_TABLE()
};
#endif
