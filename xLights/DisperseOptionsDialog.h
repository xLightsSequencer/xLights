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

//(*Headers(DisperseOptionsDialog)
#include <wx/bmpbuttn.h>
#include <wx/button.h>
#include <wx/checkbox.h>
#include <wx/choice.h>
#include <wx/dialog.h>
#include <wx/gbsizer.h>
#include <wx/listctrl.h>
#include <wx/notebook.h>
#include <wx/panel.h>
#include <wx/sizer.h>
#include <wx/spinctrl.h>
#include <wx/stattext.h>
#include <wx/textctrl.h>
//*)

#include <wx/dnd.h>

enum DisperseDirection {
    NONE,
    UP, DOWN, LEFT, RIGHT,
    UPLEFT, UPRIGHT, DOWNLEFT, DOWNRIGHT,
    BACK, BACKLEFT, BACKRIGHT,
    FRONT, FRONTLEFT, FRONTRIGHT
};

class DisperseOptionsDialogTextDropTarget : public wxTextDropTarget
{
public:
    DisperseOptionsDialogTextDropTarget(wxWindow *owner, wxListCtrl* list, wxString type) { _owner = owner; _list = list; _type = type; };

    virtual bool OnDropText(wxCoord x, wxCoord y, const wxString& data) override;
    virtual wxDragResult OnDragOver(wxCoord x, wxCoord y, wxDragResult def) override;

    wxWindow *_owner;
    wxListCtrl* _list;
    wxString _type;
};

class DisperseOptionsDialog: public wxDialog
{
    public:
    
        struct DisperseOptions {
            wxString fromWhat;
            DisperseDirection direction;
            float offset;
            std::vector<wxString> order;
        };
    
        struct SetupData {
            wxString primarySelection;
            wxArrayString availableTargets;
            wxArrayString objectsToDisperse;
            bool primaryLocked = false;
            bool is3d = true;
            bool forModels = true;
        };
        
        DisperseOptionsDialog(wxWindow* parent, wxWindowID id=wxID_ANY,const wxPoint& pos=wxDefaultPosition,const wxSize& size=wxDefaultSize);
        virtual ~DisperseOptionsDialog();
    
        void Setup(SetupData data);
        DisperseOptions GetDisperseOptions();
    
        //(*Declarations(DisperseOptionsDialog)
        wxBitmapButton* ButtonOrderDown;
        wxBitmapButton* ButtonOrderUp;
        wxButton* ButtonCancel;
        wxButton* ButtonOK;
        wxCheckBox* CheckBoxUsePrimarySel;
        wxCheckBox* DirectionBack;
        wxCheckBox* DirectionBackLeft;
        wxCheckBox* DirectionBackRight;
        wxCheckBox* DirectionDown;
        wxCheckBox* DirectionDownLeft;
        wxCheckBox* DirectionDownRight;
        wxCheckBox* DirectionFront;
        wxCheckBox* DirectionFrontLeft;
        wxCheckBox* DirectionFrontRight;
        wxCheckBox* DirectionFviewLeft;
        wxCheckBox* DirectionFviewRight;
        wxCheckBox* DirectionTviewLeft;
        wxCheckBox* DirectionTviewRight;
        wxCheckBox* DirectionUp;
        wxCheckBox* DirectionUpLeft;
        wxCheckBox* DirectionUpRight;
        wxChoice* ChoiceDisperseFrom;
        wxFlexGridSizer* DisperseOrderSizer;
        wxGridBagSizer* GridBagSizerFrontView;
        wxListView* DisperseOrderList;
        wxNotebook* DirectionNotebook;
        wxPanel* DirectionPanelFrontView;
        wxPanel* DirectionPanelTopView;
        wxSpinCtrlDouble* SpinCtrlOffset;
        wxStaticText* DisperseOrderLabel;
        wxStaticText* StaticText2;
        wxStaticText* StaticText4;
        wxTextCtrl* DirectionFrontLabel;
        wxTextCtrl* DirectionTopLabel;
        //*)

    protected:

        //(*Identifiers(DisperseOptionsDialog)
        static const long ID_STATICTEXT4;
        static const long ID_CHOICE_DISPERSE_FROM;
        static const long ID_CHECKBOX_USE_PRIMARY;
        static const long ID_STATICTEXT2;
        static const long ID_SPINCTRL_OFFSET;
        static const long ID_CB_FVIEW_UP_LEFT;
        static const long ID_CB_FVIEW_LEFT;
        static const long ID_CB_FVIEW_DOWN_LEFT;
        static const long ID_CB_FVIEW_DOWN;
        static const long ID_CB_FVIEW_UP;
        static const long ID_CB_FVIEW_RIGHT;
        static const long ID_CB_FVIEW_DOWN_RIGHT;
        static const long ID_TEXTCTRL1;
        static const long ID_CB_FVIEW_UP_RIGHT;
        static const long ID_PANEL1;
        static const long ID_CB_TVIEW_BACK_LEFT;
        static const long ID_CB_TVIEW_LEFT;
        static const long ID_CB_TVIEW_FRONT_LEFT;
        static const long ID_CB_TVIEW_FRONT;
        static const long ID_CB_TVIEW_FRONT_RIGHT;
        static const long ID_CB_TVIEW_RIGHT;
        static const long ID_CB_TVIEW_BACK;
        static const long ID_CB_TVIEW_BACK_RIGHT;
        static const long ID_TEXTCTRL2;
        static const long ID_PANEL2;
        static const long ID_NOTEBOOK1;
        static const long ID_STATICTEXT3;
        static const long ID_DISPERSE_ORDER_LIST;
        static const long ID_BUTTON_ORDER_UP;
        static const long ID_BUTTON_ORDER_DOWN;
        static const long ID_BUTTON_CANCEL;
        static const long ID_BUTTON_OK;
        //*)
    
        wxString GetTarget();
        DisperseDirection GetDirection();
        float GetOffset();
        wxArrayString GetDisperseOrder();
        void ValidateWindow();
        void PopulateOrderList(wxArrayString objectList);
        void RemoveItemFromOrderList(wxString objectName);
        void PopulateTargetChoices(wxArrayString targetList);
        void StepSelectedItems(bool up);
        void MoveSelectedItems(int whereTo);
        DisperseDirection GetDirectionByControlId(long ctrlId);
    
    private:

        //(*Handlers(DisperseOptionsDialog)
        void OnOrderListBeginDrag(wxListEvent& event);
        void OnOrderListSelect(wxListEvent& event);
        void OnButtonOrderUpClick(wxCommandEvent& event);
        void OnButtonOrderDownClick(wxCommandEvent& event);
        void OnResize(wxSizeEvent& event);
        void OnChoiceTargetSelect(wxCommandEvent& event);
        void OnCheckBoxUsePrimarySelClick(wxCommandEvent& event);
        void OnInit(wxInitDialogEvent& event);
        void OnDisperseOrderListBeginDrag(wxListEvent& event);
        void OnDisperseOrderListItemSelect(wxListEvent& event);
        void OnDisperseOrderListItemDeselect(wxListEvent& event);
        void OnDisperseOrderListItemActivated(wxListEvent& event);
        void OnButtonCancelClick(wxCommandEvent& event);
        void OnButtonOKClick(wxCommandEvent& event);
        void OnDirectionClick(wxCommandEvent& event);
        void OnSpinCtrlOffsetChange(wxSpinDoubleEvent& event);
        //*)
    
        bool _is3d;
        
        wxString _primarySelection;
        wxArrayString _availableTargets;
        wxArrayString _objectsToDisperse;
        DisperseDirection _selectedDirection = DisperseDirection::NONE;
        std::vector<wxCheckBox*> _directionCbList;
    
        void FillDirectionCbList();
        void OnDrop(wxCommandEvent& event);
    
        DECLARE_EVENT_TABLE()
};
