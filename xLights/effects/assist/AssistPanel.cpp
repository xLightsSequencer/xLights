/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/smeighan/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/smeighan/xLights/blob/master/License.txt
 **************************************************************/

//(*InternalHeaders(AssistPanel)
#include <wx/intl.h>
#include <wx/string.h>
//*)

#include "AssistPanel.h"
#include "SketchAssistPanel.h"
#include "../../xLightsMain.h"
#include "../../models/Model.h"
#include "../../sequencer/Element.h"

#include <log4cpp/Category.hh>

//(*IdInit(AssistPanel)
const long AssistPanel::ID_SCROLLEDWINDOW_Assist = wxNewId();
//*)

BEGIN_EVENT_TABLE(AssistPanel,wxPanel)
	//(*EventTable(AssistPanel)
	//*)
END_EVENT_TABLE()

AssistPanel::AssistPanel(wxWindow* parent,wxWindowID id,const wxPoint& pos,const wxSize& size)
: mGridCanvas(nullptr), mModel(nullptr), mEffect(nullptr)
{
	//(*Initialize(AssistPanel)
	Create(parent, id, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL|wxWANTS_CHARS, _T("id"));
	FlexGridSizer1 = new wxFlexGridSizer(1, 1, 0, 0);
	ScrolledWindowAssist = new wxScrolledWindow(this, ID_SCROLLEDWINDOW_Assist, wxDefaultPosition, wxDefaultSize, wxWANTS_CHARS|wxVSCROLL|wxHSCROLL, _T("ID_SCROLLEDWINDOW_Assist"));
	FlexGridSizer2 = new wxFlexGridSizer(2, 1, 0, 0);
	FlexGridSizer2->AddGrowableCol(0);
	FlexGridSizer2->AddGrowableRow(0);
	ScrolledWindowAssist->SetSizer(FlexGridSizer2);
	FlexGridSizer1->Add(ScrolledWindowAssist, 1, wxALL|wxEXPAND, 5);
	SetSizer(FlexGridSizer1);
	//*)
}

AssistPanel::~AssistPanel()
{
	//(*Destroy(AssistPanel)
	//*)
	if( mGridCanvas != nullptr )
    {
        delete mGridCanvas;
    }
}

void AssistPanel::AdjustSize(wxSize& s)
{
    SetSize(s);
    SetMinSize(s);
    SetMaxSize(s);

	if( mGridCanvas != nullptr )
    {
        mGridCanvas->AdjustSize(s);
        mGridCanvas->Refresh();
    }

    ScrolledWindowAssist->SetSize(s);
    ScrolledWindowAssist->SetMinSize(s);
    ScrolledWindowAssist->SetMaxSize(s);

    ScrolledWindowAssist->FitInside();
    ScrolledWindowAssist->SetScrollRate(5, 5);
    ScrolledWindowAssist->Refresh();

    Refresh();
}

void AssistPanel::SetGridCanvas(xlGridCanvas* canvas)
{
    mGridCanvas = canvas;
    FlexGridSizer2->Add(canvas, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
    FlexGridSizer2->Fit(ScrolledWindowAssist);
    FlexGridSizer2->SetSizeHints(ScrolledWindowAssist);
}

void AssistPanel::AddPanel(wxPanel* panel, std::optional<int> panelFlags /*=std::nullopt*/)
{
    int flags = panelFlags.value_or(wxALL | wxALIGN_CENTER_HORIZONTAL | wxALIGN_CENTER_VERTICAL);

    mPanel = panel;
    FlexGridSizer2->SetCols(2);
    FlexGridSizer2->Add(mPanel, 1, flags, 2);
    FlexGridSizer2->Fit(ScrolledWindowAssist);
    FlexGridSizer2->SetSizeHints(ScrolledWindowAssist);
    SetHandlers(this);
}

void AssistPanel::RefreshEffect() {
    if( mGridCanvas != nullptr && mModel != nullptr && mEffect != nullptr )
    {
        mGridCanvas->SetModel(mModel);
        
        int bw, bh;
        mModel->GetBufferSize(mEffect->GetSettings().Get("B_CHOICE_BufferStyle", "Default"),
                              mEffect->GetSettings().Get("B_CHOICE_PerPreviewCamera", "2D"),
                              mEffect->GetSettings().Get("B_CHOICE_BufferTransform", "None"),
                              bw, bh, mEffect->GetSettings().GetInt("B_SPINCTRL_BufferStagger", 0));
        
        wxSize sz = GetSize();
        mGridCanvas->SetNumColumns(bw);
        mGridCanvas->SetNumRows(bh);
        mGridCanvas->SetEffect(mEffect);
        mGridCanvas->AdjustSize(sz);
        mGridCanvas->Refresh();
    }
}

void AssistPanel::SetEffectInfo(Effect* effect_, xLightsFrame* xlights_parent)
{
    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));

    // removed the check on the canvas as not setting the effect may be leaving the data invalid
    mEffect = effect_;
    EffectLayer* layer = mEffect->GetParentEffectLayer();
    if (layer == nullptr) {
        logger_base.error("No layer found for effect %s", (const char *)mEffect->GetEffectName().c_str());
    }
    Element* elem = layer->GetParentElement();
    if (elem == nullptr) {
        logger_base.error("No element found for effect %s", (const char *)mEffect->GetEffectName().c_str());
    }
    std::string model_name = elem->GetModelName();
    mModel = xlights_parent->GetModel(model_name);
    if (mModel != nullptr && dynamic_cast<SubModelElement*>(elem) != nullptr) {
        Model *scls = mModel->GetSubModel(dynamic_cast<SubModelElement*>(elem)->GetName());
        if (scls != nullptr) {
            mModel = scls;
        }
    }
    if (mModel == nullptr) {
        logger_base.error("No model found for effect %s for model %s", (const char *)mEffect->GetEffectName().c_str(), (const char *)model_name.c_str());
    }
    RefreshEffect();
}

void AssistPanel::SetHandlers(wxWindow *window)
{
    if (window) {
        window->Connect(wxID_ANY,
                        wxEVT_CHAR,
                        wxKeyEventHandler(AssistPanel::OnChar),
                        (wxObject*) NULL,
                        this);
        window->Connect(wxID_ANY,
                        wxEVT_CHAR_HOOK,
                        wxKeyEventHandler(AssistPanel::OnCharHook),
                        (wxObject*) NULL,
                        this);
        window->Connect(wxID_ANY,
                        wxEVT_KEY_DOWN,
                        wxKeyEventHandler(AssistPanel::OnKeyDown),
                        (wxObject*) NULL,
                        this);

        wxWindowList &list = window->GetChildren();
        for (wxWindowList::iterator it = list.begin(); it != list.end(); ++it) {
            wxWindow* pclChild = *it;
            SetHandlers(pclChild);
        }
    }
}

void AssistPanel::OnKeyDown(wxKeyEvent& event)
{
    event.Skip();
}

void AssistPanel::OnChar(wxKeyEvent& event)
{
    event.Skip();
}

void AssistPanel::OnCharHook(wxKeyEvent& event)
{
    if (mEffect != nullptr)
    {
        if ( mEffect->GetEffectIndex() == EffectManager::eff_PICTURES )
        {
            wxChar uc = event.GetUnicodeKey();
            switch(uc)
            {
                case 'c':
                case 'C':
                case WXK_CONTROL_C:
                    if (event.CmdDown() || event.ControlDown() || event.RawControlDown()) {
                       	if (mGridCanvas != nullptr) {
                            mGridCanvas->Copy();
                        }
                        event.StopPropagation();
                    }
                    break;
                case 'v':
                case 'V':
                case WXK_CONTROL_V:
                    if (event.CmdDown() || event.ControlDown() || event.RawControlDown()) {
                       	if (mGridCanvas != nullptr) {
                            mGridCanvas->Paste();
                        }
                        event.StopPropagation();
                    }
                    break;
                case WXK_ESCAPE:
                    if (mGridCanvas != nullptr) {
                        mGridCanvas->Cancel();
                    }
                    event.StopPropagation();
                    break;
#ifdef __WXMSW__
				case WXK_INSERT:
				case WXK_NUMPAD_INSERT:
                    if (event.ControlDown()) { // Copy
                        if (mGridCanvas != nullptr) {
							mGridCanvas->Copy();
						}
						event.StopPropagation();
                    } else if (GetKeyState(VK_LSHIFT) || GetKeyState(VK_RSHIFT)) { // Paste
                        if (mGridCanvas != nullptr) {
							mGridCanvas->Paste();
						}
						event.StopPropagation();
					}
					break;
#endif
				default:
                    event.Skip();
                    break;
            }
        } else if (mEffect->GetEffectIndex() == EffectManager::eff_SKETCH) {
            auto sketchAssistPanel = dynamic_cast<SketchAssistPanel *>(mPanel);
            if (sketchAssistPanel != nullptr)
                sketchAssistPanel->ForwardKeyEvent(event);
        }
    }
}
