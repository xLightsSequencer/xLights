#include "SketchPanel.h"

BEGIN_EVENT_TABLE(SketchPanel, wxPanel)
END_EVENT_TABLE()

SketchPanel::SketchPanel(wxWindow* parent, wxWindowID id/*=wxID_ANY*/, const wxPoint& pos/*= wxDefaultPositi=on*/, const wxSize& size/*= wxDefaultSize*/)
   : xlEffectPanel(parent)
{
    Create(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("wxID_ANY"));
}

SketchPanel::~SketchPanel()
{

}


void SketchPanel::ValidateWindow()
{

}
