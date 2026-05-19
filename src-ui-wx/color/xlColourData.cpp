#include "xlColourData.h"
#include "xlColourPickerDialog.h"
#include "shared/utils/wxUtilities.h"
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
        config->Read("UseCustomColorPicker", &m_useCustomPicker, false);
    }
}

void xlColourData::Save(XLightsConfigAdapter* config)
{
    if (config != nullptr) {
        for (int i = 0; i < m_colorData.NUM_CUSTOM; ++i) {
            config->Write("CustomColour" + std::to_string(i),
                          wxString((std::string)wxColourToXlColor(m_colorData.GetCustomColour(i))));
        }
        config->Write("UseCustomColorPicker", m_useCustomPicker);
    }
}

void xlColourData::SetUseCustomPicker(bool v)
{
    m_useCustomPicker = v;
    // persist immediately so the setting survives without explicit save
    if (auto* cfg = GetXLightsConfig()) {
        cfg->Write("UseCustomColorPicker", m_useCustomPicker);
    }
}

std::tuple<int, wxColour> xlColourData::ShowColorDialog(wxWindow* parent, const wxColour& colour)
{
    if (m_useCustomPicker) {
        xlColourPickerDialog dlg(parent, colour);
        auto result = dlg.ShowModal();
        if (result == wxID_OK) {
            wxColour chosen = dlg.GetColour();
            m_colorData.SetColour(chosen);
            // Recent-color push is handled inside xlColourPickerDialog::OnOK
            return { wxID_OK, chosen };
        }
        return { wxID_CANCEL, colour };
    }

    m_colorData.SetColour(colour);
    wxColourDialog dlg(parent, &m_colorData);
    auto result = dlg.ShowModal();
    m_colorData = dlg.GetColourData();
    xlColor c = wxColourToXlColor(m_colorData.GetColour());
    return { result, xlColorToWxColour(c) };
}
