#include "ServicesPanel.h"

#include "xLightsMain.h"
#include "chatGPT.h"

//(*InternalHeaders(ServicesPanel)
#include <wx/intl.h>
#include <wx/string.h>
//*)

//(*IdInit(ServicesPanel)
const long ServicesPanel::ID_LISTCTRL1 = wxNewId();
const long ServicesPanel::ID_BUTTON1 = wxNewId();
//*)

BEGIN_EVENT_TABLE(ServicesPanel, wxPanel)
//(*EventTable(ServicesPanel)
//*)
END_EVENT_TABLE()

typedef struct SERVICE_DEFINITION {
	std::string name;
    std::string idName;
	std::string secretName1;
	std::string secretName2;
} SERVICE_DEFINITION;

static const std::list<SERVICE_DEFINITION> services = { { "ChatGPT", "", "BearerToken", "" } };

const SERVICE_DEFINITION& FindService(const std::string& name) {
    for (const auto& service : services) {
        if (service.name == name) {
            return service;
        }
    }
    return services.front(); // this is dodgy but it should never happen
}

bool IsServiceValid(const std::string& service, const std::string& id, const std::string& secret1, const std::string& secret2)
{
    const SERVICE_DEFINITION& svc = FindService(service);
    return (svc.idName == "" || !id.empty()) &&
		   (svc.secretName1 == "" || !secret1.empty()) &&
		   (svc.secretName2 == "" || !secret2.empty());
}

class EditableListCtrl : public wxListCtrl {
public:
    EditableListCtrl(wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style, const wxValidator& validator, const wxString& name) :
        wxListCtrl(parent, id, pos, size, style, validator, name) {
        Bind(wxEVT_LEFT_DOWN, &EditableListCtrl::OnLeftDown, this);
        Bind(wxEVT_TEXT_ENTER, &EditableListCtrl::OnTextEnter, this);
    }

private:
    wxTextCtrl* m_textCtrl = nullptr;
    int m_editingRow = -1;
    int m_editingCol = -1;

    void OnLeftDown(wxMouseEvent& event) {
        long col;
        int flags;
        int row = HitTest(event.GetPosition(), flags, &col);

        if (row != wxNOT_FOUND && col >= 1 && col <= 3) {
            // check this cell is editable
            std::string service = GetItemText(row, 0);
            const SERVICE_DEFINITION& svc = FindService(service);
            if ((col == 1 && svc.idName != "") ||
                (col == 2 && svc.secretName1 != "") ||
                (col == 3 && svc.secretName2 != "")) {
                EditCell(row, col);
            } else {
                event.Skip();
            }
        } else {
            event.Skip();
        }
    }

    void EditCell(long item, int col) {
        if (m_textCtrl) {
            m_textCtrl->Destroy();
        }

        wxRect rect;
        GetSubItemRect(item, col, rect);

        m_textCtrl = new wxTextCtrl(this, wxID_ANY, GetItemText(item, col),
                                    wxPoint(rect.x, rect.y), wxSize(rect.width, rect.height),
                                    wxTE_PROCESS_ENTER);
        m_textCtrl->SetFocus();
        m_textCtrl->SetSelection(-1, -1);

        m_editingRow = item;
        m_editingCol = col;
    }

    void OnTextEnter(wxCommandEvent& event) {
        if (m_textCtrl) {
            SetItem(m_editingRow, m_editingCol, m_textCtrl->GetValue());
            m_textCtrl->Destroy();
            m_textCtrl = nullptr;
        }
    }

    void OnKillFocus(wxFocusEvent& event) {
		if (m_textCtrl) {
			SetItem(m_editingRow, m_editingCol, m_textCtrl->GetValue());
			m_textCtrl->Destroy();
			m_textCtrl = nullptr;
		}
	}
};

ServicesPanel::ServicesPanel(wxWindow* parent, xLightsFrame* f, wxWindowID id, const wxPoint& pos, const wxSize& size) : frame(f) {
	//(*Initialize(ServicesPanel)
	wxFlexGridSizer* FlexGridSizer1;

	Create(parent, id, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("id"));
	FlexGridSizer1 = new wxFlexGridSizer(0, 1, 0, 0);
	servicesList = new EditableListCtrl(this, ID_LISTCTRL1, wxDefaultPosition, wxSize(600,-1), wxLC_REPORT|wxVSCROLL|wxHSCROLL, wxDefaultValidator, _T("ID_LISTCTRL1"));
	FlexGridSizer1->Add(servicesList, 1, wxALL|wxEXPAND, 5);
	ButtonTest = new wxButton(this, ID_BUTTON1, _("Test"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON1"));
	FlexGridSizer1->Add(ButtonTest, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	SetSizer(FlexGridSizer1);
	FlexGridSizer1->Fit(this);
	FlexGridSizer1->SetSizeHints(this);

	Connect(ID_BUTTON1,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&ServicesPanel::OnButtonTestClick);
	//*)

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
	servicesList->ClearAll();
    servicesList->InsertColumn(0, "Service");
    servicesList->InsertColumn(1, "Id");
    servicesList->InsertColumn(2, "Secret 1");
    servicesList->InsertColumn(3, "Secret 2");

	for (const auto& service : services) {
		long index = servicesList->InsertItem(servicesList->GetItemCount(), service.name);
        servicesList->SetItem(index, 1, frame->GetServiceSetting(service.name + service.idName));
        servicesList->SetItem(index, 2, frame->GetServiceSetting(service.name + service.secretName1));
        servicesList->SetItem(index, 3, frame->GetServiceSetting(service.name + service.secretName2));
	}

    return true;
}

bool ServicesPanel::TransferDataFromWindow() {
    for (int i = 0; i < servicesList->GetItemCount(); ++i) {

        const SERVICE_DEFINITION& svc = FindService(servicesList->GetItemText(i, 0));

		frame->SetServiceSetting(svc.name + svc.idName, servicesList->GetItemText(i, 1));
		frame->SetServiceSetting(svc.name + svc.secretName1, servicesList->GetItemText(i, 2));
		frame->SetServiceSetting(svc.name + svc.secretName2, servicesList->GetItemText(i, 3));
    }

    return true;
}

void ServicesPanel::OnButtonTestClick(wxCommandEvent& event) {
    for (int i = 0; i < servicesList->GetItemCount(); ++i) {
        if (IsServiceValid(servicesList->GetItemText(i, 0), servicesList->GetItemText(i,1), servicesList->GetItemText(i,2), servicesList->GetItemText(i,3)))
        {
            if (servicesList->GetItemText(i, 0) == "ChatGPT") {
                if (TestChatGPT(frame, servicesList->GetItemText(i, 2))) {
                    wxMessageBox("Service " + servicesList->GetItemText(i, 0) + " is valid", "Success", wxICON_INFORMATION);
                } else {
                    wxMessageBox("Service " + servicesList->GetItemText(i, 0) + " is not valid", "Error", wxICON_ERROR);
                }
            }
        }
    }
}