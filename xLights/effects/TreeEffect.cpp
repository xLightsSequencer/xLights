#include "TreeEffect.h"
#include "TreePanel.h"

#include "../sequencer/Effect.h"
#include "../RenderBuffer.h"
#include "../UtilClasses.h"

#include "../../include/tree-16.xpm"
#include "../../include/tree-24.xpm"
#include "../../include/tree-32.xpm"
#include "../../include/tree-48.xpm"
#include "../../include/tree-64.xpm"

TreeEffect::TreeEffect(int id) : RenderableEffect(id, "Tree", tree_16, tree_24, tree_32, tree_48, tree_64)
{
    //ctor
}

TreeEffect::~TreeEffect()
{
    //dtor
}

wxPanel *TreeEffect::CreatePanel(wxWindow *parent) {
    return new TreePanel(parent);
}

void TreeEffect::SetDefaultParameters(Model *cls)
{
    TreePanel *tp = (TreePanel*)panel;
    if (tp == nullptr) {
        return;
    }

    SetSliderValue(tp->Slider_Tree_Branches, 3);
    SetSliderValue(tp->Slider_Tree_Speed, 10);
    SetCheckBoxValue(tp->CheckBox1, false);
}

bool TreeEffect::needToAdjustSettings(const std::string &version)
{
    return IsVersionOlder("2016.50", version);
}

void TreeEffect::adjustSettings(const std::string &version, Effect *effect)
{
    SettingsMap &settings = effect->GetSettings();
    settings["E_CHECKBOX_Tree_ShowLights"] = "1";

    // also give the base class a chance to adjust any settings
    if (RenderableEffect::needToAdjustSettings(version))
    {
        RenderableEffect::adjustSettings(version, effect);
    }
}

void TreeEffect::Render(Effect *effect, const SettingsMap &SettingsMap, RenderBuffer &buffer) {
    int Branches = SettingsMap.GetInt("SLIDER_Tree_Branches", 1);
    int tspeed = SettingsMap.GetInt("SLIDER_Tree_Speed", 10);
    bool showlights = SettingsMap.GetBool("CHECKBOX_Tree_ShowLights", false);
    
    int effectState = (buffer.curPeriod - buffer.curEffStartPer) * tspeed * buffer.frameTimeInMs / 50;
    
    int x,y,i,r,ColorIdx,pixels_per_branch;
    int maxFrame,mod,branch,row,b,f_mod,m,frame;
    int number_garlands,f_mod_odd,s_odd_row,odd_even;
    float V,H;
    
    number_garlands=1;
    xlColor color;
    if(Branches<1)  Branches=1;
    pixels_per_branch=(int)(0.5+buffer.BufferHt/Branches);
    if(pixels_per_branch<1) pixels_per_branch=1;
    
    maxFrame=(Branches+1) * buffer.BufferWi;
    if(effectState>0 && maxFrame>0) frame = (effectState/4)%maxFrame;
    else frame=1;
    
    i=0;
    
    for (y=0; y<buffer.BufferHt; y++) // For my 20x120 megatree, BufferHt =120
    {
        for (x=0; x<buffer.BufferWi; x++) // BufferWi=20 in the above example
        {
            if(pixels_per_branch>0) mod=y%pixels_per_branch;
            else mod=0;
            if(mod==0) mod=pixels_per_branch;
            V=1-(1.0*mod/pixels_per_branch)*0.70;
            i++;
            
            ColorIdx=0;
            buffer.palette.GetColor(ColorIdx, color); // Now go and get the hsv value for this ColorIdx
            if (buffer.allowAlpha) {
                color.alpha = 255.0 * V;
            } else {
                HSVValue hsv = color.asHSV();
                hsv.value = V; // we have now set the color for the background tree
                color = hsv;
            }
            
            //   $orig_rgbval=$rgb_val;
            branch = (int)((y-1)/pixels_per_branch);
            row = pixels_per_branch-mod; // now row=0 is bottom of branch, row=1 is one above bottom
            //  mod = which pixel we are in the branch
            //	mod=1,row=pixels_per_branch-1   top picrl in branch
            //	mod=2, second pixel down into branch
            //	mod=pixels_per_branch,row=0  last pixel in branch
            //
            //	row = 0, the $p is in the bottom row of tree
            //	row =1, the $p is in second row from bottom
            b = (int) ((effectState)/buffer.BufferWi)%Branches; // what branch we are on based on frame #
            //
            //	b = 0, we are on bottomow row of tree during frames 1 to BufferWi
            //	b = 1, we are on second row from bottom, frames = BufferWi+1 to 2*BufferWi
            //	b = 2, we are on third row from bottome, frames - 2*BufferWi+1 to 3*BufferWi
            f_mod = (effectState/4)%buffer.BufferWi;
            //   if(f_mod==0) f_mod=BufferWi;
            //	f_mod is  to BufferWi-1 on each row
            //	f_mod == 0, left strand of this row
            //	f_mod==BufferWi, right strand of this row
            //
            m=(x%6);
            if(m==0) m=6;  // use $m to indicate where we are in horizontal pattern
            // m=1, 1sr strand
            // m=2, 2nd strand
            // m=6, last strand in 6 strand pattern
            
            
            
            r=branch%5;
            H = r/4.0;
            
            odd_even=b%2;
            s_odd_row = buffer.BufferWi-x+1;
            f_mod_odd = buffer.BufferWi-f_mod+1;
            
            if(branch<=b && x<=frame && // for branches below or equal to current row
               (((row==3 || (number_garlands==2 && row==6)) && (m==1 || m==6))
                ||
                ((row==2 || (number_garlands==2 && row==5)) && (m==2 || m==5))
                ||
                ((row==1 || (number_garlands==2 && row==4)) && (m==3 || m==4))
                ))

                if (showlights)
                {
                    if ((odd_even == 0 && x <= f_mod) || (odd_even == 1 && s_odd_row <= f_mod))
                    {
                        HSVValue hsv;
                        hsv.hue = H;
                        hsv.saturation = 1.0;
                        hsv.value = 1.0;
                        color = hsv;
                    }
                }
            //	if(branch>b)
            //	{
            //		return $rgb_val; // for branches below current, dont dont balnk anything out
            //	}
            //	else if(branch==b)
            //	{
            //		if(odd_even ==0 && x>f_mod)
            //		{
            //			$rgb_val=$orig_rgbval;// we are even row ,counting from bottom as zero
            //		}
            //		if(odd_even ==1 && s_odd_row>f_mod)
            //		{
            //			$rgb_val=$orig_rgbval;// we are even row ,counting from bottom as zero
            //		}
            //	}
            //if($branch>$b) $rgb_val=$orig_rgbval; // erase rows above our current row.
            
            
            // Yes, so now decide on what color it should be
            
            
            //  we left the Hue and Saturation alone, we are just modifiying the Brightness Value
            buffer.SetPixel(x,y,color); // Turn pixel on
            
        }
    }
}