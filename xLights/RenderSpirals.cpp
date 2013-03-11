void RgbEffects::RenderSpirals(int PaletteRepeat, int Direction, int Rotation, int Thickness, bool Blend, bool Show3D)
{
    int strand_base,strand,thick,x,y,ColorIdx;
    size_t colorcnt=GetColorCount();
    int SpiralCount=colorcnt * PaletteRepeat;
    int deltaStrands=BufferWi / SpiralCount;
    int SpiralThickness=(deltaStrands * Thickness / 100) + 1;
    long SpiralState=state*Direction;
    wxImage::HSVValue hsv;
    wxColour color;
    for(int ns=0; ns < SpiralCount; ns++)
    {
        strand_base=ns * deltaStrands;
        ColorIdx=ns % colorcnt;
        palette.GetColor(ColorIdx,color);
        for(thick=0; thick < SpiralThickness; thick++)
        {
            strand = (strand_base + thick) % BufferWi;
            for(y=0; y < BufferHt; y++)
            {
                x=(strand + SpiralState/10 + y*Rotation/BufferHt) % BufferWi;
                if (x < 0) x += BufferWi;
                if (Blend)
                {
                    GetMultiColorBlend(double(BufferHt-y-1)/double(BufferHt), false, color);
                }
                if (Show3D)
                {
                    Color2HSV(color,hsv);
                    if (Rotation < 0)
                    {
                        hsv.value*=double(thick+1)/SpiralThickness;
                    }
                    else
                    {
                        hsv.value*=double(SpiralThickness-thick)/SpiralThickness;
                    }
                    SetPixel(x,y,hsv);
                }
                else
                {
                    SetPixel(x,y,color);
                }
            }
        }
    }
}
