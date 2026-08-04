/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include "MHPatternPresetBitmapButton.h"
#include "effects/MovingHeadEffect.h"
#include "utils/UtilFunctions.h"

#include <wx/graphics.h>

MHPatternPresetBitmapButton::MHPatternPresetBitmapButton(wxWindow* parent, wxWindowID id, const wxBitmapBundle& bitmap, const wxPoint& pos,
    const wxSize& size, long style, const wxValidator& validator,
    const wxString& name)
    : wxBitmapButton(parent, id, bitmap, pos, size, style, validator, name)
{
}

MHPatternPresetBitmapButton::~MHPatternPresetBitmapButton()
{
}

void MHPatternPresetBitmapButton::DoSetSizeHints(int minW, int minH,
    int maxW, int maxH,
    int incW, int incH)
{
    int offset = 0;
#ifdef LINUX
    offset = 12; //Linux puts a 6 pixel border around it
#endif // LINUX
    wxBitmapButton::DoSetSizeHints(minW + offset,
        minH + offset,
        maxW + offset,
        maxH + offset,
        incW, incH);
}

void MHPatternPresetBitmapButton::SetPreset(const std::string& _settings)
{
    mSettings = _settings;
    RenderNewBitmap();
}

void MHPatternPresetBitmapButton::RenderNewBitmap() {
    SetBitmap(CreateImage(48, 48, GetContentScaleFactor()));
}

wxBitmap MHPatternPresetBitmapButton::CreateImage( int w, int h, double scaleFactor ) {
    if (scaleFactor < 1.0) {
        scaleFactor = 1.0;
    }
    float width = w * scaleFactor;
    float height = h * scaleFactor;

    wxBitmap bmp(width, height);

    wxMemoryDC dc(bmp);
    dc.SetBrush(*wxBLACK_BRUSH);
    dc.SetPen(*wxWHITE_PEN);
    dc.DrawRectangle(0, 0, width, height);

    std::string algorithm = "Circle";
    float pattern_width = 90.0f;
    float pattern_height = 45.0f;
    float x_offset = 0.0f;
    float y_offset = 0.0f;
    float rotation = 0.0f;
    float x_freq = 2.0f;
    float y_freq = 3.0f;
    float x_phase = 90.0f;
    float y_phase = 0.0f;

    wxArrayString cmds = wxSplit(mSettings, ';');
    for (size_t i = 0; i < cmds.size(); ++i) {
        std::string cmd = cmds[i].ToStdString();
        if (cmd.empty()) continue;
        size_t pos = cmd.find(":");
        if (pos == std::string::npos) continue;
        std::string cmd_type = cmd.substr(0, pos);
        std::string value = cmd.substr(pos + 2);
        if (cmd_type == "Pattern") {
            algorithm = value;
        } else if (cmd_type == "PatternWidth") {
            pattern_width = wxAtof(value);
        } else if (cmd_type == "PatternHeight") {
            pattern_height = wxAtof(value);
        } else if (cmd_type == "PatternXOffset") {
            x_offset = wxAtof(value);
        } else if (cmd_type == "PatternYOffset") {
            y_offset = wxAtof(value);
        } else if (cmd_type == "PatternRotation") {
            rotation = wxAtof(value);
        } else if (cmd_type == "PatternXFreq") {
            x_freq = wxAtof(value);
        } else if (cmd_type == "PatternYFreq") {
            y_freq = wxAtof(value);
        } else if (cmd_type == "PatternXPhase") {
            x_phase = wxAtof(value);
        } else if (cmd_type == "PatternYPhase") {
            y_phase = wxAtof(value);
        }
        // PatternRotation VC, PatternStartOffset and PatternPhaseOffset don't
        // change the shape traced by the path over a full cycle, so they're
        // not needed for this static preview.
    }

    const float cosR = std::cos((float)(rotation * PI / 180.0));
    const float sinR = std::sin((float)(rotation * PI / 180.0));
    const float x_phase_rad = (float)(x_phase * PI / 180.0);
    const float y_phase_rad = (float)(y_phase * PI / 180.0);

    const int steps = 120;
    std::vector<wxPoint2DDouble> pts;
    pts.reserve(steps + 1);
    float minX = 0.0f, maxX = 0.0f, minY = 0.0f, maxY = 0.0f;
    for (int i = 0; i <= steps; ++i) {
        float iterator = (float)(i * 2.0 * PI / steps);
        float x = 0.0f, y = 0.0f;
        MovingHeadEffect::CalculatePatternPoint(algorithm, iterator, x_freq, y_freq, x_phase_rad, y_phase_rad, x, y);
        float px = x_offset + x * cosR * pattern_width + y * sinR * pattern_height;
        float py = y_offset - x * sinR * pattern_width + y * cosR * pattern_height;
        pts.emplace_back(px, py);
        if (i == 0) {
            minX = maxX = px;
            minY = maxY = py;
        } else {
            minX = std::min(minX, px);
            maxX = std::max(maxX, px);
            minY = std::min(minY, py);
            maxY = std::max(maxY, py);
        }
    }

    {
        std::unique_ptr<wxGraphicsContext> gc(wxGraphicsContext::Create(dc));

        wxGraphicsPen pen = gc->CreatePen(wxGraphicsPenInfo(*wxGREEN));
        gc->SetPen(pen);

        const float padding = 0.15f * std::min(width, height);
        const float rangeX = std::max(maxX - minX, 0.01f);
        const float rangeY = std::max(maxY - minY, 0.01f);
        const float scale = std::min((width - 2.0f * padding) / rangeX, (height - 2.0f * padding) / rangeY);
        const float cx = (minX + maxX) / 2.0f;
        const float cy = (minY + maxY) / 2.0f;

        auto toUI = [&](const wxPoint2DDouble& pt) {
            return wxPoint2DDouble(width / 2.0f + (pt.m_x - cx) * scale, height / 2.0f - (pt.m_y - cy) * scale);
        };

        wxGraphicsPath graphicsPath(gc->CreatePath());
        graphicsPath.MoveToPoint(toUI(pts[0]));
        for (size_t i = 1; i < pts.size(); ++i) {
            graphicsPath.AddLineToPoint(toUI(pts[i]));
        }
        gc->DrawPath(graphicsPath);
    }

    if (scaleFactor > 1.0f) {
        wxImage img = bmp.ConvertToImage();
        return wxBitmap(img, 8, scaleFactor);
    }
    return bmp;
}

void MHPatternPresetBitmapButton::SetBitmap(const wxBitmapBundle& bpm)
{
    wxBitmapButton::SetBitmap(bpm);
}
