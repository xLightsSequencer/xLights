#include "VAMPPluginDialog.h"

//(*InternalHeaders(VAMPPluginDialog)
#include <wx/intl.h>
#include <wx/button.h>
#include <wx/string.h>
//*)

#include <wx/wx.h>
#include <wx/checkbox.h>
#include <wx/choice.h>
#include <wx/slider.h>
#include <wx/progdlg.h>

#include "mpg123.h"


//(*IdInit(VAMPPluginDialog)
const long VAMPPluginDialog::ID_TEXTCTRL1 = wxNewId();
const long VAMPPluginDialog::ID_STATICTEXT1 = wxNewId();
const long VAMPPluginDialog::ID_STATICTEXT2 = wxNewId();
//*)

BEGIN_EVENT_TABLE(VAMPPluginDialog,wxDialog)
	//(*EventTable(VAMPPluginDialog)
	//*)
END_EVENT_TABLE()

#ifdef HASVAMP
using namespace Vamp;
#endif

VAMPPluginDialog::VAMPPluginDialog(wxWindow* parent,wxWindowID id,const wxPoint& pos,const wxSize& size)
{
	//(*Initialize(VAMPPluginDialog)
	wxFlexGridSizer* FlexGridSizer2;
	wxStaticText* StaticText1;
	wxStaticBoxSizer* StaticBoxSizer1;
	wxFlexGridSizer* FlexGridSizer1;
	wxStdDialogButtonSizer* StdDialogButtonSizer1;

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
	FlexGridSizer1->Add(FlexGridSizer2, 1, wxALL|wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	Label1 = new wxStaticText(this, ID_STATICTEXT1, _("Label"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT1"));
	FlexGridSizer1->Add(Label1, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	Label2 = new wxStaticText(this, ID_STATICTEXT2, _("Label"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT2"));
	FlexGridSizer1->Add(Label2, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	StaticBoxSizer1 = new wxStaticBoxSizer(wxHORIZONTAL, this, _("Plugin Settings"));
	SettingsSizer = new wxFlexGridSizer(0, 2, 0, 0);
	SettingsSizer->AddGrowableCol(1);
	StaticBoxSizer1->Add(SettingsSizer, 1, wxALL|wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 0);
	FlexGridSizer1->Add(StaticBoxSizer1, 1, wxALL|wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
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

#ifdef HASVAMP
    for (int x = 0; x < loadedPlugins.size(); x++) {
        delete loadedPlugins[x];
    }
#endif
}



void cleanup(mpg123_handle *mh)
{
    /* It's really to late for error checks here;-) */
    mpg123_close(mh);
    mpg123_delete(mh);
    mpg123_exit();
}
int GetTrackSize(mpg123_handle *mh,int bits, int channels)
{
    size_t buffer_size;
    unsigned char *buffer;
    size_t done;
    int trackSize=0;
    int fileSize=0;

    if(mpg123_length(mh) > 0)
    {
        return mpg123_length(mh);
    }

    buffer_size = mpg123_outblock(mh);
    buffer = (unsigned char*) malloc(buffer_size * sizeof(unsigned char));

    mpg123_seek(mh,0,SEEK_SET);
    for (fileSize = 0 ; mpg123_read(mh, buffer, buffer_size, &done) == MPG123_OK ; )
    {
        fileSize += done;
    }

    free(buffer);
    trackSize = fileSize/(bits*channels);
    return trackSize;
}
void SplitTrackDataAndNormalize(signed short* trackData,int trackSize, float* leftData, float* rightData)
{
    signed short lSample,rSample;
    for(int i=0;i<trackSize;i++)
    {
        lSample = trackData[i*2];
        leftData[i] = (float)lSample/(float)32768;
        rSample = trackData[(i*2)+1];
        rightData[i] = (float)rSample/(float)32768;
    }
}

void NormalizeMonoTrackData(signed short* trackData,int trackSize,float* leftData)
{
    signed short lSample;
    for(int i=0;i<trackSize;i++)
    {
        lSample = trackData[i];
        leftData[i] = (float)lSample/(float)32768;
    }
}

void LoadTrackData(mpg123_handle *mh,char  * data, int maxSize)
{
    size_t buffer_size;
    unsigned char *buffer;
    size_t done;
    int bytesRead=0;
    buffer_size = mpg123_outblock(mh);
    buffer = (unsigned char*) malloc(buffer_size * sizeof(unsigned char));
    mpg123_seek(mh,0,SEEK_SET);
    for (bytesRead = 0 ; mpg123_read(mh, buffer, buffer_size, &done) == MPG123_OK ; )
    {
        if ((bytesRead + done) >= maxSize) {
            wxMessageBox("Error reading data from mp3, too much data read.");
            free(buffer);
            return;
        }
        memcpy(data+bytesRead,buffer,done);
        bytesRead+=done;
    }
    free(buffer);
}
int OpenMediaFile(const char* filename, wxString& error, float *data[2], long &rate)
{
    mpg123_handle *mh = NULL;
    int err;
    size_t buffer_size;
    int channels, encoding;

    err = mpg123_init();
    if(err != MPG123_OK || (mh = mpg123_new(NULL, &err)) == NULL)
    {
        error = wxString::Format("Basic setup goes wrong: %s", mpg123_plain_strerror(err));
        if (mh != NULL) {
            cleanup(mh);
        }
        return -1;
    }

    /* open the file and get the decoding format */
    if( mpg123_open(mh, filename) != MPG123_OK ||
       mpg123_getformat(mh, &rate, &channels, &encoding) != MPG123_OK )
    {
        error = wxString::Format("Trouble with mpg123: %s", mpg123_strerror(mh));
        cleanup(mh);
        return -1;
    }

    if( encoding != MPG123_ENC_SIGNED_16 )
    {
        error = "Encoding unsupported.  Must be signed 16 bit.";
        cleanup(mh);
        return -2;
    }

    /* set the output format and open the output device */
    int m_bits = mpg123_encsize(encoding);
    int m_channels = channels;
    /* Get Track Size */
    int mMediaTrackSize = GetTrackSize(mh,m_bits,m_channels);
    buffer_size = mpg123_outblock(mh);
    int size = (mMediaTrackSize+buffer_size)*m_bits*m_channels;
    char * trackData = (char*)malloc(size);
    LoadTrackData(mh,trackData, size);
    // Split data into left and right and normalize -1 to 1
    data[0] = (float*)calloc(sizeof(float)*mMediaTrackSize + 1024, 1);
    if( m_channels == 2 )
    {
        data[1] = (float*)calloc(sizeof(float)*mMediaTrackSize + 1024, 1);
        SplitTrackDataAndNormalize((signed short*)trackData,mMediaTrackSize,data[0],data[1]);
    }
    else if( m_channels == 1 )
    {
        NormalizeMonoTrackData((signed short*)trackData,mMediaTrackSize,data[0]);
        data[1] = data[0];
    }
    else
    {
        error = "More than 2 audio channels is not supported yet.";
    }

    mpg123_close(mh);
    mpg123_delete(mh);
    mpg123_exit();
    free(trackData);
    return mMediaTrackSize;
}



wxArrayString VAMPPluginDialog::GetAvailablePlugins(const wxString &media) {
    wxArrayString ret;
#ifdef HASVAMP
    mpg123_handle *mh = NULL;
    int err;
    int channels, encoding;

    long rate = 44100;
    err = mpg123_init();
    if(err != MPG123_OK || (mh = mpg123_new(NULL, &err)) == NULL)
    {
        if (mh != NULL) {
            cleanup(mh);
        }
    }

    /* open the file and get the decoding format */
    if( mpg123_open(mh, media.c_str()) != MPG123_OK ||
       mpg123_getformat(mh, &rate, &channels, &encoding) != MPG123_OK )
    {
        cleanup(mh);
    }
    cleanup(mh);

    Vamp::HostExt::PluginLoader *loader = Vamp::HostExt::PluginLoader::getInstance();
    Vamp::HostExt::PluginLoader::PluginKeyList pluginList = loader->listPlugins();
    for (int x = 0; x < pluginList.size(); x++) {
        Vamp::Plugin *p = loader->loadPlugin(pluginList[x], rate);
        if (p == nullptr) {
            continue;
        }
        loadedPlugins.push_back(p);
        Plugin::OutputList outputs = p->getOutputDescriptors();

        for (Plugin::OutputList::iterator j = outputs.begin(); j != outputs.end(); ++j) {
            if (j->sampleType == Plugin::OutputDescriptor::FixedSampleRate ||
                j->sampleType == Plugin::OutputDescriptor::OneSamplePerStep ||
                !j->hasFixedBinCount ||
                (j->hasFixedBinCount && j->binCount > 1)) {

                continue;
            }

            wxString name = wxString::FromUTF8(p->getName().c_str());

            if (outputs.size() > 1) {
                // This is not the plugin's only output.
                // Use "plugin name: output name" as the effect name,
                // unless the output name is the same as the plugin name
                wxString outputName = wxString::FromUTF8(j->name.c_str());
                if (outputName != name) {
                    name = wxString::Format(wxT("%s: %s"),
                                            name.c_str(), outputName.c_str());
                }
            }

            plugins[name] = p;
        }
    }

    for (std::map<wxString, Vamp::Plugin *>::iterator it = plugins.begin(); it != plugins.end(); ++it) {
        ret.push_back(it->first);
    }
#endif
    return ret;
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
        Add(slider, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxEXPAND, 1);

        max = new wxStaticText(parent, wxID_ANY, wxString::Format(format, maxv));
        Add(max, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 1);
        Add(10, 10);

        value = new wxStaticText(parent, wxID_ANY, wxString::Format(format, val));
        Add(value, 1, wxALL|wxALIGN_CENTER|wxALIGN_CENTER_VERTICAL|wxEXPAND, 1);

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
#ifdef HASVAMP
void processFeatures( Vamp::Plugin::FeatureList &feature, std::vector<int> &starts, std::vector<int> &ends, wxArrayString &labels) {
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
#endif

wxString VAMPPluginDialog::ProcessPlugin(xLightsXmlFile* xml_file, xLightsFrame *xLightsParent, const wxString &name, const wxString &media) {
#ifdef HASVAMP
    Plugin *p = plugins[name];
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
            }
        }
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
            SettingsSizer->Add(choice, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxEXPAND, 5);
            controls.push_back(choice);
        }
        else
        {
            FloatSliderControl *slider = new FloatSliderControl(this, value, params[x].minValue, params[x].maxValue, tip);
            SettingsSizer->Add(slider,
                               1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL|wxEXPAND, 5);
            controls.push_back(slider);
        }

    }
    Fit();
    int res = ShowModal();
    if (res == wxID_OK) {
        while (xml_file->TimingAlreadyExists(TimingName->GetValue(), xLightsParent)) {
            wxMessageBox("Timing track " + TimingName->GetValue() + " already exists");
            res = ShowModal();
            if (res != wxID_OK) {
                return "";
            }
        }
        std::vector<int> starts;
        std::vector<int> ends;
        wxArrayString labels;

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
        float *data[2];
        float *pdata[2];
        wxString error;
        long rate;
        int len = OpenMediaFile(media, error, data, rate);

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
        int channels = 2;
        if (channels > p->getMaxChannelCount()) {
            channels = 1;
        }
        p->initialise(channels, step, block);
        pdata[0] = data[0];
        pdata[1] = data[1];
        
        wxProgressDialog progress("Processing Audio", "");
        int totalLen = len;
        int percent = 0;
        int start = 0;
        while (len) {
            int request = block;
            if (request > len) request = len;

            pdata[0] = &data[0][start];
            pdata[1] = &data[1][start];

            Vamp::RealTime timestamp = Vamp::RealTime::frame2RealTime(start, rate);
            Vamp::Plugin::FeatureSet features = p->process(pdata, timestamp);
            processFeatures(features[output], starts, ends, labels);

            if (len > (int)step) {
                len -= step;
            } else {
                len = 0;
            }
            start += step;
            
            int newp = (start * 100) / totalLen;
            if (newp != percent) {
                percent = newp;
                progress.Update(percent);
            }
        }
        Vamp::Plugin::FeatureSet features = p->getRemainingFeatures();
        processFeatures(features[output], starts, ends, labels);
        progress.Update(100);

        xml_file->AddNewTimingSection(TimingName->GetValue(), xLightsParent, starts, ends, labels);
        free(data[0]);
        if (data[1] != data[0]) {
            free(data[1]);
        }
    }
    return TimingName->GetValue();
#endif
}

