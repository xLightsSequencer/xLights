void RgbEffects::RenderColorWash(bool HorizFade, bool VertFade, int RepeatCount)
{
    static int SpeedFactor=200;
    int x,y;
    wxColour color;
    wxImage::HSVValue hsv,hsv2;
    size_t colorcnt=GetColorCount();
    int CycleLen=colorcnt*SpeedFactor;
    if (state > (colorcnt-1)*SpeedFactor*RepeatCount && RepeatCount < 10)
    {
        GetMultiColorBlend(double(RepeatCount%2), false, color);
    }
    else
    {
        GetMultiColorBlend(double(state % CycleLen) / double(CycleLen), true, color);
    }
    Color2HSV(color,hsv);
    double HalfHt=double(BufferHt-1)/2.0;
    double HalfWi=double(BufferWi-1)/2.0;
    for (x=0; x<BufferWi; x++)
    {
        for (y=0; y<BufferHt; y++)
        {
            hsv2=hsv;
            if (HorizFade) hsv2.value*=1.0-abs(HalfWi-x)/HalfWi;
            if (VertFade) hsv2.value*=1.0-abs(HalfHt-y)/HalfHt;
            SetPixel(x,y,hsv2);
        }
    }
}
