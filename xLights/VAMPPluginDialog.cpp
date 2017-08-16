#include "VAMPPluginDialog.h"

//(*InternalHeaders(VAMPPluginDialog)
#include <wx/button.h>
#include <wx/string.h>
#include <wx/intl.h>
//*)

#include <wx/wx.h>
#include <wx/checkbox.h>
#include <wx/choice.h>
#include <wx/slider.h>
#include <wx/progdlg.h>

#include "vamp-hostsdk/PluginLoader.h"

//(*IdInit(VAMPPluginDialog)
const long VAMPPluginDialog::ID_TEXTCTRL1 = wxNewId();
const long VAMPPluginDialog::ID_STATICTEXT1 = wxNewId();
const long VAMPPluginDialog::ID_STATICTEXT2 = wxNewId();
//*)

BEGIN_EVENT_TABLE(VAMPPluginDialog,wxDialog)
	//(*EventTable(VAMPPluginDialog)
	//*)
END_EVENT_TABLE()

using namespace Vamp;

VAMPPluginDialog::VAMPPluginDialog(wxWindow* parent,wxWindowID id,const wxPoint& pos,const wxSize& size)
{
	//(*Initialize(VAMPPluginDialog)
	wxFlexGridSizer* FlexGridSizer1;
	wxFlexGridSizer* FlexGridSizer2;
	wxStaticText* StaticText1;
	wxStdDialogButtonSizer* StdDialogButtonSizer1;
	wxStaticBoxSizer* StaticBoxSizer1;

	Create(parent, id, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE, _T("id"));
	SetClientSize(wxDefaultSize);
	Move(wxDefaultPosition);
	FlexGridSizer1 = new wxFlexGridSizer(0, 1, 0, 0);
	FlexGridSizer1->AddGrowableCol(0);
	FlexGridSizer2 = new wxFlexGridSizer(0, 2, 0, 0);
	FlexGridSizer2->AddGrowableCol(1);
	StaticText1 = new wxStaticText(this, wxID_ANY, _("Timing Name:"), wxDefaultPosition, wxDefaultSize, 0, _T("wxID_ANY"));
	FlexGridSizer2->Add(StaticText1, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	TimingName = new wxTextCtrl(this, ID_TEXTCTRL1, _("Timing"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_TEXTCTRL1"));
	FlexGridSizer2->Add(TimingName, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer1->Add(FlexGridSizer2, 1, wxALL|wxEXPAND, 5);
	Label1 = new wxStaticText(this, ID_STATICTEXT1, _("Label"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT1"));
	FlexGridSizer1->Add(Label1, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	Label2 = new wxStaticText(this, ID_STATICTEXT2, _("Label"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT2"));
	FlexGridSizer1->Add(Label2, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	StaticBoxSizer1 = new wxStaticBoxSizer(wxHORIZONTAL, this, _("Plugin Settings"));
	SettingsSizer = new wxFlexGridSizer(0, 2, 0, 0);
	SettingsSizer->AddGrowableCol(1);
	StaticBoxSizer1->Add(SettingsSizer, 1, wxALL|wxEXPAND, 0);
	FlexGridSizer1->Add(StaticBoxSizer1, 1, wxALL|wxEXPAND, 5);
	StdDialogButtonSizer1 = new wxStdDialogButtonSizer();
	StdDialogButtonSizer1->AddButton(new wxButton(this, wxID_OK, wxEmptyString));
	StdDialogButtonSizer1->AddButton(new wxButton(this, wxID_CANCEL, wxEmptyString));
	StdDialogButtonSizer1->Realize();
	FlexGridSizer1->Add(StdDialogButtonSizer1, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	SetSizer(FlexGridSizer1);
	FlexGridSizer1->Fit(this);
	FlexGridSizer1->SetSizeHints(this);
	//*)

}

VAMPPluginDialog::~VAMPPluginDialog()
{
	//(*Destroy(VAMPPluginDialog)
	//*)
}

class FloatSliderControl : public wxEvtHandler, public wxFlexGridSizer {
public:
    FloatSliderControl(wxWindow *parent, float val, float minv, float maxv, const wxString &tip) : wxFlexGridSizer(2,3,0,0) {
        AddGrowableCol(1);
        scale = 10.0;
        strcpy(format, wxString::Format("%%.%df",1).c_str());
        min = new wxStaticText(parent, wxID_ANY, wxString::Format(format, minv));
        Add(min, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 1);

        slider = new wxSlider(parent, wxID_ANY, scale * val, minv * scale, maxv * scale);
        slider->SetValue(scale * val);
        slider->SetToolTip(tip);
        Add(slider, 1, wxALL|wxALIGN_CENTER_VERTICAL|wxEXPAND, 1);

        max = new wxStaticText(parent, wxID_ANY, wxString::Format(format, maxv));
        Add(max, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 1);
        Add(10, 10);

        value = new wxStaticText(parent, wxID_ANY, wxString::Format(format, val));
        Add(value, 1, wxALL|wxALIGN_CENTER_VERTICAL|wxEXPAND, 1);

        slider->Connect(slider->GetId(),wxEVT_COMMAND_SLIDER_UPDATED,(wxObjectEventFunction)&FloatSliderControl::sliderValue, nullptr, this);
    }
    virtual ~FloatSliderControl() {}


    void sliderValue(wxScrollEvent& event) {
        float sv = event.GetPosition();
        wxString val = wxString::Format(format, sv / scale);
        value->SetLabel(val);
    }

    float GetValue() {
        return wxAtof(value->GetLabel());
    }

    float scale;
    char format[24];
    wxSlider *slider;
    wxStaticText *min;
    wxStaticText *max;
    wxStaticText *value;

};
void processFeatures( Vamp::Plugin::FeatureList &feature, std::vector<int> &starts, std::vector<int> &ends, std::vector<std::string> &labels) {
    bool hadDuration = true;
    for (int x = 0; x < feature.size(); x++) {
        int start = feature[x].timestamp.msec() + feature[x].timestamp.sec * 1000;
        starts.push_back(start);
        if (!hadDuration) {
            ends.push_back(start);
        }
        hadDuration = feature[x].hasDuration;
        if (hadDuration) {
            int end = start + feature[x].duration.msec() + feature[x].duration.sec * 1000;
            ends.push_back(end);
        }
        labels.push_back(feature[x].label);
    }
    if (!hadDuration) {
        ends.push_back(starts[starts.size() - 1]);
    }
}

wxString VAMPPluginDialog::ProcessPlugin(xLightsXmlFile* xml_file, xLightsFrame *xLightsParent, const wxString &name, AudioManager* media) 
{
    Vamp::Plugin *p = media->GetVamp()->GetPlugin(std::string(name.c_str()));
    Label1->SetLabel(p->getName());
    Label2->SetLabel(p->getDescription());

    int output = 0;
    Plugin::OutputList outputs = p->getOutputDescriptors();
    if (outputs.size() > 1) {
        for (int x = 0; x < outputs.size(); x++) {
            wxString pname = wxString::FromUTF8(p->getName().c_str());
            wxString outputName = wxString::FromUTF8(outputs[x].name.c_str());
            if (outputName != pname) {
                pname = wxString::Format(wxT("%s: %s"),
                                        pname.c_str(), outputName.c_str());
            }
            if (name == pname) {
                output = x;
                TimingName->SetValue(outputName);
            }
        }
    } else {
        TimingName->SetValue(p->getName());
    }

    PluginBase::ParameterList params = p->getParameterDescriptors();
    std::vector<void *> controls;
    for (int x = 0; x < params.size(); x++) {
        wxString tip = wxString::FromUTF8(params[x].description.c_str());
        wxString unit = wxString::FromUTF8(params[x].unit.c_str());

        float value = p->getParameter(params[x].identifier);
        wxString labelText = wxString::FromUTF8(params[x].name.c_str());
        if (!unit.IsEmpty()) {
            labelText += wxT(" (") + unit + wxT(")");
        }
        wxStaticText *desc = new wxStaticText(this, wxID_ANY, labelText + wxT(":"));
        SettingsSizer->Add(desc, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);

        if (params[x].isQuantized &&
            params[x].quantizeStep == 1.0 &&
            params[x].minValue == 0.0 &&
            params[x].maxValue == 1.0)
        {
            wxCheckBox *cb = new wxCheckBox(this, wxID_ANY, _(""));
            cb->SetValue(value > 0.5);

            if (!tip.IsEmpty())
            {
                cb->SetToolTip(tip);
            }
            SettingsSizer->Add(cb, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
            controls.push_back(cb);
        }
        else if (params[x].isQuantized &&
                 params[x].quantizeStep == 1.0 &&
                 !params[x].valueNames.empty())
        {

            wxChoice *choice = new wxChoice(this, wxID_ANY);

            for (size_t i = 0, cnt = params[x].valueNames.size(); i < cnt; i++)
            {
                wxString choicetxt = wxString::FromUTF8(params[x].valueNames[i].c_str());
                choice->Append(choicetxt);
                if (size_t(value - params[x].minValue + 0.5) == i) {
                    choice->SetSelection(i);
                }
            }
            choice->SetSizeHints(-1, -1);
            if (!tip.IsEmpty())
            {
                choice->SetToolTip(tip);
            }
            SettingsSizer->Add(choice, 1, wxALL|wxALIGN_CENTER_VERTICAL|wxEXPAND, 5);
            controls.push_back(choice);
        }
        else
        {
            FloatSliderControl *slider = new FloatSliderControl(this, value, params[x].minValue, params[x].maxValue, tip);
            SettingsSizer->Add(slider,
                               1, wxALL|wxALIGN_CENTER_VERTICAL|wxEXPAND, 5);
            controls.push_back(slider);
        }

    }
    Fit();
    int res = ShowModal();
    if (res == wxID_OK) {
        while (xml_file->TimingAlreadyExists(TimingName->GetValue().ToStdString(), xLightsParent)) {
            wxMessageBox("Timing track " + TimingName->GetValue() + " already exists");
            res = ShowModal();
            if (res != wxID_OK) {
                return "";
            }
        }

        std::vector<int> starts;
        std::vector<int> ends;
        std::vector<std::string> labels;

        size_t step = p->getPreferredStepSize();
        size_t block = p->getPreferredBlockSize();
        if (block == 0) {
            if (step != 0) {
                block = step;
            }  else {
                block = 1024;
            }
        }
        if (step == 0) {
            step = block;
        }
        float *pdata[2];
        std::string error;
		media->SetStepBlock(step, block);

        for (int x = 0; x < params.size(); x++) {

            if (params[x].isQuantized &&
                params[x].quantizeStep == 1.0 &&
                params[x].minValue == 0.0 &&
                params[x].maxValue == 1.0)
            {
                wxCheckBox *cb = (wxCheckBox*)controls[x];
                p->setParameter(params[x].identifier, cb->GetValue());
            }
            else if (params[x].isQuantized &&
                     params[x].quantizeStep == 1.0 &&
                     !params[x].valueNames.empty())
            {
                wxChoice *choice = (wxChoice*)controls[x];
                p->setParameter(params[x].identifier, choice->GetSelection());
            }
            else
            {
                FloatSliderControl *slider = (FloatSliderControl*)controls[x];
                p->setParameter(params[x].identifier, slider->GetValue());
            }
        }
        int channels = media->GetChannels();
        if (channels > p->getMaxChannelCount()) {
            channels = 1;
        }
        p->initialise(channels, step, block);
        pdata[0] =media->GetLeftDataPtr(0);
        pdata[1] = media->GetRightDataPtr(0);
        
        wxProgressDialog progress("Processing Audio", "");
        long totalLen = media->GetTrackSize();
        long len = media->GetTrackSize();
		int percent = 0;
        long start = 0;
        while (len) {
            //int request = block;
            //if (request > len) request = len;

			pdata[0] = media->GetLeftDataPtr(start);
			pdata[1] = media->GetRightDataPtr(start);

            Vamp::RealTime timestamp = Vamp::RealTime::frame2RealTime(start, media->GetRate());
            Vamp::Plugin::FeatureSet features = p->process(pdata, timestamp);
            processFeatures(features[output], starts, ends, labels);

            if (len > (long)step) {
                len -= step;
            } else {
                len = 0;
            }
            start += step;
            
            int newp = (int)((start * 100) / totalLen);
            if (newp != percent) {
                percent = newp;
                progress.Update(percent);
            }
        }
        Vamp::Plugin::FeatureSet features = p->getRemainingFeatures();
        processFeatures(features[output], starts, ends, labels);
        progress.Update(100);

        xml_file->AddNewTimingSection(TimingName->GetValue().ToStdString(), xLightsParent, starts, ends, labels);
        return TimingName->GetValue();
    }
    return "";
}

