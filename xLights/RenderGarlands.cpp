void RgbEffects::RenderGarlands(int GarlandType, int Spacing)
{
    int x,y,yadj,ylimit,ring;
    double ratio;
    wxColour color;
    int PixelSpacing=Spacing*BufferHt/100+3;
    int limit=BufferHt*PixelSpacing*4;
    int GarlandsState=(limit - (state % limit))/4;
    // ring=0 is the top ring
    for (ring=0; ring<BufferHt; ring++)
    {
        ratio=double(ring)/double(BufferHt);
        GetMultiColorBlend(ratio, false, color);
        y=GarlandsState - ring*PixelSpacing;
        ylimit=BufferHt-ring-1;
        for (x=0; x<BufferWi; x++)
        {
            yadj=y;
            switch (GarlandType)
            {
            case 1:
                switch (x%5)
                {
                case 2:
                    yadj-=2;
                    break;
                case 1:
                case 3:
                    yadj-=1;
                    break;
                }
                break;
            case 2:
                switch (x%5)
                {
                case 2:
                    yadj-=4;
                    break;
                case 1:
                case 3:
                    yadj-=2;
                    break;
                }
                break;
            case 3:
                switch (x%6)
                {
                case 3:
                    yadj-=6;
                    break;
                case 2:
                case 4:
                    yadj-=4;
                    break;
                case 1:
                case 5:
                    yadj-=2;
                    break;
                }
                break;
            case 4:
                switch (x%5)
                {
                case 1:
                case 3:
                    yadj-=2;
                    break;
                }
                break;
            }
            if (yadj < ylimit) yadj=ylimit;
            if (yadj < BufferHt) SetPixel(x,yadj,color);
        }
    }
}
