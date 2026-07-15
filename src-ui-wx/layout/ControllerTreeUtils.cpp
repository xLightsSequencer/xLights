/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include "ControllerTreeUtils.h"

#include <wx/dcmemory.h>
#include <wx/gdicmn.h>
#include <wx/image.h>
#include <cmath>

#include <map>

#include "xLightsMain.h"
#include "layout/LayoutUtils.h"
#include "controllers/ControllerUploadData.h"
#include "models/Model.h"
#include "outputs/Controller.h"
#include "outputs/Output.h"
#include "outputs/OutputManager.h"

const ControllerColumnDef CTRL_COLS[] = {
    { "Controller Name", 0 },
    { "Protocol", 1 },
    { "Address", 2 },
    { "Universe/ID", 3 },
    { "Channels", 4 },
    { "Vendor", 5 },
    { "Model", 6 },
    { "Variant", 7 },
    { "Active", 8 },
    { "Auto Layout", 9 },
    { "Auto Size", 10 },
    { "Description", 11 },
    { "Auto Upload", 12 },
    { "Port Brightness", 13 },
    { "Port Gamma", 14 },
    { "Proxy", 15 },
    { "Keep Channels", 16 },
    { "Full xLights Control", 17 },
};
const size_t CTRL_COL_COUNT = sizeof(CTRL_COLS) / sizeof(CTRL_COLS[0]);

const int CTRL_FULL_COLUMN_ORDER[] = {
    0, 2, 1, 3, 4, 8, 5, 6, 7, 15, 9, 12, 10, 17, 13, 14, 16, 11
};
const size_t CTRL_FULL_COLUMN_ORDER_COUNT = sizeof(CTRL_FULL_COLUMN_ORDER) / sizeof(CTRL_FULL_COLUMN_ORDER[0]);

std::string ControllerColumnLabel(const Controller* c, int id) {
    switch (id) {
    case 1: return c->GetColumn1Label();
    case 2: return c->GetColumn2Label();
    case 3: return c->GetColumn3Label();
    case 4: return c->GetColumn4Label();
    case 5: return c->GetColumn5Label();
    case 6: return c->GetColumn6Label();
    case 7: return c->GetColumn7Label();
    case 8: return c->GetColumn8Label();
    case 9: return c->GetColumn9Label();
    case 10: return c->GetColumn10Label();
    case 11: return c->GetColumn11Label();
    case 12: return c->GetColumn12Label();
    case 13: return c->GetColumn13Label();
    case 14: return c->GetColumn14Label();
    case 15: return c->GetColumn15Label();
    case 16: return c->GetColumn16Label();
    case 17: return c->GetColumn17Label();
    default: return "";
    }
}

const char* ControllerColumnTitle(int id) {
    for (size_t i = 0; i < CTRL_COL_COUNT; i++) {
        if (CTRL_COLS[i].id == id) return CTRL_COLS[i].title;
    }
    return "";
}

wxBitmap CreateLedBitmap(const wxColour& color, int size) {
    wxImage image(size, size);
    image.InitAlpha();
    unsigned char* rgb = image.GetData();
    unsigned char* alpha = image.GetAlpha();
    double cx = size / 2.0;
    double cy = size / 2.0;
    double r = size / 2.0 - 2.0;
    double edge0 = r - 0.5;
    double edge1 = r + 0.5;

    for (int y = 0; y < size; ++y) {
        for (int x = 0; x < size; ++x) {
            int idx = y * size + x;
            rgb[idx * 3]     = color.Red();
            rgb[idx * 3 + 1] = color.Green();
            rgb[idx * 3 + 2] = color.Blue();

            double px = x + 0.5;
            double py = y + 0.5;
            double dist = std::sqrt((px - cx) * (px - cx) + (py - cy) * (py - cy));
            double alpha_val = 0.0;
            if (dist < edge0) {
                alpha_val = 255.0;
            } else if (dist < edge1) {
                alpha_val = (1.0 - (dist - edge0)) * 255.0;
            } else {
                alpha_val = 0.0;
            }
            alpha[idx] = (unsigned char)alpha_val;
        }
    }
    return wxBitmap(image);
}

wxBitmapBundle CreateLedBitmapBundle(const wxColour& color, int size) {
    wxVector<wxBitmap> bitmaps;
    for (int scale = 1; scale <= 3; ++scale) {
        bitmaps.push_back(CreateLedBitmap(color, size * scale));
    }
    return wxBitmapBundle::FromBitmaps(bitmaps);
}

ControllerLedIcons AppendControllerLedIcons(wxVector<wxBitmapBundle>& bundles) {
    ControllerLedIcons icons;
    icons.idxGreen = (int)bundles.size();
    bundles.push_back(CreateLedBitmapBundle(wxColour(40, 180, 40), 16));
    icons.idxRed = (int)bundles.size();
    bundles.push_back(CreateLedBitmapBundle(wxColour(210, 40, 40), 16));
    icons.idxGray = (int)bundles.size();
    bundles.push_back(CreateLedBitmapBundle(wxColour(128, 128, 128), 16));
    return icons;
}

ControllerLedBitmaps CreateControllerLedBitmaps(int size) {
    ControllerLedBitmaps b;
    b.green = CreateLedBitmapBundle(wxColour(40, 180, 40), size);
    b.red = CreateLedBitmapBundle(wxColour(210, 40, 40), size);
    b.gray = CreateLedBitmapBundle(wxColour(128, 128, 128), size);
    return b;
}

ControllerPingBucket ClassifyControllerPing(const Controller* c) {
    auto ps = c->GetLastPingState();
    if (ps == Output::PINGSTATE::PING_ALLFAILED) {
        return ControllerPingBucket::Red;
    } else if (ps == Output::PINGSTATE::PING_UNKNOWN || ps == Output::PINGSTATE::PING_UNAVAILABLE) {
        return ControllerPingBucket::Gray;
    }
    return ControllerPingBucket::Green;
}

int GetControllerIconIndex(const Controller* c, xLightsFrame* frame, const ControllerLedIcons& icons) {
    if (frame->GetControllerPingInterval() <= 0) {
        return -1;
    }
    if (!c->IsActive()) {
        return -1;
    }
    switch (ClassifyControllerPing(c)) {
    case ControllerPingBucket::Green: return icons.idxGreen;
    case ControllerPingBucket::Red: return icons.idxRed;
    default: return icons.idxGray;
    }
}

void PopulateControllerPorts(wxTreeListCtrl* tree, wxTreeListItem ctrlItem, xLightsFrame* frame) {
    wxTreeListItem first = tree->GetFirstChild(ctrlItem);
    if (!first.IsOk()) return;
    auto* data = dynamic_cast<CtrlRowData*>(tree->GetItemData(first));
    if (data == nullptr || data->kind != CtrlRowData::Kind::Placeholder) return; // already populated

    auto ctrlName = tree->GetItemText(ctrlItem, 0).ToStdString();
    Controller* controller = frame->GetOutputManager()->GetController(ctrlName);
    if (controller == nullptr) return;

    while (first.IsOk()) { // remove placeholder
        wxTreeListItem next = tree->GetNextSibling(first);
        tree->DeleteItem(first);
        first = next;
    }

    UDController cud(controller, frame->GetOutputManager(), &frame->AllModels, false);

    auto addPort = [&](UDControllerPort* port, const wxString& label) {
        if (port == nullptr || port->GetModelCount() == 0) return;
        int closedIcon = LayoutUtils::GetModelTreeIcon("ModelGroup", LayoutUtils::GroupMode::Closed);
        int openedIcon = LayoutUtils::GetModelTreeIcon("ModelGroup", LayoutUtils::GroupMode::Opened);
        wxTreeListItem portItem = tree->AppendItem(ctrlItem, label, closedIcon, openedIcon);
        tree->SetItemData(portItem, new CtrlRowData(CtrlRowData::Kind::Port, ctrlName));

        std::map<Model*, int> entryCount;
        for (auto* pm : port->GetModels()) {
            if (pm->GetModel() != nullptr) entryCount[pm->GetModel()]++;
        }
        for (auto* pm : port->GetModels()) {
            Model* m = pm->GetModel();
            if (m == nullptr) continue;
            wxString mLabel = m->name;
            if (entryCount[m] > 1 && pm->GetString() >= 0) {
                mLabel += wxString::Format(" (string %d)", pm->GetString() + 1);
            }
            if (pm->GetSmartRemote() != 0) {
                mLabel += wxString::Format("  [SR %c]", pm->GetSmartRemoteLetter());
            }
            mLabel += wxString::Format("  ch %d-%d", pm->GetStartChannel(), pm->GetEndChannel());
            int mClosedIcon = LayoutUtils::GetModelTreeIcon(DisplayAsTypeToString(m->GetDisplayAs()), LayoutUtils::GroupMode::Closed);
            int mOpenedIcon = LayoutUtils::GetModelTreeIcon(DisplayAsTypeToString(m->GetDisplayAs()), LayoutUtils::GroupMode::Opened);
            wxTreeListItem mi = tree->AppendItem(portItem, mLabel, mClosedIcon, mOpenedIcon);
            auto* md = new CtrlRowData(CtrlRowData::Kind::Model, ctrlName);
            md->modelName = m->name;
            md->stringIndex = (entryCount[m] > 1) ? -1 : pm->GetString();
            md->startChannel = pm->GetStartChannel();
            md->endChannel = pm->GetEndChannel();
            md->isShadow = m->IsShadowModel();
            md->shadowOf = m->GetShadowModelFor();
            tree->SetItemData(mi, md);
        }
    };

    for (int p = 1; p <= cud.GetMaxPixelPort(); p++) {
        if (!cud.HasPixelPort(p)) continue;
        auto* port = cud.GetControllerPixelPort(p);
        addPort(port, wxString::Format("Pixel Port %d (%s)", p, port->GetProtocol()));
    }
    for (int p = 1; p <= cud.GetMaxSerialPort(); p++) {
        if (!cud.HasSerialPort(p)) continue;
        auto* port = cud.GetControllerSerialPort(p);
        addPort(port, wxString::Format("Serial Port %d (%s)", p, port->GetProtocol()));
    }
    for (int p = 1; p <= cud.GetMaxPWMPort(); p++) {
        if (!cud.HasPWMPort(p)) continue;
        addPort(cud.GetControllerPWMPort(p), wxString::Format("PWM Port %d", p));
    }
    for (int p = 1; p <= cud.GetMaxVirtualMatrixPort(); p++) {
        if (!cud.HasVirtualMatrixPort(p)) continue;
        addPort(cud.GetControllerVirtualMatrixPort(p), wxString::Format("Virtual Matrix %d", p));
    }
    for (int p = 1; p <= cud.GetMaxLEDPanelMatrixPort(); p++) {
        if (!cud.HasLEDPanelMatrixPort(p)) continue;
        addPort(cud.GetControllerLEDPanelMatrixPort(p), wxString::Format("LED Panel %d", p));
    }
}
