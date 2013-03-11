void RgbEffects::RenderText(int Top, const wxString& Line1, const wxString& Line2, const wxString& FontString, int dir)
{
    wxColour c;
    wxBitmap bitmap(BufferWi,BufferHt);
    wxMemoryDC dc(bitmap);
    wxFont font;
    font.SetNativeFontInfoUserDesc(FontString);
    dc.SetFont(font);
    palette.GetColor(0,c);
    dc.SetTextForeground(c);
    wxString msg = Line1;
    if (!Line2.IsEmpty()) msg+=wxT("\n")+Line2;
    wxSize sz1 = dc.GetTextExtent(Line1);
    wxSize sz2 = dc.GetTextExtent(Line2);
    int maxwidth=sz1.GetWidth() > sz2.GetWidth() ? sz1.GetWidth() : sz2.GetWidth();
    int maxht=sz1.GetHeight() > sz2.GetHeight() ? sz1.GetHeight() : sz2.GetHeight();
    int dctop=Top * BufferHt / 50 - BufferHt/2;
    int xlimit=(BufferWi+maxwidth)*8 + 1;
    int ylimit=(BufferHt+maxht)*8 + 1;
    int xcentered=(BufferWi-maxwidth)/2;
    switch (dir)
    {
    case 0:
        // left
        dc.DrawText(msg,BufferWi-state % xlimit/8,dctop);
        break;
    case 1:
        // right
        dc.DrawText(msg,state % xlimit/8-BufferWi,dctop);
        break;
    case 2:
        // up
        dc.DrawText(msg,xcentered,BufferHt-state % ylimit/8);
        break;
    case 3:
        // down
        dc.DrawText(msg,xcentered,state % ylimit / 8 - BufferHt);
        break;
    default:
        // no movement - centered
        dc.DrawText(msg,xcentered,dctop);
        break;
    }

    // copy dc to buffer
    for(wxCoord x=0; x<BufferWi; x++)
    {
        for(wxCoord y=0; y<BufferHt; y++)
        {
            dc.GetPixel(x,BufferHt-y-1,&c);
            SetPixel(x,y,c);
        }
    }
}
