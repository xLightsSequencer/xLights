void RgbEffects::RenderTree(int Branches)
{

    int x,y,i,i7,r,ColorIdx,Count,pixels_per_branch;
    int maxFrame,mod,branch,row,b,f_mod,m,frame;
    int number_garlands,f_mod_odd,s_odd_row,odd_even;
    float V,H;

    number_garlands=1;
    srand(1); // always have the same random numbers for each frame (state)
    wxImage::HSVValue hsv; //   we will define an hsv color model. The RGB colot model would have been "wxColour color;"
    pixels_per_branch=(int)(0.5+BufferHt/Branches);
    maxFrame=(Branches+1) *BufferWi;
    size_t colorcnt=GetColorCount();
    frame = (state/4)%maxFrame;

    i=0;

    for (y=0; y<BufferHt; y++) // For my 20x120 megatree, BufferHt=120
    {
        for (x=0; x<BufferWi; x++) // BufferWi=20 in the above example
        {
            mod=y%pixels_per_branch;
            if(mod==0) mod=pixels_per_branch;
            V=1-(1.0*mod/pixels_per_branch)*0.70;
            i++;

            ColorIdx=rand() % colorcnt; // Select random numbers from 0 up to number of colors the user has checked. 0-5 if 6 boxes checked
            ColorIdx=0;
            palette.GetHSV(ColorIdx, hsv); // Now go and get the hsv value for this ColorIdx
            hsv.value = V; // we have now set the color for the background tree

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
            b = (int) ((state)/BufferWi)%Branches; // what branch we are on based on frame #
            //
            //	b = 0, we are on bottomow row of tree during frames 1 to BufferWi
            //	b = 1, we are on second row from bottom, frames = BufferWi+1 to 2*BufferWi
            //	b = 2, we are on third row from bottome, frames - 2*BufferWi+1 to 3*BufferWi
            f_mod = (state/4)%BufferWi;
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
            s_odd_row = BufferWi-x+1;
            f_mod_odd = BufferWi-f_mod+1;

            if(branch<=b && x<=frame && // for branches below or equal to current row
                    (((row==3 or (number_garlands==2 and row==6)) and (m==1 or m==6))
                     ||
                     ((row==2 or (number_garlands==2 and row==5)) and (m==2 or m==5))
                     ||
                     ((row==1 or (number_garlands==2 and row==4)) and (m==3 or m==4))
                    ))
                if((odd_even ==0 and x<=f_mod) || (odd_even ==1 and s_odd_row<=f_mod))
                {
                    hsv.hue = H;
                    hsv.saturation=1.0;
                    hsv.value=1.0;
                }
            //	if(branch>b)
//	{
//		return $rgb_val; // for branches below current, dont dont balnk anything out
//	}
//	else if(branch==b)
//	{
//		if(odd_even ==0 and x>f_mod)
//		{
//			$rgb_val=$orig_rgbval;// we are even row ,counting from bottom as zero
//		}
//		if(odd_even ==1 and s_odd_row>f_mod)
//		{
//			$rgb_val=$orig_rgbval;// we are even row ,counting from bottom as zero
//		}
//	}
            //if($branch>$b) $rgb_val=$orig_rgbval; // erase rows above our current row.


            // Yes, so now decide on what color it should be


            //  we left the Hue and Saturation alone, we are just modifiying the Brightness Value
            SetPixel(x,y,hsv); // Turn pixel on

        }
    }
}
