#ifndef TESTDIALOG_H
#define TESTDIALOG_H

// Need to do these manually due to issues with wxSmith and wxTreeListCtrl
#include <wx/treelist.h>
#include <wx/treectrl.h>
#include <wx/xml/xml.h>

//(*Headers(TestDialog)
#include <wx/splitter.h>
#include <wx/aui/aui.h>
#include <wx/dialog.h>
//*)

class TreeController
{
public:
	typedef enum CONTROLLERTYPE
	{
		CT_E131, CT_NULL, CT_DMX, CT_CHANNEL, CT_ROOT
	} CONTROLLERTYPE;
private:
	std::string _name;
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
	CONTROLLERTYPE _type;
	std::string GenerateName();
	TreeController(CONTROLLERTYPE type, std::string comport, int universe, int startxlightschannel, int channels, bool inactive, bool multiuniversedmx, std::string description);
	int _universes;
	bool _multiuniversedmx;
	wxTreeListItem* _treeListItem;

public:
	TreeController(wxXmlNode* n, int xlightsstartchannel, int nullcount);
	TreeController(int channel, CONTROLLERTYPE type, int xLightsChannel);
	TreeController(CONTROLLERTYPE type, int start, int end);
	TreeController* GenerateDMXUniverse(int universeoffset);
	std::string Name() { return _name; };
	bool IsNULL() { return _type == CONTROLLERTYPE::CT_NULL; };
	bool Inactive() { return _inactive; };
	int Channels() { return _endchannel - _startchannel + 1; };
	int StartChannel() { return _startchannel; };
	int EndChannel() { return _endchannel; };
	int StartXLightsChannel() { return _startxlightschannel; };
	void SetTreeListItem(wxTreeListItem* tli) { _treeListItem = tli; };
	wxTreeListItem* GetTreeListItem() { return _treeListItem; };
	int EndXLightsChannel() { return _endxlightschannel; };
};

class TreeChannel
{

};

class TestDialog: public wxDialog
{
	public:

		TestDialog(wxWindow* parent, wxXmlDocument* network, wxWindowID id=wxID_ANY);
		virtual ~TestDialog();
		wxTreeListCtrl* TreeListCtrl_Channels;
		wxTreeListItem  _all;

		//(*Declarations(TestDialog)
		wxAuiNotebook* AuiNotebook1;
		wxSplitterWindow* SplitterWindow1;
		//*)

	protected:

		wxXmlDocument* _network;
		static const long ID_TREELISTCTRL_Channels;

		//(*Identifiers(TestDialog)
		static const long ID_AUINOTEBOOK1;
		static const long ID_SPLITTERWINDOW1;
		//*)

	private:

		//(*Handlers(TestDialog)
		//*)

		void OnTreeListCtrl1ItemGetToolTip(wxTreeEvent& event);
		void OnTreeListCtrl1Checkboxtoggled(wxTreeListEvent& event);

		void PopulateTree(wxXmlDocument* network);
		void SetSelected(wxTreeListItem& item, wxCheckBoxState state);
		void UpdateSelectedState(wxTreeListItem& item, wxCheckBoxState state);
		void DestroyTreeControllerData(wxTreeListItem& item);

		DECLARE_EVENT_TABLE()
};

#endif
