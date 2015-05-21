void RenderBars(int PaletteRepeat, int Direction, bool Highlight, bool Show3D);
void RenderOff(void);
void RenderOn(int start, int end, bool shimmer);
void RenderButterfly(int ColorScheme, int Style, int Chunks, int Skip, int ButterflyDirection);
void RenderCircles(int number,int radius, bool bounce, bool collide, bool random,
                   bool radial, bool radial_3D, bool bubbles, int start_x, int start_y, bool plasma, bool fade);
void RenderColorWash(bool HorizFade, bool VertFade, int RepeatCount,
                     bool EntireModel, int x1, int y1, int x2, int y2, bool shimmer);
void RenderCurtain(int edge, int effect, int swag, bool repeat);
void RenderFaces(int Phoneme);
//void RenderCoroFaces( int Phoneme, const wxString& x_y, const wxString& Outline_x_y, const wxString& Eyes_x_y/*, const wxString& parsed_xy*/);
void RenderCoroFaces(const wxString& Phoneme, const wxString& eyes, bool face_outline);
void RenderFan(int center_x, int center_y, int start_radius, int end_radius, int start_angle, int revolutions,
               int duration, int acceleration, bool reverse_dir, bool blend_edges,
               int num_blades, int blade_width, int blade_angle, int num_elements, int element_width );
void RenderFire(int HeightPct,int HueShift,bool GrowFire);
void RenderFireworks(int Number_Explosions,int Count,float Velocity,int Fade);
void RenderGalaxy(int center_x, int center_y, int start_radius, int end_radius, int start_angle, int revolutions,
                  int start_width, int end_width, int duration, int acceleration, bool reverse_dir, bool blend_edges, bool inward );
void RenderGarlands(int GarlandType, int Spacing);
void RenderGlediator(const wxString& NewPictureName);
void RenderLife(int Count, int Type);
void RenderMeteors(int ColorScheme, int Count, int Length, int MeteorsEffect, int SwirlIntensity);
void RenderMorph(int start_x1, int start_y1, int start_x2, int start_y2,
                 int end_x1,   int end_y1,   int end_x2,   int end_y2,
                 int start_length, int end_length, bool start_linked, bool end_linked,
                 int duration, int acceleration,
                 bool showEntireHeadAtStart, int repeat_count, int repeat_skip, int stagger );
void RenderPiano(int Style, int NumKeys, int NumRows, int KeyPlacement, bool Clipping,
                 const wxString& CueFilename, const wxString& MapFilename, const wxString& ShapeFilename); //added controls -DJ
void RenderPictures(int dir, const wxString& NewPictureName,int GifSpeed, bool is20fps,
                    int xc_adj, int yc_adj, int xce_adj, int yce_adj,
                    bool pixelOffsets, bool wrap_x);
void RenderPinwheel(int pinwheel_arms, int pinwheel_twist,int pinwheel_thickness,
                    bool pinwheel_rotation,const wxString &pinwheel_3d,int xc_adj, int yc_adj, int pinwheel_armsize);
void Draw_arm(int base_degrees,int max_radius,int pinwheel_twist, const xlColor &rgb,
              int xc_adj, int yc_adj);

void RenderRipple(int Object_To_Draw, int Movement, int Ripple_Thickness,int CheckBox_Ripple3D);
void Drawsquare(int Movement, int x1, int x2, int y1,int y2,int Ripple_Thickness,int CheckBox_Ripple3D,wxImage::HSVValue hsv);
void Drawcircle(int Movement, int xc,int yc,double radius, wxImage::HSVValue hsv, int Ripple_Thickness,int CheckBox_Ripple3D);



void RenderShimmer(int Duty_Factor,bool Use_All_Colors,bool Blink_Timing,int Blinks_Per_Row);
void RenderShockwave(int center_x, int center_y, int start_radius, int end_radius,
                     int start_width, int end_width, int acceleration, bool blend_edges );
void RenderStrobe(int Number_Strobes, int StrobeDuration,int Strobe_Type);



void RenderSingleStrandChase(int ColorScheme,int Number_Chases,int Color_Mix1,int Chase_Spacing1,int Chase_Type1,
                             bool Chase_3dFade1, bool Chase_Group_All);

void RenderSingleStrandSkips(int Skips_BandSize, int Skips_SkipSize, int Skips_StartPos, const wxString& Skips_Direction);
void draw_chase(int x,int y,wxImage::HSVValue hsv,int ColorScheme,int Number_Chases,int width,bool R_TO_L1,
                int Color_Mix1,bool Chase_Fade3d1,int ChaseDirection);
void pulse(int x,int y,wxImage::HSVValue hsv,float adjust_brightness);

void RenderSnowflakes(int Count, int SnowflakeType);
void RenderSnowstorm(int Count, int Length);
void RenderSpirals(int PaletteRepeat, int Direction, int Rotation, int Thickness,
                   bool Blend, bool Show3D, bool grow, bool shrink);
void RenderSpirograph(int R, int r, int d,bool Animate);


void RenderText(int Position1, const wxString& Line1, const wxString& FontString1,int dir1,bool center1,int TextRotation1,int Effect1,
                int Position2, const wxString& Line2, const wxString& FontString2,int dir2,bool center2,int TextRotation2,int Effect2,
                int Position3, const wxString& Line3, const wxString& FontString3,int dir3,bool center3,int TextRotation3,int Effect3,
                int Position4, const wxString& Line4, const wxString& FontString4,int dir4,bool center4,int TextRotation4,int Effect4);
void RenderTree(int Branches);
void RenderTwinkle(int Count,int Steps,bool Strobe);
void RenderWave(int WaveType,int FillColor,bool MirrorWave,int NumberWaves,int ThicknessWave,int WaveHeight, int WaveDirection);
