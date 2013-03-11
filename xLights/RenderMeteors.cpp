void RgbEffects::RenderMeteors(int MeteorType, int Count, int Length)
{
    if (state == 0) meteors.clear();
    int mspeed=state/4;
    state-=mspeed*4;

    // create new meteors
    MeteorClass m;
    wxImage::HSVValue hsv,hsv0,hsv1;
    palette.GetHSV(0,hsv0);
    palette.GetHSV(1,hsv1);
    size_t colorcnt=GetColorCount();
    Count=BufferWi * Count / 100;
    int TailLength=(BufferHt < 10) ? Length / 10 : BufferHt * Length / 100;
    if (TailLength < 1) TailLength=1;
    int TailStart=BufferHt - TailLength;
    if (TailStart < 1) TailStart=1;
    for(int i=0; i<Count; i++)
    {
        m.x=rand() % BufferWi;
        m.y=BufferHt - 1 - rand() % TailStart;
        switch (MeteorType)
        {
        case 1:
            SetRangeColor(hsv0,hsv1,m.hsv);
            break;
        case 2:
            palette.GetHSV(rand()%colorcnt, m.hsv);
            break;
        }
        meteors.push_back(m);
    }

    // render meteors
    for (MeteorList::iterator it=meteors.begin(); it!=meteors.end(); ++it)
    {
        for(int ph=0; ph<TailLength; ph++)
        {
            switch (MeteorType)
            {
            case 0:
                hsv.hue=double(rand() % 1000) / 1000.0;
                hsv.saturation=1.0;
                hsv.value=1.0;
                break;
            default:
                hsv=it->hsv;
                break;
            }
            hsv.value*=1.0 - double(ph)/TailLength;
            SetPixel(it->x,it->y+ph,hsv);
        }
        it->y -= mspeed;
    }

    // delete old meteors
    meteors.remove_if(MeteorHasExpired(TailLength));
}
