#include "xlColourData.h"
#include "ui/shared/utils/wxUtilities.h"
#include "settings/XLightsConfigAdapter.h"
#include <wx/string.h>
#include <string>

#include <wx/colordlg.h>
#include <wx/window.h>

xlColourData xlColourData::INSTANCE;

xlColourData::xlColourData()
{ }

xlColourData::~xlColourData()
{ }

void xlColourData::Load(XLightsConfigAdapter* config)
{
    if (config != nullptr) {
        for (int i = 0; i < m_colorData.NUM_CUSTOM; ++i) {
            wxString color;
            config->Read("CustomColour" + std::to_string(i), &color, "");
            if (!color.IsEmpty()) {
                m_colorData.SetCustomColour(i, wxColour(color));
            }
        }
    }
}

void xlColourData::Save(XLightsConfigAdapter* config)
{
    if (config != nullptr) {
        for (int i = 0; i < m_colorData.NUM_CUSTOM; ++i) {
            config->Write("CustomColour" + std::to_string(i),
                          m_colorData.GetCustomColour(i).GetAsString());
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
