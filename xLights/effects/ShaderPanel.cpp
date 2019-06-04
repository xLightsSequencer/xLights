#include "ShaderPanel.h"
#include "ShaderEffect.h"
#include "../BulkEditControls.h"
#include "EffectPanelUtils.h"

//(*InternalHeaders(ShaderPanel)
#include <wx/intl.h>
#include <wx/string.h>
//*)
#include <wx/artprov.h>

//(*IdInit(ShaderPanel)
const long ShaderPanel::ID_STATICTEXT1 = wxNewId();
const long ShaderPanel::ID_0FILEPICKERCTRL_IFS = wxNewId();
//*)

ShaderPreview::ShaderPreview( wxWindow* parent, wxWindowID id, const wxPoint &pos, const wxSize &size, long style, const wxString &name, bool coreProfile)
   : xlGLCanvas( parent, id, pos, size, style, name, coreProfile )
{

}

ShaderPreview::~ShaderPreview()
{

}

void ShaderPreview::InitializeGLContext()
{
   // should just be doing init stuff?
   SetCurrentGLContext();
}

const long ShaderPanel::ID_CANVAS = wxNewId();

BEGIN_EVENT_TABLE(ShaderPanel,wxPanel)
	//(*EventTable(ShaderPanel)
	//*)
END_EVENT_TABLE()

ShaderPanel::ShaderPanel(wxWindow* parent,wxWindowID id,const wxPoint& pos,const wxSize& size)
{
    // I have deliberately given the file picker a ID_- prefix to force it to be processed first

	//(*Initialize(ShaderPanel)
	wxFlexGridSizer* FlexGridSizer1;
	wxFlexGridSizer* FlexGridSizer2;

	Create(parent, id, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("id"));
	FlexGridSizer1 = new wxFlexGridSizer(0, 1, 0, 0);
	FlexGridSizer1->AddGrowableCol(0);
	FlexGridSizer2 = new wxFlexGridSizer(0, 2, 0, 0);
	FlexGridSizer2->AddGrowableCol(1);
	StaticText1 = new wxStaticText(this, ID_STATICTEXT1, _("Shader File:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT1"));
	FlexGridSizer2->Add(StaticText1, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	FilePickerCtrl1 = new wxFilePickerCtrl(this, ID_0FILEPICKERCTRL_IFS, wxEmptyString, wxEmptyString, _T("*.fs"), wxDefaultPosition, wxDefaultSize, wxFLP_FILE_MUST_EXIST|wxFLP_OPEN|wxFLP_USE_TEXTCTRL, wxDefaultValidator, _T("ID_0FILEPICKERCTRL_IFS"));
	FlexGridSizer2->Add(FilePickerCtrl1, 1, wxALL|wxEXPAND, 5);
	FlexGridSizer1->Add(FlexGridSizer2, 1, wxALL|wxEXPAND, 5);
	FlexGridSizer_Dynamic = new wxFlexGridSizer(0, 3, 0, 0);
	FlexGridSizer_Dynamic->AddGrowableCol(1);
	FlexGridSizer1->Add(FlexGridSizer_Dynamic, 1, wxALL|wxEXPAND, 5);
	SetSizer(FlexGridSizer1);
	FlexGridSizer1->Fit(this);
	FlexGridSizer1->SetSizeHints(this);

	Connect(ID_0FILEPICKERCTRL_IFS,wxEVT_COMMAND_FILEPICKER_CHANGED,(wxObjectEventFunction)&ShaderPanel::OnFilePickerCtrl1FileChanged);
	//*)
    Connect(wxID_ANY, EVT_VC_CHANGED, (wxObjectEventFunction)& ShaderPanel::OnVCChanged, 0, this);

	_preview = new ShaderPreview( this, ID_CANVAS );
}

ShaderPanel::~ShaderPanel()
{
    if (_shaderConfig != nullptr) delete _shaderConfig;
	//(*Destroy(ShaderPanel)
	//*)
}

PANEL_EVENT_HANDLERS(ShaderPanel)

void ShaderPanel::OnFilePickerCtrl1FileChanged(wxFileDirPickerEvent& event)
{
    if (wxFile::Exists(FilePickerCtrl1->GetFileName().GetFullPath()))
    {
        BuildUI(FilePickerCtrl1->GetFileName().GetFullPath());
    }
    else
    {
        Freeze();
        FlexGridSizer_Dynamic->DeleteWindows();
        Thaw();
    }
}

void ShaderPanel::BuildUI(const wxString& filename)
{
    if (_shaderConfig != nullptr && _shaderConfig->GetFilename() == filename) return;

    Freeze();

    FlexGridSizer_Dynamic->DeleteWindows();

    _shaderConfig = ShaderEffect::ParseShader(filename);

    if (_shaderConfig != nullptr)
    {
        int id = 1;
        for (auto it : _shaderConfig->GetParms())
        {
            if (it.ShowParm())
            {
                if (it._type == ShaderParmType::SHADER_PARM_FLOAT)
                {
                    auto staticText = new wxStaticText(this, wxNewId(), it.GetLabel(), wxDefaultPosition, wxDefaultSize, 0, it.GetId(ShaderCtrlType::SHADER_CTRL_STATIC));
                    FlexGridSizer_Dynamic->Add(staticText, 1, wxALL | wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL, 2);

                    auto sizer = new wxFlexGridSizer(0, 2, 0, 0);
                    sizer->AddGrowableCol(0);

                    auto slider = new BulkEditSliderF2(this, wxNewId(), it._default * 100, it._min * 100, it._max * 100, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, it.GetId(ShaderCtrlType::SHADER_CTRL_SLIDER));
                    sizer->Add(slider, 1, wxALL | wxEXPAND, 2);
                    auto id = wxNewId();
                    auto vcb = new BulkEditValueCurveButton(this, id, wxArtProvider::GetBitmap(wxART_MAKE_ART_ID_FROM_STR(_T("xlART_valuecurve_notselected")), wxART_BUTTON), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW | wxNO_BORDER, wxDefaultValidator, it.GetId(ShaderCtrlType::SHADER_CTRL_VALUECURVE));
                    sizer->Add(vcb, 1, wxALL | wxALIGN_CENTER_HORIZONTAL | wxALIGN_CENTER_VERTICAL, 2);
                    vcb->GetValue()->SetLimits(it._min*100, it._max*100);
                    vcb->GetValue()->SetDivisor(100);
                    Connect(id, wxEVT_COMMAND_BUTTON_CLICKED, (wxObjectEventFunction)& ShaderPanel::OnVCButtonClick);

                    FlexGridSizer_Dynamic->Add(sizer, 1, wxALL | wxEXPAND, 0);

                    auto def = wxString::Format("%.2f", it._default);
                    auto text = new BulkEditTextCtrlF2(this, wxNewId(), def, wxDefaultPosition, wxDLG_UNIT(this, wxSize(30, -1)), 0, wxDefaultValidator, it.GetId(ShaderCtrlType::SHADER_CTRL_TEXTCTRL));
                    FlexGridSizer_Dynamic->Add(text, 1, wxALL | wxALIGN_CENTER_HORIZONTAL | wxALIGN_CENTER_VERTICAL, 2);
                }
                else if (it._type == ShaderParmType::SHADER_PARM_LONG)
                {
                    auto staticText = new wxStaticText(this, wxNewId(), it.GetLabel(), wxDefaultPosition, wxDefaultSize, 0, it.GetId(ShaderCtrlType::SHADER_CTRL_STATIC));
                    FlexGridSizer_Dynamic->Add(staticText, 1, wxALL | wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL, 2);

                    auto sizer = new wxFlexGridSizer(0, 2, 0, 0);
                    sizer->AddGrowableCol(0);

                    auto slider = new BulkEditSlider(this, wxNewId(), it._default, it._min, it._max, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, it.GetId(ShaderCtrlType::SHADER_CTRL_SLIDER));
                    sizer->Add(slider, 1, wxALL | wxEXPAND, 2);
                    auto id = wxNewId();
                    auto vcb = new BulkEditValueCurveButton(this, id, wxArtProvider::GetBitmap(wxART_MAKE_ART_ID_FROM_STR(_T("xlART_valuecurve_notselected")), wxART_BUTTON), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW | wxNO_BORDER, wxDefaultValidator, it.GetId(ShaderCtrlType::SHADER_CTRL_VALUECURVE));
                    sizer->Add(vcb, 1, wxALL | wxALIGN_CENTER_HORIZONTAL | wxALIGN_CENTER_VERTICAL, 2);
                    vcb->GetValue()->SetLimits(it._min, it._max);
                    Connect(id, wxEVT_COMMAND_BUTTON_CLICKED, (wxObjectEventFunction)& ShaderPanel::OnVCButtonClick);

                    FlexGridSizer_Dynamic->Add(sizer, 1, wxALL | wxEXPAND, 0);

                    auto def = wxString::Format("%l", (long)it._default);
                    auto text = new BulkEditTextCtrl(this, wxNewId(), def, wxDefaultPosition, wxDLG_UNIT(this, wxSize(30, -1)), 0, wxDefaultValidator, it.GetId(ShaderCtrlType::SHADER_CTRL_TEXTCTRL));
                    FlexGridSizer_Dynamic->Add(text, 1, wxALL | wxALIGN_CENTER_HORIZONTAL | wxALIGN_CENTER_VERTICAL, 2);
                }
                else if (it._type == ShaderParmType::SHADER_PARM_LONGCHOICE)
                {
                    auto staticText = new wxStaticText(this, wxNewId(), it.GetLabel(), wxDefaultPosition, wxDefaultSize, 0, it.GetId(ShaderCtrlType::SHADER_CTRL_STATIC));
                    FlexGridSizer_Dynamic->Add(staticText, 1, wxALL | wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL, 2);

                    auto choice = new BulkEditChoice(this, wxNewId(), wxDefaultPosition, wxDefaultSize, 0, 0, 0, wxDefaultValidator, it.GetId(ShaderCtrlType::SHADER_CTRL_CHOICE));
                    for (auto it2 : it.GetChoices())
                    {
                        choice->AppendString(it2);
                    }
                    choice->SetSelection(it._default);
                    FlexGridSizer_Dynamic->Add(choice, 1, wxTOP | wxBOTTOM | wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL, 5);
                    FlexGridSizer_Dynamic->Add(-1, -1, 1, wxALL | wxALIGN_CENTER_HORIZONTAL | wxALIGN_CENTER_VERTICAL, 2);
                }
                else if (it._type == ShaderParmType::SHADER_PARM_BOOL)
                {
                    auto staticText = new wxStaticText(this, wxNewId(), it.GetLabel(), wxDefaultPosition, wxDefaultSize, 0, it.GetId(ShaderCtrlType::SHADER_CTRL_STATIC));
                    FlexGridSizer_Dynamic->Add(staticText, 1, wxALL | wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL, 2);
                    auto checkbox = new BulkEditCheckBox(this, wxNewId(), _(""), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, it.GetId(ShaderCtrlType::SHADER_CTRL_CHECKBOX));
                    checkbox->SetValue(it._default == 1);
                    FlexGridSizer_Dynamic->Add(checkbox, 1, wxALL | wxEXPAND, 2);
                    FlexGridSizer_Dynamic->Add(-1, -1, 1, wxALL | wxALIGN_CENTER_HORIZONTAL | wxALIGN_CENTER_VERTICAL, 2);
                }
                else if (it._type == ShaderParmType::SHADER_PARM_POINT2D)
                {
                    auto staticText = new wxStaticText(this, wxNewId(), it.GetLabel() + " X", wxDefaultPosition, wxDefaultSize, 0, it.GetId(ShaderCtrlType::SHADER_CTRL_STATIC) +"X");
                    FlexGridSizer_Dynamic->Add(staticText, 1, wxALL | wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL, 2);

                    auto sizer = new wxFlexGridSizer(0, 2, 0, 0);
                    sizer->AddGrowableCol(0);

                    auto slider = new BulkEditSliderF2(this, wxNewId(), it._defaultPt.x*100, it._minPt.x*100, it._maxPt.x*100, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, it.GetId(ShaderCtrlType::SHADER_CTRL_SLIDER)+"X");
                    sizer->Add(slider, 1, wxALL | wxEXPAND, 2);
                    auto id = wxNewId();
                    auto vcb = new BulkEditValueCurveButton(this, id, wxArtProvider::GetBitmap(wxART_MAKE_ART_ID_FROM_STR(_T("xlART_valuecurve_notselected")), wxART_BUTTON), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW | wxNO_BORDER, wxDefaultValidator, it.GetId(ShaderCtrlType::SHADER_CTRL_VALUECURVE)+"X");
                    sizer->Add(vcb, 1, wxALL | wxALIGN_CENTER_HORIZONTAL | wxALIGN_CENTER_VERTICAL, 2);
                    vcb->GetValue()->SetLimits(it._minPt.x*100, it._maxPt.x*100);
                    vcb->GetValue()->SetDivisor(100);
                    Connect(id, wxEVT_COMMAND_BUTTON_CLICKED, (wxObjectEventFunction)& ShaderPanel::OnVCButtonClick);

                    FlexGridSizer_Dynamic->Add(sizer, 1, wxALL | wxEXPAND, 0);

                    auto def = wxString::Format("%.2f", it._defaultPt.x);
                    auto text = new BulkEditTextCtrlF2(this, wxNewId(), def, wxDefaultPosition, wxDLG_UNIT(this, wxSize(30, -1)), 0, wxDefaultValidator, it.GetId(ShaderCtrlType::SHADER_CTRL_TEXTCTRL)+"X");
                    FlexGridSizer_Dynamic->Add(text, 1, wxALL | wxALIGN_CENTER_HORIZONTAL | wxALIGN_CENTER_VERTICAL, 2);

                    staticText = new wxStaticText(this, wxNewId(), it.GetLabel() + " Y", wxDefaultPosition, wxDefaultSize, 0, it.GetId(ShaderCtrlType::SHADER_CTRL_STATIC) + "Y");
                    FlexGridSizer_Dynamic->Add(staticText, 1, wxALL | wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL, 2);

                    sizer = new wxFlexGridSizer(0, 2, 0, 0);
                    sizer->AddGrowableCol(0);

                    slider = new BulkEditSliderF2(this, wxNewId(), it._defaultPt.y * 100, it._minPt.y * 100, it._maxPt.y * 100, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, it.GetId(ShaderCtrlType::SHADER_CTRL_SLIDER) + "Y");
                    sizer->Add(slider, 1, wxALL | wxEXPAND, 2);
                    id = wxNewId();
                    vcb = new BulkEditValueCurveButton(this, id, wxArtProvider::GetBitmap(wxART_MAKE_ART_ID_FROM_STR(_T("xlART_valuecurve_notselected")), wxART_BUTTON), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW | wxNO_BORDER, wxDefaultValidator, it.GetId(ShaderCtrlType::SHADER_CTRL_VALUECURVE) + "Y");
                    sizer->Add(vcb, 1, wxALL | wxALIGN_CENTER_HORIZONTAL | wxALIGN_CENTER_VERTICAL, 2);
                    vcb->GetValue()->SetLimits(it._minPt.y * 100, it._maxPt.y * 100);
                    vcb->GetValue()->SetDivisor(100);
                    Connect(id, wxEVT_COMMAND_BUTTON_CLICKED, (wxObjectEventFunction)& ShaderPanel::OnVCButtonClick);

                    FlexGridSizer_Dynamic->Add(sizer, 1, wxALL | wxEXPAND, 0);

                    def = wxString::Format("%.2f", it._defaultPt.y);
                    text = new BulkEditTextCtrlF2(this, wxNewId(), def, wxDefaultPosition, wxDLG_UNIT(this, wxSize(30, -1)), 0, wxDefaultValidator, it.GetId(ShaderCtrlType::SHADER_CTRL_TEXTCTRL) + "Y");
                    FlexGridSizer_Dynamic->Add(text, 1, wxALL | wxALIGN_CENTER_HORIZONTAL | wxALIGN_CENTER_VERTICAL, 2);
                }
            }
        }
        FlexGridSizer_Dynamic->Layout();
        Layout();
    }

    Thaw();
}