#include "xlColourData.h"
#include <wx/string.h>

#include <wx/colordlg.h>
#include <wx/window.h>
#include <wx/config.h>

xlColourData xlColourData::INSTANCE;

xlColourData::xlColourData()
{ }

xlColourData::~xlColourData()
{ }

void xlColourData::Load(wxConfigBase* config)
{
    if (config != nullptr) {
        for (int i = 0;i < m_colorData.NUM_CUSTOM; ++i) {
            wxString color;
            config->Read(wxString::Format("CustomColour%d", i), &color, "");
            if (!color.IsEmpty()) {
                m_colorData.SetCustomColour(i, wxColour(color));
            }
        }
    }
}
void xlColourData::Save(wxConfigBase* config)
{
    if (config != nullptr) {
        for (int i = 0;i < m_colorData.NUM_CUSTOM; ++i) {
            config->Write(wxString::Format("CustomColour%d", i), m_colorData.GetCustomColour(i).GetAsString());
        }       
    }
}

std::tuple<int, wxColour> xlColourData::ShowColorDialog(wxWindow* parent, const wxColour& colour)
{
    m_colorData.SetColour(colour);
    wxColourDialog dlg(parent, &m_colorData);
    // OptimiseDialogPosition(&dlg);
    auto result = dlg.ShowModal();
    m_colorData = dlg.GetColourData();
    return { result, m_colorData.GetColour() };
}