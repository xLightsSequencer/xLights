
void RenderFaces(const wxString &Phoneme, const wxString& eyes, bool face_outline);
void RenderCoroFacesFromPGO(const wxString& Phoneme, const wxString& eyes, bool face_outline);
void RenderFaces(SequenceElements *elements, const wxString &faceDefintion,
                 const wxString &Phoneme, const wxString &track, const wxString& eyes, bool face_outline);

void RenderFan(int center_x, int center_y, int start_radius, int end_radius, int start_angle, int revolutions,
               int duration, int acceleration, bool reverse_dir, bool blend_edges,
               int num_blades, int blade_width, int blade_angle, int num_elements, int element_width );
void RenderFire(int HeightPct,int HueShift,float GrowCycles, const wxString &location);
void RenderGalaxy(int center_x, int center_y, int start_radius, int end_radius, int start_angle, int revolutions,
                  int start_width, int end_width, int duration, int acceleration, bool reverse_dir, bool blend_edges, bool inward );
void RenderGarlands(int GarlandType, int Spacing, float cycles, const wxString &direction);
void RenderGlediator(const wxString& NewPictureName);

void RenderMarquee(int BandSize, int SkipSize, int Thickness, int stagger, int mSpeed, int mStart, bool reverse_dir,
                   int x_scale, int y_scale, int xc_adj, int yc_adj, bool pixelOffsets, bool wrap_x);
void RenderMeteors(const wxString & ColorScheme, int Count, int Length, const wxString & MeteorsEffect, int SwirlIntensity, int MSpeed);
void RenderMorph(int start_x1, int start_y1, int start_x2, int start_y2,
                 int end_x1,   int end_y1,   int end_x2,   int end_y2,
                 int start_length, int end_length, bool start_linked, bool end_linked,
                 int duration, int acceleration,
                 bool showEntireHeadAtStart, int repeat_count, int repeat_skip, int stagger );
void RenderPiano(const wxString & Style, int NumKeys, int NumRows, const wxString & KeyPlacement, bool Clipping,
                 const wxString& CueFilename, const wxString& MapFilename, const wxString& ShapeFilename); //added controls -DJ
void RenderPictures(const wxString & dir, const wxString& NewPictureName,float moveSpeed, float frameRateAdj,
                    int xc_adj, int yc_adj, int xce_adj, int yce_adj,
                    bool pixelOffsets, bool wrap_x);

void RenderRipple(const wxString &Object_To_Draw, const wxString &Movement, int Ripple_Thickness,int CheckBox_Ripple3D, float cycles);
void Drawsquare(int Movement, int x1, int x2, int y1,int y2,int Ripple_Thickness,int CheckBox_Ripple3D,wxImage::HSVValue hsv);
void Drawcircle(int Movement, int xc,int yc,double radius, wxImage::HSVValue hsv, int Ripple_Thickness,int CheckBox_Ripple3D);



void RenderShockwave(int center_x, int center_y, int start_radius, int end_radius,
                     int start_width, int end_width, int acceleration, bool blend_edges );


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







