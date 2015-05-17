/***************************************************************
 * Name:      RenderMorph.cpp
 * Purpose:   Implements RGB effects
 * Author:    Gil Jones (gil@threebuttes.com)
 * Created:   2015-2-27
 * Copyright: 2015 by Gil Jones
 * License:
     This file is part of xLights.

    xLights is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    xLights is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with xLights.  If not, see <http://www.gnu.org/licenses/>.
**************************************************************/

#include "RgbEffects.h"
#include <cmath>

static void StoreLine( const int x0_, const int y0_, const int x1_, const int y1_, std::vector<int> *vx,  std::vector<int> *vy)
{
    int x0 = x0_;
    int x1 = x1_;
    int y0 = y0_;
    int y1 = y1_;

    int dx = abs(x1-x0), sx = x0<x1 ? 1 : -1;
    int dy = abs(y1-y0), sy = y0<y1 ? 1 : -1;
    int err = (dx>dy ? dx : -dy)/2, e2;

  for(;;){
    vx->push_back(x0);
    vy->push_back(y0);
    if (x0==x1 && y0==y1) break;
    e2 = err;
    if (e2 >-dx) { err -= dy; x0 += sx; }
    if (e2 < dy) { err += dx; y0 += sy; }
  }
}

double RgbEffects::calcAccel(double ratio, double accel)
{
    accel /= 3.0;
    if( accel == 0 ) return 1.0;
    else if( accel > 0 ) return std::pow(ratio, accel);
    else return (1 - std::pow(1 - ratio, -accel));
}

void RgbEffects::RenderMorph(int start_x1, int start_y1, int start_x2, int start_y2, int end_x1, int end_y1, int end_x2, int end_y2,
                             int start_length, int end_length, bool start_linked, bool end_linked, int duration, int acceleration,
                             bool showEntireHeadAtStart, int repeat_count, int repeat_skip, int stagger )
{
    double eff_pos = GetEffectTimeIntervalPosition();
    double step_size = 0.1;

    int hcols = 0, hcole = 1;
    int tcols = 2, tcole = 3;
    int num_tail_colors = 2;
    switch (palette.Size()) {
        case 1:  //one color selected, use it for all
            hcols = hcole = tcols = tcole = 0;
            break;
        case 2: //two colors, head/tail
            hcols = hcole = 0;
            tcols = tcole = 1;
            break;
        case 3: //three colors, head /tail start/end
            hcols = hcole = 0;
            tcols = 1;
            tcole = 2;
            break;
        case 4:
            break;
        case 5:
            num_tail_colors = 3;
            break;
        case 6:
            num_tail_colors = 4;
            break;
    }

    int x1a = (BufferWi-1) * (start_x1/100.0);
    int y1a = (BufferHt-1) * (start_y1/100.0);
    int x2a = (BufferWi-1) * (end_x1/100.0);
    int y2a = (BufferHt-1) * (end_y1/100.0);

    int x1b, x2b, y1b, y2b;

    if( start_linked )
    {
        x1b = x1a;
        y1b = y1a;
    }
    else
    {
        x1b = (BufferWi-1) * (start_x2/100.0);
        y1b = (BufferHt-1) * (start_y2/100.0);
    }

    if( end_linked )
    {
        x2b = x2a;
        y2b = y2a;
    }
    else
    {
        x2b = (BufferWi-1) * (end_x2/100.0);
        y2b = (BufferHt-1) * (end_y2/100.0);
    }

    xlColor head_color, tail_color, test_color;

    // compute direction
    int delta_xa = x2a - x1a;
    int delta_xb = x2b - x1b;
    int delta_ya = y2a - y1a;
    int delta_yb = y2b - y1b;
    int direction = delta_xa + delta_xb + delta_ya + delta_yb;
    int repeat_x = 0;
    int repeat_y = 0;
    double effect_pct = 1.0;
    double stagger_pct = 0.0;
    if( repeat_count > 0 )
    {
        if( (std::abs((float)delta_xa) + std::abs((float)delta_xb)) < (std::abs((float)delta_ya) + std::abs((float)delta_yb)) )
        {
            repeat_x = repeat_skip;
        }
        else
        {
            repeat_y = repeat_skip;
        }
        double stagger_val = (double)(std::abs(stagger))/200.0;
        effect_pct = 1.0 / (1 + stagger_val * repeat_count);
        stagger_pct = effect_pct * stagger_val;
    }

    std::vector<int> v_ax;
    std::vector<int> v_ay;
    std::vector<int> v_bx;
    std::vector<int> v_by;

    StoreLine(x1a, y1a, x2a, y2a, &v_ax, &v_ay);  // store side a
    StoreLine(x1b, y1b, x2b, y2b, &v_bx, &v_by);  // store side b

    int size_a = v_ax.size();
    int size_b = v_bx.size();

    std::vector<int> *v_lngx;  // pointer to longest vector x
    std::vector<int> *v_lngy;  // pointer to longest vector y
    std::vector<int> *v_shtx;  // pointer to shorter vector x
    std::vector<int> *v_shty;  // pointer to shorter vector y

    if( size_a > size_b )
    {
        v_lngx = &v_ax;
        v_lngy = &v_ay;
        v_shtx = &v_bx;
        v_shty = &v_by;
    }
    else
    {
        v_lngx = &v_bx;
        v_lngy = &v_by;
        v_shtx = &v_ax;
        v_shty = &v_ay;
    }

    double pos_a, pos_b;
    double total_tail_length, alpha_pct;
    double total_length = v_lngx->size();     // total length of longest vector
    double head_duration = duration/100.0;    // time the head is in the frame
    double head_end_of_head_pos = total_length + 1;
    double tail_end_of_head_pos = total_length + 1;
    double head_end_of_tail_pos = -1;
    double tail_end_of_tail_pos = -1;

    for( int repeat = 0; repeat <= repeat_count; repeat++ )
    {
        double eff_pos_adj = eff_pos * calcAccel(eff_pos, acceleration);
        double eff_start_pct = (stagger >= 0) ? stagger_pct*repeat : stagger_pct*(repeat_count-repeat);
        double eff_end_pct = eff_start_pct + effect_pct;
        eff_pos_adj = (eff_pos_adj - eff_start_pct) / (eff_end_pct - eff_start_pct);
        if( eff_pos_adj < 0.0 )
        {
            head_end_of_head_pos = -1;
            tail_end_of_head_pos = -1;
            head_end_of_tail_pos = -1;
            tail_end_of_tail_pos = -1;
            total_tail_length = 1.0;
            if( showEntireHeadAtStart )
            {
                head_end_of_head_pos = start_length;
            }
        }
        else
        {
            if( head_duration > 0.0 )
            {
                double head_loc_pct = eff_pos_adj / head_duration;
                head_end_of_head_pos = total_length * head_loc_pct;
                double current_total_head_length = end_length * head_loc_pct + start_length * (1.0 - head_loc_pct);  // adjusted head length excluding clipping
                head_end_of_head_pos += current_total_head_length * head_loc_pct * head_duration;
                total_tail_length = total_length * ( 1 / head_duration - 1.0);
                if( showEntireHeadAtStart )
                {
                    head_end_of_head_pos += current_total_head_length * (1.0 - eff_pos_adj);
                }
                tail_end_of_head_pos = head_end_of_head_pos - current_total_head_length;
                head_end_of_tail_pos = tail_end_of_head_pos - step_size;
                tail_end_of_tail_pos = head_end_of_tail_pos - total_tail_length;
                Get2ColorBlend(hcols, hcole, std::min( head_loc_pct, 1.0), head_color);
            }
            else
            {
                total_tail_length = total_length;
                head_end_of_tail_pos = total_length * 2 * eff_pos_adj;
                tail_end_of_tail_pos = head_end_of_tail_pos - total_tail_length;
            }
        }

        // draw the tail
        for( double i = std::min(head_end_of_tail_pos, total_length-1); i >= tail_end_of_tail_pos && i >= 0.0; i -= step_size )
        {
            double pct = ((total_length == 0) ? 0.0 : i / total_length);
            pos_a = i;
            pos_b = v_shtx->size() * pct;
            double tail_color_pct = (i-tail_end_of_tail_pos) / total_tail_length;
            if( num_tail_colors > 2 )
            {
                double color_index = ((double)num_tail_colors - 1.0) * (1.0 - tail_color_pct);
                tail_color_pct = color_index - (double)((int)color_index);
                tcols = (int)color_index + 2;
                tcole = tcols + 1;
                if( tcole == num_tail_colors+1 )
                {
                    alpha_pct = (1.0 - tail_color_pct);
                }
                else
                {
                    alpha_pct = 1.0;
                }
                Get2ColorBlend(tcols, tcole, tail_color_pct, tail_color);
            }
            else
            {
                if( tail_color_pct > 0.5 )
                {
                    alpha_pct = 1.0;
                }
                else
                {
                    alpha_pct = tail_color_pct / 0.5;
                }
                Get2ColorBlend(tcole, tcols, tail_color_pct, tail_color);
            }
            if( allowAlpha ) {
                tail_color.alpha = 255 * alpha_pct;
            }
            DrawThickLine( (*v_lngx)[pos_a]+(repeat_x*repeat), (*v_lngy)[pos_a]+(repeat_y*repeat), (*v_shtx)[pos_b]+(repeat_x*repeat), (*v_shty)[pos_b]+(repeat_y*repeat), tail_color, direction >= 0);
        }

        // draw the head
        for( double i = std::max(tail_end_of_head_pos, 0.0); i <= head_end_of_head_pos && i < total_length; i += step_size )
        {
            double pct = ((total_length == 0) ? 0.0 : i / total_length);
            pos_a = i;
            pos_b = v_shtx->size() * pct;
            DrawThickLine( (*v_lngx)[pos_a]+(repeat_x*repeat), (*v_lngy)[pos_a]+(repeat_y*repeat), (*v_shtx)[pos_b]+(repeat_x*repeat), (*v_shty)[pos_b]+(repeat_y*repeat), head_color, direction >= 0);
        }
    }
}

