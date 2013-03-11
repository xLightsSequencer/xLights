void RgbEffects::RenderTwinkle(int Count)
{

    int x,y,i,i7,r,ColorIdx;
    int lights = (BufferHt*BufferWi)*(Count/100.0); // Count is in range of 1-100 from slider bar
    int step=BufferHt*BufferWi/lights;
    if(step<1) step=1;
    srand(1); // always have the same random numbers for each frame (state)
    wxImage::HSVValue hsv; //   we will define an hsv color model. The RGB colot model would have been "wxColour color;"

    size_t colorcnt=GetColorCount();

    i=0;

    for (y=0; y<BufferHt; y++) // For my 20x120 megatree, BufferHt=120
    {
        for (x=0; x<BufferWi; x++) // BufferWi=20 in the above example
        {
            i++;
            if(i%step==0) // Should we draw a light?
            {
                // Yes, so now decide on what color it should be

                ColorIdx=rand() % colorcnt; // Select random numbers from 0 up to number of colors the user has checked. 0-5 if 6 boxes checked
                palette.GetHSV(ColorIdx, hsv); // Now go and get the hsv value for this ColorIdx
                i7=(state/4+rand())%9; // Our twinkle is 9 steps. 4 ramping up, 5th at full brightness and then 4 more ramping down
                //  Note that we are adding state to this calculation, this causes a different blink rate for each light

                if(i7==0 || i7==8)  hsv.value = 0.1;
                if(i7==1 || i7==7)  hsv.value = 0.3;
                if(i7==2 || i7==6)  hsv.value = 0.5;
                if(i7==3 || i7==5)  hsv.value = 0.7;
                if(i7==4  )  hsv.value = 1.0;
                //  we left the Hue and Saturation alone, we are just modifiying the Brightness Value
                SetPixel(x,y,hsv); // Turn pixel on
            }
        }
    }
}
