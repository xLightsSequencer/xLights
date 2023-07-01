/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/smeighan/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/smeighan/xLights/blob/master/License.txt
 **************************************************************/
 
//(*InternalHeaders(JukeboxPanel)
#include <wx/intl.h>
#include <wx/string.h>
//*)

#include <wx/button.h>

#include "JukeboxPanel.h"
#include "sequencer/MainSequencer.h"
#include "xLightsApp.h"
#include "xLightsMain.h"
#include "LinkJukeboxButtonDialog.h"
#include "UtilFunctions.h"

ButtonControl::ButtonControl(int i)
{
    _number = i;
    _type = LOOKUPTYPE::LTDISABLED;
}

ButtonControl::ButtonControl(int i, std::string description, std::string tooltip, bool loop)
{
    _tooltip = tooltip;
    _number = i;
    _loop = loop;
    _description = description;
    _type = LOOKUPTYPE::LTDESCRIPTION;
}

ButtonControl::ButtonControl(int i, std::string element, int layer, int time, std::string tooltip, bool loop)
{
    _element = element;
    _layer = layer;
    _time = time;
    _number = i;
    _tooltip = tooltip;
    _loop = loop;
    _type = LOOKUPTYPE::LTMLT;
}

ButtonControl::ButtonControl(wxXmlNode* n)
{
    _type = LOOKUPTYPE::LTDISABLED;

    if (n->GetName() == "Button") {
        _element = n->GetAttribute("Element", "").ToStdString();
        _layer = wxAtoi(n->GetAttribute("Layer", "-1"));
        _time = wxAtoi(n->GetAttribute("Time", "-1"));
        _number = wxAtoi(n->GetAttribute("Number", "-1"));
        _description = n->GetAttribute("Description", "").ToStdString();
        _tooltip = n->GetAttribute("Tooltip", "").ToStdString();
        _loop = n->GetAttribute("Loop", "TRUE") == "TRUE";
        _type = n->GetAttribute("Type", "") == "DESCRIPTION" ? LOOKUPTYPE::LTDESCRIPTION : LOOKUPTYPE::LTMLT;
    }
}

wxXmlNode* ButtonControl::Save()
{
    auto res = new wxXmlNode(wxXML_ELEMENT_NODE, "Button");
    res->AddAttribute("Type", _type == LOOKUPTYPE::LTDESCRIPTION ? "DESCRIPTION" : "MLT");
    res->AddAttribute("Description", _description);
    res->AddAttribute("Tooltip", _tooltip);
    res->AddAttribute("Element", _element);
    res->AddAttribute("Layer", wxString::Format("%d", _layer));
    res->AddAttribute("Time", wxString::Format("%d", _time));
    res->AddAttribute("Number", wxString::Format("%d", _number));
    res->AddAttribute("Loop", _loop ? "TRUE" : "FALSE");
    return res;
}

void ButtonControl::SelectEffect(MainSequencer* sequencer)
{
    if (sequencer != nullptr) {
        Effect* e = nullptr;
        if (_type == LOOKUPTYPE::LTDESCRIPTION) {
            e = sequencer->SelectEffectUsingDescription(_description);
        } else if (_type == LOOKUPTYPE::LTMLT) {
            e = sequencer->SelectEffectUsingElementLayerTime(_element, _layer - 1, _time);
        }

        if (e != nullptr) {
            sequencer->PanelEffectGrid->PlayLoopedEffect(e, _loop);
        } else {
            wxCommandEvent playEvent(EVT_STOP_SEQUENCE);
            wxPostEvent(sequencer->PanelEffectGrid->GetSequenceElements()->GetXLightsFrame(), playEvent);
        }
    }
}

//(*IdInit(JukeboxPanel)
//*)

BEGIN_EVENT_TABLE(JukeboxPanel,wxPanel)
	//(*EventTable(JukeboxPanel)
	//*)
END_EVENT_TABLE()

JukeboxPanel::JukeboxPanel(wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size)
{
    //(*Initialize(JukeboxPanel)
    Create(parent, id, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("id"));
    GridSizer1 = new wxGridSizer(0, 5, 0, 0);
    SetSizer(GridSizer1);

    Connect(wxEVT_SIZE,(wxObjectEventFunction)&JukeboxPanel::OnResize);
    //*)

    GridSizer1->SetCols(5);
    for (int i = 0; i < JUKEBOXBUTTONS; i++) {
        wxButton* button = new wxButton(this, wxID_ANY, wxString::Format("%d", i + 1), wxDefaultPosition, wxSize(BUTTONWIDTH, BUTTONHEIGHT),
                                        0, wxDefaultValidator, _T("ID_BITMAPBUTTON_JB") + wxString::Format("%d", i + 1));
        button->SetMinSize(wxSize(BUTTONWIDTH, BUTTONHEIGHT));
        button->SetMaxSize(wxSize(BUTTONWIDTH, BUTTONHEIGHT));
        GridSizer1->Add(button, 1, wxALL | wxEXPAND);
        Connect(button->GetId(), wxEVT_COMMAND_BUTTON_CLICKED, (wxObjectEventFunction)&JukeboxPanel::OnButtonClick);
        Connect(button->GetId(), wxEVT_CONTEXT_MENU, (wxObjectEventFunction)&JukeboxPanel::OnButtonRClick);
    }

    // This is used by xSchedule
    SetLabel("XLIGHTS_JUKEBOX");

    wxSizeEvent evt;
    OnResize(evt);

    GridSizer1->Fit(this);
    GridSizer1->SetSizeHints(this);

    ValidateWindow();
}

JukeboxPanel::~JukeboxPanel()
{
    //(*Destroy(JukeboxPanel)
    //*)

    for (const auto& it : _buttons) {
        SetButtonTooltip(it.second->_number, "");
        delete it.second;
    }
}

wxXmlNode* JukeboxPanel::Save()
{
    auto res = new wxXmlNode(wxXML_ELEMENT_NODE, "Jukebox");
    for (const auto& it : _buttons) {
        res->AddChild(it.second->Save());
    }

    return res;
}

void JukeboxPanel::SetButtonTooltip(int b, std::string tooltip)
{
    wxWindow* button = FindWindowByLabel(wxString::Format("%d", b), this);
    if (button != nullptr) {
        if (tooltip == "") {
            button->UnsetToolTip();
        } else {
            button->SetToolTip(tooltip);
        }
    }
}

void JukeboxPanel::Load(wxXmlNode* node)
{
    for (const auto& it : _buttons) {
        SetButtonTooltip(it.second->_number, "");
        delete it.second;
    }
    _buttons.clear();

    if (node->GetName() == "Jukebox") {
        for (wxXmlNode* n = node->GetChildren(); n != nullptr; n = n->GetNext()) {
            ButtonControl* b = new ButtonControl(n);
            _buttons[b->_number] = b;
            SetButtonTooltip(b->_number, b->_tooltip);
        }
    }
    ValidateWindow();
}

void JukeboxPanel::PlayItem(int item)
{
    if (_buttons.find(item) != _buttons.end()) {
        _buttons[item]->SelectEffect(xLightsApp::GetFrame()->GetMainSequencer());
    } else {
        xLightsApp::GetFrame()->GetMainSequencer()->UnselectAllEffects();
        xLightsApp::GetFrame()->UnselectEffect();
        xLightsApp::GetFrame()->GetOutputManager()->AllOff();

        // turn all the lights off in case we are outputting to lights
        wxCommandEvent playEvent(EVT_STOP_SEQUENCE);
        wxPostEvent(xLightsApp::GetFrame(), playEvent);
    }
}

void JukeboxPanel::OnResize(wxSizeEvent& event)
{
    wxSize wsz = event.GetSize();
    if (wsz.GetWidth() <= 10) {
        return;
    }

    if (wsz.GetWidth() < 10)
        return;

    double cols = wsz.GetWidth() * JUKEBOXBUTTONS;
    cols /= std::max(wsz.GetHeight(), 1);
    cols = std::sqrt(cols);
    int i = std::round(cols);

    GridSizer1->SetCols(i);
    GridSizer1->SetDimension(0, 0, wsz.GetWidth(), wsz.GetHeight());
    GridSizer1->Layout();
}

void JukeboxPanel::OnButtonClick(wxCommandEvent& event)
{
    int button = wxAtoi(((wxButton*)event.GetEventObject())->GetLabel());
    PlayItem(button);
}

void JukeboxPanel::OnButtonRClick(wxContextMenuEvent& event)
{
    int button = wxAtoi(((wxButton*)event.GetEventObject())->GetLabel());

    ButtonControl* control = nullptr;
    if (_buttons.find(button) != _buttons.end()) {
        control = _buttons[button];
    }

    LinkJukeboxButtonDialog dlg(this, button, control, xLightsApp::GetFrame()->GetMainSequencer());
    OptimiseDialogPosition(&dlg);

    if (dlg.ShowModal() == wxID_OK) {
        if (control != nullptr) {
            SetButtonTooltip(control->_number, "");
            delete control;
            control = nullptr;
            _buttons.erase(button);
        }

        if (dlg.RadioButton_ED->GetValue() && dlg.Choice_Description->GetSelection() >= 0) {
            control = new ButtonControl(button, dlg.Choice_Description->GetStringSelection().ToStdString(), dlg.TextCtrl_Tooltip->GetValue().ToStdString(), dlg.CheckBox_LoopEffect->IsChecked());
        } else if (dlg.Choice_Model->GetSelection() >= 0) {
            control = new ButtonControl(button, dlg.Choice_Model->GetStringSelection().ToStdString(), wxAtoi(dlg.Choice_Layer->GetStringSelection()), wxAtoi(dlg.Choice_Time->GetStringSelection()), dlg.TextCtrl_Tooltip->GetValue().ToStdString(), dlg.CheckBox_LoopEffect->IsChecked());
        }

        if (control != nullptr) {
            _buttons[button] = control;
            SetButtonTooltip(control->_number, control->_tooltip);
        }
        xLightsApp::GetFrame()->GetMainSequencer()->SetChanged();
        ValidateWindow();
    }
}

void JukeboxPanel::ValidateWindow()
{
    auto children = GetChildren();

    for (const auto& it : children) {
        int b = wxAtoi(it->GetLabel());

        if (b > 0) {
            if (_buttons.find(b) != _buttons.end()) {
                // button has a control
                it->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BTNFACE));
            } else {
                it->SetBackgroundColour(wxColour(255, 108, 108));
            }
        }
    }
}

wxString JukeboxPanel::GetTooltipsJSON() const
{
    wxString res = "[";
    for (int i = 0; i < JUKEBOXBUTTONS; i++) {
        if (i != 0)
            res += ",";
        if (_buttons.find(i + 1) != _buttons.end()) {
            res += "\"" + _buttons.at(i + 1)->GetTooltip() + "\"";
        } else {
            res += "\"\"";
        }
    }
    res += "]";
    return res;
}

wxString JukeboxPanel::GetEffectPresentJSON() const
{
    wxString res = "[";
    for (int i = 0; i < JUKEBOXBUTTONS; i++) {
        if (i != 0)
            res += ",";
        if (_buttons.find(i + 1) != _buttons.end()) {
            res += "1";
        } else {
            res += "0";
        }
    }
    res += "]";
    return res;
}

wxString JukeboxPanel::GetTooltips() const
{
    wxString res = "|";
    for (int i = 0; i < JUKEBOXBUTTONS; i++) {
        if (_buttons.find(i + 1) != _buttons.end()) {
            res += _buttons.at(i + 1)->GetTooltip() + "|";
        } else {
            res += "|";
        }
    }

    return res;
}

wxString JukeboxPanel::GetEffectPresent() const
{
    wxString res = "|";
    for (int i = 0; i < JUKEBOXBUTTONS; i++) {
        if (_buttons.find(i + 1) != _buttons.end()) {
            res += "TRUE|";
        } else {
            res += "|";
        }
    }

    return res;
}
