/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include "MHPresetBitmapButton.h"

#include "Color.h"
#include "shared/utils/wxUtilities.h"
#include "utils/string_utils.h"
#include "UtilFunctions.h"

#include <glm/glm.hpp>
#include <glm/gtx/matrix_transform_2d.hpp>
#include <glm/gtx/rotate_vector.hpp>

#include <algorithm>
#include <map>

MHPresetBitmapButton::MHPresetBitmapButton(wxWindow* parent, wxWindowID id, const wxBitmapBundle& bitmap, const wxPoint& pos,
    const wxSize& size, long style, const wxValidator& validator,
    const wxString& name)
    : wxBitmapButton(parent, id, bitmap, pos, size, style, validator, name)
{
    //Connect(wxEVT_LEFT_DOWN, wxMouseEventHandler(DragValueCurveBitmapButton::OnMouseLeftDown));
}

MHPresetBitmapButton::~MHPresetBitmapButton()
{
}
void MHPresetBitmapButton::DoSetSizeHints(int minW, int minH,
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

void MHPresetBitmapButton::SetPreset(const wxArrayString& _settings)
{
    mSettings = _settings;
    RenderNewBitmap();
}

void MHPresetBitmapButton::RenderNewBitmap() {
    wxSize sz = GetSize();
    int w = sz.GetWidth()  > 8 ? sz.GetWidth()  : 48;
    int h = sz.GetHeight() > 8 ? sz.GetHeight() : 24;
    SetBitmap(CreateImage(w, h, GetContentScaleFactor()));
}

wxBitmap MHPresetBitmapButton::CreateImage( int w, int h, double scaleFactor ) {
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

    bool has_pan_vc = false;
    bool pan_vc_reverse = false;

    for( int i = 1; i <= 8; ++i ) {
        if( mSettings[i-1] != "" ) {
            // parse all the commands
            float pan_pos = 0.0f;
            float tilt_pos = 0.0f;
            float pan_offset = 0.0f;
            float tilt_offset = 0.0f;
            int groupings = 1;
            wxArrayString colors;
            wxArrayString heads;
            wxArrayString all_cmds = wxSplit(mSettings[i-1], ';');
            for (size_t j = 0; j < all_cmds.size(); ++j )
            {
                std::string cmd = all_cmds[j];
                if( cmd == xlEMPTY_STRING ) continue;
                int pos = cmd.find(":");
                std::string cmd_type = cmd.substr(0, pos);
                std::string settings = cmd.substr(pos+2, cmd.length());
                std::replace( settings.begin(), settings.end(), '@', ';');
                
                if( cmd_type == "Pan" ) {
                    pan_pos = atof(settings.c_str());
                } else if ( cmd_type == "Tilt" ) {
                    tilt_pos = atof(settings.c_str());
                } else if( cmd_type == "PanOffset" ) {
                    pan_offset = atof(settings.c_str());
                } else if( cmd_type == "TiltOffset" ) {
                    tilt_offset = atof(settings.c_str());
                } else if( cmd_type == "Groupings" ) {
                    groupings = atoi(settings.c_str());
                } else if( cmd_type == "Heads" ) {
                    heads = wxSplit(settings, ',');
                } else if( cmd_type == "Color" ) {
                    colors = wxSplit(settings, ',');
                } else if (cmd_type == "Wheel") {
                    colors = wxSplit(settings, ',');
                } else if (cmd_type == "Pan VC") {
                    if (settings.find("Active=TRUE") != std::string::npos) {
                        has_pan_vc = true;
                        pan_vc_reverse = settings.find("RV=TRUE") != std::string::npos;
                    }
                }
            }

            const float headGap = width * 0.7f / 8.0f;
            const float headHeight = height * 0.3f;
            const float headCenter = width / 2.0f;
            const float headPos = (float)i - 4.5f;

            if( colors.size() >= 3 ) {
                double hue { wxAtof(colors[0]) };
                double sat { wxAtof(colors[1]) };
                double val { wxAtof(colors[2]) };
                HSVValue hsv(hue, sat, val);
                xlColor color;
                color.fromHSV(hsv);
                wxColour beam_color {xlColorToWxColour(color)};
                wxPen pen(beam_color);
                dc.SetPen(pen);
            } else {
                dc.SetPen(*wxWHITE_PEN);
            }

            float pan_latched {pan_pos};
            float tilt_latched {tilt_pos};

            pan_pos = pan_latched;
            tilt_pos = tilt_latched;
            CalculatePosition( i, pan_pos, heads, groupings, pan_offset );
            CalculatePosition( i, tilt_pos, heads, groupings, tilt_offset);

            float pan_angle = wxDegToRad(pan_pos);
            float tilt_angle = wxDegToRad(tilt_pos);

            float head_x = headCenter + (headGap * headPos);

            glm::vec4 beam_point = glm::vec4(glm::vec3(0.0f, 100.0f, 0.0f), 1.0f);
            glm::mat4 rotationMatrixPan = glm::rotate(glm::mat4(1.0f), pan_angle, glm::vec3(0.0f, 1.0f, 0.0f));
            glm::mat4 rotationMatrixTilt = glm::rotate(glm::mat4(1.0f), tilt_angle, glm::vec3(1.0f, 0.0f, 0.0f));
            glm::vec4 beam = beam_point * rotationMatrixTilt * rotationMatrixPan;

            float x1 {head_x};
            float y1 {height - headHeight};
            float x2 {head_x + beam.x};
            float y2 {headHeight - beam.y};

            float xleg = x2 - x1;
            float yleg = y2 - y1;
            float hyp = sqrt(xleg * xleg + yleg * yleg);
            float lineAngle = atan2(yleg, xleg);

            dc.DrawLine( wxPoint(x1, y1), wxPoint(x2, y2));

            // fixed size, not scaled off hyp, so short beams still get a visible arrowhead
            const float arrowLen = std::min(6.0f * (float)scaleFactor, hyp);
            const float arrowAngle = 25.0f * (float)PI / 180.0f;
            wxPoint tip(x2, y2);
            wxPoint arrowP1(x2 - arrowLen * cos(lineAngle - arrowAngle), y2 - arrowLen * sin(lineAngle - arrowAngle));
            wxPoint arrowP2(x2 - arrowLen * cos(lineAngle + arrowAngle), y2 - arrowLen * sin(lineAngle + arrowAngle));
            dc.DrawLine(tip, arrowP1);
            dc.DrawLine(tip, arrowP2);
        }
    }

    if (has_pan_vc) {
        // flags a preset that pans over time; mirrors corner when reversed
        dc.SetPen(wxPen(*wxRED, std::max(1, (int)scaleFactor)));
        const float margin = 4.0f * (float)scaleFactor;
        const float len = 10.0f * (float)scaleFactor;
        float ax1, ay1, ax2, ay2;
        if (!pan_vc_reverse) {
            ax1 = width - margin - len; ay1 = margin + len;
            ax2 = width - margin;       ay2 = margin;
        } else {
            ax1 = margin + len; ay1 = margin + len;
            ax2 = margin;       ay2 = margin;
        }
        dc.DrawLine(wxPoint(ax1, ay1), wxPoint(ax2, ay2));

        const float headAngle = atan2(ay2 - ay1, ax2 - ax1);
        const float headLen = 4.0f * (float)scaleFactor;
        const float headSpread = 25.0f * (float)PI / 180.0f;
        wxPoint tip(ax2, ay2);
        wxPoint head1(ax2 - headLen * cos(headAngle - headSpread), ay2 - headLen * sin(headAngle - headSpread));
        wxPoint head2(ax2 - headLen * cos(headAngle + headSpread), ay2 - headLen * sin(headAngle + headSpread));
        dc.DrawLine(tip, head1);
        dc.DrawLine(tip, head2);
    }

    dc.SetBrush(*wxTRANSPARENT_BRUSH);
    dc.SetPen(*wxWHITE_PEN);
    dc.DrawRectangle(0, 0, width, height);

    if (scaleFactor > 1.0f) {
        wxImage img = bmp.ConvertToImage();
        return wxBitmap(img, 8, scaleFactor);
    }
    return bmp;
}

void MHPresetBitmapButton::SetBitmap(const wxBitmapBundle& bpm)
{
    wxBitmapButton::SetBitmap(bpm);
}

void MHPresetBitmapButton::CalculatePosition(int location, float& position, wxArrayString& heads, int groupings, float offset )
{
    std::map<int, int> locations;
    for (size_t i = 0; i < heads.size(); ++i )
    {
        int head = wxAtoi(heads[i]);
        locations[head] = i+1;
    }

    // calculate the slot number within the group
    float slot = (float)locations[location];
    float center = (float)(groupings > 1 ? groupings : heads.size()) / 2.0f + 0.5;
    if( groupings > 1 ) {
        slot = (float)((locations[location]-1) % groupings + 1);
    }
    position = (slot - center) * offset + position;
}

