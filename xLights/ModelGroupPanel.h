#pragma once

/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include <wx/dnd.h>

//(*Headers(ModelGroupPanel)
#include <wx/bmpbuttn.h>
#include <wx/button.h>
#include <wx/checkbox.h>
#include <wx/choice.h>
#include <wx/clrpicker.h>
#include <wx/listctrl.h>
#include <wx/panel.h>
#include <wx/sizer.h>
#include <wx/spinctrl.h>
#include <wx/stattext.h>
#include <wx/textctrl.h>
//*)

class ModelManager;
class LayoutPanel;

wxDECLARE_EVENT(EVT_MGDROP, wxCommandEvent);

class MGTextDropTarget : public wxTextDropTarget
{
public:
    MGTextDropTarget(wxWindow *owner, wxListCtrl* list, wxString type) { _owner = owner; _list = list; _type = type; };

    virtual bool OnDropText(wxCoord x, wxCoord y, const wxString& data) override;
    virtual wxDragResult OnDragOver(wxCoord x, wxCoord y, wxDragResult def) override;

    wxWindow *_owner;
    wxListCtrl* _list;
    wxString _type;
};

class ModelGroupPanel : public wxPanel
{
	int _lastFirstSelectedModelIndex = -1;
	int _lastFirstSelectedModelInGroupIndex = -1;

	int GetModelsVisibleInList(wxListCtrl* list);

public:

	ModelGroupPanel(wxWindow* parent, ModelManager& Models, LayoutPanel* xl, wxWindowID id = wxID_ANY, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize);
	virtual ~ModelGroupPanel();

	void UpdatePanel(const std::string& group);
	void AddPreviewChoice(const std::string& name);
	std::string GetGroupName() const { return mGroup; }

	//(*Declarations(ModelGroupPanel)
	wxBitmapButton* ButtonAddModel;
	wxBitmapButton* ButtonMoveDown;
	wxBitmapButton* ButtonMoveUp;
	wxBitmapButton* ButtonRemoveModel;
	wxButton* ButtonAliases;
	wxButton* ButtonClearFilter;
	wxCheckBox* CheckBox_ShowInactiveModels;
	wxCheckBox* CheckBox_ShowModelGroups;
	wxCheckBox* CheckBox_ShowOnlyModelsInCurrentView;
	wxCheckBox* CheckBox_ShowSubmodels;
	wxChoice* ChoiceModelLayoutType;
	wxChoice* ChoicePreviews;
	wxChoice* Choice_DefaultCamera;
	wxColourPickerCtrl* ColourPickerCtrl_ModelGroupTagColour;
	wxFlexGridSizer* Panel_Sizer;
	wxListCtrl* ListBoxAddToModelGroup;
	wxListCtrl* ListBoxModelsInGroup;
	wxSpinCtrl* SizeSpinCtrl;
	wxSpinCtrl* SpinCtrl_XCentreOffset;
	wxSpinCtrl* SpinCtrl_YCentreOffset;
	wxStaticText* GridSizeLabel;
	wxStaticText* LabelModelGroupName;
	wxStaticText* StaticText10;
	wxStaticText* StaticText11;
	wxStaticText* StaticText12;
	wxStaticText* StaticText1;
	wxStaticText* StaticText2;
	wxStaticText* StaticText3;
	wxStaticText* StaticText5;
	wxStaticText* StaticText7;
	wxStaticText* StaticText8;
	wxStaticText* StaticText9;
	wxTextCtrl* TextCtrl_Filter;
	//*)

protected:

	//(*Identifiers(ModelGroupPanel)
	static const long ID_STATICTEXT5;
	static const long ID_STATICTEXT6;
	static const long ID_CHOICE1;
	static const long ID_STATICTEXT12;
	static const long ID_CHOICE2;
	static const long ID_STATICTEXT4;
	static const long ID_SPINCTRL1;
	static const long ID_CHOICE_PREVIEWS;
	static const long ID_STATICTEXT7;
	static const long ID_STATICTEXT10;
	static const long ID_SPINCTRL2;
	static const long ID_STATICTEXT11;
	static const long ID_SPINCTRL3;
	static const long ID_STATICTEXT8;
	static const long ID_COLOURPICKERCTRL_MG_TAGCOLOUR;
	static const long ID_BUTTON2;
	static const long ID_CHECKBOX1;
	static const long ID_CHECKBOX3;
	static const long ID_CHECKBOX2;
	static const long ID_CHECKBOX4;
	static const long ID_STATICTEXT3;
	static const long ID_STATICTEXT2;
	static const long ID_STATICTEXT9;
	static const long ID_TEXTCTRL1;
	static const long ID_BUTTON1;
	static const long ID_LISTCTRL1;
	static const long ID_BITMAPBUTTON4;
	static const long ID_BITMAPBUTTON3;
	static const long ID_BITMAPBUTTON1;
	static const long ID_BITMAPBUTTON2;
	static const long ID_STATICTEXT1;
	static const long ID_LISTCTRL2;
	//*)

	static const long ID_MNU_CLEARALL;
	static const long ID_MNU_COPY;
	static const long ID_MNU_SORTBYNAME;
    static const long ID_MNU_SORTBYLOCATION;

private:
	LayoutPanel* layoutPanel = nullptr;
	ModelManager& mModels;
	std::string mGroup;
	bool _dragRowModel = false;
	bool _dragRowNonModel = false;

	//(*Handlers(ModelGroupPanel)
	void OnButtonAddToModelGroupClick(wxCommandEvent& event);
	void OnButtonRemoveFromModelGroupClick(wxCommandEvent& event);
	void OnButtonUpClick(wxCommandEvent& event);
	void OnButtonDownClick(wxCommandEvent& event);
	void OnChoiceModelLayoutTypeSelect(wxCommandEvent& event);
	void OnChoicePreviewsSelect(wxCommandEvent& event);
	void OnSizeSpinCtrlChange(wxSpinEvent& event);
	void OnListBoxAddToModelGroupBeginDrag(wxListEvent& event);
	void OnListBoxAddToModelGroupItemSelect(wxListEvent& event);
	void OnListBoxModelsInGroupBeginDrag(wxListEvent& event);
	void OnListBoxModelsInGroupItemSelect(wxListEvent& event);
	void OnCheckBox_ShowSubmodelsClick(wxCommandEvent& event);
	void OnListBoxAddToModelGroupItemActivated(wxListEvent& event);
	void OnListBoxModelsInGroupItemActivated(wxListEvent& event);
	void OnCheckBox_ShowModelGroupsClick(wxCommandEvent& event);
	void OnListBoxModelsInGroupItemDeselect(wxListEvent& event);
	void OnListBoxAddToModelGroupItemDeselect(wxListEvent& event);
	void OnSpinCtrl_XCentreOffsetChange(wxSpinEvent& event);
	void OnSpinCtrl_YCentreOffsetChange(wxSpinEvent& event);
	void OnListBoxModelsInGroupItemRClick(wxListEvent& event);
	void OnButtonClearFilterClick(wxCommandEvent& event);
	void OnTextCtrl_FilterText(wxCommandEvent& event);
	void OnCheckBox_ShowInactiveModelsClick(wxCommandEvent& event);
	void OnCheckBox_ShowOnlyModelsInCurrentViewClick(wxCommandEvent& event);
	void OnColourPickerCtrl_ModelGroupTagColourColourChanged(wxColourPickerEvent& event);
	void OnChoice_DefaultCameraSelect(wxCommandEvent& event);
	void OnButtonAliasesClick(wxCommandEvent& event);
	//*)

	DECLARE_EVENT_TABLE()

	void OnDrop(wxCommandEvent& event);
	bool IsItemSelected(wxListCtrl* ctrl, int item);
	void ValidateWindow();
	int GetSelectedModelCount();
	void OnPopup(wxCommandEvent& event);

	void SaveGroupChanges(bool updateCentre = false);
	void AddSelectedModels(int index);
	void RemoveSelectedModels();
	void MoveSelectedModelsTo(int indexTo);
	void ClearSelections(wxListCtrl* listCtrl, long stateMask);
	int GetFirstSelectedModel(wxListCtrl* list);
	void ResizeColumns();
	void SortModelsByName();
    void SortModelsByLocation();
	void CopyModelList();
	wxArrayString getGroupList();
    void OnSpinCtrlTextEnter(wxCommandEvent& evt);
};
