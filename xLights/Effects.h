
void RenderBars(int PaletteRepeat, const wxString & Direction, bool Highlight, bool Show3D, float cycles);
void RenderOff(void);
void RenderOn(Effect *eff, int start, int end, bool shimmer, float cycles);
void RenderButterfly(int ColorScheme, int Style, int Chunks, int Skip,
                     int ButterflyDirection, int butterFlySpeed);
void RenderCircles(int number,int radius, bool bounce, bool collide, bool random,
                   bool radial, bool radial_3D, bool bubbles, int start_x, int start_y, bool plasma, bool fade,
                   int circleSpeed);
void RenderColorWash(Effect *eff,
                     bool HorizFade, bool VertFade, float cycles,
                     bool EntireModel, int x1, int y1, int x2, int y2,
                     bool shimmer,
                     bool circularPalette);
void RenderCurtain(int edge, int effect, int swag, bool repeat, float curtainSpeed);
void RenderDMX(int ch1, int ch2, int ch3, int ch4, int ch5, int ch6, int ch7, int ch8, int ch9, int ch10, int ch11, int ch12, int ch13, int ch14, int ch15,
               int ch1_ramp, int ch2_ramp, int ch3_ramp, int ch4_ramp, int ch5_ramp, int ch6_ramp,
               int ch7_ramp, int ch8_ramp, int ch9_ramp, int ch10_ramp, int ch11_ramp, int ch12_ramp,
               int ch13_ramp, int ch14_ramp, int ch15_ramp,
               bool use_ramps,const wxString& num_channels);

void RenderFaces(const wxString &Phoneme, const wxString& eyes, bool face_outline);
void RenderCoroFacesFromPGO(const wxString& Phoneme, const wxString& eyes, bool face_outline);
void RenderFaces(SequenceElements *elements, const wxString &faceDefintion,
                 const wxString &Phoneme, const wxString &track, const wxString& eyes, bool face_outline);

void RenderFan(int center_x, int center_y, int start_radius, int end_radius, int start_angle, int revolutions,
               int duration, int acceleration, bool reverse_dir, bool blend_edges,
               int num_blades, int blade_width, int blade_angle, int num_elements, int element_width );
void RenderFire(int HeightPct,int HueShift,float GrowCycles, const wxString &location);
void RenderFireworks(int Number_Explosions,int Count,float Velocity,int Fade);
void RenderGalaxy(int center_x, int center_y, int start_radius, int end_radius, int start_angle, int revolutions,
                  int start_width, int end_width, int duration, int acceleration, bool reverse_dir, bool blend_edges, bool inward );
void RenderGarlands(int GarlandType, int Spacing, float cycles, const wxString &direction);
void RenderGlediator(const wxString& NewPictureName);
void RenderLife(int Count, int Type, int lifeSpeed);
void RenderLightning(int Number_Bolts, int Number_Segments, bool ForkedLightning,
                     int topX,int topY, int botX, int botY);
void LightningDrawBolt(const int x0_, const int y0_, const int x1_, const int y1_,  xlColor& color, int curState );

void RenderMarquee(int BandSize, int SkipSize, int Thickness, int stagger, int mSpeed, int mStart, bool reverse_dir,
                   int x_scale, int y_scale, int xc_adj, int yc_adj, bool pixelOffsets, bool wrap_x);
void RenderMeteors(int ColorScheme, int Count, int Length, int MeteorsEffect, int SwirlIntensity, int MSpeed);
void RenderMorph(int start_x1, int start_y1, int start_x2, int start_y2,
                 int end_x1,   int end_y1,   int end_x2,   int end_y2,
                 int start_length, int end_length, bool start_linked, bool end_linked,
                 int duration, int acceleration,
                 bool showEntireHeadAtStart, int repeat_count, int repeat_skip, int stagger );
void RenderPiano(int Style, int NumKeys, int NumRows, int KeyPlacement, bool Clipping,
                 const wxString& CueFilename, const wxString& MapFilename, const wxString& ShapeFilename); //added controls -DJ
void RenderPictures(int dir, const wxString& NewPictureName,float moveSpeed, float frameRateAdj,
                    int xc_adj, int yc_adj, int xce_adj, int yce_adj,
                    bool pixelOffsets, bool wrap_x);
void RenderPinwheel(int pinwheel_arms, int pinwheel_twist,int pinwheel_thickness,
                    bool pinwheel_rotation,const wxString &pinwheel_3d,int xc_adj, int yc_adj, int pinwheel_armsize,
                    int pspeed);
void RenderPlasma(int ColorScheme, int Style, int Line_Density,int PlasmaDirection, int PlasmaSpeed);
void Draw_arm(int base_degrees,int max_radius,int pinwheel_twist, const xlColor &rgb,
              int xc_adj, int yc_adj);

void RenderRipple(int Object_To_Draw, int Movement, int Ripple_Thickness,int CheckBox_Ripple3D, float cycles);
void Drawsquare(int Movement, int x1, int x2, int y1,int y2,int Ripple_Thickness,int CheckBox_Ripple3D,wxImage::HSVValue hsv);
void Drawcircle(int Movement, int xc,int yc,double radius, wxImage::HSVValue hsv, int Ripple_Thickness,int CheckBox_Ripple3D);



void RenderShimmer(int Duty_Factor,bool Use_All_Colors, float cycles);
void RenderShockwave(int center_x, int center_y, int start_radius, int end_radius,
                     int start_width, int end_width, int acceleration, bool blend_edges );
void RenderStrobe(int Number_Strobes, int StrobeDuration,int Strobe_Type);



void RenderSingleStrandChase(const wxString &ColorScheme,int Number_Chases,int chaseSize,
                             const wxString &Chase_Type1,
                             bool Chase_3dFade1, bool Chase_Group_All,
                             float chaseSpeed);

void RenderSingleStrandSkips(Effect *eff, int Skips_BandSize, int Skips_SkipSize, int Skips_StartPos, const wxString& Skips_Direction, int advances);
void draw_chase(int x, bool group, int ColorScheme,int Number_Chases,bool autoReverse,int width,
                int Color_Mix1,bool Chase_Fade3d1,int ChaseDirection, bool mirror);

int possible_downward_moves(int x, int y);
void set_pixel_if_not_color(int x, int y, xlColor toColor, xlColor notColor, bool wrapx, bool wrapy);
void RenderSnowflakes(int Count, int SnowflakeType, int sSpeed, bool accumulate);
void RenderSnowstorm(int Count, int Length, int sSpeed);
void RenderSpirals(int PaletteRepeat, float Movement, int Rotation, int Thickness,
                   bool Blend, bool Show3D, bool grow, bool shrink);
void RenderSpirograph(int R, int r, int d, int Animate, int speed, int length);


void RenderText(const SettingsMap& SettingsMap);

void RenderTree(int Branches, int tspeed);
void RenderTwinkle(int Count,int Steps,bool Strobe, bool reRandomize);
void RenderWave(int WaveType,int FillColor,bool MirrorWave,int NumberWaves,int ThicknessWave,int WaveHeight, int WaveDirection, int wspeed);
