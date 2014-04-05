void RenderBars(int PaletteRepeat, int Direction, bool Highlight, bool Show3D);
void RenderButterfly(int ColorScheme, int Style, int Chunks, int Skip);
void RenderCircles(int number,int radius, bool bounce, bool collide, bool random,
                   bool radial, bool radial_3D,  int start_x, int start_y, bool plasma);
void RenderColorWash(bool HorizFade, bool VertFade, int RepeatCount);
void RenderCurtain(int edge, int effect, int swag, bool repeat);
void RenderFaces(int ColorScheme, int Style, int Chunks, int Skip);
void RenderFire(int HeightPct,int HueShift,bool GrowFire);
void RenderFireworks(int Number_Explosions,int Count,float Velocity,int Fade);
void RenderGarlands(int GarlandType, int Spacing);
void RenderLife(int Count, int Type);
void RenderMeteors(int ColorScheme, int Count, int Length, int MeteorsEffect, int SwirlIntensity);
void RenderPiano(int Style, int NumKeys, int NumRows, int KeyPlacement, bool Clipping, const wxString& CueFilename, const wxString& MapFilename, const wxString& ShapeFilename); //added controls -DJ
void RenderPictures(int dir, const wxString& NewPictureName,int GifSpeed);
void RenderSingleStrand(int Color_Mix1,int Chase_Spacing1,int Chase_Type1, bool Chase_3dFade1, bool Chase_Group_All);
void RenderSnowflakes(int Count, int SnowflakeType);
void RenderSnowstorm(int Count, int Length);
void RenderSpirals(int PaletteRepeat, int Direction, int Rotation, int Thickness,
                   bool Blend, bool Show3D, bool grow, bool shrink);
void RenderSpirograph(int R, int r, int d,bool Animate);
void RenderText(int Position1, const wxString& Line1, const wxString& FontString1,int dir1,int TextRotation1,int Effect1,
                int Position2, const wxString& Line2, const wxString& FontString2,int dir2,int TextRotation2,int Effect2,
                int Position3, const wxString& Line3, const wxString& FontString3,int dir3,int TextRotation3,int Effect3,
                int Position4, const wxString& Line4, const wxString& FontString4,int dir4,int TextRotation4,int Effect4);
void RenderTree(int Branches);
void RenderTwinkle(int Count,int Steps,bool Strobe);
