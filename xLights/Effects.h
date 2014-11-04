void RenderBars(wxXmlNode* model,int PaletteRepeat, int Direction, bool Highlight, bool Show3D);
void RenderButterfly(wxXmlNode* model, int ColorScheme, int Style, int Chunks, int Skip, int ButterflyDirection);
void RenderCircles(wxXmlNode* model, int number,int radius, bool bounce, bool collide, bool random,
                   bool radial, bool radial_3D,  int start_x, int start_y, bool plasma);
void RenderColorWash(wxXmlNode* model, bool HorizFade, bool VertFade, int RepeatCount);
void RenderCurtain(wxXmlNode* model, int edge, int effect, int swag, bool repeat);
void RenderFaces(wxXmlNode* model, int Phoneme);
//void RenderCoroFaces( int Phoneme, const wxString& x_y, const wxString& Outline_x_y, const wxString& Eyes_x_y/*, const wxString& parsed_xy*/);
void RenderCoroFaces(wxXmlNode* model, const wxString& Phoneme, const wxString& eyes, bool face_outline);
void RenderFire(wxXmlNode* model, int HeightPct,int HueShift,bool GrowFire);
void RenderFireworks(wxXmlNode* model, int Number_Explosions,int Count,float Velocity,int Fade);
void RenderGarlands(wxXmlNode* model, int GarlandType, int Spacing);
void RenderGlediator(wxXmlNode* model, const wxString& NewPictureName);
void RenderLife(wxXmlNode* model, int Count, int Type);
void RenderMeteors(wxXmlNode* model, int ColorScheme, int Count, int Length, int MeteorsEffect, int SwirlIntensity);
void RenderPiano(wxXmlNode* model, int Style, int NumKeys, int NumRows, int KeyPlacement, bool Clipping,
                 const wxString& CueFilename, const wxString& MapFilename, const wxString& ShapeFilename); //added controls -DJ
void RenderPictures(wxXmlNode* model, int dir, const wxString& NewPictureName,int GifSpeed);
void RenderRipple(wxXmlNode* model, int Object_To_Draw, int Movement);
void RenderShimmer(wxXmlNode* model, int Duty_Factor,bool Use_All_Colors,bool Blink_Timing,int Blinks_Per_Row);
void SetP(int x, int y, wxImage::HSVValue hsv);
void Drawcircle(int xc,int yc,double radius, wxImage::HSVValue hsv);

void RenderSingleStrand(wxXmlNode* model, int ColorScheme,int Number_Chases,int Color_Mix1,int Chase_Spacing1,int Chase_Type1,
                        bool Chase_3dFade1, bool Chase_Group_All);
void draw_chase(int x,int y,wxImage::HSVValue hsv,int ColorScheme,int Number_Chases,int width,bool R_TO_L1,
                int Color_Mix1,bool Chase_Fade3d1,int ChaseDirection);
void pulse(int x,int y,wxImage::HSVValue hsv,float adjust_brightness);

void RenderSnowflakes(wxXmlNode* model, int Count, int SnowflakeType);
void RenderSnowstorm(wxXmlNode* model, int Count, int Length);
void RenderSpirals(wxXmlNode* model, int PaletteRepeat, int Direction, int Rotation, int Thickness,
                   bool Blend, bool Show3D, bool grow, bool shrink);
void RenderSpirograph(wxXmlNode* model, int R, int r, int d,bool Animate);
void RenderPinwheel(wxXmlNode* model, int R);


void RenderText(wxXmlNode* model, int Position1, const wxString& Line1, const wxString& FontString1,int dir1,bool center1,int TextRotation1,int Effect1,
                int Position2, const wxString& Line2, const wxString& FontString2,int dir2,bool center2,int TextRotation2,int Effect2,
                int Position3, const wxString& Line3, const wxString& FontString3,int dir3,bool center3,int TextRotation3,int Effect3,
                int Position4, const wxString& Line4, const wxString& FontString4,int dir4,bool center4,int TextRotation4,int Effect4);
void RenderTree(wxXmlNode* model, int Branches);
void RenderTwinkle(wxXmlNode* model, int Count,int Steps,bool Strobe);
void RenderWave(wxXmlNode* model, int WaveType,int FillColor,bool MirrorWave,int NumberWaves,int ThicknessWave,int WaveHeight, int WaveDirection);

