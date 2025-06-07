#include "ServicesPanel.h"

#include "xLightsMain.h"
#include "ai/chatGPT.h"
#include "ai/ServiceManager.h"

//(*InternalHeaders(ServicesPanel)
#include <wx/intl.h>
#include <wx/string.h>
//*)

//(*IdInit(ServicesPanel)
const wxWindowID ServicesPanel::ID_PROPERYMANAGER_SERVICES = wxNewId();
const wxWindowID ServicesPanel::ID_CHOICE_SERVICES = wxNewId();
const wxWindowID ServicesPanel::ID_BUTTON_TEST = wxNewId();
//*)

BEGIN_EVENT_TABLE(ServicesPanel, wxPanel)
//(*EventTable(ServicesPanel)
//*)
END_EVENT_TABLE()
/*
class EditableListCtrl : public wxListCtrl {
public:
    EditableListCtrl(ServicesPanel* p, wxWindowID id, const wxPoint& pos, const wxSize& size, long style, const wxValidator& validator, const wxString& name) :
        wxListCtrl(p, id, pos, size, style, validator, name), parent(p) {
        Bind(wxEVT_LEFT_DOWN, &EditableListCtrl::OnLeftDown, this);
        Bind(wxEVT_TEXT_ENTER, &EditableListCtrl::OnTextEnter, this);
    }

    void SetServiceManager(ServiceManager* sm) {
        m_sm = sm;
    }

private:
    wxTextCtrl* m_textCtrl = nullptr;
    int m_editingRow = -1;
    int m_editingCol = -1;
    ServicesPanel *parent;
    ServiceManager* m_sm{nullptr};

    void OnLeftDown(wxMouseEvent& event) {
        int flags;
        int row = HitTest(event.GetPosition(), flags, nullptr);

        if (row != wxNOT_FOUND) {
            // check this cell is editable
            if (EditCell(row, event.GetX())) {
                event.Skip();
            }
        } else {
            event.Skip();
        }
    }

    bool EditCell(long item, int xpos) {
        if (m_textCtrl) {
            m_textCtrl->Destroy();
        }

        std::string service = GetItemText(item, 0);
        auto* svc = m_sm->getService(service);
        for (int col = 0; col <= 3; col++) {
            wxRect rect;
            GetSubItemRect(item, col, rect);
            //if (xpos >= rect.x && xpos < (rect.x + rect.width)) {
            //    if ((col == 1 && svc.idName != "") ||
            //        (col == 2 && svc.secretName1 != "") ||
            //        (col == 3 && svc.secretName2 != "")) {
            //        m_textCtrl = new wxTextCtrl(this, wxID_ANY, GetItemText(item, col),
            //                                    wxPoint(rect.x, rect.y), wxSize(rect.width, rect.height),
            //                                    wxTE_PROCESS_ENTER);
            //        m_textCtrl->SetFocus();
            //        m_textCtrl->SetSelection(-1, -1);
            //
            //        m_editingRow = item;
            //        m_editingCol = col;
            //        return false;
            //    }
            //    return true;
            //}
        }
        return true;

    }

    void OnTextEnter(wxCommandEvent& event) {
        if (m_textCtrl) {
            SetItem(m_editingRow, m_editingCol, m_textCtrl->GetValue());
            //if (wxPreferencesEditor::ShouldApplyChangesImmediately()) {
            //    parent->TransferDataFromWindow();
            //}
            m_textCtrl->Destroy();
            m_textCtrl = nullptr;
        }
    }

    void OnKillFocus(wxFocusEvent& event) {
		if (m_textCtrl) {
			SetItem(m_editingRow, m_editingCol, m_textCtrl->GetValue());
            //if (wxPreferencesEditor::ShouldApplyChangesImmediately()) {
            //    parent->TransferDataFromWindow();
            //}
			m_textCtrl->Destroy();
			m_textCtrl = nullptr;
		}
	}
};
*/
ServicesPanel::ServicesPanel(wxWindow* parent, ServiceManager* sm, wxWindowID id, const wxPoint& pos, const wxSize& size) :
    m_serviceManager(sm) {
	//(*Initialize(ServicesPanel)
	wxBoxSizer* BoxSizer1;
	wxFlexGridSizer* FlexGridSizer1;

	Create(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("wxID_ANY"));
	FlexGridSizer1 = new wxFlexGridSizer(0, 1, 0, 0);
	FlexGridSizer1->AddGrowableCol(0);
	FlexGridSizer1->AddGrowableRow(0);
	servicesGrid = new wxPropertyGrid(this,ID_PROPERYMANAGER_SERVICES,wxDefaultPosition,wxDefaultSize,wxPG_TOOLBAR|wxPG_SPLITTER_AUTO_CENTER|wxPG_DEFAULT_STYLE,_T("ID_PROPERYMANAGER_SERVICES"));
	FlexGridSizer1->Add(servicesGrid, 1, wxALL|wxEXPAND, 5);
	BoxSizer1 = new wxBoxSizer(wxHORIZONTAL);
	ChoiceServicesTest = new wxChoice(this, ID_CHOICE_SERVICES, wxDefaultPosition, wxDefaultSize, 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE_SERVICES"));
	BoxSizer1->Add(ChoiceServicesTest, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	ButtonTest = new wxButton(this, ID_BUTTON_TEST, _("Test"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON_TEST"));
	BoxSizer1->Add(ButtonTest, 0, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer1->Add(BoxSizer1, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	SetSizer(FlexGridSizer1);
	FlexGridSizer1->SetSizeHints(this);

	Connect(ID_BUTTON_TEST, wxEVT_COMMAND_BUTTON_CLICKED, (wxObjectEventFunction)&ServicesPanel::OnButtonTestClick);
	//*)
    servicesGrid->SetPropertyAttributeAll(wxPG_BOOL_USE_CHECKBOX, true);
    servicesGrid->Connect(wxEVT_PG_CHANGED, (wxObjectEventFunction)&ServicesPanel::OnPropertyGridChange, 0, this);

    for (auto const& ss : m_serviceManager->getServices()) {
        ChoiceServicesTest->Append(ss->GetLLMName());
    }

#ifdef _MSC_VER
    MSWDisableComposited();
#endif
}

ServicesPanel::~ServicesPanel()
{
	//(*Destroy(ServicesPanel)
	//*)
}

bool ServicesPanel::TransferDataToWindow() {
    for (auto const& ss : m_serviceManager->getServices()) {
        ss->PopulateLLMSettings(servicesGrid);
    }
    return true;
}

bool ServicesPanel::TransferDataFromWindow() {

    for (auto const& ss : m_serviceManager->getServices()) {
        ss->SaveSettings();
    }

    return true;
}

void ServicesPanel::OnButtonTestClick(wxCommandEvent& event) {

    auto ts = m_serviceManager->getService(ChoiceServicesTest->GetStringSelection());
    if (!ts) {
        return;
    }
    if (!ts->IsAvailable()) {
        wxMessageBox("Service " + ts->GetLLMName() + " is not yet configured or enabled. Please check configure parameter(s).", "Error", wxICON_ERROR);
        return;
    }
    auto const[msg, work] = ts->TestLLM();
    if (work) {
        wxMessageBox("Service " + ts->GetLLMName() + " is valid", "Success", wxICON_INFORMATION);
    } else {
        wxMessageBox("Service " + ts->GetLLMName() + " returned: " + msg, "Error", wxICON_ERROR);
    }
}

void ServicesPanel::OnPropertyGridChange(wxPropertyGridEvent& event) {
    wxString const name = event.GetPropertyName();
    auto const names = wxSplit(name, '.');
    if (names.size() < 2) {
        return; // Not a valid service property
    }
    m_serviceManager->getService(names[0])->SetSetting(name, event.GetPropertyValue());

    if (wxPreferencesEditor::ShouldApplyChangesImmediately()) {
        TransferDataFromWindow();
    }
}
