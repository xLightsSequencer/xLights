#include "EffectsPanel.h"
#include "../include/padlock16x16-green.xpm" //-DJ
#include "../include/padlock16x16-red.xpm" //-DJ
#include "../include/padlock16x16-blue.xpm" //-DJ

//(*InternalHeaders(EffectsPanel)
#include <wx/bitmap.h>
#include <wx/settings.h>
#include <wx/intl.h>
#include <wx/image.h>
#include <wx/string.h>
//*)

//(*IdInit(EffectsPanel)
const long EffectsPanel::ID_PANEL4 = wxNewId();
const long EffectsPanel::ID_STATICTEXT25 = wxNewId();
const long EffectsPanel::ID_SLIDER_Bars_BarCount = wxNewId();
const long EffectsPanel::ID_BITMAPBUTTON_SLIDER_Bars_BarCount = wxNewId();
const long EffectsPanel::ID_STATICTEXT26 = wxNewId();
const long EffectsPanel::ID_CHOICE_Bars_Direction = wxNewId();
const long EffectsPanel::ID_BITMAPBUTTON_CHOICE_Bars_Direction = wxNewId();
const long EffectsPanel::ID_CHECKBOX_Bars_Highlight = wxNewId();
const long EffectsPanel::ID_BITMAPBUTTON_CHECKBOX_Bars_Highlight = wxNewId();
const long EffectsPanel::ID_CHECKBOX_Bars_3D = wxNewId();
const long EffectsPanel::ID_BITMAPBUTTON_CHECKBOX_Bars_3D = wxNewId();
const long EffectsPanel::ID_PANEL8 = wxNewId();
const long EffectsPanel::ID_STATICTEXT27 = wxNewId();
const long EffectsPanel::ID_CHOICE_Butterfly_Colors = wxNewId();
const long EffectsPanel::ID_BITMAPBUTTON_CHOICE_Butterfly_Color = wxNewId();
const long EffectsPanel::ID_STATICTEXT28 = wxNewId();
const long EffectsPanel::ID_SLIDER_Butterfly_Style = wxNewId();
const long EffectsPanel::ID_BITMAPBUTTON_SLIDER_Butterfly_Style = wxNewId();
const long EffectsPanel::ID_STATICTEXT30 = wxNewId();
const long EffectsPanel::ID_SLIDER_Butterfly_Chunks = wxNewId();
const long EffectsPanel::ID_BITMAPBUTTON_SLIDER_Butterfly_Chunks = wxNewId();
const long EffectsPanel::ID_STATICTEXT31 = wxNewId();
const long EffectsPanel::ID_SLIDER_Butterfly_Skip = wxNewId();
const long EffectsPanel::ID_BITMAPBUTTON_SLIDER_Butterfly_Skip = wxNewId();
const long EffectsPanel::ID_STATICTEXT60 = wxNewId();
const long EffectsPanel::ID_CHOICE_Butterfly_Direction = wxNewId();
const long EffectsPanel::ID_BITMAPBUTTON9 = wxNewId();
const long EffectsPanel::ID_PANEL10 = wxNewId();
const long EffectsPanel::ID_STATICTEXT137 = wxNewId();
const long EffectsPanel::ID_SLIDER_Circles_Count = wxNewId();
const long EffectsPanel::ID_BITMAPBUTTON_SLIDER_Circles_Count = wxNewId();
const long EffectsPanel::ID_STATICTEXT138 = wxNewId();
const long EffectsPanel::ID_SLIDER_Circles_Size = wxNewId();
const long EffectsPanel::ID_BITMAPBUTTON_SLIDER_Circles_Size = wxNewId();
const long EffectsPanel::ID_CHECKBOX_Circles_Bounce = wxNewId();
const long EffectsPanel::ID_BITMAPBUTTON_CHECKBOX_Circles_Bounce = wxNewId();
const long EffectsPanel::ID_CHECKBOX_Circles_Collide = wxNewId();
const long EffectsPanel::ID_BITMAPBUTTON_CHECKBOX_Circles_Collide = wxNewId();
const long EffectsPanel::ID_CHECKBOX_Circles_Radial = wxNewId();
const long EffectsPanel::ID_BITMAPBUTTON_CHECKBOX_Circles_Radial = wxNewId();
const long EffectsPanel::ID_CHECKBOX_Circles_Plasma = wxNewId();
const long EffectsPanel::ID_BITMAPBUTTON_CHECKBOX_Circles_Plasma = wxNewId();
const long EffectsPanel::ID_CHECKBOX_Circles_Radial_3D = wxNewId();
const long EffectsPanel::ID_BITMAPBUTTON_CHECKBOX_Circles_Radial_3D = wxNewId();
const long EffectsPanel::ID_CHECKBOX_Circles_Random_m = wxNewId();
const long EffectsPanel::ID_BITMAPBUTTON_CHECKBOX_Circles_Random_m = wxNewId();
const long EffectsPanel::ID_PANEL37 = wxNewId();
const long EffectsPanel::ID_STATICTEXT21 = wxNewId();
const long EffectsPanel::ID_SLIDER_ColorWash_Count = wxNewId();
const long EffectsPanel::ID_BITMAPBUTTON_SLIDER_ColorWash_Count = wxNewId();
const long EffectsPanel::ID_STATICTEXT32 = wxNewId();
const long EffectsPanel::ID_CHECKBOX_ColorWash_HFade = wxNewId();
const long EffectsPanel::ID_BITMAPBUTTON_CHECKBOX_ColorWash_HFade = wxNewId();
const long EffectsPanel::ID_STATICTEXT33 = wxNewId();
const long EffectsPanel::ID_CHECKBOX_ColorWash_VFade = wxNewId();
const long EffectsPanel::ID_BITMAPBUTTON_CHECKBOX_ColorWash_VFade = wxNewId();
const long EffectsPanel::ID_PANEL5 = wxNewId();
const long EffectsPanel::ID_STATICTEXT6 = wxNewId();
const long EffectsPanel::ID_CHOICE_Curtain_Edge = wxNewId();
const long EffectsPanel::ID_BITMAPBUTTON_CHOICE_Curtain_Edge = wxNewId();
const long EffectsPanel::ID_STATICTEXT5 = wxNewId();
const long EffectsPanel::ID_CHOICE_Curtain_Effect = wxNewId();
const long EffectsPanel::ID_BITMAPBUTTON_CHOICE_Curtain_Effect = wxNewId();
const long EffectsPanel::ID_STATICTEXT7 = wxNewId();
const long EffectsPanel::ID_SLIDER_Curtain_Swag = wxNewId();
const long EffectsPanel::ID_BITMAPBUTTON_SLIDER_Curtain_Swag = wxNewId();
const long EffectsPanel::ID_CHECKBOX_Curtain_Repeat = wxNewId();
const long EffectsPanel::ID_BITMAPBUTTON_CHECKBOX_Curtain_Repeat = wxNewId();
const long EffectsPanel::ID_PANEL1 = wxNewId();
const long EffectsPanel::ID_STATICTEXT15 = wxNewId();
const long EffectsPanel::ID_CHOICE_Faces_Phoneme = wxNewId();
const long EffectsPanel::ID_PANEL6 = wxNewId();
const long EffectsPanel::ID_STATICTEXT84 = wxNewId();
const long EffectsPanel::ID_SLIDER_Fire_Height = wxNewId();
const long EffectsPanel::ID_BITMAPBUTTON_SLIDER_Fire_Height = wxNewId();
const long EffectsPanel::ID_STATICTEXT133 = wxNewId();
const long EffectsPanel::ID_SLIDER_Fire_HueShift = wxNewId();
const long EffectsPanel::ID_BITMAPBUTTON_SLIDER_Fire_HueShift = wxNewId();
const long EffectsPanel::ID_STATICTEXT135 = wxNewId();
const long EffectsPanel::ID_CHECKBOX_Fire_GrowFire = wxNewId();
const long EffectsPanel::ID_BITMAPBUTTON_CHECKBOX_Fire_GrowFire = wxNewId();
const long EffectsPanel::ID_PANEL12 = wxNewId();
const long EffectsPanel::ID_STATICTEXT95 = wxNewId();
const long EffectsPanel::ID_SLIDER_Fireworks_Number_Explosions = wxNewId();
const long EffectsPanel::ID_BITMAPBUTTON_SLIDER_Fireworks_Number_Explosions = wxNewId();
const long EffectsPanel::ID_STATICTEXT92 = wxNewId();
const long EffectsPanel::ID_SLIDER_Fireworks_Count = wxNewId();
const long EffectsPanel::ID_BITMAPBUTTON_SLIDER_Fireworks_Count = wxNewId();
const long EffectsPanel::ID_STATICTEXT94 = wxNewId();
const long EffectsPanel::ID_SLIDER_Fireworks_Velocity = wxNewId();
const long EffectsPanel::ID_BITMAPBUTTON_SLIDER_Fireworks_Velocity = wxNewId();
const long EffectsPanel::ID_STATICTEXT96 = wxNewId();
const long EffectsPanel::ID_SLIDER_Fireworks_Fade = wxNewId();
const long EffectsPanel::ID_BITMAPBUTTON_SLIDER_Fireworks_Fade = wxNewId();
const long EffectsPanel::ID_PANEL36 = wxNewId();
const long EffectsPanel::ID_STATICTEXT34 = wxNewId();
const long EffectsPanel::ID_SLIDER_Garlands_Type = wxNewId();
const long EffectsPanel::ID_BITMAPBUTTON_SLIDER_Garlands_Type = wxNewId();
const long EffectsPanel::ID_STATICTEXT35 = wxNewId();
const long EffectsPanel::ID_SLIDER_Garlands_Spacing = wxNewId();
const long EffectsPanel::ID_BITMAPBUTTON_SLIDER_Garlands_Spacing = wxNewId();
const long EffectsPanel::ID_PANEL14 = wxNewId();
const long EffectsPanel::ID_BUTTON3 = wxNewId();
const long EffectsPanel::ID_BITMAPBUTTON10 = wxNewId();
const long EffectsPanel::ID_TEXTCTRL_Glediator_Filename = wxNewId();
const long EffectsPanel::ID_PANEL17 = wxNewId();
const long EffectsPanel::ID_STATICTEXT36 = wxNewId();
const long EffectsPanel::ID_SLIDER_Life_Count = wxNewId();
const long EffectsPanel::ID_BITMAPBUTTON_SLIDER_Life_Count = wxNewId();
const long EffectsPanel::ID_STATICTEXT37 = wxNewId();
const long EffectsPanel::ID_SLIDER_Life_Seed = wxNewId();
const long EffectsPanel::ID_BITMAPBUTTON_SLIDER_Life_Seed = wxNewId();
const long EffectsPanel::ID_PANEL16 = wxNewId();
const long EffectsPanel::ID_STATICTEXT39 = wxNewId();
const long EffectsPanel::ID_CHOICE_Meteors_Type = wxNewId();
const long EffectsPanel::ID_BITMAPBUTTON_CHOICE_Meteors_Type = wxNewId();
const long EffectsPanel::ID_STATICTEXT129 = wxNewId();
const long EffectsPanel::ID_CHOICE_Meteors_Effect = wxNewId();
const long EffectsPanel::ID_BITMAPBUTTON_CHOICE_Meteors_Effect = wxNewId();
const long EffectsPanel::ID_STATICTEXT41 = wxNewId();
const long EffectsPanel::ID_SLIDER_Meteors_Count = wxNewId();
const long EffectsPanel::ID_BITMAPBUTTON_SLIDER_Meteors_Count = wxNewId();
const long EffectsPanel::ID_STATICTEXT43 = wxNewId();
const long EffectsPanel::ID_SLIDER_Meteors_Length = wxNewId();
const long EffectsPanel::ID_BITMAPBUTTON_SLIDER_Meteors_Length = wxNewId();
const long EffectsPanel::ID_STATICTEXT131 = wxNewId();
const long EffectsPanel::ID_SLIDER_Meteors_Swirl_Intensity = wxNewId();
const long EffectsPanel::ID_BITMAPBUTTON_SLIDER_Meteors_Swirl_Intensity = wxNewId();
const long EffectsPanel::ID_PANEL18 = wxNewId();
const long EffectsPanel::ID_STATICTEXT1 = wxNewId();
const long EffectsPanel::ID_CHOICE_Piano_Style = wxNewId();
const long EffectsPanel::ID_BITMAPBUTTON_SLIDER_Piano_Style = wxNewId();
const long EffectsPanel::ID_STATICTEXT_Piano_NumKeys = wxNewId();
const long EffectsPanel::ID_SLIDER_Piano_NumKeys = wxNewId();
const long EffectsPanel::ID_BITMAPBUTTON_Piano_NumKeys = wxNewId();
const long EffectsPanel::ID_STATICTEXT_Piano_NumRows = wxNewId();
const long EffectsPanel::ID_SLIDER_Piano_KeyWidth = wxNewId();
const long EffectsPanel::ID_BITMAPBUTTON_Piano_NumRows = wxNewId();
const long EffectsPanel::ID_STATICTEXT20 = wxNewId();
const long EffectsPanel::ID_CHOICE_PianoPlacement = wxNewId();
const long EffectsPanel::ID_BITMAPBUTTON_Piano_KeyPlacement = wxNewId();
const long EffectsPanel::ID_BUTTON_Piano_CueFilename = wxNewId();
const long EffectsPanel::ID_BITMAPBUTTON_Piano_Filename = wxNewId();
const long EffectsPanel::ID_STATICTEXT14 = wxNewId();
const long EffectsPanel::ID_TEXTCTRL_Piano_CueFilename = wxNewId();
const long EffectsPanel::ID_PANEL9 = wxNewId();
const long EffectsPanel::ID_BUTTON1 = wxNewId();
const long EffectsPanel::ID_BITMAPBUTTON_Piano_MapFilename = wxNewId();
const long EffectsPanel::ID_STATICTEXT13 = wxNewId();
const long EffectsPanel::ID_TEXTCTRL_Piano_MapFilename = wxNewId();
const long EffectsPanel::ID_PANEL11 = wxNewId();
const long EffectsPanel::ID_BUTTON2 = wxNewId();
const long EffectsPanel::ID_BITMAPBUTTON_Piano_ShapeFilename = wxNewId();
const long EffectsPanel::ID_CHECKBOX1 = wxNewId();
const long EffectsPanel::ID_BITMAPBUTTON_Piano_Clipping = wxNewId();
const long EffectsPanel::ID_TEXTCTRL_Piano_ShapeFilename = wxNewId();
const long EffectsPanel::ID_PANEL13 = wxNewId();
const long EffectsPanel::ID_NOTEBOOK2 = wxNewId();
const long EffectsPanel::ID_STATICTEXT4 = wxNewId();
const long EffectsPanel::ID_PANEL39 = wxNewId();
const long EffectsPanel::ID_BUTTON_PICTURES_FILENAME = wxNewId();
const long EffectsPanel::ID_BITMAPBUTTON_BUTTON_PICTURES_FILENAME = wxNewId();
const long EffectsPanel::ID_TEXTCTRL_Pictures_Filename = wxNewId();
const long EffectsPanel::ID_STATICTEXT46 = wxNewId();
const long EffectsPanel::ID_CHOICE_Pictures_Direction = wxNewId();
const long EffectsPanel::ID_BITMAPBUTTON_CHOICE_Pictures_Direction = wxNewId();
const long EffectsPanel::ID_STATICTEXT97 = wxNewId();
const long EffectsPanel::ID_SLIDER_Pictures_GifSpeed = wxNewId();
const long EffectsPanel::ID_BITMAPBUTTON_SLIDER_Pictures_GifSpeed = wxNewId();
const long EffectsPanel::ID_PANEL20 = wxNewId();
const long EffectsPanel::ID_STATICTEXT80 = wxNewId();
const long EffectsPanel::ID_SLIDER_Snowflakes_Count = wxNewId();
const long EffectsPanel::ID_BITMAPBUTTON_SLIDER_Snowflakes_Count = wxNewId();
const long EffectsPanel::ID_STATICTEXT81 = wxNewId();
const long EffectsPanel::ID_SLIDER_Snowflakes_Type = wxNewId();
const long EffectsPanel::ID_BITMAPBUTTON_SLIDER_Snowflakes_Type = wxNewId();
const long EffectsPanel::ID_PANEL22 = wxNewId();
const long EffectsPanel::ID_STATICTEXT45 = wxNewId();
const long EffectsPanel::ID_SLIDER_Snowstorm_Count = wxNewId();
const long EffectsPanel::ID_BITMAPBUTTON_SLIDER_Snowstorm_Count = wxNewId();
const long EffectsPanel::ID_STATICTEXT51 = wxNewId();
const long EffectsPanel::ID_SLIDER_Snowstorm_Length = wxNewId();
const long EffectsPanel::ID_BITMAPBUTTON_SLIDER_Snowstorm_Length = wxNewId();
const long EffectsPanel::ID_PANEL24 = wxNewId();
const long EffectsPanel::ID_STATICTEXT38 = wxNewId();
const long EffectsPanel::ID_SLIDER_Spirals_Count = wxNewId();
const long EffectsPanel::ID_BITMAPBUTTON_SLIDER_Spirals_Count = wxNewId();
const long EffectsPanel::ID_STATICTEXT40 = wxNewId();
const long EffectsPanel::ID_SLIDER_Spirals_Rotation = wxNewId();
const long EffectsPanel::ID_BITMAPBUTTON_SLIDER_Spirals_Rotation = wxNewId();
const long EffectsPanel::ID_STATICTEXT42 = wxNewId();
const long EffectsPanel::ID_SLIDER_Spirals_Thickness = wxNewId();
const long EffectsPanel::ID_BITMAPBUTTON_SLIDER_Spirals_Thickness = wxNewId();
const long EffectsPanel::ID_STATICTEXT44 = wxNewId();
const long EffectsPanel::ID_SLIDER_Spirals_Direction = wxNewId();
const long EffectsPanel::ID_BITMAPBUTTON_SLIDER_Spirals_Direction = wxNewId();
const long EffectsPanel::ID_CHECKBOX_Spirals_Blend = wxNewId();
const long EffectsPanel::ID_BITMAPBUTTON_CHECKBOX_Spirals_Blend = wxNewId();
const long EffectsPanel::ID_CHECKBOX_Spirals_3D = wxNewId();
const long EffectsPanel::ID_BITMAPBUTTON_CHECKBOX_Spirals_3D = wxNewId();
const long EffectsPanel::ID_CHECKBOX_Spirals_Grow = wxNewId();
const long EffectsPanel::ID_BITMAPBUTTON_CHECKBOX_Spirals_Grow = wxNewId();
const long EffectsPanel::ID_CHECKBOX_Spirals_Shrink = wxNewId();
const long EffectsPanel::ID_BITMAPBUTTON_CHECKBOX_Spirals_Shrink = wxNewId();
const long EffectsPanel::ID_PANEL26 = wxNewId();
const long EffectsPanel::ID_STATICTEXT88 = wxNewId();
const long EffectsPanel::ID_SLIDER_Spirograph_R = wxNewId();
const long EffectsPanel::ID_BITMAPBUTTON_SLIDER_Spirograph_R = wxNewId();
const long EffectsPanel::ID_STATICTEXT89 = wxNewId();
const long EffectsPanel::ID_SLIDER_Spirograph_r = wxNewId();
const long EffectsPanel::ID_BITMAPBUTTON_SLIDER_Spirograph_r = wxNewId();
const long EffectsPanel::ID_STATICTEXT90 = wxNewId();
const long EffectsPanel::ID_SLIDER_Spirograph_d = wxNewId();
const long EffectsPanel::ID_BITMAPBUTTON_SLIDER_Spirograph_d = wxNewId();
const long EffectsPanel::ID_STATICTEXT93 = wxNewId();
const long EffectsPanel::ID_CHECKBOX_Spirograph_Animate = wxNewId();
const long EffectsPanel::ID_BITMAPBUTTON_CHECKBOX_Spirograph_Animate = wxNewId();
const long EffectsPanel::ID_STATICTEXT91 = wxNewId();
const long EffectsPanel::ID_PANEL35 = wxNewId();
const long EffectsPanel::ID_STATICTEXT53 = wxNewId();
const long EffectsPanel::ID_TEXTCTRL_Text_Line1 = wxNewId();
const long EffectsPanel::ID_BITMAPBUTTON_TEXTCTRL_Text_Line1 = wxNewId();
const long EffectsPanel::ID_STATICTEXT54 = wxNewId();
const long EffectsPanel::ID_TEXTCTRL_Text_Line2 = wxNewId();
const long EffectsPanel::ID_BITMAPBUTTON_TEXTCTRL_Text_Line2 = wxNewId();
const long EffectsPanel::ID_STATICTEXT22 = wxNewId();
const long EffectsPanel::ID_TEXTCTRL_Text_Line3 = wxNewId();
const long EffectsPanel::ID_BITMAPBUTTON_TEXTCTRL_Text_Line3 = wxNewId();
const long EffectsPanel::ID_STATICTEXT23 = wxNewId();
const long EffectsPanel::ID_TEXTCTRL_Text_Line4 = wxNewId();
const long EffectsPanel::ID_BITMAPBUTTON_TEXTCTRL_Text_Line4 = wxNewId();
const long EffectsPanel::ID_BUTTON_TEXT_FONT1 = wxNewId();
const long EffectsPanel::ID_TEXTCTRL_Text_Font1 = wxNewId();
const long EffectsPanel::ID_BITMAPBUTTON_TEXTCTRL_Text_Font1 = wxNewId();
const long EffectsPanel::ID_STATICTEXT79 = wxNewId();
const long EffectsPanel::ID_CHOICE_Text_Dir1 = wxNewId();
const long EffectsPanel::ID_BITMAPBUTTON_CHOICE_Text_Dir1 = wxNewId();
const long EffectsPanel::ID_CHECKBOX_TextToCenter1 = wxNewId();
const long EffectsPanel::ID_BITMAPBUTTON_TextToCenter1 = wxNewId();
const long EffectsPanel::ID_STATICTEXT108 = wxNewId();
const long EffectsPanel::ID_CHOICE_Text_Effect1 = wxNewId();
const long EffectsPanel::ID_BITMAPBUTTON_CHOICE_Text_Effect1 = wxNewId();
const long EffectsPanel::ID_STATICTEXT99 = wxNewId();
const long EffectsPanel::ID_CHOICE_Text_Count1 = wxNewId();
const long EffectsPanel::ID_BITMAPBUTTON_CHOICE_Text_Count1 = wxNewId();
const long EffectsPanel::ID_STATICTEXT101 = wxNewId();
const long EffectsPanel::ID_SLIDER_Text_Position1 = wxNewId();
const long EffectsPanel::ID_BITMAPBUTTON_SLIDER_Text_Position1 = wxNewId();
const long EffectsPanel::ID_PANEL_Text1 = wxNewId();
const long EffectsPanel::ID_BUTTON_TEXT_FONT2 = wxNewId();
const long EffectsPanel::ID_TEXTCTRL_Text_Font2 = wxNewId();
const long EffectsPanel::ID_BITMAPBUTTON_TEXTCTRL_Text_Font2 = wxNewId();
const long EffectsPanel::ID_STATICTEXT57 = wxNewId();
const long EffectsPanel::ID_CHOICE_Text_Dir2 = wxNewId();
const long EffectsPanel::ID_BITMAPBUTTON_CHOICE_Text_Dir2 = wxNewId();
const long EffectsPanel::ID_CHECKBOX_TextToCenter2 = wxNewId();
const long EffectsPanel::ID_BITMAPBUTTON_Text2Center2 = wxNewId();
const long EffectsPanel::ID_STATICTEXT109 = wxNewId();
const long EffectsPanel::ID_CHOICE_Text_Effect2 = wxNewId();
const long EffectsPanel::ID_BITMAPBUTTON_CHOICE_Text_Effect2 = wxNewId();
const long EffectsPanel::ID_STATICTEXT104 = wxNewId();
const long EffectsPanel::ID_CHOICE_Text_Count2 = wxNewId();
const long EffectsPanel::ID_BITMAPBUTTON_CHOICE_Text_Count2 = wxNewId();
const long EffectsPanel::ID_STATICTEXT103 = wxNewId();
const long EffectsPanel::ID_SLIDER_Text_Position2 = wxNewId();
const long EffectsPanel::ID_BITMAPBUTTON_SLIDER_Text_Position2 = wxNewId();
const long EffectsPanel::ID_PANEL_Text2 = wxNewId();
const long EffectsPanel::ID_BUTTON_TEXT_FONT3 = wxNewId();
const long EffectsPanel::ID_TEXTCTRL_Text_Font3 = wxNewId();
const long EffectsPanel::ID_BITMAPBUTTON_TEXTCTRL_Text_Font3 = wxNewId();
const long EffectsPanel::ID_STATICTEXT16 = wxNewId();
const long EffectsPanel::ID_CHOICE_Text_Dir3 = wxNewId();
const long EffectsPanel::ID_BITMAPBUTTON_CHOICE_Text_Dir3 = wxNewId();
const long EffectsPanel::ID_CHECKBOX_TextToCenter3 = wxNewId();
const long EffectsPanel::ID_BITMAPBUTTON_TextToCenter3 = wxNewId();
const long EffectsPanel::ID_STATICTEXT17 = wxNewId();
const long EffectsPanel::ID_CHOICE_Text_Effect3 = wxNewId();
const long EffectsPanel::ID_BITMAPBUTTON_CHOICE_Text_Effect3 = wxNewId();
const long EffectsPanel::ID_STATICTEXT18 = wxNewId();
const long EffectsPanel::ID_CHOICE_Text_Count3 = wxNewId();
const long EffectsPanel::ID_BITMAPBUTTON_CHOICE_Text_Count3 = wxNewId();
const long EffectsPanel::ID_STATICTEXT19 = wxNewId();
const long EffectsPanel::ID_SLIDER_Text_Position3 = wxNewId();
const long EffectsPanel::ID_BITMAPBUTTON_SLIDER_Text_Position3 = wxNewId();
const long EffectsPanel::ID_PANEL_Text3 = wxNewId();
const long EffectsPanel::ID_BUTTON_TEXT_FONT4 = wxNewId();
const long EffectsPanel::ID_TEXTCTRL_Text_Font4 = wxNewId();
const long EffectsPanel::ID_BITMAPBUTTON_TEXTCTRL_Text_Font4 = wxNewId();
const long EffectsPanel::ID_STATICTEXT24 = wxNewId();
const long EffectsPanel::ID_CHOICE_Text_Dir4 = wxNewId();
const long EffectsPanel::ID_BITMAPBUTTON_CHOICE_Text_Dir4 = wxNewId();
const long EffectsPanel::ID_CHECKBOX_TextToCenter4 = wxNewId();
const long EffectsPanel::ID_BITMAPBUTTON_TextToCenter4 = wxNewId();
const long EffectsPanel::ID_STATICTEXT29 = wxNewId();
const long EffectsPanel::ID_CHOICE_Text_Effect4 = wxNewId();
const long EffectsPanel::ID_BITMAPBUTTON_CHOICE_Text_Effect4 = wxNewId();
const long EffectsPanel::ID_STATICTEXT47 = wxNewId();
const long EffectsPanel::ID_CHOICE_Text_Count4 = wxNewId();
const long EffectsPanel::ID_BITMAPBUTTON_CHOICE_Text_Count4 = wxNewId();
const long EffectsPanel::ID_STATICTEXT48 = wxNewId();
const long EffectsPanel::ID_SLIDER_Text_Position4 = wxNewId();
const long EffectsPanel::ID_BITMAPBUTTON_SLIDER_Text_Position4 = wxNewId();
const long EffectsPanel::ID_PANEL_Text4 = wxNewId();
const long EffectsPanel::ID_NOTEBOOK_Text1 = wxNewId();
const long EffectsPanel::ID_PANEL7 = wxNewId();
const long EffectsPanel::ID_STATICTEXT87 = wxNewId();
const long EffectsPanel::ID_SLIDER_Tree_Branches = wxNewId();
const long EffectsPanel::ID_BITMAPBUTTON_SLIDER_Tree_Branches = wxNewId();
const long EffectsPanel::ID_PANEL34 = wxNewId();
const long EffectsPanel::ID_STATICTEXT86 = wxNewId();
const long EffectsPanel::ID_SLIDER_Twinkle_Count = wxNewId();
const long EffectsPanel::ID_BITMAPBUTTON_SLIDER_Twinkle_Count = wxNewId();
const long EffectsPanel::ID_STATICTEXT105 = wxNewId();
const long EffectsPanel::ID_SLIDER_Twinkle_Steps = wxNewId();
const long EffectsPanel::ID_BITMAPBUTTON_SLIDER_Twinkle_Steps = wxNewId();
const long EffectsPanel::ID_CHECKBOX_Twinkle_Strobe = wxNewId();
const long EffectsPanel::ID_BITMAPBUTTON_CHECKBOX_Twinkle_Strobe = wxNewId();
const long EffectsPanel::ID_PANEL33 = wxNewId();
const long EffectsPanel::ID_STATICTEXT63 = wxNewId();
const long EffectsPanel::ID_CHOICE_SingleStrand_Colors = wxNewId();
const long EffectsPanel::ID_BITMAPBUTTON_SingleStrand_Colors = wxNewId();
const long EffectsPanel::ID_STATICTEXT62 = wxNewId();
const long EffectsPanel::ID_SLIDER_Number_Chases = wxNewId();
const long EffectsPanel::ID_BITMAPBUTTON_Number_Chases = wxNewId();
const long EffectsPanel::ID_STATICTEXT8 = wxNewId();
const long EffectsPanel::ID_SLIDER_Color_Mix1 = wxNewId();
const long EffectsPanel::ID_BITMAPBUTTON_SLIDER_Color_Mix1 = wxNewId();
const long EffectsPanel::ID_STATICTEXT9 = wxNewId();
const long EffectsPanel::ID_SLIDER_Chase_Spacing1 = wxNewId();
const long EffectsPanel::ID_BITMAPBUTTON_SLIDER_Chase_Spacing1 = wxNewId();
const long EffectsPanel::ID_STATICTEXT10 = wxNewId();
const long EffectsPanel::ID_CHOICE_Chase_Type1 = wxNewId();
const long EffectsPanel::ID_BITMAPBUTTON_CHOICE_Chase_Type1 = wxNewId();
const long EffectsPanel::ID_STATICTEXT11 = wxNewId();
const long EffectsPanel::ID_CHECKBOX_Chase_3dFade1 = wxNewId();
const long EffectsPanel::ID_BITMAPBUTTON_CHECKBOX_Chase_3dFade1 = wxNewId();
const long EffectsPanel::ID_STATICTEXT12 = wxNewId();
const long EffectsPanel::ID_CHECKBOX_Chase_Group_All = wxNewId();
const long EffectsPanel::ID_BITMAPBUTTON_Chase_Group_All = wxNewId();
const long EffectsPanel::ID_PANEL3 = wxNewId();
const long EffectsPanel::ID_NOTEBOOK1 = wxNewId();
const long EffectsPanel::ID_PANEL2 = wxNewId();
const long EffectsPanel::ID_STATICTEXT49 = wxNewId();
const long EffectsPanel::ID_CHOICE_Wave_Type = wxNewId();
const long EffectsPanel::ID_BITMAPBUTTON2 = wxNewId();
const long EffectsPanel::ID_STATICTEXT50 = wxNewId();
const long EffectsPanel::ID_CHOICE_Fill_Colors = wxNewId();
const long EffectsPanel::ID_BITMAPBUTTON3 = wxNewId();
const long EffectsPanel::ID_STATICTEXT52 = wxNewId();
const long EffectsPanel::ID_CHECKBOX_Mirror_Wave = wxNewId();
const long EffectsPanel::ID_BITMAPBUTTON4 = wxNewId();
const long EffectsPanel::ID_STATICTEXT55 = wxNewId();
const long EffectsPanel::ID_SLIDER_Number_Waves = wxNewId();
const long EffectsPanel::ID_BITMAPBUTTON5 = wxNewId();
const long EffectsPanel::ID_STATICTEXT56 = wxNewId();
const long EffectsPanel::ID_SLIDER_Thickness_Percentage = wxNewId();
const long EffectsPanel::ID_BITMAPBUTTON6 = wxNewId();
const long EffectsPanel::ID_STATICTEXT58 = wxNewId();
const long EffectsPanel::ID_SLIDER_Wave_Height = wxNewId();
const long EffectsPanel::ID_BITMAPBUTTON7 = wxNewId();
const long EffectsPanel::ID_STATICTEXT59 = wxNewId();
const long EffectsPanel::ID_CHOICE_Wave_Direction = wxNewId();
const long EffectsPanel::ID_BITMAPBUTTON8 = wxNewId();
const long EffectsPanel::ID_PANEL15 = wxNewId();
const long EffectsPanel::ID_STATICTEXT73 = wxNewId();
const long EffectsPanel::ID_CHOICE_CoroFaces_Phoneme = wxNewId();
const long EffectsPanel::ID_STATICTEXT74 = wxNewId();
const long EffectsPanel::ID_TEXTCTRL_X_Y = wxNewId();
const long EffectsPanel::ID_STATICTEXT75 = wxNewId();
const long EffectsPanel::ID_TEXTCTRL_Outline_X_Y = wxNewId();
const long EffectsPanel::ID_STATICTEXT76 = wxNewId();
const long EffectsPanel::ID_TEXTCTRL_Eyes_X_Y = wxNewId();
const long EffectsPanel::ID_PANEL28 = wxNewId();
const long EffectsPanel::ID_CHOICEBOOK1 = wxNewId();
const long EffectsPanel::ID_BITMAPBUTTON_CHOICEBOOK1 = wxNewId();
const long EffectsPanel::ID_BITMAPBUTTON87 = wxNewId();
const long EffectsPanel::ID_BITMAPBUTTON1 = wxNewId();
const long EffectsPanel::ID_BITMAPBUTTON88 = wxNewId();
const long EffectsPanel::ID_CHECKBOX_Palette1 = wxNewId();
const long EffectsPanel::ID_BUTTON_Palette1 = wxNewId();
const long EffectsPanel::ID_BITMAPBUTTON_BUTTON_Palette1 = wxNewId();
const long EffectsPanel::ID_CHECKBOX_Palette2 = wxNewId();
const long EffectsPanel::ID_BUTTON_Palette2 = wxNewId();
const long EffectsPanel::ID_BITMAPBUTTON_BUTTON_Palette2 = wxNewId();
const long EffectsPanel::ID_CHECKBOX_Palette3 = wxNewId();
const long EffectsPanel::ID_BUTTON_Palette3 = wxNewId();
const long EffectsPanel::ID_BITMAPBUTTON_BUTTON_Palette3 = wxNewId();
const long EffectsPanel::ID_CHECKBOX_Palette4 = wxNewId();
const long EffectsPanel::ID_BUTTON_Palette4 = wxNewId();
const long EffectsPanel::ID_BITMAPBUTTON_BUTTON_Palette4 = wxNewId();
const long EffectsPanel::ID_CHECKBOX_Palette5 = wxNewId();
const long EffectsPanel::ID_BUTTON_Palette5 = wxNewId();
const long EffectsPanel::ID_BITMAPBUTTON_BUTTON_Palette5 = wxNewId();
const long EffectsPanel::ID_CHECKBOX_Palette6 = wxNewId();
const long EffectsPanel::ID_BUTTON_Palette6 = wxNewId();
const long EffectsPanel::ID_BITMAPBUTTON_BUTTON_Palette6 = wxNewId();
const long EffectsPanel::ID_STATICTEXT61 = wxNewId();
const long EffectsPanel::ID_SLIDER_Speed = wxNewId();
const long EffectsPanel::ID_BITMAPBUTTON_SLIDER_Speed = wxNewId();
const long EffectsPanel::ID_TEXTCTRL_Speed = wxNewId();
const long EffectsPanel::ID_STATICTEXT2 = wxNewId();
const long EffectsPanel::ID_TEXTCTRL_Fadein = wxNewId();
const long EffectsPanel::ID_BITMAPBUTTON_TEXTCTRL_Fadein = wxNewId();
const long EffectsPanel::ID_STATICTEXT3 = wxNewId();
const long EffectsPanel::ID_TEXTCTRL_Fadeout = wxNewId();
const long EffectsPanel::ID_BITMAPBUTTON_TEXTCTRL_Fadeout = wxNewId();
const long EffectsPanel::ID_CHECKBOX_FitToTime = wxNewId();
const long EffectsPanel::ID_BITMAPBUTTON_CHECKBOX_FitToTime = wxNewId();
const long EffectsPanel::ID_CHECKBOX_OverlayBkg = wxNewId();
const long EffectsPanel::ID_BITMAPBUTTON_OverlayBkg = wxNewId();
//*)

BEGIN_EVENT_TABLE(EffectsPanel,wxPanel)
    //(*EventTable(EffectsPanel)
    //*)
END_EVENT_TABLE()

EffectsPanel::EffectsPanel(wxWindow *parent, wxWindowID id, const wxPoint &pos, const wxSize &size, long style, const wxString &name)
{
    //(*Initialize(EffectsPanel)
    wxFlexGridSizer* FlexGridSizer4;
    wxFlexGridSizer* FlexGridSizer71;
    wxFlexGridSizer* FlexGridSizer45;
    wxFlexGridSizer* FlexGridSizer47;
    wxFlexGridSizer* FlexGridSizer54;
    wxFlexGridSizer* FlexGridSizer16;
    wxFlexGridSizer* FlexGridSizer73;
    wxFlexGridSizer* FlexGridSizer24;
    wxFlexGridSizer* FlexGridSizer19;
    wxFlexGridSizer* FlexGridSizer23;
    wxFlexGridSizer* FlexGridSizer38;
    wxFlexGridSizer* FlexGridSizer41;
    wxFlexGridSizer* FlexGridSizer51;
    wxFlexGridSizer* FlexGridSizer10;
    wxFlexGridSizer* FlexGridSizer3;
    wxFlexGridSizer* FlexGridSizer80;
    wxFlexGridSizer* FlexGridSizer27;
    wxFlexGridSizer* FlexGridSizer44;
    wxFlexGridSizer* FlexGridSizer37;
    wxFlexGridSizer* FlexGridSizer25;
    wxFlexGridSizer* FlexGridSizer5;
    wxFlexGridSizer* FlexGridSizer22;
    wxFlexGridSizer* FlexGridSizer9;
    wxFlexGridSizer* FlexGridSizer2;
    wxFlexGridSizer* FlexGridSizer69;
    wxFlexGridSizer* FlexGridSizer53;
    wxFlexGridSizer* FlexGridSizer7;
    wxFlexGridSizer* FlexGridSizer74;
    wxFlexGridSizer* FlexGridSizer52;
    wxFlexGridSizer* FlexGridSizer29;
    wxFlexGridSizer* FlexGridSizer34;
    wxFlexGridSizer* FlexGridSizer49;
    wxFlexGridSizer* FlexGridSizer15;
    wxFlexGridSizer* FlexGridSizer18;
    wxFlexGridSizer* FlexGridSizer8;
    wxFlexGridSizer* FlexGridSizer21;
    wxFlexGridSizer* FlexGridSizer14;
    wxFlexGridSizer* FlexGridSizer20;
    wxFlexGridSizer* FlexGridSizer50;
    wxFlexGridSizer* FlexGridSizer13;
    wxFlexGridSizer* FlexGridSizer12;
    wxFlexGridSizer* FlexGridSizer35;
    wxFlexGridSizer* FlexGridSizer36;
    wxFlexGridSizer* FlexGridSizer6;
    wxFlexGridSizer* FlexGridSizer46;
    wxFlexGridSizer* FlexGridSizer48;
    wxFlexGridSizer* FlexGridSizer1;
    wxFlexGridSizer* FlexGridSizer33;
    wxFlexGridSizer* FlexGridSizer43;
    wxFlexGridSizer* FlexGridSizer11;
    wxFlexGridSizer* FlexGridSizer17;
    wxFlexGridSizer* FlexGridSizer32;
    wxFlexGridSizer* FlexGridSizer65;
    wxFlexGridSizer* FlexGridSizer42;
    wxFlexGridSizer* FlexGridSizer58;
    wxFlexGridSizer* FlexGridSizer72;
    wxFlexGridSizer* FlexGridSizer31;
    wxFlexGridSizer* FlexGridSizer40;
    wxFlexGridSizer* FlexGridSizer39;
    wxFlexGridSizer* FlexGridSizer28;
    wxFlexGridSizer* FlexGridSizer26;
    wxFlexGridSizer* FlexGridSizer30;

    Create(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("wxID_ANY"));
    FlexGridSizer1 = new wxFlexGridSizer(0, 2, 0, 0);
    Choicebook1 = new wxChoicebook(this, ID_CHOICEBOOK1, wxDefaultPosition, wxDefaultSize, 0, _T("ID_CHOICEBOOK1"));
    Panel1_None = new wxPanel(Choicebook1, ID_PANEL4, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("ID_PANEL4"));
    Panel1_Bars = new wxPanel(Choicebook1, ID_PANEL8, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("ID_PANEL8"));
    FlexGridSizer35 = new wxFlexGridSizer(0, 3, 0, 0);
    FlexGridSizer35->AddGrowableCol(1);
    StaticText23 = new wxStaticText(Panel1_Bars, ID_STATICTEXT25, _("Palette Rep"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT25"));
    FlexGridSizer35->Add(StaticText23, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
    Slider_Bars_BarCount = new wxSlider(Panel1_Bars, ID_SLIDER_Bars_BarCount, 1, 1, 5, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_Bars_BarCount"));
    FlexGridSizer35->Add(Slider_Bars_BarCount, 1, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
    BitmapButton_PaletteRep = new wxBitmapButton(Panel1_Bars, ID_BITMAPBUTTON_SLIDER_Bars_BarCount, padlock16x16_blue_xpm, wxDefaultPosition, wxSize(20,20), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON_SLIDER_Bars_BarCount"));
    BitmapButton_PaletteRep->SetDefault();
    BitmapButton_PaletteRep->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_ACTIVECAPTION));
    FlexGridSizer35->Add(BitmapButton_PaletteRep, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 1);
    StaticText24 = new wxStaticText(Panel1_Bars, ID_STATICTEXT26, _("Direction"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT26"));
    FlexGridSizer35->Add(StaticText24, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
    Choice_Bars_Direction = new wxChoice(Panel1_Bars, ID_CHOICE_Bars_Direction, wxDefaultPosition, wxDefaultSize, 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE_Bars_Direction"));
    FlexGridSizer35->Add(Choice_Bars_Direction, 1, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
    BitmapButton_Direction = new wxBitmapButton(Panel1_Bars, ID_BITMAPBUTTON_CHOICE_Bars_Direction, padlock16x16_blue_xpm, wxDefaultPosition, wxSize(20,20), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON_CHOICE_Bars_Direction"));
    BitmapButton_Direction->SetDefault();
    BitmapButton_Direction->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_ACTIVECAPTION));
    FlexGridSizer35->Add(BitmapButton_Direction, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 1);
    FlexGridSizer35->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizer10 = new wxFlexGridSizer(0, 4, 0, 0);
    CheckBox_Bars_Highlight = new wxCheckBox(Panel1_Bars, ID_CHECKBOX_Bars_Highlight, _("Highlight"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX_Bars_Highlight"));
    CheckBox_Bars_Highlight->SetValue(false);
    FlexGridSizer10->Add(CheckBox_Bars_Highlight, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
    BitmapButton_Highlight = new wxBitmapButton(Panel1_Bars, ID_BITMAPBUTTON_CHECKBOX_Bars_Highlight, padlock16x16_blue_xpm, wxDefaultPosition, wxSize(22,21), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON_CHECKBOX_Bars_Highlight"));
    BitmapButton_Highlight->SetDefault();
    FlexGridSizer10->Add(BitmapButton_Highlight, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 1);
    FlexGridSizer10->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    CheckBox_Bars_3D = new wxCheckBox(Panel1_Bars, ID_CHECKBOX_Bars_3D, _("3D"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX_Bars_3D"));
    CheckBox_Bars_3D->SetValue(false);
    FlexGridSizer10->Add(CheckBox_Bars_3D, 1, wxALL|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 2);
    FlexGridSizer35->Add(FlexGridSizer10, 1, wxALL|wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    BitmapButton_3D = new wxBitmapButton(Panel1_Bars, ID_BITMAPBUTTON_CHECKBOX_Bars_3D, padlock16x16_blue_xpm, wxDefaultPosition, wxSize(20,20), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON_CHECKBOX_Bars_3D"));
    BitmapButton_3D->SetDefault();
    BitmapButton_3D->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_ACTIVECAPTION));
    FlexGridSizer35->Add(BitmapButton_3D, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 1);
    Panel1_Bars->SetSizer(FlexGridSizer35);
    FlexGridSizer35->Fit(Panel1_Bars);
    FlexGridSizer35->SetSizeHints(Panel1_Bars);
    Panel1_Butterfly = new wxPanel(Choicebook1, ID_PANEL10, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("ID_PANEL10"));
    FlexGridSizer36 = new wxFlexGridSizer(0, 3, 0, 0);
    FlexGridSizer36->AddGrowableCol(1);
    StaticText25 = new wxStaticText(Panel1_Butterfly, ID_STATICTEXT27, _("Colors"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT27"));
    FlexGridSizer36->Add(StaticText25, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
    Choice_Butterfly_Colors = new wxChoice(Panel1_Butterfly, ID_CHOICE_Butterfly_Colors, wxDefaultPosition, wxDefaultSize, 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE_Butterfly_Colors"));
    FlexGridSizer36->Add(Choice_Butterfly_Colors, 1, wxTOP|wxBOTTOM|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    BitmapButton_ButterflyColors = new wxBitmapButton(Panel1_Butterfly, ID_BITMAPBUTTON_CHOICE_Butterfly_Color, padlock16x16_blue_xpm, wxDefaultPosition, wxSize(20,20), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON_CHOICE_Butterfly_Color"));
    BitmapButton_ButterflyColors->SetDefault();
    BitmapButton_ButterflyColors->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_ACTIVECAPTION));
    FlexGridSizer36->Add(BitmapButton_ButterflyColors, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 1);
    StaticText26 = new wxStaticText(Panel1_Butterfly, ID_STATICTEXT28, _("Style"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT28"));
    FlexGridSizer36->Add(StaticText26, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
    Slider_Butterfly_Style = new wxSlider(Panel1_Butterfly, ID_SLIDER_Butterfly_Style, 1, 1, 5, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_Butterfly_Style"));
    FlexGridSizer36->Add(Slider_Butterfly_Style, 1, wxALL|wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
    BitmapButton_ButterflyStyle = new wxBitmapButton(Panel1_Butterfly, ID_BITMAPBUTTON_SLIDER_Butterfly_Style, padlock16x16_blue_xpm, wxDefaultPosition, wxSize(20,20), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON_SLIDER_Butterfly_Style"));
    BitmapButton_ButterflyStyle->SetDefault();
    BitmapButton_ButterflyStyle->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_ACTIVECAPTION));
    FlexGridSizer36->Add(BitmapButton_ButterflyStyle, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 1);
    StaticText27 = new wxStaticText(Panel1_Butterfly, ID_STATICTEXT30, _("Bkgrd Chunks"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT30"));
    FlexGridSizer36->Add(StaticText27, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
    Slider_Butterfly_Chunks = new wxSlider(Panel1_Butterfly, ID_SLIDER_Butterfly_Chunks, 1, 1, 10, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_Butterfly_Chunks"));
    FlexGridSizer36->Add(Slider_Butterfly_Chunks, 1, wxALL|wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
    BitmapButton_ButterflyChunks = new wxBitmapButton(Panel1_Butterfly, ID_BITMAPBUTTON_SLIDER_Butterfly_Chunks, padlock16x16_blue_xpm, wxDefaultPosition, wxSize(20,20), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON_SLIDER_Butterfly_Chunks"));
    BitmapButton_ButterflyChunks->SetDefault();
    BitmapButton_ButterflyChunks->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_ACTIVECAPTION));
    FlexGridSizer36->Add(BitmapButton_ButterflyChunks, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 1);
    StaticText28 = new wxStaticText(Panel1_Butterfly, ID_STATICTEXT31, _("Bkgrd Skip"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT31"));
    FlexGridSizer36->Add(StaticText28, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
    Slider_Butterfly_Skip = new wxSlider(Panel1_Butterfly, ID_SLIDER_Butterfly_Skip, 2, 2, 10, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_Butterfly_Skip"));
    FlexGridSizer36->Add(Slider_Butterfly_Skip, 1, wxALL|wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
    BitmapButton_ButterflySkip = new wxBitmapButton(Panel1_Butterfly, ID_BITMAPBUTTON_SLIDER_Butterfly_Skip, padlock16x16_blue_xpm, wxDefaultPosition, wxSize(20,20), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON_SLIDER_Butterfly_Skip"));
    BitmapButton_ButterflySkip->SetDefault();
    BitmapButton_ButterflySkip->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_ACTIVECAPTION));
    FlexGridSizer36->Add(BitmapButton_ButterflySkip, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 1);
    StaticText58 = new wxStaticText(Panel1_Butterfly, ID_STATICTEXT60, _("Direction"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT60"));
    FlexGridSizer36->Add(StaticText58, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    Choice_Butterfly_Direction = new wxChoice(Panel1_Butterfly, ID_CHOICE_Butterfly_Direction, wxDefaultPosition, wxDefaultSize, 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE_Butterfly_Direction"));
    FlexGridSizer36->Add(Choice_Butterfly_Direction, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    BitmapButton8 = new wxBitmapButton(Panel1_Butterfly, ID_BITMAPBUTTON9, padlock16x16_blue_xpm, wxDefaultPosition, wxSize(20,20), wxBU_AUTODRAW, wxDefaultValidator, _T("ID_BITMAPBUTTON9"));
    BitmapButton8->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_ACTIVECAPTION));
    FlexGridSizer36->Add(BitmapButton8, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    Panel1_Butterfly->SetSizer(FlexGridSizer36);
    FlexGridSizer36->Fit(Panel1_Butterfly);
    FlexGridSizer36->SetSizeHints(Panel1_Butterfly);
    Panel1_Circles = new wxPanel(Choicebook1, ID_PANEL37, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("ID_PANEL37"));
    FlexGridSizer80 = new wxFlexGridSizer(0, 2, 0, 0);
    StaticText136 = new wxStaticText(Panel1_Circles, ID_STATICTEXT137, _("Number of Circles"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT137"));
    FlexGridSizer80->Add(StaticText136, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizer17 = new wxFlexGridSizer(0, 3, 0, 0);
    Slider_Circles_Count = new wxSlider(Panel1_Circles, ID_SLIDER_Circles_Count, 3, 1, 10, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_Circles_Count"));
    FlexGridSizer17->Add(Slider_Circles_Count, 1, wxALL|wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    BitmapButton_CirclesCount = new wxBitmapButton(Panel1_Circles, ID_BITMAPBUTTON_SLIDER_Circles_Count, padlock16x16_blue_xpm, wxDefaultPosition, wxSize(22,21), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON_SLIDER_Circles_Count"));
    BitmapButton_CirclesCount->SetDefault();
    FlexGridSizer17->Add(BitmapButton_CirclesCount, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 1);
    FlexGridSizer80->Add(FlexGridSizer17, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    StaticText137 = new wxStaticText(Panel1_Circles, ID_STATICTEXT138, _("Size"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT138"));
    FlexGridSizer80->Add(StaticText137, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizer18 = new wxFlexGridSizer(0, 3, 0, 0);
    Slider_Circles_Size = new wxSlider(Panel1_Circles, ID_SLIDER_Circles_Size, 5, 1, 20, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_Circles_Size"));
    FlexGridSizer18->Add(Slider_Circles_Size, 1, wxALL|wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    BitmapButton_CirclesSize = new wxBitmapButton(Panel1_Circles, ID_BITMAPBUTTON_SLIDER_Circles_Size, padlock16x16_blue_xpm, wxDefaultPosition, wxSize(22,21), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON_SLIDER_Circles_Size"));
    BitmapButton_CirclesSize->SetDefault();
    FlexGridSizer18->Add(BitmapButton_CirclesSize, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 1);
    FlexGridSizer80->Add(FlexGridSizer18, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizer11 = new wxFlexGridSizer(0, 3, 0, 0);
    CheckBox_Circles_Bounce = new wxCheckBox(Panel1_Circles, ID_CHECKBOX_Circles_Bounce, _("Bounce"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX_Circles_Bounce"));
    CheckBox_Circles_Bounce->SetValue(false);
    FlexGridSizer11->Add(CheckBox_Circles_Bounce, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizer11->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 1);
    BitmapButton_CirclesBounce = new wxBitmapButton(Panel1_Circles, ID_BITMAPBUTTON_CHECKBOX_Circles_Bounce, padlock16x16_blue_xpm, wxDefaultPosition, wxSize(22,21), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON_CHECKBOX_Circles_Bounce"));
    BitmapButton_CirclesBounce->SetDefault();
    FlexGridSizer11->Add(BitmapButton_CirclesBounce, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 1);
    FlexGridSizer80->Add(FlexGridSizer11, 1, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizer15 = new wxFlexGridSizer(0, 4, 0, 0);
    CheckBox_Circles_Collide = new wxCheckBox(Panel1_Circles, ID_CHECKBOX_Circles_Collide, _("Collide"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX_Circles_Collide"));
    CheckBox_Circles_Collide->SetValue(false);
    CheckBox_Circles_Collide->Hide();
    FlexGridSizer15->Add(CheckBox_Circles_Collide, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizer15->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 1);
    BitmapButton_CirclesCollide = new wxBitmapButton(Panel1_Circles, ID_BITMAPBUTTON_CHECKBOX_Circles_Collide, padlock16x16_blue_xpm, wxDefaultPosition, wxSize(22,21), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON_CHECKBOX_Circles_Collide"));
    BitmapButton_CirclesCollide->SetDefault();
    BitmapButton_CirclesCollide->Hide();
    FlexGridSizer15->Add(BitmapButton_CirclesCollide, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 1);
    FlexGridSizer80->Add(FlexGridSizer15, 1, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizer12 = new wxFlexGridSizer(0, 3, 0, 0);
    CheckBox_Circles_Radial = new wxCheckBox(Panel1_Circles, ID_CHECKBOX_Circles_Radial, _("Radial"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX_Circles_Radial"));
    CheckBox_Circles_Radial->SetValue(false);
    FlexGridSizer12->Add(CheckBox_Circles_Radial, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizer12->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 1);
    BitmapButton_CirclesRadial = new wxBitmapButton(Panel1_Circles, ID_BITMAPBUTTON_CHECKBOX_Circles_Radial, padlock16x16_blue_xpm, wxDefaultPosition, wxSize(22,21), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON_CHECKBOX_Circles_Radial"));
    BitmapButton_CirclesRadial->SetDefault();
    FlexGridSizer12->Add(BitmapButton_CirclesRadial, 1, wxALL|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 1);
    FlexGridSizer80->Add(FlexGridSizer12, 1, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizer14 = new wxFlexGridSizer(0, 3, 0, 0);
    CheckBox_Circles_Plasma = new wxCheckBox(Panel1_Circles, ID_CHECKBOX_Circles_Plasma, _("Plasma"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX_Circles_Plasma"));
    CheckBox_Circles_Plasma->SetValue(false);
    FlexGridSizer14->Add(CheckBox_Circles_Plasma, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizer14->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 1);
    BitmapButton_CirclesPlasma = new wxBitmapButton(Panel1_Circles, ID_BITMAPBUTTON_CHECKBOX_Circles_Plasma, padlock16x16_blue_xpm, wxDefaultPosition, wxSize(22,21), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON_CHECKBOX_Circles_Plasma"));
    BitmapButton_CirclesPlasma->SetDefault();
    FlexGridSizer14->Add(BitmapButton_CirclesPlasma, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 1);
    FlexGridSizer80->Add(FlexGridSizer14, 1, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizer13 = new wxFlexGridSizer(0, 3, 0, 0);
    CheckBox_Circles_Radial_3D = new wxCheckBox(Panel1_Circles, ID_CHECKBOX_Circles_Radial_3D, _("Radial 3D"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX_Circles_Radial_3D"));
    CheckBox_Circles_Radial_3D->SetValue(false);
    FlexGridSizer13->Add(CheckBox_Circles_Radial_3D, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    BitmapButton_CirclesRadial3D = new wxBitmapButton(Panel1_Circles, ID_BITMAPBUTTON_CHECKBOX_Circles_Radial_3D, padlock16x16_blue_xpm, wxDefaultPosition, wxSize(22,21), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON_CHECKBOX_Circles_Radial_3D"));
    BitmapButton_CirclesRadial3D->SetDefault();
    FlexGridSizer13->Add(BitmapButton_CirclesRadial3D, 1, wxALL|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 1);
    FlexGridSizer80->Add(FlexGridSizer13, 1, wxALL|wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizer16 = new wxFlexGridSizer(0, 3, 0, 0);
    CheckBox_Circles_Random_m = new wxCheckBox(Panel1_Circles, ID_CHECKBOX_Circles_Random_m, _("Random Motion"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX_Circles_Random_m"));
    CheckBox_Circles_Random_m->SetValue(false);
    CheckBox_Circles_Random_m->Hide();
    FlexGridSizer16->Add(CheckBox_Circles_Random_m, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizer16->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 1);
    BitmapButton_RandomMotion = new wxBitmapButton(Panel1_Circles, ID_BITMAPBUTTON_CHECKBOX_Circles_Random_m, padlock16x16_blue_xpm, wxDefaultPosition, wxSize(22,21), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON_CHECKBOX_Circles_Random_m"));
    BitmapButton_RandomMotion->SetDefault();
    BitmapButton_RandomMotion->Hide();
    FlexGridSizer16->Add(BitmapButton_RandomMotion, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 1);
    FlexGridSizer80->Add(FlexGridSizer16, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    Panel1_Circles->SetSizer(FlexGridSizer80);
    FlexGridSizer80->Fit(Panel1_Circles);
    FlexGridSizer80->SetSizeHints(Panel1_Circles);
    Panel1_ColorWash = new wxPanel(Choicebook1, ID_PANEL5, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("ID_PANEL5"));
    FlexGridSizer37 = new wxFlexGridSizer(0, 3, 0, 0);
    FlexGridSizer37->AddGrowableCol(1);
    StaticText5 = new wxStaticText(Panel1_ColorWash, ID_STATICTEXT21, _("Count"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT21"));
    FlexGridSizer37->Add(StaticText5, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    Slider_ColorWash_Count = new wxSlider(Panel1_ColorWash, ID_SLIDER_ColorWash_Count, 1, 1, 10, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_ColorWash_Count"));
    FlexGridSizer37->Add(Slider_ColorWash_Count, 1, wxALL|wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    BitmapButton_ColorWashCount = new wxBitmapButton(Panel1_ColorWash, ID_BITMAPBUTTON_SLIDER_ColorWash_Count, padlock16x16_blue_xpm, wxDefaultPosition, wxSize(20,20), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON_SLIDER_ColorWash_Count"));
    BitmapButton_ColorWashCount->SetDefault();
    BitmapButton_ColorWashCount->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_ACTIVECAPTION));
    FlexGridSizer37->Add(BitmapButton_ColorWashCount, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 1);
    StaticText30 = new wxStaticText(Panel1_ColorWash, ID_STATICTEXT32, _("Horizontal"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT32"));
    FlexGridSizer37->Add(StaticText30, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    CheckBox_ColorWash_HFade = new wxCheckBox(Panel1_ColorWash, ID_CHECKBOX_ColorWash_HFade, _("Fade"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX_ColorWash_HFade"));
    CheckBox_ColorWash_HFade->SetValue(false);
    FlexGridSizer37->Add(CheckBox_ColorWash_HFade, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    BitmapButton_ColorWashHFade = new wxBitmapButton(Panel1_ColorWash, ID_BITMAPBUTTON_CHECKBOX_ColorWash_HFade, padlock16x16_blue_xpm, wxDefaultPosition, wxSize(20,20), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON_CHECKBOX_ColorWash_HFade"));
    BitmapButton_ColorWashHFade->SetDefault();
    BitmapButton_ColorWashHFade->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_ACTIVECAPTION));
    FlexGridSizer37->Add(BitmapButton_ColorWashHFade, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 1);
    StaticText31 = new wxStaticText(Panel1_ColorWash, ID_STATICTEXT33, _("Vertical"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT33"));
    FlexGridSizer37->Add(StaticText31, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    CheckBox_ColorWash_VFade = new wxCheckBox(Panel1_ColorWash, ID_CHECKBOX_ColorWash_VFade, _("Fade"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX_ColorWash_VFade"));
    CheckBox_ColorWash_VFade->SetValue(false);
    FlexGridSizer37->Add(CheckBox_ColorWash_VFade, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    BitmapButton_ColorWashVFade = new wxBitmapButton(Panel1_ColorWash, ID_BITMAPBUTTON_CHECKBOX_ColorWash_VFade, padlock16x16_blue_xpm, wxDefaultPosition, wxSize(20,20), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON_CHECKBOX_ColorWash_VFade"));
    BitmapButton_ColorWashVFade->SetDefault();
    BitmapButton_ColorWashVFade->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_ACTIVECAPTION));
    FlexGridSizer37->Add(BitmapButton_ColorWashVFade, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 1);
    Panel1_ColorWash->SetSizer(FlexGridSizer37);
    FlexGridSizer37->Fit(Panel1_ColorWash);
    FlexGridSizer37->SetSizeHints(Panel1_ColorWash);
    Panel1_Curtain = new wxPanel(Choicebook1, ID_PANEL1, wxPoint(150,14), wxDefaultSize, wxTAB_TRAVERSAL, _T("ID_PANEL1"));
    FlexGridSizer7 = new wxFlexGridSizer(0, 3, 0, 0);
    FlexGridSizer7->AddGrowableCol(1);
    StaticText7 = new wxStaticText(Panel1_Curtain, ID_STATICTEXT6, _("Curtain Edge"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT6"));
    FlexGridSizer7->Add(StaticText7, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    Choice_Curtain_Edge = new wxChoice(Panel1_Curtain, ID_CHOICE_Curtain_Edge, wxDefaultPosition, wxDefaultSize, 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE_Curtain_Edge"));
    Choice_Curtain_Edge->SetSelection( Choice_Curtain_Edge->Append(_("left")) );
    Choice_Curtain_Edge->Append(_("center"));
    Choice_Curtain_Edge->Append(_("right"));
    Choice_Curtain_Edge->Append(_("bottom"));
    Choice_Curtain_Edge->Append(_("middle"));
    Choice_Curtain_Edge->Append(_("top"));
    FlexGridSizer7->Add(Choice_Curtain_Edge, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    BitmapButton_CurtainEdge = new wxBitmapButton(Panel1_Curtain, ID_BITMAPBUTTON_CHOICE_Curtain_Edge, padlock16x16_blue_xpm, wxDefaultPosition, wxSize(20,20), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON_CHOICE_Curtain_Edge"));
    BitmapButton_CurtainEdge->SetDefault();
    BitmapButton_CurtainEdge->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_ACTIVECAPTION));
    FlexGridSizer7->Add(BitmapButton_CurtainEdge, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 1);
    StaticText6 = new wxStaticText(Panel1_Curtain, ID_STATICTEXT5, _("Effect"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT5"));
    FlexGridSizer7->Add(StaticText6, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    Choice_Curtain_Effect = new wxChoice(Panel1_Curtain, ID_CHOICE_Curtain_Effect, wxDefaultPosition, wxDefaultSize, 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE_Curtain_Effect"));
    Choice_Curtain_Effect->SetSelection( Choice_Curtain_Effect->Append(_("open")) );
    Choice_Curtain_Effect->Append(_("close"));
    Choice_Curtain_Effect->Append(_("open then close"));
    Choice_Curtain_Effect->Append(_("close then open"));
    FlexGridSizer7->Add(Choice_Curtain_Effect, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    BitmapButton_CurtainEffect = new wxBitmapButton(Panel1_Curtain, ID_BITMAPBUTTON_CHOICE_Curtain_Effect, padlock16x16_blue_xpm, wxDefaultPosition, wxSize(20,20), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON_CHOICE_Curtain_Effect"));
    BitmapButton_CurtainEffect->SetDefault();
    BitmapButton_CurtainEffect->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_ACTIVECAPTION));
    FlexGridSizer7->Add(BitmapButton_CurtainEffect, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 1);
    StaticText8 = new wxStaticText(Panel1_Curtain, ID_STATICTEXT7, _("Swag Width"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT7"));
    FlexGridSizer7->Add(StaticText8, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    Slider_Curtain_Swag = new wxSlider(Panel1_Curtain, ID_SLIDER_Curtain_Swag, 3, 0, 10, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_Curtain_Swag"));
    FlexGridSizer7->Add(Slider_Curtain_Swag, 1, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    BitmapButton_CurtainSwag = new wxBitmapButton(Panel1_Curtain, ID_BITMAPBUTTON_SLIDER_Curtain_Swag, padlock16x16_blue_xpm, wxDefaultPosition, wxSize(20,20), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON_SLIDER_Curtain_Swag"));
    BitmapButton_CurtainSwag->SetDefault();
    BitmapButton_CurtainSwag->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_ACTIVECAPTION));
    FlexGridSizer7->Add(BitmapButton_CurtainSwag, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 1);
    CheckBox_Curtain_Repeat = new wxCheckBox(Panel1_Curtain, ID_CHECKBOX_Curtain_Repeat, _("Repeat"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX_Curtain_Repeat"));
    CheckBox_Curtain_Repeat->SetValue(false);
    FlexGridSizer7->Add(CheckBox_Curtain_Repeat, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizer7->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 1);
    BitmapButton_CurtainRepeat = new wxBitmapButton(Panel1_Curtain, ID_BITMAPBUTTON_CHECKBOX_Curtain_Repeat, padlock16x16_blue_xpm, wxDefaultPosition, wxSize(20,20), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON_CHECKBOX_Curtain_Repeat"));
    BitmapButton_CurtainRepeat->SetDefault();
    BitmapButton_CurtainRepeat->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_ACTIVECAPTION));
    FlexGridSizer7->Add(BitmapButton_CurtainRepeat, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 1);
    Panel1_Curtain->SetSizer(FlexGridSizer7);
    FlexGridSizer7->Fit(Panel1_Curtain);
    FlexGridSizer7->SetSizeHints(Panel1_Curtain);
    Panel1_Faces = new wxPanel(Choicebook1, ID_PANEL6, wxPoint(141,8), wxDefaultSize, wxTAB_TRAVERSAL, _T("ID_PANEL6"));
    FlexGridSizer47 = new wxFlexGridSizer(0, 2, 0, 0);
    StaticText14 = new wxStaticText(Panel1_Faces, ID_STATICTEXT15, _("Phoneme"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT15"));
    FlexGridSizer47->Add(StaticText14, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    Choice_Faces_Phoneme = new wxChoice(Panel1_Faces, ID_CHOICE_Faces_Phoneme, wxDefaultPosition, wxDefaultSize, 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE_Faces_Phoneme"));
    FlexGridSizer47->Add(Choice_Faces_Phoneme, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    Panel1_Faces->SetSizer(FlexGridSizer47);
    FlexGridSizer47->Fit(Panel1_Faces);
    FlexGridSizer47->SetSizeHints(Panel1_Faces);
    Panel1_Fire = new wxPanel(Choicebook1, ID_PANEL12, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("ID_PANEL12"));
    FlexGridSizer38 = new wxFlexGridSizer(0, 3, 0, 0);
    FlexGridSizer38->AddGrowableCol(1);
    StaticText83 = new wxStaticText(Panel1_Fire, ID_STATICTEXT84, _("Height"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT84"));
    FlexGridSizer38->Add(StaticText83, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
    Slider_Fire_Height = new wxSlider(Panel1_Fire, ID_SLIDER_Fire_Height, 50, 10, 100, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_Fire_Height"));
    FlexGridSizer38->Add(Slider_Fire_Height, 1, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
    BitmapButton_FireHeight = new wxBitmapButton(Panel1_Fire, ID_BITMAPBUTTON_SLIDER_Fire_Height, padlock16x16_blue_xpm, wxDefaultPosition, wxSize(20,20), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON_SLIDER_Fire_Height"));
    BitmapButton_FireHeight->SetDefault();
    BitmapButton_FireHeight->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_ACTIVECAPTION));
    FlexGridSizer38->Add(BitmapButton_FireHeight, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 1);
    StaticText132 = new wxStaticText(Panel1_Fire, ID_STATICTEXT133, _("Hue Shift"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT133"));
    FlexGridSizer38->Add(StaticText132, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
    Slider_Fire_HueShift = new wxSlider(Panel1_Fire, ID_SLIDER_Fire_HueShift, 0, 0, 100, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_Fire_HueShift"));
    FlexGridSizer38->Add(Slider_Fire_HueShift, 1, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
    BitmapButton_FireWidth = new wxBitmapButton(Panel1_Fire, ID_BITMAPBUTTON_SLIDER_Fire_HueShift, padlock16x16_blue_xpm, wxDefaultPosition, wxSize(20,20), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON_SLIDER_Fire_HueShift"));
    BitmapButton_FireWidth->SetDefault();
    BitmapButton_FireWidth->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_ACTIVECAPTION));
    FlexGridSizer38->Add(BitmapButton_FireWidth, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 1);
    StaticText134 = new wxStaticText(Panel1_Fire, ID_STATICTEXT135, _("Grow Fire\?"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT135"));
    FlexGridSizer38->Add(StaticText134, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
    CheckBox_Fire_GrowFire = new wxCheckBox(Panel1_Fire, ID_CHECKBOX_Fire_GrowFire, _("Yes"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX_Fire_GrowFire"));
    CheckBox_Fire_GrowFire->SetValue(false);
    FlexGridSizer38->Add(CheckBox_Fire_GrowFire, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    BitmapButton_FireGrow = new wxBitmapButton(Panel1_Fire, ID_BITMAPBUTTON_CHECKBOX_Fire_GrowFire, padlock16x16_blue_xpm, wxDefaultPosition, wxSize(20,20), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON_CHECKBOX_Fire_GrowFire"));
    BitmapButton_FireGrow->SetDefault();
    BitmapButton_FireGrow->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_ACTIVECAPTION));
    FlexGridSizer38->Add(BitmapButton_FireGrow, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 1);
    Panel1_Fire->SetSizer(FlexGridSizer38);
    FlexGridSizer38->Fit(Panel1_Fire);
    FlexGridSizer38->SetSizeHints(Panel1_Fire);
    Panel1_Fireworks = new wxPanel(Choicebook1, ID_PANEL36, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("ID_PANEL36"));
    FlexGridSizer73 = new wxFlexGridSizer(0, 3, 0, 0);
    FlexGridSizer73->AddGrowableCol(1);
    StaticText94 = new wxStaticText(Panel1_Fireworks, ID_STATICTEXT95, _("Number of Explosions"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT95"));
    FlexGridSizer73->Add(StaticText94, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    Slider_Fireworks_Number_Explosions = new wxSlider(Panel1_Fireworks, ID_SLIDER_Fireworks_Number_Explosions, 10, 1, 95, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_Fireworks_Number_Explosions"));
    FlexGridSizer73->Add(Slider_Fireworks_Number_Explosions, 1, wxALL|wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    BitmapButton_FireworksNumberExplosions = new wxBitmapButton(Panel1_Fireworks, ID_BITMAPBUTTON_SLIDER_Fireworks_Number_Explosions, padlock16x16_blue_xpm, wxDefaultPosition, wxSize(20,20), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON_SLIDER_Fireworks_Number_Explosions"));
    BitmapButton_FireworksNumberExplosions->SetDefault();
    BitmapButton_FireworksNumberExplosions->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_ACTIVECAPTION));
    FlexGridSizer73->Add(BitmapButton_FireworksNumberExplosions, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 1);
    StaticText91 = new wxStaticText(Panel1_Fireworks, ID_STATICTEXT92, _("Particles in Explosion"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT92"));
    FlexGridSizer73->Add(StaticText91, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    Slider_Fireworks_Count = new wxSlider(Panel1_Fireworks, ID_SLIDER_Fireworks_Count, 50, 1, 100, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_Fireworks_Count"));
    FlexGridSizer73->Add(Slider_Fireworks_Count, 1, wxALL|wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    BitmapButton_FireworksCount = new wxBitmapButton(Panel1_Fireworks, ID_BITMAPBUTTON_SLIDER_Fireworks_Count, padlock16x16_blue_xpm, wxDefaultPosition, wxSize(20,20), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON_SLIDER_Fireworks_Count"));
    BitmapButton_FireworksCount->SetDefault();
    BitmapButton_FireworksCount->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_ACTIVECAPTION));
    FlexGridSizer73->Add(BitmapButton_FireworksCount, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 1);
    StaticText93 = new wxStaticText(Panel1_Fireworks, ID_STATICTEXT94, _("Velocity of Particles"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT94"));
    FlexGridSizer73->Add(StaticText93, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    Slider_Fireworks_Velocity = new wxSlider(Panel1_Fireworks, ID_SLIDER_Fireworks_Velocity, 2, 1, 10, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_Fireworks_Velocity"));
    FlexGridSizer73->Add(Slider_Fireworks_Velocity, 1, wxALL|wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    BitmapButton_FireworksVelocity = new wxBitmapButton(Panel1_Fireworks, ID_BITMAPBUTTON_SLIDER_Fireworks_Velocity, padlock16x16_blue_xpm, wxDefaultPosition, wxSize(20,20), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON_SLIDER_Fireworks_Velocity"));
    BitmapButton_FireworksVelocity->SetDefault();
    BitmapButton_FireworksVelocity->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_ACTIVECAPTION));
    FlexGridSizer73->Add(BitmapButton_FireworksVelocity, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 1);
    StaticText95 = new wxStaticText(Panel1_Fireworks, ID_STATICTEXT96, _("Particle Fade"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT96"));
    FlexGridSizer73->Add(StaticText95, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    Slider_Fireworks_Fade = new wxSlider(Panel1_Fireworks, ID_SLIDER_Fireworks_Fade, 50, 1, 100, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_Fireworks_Fade"));
    FlexGridSizer73->Add(Slider_Fireworks_Fade, 1, wxALL|wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    BitmapButton_FireworksFade = new wxBitmapButton(Panel1_Fireworks, ID_BITMAPBUTTON_SLIDER_Fireworks_Fade, padlock16x16_blue_xpm, wxDefaultPosition, wxSize(20,20), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON_SLIDER_Fireworks_Fade"));
    BitmapButton_FireworksFade->SetDefault();
    BitmapButton_FireworksFade->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_ACTIVECAPTION));
    FlexGridSizer73->Add(BitmapButton_FireworksFade, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 1);
    Panel1_Fireworks->SetSizer(FlexGridSizer73);
    FlexGridSizer73->Fit(Panel1_Fireworks);
    FlexGridSizer73->SetSizeHints(Panel1_Fireworks);
    Panel1_Garlands = new wxPanel(Choicebook1, ID_PANEL14, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("ID_PANEL14"));
    FlexGridSizer39 = new wxFlexGridSizer(0, 3, 0, 0);
    FlexGridSizer39->AddGrowableCol(1);
    StaticText32 = new wxStaticText(Panel1_Garlands, ID_STATICTEXT34, _("Garland Type"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT34"));
    FlexGridSizer39->Add(StaticText32, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
    Slider_Garlands_Type = new wxSlider(Panel1_Garlands, ID_SLIDER_Garlands_Type, 0, 0, 4, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_Garlands_Type"));
    FlexGridSizer39->Add(Slider_Garlands_Type, 1, wxALL|wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
    BitmapButton_GarlandsType = new wxBitmapButton(Panel1_Garlands, ID_BITMAPBUTTON_SLIDER_Garlands_Type, padlock16x16_blue_xpm, wxDefaultPosition, wxSize(20,20), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON_SLIDER_Garlands_Type"));
    BitmapButton_GarlandsType->SetDefault();
    BitmapButton_GarlandsType->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_ACTIVECAPTION));
    FlexGridSizer39->Add(BitmapButton_GarlandsType, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 1);
    StaticText33 = new wxStaticText(Panel1_Garlands, ID_STATICTEXT35, _("Spacing"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT35"));
    FlexGridSizer39->Add(StaticText33, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
    Slider_Garlands_Spacing = new wxSlider(Panel1_Garlands, ID_SLIDER_Garlands_Spacing, 0, 0, 100, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_Garlands_Spacing"));
    FlexGridSizer39->Add(Slider_Garlands_Spacing, 1, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
    BitmapButton_GarlandsSpacing = new wxBitmapButton(Panel1_Garlands, ID_BITMAPBUTTON_SLIDER_Garlands_Spacing, padlock16x16_blue_xpm, wxDefaultPosition, wxSize(20,20), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON_SLIDER_Garlands_Spacing"));
    BitmapButton_GarlandsSpacing->SetDefault();
    BitmapButton_GarlandsSpacing->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_ACTIVECAPTION));
    FlexGridSizer39->Add(BitmapButton_GarlandsSpacing, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 1);
    Panel1_Garlands->SetSizer(FlexGridSizer39);
    FlexGridSizer39->Fit(Panel1_Garlands);
    FlexGridSizer39->SetSizeHints(Panel1_Garlands);
    Panel1_Glediator = new wxPanel(Choicebook1, ID_PANEL17, wxPoint(37,9), wxDefaultSize, wxTAB_TRAVERSAL, _T("ID_PANEL17"));
    FlexGridSizer53 = new wxFlexGridSizer(0, 1, 0, 0);
    FlexGridSizer54 = new wxFlexGridSizer(0, 2, 0, 0);
    Button1 = new wxButton(Panel1_Glediator, ID_BUTTON3, _("Glediator *.gled File"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON3"));
    FlexGridSizer54->Add(Button1, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    BitmapButton_Glediator_Filename = new wxBitmapButton(Panel1_Glediator, ID_BITMAPBUTTON10, padlock16x16_blue_xpm, wxDefaultPosition, wxSize(20,17), wxBU_AUTODRAW, wxDefaultValidator, _T("ID_BITMAPBUTTON10"));
    BitmapButton_Glediator_Filename->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_ACTIVECAPTION));
    FlexGridSizer54->Add(BitmapButton_Glediator_Filename, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizer53->Add(FlexGridSizer54, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    TextCtrl_Glediator_Filename = new wxTextCtrl(Panel1_Glediator, ID_TEXTCTRL_Glediator_Filename, wxEmptyString, wxDefaultPosition, wxSize(255,21), wxTE_READONLY, wxDefaultValidator, _T("ID_TEXTCTRL_Glediator_Filename"));
    FlexGridSizer53->Add(TextCtrl_Glediator_Filename, 1, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
    Panel1_Glediator->SetSizer(FlexGridSizer53);
    FlexGridSizer53->Fit(Panel1_Glediator);
    FlexGridSizer53->SetSizeHints(Panel1_Glediator);
    Panel1_Life = new wxPanel(Choicebook1, ID_PANEL16, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("ID_PANEL16"));
    FlexGridSizer40 = new wxFlexGridSizer(0, 3, 0, 0);
    FlexGridSizer40->AddGrowableCol(1);
    StaticText35 = new wxStaticText(Panel1_Life, ID_STATICTEXT36, _("Cells to start"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT36"));
    FlexGridSizer40->Add(StaticText35, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
    Slider_Life_Count = new wxSlider(Panel1_Life, ID_SLIDER_Life_Count, 50, 0, 100, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_Life_Count"));
    FlexGridSizer40->Add(Slider_Life_Count, 1, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
    BitmapButton_LifeCount = new wxBitmapButton(Panel1_Life, ID_BITMAPBUTTON_SLIDER_Life_Count, padlock16x16_blue_xpm, wxDefaultPosition, wxSize(20,20), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON_SLIDER_Life_Count"));
    BitmapButton_LifeCount->SetDefault();
    BitmapButton_LifeCount->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_ACTIVECAPTION));
    FlexGridSizer40->Add(BitmapButton_LifeCount, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 1);
    StaticText37 = new wxStaticText(Panel1_Life, ID_STATICTEXT37, _("Type"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT37"));
    FlexGridSizer40->Add(StaticText37, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
    Slider_Life_Seed = new wxSlider(Panel1_Life, ID_SLIDER_Life_Seed, 0, 0, 4, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_Life_Seed"));
    FlexGridSizer40->Add(Slider_Life_Seed, 1, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
    BitmapButton_LifeSpeed = new wxBitmapButton(Panel1_Life, ID_BITMAPBUTTON_SLIDER_Life_Seed, padlock16x16_blue_xpm, wxDefaultPosition, wxSize(20,20), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON_SLIDER_Life_Seed"));
    BitmapButton_LifeSpeed->SetDefault();
    BitmapButton_LifeSpeed->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_ACTIVECAPTION));
    FlexGridSizer40->Add(BitmapButton_LifeSpeed, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 1);
    Panel1_Life->SetSizer(FlexGridSizer40);
    FlexGridSizer40->Fit(Panel1_Life);
    FlexGridSizer40->SetSizeHints(Panel1_Life);
    Panel1_Meteors = new wxPanel(Choicebook1, ID_PANEL18, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("ID_PANEL18"));
    FlexGridSizer41 = new wxFlexGridSizer(0, 3, 0, 0);
    FlexGridSizer41->AddGrowableCol(1);
    StaticText39 = new wxStaticText(Panel1_Meteors, ID_STATICTEXT39, _("Colors"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT39"));
    FlexGridSizer41->Add(StaticText39, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
    Choice_Meteors_Type = new wxChoice(Panel1_Meteors, ID_CHOICE_Meteors_Type, wxDefaultPosition, wxDefaultSize, 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE_Meteors_Type"));
    FlexGridSizer41->Add(Choice_Meteors_Type, 1, wxTOP|wxBOTTOM|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    BitmapButton_MeteorsType = new wxBitmapButton(Panel1_Meteors, ID_BITMAPBUTTON_CHOICE_Meteors_Type, padlock16x16_blue_xpm, wxDefaultPosition, wxSize(20,20), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON_CHOICE_Meteors_Type"));
    BitmapButton_MeteorsType->SetDefault();
    BitmapButton_MeteorsType->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_ACTIVECAPTION));
    FlexGridSizer41->Add(BitmapButton_MeteorsType, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 1);
    StaticText128 = new wxStaticText(Panel1_Meteors, ID_STATICTEXT129, _("Effect"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT129"));
    FlexGridSizer41->Add(StaticText128, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
    Choice_Meteors_Effect = new wxChoice(Panel1_Meteors, ID_CHOICE_Meteors_Effect, wxDefaultPosition, wxDefaultSize, 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE_Meteors_Effect"));
    FlexGridSizer41->Add(Choice_Meteors_Effect, 1, wxTOP|wxBOTTOM|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
    BitmapButton_MeteorsEffect = new wxBitmapButton(Panel1_Meteors, ID_BITMAPBUTTON_CHOICE_Meteors_Effect, padlock16x16_blue_xpm, wxDefaultPosition, wxSize(20,20), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON_CHOICE_Meteors_Effect"));
    BitmapButton_MeteorsEffect->SetDefault();
    BitmapButton_MeteorsEffect->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_ACTIVECAPTION));
    FlexGridSizer41->Add(BitmapButton_MeteorsEffect, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 1);
    StaticText41 = new wxStaticText(Panel1_Meteors, ID_STATICTEXT41, _("Count"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT41"));
    FlexGridSizer41->Add(StaticText41, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
    Slider_Meteors_Count = new wxSlider(Panel1_Meteors, ID_SLIDER_Meteors_Count, 10, 1, 100, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_Meteors_Count"));
    FlexGridSizer41->Add(Slider_Meteors_Count, 1, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
    BitmapButton_MeteorsCount = new wxBitmapButton(Panel1_Meteors, ID_BITMAPBUTTON_SLIDER_Meteors_Count, padlock16x16_blue_xpm, wxDefaultPosition, wxSize(20,20), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON_SLIDER_Meteors_Count"));
    BitmapButton_MeteorsCount->SetDefault();
    BitmapButton_MeteorsCount->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_ACTIVECAPTION));
    FlexGridSizer41->Add(BitmapButton_MeteorsCount, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 1);
    StaticText43 = new wxStaticText(Panel1_Meteors, ID_STATICTEXT43, _("Trail Length"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT43"));
    FlexGridSizer41->Add(StaticText43, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
    Slider_Meteors_Length = new wxSlider(Panel1_Meteors, ID_SLIDER_Meteors_Length, 25, 1, 100, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_Meteors_Length"));
    FlexGridSizer41->Add(Slider_Meteors_Length, 1, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
    BitmapButton_MeteorsLength = new wxBitmapButton(Panel1_Meteors, ID_BITMAPBUTTON_SLIDER_Meteors_Length, padlock16x16_blue_xpm, wxDefaultPosition, wxSize(20,20), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON_SLIDER_Meteors_Length"));
    BitmapButton_MeteorsLength->SetDefault();
    BitmapButton_MeteorsLength->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_ACTIVECAPTION));
    FlexGridSizer41->Add(BitmapButton_MeteorsLength, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 1);
    StaticText130 = new wxStaticText(Panel1_Meteors, ID_STATICTEXT131, _("Swirl Intensity"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT131"));
    FlexGridSizer41->Add(StaticText130, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
    Slider_Meteors_Swirl_Intensity = new wxSlider(Panel1_Meteors, ID_SLIDER_Meteors_Swirl_Intensity, 0, 0, 20, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_Meteors_Swirl_Intensity"));
    FlexGridSizer41->Add(Slider_Meteors_Swirl_Intensity, 1, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
    BitmapButton_MeteorsSwirlIntensity = new wxBitmapButton(Panel1_Meteors, ID_BITMAPBUTTON_SLIDER_Meteors_Swirl_Intensity, padlock16x16_blue_xpm, wxDefaultPosition, wxSize(20,20), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON_SLIDER_Meteors_Swirl_Intensity"));
    BitmapButton_MeteorsSwirlIntensity->SetDefault();
    BitmapButton_MeteorsSwirlIntensity->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_ACTIVECAPTION));
    FlexGridSizer41->Add(BitmapButton_MeteorsSwirlIntensity, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 1);
    Panel1_Meteors->SetSizer(FlexGridSizer41);
    FlexGridSizer41->Fit(Panel1_Meteors);
    FlexGridSizer41->SetSizeHints(Panel1_Meteors);
    Panel1_Piano = new wxPanel(Choicebook1, ID_PANEL39, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("ID_PANEL39"));
    FlexGridSizer5 = new wxFlexGridSizer(0, 1, 0, 0);
    FlexGridSizer5->AddGrowableCol(0);
    FlexGridSizer27 = new wxFlexGridSizer(0, 3, 0, 0);
    FlexGridSizer27->AddGrowableCol(0);
    StaticText1 = new wxStaticText(Panel1_Piano, ID_STATICTEXT1, _("Style:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT1"));
    FlexGridSizer27->Add(StaticText1, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    Choice_Piano_Style = new wxChoice(Panel1_Piano, ID_CHOICE_Piano_Style, wxDefaultPosition, wxDefaultSize, 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE_Piano_Style"));
    Choice_Piano_Style->SetSelection( Choice_Piano_Style->Append(_("Anim Image (face)")) );
    Choice_Piano_Style->Append(_("Equalizer (bars)"));
    Choice_Piano_Style->Append(_("Piano Keys (top/edge)"));
    Choice_Piano_Style->Append(_("Player Piano (scroll)"));
    FlexGridSizer27->Add(Choice_Piano_Style, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    BitmapButton_Piano_Style = new wxBitmapButton(Panel1_Piano, ID_BITMAPBUTTON_SLIDER_Piano_Style, padlock16x16_blue_xpm, wxDefaultPosition, wxSize(22,21), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON_SLIDER_Piano_Style"));
    BitmapButton_Piano_Style->SetDefault();
    FlexGridSizer27->Add(BitmapButton_Piano_Style, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 1);
    StaticText_Piano_NumKeys = new wxStaticText(Panel1_Piano, ID_STATICTEXT_Piano_NumKeys, _("# Keys:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_Piano_NumKeys"));
    FlexGridSizer27->Add(StaticText_Piano_NumKeys, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    Slider_Piano_NumKeys = new wxSlider(Panel1_Piano, ID_SLIDER_Piano_NumKeys, 10, 1, 95, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_Piano_NumKeys"));
    FlexGridSizer27->Add(Slider_Piano_NumKeys, 1, wxALL|wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    BitmapButton_Piano_NumKeys = new wxBitmapButton(Panel1_Piano, ID_BITMAPBUTTON_Piano_NumKeys, padlock16x16_blue_xpm, wxDefaultPosition, wxSize(22,21), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON_Piano_NumKeys"));
    BitmapButton_Piano_NumKeys->SetDefault();
    FlexGridSizer27->Add(BitmapButton_Piano_NumKeys, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    StaticText_Piano_NumRows = new wxStaticText(Panel1_Piano, ID_STATICTEXT_Piano_NumRows, _("# Rows:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT_Piano_NumRows"));
    FlexGridSizer27->Add(StaticText_Piano_NumRows, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    Slider_Piano_NumRows = new wxSlider(Panel1_Piano, ID_SLIDER_Piano_KeyWidth, 10, 1, 95, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_Piano_KeyWidth"));
    FlexGridSizer27->Add(Slider_Piano_NumRows, 1, wxALL|wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    BitmapButton_Piano_NumRows = new wxBitmapButton(Panel1_Piano, ID_BITMAPBUTTON_Piano_NumRows, padlock16x16_blue_xpm, wxDefaultPosition, wxSize(22,21), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON_Piano_NumRows"));
    BitmapButton_Piano_NumRows->SetDefault();
    FlexGridSizer27->Add(BitmapButton_Piano_NumRows, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    StaticText21 = new wxStaticText(Panel1_Piano, ID_STATICTEXT20, _("Placement:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT20"));
    FlexGridSizer27->Add(StaticText21, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    Choice_Piano_KeyPlacement = new wxChoice(Panel1_Piano, ID_CHOICE_PianoPlacement, wxDefaultPosition, wxDefaultSize, 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE_PianoPlacement"));
    Choice_Piano_KeyPlacement->SetSelection( Choice_Piano_KeyPlacement->Append(_("Tile")) );
    Choice_Piano_KeyPlacement->Append(_("Stretch/shrink"));
    Choice_Piano_KeyPlacement->Append(_("Top left"));
    Choice_Piano_KeyPlacement->Append(_("Top center"));
    Choice_Piano_KeyPlacement->Append(_("Top right"));
    Choice_Piano_KeyPlacement->Append(_("Middle left"));
    Choice_Piano_KeyPlacement->Append(_("Middle center"));
    Choice_Piano_KeyPlacement->Append(_("Middle right"));
    Choice_Piano_KeyPlacement->Append(_("Bottom left"));
    Choice_Piano_KeyPlacement->Append(_("Bottom middle"));
    Choice_Piano_KeyPlacement->Append(_("Bottom right"));
    FlexGridSizer27->Add(Choice_Piano_KeyPlacement, 1, wxALL|wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    BitmapButton_Piano_KeyPlacement = new wxBitmapButton(Panel1_Piano, ID_BITMAPBUTTON_Piano_KeyPlacement, padlock16x16_blue_xpm, wxDefaultPosition, wxSize(22,21), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON_Piano_KeyPlacement"));
    BitmapButton_Piano_KeyPlacement->SetDefault();
    FlexGridSizer27->Add(BitmapButton_Piano_KeyPlacement, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizer5->Add(FlexGridSizer27, 1, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    Notebook2 = new wxNotebook(Panel1_Piano, ID_NOTEBOOK2, wxDefaultPosition, wxDefaultSize, 0, _T("ID_NOTEBOOK2"));
    Notebook2->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_SCROLLBAR));
    Panel3 = new wxPanel(Notebook2, ID_PANEL9, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("ID_PANEL9"));
    FlexGridSizer28 = new wxFlexGridSizer(5, 1, 0, 0);
    FlexGridSizer26 = new wxFlexGridSizer(0, 6, 0, 0);
    Button_Piano_CueFilename = new wxButton(Panel3, ID_BUTTON_Piano_CueFilename, _("File name"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON_Piano_CueFilename"));
    Button_Piano_CueFilename->SetBackgroundColour(wxColour(224,224,224));
    FlexGridSizer26->Add(Button_Piano_CueFilename, 1, wxTOP|wxLEFT|wxRIGHT|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
    BitmapButton_Piano_CueFilename = new wxBitmapButton(Panel3, ID_BITMAPBUTTON_Piano_Filename, padlock16x16_blue_xpm, wxDefaultPosition, wxSize(22,21), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON_Piano_Filename"));
    BitmapButton_Piano_CueFilename->SetDefault();
    FlexGridSizer26->Add(BitmapButton_Piano_CueFilename, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 1);
    FlexGridSizer26->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    StaticText15_layout_kludge_1 = new wxStaticText(Panel3, ID_STATICTEXT14, _("                                    x"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT14"));
    StaticText15_layout_kludge_1->SetForegroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_WINDOW));
    FlexGridSizer26->Add(StaticText15_layout_kludge_1, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizer28->Add(FlexGridSizer26, 1, wxALL|wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    TextCtrl_Piano_CueFilename = new wxTextCtrl(Panel3, ID_TEXTCTRL_Piano_CueFilename, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_READONLY, wxDefaultValidator, _T("ID_TEXTCTRL_Piano_CueFilename"));
    FlexGridSizer28->Add(TextCtrl_Piano_CueFilename, 1, wxALL|wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    Panel3->SetSizer(FlexGridSizer28);
    FlexGridSizer28->Fit(Panel3);
    FlexGridSizer28->SetSizeHints(Panel3);
    Panel4 = new wxPanel(Notebook2, ID_PANEL11, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("ID_PANEL11"));
    FlexGridSizer30 = new wxFlexGridSizer(0, 1, 0, 0);
    FlexGridSizer33 = new wxFlexGridSizer(0, 6, 0, 0);
    Button_Piano_MapFilename = new wxButton(Panel4, ID_BUTTON1, _("File name"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON1"));
    Button_Piano_MapFilename->SetBackgroundColour(wxColour(224,224,224));
    FlexGridSizer33->Add(Button_Piano_MapFilename, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
    BitmapButton_Piano_MapFilename = new wxBitmapButton(Panel4, ID_BITMAPBUTTON_Piano_MapFilename, padlock16x16_blue_xpm, wxDefaultPosition, wxSize(22,21), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON_Piano_MapFilename"));
    BitmapButton_Piano_MapFilename->SetDefault();
    FlexGridSizer33->Add(BitmapButton_Piano_MapFilename, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizer33->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    StaticText14_layout_kludge2 = new wxStaticText(Panel4, ID_STATICTEXT13, _("                                    x"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT13"));
    StaticText14_layout_kludge2->SetForegroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_WINDOW));
    FlexGridSizer33->Add(StaticText14_layout_kludge2, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizer30->Add(FlexGridSizer33, 1, wxALL|wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    TextCtrl_Piano_MapFilename = new wxTextCtrl(Panel4, ID_TEXTCTRL_Piano_MapFilename, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_READONLY, wxDefaultValidator, _T("ID_TEXTCTRL_Piano_MapFilename"));
    FlexGridSizer30->Add(TextCtrl_Piano_MapFilename, 1, wxALL|wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    Panel4->SetSizer(FlexGridSizer30);
    FlexGridSizer30->Fit(Panel4);
    FlexGridSizer30->SetSizeHints(Panel4);
    Panel5 = new wxPanel(Notebook2, ID_PANEL13, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("ID_PANEL13"));
    FlexGridSizer32 = new wxFlexGridSizer(0, 1, 0, 0);
    FlexGridSizer34 = new wxFlexGridSizer(0, 5, 0, 0);
    Button_Piano_ShapeFilename = new wxButton(Panel5, ID_BUTTON2, _("File name"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON2"));
    Button_Piano_ShapeFilename->SetBackgroundColour(wxColour(224,224,224));
    FlexGridSizer34->Add(Button_Piano_ShapeFilename, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
    BitmapButton_Piano_ShapeFilename = new wxBitmapButton(Panel5, ID_BITMAPBUTTON_Piano_ShapeFilename, padlock16x16_blue_xpm, wxDefaultPosition, wxSize(22,21), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON_Piano_ShapeFilename"));
    BitmapButton_Piano_ShapeFilename->SetDefault();
    FlexGridSizer34->Add(BitmapButton_Piano_ShapeFilename, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizer34->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    CheckBox_Piano_Clipping = new wxCheckBox(Panel5, ID_CHECKBOX1, _("Clip shapes"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX1"));
    CheckBox_Piano_Clipping->SetValue(false);
    FlexGridSizer34->Add(CheckBox_Piano_Clipping, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    BitmapButton_Piano_Clipping = new wxBitmapButton(Panel5, ID_BITMAPBUTTON_Piano_Clipping, padlock16x16_blue_xpm, wxDefaultPosition, wxSize(22,21), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON_Piano_Clipping"));
    BitmapButton_Piano_Clipping->SetDefault();
    FlexGridSizer34->Add(BitmapButton_Piano_Clipping, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 1);
    FlexGridSizer32->Add(FlexGridSizer34, 1, wxALL|wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
    TextCtrl_Piano_ShapeFilename = new wxTextCtrl(Panel5, ID_TEXTCTRL_Piano_ShapeFilename, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_READONLY, wxDefaultValidator, _T("ID_TEXTCTRL_Piano_ShapeFilename"));
    FlexGridSizer32->Add(TextCtrl_Piano_ShapeFilename, 1, wxALL|wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    Panel5->SetSizer(FlexGridSizer32);
    FlexGridSizer32->Fit(Panel5);
    FlexGridSizer32->SetSizeHints(Panel5);
    Notebook2->AddPage(Panel3, _("Cues"), false);
    Notebook2->AddPage(Panel4, _("Cue-Shape Map"), false);
    Notebook2->AddPage(Panel5, _("Shapes"), false);
    FlexGridSizer5->Add(Notebook2, 1, wxALL|wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    StaticText4 = new wxStaticText(Panel1_Piano, ID_STATICTEXT4, _("*** STILL UNDER DEVELOPMENT *** \n              Please do not use yet"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT4"));
    StaticText4->Hide();
    FlexGridSizer5->Add(StaticText4, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    Panel1_Piano->SetSizer(FlexGridSizer5);
    FlexGridSizer5->Fit(Panel1_Piano);
    FlexGridSizer5->SetSizeHints(Panel1_Piano);
    Panel1_Pictures = new wxPanel(Choicebook1, ID_PANEL20, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("ID_PANEL20"));
    FlexGridSizer42 = new wxFlexGridSizer(0, 1, 0, 0);
    FlexGridSizer42->AddGrowableCol(0);
    FlexGridSizer19 = new wxFlexGridSizer(0, 3, 0, 0);
    Button_Pictures_Filename = new wxButton(Panel1_Pictures, ID_BUTTON_PICTURES_FILENAME, _("File name"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON_PICTURES_FILENAME"));
    Button_Pictures_Filename->SetBackgroundColour(wxColour(224,224,224));
    FlexGridSizer19->Add(Button_Pictures_Filename, 1, wxTOP|wxLEFT|wxRIGHT|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
    BitmapButton_PicturesFilename = new wxBitmapButton(Panel1_Pictures, ID_BITMAPBUTTON_BUTTON_PICTURES_FILENAME, padlock16x16_blue_xpm, wxDefaultPosition, wxSize(22,21), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON_BUTTON_PICTURES_FILENAME"));
    BitmapButton_PicturesFilename->SetDefault();
    FlexGridSizer19->Add(BitmapButton_PicturesFilename, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 1);
    FlexGridSizer42->Add(FlexGridSizer19, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    TextCtrl_Pictures_Filename = new wxTextCtrl(Panel1_Pictures, ID_TEXTCTRL_Pictures_Filename, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_READONLY, wxDefaultValidator, _T("ID_TEXTCTRL_Pictures_Filename"));
    FlexGridSizer42->Add(TextCtrl_Pictures_Filename, 1, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
    FlexGridSizer31 = new wxFlexGridSizer(0, 3, 0, 0);
    FlexGridSizer31->AddGrowableCol(1);
    StaticText46 = new wxStaticText(Panel1_Pictures, ID_STATICTEXT46, _("Movement"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT46"));
    FlexGridSizer31->Add(StaticText46, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
    Choice_Pictures_Direction = new wxChoice(Panel1_Pictures, ID_CHOICE_Pictures_Direction, wxDefaultPosition, wxDefaultSize, 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE_Pictures_Direction"));
    FlexGridSizer31->Add(Choice_Pictures_Direction, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
    BitmapButton_PicturesDirection = new wxBitmapButton(Panel1_Pictures, ID_BITMAPBUTTON_CHOICE_Pictures_Direction, padlock16x16_blue_xpm, wxDefaultPosition, wxSize(22,21), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON_CHOICE_Pictures_Direction"));
    BitmapButton_PicturesDirection->SetDefault();
    FlexGridSizer31->Add(BitmapButton_PicturesDirection, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 1);
    StaticText96 = new wxStaticText(Panel1_Pictures, ID_STATICTEXT97, _("Anim Gif Speed"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT97"));
    FlexGridSizer31->Add(StaticText96, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
    Slider_Pictures_GifSpeed = new wxSlider(Panel1_Pictures, ID_SLIDER_Pictures_GifSpeed, 20, 1, 20, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_Pictures_GifSpeed"));
    FlexGridSizer31->Add(Slider_Pictures_GifSpeed, 1, wxALL|wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
    BitmapButton_PicturesSpeed = new wxBitmapButton(Panel1_Pictures, ID_BITMAPBUTTON_SLIDER_Pictures_GifSpeed, padlock16x16_blue_xpm, wxDefaultPosition, wxSize(22,21), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON_SLIDER_Pictures_GifSpeed"));
    BitmapButton_PicturesSpeed->SetDefault();
    FlexGridSizer31->Add(BitmapButton_PicturesSpeed, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 1);
    FlexGridSizer42->Add(FlexGridSizer31, 1, wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
    Panel1_Pictures->SetSizer(FlexGridSizer42);
    FlexGridSizer42->Fit(Panel1_Pictures);
    FlexGridSizer42->SetSizeHints(Panel1_Pictures);
    Panel1_Snowflakes = new wxPanel(Choicebook1, ID_PANEL22, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("ID_PANEL22"));
    FlexGridSizer43 = new wxFlexGridSizer(0, 3, 0, 0);
    FlexGridSizer43->AddGrowableCol(1);
    StaticText79 = new wxStaticText(Panel1_Snowflakes, ID_STATICTEXT80, _("Max flakes"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT80"));
    FlexGridSizer43->Add(StaticText79, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
    Slider_Snowflakes_Count = new wxSlider(Panel1_Snowflakes, ID_SLIDER_Snowflakes_Count, 5, 1, 20, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_Snowflakes_Count"));
    FlexGridSizer43->Add(Slider_Snowflakes_Count, 1, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
    BitmapButton_SnowflakesCount = new wxBitmapButton(Panel1_Snowflakes, ID_BITMAPBUTTON_SLIDER_Snowflakes_Count, padlock16x16_blue_xpm, wxDefaultPosition, wxSize(22,21), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON_SLIDER_Snowflakes_Count"));
    BitmapButton_SnowflakesCount->SetDefault();
    FlexGridSizer43->Add(BitmapButton_SnowflakesCount, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 1);
    StaticText80 = new wxStaticText(Panel1_Snowflakes, ID_STATICTEXT81, _("Type"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT81"));
    FlexGridSizer43->Add(StaticText80, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
    Slider_Snowflakes_Type = new wxSlider(Panel1_Snowflakes, ID_SLIDER_Snowflakes_Type, 1, 0, 5, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_Snowflakes_Type"));
    FlexGridSizer43->Add(Slider_Snowflakes_Type, 1, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
    BitmapButton_SnowflakesType = new wxBitmapButton(Panel1_Snowflakes, ID_BITMAPBUTTON_SLIDER_Snowflakes_Type, padlock16x16_blue_xpm, wxDefaultPosition, wxSize(22,21), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON_SLIDER_Snowflakes_Type"));
    BitmapButton_SnowflakesType->SetDefault();
    FlexGridSizer43->Add(BitmapButton_SnowflakesType, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 1);
    Panel1_Snowflakes->SetSizer(FlexGridSizer43);
    FlexGridSizer43->Fit(Panel1_Snowflakes);
    FlexGridSizer43->SetSizeHints(Panel1_Snowflakes);
    Panel1_Snowstorm = new wxPanel(Choicebook1, ID_PANEL24, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("ID_PANEL24"));
    FlexGridSizer44 = new wxFlexGridSizer(0, 3, 0, 0);
    FlexGridSizer44->AddGrowableCol(1);
    StaticText45 = new wxStaticText(Panel1_Snowstorm, ID_STATICTEXT45, _("Max flakes"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT45"));
    FlexGridSizer44->Add(StaticText45, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
    Slider_Snowstorm_Count = new wxSlider(Panel1_Snowstorm, ID_SLIDER_Snowstorm_Count, 50, 0, 100, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_Snowstorm_Count"));
    FlexGridSizer44->Add(Slider_Snowstorm_Count, 1, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
    BitmapButton_SnowstormCount = new wxBitmapButton(Panel1_Snowstorm, ID_BITMAPBUTTON_SLIDER_Snowstorm_Count, padlock16x16_blue_xpm, wxDefaultPosition, wxSize(22,21), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON_SLIDER_Snowstorm_Count"));
    BitmapButton_SnowstormCount->SetDefault();
    FlexGridSizer44->Add(BitmapButton_SnowstormCount, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 1);
    StaticText51 = new wxStaticText(Panel1_Snowstorm, ID_STATICTEXT51, _("Trail Length"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT51"));
    FlexGridSizer44->Add(StaticText51, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
    Slider_Snowstorm_Length = new wxSlider(Panel1_Snowstorm, ID_SLIDER_Snowstorm_Length, 50, 0, 100, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_Snowstorm_Length"));
    FlexGridSizer44->Add(Slider_Snowstorm_Length, 1, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
    BitmapButton_SnowstormLength = new wxBitmapButton(Panel1_Snowstorm, ID_BITMAPBUTTON_SLIDER_Snowstorm_Length, padlock16x16_blue_xpm, wxDefaultPosition, wxSize(22,21), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON_SLIDER_Snowstorm_Length"));
    BitmapButton_SnowstormLength->SetDefault();
    FlexGridSizer44->Add(BitmapButton_SnowstormLength, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 1);
    Panel1_Snowstorm->SetSizer(FlexGridSizer44);
    FlexGridSizer44->Fit(Panel1_Snowstorm);
    FlexGridSizer44->SetSizeHints(Panel1_Snowstorm);
    Panel1_Spirals = new wxPanel(Choicebook1, ID_PANEL26, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("ID_PANEL26"));
    FlexGridSizer45 = new wxFlexGridSizer(0, 3, 0, 0);
    FlexGridSizer45->AddGrowableCol(1);
    StaticText34 = new wxStaticText(Panel1_Spirals, ID_STATICTEXT38, _("Palette Rep"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT38"));
    FlexGridSizer45->Add(StaticText34, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    Slider_Spirals_Count = new wxSlider(Panel1_Spirals, ID_SLIDER_Spirals_Count, 1, 1, 5, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_Spirals_Count"));
    FlexGridSizer45->Add(Slider_Spirals_Count, 1, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
    BitmapButton_SpiralsCount = new wxBitmapButton(Panel1_Spirals, ID_BITMAPBUTTON_SLIDER_Spirals_Count, padlock16x16_blue_xpm, wxDefaultPosition, wxSize(22,21), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON_SLIDER_Spirals_Count"));
    BitmapButton_SpiralsCount->SetDefault();
    FlexGridSizer45->Add(BitmapButton_SpiralsCount, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 1);
    StaticText36 = new wxStaticText(Panel1_Spirals, ID_STATICTEXT40, _("Rotations"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT40"));
    FlexGridSizer45->Add(StaticText36, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    Slider_Spirals_Rotation = new wxSlider(Panel1_Spirals, ID_SLIDER_Spirals_Rotation, 20, -500, 500, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_Spirals_Rotation"));
    FlexGridSizer45->Add(Slider_Spirals_Rotation, 1, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
    BitmapButton_SpiralsRotation = new wxBitmapButton(Panel1_Spirals, ID_BITMAPBUTTON_SLIDER_Spirals_Rotation, padlock16x16_blue_xpm, wxDefaultPosition, wxSize(22,21), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON_SLIDER_Spirals_Rotation"));
    BitmapButton_SpiralsRotation->SetDefault();
    FlexGridSizer45->Add(BitmapButton_SpiralsRotation, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 1);
    StaticText38 = new wxStaticText(Panel1_Spirals, ID_STATICTEXT42, _("Thickness"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT42"));
    FlexGridSizer45->Add(StaticText38, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    Slider_Spirals_Thickness = new wxSlider(Panel1_Spirals, ID_SLIDER_Spirals_Thickness, 50, 0, 100, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_Spirals_Thickness"));
    FlexGridSizer45->Add(Slider_Spirals_Thickness, 1, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
    BitmapButton_SpiralsThickness = new wxBitmapButton(Panel1_Spirals, ID_BITMAPBUTTON_SLIDER_Spirals_Thickness, padlock16x16_blue_xpm, wxDefaultPosition, wxSize(22,21), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON_SLIDER_Spirals_Thickness"));
    BitmapButton_SpiralsThickness->SetDefault();
    FlexGridSizer45->Add(BitmapButton_SpiralsThickness, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 1);
    StaticText40 = new wxStaticText(Panel1_Spirals, ID_STATICTEXT44, _("Direction"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT44"));
    FlexGridSizer45->Add(StaticText40, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    Slider_Spirals_Direction = new wxSlider(Panel1_Spirals, ID_SLIDER_Spirals_Direction, 1, -1, 1, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_Spirals_Direction"));
    FlexGridSizer45->Add(Slider_Spirals_Direction, 1, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    BitmapButton_SpiralsDirection = new wxBitmapButton(Panel1_Spirals, ID_BITMAPBUTTON_SLIDER_Spirals_Direction, padlock16x16_blue_xpm, wxDefaultPosition, wxSize(22,21), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON_SLIDER_Spirals_Direction"));
    BitmapButton_SpiralsDirection->SetDefault();
    FlexGridSizer45->Add(BitmapButton_SpiralsDirection, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 1);
    CheckBox_Spirals_Blend = new wxCheckBox(Panel1_Spirals, ID_CHECKBOX_Spirals_Blend, _("Blend"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX_Spirals_Blend"));
    CheckBox_Spirals_Blend->SetValue(false);
    FlexGridSizer45->Add(CheckBox_Spirals_Blend, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
    FlexGridSizer20 = new wxFlexGridSizer(0, 3, 0, 0);
    BitmapButton_SpiralsBlend = new wxBitmapButton(Panel1_Spirals, ID_BITMAPBUTTON_CHECKBOX_Spirals_Blend, padlock16x16_blue_xpm, wxDefaultPosition, wxSize(22,21), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON_CHECKBOX_Spirals_Blend"));
    BitmapButton_SpiralsBlend->SetDefault();
    FlexGridSizer20->Add(BitmapButton_SpiralsBlend, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 1);
    FlexGridSizer20->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 1);
    CheckBox_Spirals_3D = new wxCheckBox(Panel1_Spirals, ID_CHECKBOX_Spirals_3D, _("3D"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX_Spirals_3D"));
    CheckBox_Spirals_3D->SetValue(false);
    FlexGridSizer20->Add(CheckBox_Spirals_3D, 1, wxALL|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 2);
    FlexGridSizer45->Add(FlexGridSizer20, 1, wxALL|wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    BitmapButton_Spirals3D = new wxBitmapButton(Panel1_Spirals, ID_BITMAPBUTTON_CHECKBOX_Spirals_3D, padlock16x16_blue_xpm, wxDefaultPosition, wxSize(22,21), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON_CHECKBOX_Spirals_3D"));
    BitmapButton_Spirals3D->SetDefault();
    FlexGridSizer45->Add(BitmapButton_Spirals3D, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 1);
    CheckBox_Spirals_Grow = new wxCheckBox(Panel1_Spirals, ID_CHECKBOX_Spirals_Grow, _("Grow"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX_Spirals_Grow"));
    CheckBox_Spirals_Grow->SetValue(false);
    FlexGridSizer45->Add(CheckBox_Spirals_Grow, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
    FlexGridSizer21 = new wxFlexGridSizer(0, 3, 0, 0);
    BitmapButton_SpiralsGrow = new wxBitmapButton(Panel1_Spirals, ID_BITMAPBUTTON_CHECKBOX_Spirals_Grow, padlock16x16_blue_xpm, wxDefaultPosition, wxSize(22,21), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON_CHECKBOX_Spirals_Grow"));
    BitmapButton_SpiralsGrow->SetDefault();
    FlexGridSizer21->Add(BitmapButton_SpiralsGrow, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 1);
    FlexGridSizer21->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 1);
    CheckBox_Spirlas_Shrink = new wxCheckBox(Panel1_Spirals, ID_CHECKBOX_Spirals_Shrink, _("Shrink"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX_Spirals_Shrink"));
    CheckBox_Spirlas_Shrink->SetValue(false);
    FlexGridSizer21->Add(CheckBox_Spirlas_Shrink, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
    FlexGridSizer45->Add(FlexGridSizer21, 1, wxALL|wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    BitmapButton_SpiralsShrink = new wxBitmapButton(Panel1_Spirals, ID_BITMAPBUTTON_CHECKBOX_Spirals_Shrink, padlock16x16_blue_xpm, wxDefaultPosition, wxSize(22,21), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON_CHECKBOX_Spirals_Shrink"));
    BitmapButton_SpiralsShrink->SetDefault();
    FlexGridSizer45->Add(BitmapButton_SpiralsShrink, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 1);
    Panel1_Spirals->SetSizer(FlexGridSizer45);
    FlexGridSizer45->Fit(Panel1_Spirals);
    FlexGridSizer45->SetSizeHints(Panel1_Spirals);
    Panel1_Spirograph = new wxPanel(Choicebook1, ID_PANEL35, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("ID_PANEL35"));
    FlexGridSizer72 = new wxFlexGridSizer(0, 3, 0, 0);
    FlexGridSizer72->AddGrowableCol(1);
    StaticText87 = new wxStaticText(Panel1_Spirograph, ID_STATICTEXT88, _("R - Radius of outer circle"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT88"));
    FlexGridSizer72->Add(StaticText87, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
    Slider_Spirograph_R = new wxSlider(Panel1_Spirograph, ID_SLIDER_Spirograph_R, 20, 1, 100, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_Spirograph_R"));
    FlexGridSizer72->Add(Slider_Spirograph_R, 1, wxALL|wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
    BitmapButton_SpirographR = new wxBitmapButton(Panel1_Spirograph, ID_BITMAPBUTTON_SLIDER_Spirograph_R, padlock16x16_blue_xpm, wxDefaultPosition, wxSize(22,21), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON_SLIDER_Spirograph_R"));
    BitmapButton_SpirographR->SetDefault();
    FlexGridSizer72->Add(BitmapButton_SpirographR, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 1);
    StaticText88 = new wxStaticText(Panel1_Spirograph, ID_STATICTEXT89, _("r - Radius of inner circle"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT89"));
    FlexGridSizer72->Add(StaticText88, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
    Slider_Spirograph_r = new wxSlider(Panel1_Spirograph, ID_SLIDER_Spirograph_r, 10, 1, 100, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_Spirograph_r"));
    FlexGridSizer72->Add(Slider_Spirograph_r, 1, wxALL|wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
    BitmapButton_Spirographr = new wxBitmapButton(Panel1_Spirograph, ID_BITMAPBUTTON_SLIDER_Spirograph_r, padlock16x16_blue_xpm, wxDefaultPosition, wxSize(22,21), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON_SLIDER_Spirograph_r"));
    BitmapButton_Spirographr->SetDefault();
    FlexGridSizer72->Add(BitmapButton_Spirographr, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 1);
    StaticText89 = new wxStaticText(Panel1_Spirograph, ID_STATICTEXT90, _("d - Distance"), wxDefaultPosition, wxSize(84,13), 0, _T("ID_STATICTEXT90"));
    FlexGridSizer72->Add(StaticText89, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
    Slider_Spirograph_d = new wxSlider(Panel1_Spirograph, ID_SLIDER_Spirograph_d, 30, 1, 100, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_Spirograph_d"));
    FlexGridSizer72->Add(Slider_Spirograph_d, 1, wxALL|wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
    BitmapButton_SpirographAnimate = new wxBitmapButton(Panel1_Spirograph, ID_BITMAPBUTTON_SLIDER_Spirograph_d, padlock16x16_blue_xpm, wxDefaultPosition, wxSize(22,21), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON_SLIDER_Spirograph_d"));
    BitmapButton_SpirographAnimate->SetDefault();
    FlexGridSizer72->Add(BitmapButton_SpirographAnimate, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 1);
    StaticText92 = new wxStaticText(Panel1_Spirograph, ID_STATICTEXT93, _("Should distance\nbe animated\?"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT93"));
    FlexGridSizer72->Add(StaticText92, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
    CheckBox_Spirograph_Animate = new wxCheckBox(Panel1_Spirograph, ID_CHECKBOX_Spirograph_Animate, _("Yes"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX_Spirograph_Animate"));
    CheckBox_Spirograph_Animate->SetValue(false);
    FlexGridSizer72->Add(CheckBox_Spirograph_Animate, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
    BitmapButton_Spirographd = new wxBitmapButton(Panel1_Spirograph, ID_BITMAPBUTTON_CHECKBOX_Spirograph_Animate, padlock16x16_blue_xpm, wxDefaultPosition, wxSize(22,21), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON_CHECKBOX_Spirograph_Animate"));
    BitmapButton_Spirographd->SetDefault();
    FlexGridSizer72->Add(BitmapButton_Spirographd, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 1);
    StaticText90 = new wxStaticText(Panel1_Spirograph, ID_STATICTEXT91, _("Note: r should be <= R"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT91"));
    FlexGridSizer72->Add(StaticText90, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
    Panel1_Spirograph->SetSizer(FlexGridSizer72);
    FlexGridSizer72->Fit(Panel1_Spirograph);
    FlexGridSizer72->SetSizeHints(Panel1_Spirograph);
    Panel1_Text = new wxPanel(Choicebook1, ID_PANEL7, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("ID_PANEL7"));
    FlexGridSizer46 = new wxFlexGridSizer(0, 1, 0, 0);
    FlexGridSizer46->AddGrowableCol(0);
    FlexGridSizer65 = new wxFlexGridSizer(4, 0, 0, 0);
    FlexGridSizer65->AddGrowableCol(0);
    StaticText53 = new wxStaticText(Panel1_Text, ID_STATICTEXT53, _("Line 1"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT53"));
    FlexGridSizer65->Add(StaticText53, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
    TextCtrl_Text_Line1 = new wxTextCtrl(Panel1_Text, ID_TEXTCTRL_Text_Line1, wxEmptyString, wxDefaultPosition, wxSize(216,21), wxTE_PROCESS_ENTER, wxDefaultValidator, _T("ID_TEXTCTRL_Text_Line1"));
    TextCtrl_Text_Line1->SetMaxLength(256);
    FlexGridSizer65->Add(TextCtrl_Text_Line1, 1, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
    BitmapButton_TextLine1 = new wxBitmapButton(Panel1_Text, ID_BITMAPBUTTON_TEXTCTRL_Text_Line1, padlock16x16_blue_xpm, wxDefaultPosition, wxSize(20,20), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON_TEXTCTRL_Text_Line1"));
    BitmapButton_TextLine1->SetDefault();
    BitmapButton_TextLine1->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_ACTIVECAPTION));
    FlexGridSizer65->Add(BitmapButton_TextLine1, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 1);
    StaticText54 = new wxStaticText(Panel1_Text, ID_STATICTEXT54, _("Line 2"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT54"));
    FlexGridSizer65->Add(StaticText54, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
    TextCtrl_Text_Line2 = new wxTextCtrl(Panel1_Text, ID_TEXTCTRL_Text_Line2, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_PROCESS_ENTER, wxDefaultValidator, _T("ID_TEXTCTRL_Text_Line2"));
    TextCtrl_Text_Line2->SetMaxLength(256);
    FlexGridSizer65->Add(TextCtrl_Text_Line2, 1, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
    BitmapButton_TextLine2 = new wxBitmapButton(Panel1_Text, ID_BITMAPBUTTON_TEXTCTRL_Text_Line2, padlock16x16_blue_xpm, wxDefaultPosition, wxSize(20,20), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON_TEXTCTRL_Text_Line2"));
    BitmapButton_TextLine2->SetDefault();
    BitmapButton_TextLine2->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_ACTIVECAPTION));
    FlexGridSizer65->Add(BitmapButton_TextLine2, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 1);
    StaticText19 = new wxStaticText(Panel1_Text, ID_STATICTEXT22, _("Line 3"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT22"));
    FlexGridSizer65->Add(StaticText19, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
    TextCtrl_Text_Line3 = new wxTextCtrl(Panel1_Text, ID_TEXTCTRL_Text_Line3, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_PROCESS_ENTER, wxDefaultValidator, _T("ID_TEXTCTRL_Text_Line3"));
    TextCtrl_Text_Line3->SetMaxLength(256);
    FlexGridSizer65->Add(TextCtrl_Text_Line3, 1, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
    BitmapButton_TextLine3 = new wxBitmapButton(Panel1_Text, ID_BITMAPBUTTON_TEXTCTRL_Text_Line3, padlock16x16_blue_xpm, wxDefaultPosition, wxSize(20,20), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON_TEXTCTRL_Text_Line3"));
    BitmapButton_TextLine3->SetDefault();
    BitmapButton_TextLine3->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_ACTIVECAPTION));
    FlexGridSizer65->Add(BitmapButton_TextLine3, 0, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 0);
    StaticText20 = new wxStaticText(Panel1_Text, ID_STATICTEXT23, _("Line 4"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT23"));
    FlexGridSizer65->Add(StaticText20, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
    TextCtrl_Text_Line4 = new wxTextCtrl(Panel1_Text, ID_TEXTCTRL_Text_Line4, wxEmptyString, wxDefaultPosition, wxSize(174,21), wxTE_PROCESS_ENTER, wxDefaultValidator, _T("ID_TEXTCTRL_Text_Line4"));
    TextCtrl_Text_Line4->SetMaxLength(256);
    FlexGridSizer65->Add(TextCtrl_Text_Line4, 1, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
    BitmapButton_TextLine4 = new wxBitmapButton(Panel1_Text, ID_BITMAPBUTTON_TEXTCTRL_Text_Line4, padlock16x16_blue_xpm, wxDefaultPosition, wxSize(20,20), wxBU_AUTODRAW, wxDefaultValidator, _T("ID_BITMAPBUTTON_TEXTCTRL_Text_Line4"));
    BitmapButton_TextLine4->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_ACTIVECAPTION));
    FlexGridSizer65->Add(BitmapButton_TextLine4, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 1);
    FlexGridSizer46->Add(FlexGridSizer65, 1, wxBOTTOM|wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
    Notebook_Text1 = new wxNotebook(Panel1_Text, ID_NOTEBOOK_Text1, wxDefaultPosition, wxDefaultSize, 0, _T("ID_NOTEBOOK_Text1"));
    Notebook_Text1->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_SCROLLBAR));
    Panel_Text1 = new wxPanel(Notebook_Text1, ID_PANEL_Text1, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("ID_PANEL_Text1"));
    FlexGridSizer69 = new wxFlexGridSizer(0, 3, 0, 0);
    FlexGridSizer69->AddGrowableCol(1);
    Button_Text_Font1 = new wxButton(Panel_Text1, ID_BUTTON_TEXT_FONT1, _("Font1"), wxDefaultPosition, wxSize(48,23), 0, wxDefaultValidator, _T("ID_BUTTON_TEXT_FONT1"));
    FlexGridSizer69->Add(Button_Text_Font1, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
    TextCtrl_Text_Font1 = new wxTextCtrl(Panel_Text1, ID_TEXTCTRL_Text_Font1, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_READONLY, wxDefaultValidator, _T("ID_TEXTCTRL_Text_Font1"));
    FlexGridSizer69->Add(TextCtrl_Text_Font1, 1, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
    BitmapButton_TextFont1 = new wxBitmapButton(Panel_Text1, ID_BITMAPBUTTON_TEXTCTRL_Text_Font1, padlock16x16_blue_xpm, wxDefaultPosition, wxSize(20,20), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON_TEXTCTRL_Text_Font1"));
    BitmapButton_TextFont1->SetDefault();
    BitmapButton_TextFont1->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_ACTIVECAPTION));
    FlexGridSizer69->Add(BitmapButton_TextFont1, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 1);
    StaticText78 = new wxStaticText(Panel_Text1, ID_STATICTEXT79, _("Movement"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT79"));
    FlexGridSizer69->Add(StaticText78, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
    FlexGridSizer48 = new wxFlexGridSizer(0, 3, 0, 0);
    Choice_Text_Dir1 = new wxChoice(Panel_Text1, ID_CHOICE_Text_Dir1, wxDefaultPosition, wxDefaultSize, 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE_Text_Dir1"));
    FlexGridSizer48->Add(Choice_Text_Dir1, 1, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
    BitmapButton_TextDir1 = new wxBitmapButton(Panel_Text1, ID_BITMAPBUTTON_CHOICE_Text_Dir1, padlock16x16_blue_xpm, wxDefaultPosition, wxSize(20,20), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON_CHOICE_Text_Dir1"));
    BitmapButton_TextDir1->SetDefault();
    BitmapButton_TextDir1->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_ACTIVECAPTION));
    FlexGridSizer48->Add(BitmapButton_TextDir1, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 1);
    CheckBox_TextToCenter1 = new wxCheckBox(Panel_Text1, ID_CHECKBOX_TextToCenter1, _("C"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX_TextToCenter1"));
    CheckBox_TextToCenter1->SetValue(false);
    CheckBox_TextToCenter1->SetToolTip(_("Move to center and stop"));
    FlexGridSizer48->Add(CheckBox_TextToCenter1, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizer69->Add(FlexGridSizer48, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 1);
    BitmapButton_TextToCenter1 = new wxBitmapButton(Panel_Text1, ID_BITMAPBUTTON_TextToCenter1, padlock16x16_blue_xpm, wxDefaultPosition, wxSize(20,20), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON_TextToCenter1"));
    BitmapButton_TextToCenter1->SetDefault();
    BitmapButton_TextToCenter1->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_ACTIVECAPTION));
    FlexGridSizer69->Add(BitmapButton_TextToCenter1, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    StaticText107 = new wxStaticText(Panel_Text1, ID_STATICTEXT108, _("Effect"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT108"));
    FlexGridSizer69->Add(StaticText107, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
    Choice_Text_Effect1 = new wxChoice(Panel_Text1, ID_CHOICE_Text_Effect1, wxDefaultPosition, wxDefaultSize, 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE_Text_Effect1"));
    FlexGridSizer69->Add(Choice_Text_Effect1, 1, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
    BitmapButton_TextEffect1 = new wxBitmapButton(Panel_Text1, ID_BITMAPBUTTON_CHOICE_Text_Effect1, padlock16x16_blue_xpm, wxDefaultPosition, wxSize(20,20), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON_CHOICE_Text_Effect1"));
    BitmapButton_TextEffect1->SetDefault();
    BitmapButton_TextEffect1->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_ACTIVECAPTION));
    FlexGridSizer69->Add(BitmapButton_TextEffect1, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 1);
    StaticText98 = new wxStaticText(Panel_Text1, ID_STATICTEXT99, _("Count down"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT99"));
    FlexGridSizer69->Add(StaticText98, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
    Choice_Text_Count1 = new wxChoice(Panel_Text1, ID_CHOICE_Text_Count1, wxDefaultPosition, wxDefaultSize, 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE_Text_Count1"));
    FlexGridSizer69->Add(Choice_Text_Count1, 1, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
    BitmapButton_TextCount1 = new wxBitmapButton(Panel_Text1, ID_BITMAPBUTTON_CHOICE_Text_Count1, padlock16x16_blue_xpm, wxDefaultPosition, wxSize(20,20), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON_CHOICE_Text_Count1"));
    BitmapButton_TextCount1->SetDefault();
    BitmapButton_TextCount1->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_ACTIVECAPTION));
    FlexGridSizer69->Add(BitmapButton_TextCount1, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 1);
    StaticText100 = new wxStaticText(Panel_Text1, ID_STATICTEXT101, _("Position"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT101"));
    FlexGridSizer69->Add(StaticText100, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
    Slider_Text_Position1 = new wxSlider(Panel_Text1, ID_SLIDER_Text_Position1, 50, 0, 100, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_Text_Position1"));
    FlexGridSizer69->Add(Slider_Text_Position1, 1, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
    BitmapButton_TextPosition1 = new wxBitmapButton(Panel_Text1, ID_BITMAPBUTTON_SLIDER_Text_Position1, padlock16x16_blue_xpm, wxDefaultPosition, wxSize(20,20), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON_SLIDER_Text_Position1"));
    BitmapButton_TextPosition1->SetDefault();
    BitmapButton_TextPosition1->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_ACTIVECAPTION));
    FlexGridSizer69->Add(BitmapButton_TextPosition1, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 1);
    Panel_Text1->SetSizer(FlexGridSizer69);
    FlexGridSizer69->Fit(Panel_Text1);
    FlexGridSizer69->SetSizeHints(Panel_Text1);
    Panel_Text2 = new wxPanel(Notebook_Text1, ID_PANEL_Text2, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("ID_PANEL_Text2"));
    FlexGridSizer74 = new wxFlexGridSizer(0, 3, 0, 0);
    FlexGridSizer74->AddGrowableCol(1);
    Button_Text_Font2 = new wxButton(Panel_Text2, ID_BUTTON_TEXT_FONT2, _("Font2"), wxDefaultPosition, wxSize(48,23), 0, wxDefaultValidator, _T("ID_BUTTON_TEXT_FONT2"));
    FlexGridSizer74->Add(Button_Text_Font2, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
    TextCtrl_Text_Font2 = new wxTextCtrl(Panel_Text2, ID_TEXTCTRL_Text_Font2, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_READONLY, wxDefaultValidator, _T("ID_TEXTCTRL_Text_Font2"));
    FlexGridSizer74->Add(TextCtrl_Text_Font2, 1, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
    BitmapButton_TextFont2 = new wxBitmapButton(Panel_Text2, ID_BITMAPBUTTON_TEXTCTRL_Text_Font2, padlock16x16_blue_xpm, wxDefaultPosition, wxSize(20,20), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON_TEXTCTRL_Text_Font2"));
    BitmapButton_TextFont2->SetDefault();
    BitmapButton_TextFont2->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_ACTIVECAPTION));
    FlexGridSizer74->Add(BitmapButton_TextFont2, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 1);
    StaticText57 = new wxStaticText(Panel_Text2, ID_STATICTEXT57, _("Movement"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT57"));
    FlexGridSizer74->Add(StaticText57, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
    FlexGridSizer49 = new wxFlexGridSizer(0, 3, 0, 0);
    Choice_Text_Dir2 = new wxChoice(Panel_Text2, ID_CHOICE_Text_Dir2, wxDefaultPosition, wxDefaultSize, 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE_Text_Dir2"));
    FlexGridSizer49->Add(Choice_Text_Dir2, 1, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
    BitmapButton_TextDir2 = new wxBitmapButton(Panel_Text2, ID_BITMAPBUTTON_CHOICE_Text_Dir2, padlock16x16_blue_xpm, wxDefaultPosition, wxSize(20,20), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON_CHOICE_Text_Dir2"));
    BitmapButton_TextDir2->SetDefault();
    BitmapButton_TextDir2->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_ACTIVECAPTION));
    FlexGridSizer49->Add(BitmapButton_TextDir2, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 1);
    CheckBox_TextToCenter2 = new wxCheckBox(Panel_Text2, ID_CHECKBOX_TextToCenter2, _("C"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX_TextToCenter2"));
    CheckBox_TextToCenter2->SetValue(false);
    CheckBox_TextToCenter2->SetToolTip(_("Move to center and stop"));
    FlexGridSizer49->Add(CheckBox_TextToCenter2, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizer74->Add(FlexGridSizer49, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 1);
    BitmapButton_TextToCenter2 = new wxBitmapButton(Panel_Text2, ID_BITMAPBUTTON_Text2Center2, padlock16x16_blue_xpm, wxDefaultPosition, wxSize(20,20), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON_Text2Center2"));
    BitmapButton_TextToCenter2->SetDefault();
    BitmapButton_TextToCenter2->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_ACTIVECAPTION));
    FlexGridSizer74->Add(BitmapButton_TextToCenter2, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    StaticText108 = new wxStaticText(Panel_Text2, ID_STATICTEXT109, _("Effect"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT109"));
    FlexGridSizer74->Add(StaticText108, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
    Choice_Text_Effect2 = new wxChoice(Panel_Text2, ID_CHOICE_Text_Effect2, wxDefaultPosition, wxDefaultSize, 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE_Text_Effect2"));
    FlexGridSizer74->Add(Choice_Text_Effect2, 1, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
    BitmapButton_TextEffect2 = new wxBitmapButton(Panel_Text2, ID_BITMAPBUTTON_CHOICE_Text_Effect2, padlock16x16_blue_xpm, wxDefaultPosition, wxSize(20,20), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON_CHOICE_Text_Effect2"));
    BitmapButton_TextEffect2->SetDefault();
    BitmapButton_TextEffect2->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_ACTIVECAPTION));
    FlexGridSizer74->Add(BitmapButton_TextEffect2, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 1);
    StaticText103 = new wxStaticText(Panel_Text2, ID_STATICTEXT104, _("Count down"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT104"));
    FlexGridSizer74->Add(StaticText103, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
    Choice_Text_Count2 = new wxChoice(Panel_Text2, ID_CHOICE_Text_Count2, wxDefaultPosition, wxDefaultSize, 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE_Text_Count2"));
    FlexGridSizer74->Add(Choice_Text_Count2, 1, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
    BitmapButton_TextCount2 = new wxBitmapButton(Panel_Text2, ID_BITMAPBUTTON_CHOICE_Text_Count2, padlock16x16_blue_xpm, wxDefaultPosition, wxSize(20,20), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON_CHOICE_Text_Count2"));
    BitmapButton_TextCount2->SetDefault();
    BitmapButton_TextCount2->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_ACTIVECAPTION));
    FlexGridSizer74->Add(BitmapButton_TextCount2, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 1);
    StaticText102 = new wxStaticText(Panel_Text2, ID_STATICTEXT103, _("Position"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT103"));
    FlexGridSizer74->Add(StaticText102, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
    Slider_Text_Position2 = new wxSlider(Panel_Text2, ID_SLIDER_Text_Position2, 50, 0, 100, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_Text_Position2"));
    FlexGridSizer74->Add(Slider_Text_Position2, 1, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
    BitmapButton_TextPosition2 = new wxBitmapButton(Panel_Text2, ID_BITMAPBUTTON_SLIDER_Text_Position2, padlock16x16_blue_xpm, wxDefaultPosition, wxSize(20,20), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON_SLIDER_Text_Position2"));
    BitmapButton_TextPosition2->SetDefault();
    BitmapButton_TextPosition2->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_ACTIVECAPTION));
    FlexGridSizer74->Add(BitmapButton_TextPosition2, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 1);
    Panel_Text2->SetSizer(FlexGridSizer74);
    FlexGridSizer74->Fit(Panel_Text2);
    FlexGridSizer74->SetSizeHints(Panel_Text2);
    Panel_Text3 = new wxPanel(Notebook_Text1, ID_PANEL_Text3, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("ID_PANEL_Text3"));
    FlexGridSizer25 = new wxFlexGridSizer(0, 3, 0, 0);
    FlexGridSizer25->AddGrowableCol(1);
    Button_Text_Font3 = new wxButton(Panel_Text3, ID_BUTTON_TEXT_FONT3, _("Font3"), wxDefaultPosition, wxSize(48,23), 0, wxDefaultValidator, _T("ID_BUTTON_TEXT_FONT3"));
    FlexGridSizer25->Add(Button_Text_Font3, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
    TextCtrl_Text_Font3 = new wxTextCtrl(Panel_Text3, ID_TEXTCTRL_Text_Font3, wxEmptyString, wxDefaultPosition, wxSize(180,23), wxTE_READONLY, wxDefaultValidator, _T("ID_TEXTCTRL_Text_Font3"));
    FlexGridSizer25->Add(TextCtrl_Text_Font3, 1, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
    BitmapButton_TextFont3 = new wxBitmapButton(Panel_Text3, ID_BITMAPBUTTON_TEXTCTRL_Text_Font3, padlock16x16_blue_xpm, wxDefaultPosition, wxSize(20,20), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON_TEXTCTRL_Text_Font3"));
    BitmapButton_TextFont3->SetDefault();
    BitmapButton_TextFont3->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_ACTIVECAPTION));
    FlexGridSizer25->Add(BitmapButton_TextFont3, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 1);
    StaticText15 = new wxStaticText(Panel_Text3, ID_STATICTEXT16, _("Movement"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT16"));
    FlexGridSizer25->Add(StaticText15, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
    FlexGridSizer50 = new wxFlexGridSizer(0, 3, 0, 0);
    Choice_Text_Dir3 = new wxChoice(Panel_Text3, ID_CHOICE_Text_Dir3, wxDefaultPosition, wxDefaultSize, 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE_Text_Dir3"));
    FlexGridSizer50->Add(Choice_Text_Dir3, 1, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
    BitmapButton_TextDir3 = new wxBitmapButton(Panel_Text3, ID_BITMAPBUTTON_CHOICE_Text_Dir3, padlock16x16_blue_xpm, wxDefaultPosition, wxSize(20,20), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON_CHOICE_Text_Dir3"));
    BitmapButton_TextDir3->SetDefault();
    BitmapButton_TextDir3->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_ACTIVECAPTION));
    FlexGridSizer50->Add(BitmapButton_TextDir3, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 1);
    CheckBox_TextToCenter3 = new wxCheckBox(Panel_Text3, ID_CHECKBOX_TextToCenter3, _("C"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX_TextToCenter3"));
    CheckBox_TextToCenter3->SetValue(false);
    CheckBox_TextToCenter3->SetToolTip(_("Move to center and stop"));
    FlexGridSizer50->Add(CheckBox_TextToCenter3, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizer25->Add(FlexGridSizer50, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 1);
    BitmapButton_TextToCenter3 = new wxBitmapButton(Panel_Text3, ID_BITMAPBUTTON_TextToCenter3, padlock16x16_blue_xpm, wxDefaultPosition, wxSize(20,20), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON_TextToCenter3"));
    BitmapButton_TextToCenter3->SetDefault();
    BitmapButton_TextToCenter3->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_ACTIVECAPTION));
    FlexGridSizer25->Add(BitmapButton_TextToCenter3, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    StaticText16 = new wxStaticText(Panel_Text3, ID_STATICTEXT17, _("Effect"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT17"));
    FlexGridSizer25->Add(StaticText16, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
    Choice_Text_Effect3 = new wxChoice(Panel_Text3, ID_CHOICE_Text_Effect3, wxDefaultPosition, wxDefaultSize, 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE_Text_Effect3"));
    FlexGridSizer25->Add(Choice_Text_Effect3, 1, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
    BitmapButton_TextEffect3 = new wxBitmapButton(Panel_Text3, ID_BITMAPBUTTON_CHOICE_Text_Effect3, padlock16x16_blue_xpm, wxDefaultPosition, wxSize(20,20), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON_CHOICE_Text_Effect3"));
    BitmapButton_TextEffect3->SetDefault();
    BitmapButton_TextEffect3->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_ACTIVECAPTION));
    FlexGridSizer25->Add(BitmapButton_TextEffect3, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 1);
    StaticText17 = new wxStaticText(Panel_Text3, ID_STATICTEXT18, _("Count down"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT18"));
    FlexGridSizer25->Add(StaticText17, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
    Choice_Text_Count3 = new wxChoice(Panel_Text3, ID_CHOICE_Text_Count3, wxDefaultPosition, wxDefaultSize, 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE_Text_Count3"));
    FlexGridSizer25->Add(Choice_Text_Count3, 1, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
    BitmapButton_TextCount3 = new wxBitmapButton(Panel_Text3, ID_BITMAPBUTTON_CHOICE_Text_Count3, padlock16x16_blue_xpm, wxDefaultPosition, wxSize(20,20), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON_CHOICE_Text_Count3"));
    BitmapButton_TextCount3->SetDefault();
    BitmapButton_TextCount3->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_ACTIVECAPTION));
    FlexGridSizer25->Add(BitmapButton_TextCount3, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 1);
    StaticText18 = new wxStaticText(Panel_Text3, ID_STATICTEXT19, _("Position"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT19"));
    FlexGridSizer25->Add(StaticText18, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
    Slider_Text_Position3 = new wxSlider(Panel_Text3, ID_SLIDER_Text_Position3, 50, 0, 100, wxDefaultPosition, wxSize(177,24), 0, wxDefaultValidator, _T("ID_SLIDER_Text_Position3"));
    FlexGridSizer25->Add(Slider_Text_Position3, 1, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
    BitmapButton_TextPosition3 = new wxBitmapButton(Panel_Text3, ID_BITMAPBUTTON_SLIDER_Text_Position3, padlock16x16_blue_xpm, wxDefaultPosition, wxSize(20,20), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON_SLIDER_Text_Position3"));
    BitmapButton_TextPosition3->SetDefault();
    BitmapButton_TextPosition3->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_ACTIVECAPTION));
    FlexGridSizer25->Add(BitmapButton_TextPosition3, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 1);
    Panel_Text3->SetSizer(FlexGridSizer25);
    FlexGridSizer25->Fit(Panel_Text3);
    FlexGridSizer25->SetSizeHints(Panel_Text3);
    Panel_Text4 = new wxPanel(Notebook_Text1, ID_PANEL_Text4, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("ID_PANEL_Text4"));
    FlexGridSizer29 = new wxFlexGridSizer(0, 3, 0, 0);
    FlexGridSizer29->AddGrowableCol(1);
    Button_Text_Font4 = new wxButton(Panel_Text4, ID_BUTTON_TEXT_FONT4, _("Font4"), wxDefaultPosition, wxSize(48,23), 0, wxDefaultValidator, _T("ID_BUTTON_TEXT_FONT4"));
    FlexGridSizer29->Add(Button_Text_Font4, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
    TextCtrl_Text_Font4 = new wxTextCtrl(Panel_Text4, ID_TEXTCTRL_Text_Font4, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_READONLY, wxDefaultValidator, _T("ID_TEXTCTRL_Text_Font4"));
    FlexGridSizer29->Add(TextCtrl_Text_Font4, 1, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
    BitmapButton_TextFont4 = new wxBitmapButton(Panel_Text4, ID_BITMAPBUTTON_TEXTCTRL_Text_Font4, padlock16x16_blue_xpm, wxDefaultPosition, wxSize(20,20), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON_TEXTCTRL_Text_Font4"));
    BitmapButton_TextFont4->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_ACTIVECAPTION));
    FlexGridSizer29->Add(BitmapButton_TextFont4, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 1);
    StaticText22 = new wxStaticText(Panel_Text4, ID_STATICTEXT24, _("Movement"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT24"));
    FlexGridSizer29->Add(StaticText22, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
    FlexGridSizer52 = new wxFlexGridSizer(0, 3, 0, 0);
    Choice_Text_Dir4 = new wxChoice(Panel_Text4, ID_CHOICE_Text_Dir4, wxDefaultPosition, wxDefaultSize, 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE_Text_Dir4"));
    FlexGridSizer52->Add(Choice_Text_Dir4, 1, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
    BitmapButton_TextDir4 = new wxBitmapButton(Panel_Text4, ID_BITMAPBUTTON_CHOICE_Text_Dir4, padlock16x16_blue_xpm, wxDefaultPosition, wxSize(20,20), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON_CHOICE_Text_Dir4"));
    BitmapButton_TextDir4->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_ACTIVECAPTION));
    FlexGridSizer52->Add(BitmapButton_TextDir4, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 1);
    CheckBox_TextToCenter4 = new wxCheckBox(Panel_Text4, ID_CHECKBOX_TextToCenter4, _("C"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX_TextToCenter4"));
    CheckBox_TextToCenter4->SetValue(false);
    CheckBox_TextToCenter4->SetToolTip(_("Move to center and stop"));
    FlexGridSizer52->Add(CheckBox_TextToCenter4, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizer29->Add(FlexGridSizer52, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 1);
    BitmapButton_TextToCenter4 = new wxBitmapButton(Panel_Text4, ID_BITMAPBUTTON_TextToCenter4, padlock16x16_blue_xpm, wxDefaultPosition, wxSize(20,20), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON_TextToCenter4"));
    BitmapButton_TextToCenter4->SetDefault();
    BitmapButton_TextToCenter4->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_ACTIVECAPTION));
    FlexGridSizer29->Add(BitmapButton_TextToCenter4, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    StaticText29 = new wxStaticText(Panel_Text4, ID_STATICTEXT29, _("Effect"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT29"));
    FlexGridSizer29->Add(StaticText29, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
    Choice_Text_Effect4 = new wxChoice(Panel_Text4, ID_CHOICE_Text_Effect4, wxDefaultPosition, wxDefaultSize, 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE_Text_Effect4"));
    FlexGridSizer29->Add(Choice_Text_Effect4, 1, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
    BitmapButton_TextEffect4 = new wxBitmapButton(Panel_Text4, ID_BITMAPBUTTON_CHOICE_Text_Effect4, padlock16x16_blue_xpm, wxDefaultPosition, wxSize(20,20), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON_CHOICE_Text_Effect4"));
    BitmapButton_TextEffect4->SetDefault();
    BitmapButton_TextEffect4->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_ACTIVECAPTION));
    FlexGridSizer29->Add(BitmapButton_TextEffect4, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 1);
    StaticText42 = new wxStaticText(Panel_Text4, ID_STATICTEXT47, _("Count down"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT47"));
    FlexGridSizer29->Add(StaticText42, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
    Choice_Text_Count4 = new wxChoice(Panel_Text4, ID_CHOICE_Text_Count4, wxDefaultPosition, wxDefaultSize, 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE_Text_Count4"));
    FlexGridSizer29->Add(Choice_Text_Count4, 1, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
    BitmapButton_TextCount4 = new wxBitmapButton(Panel_Text4, ID_BITMAPBUTTON_CHOICE_Text_Count4, padlock16x16_blue_xpm, wxDefaultPosition, wxSize(20,20), wxBU_AUTODRAW, wxDefaultValidator, _T("ID_BITMAPBUTTON_CHOICE_Text_Count4"));
    BitmapButton_TextCount4->SetDefault();
    BitmapButton_TextCount4->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_ACTIVECAPTION));
    FlexGridSizer29->Add(BitmapButton_TextCount4, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 1);
    StaticText44 = new wxStaticText(Panel_Text4, ID_STATICTEXT48, _("Position"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT48"));
    FlexGridSizer29->Add(StaticText44, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
    Slider_Text_Position4 = new wxSlider(Panel_Text4, ID_SLIDER_Text_Position4, 50, 0, 100, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_Text_Position4"));
    FlexGridSizer29->Add(Slider_Text_Position4, 1, wxALL|wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
    BitmapButton_TextPosition4 = new wxBitmapButton(Panel_Text4, ID_BITMAPBUTTON_SLIDER_Text_Position4, padlock16x16_blue_xpm, wxDefaultPosition, wxSize(20,20), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON_SLIDER_Text_Position4"));
    BitmapButton_TextPosition4->SetDefault();
    BitmapButton_TextPosition4->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_ACTIVECAPTION));
    FlexGridSizer29->Add(BitmapButton_TextPosition4, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 1);
    Panel_Text4->SetSizer(FlexGridSizer29);
    FlexGridSizer29->Fit(Panel_Text4);
    FlexGridSizer29->SetSizeHints(Panel_Text4);
    Notebook_Text1->AddPage(Panel_Text1, _("Line1 Font"), false);
    Notebook_Text1->AddPage(Panel_Text2, _("Line2 Font"), false);
    Notebook_Text1->AddPage(Panel_Text3, _("Line3 Font"), false);
    Notebook_Text1->AddPage(Panel_Text4, _("Line4 Font"), false);
    FlexGridSizer46->Add(Notebook_Text1, 0, wxEXPAND|wxALIGN_LEFT|wxALIGN_TOP, 0);
    Panel1_Text->SetSizer(FlexGridSizer46);
    FlexGridSizer46->Fit(Panel1_Text);
    FlexGridSizer46->SetSizeHints(Panel1_Text);
    Panel1_Tree = new wxPanel(Choicebook1, ID_PANEL34, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("ID_PANEL34"));
    FlexGridSizer71 = new wxFlexGridSizer(5, 3, 0, 0);
    FlexGridSizer71->AddGrowableCol(1);
    StaticText86 = new wxStaticText(Panel1_Tree, ID_STATICTEXT87, _("Number Branches"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT87"));
    FlexGridSizer71->Add(StaticText86, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    Slider_Tree_Branches = new wxSlider(Panel1_Tree, ID_SLIDER_Tree_Branches, 3, 1, 10, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_Tree_Branches"));
    FlexGridSizer71->Add(Slider_Tree_Branches, 1, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    BitmapButton_TreeBranches = new wxBitmapButton(Panel1_Tree, ID_BITMAPBUTTON_SLIDER_Tree_Branches, padlock16x16_blue_xpm, wxDefaultPosition, wxSize(22,21), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON_SLIDER_Tree_Branches"));
    BitmapButton_TreeBranches->SetDefault();
    FlexGridSizer71->Add(BitmapButton_TreeBranches, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 1);
    Panel1_Tree->SetSizer(FlexGridSizer71);
    FlexGridSizer71->Fit(Panel1_Tree);
    FlexGridSizer71->SetSizeHints(Panel1_Tree);
    Panel1_Twinkle = new wxPanel(Choicebook1, ID_PANEL33, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("ID_PANEL33"));
    FlexGridSizer4 = new wxFlexGridSizer(0, 3, 0, 0);
    FlexGridSizer4->AddGrowableCol(1);
    StaticText85 = new wxStaticText(Panel1_Twinkle, ID_STATICTEXT86, _("Number Lights"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT86"));
    FlexGridSizer4->Add(StaticText85, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    Slider_Twinkle_Count = new wxSlider(Panel1_Twinkle, ID_SLIDER_Twinkle_Count, 3, 2, 100, wxDefaultPosition, wxSize(117,24), 0, wxDefaultValidator, _T("ID_SLIDER_Twinkle_Count"));
    FlexGridSizer4->Add(Slider_Twinkle_Count, 1, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    BitmapButton_TwinkleCount = new wxBitmapButton(Panel1_Twinkle, ID_BITMAPBUTTON_SLIDER_Twinkle_Count, padlock16x16_blue_xpm, wxDefaultPosition, wxSize(22,21), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON_SLIDER_Twinkle_Count"));
    BitmapButton_TwinkleCount->SetDefault();
    FlexGridSizer4->Add(BitmapButton_TwinkleCount, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 1);
    StaticText104 = new wxStaticText(Panel1_Twinkle, ID_STATICTEXT105, _("Twinkle Steps"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT105"));
    FlexGridSizer4->Add(StaticText104, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    Slider_Twinkle_Steps = new wxSlider(Panel1_Twinkle, ID_SLIDER_Twinkle_Steps, 30, 2, 200, wxDefaultPosition, wxSize(119,24), 0, wxDefaultValidator, _T("ID_SLIDER_Twinkle_Steps"));
    FlexGridSizer4->Add(Slider_Twinkle_Steps, 1, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    BitmapButton_TwinkleSteps = new wxBitmapButton(Panel1_Twinkle, ID_BITMAPBUTTON_SLIDER_Twinkle_Steps, padlock16x16_blue_xpm, wxDefaultPosition, wxSize(22,21), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON_SLIDER_Twinkle_Steps"));
    BitmapButton_TwinkleSteps->SetDefault();
    FlexGridSizer4->Add(BitmapButton_TwinkleSteps, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 1);
    CheckBox_Twinkle_Strobe = new wxCheckBox(Panel1_Twinkle, ID_CHECKBOX_Twinkle_Strobe, _("Strobe"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX_Twinkle_Strobe"));
    CheckBox_Twinkle_Strobe->SetValue(false);
    FlexGridSizer4->Add(CheckBox_Twinkle_Strobe, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizer4->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    BitmapButton_TwinkleStrobe = new wxBitmapButton(Panel1_Twinkle, ID_BITMAPBUTTON_CHECKBOX_Twinkle_Strobe, padlock16x16_blue_xpm, wxDefaultPosition, wxSize(22,21), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON_CHECKBOX_Twinkle_Strobe"));
    BitmapButton_TwinkleStrobe->SetDefault();
    FlexGridSizer4->Add(BitmapButton_TwinkleStrobe, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 1);
    Panel1_Twinkle->SetSizer(FlexGridSizer4);
    FlexGridSizer4->Fit(Panel1_Twinkle);
    FlexGridSizer4->SetSizeHints(Panel1_Twinkle);
    Panel1_SingleStrand = new wxPanel(Choicebook1, ID_PANEL2, wxPoint(39,6), wxDefaultSize, wxTAB_TRAVERSAL, _T("ID_PANEL2"));
    Notebook1 = new wxNotebook(Panel1_SingleStrand, ID_NOTEBOOK1, wxPoint(0,0), wxSize(288,280), 0, _T("ID_NOTEBOOK1"));
    Panel1 = new wxPanel(Notebook1, ID_PANEL3, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("ID_PANEL3"));
    FlexGridSizer24 = new wxFlexGridSizer(0, 3, 0, 0);
    StaticText61 = new wxStaticText(Panel1, ID_STATICTEXT63, _("Colors"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT63"));
    FlexGridSizer24->Add(StaticText61, 0, wxALIGN_LEFT|wxALIGN_TOP, 0);
    Choice_SingleStrand_Colors = new wxChoice(Panel1, ID_CHOICE_SingleStrand_Colors, wxDefaultPosition, wxSize(95,21), 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE_SingleStrand_Colors"));
    FlexGridSizer24->Add(Choice_SingleStrand_Colors, 1, wxALIGN_LEFT|wxALIGN_TOP, 5);
    BitmapButton_SingleStrand_Colors = new wxBitmapButton(Panel1, ID_BITMAPBUTTON_SingleStrand_Colors, padlock16x16_blue_xpm, wxDefaultPosition, wxSize(20,20), wxBU_AUTODRAW, wxDefaultValidator, _T("ID_BITMAPBUTTON_SingleStrand_Colors"));
    BitmapButton_SingleStrand_Colors->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_ACTIVECAPTION));
    FlexGridSizer24->Add(BitmapButton_SingleStrand_Colors, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    StaticText60 = new wxStaticText(Panel1, ID_STATICTEXT62, _("Number Chases"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT62"));
    FlexGridSizer24->Add(StaticText60, 1, wxALL|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 5);
    Slider_Number_Chases = new wxSlider(Panel1, ID_SLIDER_Number_Chases, 1, 1, 20, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_Number_Chases"));
    Slider_Number_Chases->SetSelection(1, 20);
    FlexGridSizer24->Add(Slider_Number_Chases, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    BitmapButton_Number_Chases = new wxBitmapButton(Panel1, ID_BITMAPBUTTON_Number_Chases, padlock16x16_blue_xpm, wxDefaultPosition, wxSize(20,20), wxBU_AUTODRAW, wxDefaultValidator, _T("ID_BITMAPBUTTON_Number_Chases"));
    BitmapButton_Number_Chases->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_ACTIVECAPTION));
    FlexGridSizer24->Add(BitmapButton_Number_Chases, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    StaticText9 = new wxStaticText(Panel1, ID_STATICTEXT8, _("Chase Size"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT8"));
    FlexGridSizer24->Add(StaticText9, 1, wxALL|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 5);
    Slider_Color_Mix1 = new wxSlider(Panel1, ID_SLIDER_Color_Mix1, 10, 1, 100, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_Color_Mix1"));
    FlexGridSizer24->Add(Slider_Color_Mix1, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    BitmapButton_Color_Mix1 = new wxBitmapButton(Panel1, ID_BITMAPBUTTON_SLIDER_Color_Mix1, padlock16x16_blue_xpm, wxDefaultPosition, wxSize(20,20), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON_SLIDER_Color_Mix1"));
    BitmapButton_Color_Mix1->SetDefault();
    BitmapButton_Color_Mix1->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_ACTIVECAPTION));
    FlexGridSizer24->Add(BitmapButton_Color_Mix1, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    StaticText10 = new wxStaticText(Panel1, ID_STATICTEXT9, _("Chase Spacing"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT9"));
    FlexGridSizer24->Add(StaticText10, 1, wxALL|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 5);
    Slider_Chase_Spacing1 = new wxSlider(Panel1, ID_SLIDER_Chase_Spacing1, 1, 1, 20, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_Chase_Spacing1"));
    FlexGridSizer24->Add(Slider_Chase_Spacing1, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    BitmapButton_Chase_Spacing1 = new wxBitmapButton(Panel1, ID_BITMAPBUTTON_SLIDER_Chase_Spacing1, padlock16x16_blue_xpm, wxDefaultPosition, wxSize(20,20), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON_SLIDER_Chase_Spacing1"));
    BitmapButton_Chase_Spacing1->SetDefault();
    BitmapButton_Chase_Spacing1->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_ACTIVECAPTION));
    FlexGridSizer24->Add(BitmapButton_Chase_Spacing1, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    StaticText11 = new wxStaticText(Panel1, ID_STATICTEXT10, _("Chase Types"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT10"));
    FlexGridSizer24->Add(StaticText11, 1, wxALL|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 5);
    Choice_Chase_Type1 = new wxChoice(Panel1, ID_CHOICE_Chase_Type1, wxDefaultPosition, wxSize(95,21), 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE_Chase_Type1"));
    FlexGridSizer24->Add(Choice_Chase_Type1, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    BitmapButton_Chase_Type1 = new wxBitmapButton(Panel1, ID_BITMAPBUTTON_CHOICE_Chase_Type1, padlock16x16_blue_xpm, wxDefaultPosition, wxSize(20,20), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON_CHOICE_Chase_Type1"));
    BitmapButton_Chase_Type1->SetDefault();
    BitmapButton_Chase_Type1->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_ACTIVECAPTION));
    FlexGridSizer24->Add(BitmapButton_Chase_Type1, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    StaticText12 = new wxStaticText(Panel1, ID_STATICTEXT11, _("3d Fade"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT11"));
    FlexGridSizer24->Add(StaticText12, 1, wxALL|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 5);
    CheckBox_Chase_3dFade1 = new wxCheckBox(Panel1, ID_CHECKBOX_Chase_3dFade1, _("Yes"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX_Chase_3dFade1"));
    CheckBox_Chase_3dFade1->SetValue(false);
    FlexGridSizer24->Add(CheckBox_Chase_3dFade1, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    BitmapButton_Chase_3dFade1 = new wxBitmapButton(Panel1, ID_BITMAPBUTTON_CHECKBOX_Chase_3dFade1, padlock16x16_blue_xpm, wxDefaultPosition, wxSize(20,20), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON_CHECKBOX_Chase_3dFade1"));
    BitmapButton_Chase_3dFade1->SetDefault();
    BitmapButton_Chase_3dFade1->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_ACTIVECAPTION));
    FlexGridSizer24->Add(BitmapButton_Chase_3dFade1, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    StaticText13 = new wxStaticText(Panel1, ID_STATICTEXT12, _("Group All Arches"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT12"));
    FlexGridSizer24->Add(StaticText13, 1, wxALL|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 5);
    CheckBox_Chase_Group_All = new wxCheckBox(Panel1, ID_CHECKBOX_Chase_Group_All, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX_Chase_Group_All"));
    CheckBox_Chase_Group_All->SetValue(false);
    FlexGridSizer24->Add(CheckBox_Chase_Group_All, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    BitmapButton_Chase_Group_All = new wxBitmapButton(Panel1, ID_BITMAPBUTTON_Chase_Group_All, padlock16x16_blue_xpm, wxDefaultPosition, wxSize(20,20), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON_Chase_Group_All"));
    BitmapButton_Chase_Group_All->SetDefault();
    BitmapButton_Chase_Group_All->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_ACTIVECAPTION));
    FlexGridSizer24->Add(BitmapButton_Chase_Group_All, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    Panel1->SetSizer(FlexGridSizer24);
    FlexGridSizer24->Fit(Panel1);
    FlexGridSizer24->SetSizeHints(Panel1);
    Notebook1->AddPage(Panel1, _("Chase1"), false);
    Panel1_Wave = new wxPanel(Choicebook1, ID_PANEL15, wxPoint(184,10), wxDefaultSize, wxTAB_TRAVERSAL, _T("ID_PANEL15"));
    FlexGridSizer51 = new wxFlexGridSizer(0, 3, 0, 0);
    StaticText47 = new wxStaticText(Panel1_Wave, ID_STATICTEXT49, _("Wave Type"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT49"));
    FlexGridSizer51->Add(StaticText47, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    Choice_Wave_Type = new wxChoice(Panel1_Wave, ID_CHOICE_Wave_Type, wxDefaultPosition, wxDefaultSize, 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE_Wave_Type"));
    FlexGridSizer51->Add(Choice_Wave_Type, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    BitmapButton_WaveType = new wxBitmapButton(Panel1_Wave, ID_BITMAPBUTTON2, padlock16x16_green_xpm, wxDefaultPosition, wxSize(20,20), wxBU_AUTODRAW, wxDefaultValidator, _T("ID_BITMAPBUTTON2"));
    BitmapButton_WaveType->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_ACTIVECAPTION));
    FlexGridSizer51->Add(BitmapButton_WaveType, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    StaticText48 = new wxStaticText(Panel1_Wave, ID_STATICTEXT50, _("Fill Colors"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT50"));
    FlexGridSizer51->Add(StaticText48, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    Choice_Fill_Colors = new wxChoice(Panel1_Wave, ID_CHOICE_Fill_Colors, wxDefaultPosition, wxDefaultSize, 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE_Fill_Colors"));
    FlexGridSizer51->Add(Choice_Fill_Colors, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    BitmapButton_Wave_FillColors = new wxBitmapButton(Panel1_Wave, ID_BITMAPBUTTON3, padlock16x16_blue_xpm, wxDefaultPosition, wxSize(20,20), wxBU_AUTODRAW, wxDefaultValidator, _T("ID_BITMAPBUTTON3"));
    BitmapButton_Wave_FillColors->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_ACTIVECAPTION));
    FlexGridSizer51->Add(BitmapButton_Wave_FillColors, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    StaticText49 = new wxStaticText(Panel1_Wave, ID_STATICTEXT52, _("Mirror Wave"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT52"));
    FlexGridSizer51->Add(StaticText49, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    CheckBox_Mirror_Wave = new wxCheckBox(Panel1_Wave, ID_CHECKBOX_Mirror_Wave, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX_Mirror_Wave"));
    CheckBox_Mirror_Wave->SetValue(false);
    FlexGridSizer51->Add(CheckBox_Mirror_Wave, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    BitmapButton_Wave_MirrorWave = new wxBitmapButton(Panel1_Wave, ID_BITMAPBUTTON4, padlock16x16_blue_xpm, wxDefaultPosition, wxSize(20,20), wxBU_AUTODRAW, wxDefaultValidator, _T("ID_BITMAPBUTTON4"));
    BitmapButton_Wave_MirrorWave->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_ACTIVECAPTION));
    FlexGridSizer51->Add(BitmapButton_Wave_MirrorWave, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    StaticText50 = new wxStaticText(Panel1_Wave, ID_STATICTEXT55, _("Number of Waves"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT55"));
    FlexGridSizer51->Add(StaticText50, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    Slider_Number_Waves = new wxSlider(Panel1_Wave, ID_SLIDER_Number_Waves, 900, 180, 3600, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_Number_Waves"));
    FlexGridSizer51->Add(Slider_Number_Waves, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    BitmapButton_NumberWaves = new wxBitmapButton(Panel1_Wave, ID_BITMAPBUTTON5, padlock16x16_blue_xpm, wxDefaultPosition, wxSize(20,20), wxBU_AUTODRAW, wxDefaultValidator, _T("ID_BITMAPBUTTON5"));
    BitmapButton_NumberWaves->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_ACTIVECAPTION));
    FlexGridSizer51->Add(BitmapButton_NumberWaves, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    StaticText52 = new wxStaticText(Panel1_Wave, ID_STATICTEXT56, _("Thickness of Wave"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT56"));
    FlexGridSizer51->Add(StaticText52, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    Slider_Thickness_Percentage = new wxSlider(Panel1_Wave, ID_SLIDER_Thickness_Percentage, 5, 0, 100, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_Thickness_Percentage"));
    FlexGridSizer51->Add(Slider_Thickness_Percentage, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    BitmapButton_Wave_Thickness = new wxBitmapButton(Panel1_Wave, ID_BITMAPBUTTON6, padlock16x16_blue_xpm, wxDefaultPosition, wxSize(20,20), wxBU_AUTODRAW, wxDefaultValidator, _T("ID_BITMAPBUTTON6"));
    BitmapButton_Wave_Thickness->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_ACTIVECAPTION));
    FlexGridSizer51->Add(BitmapButton_Wave_Thickness, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    StaticText55 = new wxStaticText(Panel1_Wave, ID_STATICTEXT58, _("Scale Height of Wave"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT58"));
    FlexGridSizer51->Add(StaticText55, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    Slider_Wave_Height = new wxSlider(Panel1_Wave, ID_SLIDER_Wave_Height, 50, 0, 100, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_Wave_Height"));
    FlexGridSizer51->Add(Slider_Wave_Height, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    BitmapButton_Wave_Height = new wxBitmapButton(Panel1_Wave, ID_BITMAPBUTTON7, padlock16x16_blue_xpm, wxDefaultPosition, wxSize(20,20), wxBU_AUTODRAW, wxDefaultValidator, _T("ID_BITMAPBUTTON7"));
    BitmapButton_Wave_Height->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_ACTIVECAPTION));
    FlexGridSizer51->Add(BitmapButton_Wave_Height, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    StaticText56 = new wxStaticText(Panel1_Wave, ID_STATICTEXT59, _("Wave Direction"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT59"));
    FlexGridSizer51->Add(StaticText56, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    Choice_Wave_Direction = new wxChoice(Panel1_Wave, ID_CHOICE_Wave_Direction, wxDefaultPosition, wxDefaultSize, 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE_Wave_Direction"));
    FlexGridSizer51->Add(Choice_Wave_Direction, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    BitmapButton_Wave_Direction = new wxBitmapButton(Panel1_Wave, ID_BITMAPBUTTON8, padlock16x16_blue_xpm, wxDefaultPosition, wxSize(20,20), wxBU_AUTODRAW, wxDefaultValidator, _T("ID_BITMAPBUTTON8"));
    BitmapButton_Wave_Direction->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_ACTIVECAPTION));
    FlexGridSizer51->Add(BitmapButton_Wave_Direction, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    Panel1_Wave->SetSizer(FlexGridSizer51);
    FlexGridSizer51->Fit(Panel1_Wave);
    FlexGridSizer51->SetSizeHints(Panel1_Wave);
    Panel1_CoroFaces = new wxPanel(Choicebook1, ID_PANEL28, wxPoint(202,8), wxDefaultSize, wxTAB_TRAVERSAL, _T("ID_PANEL28"));
    FlexGridSizer58 = new wxFlexGridSizer(0, 2, 0, 0);
    StaticText71 = new wxStaticText(Panel1_CoroFaces, ID_STATICTEXT73, _("Phoneme"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT73"));
    FlexGridSizer58->Add(StaticText71, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    Choice_CoroFaces_Phoneme = new wxChoice(Panel1_CoroFaces, ID_CHOICE_CoroFaces_Phoneme, wxDefaultPosition, wxDefaultSize, 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE_CoroFaces_Phoneme"));
    FlexGridSizer58->Add(Choice_CoroFaces_Phoneme, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    StaticText72 = new wxStaticText(Panel1_CoroFaces, ID_STATICTEXT74, _("Mouth X:Y"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT74"));
    FlexGridSizer58->Add(StaticText72, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    TextCtrl_X_Y = new wxTextCtrl(Panel1_CoroFaces, ID_TEXTCTRL_X_Y, _("0:0"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_TEXTCTRL_X_Y"));
    FlexGridSizer58->Add(TextCtrl_X_Y, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    StaticText73 = new wxStaticText(Panel1_CoroFaces, ID_STATICTEXT75, _("Face Outline X:Y"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT75"));
    FlexGridSizer58->Add(StaticText73, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    TextCtrl_Outline_X_Y = new wxTextCtrl(Panel1_CoroFaces, ID_TEXTCTRL_Outline_X_Y, _("0:0"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_TEXTCTRL_Outline_X_Y"));
    FlexGridSizer58->Add(TextCtrl_Outline_X_Y, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    StaticText74 = new wxStaticText(Panel1_CoroFaces, ID_STATICTEXT76, _("Eyes X:Y"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT76"));
    FlexGridSizer58->Add(StaticText74, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    TextCtrl_Eyes_X_Y = new wxTextCtrl(Panel1_CoroFaces, ID_TEXTCTRL_Eyes_X_Y, _("0:0"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_TEXTCTRL_Eyes_X_Y"));
    FlexGridSizer58->Add(TextCtrl_Eyes_X_Y, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    Panel1_CoroFaces->SetSizer(FlexGridSizer58);
    FlexGridSizer58->Fit(Panel1_CoroFaces);
    FlexGridSizer58->SetSizeHints(Panel1_CoroFaces);
    Choicebook1->AddPage(Panel1_None, _("None"), false);
    Choicebook1->AddPage(Panel1_Bars, _("Bars"), false);
    Choicebook1->AddPage(Panel1_Butterfly, _("Butterfly"), false);
    Choicebook1->AddPage(Panel1_Circles, _("Circles"), false);
    Choicebook1->AddPage(Panel1_ColorWash, _("Color Wash"), false);
    Choicebook1->AddPage(Panel1_Curtain, _("Curtain"), false);
    Choicebook1->AddPage(Panel1_Faces, _("Faces"), false);
    Choicebook1->AddPage(Panel1_Fire, _("Fire"), false);
    Choicebook1->AddPage(Panel1_Fireworks, _("Fireworks"), false);
    Choicebook1->AddPage(Panel1_Garlands, _("Garlands"), false);
    Choicebook1->AddPage(Panel1_Glediator, _("Glediator"), false);
    Choicebook1->AddPage(Panel1_Life, _("Life"), false);
    Choicebook1->AddPage(Panel1_Meteors, _("Meteors"), false);
    Choicebook1->AddPage(Panel1_Piano, _("Piano"), false);
    Choicebook1->AddPage(Panel1_Pictures, _("Pictures"), false);
    Choicebook1->AddPage(Panel1_Snowflakes, _("Snowflakes"), false);
    Choicebook1->AddPage(Panel1_Snowstorm, _("Snowstorm"), false);
    Choicebook1->AddPage(Panel1_Spirals, _("Spirals"), false);
    Choicebook1->AddPage(Panel1_Spirograph, _("Spirograph"), false);
    Choicebook1->AddPage(Panel1_Text, _("Text"), false);
    Choicebook1->AddPage(Panel1_Tree, _("Tree"), false);
    Choicebook1->AddPage(Panel1_Twinkle, _("Twinkle"), false);
    Choicebook1->AddPage(Panel1_SingleStrand, _("SingleStrand"), false);
    Choicebook1->AddPage(Panel1_Wave, _("Wave"), false);
    Choicebook1->AddPage(Panel1_CoroFaces, _("CoroFaces"), false);
    FlexGridSizer1->Add(Choicebook1, 1, wxRIGHT|wxALIGN_TOP|wxALIGN_CENTER_HORIZONTAL, 5);
    FlexGridSizer8 = new wxFlexGridSizer(0, 1, 0, 0);
    FlexGridSizer6 = new wxFlexGridSizer(0, 4, 0, 0);
    BitmapButton_LayerEffect = new wxBitmapButton(this, ID_BITMAPBUTTON_CHOICEBOOK1, padlock16x16_blue_xpm, wxDefaultPosition, wxSize(22,21), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON_CHOICEBOOK1"));
    BitmapButton_LayerEffect->SetDefault();
    FlexGridSizer6->Add(BitmapButton_LayerEffect, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 1);
    BitmapButton_normal = new wxBitmapButton(this, ID_BITMAPBUTTON87, padlock16x16_green_xpm, wxDefaultPosition, wxSize(22,21), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON87"));
    BitmapButton_normal->SetDefault();
    BitmapButton_normal->Hide();
    FlexGridSizer6->Add(BitmapButton_normal, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 1);
    BitmapButton_locked = new wxBitmapButton(this, ID_BITMAPBUTTON1, padlock16x16_red_xpm, wxDefaultPosition, wxSize(22,21), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON1"));
    BitmapButton_locked->SetDefault();
    BitmapButton_locked->Hide();
    FlexGridSizer6->Add(BitmapButton_locked, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    BitmapButton_random = new wxBitmapButton(this, ID_BITMAPBUTTON88, padlock16x16_blue_xpm, wxDefaultPosition, wxSize(22,21), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON88"));
    BitmapButton_random->SetDefault();
    BitmapButton_random->Hide();
    FlexGridSizer6->Add(BitmapButton_random, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 1);
    FlexGridSizer8->Add(FlexGridSizer6, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 1);
    FlexGridSizer9 = new wxFlexGridSizer(0, 2, 0, 0);
    FlexGridSizer_Palette = new wxFlexGridSizer(0, 3, 0, 0);
    CheckBox_Palette1 = new wxCheckBox(this, ID_CHECKBOX_Palette1, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX_Palette1"));
    CheckBox_Palette1->SetValue(true);
    FlexGridSizer_Palette->Add(CheckBox_Palette1, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
    Button_Palette1 = new wxButton(this, ID_BUTTON_Palette1, _("1"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON_Palette1"));
    Button_Palette1->SetMinSize(wxSize(30,21));
    FlexGridSizer_Palette->Add(Button_Palette1, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
    BitmapButton_Palette1 = new wxBitmapButton(this, ID_BITMAPBUTTON_BUTTON_Palette1, padlock16x16_blue_xpm, wxDefaultPosition, wxSize(20,20), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON_BUTTON_Palette1"));
    BitmapButton_Palette1->SetDefault();
    BitmapButton_Palette1->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_ACTIVECAPTION));
    FlexGridSizer_Palette->Add(BitmapButton_Palette1, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 1);
    CheckBox_Palette2 = new wxCheckBox(this, ID_CHECKBOX_Palette2, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX_Palette2"));
    CheckBox_Palette2->SetValue(true);
    FlexGridSizer_Palette->Add(CheckBox_Palette2, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
    Button_Palette2 = new wxButton(this, ID_BUTTON_Palette2, _("2"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON_Palette2"));
    Button_Palette2->SetMinSize(wxSize(30,21));
    FlexGridSizer_Palette->Add(Button_Palette2, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
    BitmapButton_Palette2 = new wxBitmapButton(this, ID_BITMAPBUTTON_BUTTON_Palette2, padlock16x16_blue_xpm, wxDefaultPosition, wxSize(20,20), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON_BUTTON_Palette2"));
    BitmapButton_Palette2->SetDefault();
    BitmapButton_Palette2->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_ACTIVECAPTION));
    FlexGridSizer_Palette->Add(BitmapButton_Palette2, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 1);
    CheckBox_Palette3 = new wxCheckBox(this, ID_CHECKBOX_Palette3, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX_Palette3"));
    CheckBox_Palette3->SetValue(false);
    FlexGridSizer_Palette->Add(CheckBox_Palette3, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
    Button_Palette3 = new wxButton(this, ID_BUTTON_Palette3, _("3"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON_Palette3"));
    Button_Palette3->SetMinSize(wxSize(30,21));
    FlexGridSizer_Palette->Add(Button_Palette3, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
    BitmapButton_Palette3 = new wxBitmapButton(this, ID_BITMAPBUTTON_BUTTON_Palette3, padlock16x16_blue_xpm, wxDefaultPosition, wxSize(20,20), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON_BUTTON_Palette3"));
    BitmapButton_Palette3->SetDefault();
    BitmapButton_Palette3->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_ACTIVECAPTION));
    FlexGridSizer_Palette->Add(BitmapButton_Palette3, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 1);
    CheckBox_Palette4 = new wxCheckBox(this, ID_CHECKBOX_Palette4, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX_Palette4"));
    CheckBox_Palette4->SetValue(false);
    FlexGridSizer_Palette->Add(CheckBox_Palette4, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
    Button_Palette4 = new wxButton(this, ID_BUTTON_Palette4, _("4"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON_Palette4"));
    Button_Palette4->SetMinSize(wxSize(30,21));
    FlexGridSizer_Palette->Add(Button_Palette4, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
    BitmapButton_Palette4 = new wxBitmapButton(this, ID_BITMAPBUTTON_BUTTON_Palette4, padlock16x16_blue_xpm, wxDefaultPosition, wxSize(20,20), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON_BUTTON_Palette4"));
    BitmapButton_Palette4->SetDefault();
    BitmapButton_Palette4->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_ACTIVECAPTION));
    FlexGridSizer_Palette->Add(BitmapButton_Palette4, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 1);
    CheckBox_Palette5 = new wxCheckBox(this, ID_CHECKBOX_Palette5, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX_Palette5"));
    CheckBox_Palette5->SetValue(false);
    FlexGridSizer_Palette->Add(CheckBox_Palette5, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
    Button_Palette5 = new wxButton(this, ID_BUTTON_Palette5, _("5"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON_Palette5"));
    Button_Palette5->SetMinSize(wxSize(30,21));
    FlexGridSizer_Palette->Add(Button_Palette5, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
    BitmapButton_Palette5 = new wxBitmapButton(this, ID_BITMAPBUTTON_BUTTON_Palette5, padlock16x16_blue_xpm, wxDefaultPosition, wxSize(20,20), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON_BUTTON_Palette5"));
    BitmapButton_Palette5->SetDefault();
    BitmapButton_Palette5->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_ACTIVECAPTION));
    FlexGridSizer_Palette->Add(BitmapButton_Palette5, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 1);
    CheckBox_Palette6 = new wxCheckBox(this, ID_CHECKBOX_Palette6, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX_Palette6"));
    CheckBox_Palette6->SetValue(false);
    FlexGridSizer_Palette->Add(CheckBox_Palette6, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
    Button_Palette6 = new wxButton(this, ID_BUTTON_Palette6, _("6"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON_Palette6"));
    Button_Palette6->SetMinSize(wxSize(30,21));
    FlexGridSizer_Palette->Add(Button_Palette6, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
    BitmapButton_Palette6 = new wxBitmapButton(this, ID_BITMAPBUTTON_BUTTON_Palette6, padlock16x16_blue_xpm, wxDefaultPosition, wxSize(20,20), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON_BUTTON_Palette6"));
    BitmapButton_Palette6->SetDefault();
    BitmapButton_Palette6->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_ACTIVECAPTION));
    FlexGridSizer_Palette->Add(BitmapButton_Palette6, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 1);
    FlexGridSizer9->Add(FlexGridSizer_Palette, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 1);
    FlexGridSizer3 = new wxFlexGridSizer(0, 1, 0, 0);
    StaticText59 = new wxStaticText(this, ID_STATICTEXT61, _("Speed"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT61"));
    FlexGridSizer3->Add(StaticText59, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
    Slider_Speed = new wxSlider(this, ID_SLIDER_Speed, 10, 1, 30, wxDefaultPosition, wxSize(30,100), wxSL_VERTICAL|wxSL_INVERSE, wxDefaultValidator, _T("ID_SLIDER_Speed"));
    FlexGridSizer3->Add(Slider_Speed, 1, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    BitmapButton_Speed = new wxBitmapButton(this, ID_BITMAPBUTTON_SLIDER_Speed, padlock16x16_blue_xpm, wxDefaultPosition, wxSize(22,21), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON_SLIDER_Speed"));
    BitmapButton_Speed->SetDefault();
    FlexGridSizer3->Add(BitmapButton_Speed, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 1);
    TextCtrl_Speed = new wxTextCtrl(this, ID_TEXTCTRL_Speed, _("10"), wxDefaultPosition, wxSize(30,20), wxTE_READONLY, wxDefaultValidator, _T("ID_TEXTCTRL_Speed"));
    FlexGridSizer3->Add(TextCtrl_Speed, 1, wxBOTTOM|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
    FlexGridSizer9->Add(FlexGridSizer3, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 1);
    FlexGridSizer8->Add(FlexGridSizer9, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 1);
    FlexGridSizer2 = new wxFlexGridSizer(0, 1, 0, 0);
    FlexGridSizer22 = new wxFlexGridSizer(0, 3, 0, 0);
    StaticText2 = new wxStaticText(this, ID_STATICTEXT2, _("Fade In"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT2"));
    FlexGridSizer22->Add(StaticText2, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
    TextCtrl_Fadein = new wxTextCtrl(this, ID_TEXTCTRL_Fadein, _("0.00"), wxDefaultPosition, wxSize(37,20), 0, wxDefaultValidator, _T("ID_TEXTCTRL_Fadein"));
    TextCtrl_Fadein->SetMaxLength(4);
    FlexGridSizer22->Add(TextCtrl_Fadein, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
    BitmapButton_FadeOut = new wxBitmapButton(this, ID_BITMAPBUTTON_TEXTCTRL_Fadein, padlock16x16_blue_xpm, wxDefaultPosition, wxSize(20,20), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON_TEXTCTRL_Fadein"));
    BitmapButton_FadeOut->SetDefault();
    BitmapButton_FadeOut->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_ACTIVECAPTION));
    FlexGridSizer22->Add(BitmapButton_FadeOut, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 1);
    StaticText3 = new wxStaticText(this, ID_STATICTEXT3, _("Fade Out"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT3"));
    FlexGridSizer22->Add(StaticText3, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
    TextCtrl_Fadeout = new wxTextCtrl(this, ID_TEXTCTRL_Fadeout, _("0.00"), wxDefaultPosition, wxSize(37,20), 0, wxDefaultValidator, _T("ID_TEXTCTRL_Fadeout"));
    TextCtrl_Fadeout->SetMaxLength(4);
    FlexGridSizer22->Add(TextCtrl_Fadeout, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
    BitmapButton_FadeIn = new wxBitmapButton(this, ID_BITMAPBUTTON_TEXTCTRL_Fadeout, padlock16x16_blue_xpm, wxDefaultPosition, wxSize(20,20), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON_TEXTCTRL_Fadeout"));
    BitmapButton_FadeIn->SetDefault();
    BitmapButton_FadeIn->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_ACTIVECAPTION));
    FlexGridSizer22->Add(BitmapButton_FadeIn, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 1);
    FlexGridSizer2->Add(FlexGridSizer22, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 1);
    FlexGridSizer23 = new wxFlexGridSizer(0, 3, 0, 0);
    CheckBox_FitToTime = new wxCheckBox(this, ID_CHECKBOX_FitToTime, _("Fit to time"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX_FitToTime"));
    CheckBox_FitToTime->SetValue(false);
    FlexGridSizer23->Add(CheckBox_FitToTime, 1, wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
    FlexGridSizer23->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 1);
    BitmapButton_FitToTime = new wxBitmapButton(this, ID_BITMAPBUTTON_CHECKBOX_FitToTime, padlock16x16_blue_xpm, wxDefaultPosition, wxSize(20,20), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON_CHECKBOX_FitToTime"));
    BitmapButton_FitToTime->SetDefault();
    BitmapButton_FitToTime->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_ACTIVECAPTION));
    FlexGridSizer23->Add(BitmapButton_FitToTime, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 1);
    CheckBox_OverlayBkg = new wxCheckBox(this, ID_CHECKBOX_OverlayBkg, _("Persistent"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX_OverlayBkg"));
    CheckBox_OverlayBkg->SetValue(false);
    FlexGridSizer23->Add(CheckBox_OverlayBkg, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 2);
    FlexGridSizer23->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    BitmapButton_OverlayBkg = new wxBitmapButton(this, ID_BITMAPBUTTON_OverlayBkg, padlock16x16_blue_xpm, wxDefaultPosition, wxSize(20,20), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON_OverlayBkg"));
    BitmapButton_OverlayBkg->SetDefault();
    BitmapButton_OverlayBkg->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_ACTIVECAPTION));
    FlexGridSizer23->Add(BitmapButton_OverlayBkg, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizer2->Add(FlexGridSizer23, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 1);
    FlexGridSizer8->Add(FlexGridSizer2, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 1);
    FlexGridSizer1->Add(FlexGridSizer8, 1, wxALIGN_TOP|wxALIGN_CENTER_HORIZONTAL, 5);
    SetSizer(FlexGridSizer1);
    FlexGridSizer1->Fit(this);
    FlexGridSizer1->SetSizeHints(this);

    Connect(ID_BITMAPBUTTON_SLIDER_Bars_BarCount,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&EffectsPanel::OnBitmapButton_PaletteRepClick);
    Connect(ID_CHOICE_Bars_Direction,wxEVT_COMMAND_CHOICE_SELECTED,(wxObjectEventFunction)&EffectsPanel::OnChoice_Bars_DirectionSelect);
    Connect(ID_BITMAPBUTTON_CHOICE_Bars_Direction,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&EffectsPanel::OnBitmapButton_DirectionClick);
    Connect(ID_BITMAPBUTTON_CHECKBOX_Bars_Highlight,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&EffectsPanel::OnBitmapButton_HighlightClick);
    Connect(ID_BITMAPBUTTON_CHECKBOX_Bars_3D,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&EffectsPanel::OnBitmapButton_3DClick);
    Connect(ID_BITMAPBUTTON_CHOICE_Butterfly_Color,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&EffectsPanel::OnBitmapButton_ButterflyColorsClick);
    Connect(ID_BITMAPBUTTON_SLIDER_Butterfly_Style,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&EffectsPanel::OnBitmapButton_ButterflyStyleClick);
    Connect(ID_BITMAPBUTTON_SLIDER_Butterfly_Chunks,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&EffectsPanel::OnBitmapButton_ButterflyChunksClick);
    Connect(ID_BITMAPBUTTON_SLIDER_Butterfly_Skip,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&EffectsPanel::OnBitmapButton_ButterflySkipClick);
    Connect(ID_BITMAPBUTTON_SLIDER_Circles_Count,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&EffectsPanel::OnBitmapButton_CirclesCountClick);
    Connect(ID_BITMAPBUTTON_SLIDER_Circles_Size,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&EffectsPanel::OnBitmapButton_CirclesSizeClick);
    Connect(ID_BITMAPBUTTON_CHECKBOX_Circles_Bounce,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&EffectsPanel::OnBitmapButton_CirclesBounceClick);
    Connect(ID_BITMAPBUTTON_CHECKBOX_Circles_Collide,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&EffectsPanel::OnBitmapButton_CirclesCollideClick);
    Connect(ID_BITMAPBUTTON_CHECKBOX_Circles_Radial,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&EffectsPanel::OnBitmapButton_CirclesRadialClick);
    Connect(ID_BITMAPBUTTON_CHECKBOX_Circles_Plasma,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&EffectsPanel::OnBitmapButton_CirclesPlasmaClick);
    Connect(ID_BITMAPBUTTON_CHECKBOX_Circles_Radial_3D,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&EffectsPanel::OnBitmapButton_CirclesRadial3DClick);
    Connect(ID_BITMAPBUTTON_CHECKBOX_Circles_Random_m,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&EffectsPanel::OnBitmapButton_RandomMotionClick);
    Connect(ID_BITMAPBUTTON_SLIDER_ColorWash_Count,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&EffectsPanel::OnBitmapButton_ColorWashCountClick);
    Connect(ID_BITMAPBUTTON_CHECKBOX_ColorWash_HFade,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&EffectsPanel::OnBitmapButton_ColorWashHFadeClick);
    Connect(ID_BITMAPBUTTON_CHECKBOX_ColorWash_VFade,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&EffectsPanel::OnBitmapButton_ColorWashVFadeClick);
    Connect(ID_BITMAPBUTTON_CHOICE_Curtain_Edge,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&EffectsPanel::OnBitmapButton_CurtainEdgeClick);
    Connect(ID_BITMAPBUTTON_CHOICE_Curtain_Effect,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&EffectsPanel::OnBitmapButton_CurtainEffectClick);
    Connect(ID_BITMAPBUTTON_SLIDER_Curtain_Swag,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&EffectsPanel::OnBitmapButton_CurtainSwagClick);
    Connect(ID_BITMAPBUTTON_CHECKBOX_Curtain_Repeat,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&EffectsPanel::OnBitmapButton_CurtainRepeatClick);
    Connect(ID_BITMAPBUTTON_SLIDER_Fire_Height,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&EffectsPanel::OnBitmapButton_FireHeightClick);
    Connect(ID_BITMAPBUTTON_SLIDER_Fire_HueShift,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&EffectsPanel::OnBitmapButton_FireWidthClick);
    Connect(ID_BITMAPBUTTON_CHECKBOX_Fire_GrowFire,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&EffectsPanel::OnBitmapButton_FireGrowClick);
    Connect(ID_BITMAPBUTTON_SLIDER_Fireworks_Number_Explosions,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&EffectsPanel::OnBitmapButton_FireworksNumberExplosionsClick);
    Connect(ID_BITMAPBUTTON_SLIDER_Fireworks_Count,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&EffectsPanel::OnBitmapButton_FireworksCountClick);
    Connect(ID_BITMAPBUTTON_SLIDER_Fireworks_Velocity,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&EffectsPanel::OnBitmapButton_FireworksVelocityClick);
    Connect(ID_BITMAPBUTTON_SLIDER_Fireworks_Fade,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&EffectsPanel::OnBitmapButton_FireworksFadeClick);
    Connect(ID_BITMAPBUTTON_SLIDER_Garlands_Type,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&EffectsPanel::OnBitmapButton_GarlandsTypeClick);
    Connect(ID_BITMAPBUTTON_SLIDER_Garlands_Spacing,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&EffectsPanel::OnBitmapButton_GarlandsSpacingClick);
    Connect(ID_BUTTON3,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&EffectsPanel::OnButton_Glediator_FilenameClick);
    Connect(ID_BITMAPBUTTON10,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&EffectsPanel::OnBitmapButton_Glediator_FilenameClick);
    Connect(ID_BITMAPBUTTON_SLIDER_Life_Count,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&EffectsPanel::OnBitmapButton_LifeCountClick);
    Connect(ID_BITMAPBUTTON_SLIDER_Life_Seed,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&EffectsPanel::OnBitmapButton_LifeSpeedClick);
    Connect(ID_BITMAPBUTTON_CHOICE_Meteors_Type,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&EffectsPanel::OnBitmapButton_MeteorsTypeClick);
    Connect(ID_BITMAPBUTTON_CHOICE_Meteors_Effect,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&EffectsPanel::OnBitmapButton_MeteorsEffectClick);
    Connect(ID_BITMAPBUTTON_SLIDER_Meteors_Count,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&EffectsPanel::OnBitmapButton_MeteorsCountClick);
    Connect(ID_BITMAPBUTTON_SLIDER_Meteors_Length,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&EffectsPanel::OnBitmapButton_MeteorsLengthClick);
    Connect(ID_BITMAPBUTTON_SLIDER_Meteors_Swirl_Intensity,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&EffectsPanel::OnBitmapButton_MeteorsSwirlIntensityClick);
    Connect(ID_CHOICE_Piano_Style,wxEVT_COMMAND_CHOICE_SELECTED,(wxObjectEventFunction)&EffectsPanel::OnPiano_StyleSelect);
    Connect(ID_BITMAPBUTTON_SLIDER_Piano_Style,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&EffectsPanel::OnBitmapButton_Piano_StyleClick);
    Connect(ID_SLIDER_Piano_NumKeys,wxEVT_SCROLL_TOP|wxEVT_SCROLL_BOTTOM|wxEVT_SCROLL_LINEUP|wxEVT_SCROLL_LINEDOWN|wxEVT_SCROLL_PAGEUP|wxEVT_SCROLL_PAGEDOWN|wxEVT_SCROLL_THUMBTRACK|wxEVT_SCROLL_THUMBRELEASE|wxEVT_SCROLL_CHANGED,(wxObjectEventFunction)&EffectsPanel::OnSlider_Piano_NumKeysCmdScroll);
    Connect(ID_BITMAPBUTTON_Piano_NumKeys,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&EffectsPanel::OnBitmapButton_Piano_NumKeysClick);
    Connect(ID_SLIDER_Piano_KeyWidth,wxEVT_SCROLL_TOP|wxEVT_SCROLL_BOTTOM|wxEVT_SCROLL_LINEUP|wxEVT_SCROLL_LINEDOWN|wxEVT_SCROLL_PAGEUP|wxEVT_SCROLL_PAGEDOWN|wxEVT_SCROLL_THUMBTRACK|wxEVT_SCROLL_THUMBRELEASE|wxEVT_SCROLL_CHANGED,(wxObjectEventFunction)&EffectsPanel::OnSlider_Piano_NumRowsCmdScroll);
    Connect(ID_BITMAPBUTTON_Piano_NumRows,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&EffectsPanel::OnBitmapButton_Piano_NumRowsClick);
    Connect(ID_BITMAPBUTTON_Piano_KeyPlacement,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&EffectsPanel::OnBitmapButton_Piano_KeyPlacementClick);
    Connect(ID_BUTTON_Piano_CueFilename,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&EffectsPanel::OnButton_Piano_CueFilenameClick);
    Connect(ID_BITMAPBUTTON_Piano_Filename,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&EffectsPanel::OnBitmapButton_Piano_CueFilenameClick);
    Connect(ID_BUTTON1,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&EffectsPanel::OnButton_Piano_MapFilenameClick);
    Connect(ID_BITMAPBUTTON_Piano_MapFilename,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&EffectsPanel::OnBitmapButton_Piano_MapFilenameClick);
    Connect(ID_BUTTON2,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&EffectsPanel::OnButton_Piano_ShapeFilenameClick);
    Connect(ID_BITMAPBUTTON_Piano_ShapeFilename,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&EffectsPanel::OnBitmapButton_Piano_ShapeFilenameClick);
    Connect(ID_BITMAPBUTTON_Piano_Clipping,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&EffectsPanel::OnBitmapButton_Piano_ClippingClick);
    Connect(ID_BUTTON_PICTURES_FILENAME,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&EffectsPanel::OnButton_Pictures_FilenameClick);
    Connect(ID_BITMAPBUTTON_BUTTON_PICTURES_FILENAME,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&EffectsPanel::OnBitmapButton_PicturesFilenameClick);
    Connect(ID_BITMAPBUTTON_CHOICE_Pictures_Direction,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&EffectsPanel::OnBitmapButton_PicturesDirectionClick);
    Connect(ID_BITMAPBUTTON_SLIDER_Pictures_GifSpeed,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&EffectsPanel::OnBitmapButton_PicturesSpeedClick);
    Connect(ID_BITMAPBUTTON_SLIDER_Snowflakes_Count,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&EffectsPanel::OnBitmapButton_SnowflakesCountClick);
    Connect(ID_BITMAPBUTTON_SLIDER_Snowflakes_Type,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&EffectsPanel::OnBitmapButton_SnowflakesTypeClick);
    Connect(ID_BITMAPBUTTON_SLIDER_Snowstorm_Count,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&EffectsPanel::OnBitmapButton_SnowstormCountClick);
    Connect(ID_BITMAPBUTTON_SLIDER_Snowstorm_Length,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&EffectsPanel::OnBitmapButton_SnowstormLengthClick);
    Connect(ID_BITMAPBUTTON_SLIDER_Spirals_Count,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&EffectsPanel::OnBitmapButton_SpiralsCountClick);
    Connect(ID_BITMAPBUTTON_SLIDER_Spirals_Rotation,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&EffectsPanel::OnBitmapButton_SpiralsRotationClick);
    Connect(ID_BITMAPBUTTON_SLIDER_Spirals_Thickness,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&EffectsPanel::OnBitmapButton_SpiralsThicknessClick);
    Connect(ID_BITMAPBUTTON_SLIDER_Spirals_Direction,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&EffectsPanel::OnBitmapButton_SpiralsDirectionClick);
    Connect(ID_BITMAPBUTTON_CHECKBOX_Spirals_Blend,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&EffectsPanel::OnBitmapButton_SpiralsBlendClick);
    Connect(ID_BITMAPBUTTON_CHECKBOX_Spirals_3D,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&EffectsPanel::OnBitmapButton_Spirals3DClick);
    Connect(ID_BITMAPBUTTON_CHECKBOX_Spirals_Grow,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&EffectsPanel::OnBitmapButton_SpiralsGrowClick);
    Connect(ID_BITMAPBUTTON_CHECKBOX_Spirals_Shrink,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&EffectsPanel::OnBitmapButton_SpiralsShrinkClick);
    Connect(ID_BITMAPBUTTON_SLIDER_Spirograph_R,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&EffectsPanel::OnBitmapButton_SpirographRClick);
    Connect(ID_BITMAPBUTTON_SLIDER_Spirograph_r,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&EffectsPanel::OnBitmapButton_SpirographrClick);
    Connect(ID_BITMAPBUTTON_SLIDER_Spirograph_d,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&EffectsPanel::OnBitmapButton_SpirographAnimateClick);
    Connect(ID_BITMAPBUTTON_CHECKBOX_Spirograph_Animate,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&EffectsPanel::OnBitmapButton_SpirographdClick);
    Connect(ID_BITMAPBUTTON_TEXTCTRL_Text_Line1,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&EffectsPanel::OnBitmapButton_TextLine1Click);
    Connect(ID_BITMAPBUTTON_TEXTCTRL_Text_Line2,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&EffectsPanel::OnBitmapButton_TextLine2Click);
    Connect(ID_BITMAPBUTTON_TEXTCTRL_Text_Line3,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&EffectsPanel::OnBitmapButton_TextLine3Click);
    Connect(ID_BITMAPBUTTON_TEXTCTRL_Text_Line4,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&EffectsPanel::OnBitmapButton_TextLine4Click);
    Connect(ID_BUTTON_TEXT_FONT1,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&EffectsPanel::OnButton_Text_Font1Click);
    Connect(ID_BITMAPBUTTON_TEXTCTRL_Text_Font1,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&EffectsPanel::OnBitmapButton_TextFont1Click);
    Connect(ID_BITMAPBUTTON_CHOICE_Text_Dir1,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&EffectsPanel::OnBitmapButton_TextDir1Click);
    Connect(ID_BITMAPBUTTON_TextToCenter1,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&EffectsPanel::OnBitmapButton_TextToCenter1Click);
    Connect(ID_BITMAPBUTTON_CHOICE_Text_Effect1,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&EffectsPanel::OnBitmapButton_TextEffect1Click);
    Connect(ID_BITMAPBUTTON_CHOICE_Text_Count1,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&EffectsPanel::OnBitmapButton_TextCount1Click);
    Connect(ID_BITMAPBUTTON_SLIDER_Text_Position1,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&EffectsPanel::OnBitmapButton_TextPosition1Click);
    Connect(ID_BUTTON_TEXT_FONT2,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&EffectsPanel::OnButton_Text_Font2Click);
    Connect(ID_BITMAPBUTTON_TEXTCTRL_Text_Font2,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&EffectsPanel::OnBitmapButton_TextFont2Click);
    Connect(ID_BITMAPBUTTON_CHOICE_Text_Dir2,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&EffectsPanel::OnBitmapButton_TextDir2Click);
    Connect(ID_BITMAPBUTTON_Text2Center2,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&EffectsPanel::OnBitmapButton_TextToCenter2Click);
    Connect(ID_BITMAPBUTTON_CHOICE_Text_Effect2,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&EffectsPanel::OnBitmapButton_TextEffect2Click);
    Connect(ID_BITMAPBUTTON_CHOICE_Text_Count2,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&EffectsPanel::OnBitmapButton_TextCount2Click);
    Connect(ID_BITMAPBUTTON_SLIDER_Text_Position2,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&EffectsPanel::OnBitmapButton_TextPosition2Click);
    Connect(ID_BUTTON_TEXT_FONT3,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&EffectsPanel::OnButton_Text_Font3Click);
    Connect(ID_BITMAPBUTTON_TEXTCTRL_Text_Font3,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&EffectsPanel::OnBitmapButton_TextFont3Click);
    Connect(ID_BITMAPBUTTON_CHOICE_Text_Dir3,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&EffectsPanel::OnBitmapButton_TextDir3Click);
    Connect(ID_BITMAPBUTTON_TextToCenter3,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&EffectsPanel::OnBitmapButton_TextToCenter3Click);
    Connect(ID_BITMAPBUTTON_CHOICE_Text_Effect3,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&EffectsPanel::OnBitmapButton_TextEffect3Click);
    Connect(ID_BITMAPBUTTON_CHOICE_Text_Count3,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&EffectsPanel::OnBitmapButton_TextCount3Click);
    Connect(ID_BITMAPBUTTON_SLIDER_Text_Position3,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&EffectsPanel::OnBitmapButton_TextPosition3Click);
    Connect(ID_BUTTON_TEXT_FONT4,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&EffectsPanel::OnButton_Text_Font4Click);
    Connect(ID_BITMAPBUTTON_TEXTCTRL_Text_Font4,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&EffectsPanel::OnBitmapButton_TextFont4Click);
    Connect(ID_BITMAPBUTTON_CHOICE_Text_Dir4,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&EffectsPanel::OnBitmapButton_TextDir4Click);
    Connect(ID_BITMAPBUTTON_TextToCenter4,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&EffectsPanel::OnBitmapButton_TextToCenter4Click);
    Connect(ID_BITMAPBUTTON_CHOICE_Text_Effect4,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&EffectsPanel::OnBitmapButton_TextEffect4Click);
    Connect(ID_BITMAPBUTTON_CHOICE_Text_Count4,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&EffectsPanel::OnBitmapButton_TextCount4Click);
    Connect(ID_BITMAPBUTTON_SLIDER_Text_Position4,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&EffectsPanel::OnBitmapButton_TextPosition4Click);
    Connect(ID_BITMAPBUTTON_SLIDER_Tree_Branches,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&EffectsPanel::OnBitmapButton_TreeBranchesClick);
    Connect(ID_BITMAPBUTTON_SLIDER_Twinkle_Count,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&EffectsPanel::OnBitmapButton_TwinkleCountClick);
    Connect(ID_BITMAPBUTTON_SLIDER_Twinkle_Steps,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&EffectsPanel::OnBitmapButton_TwinkleStepsClick);
    Connect(ID_BITMAPBUTTON_CHECKBOX_Twinkle_Strobe,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&EffectsPanel::OnBitmapButton_TwinkleStrobeClick);
    Connect(ID_BITMAPBUTTON_SingleStrand_Colors,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&EffectsPanel::OnBitmapButton_SingleStrand_ColorsClick);
    Connect(ID_BITMAPBUTTON_Number_Chases,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&EffectsPanel::OnBitmapButton_Number_ChasesClick);
    Connect(ID_BITMAPBUTTON_SLIDER_Color_Mix1,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&EffectsPanel::OnBitmapButton_Color_Mix1Click);
    Connect(ID_BITMAPBUTTON_SLIDER_Chase_Spacing1,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&EffectsPanel::OnBitmapButton_Chase_Spacing1Click);
    Connect(ID_BITMAPBUTTON_CHOICE_Chase_Type1,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&EffectsPanel::OnBitmapButton_Chase_Type1Click);
    Connect(ID_BITMAPBUTTON_CHECKBOX_Chase_3dFade1,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&EffectsPanel::OnBitmapButton_Chase_3dFade1Click);
    Connect(ID_BITMAPBUTTON_Chase_Group_All,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&EffectsPanel::OnBitmapButton_Chase_Group_AllClick);
    Connect(ID_BITMAPBUTTON2,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&EffectsPanel::OnBitmapButton_WaveTypeClick);
    Connect(ID_BITMAPBUTTON3,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&EffectsPanel::OnBitmapButton_Wave_FillColorsClick);
    Connect(ID_BITMAPBUTTON4,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&EffectsPanel::OnBitmapButton_Wave_MirrorWaveClick);
    Connect(ID_BITMAPBUTTON5,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&EffectsPanel::OnBitmapButton_NumberWavesClick);
    Connect(ID_BITMAPBUTTON6,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&EffectsPanel::OnBitmapButton_Wave_ThicknessClick);
    Connect(ID_BITMAPBUTTON7,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&EffectsPanel::OnBitmapButton_Wave_HeightClick);
    Connect(ID_BITMAPBUTTON8,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&EffectsPanel::OnBitmapButton_Wave_DirectionClick);
    Connect(ID_TEXTCTRL_X_Y,wxEVT_COMMAND_TEXT_UPDATED,(wxObjectEventFunction)&EffectsPanel::OnTextCtrl1Text);
    Connect(ID_CHOICEBOOK1,wxEVT_COMMAND_CHOICEBOOK_PAGE_CHANGED,(wxObjectEventFunction)&EffectsPanel::OnChoicebook1PageChanged);
    Connect(ID_BITMAPBUTTON_CHOICEBOOK1,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&EffectsPanel::OnBitmapButton_LayerEffectClick);
    Connect(ID_CHECKBOX_Palette1,wxEVT_COMMAND_CHECKBOX_CLICKED,(wxObjectEventFunction)&EffectsPanel::OnCheckBox_PaletteClick);
    Connect(ID_BUTTON_Palette1,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&EffectsPanel::OnButton_PaletteNumberClick);
    Connect(ID_BITMAPBUTTON_BUTTON_Palette1,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&EffectsPanel::OnBitmapButton_Palette1Click);
    Connect(ID_CHECKBOX_Palette2,wxEVT_COMMAND_CHECKBOX_CLICKED,(wxObjectEventFunction)&EffectsPanel::OnCheckBox_PaletteClick);
    Connect(ID_BUTTON_Palette2,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&EffectsPanel::OnButton_PaletteNumberClick);
    Connect(ID_BITMAPBUTTON_BUTTON_Palette2,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&EffectsPanel::OnBitmapButton_Palette2Click);
    Connect(ID_CHECKBOX_Palette3,wxEVT_COMMAND_CHECKBOX_CLICKED,(wxObjectEventFunction)&EffectsPanel::OnCheckBox_PaletteClick);
    Connect(ID_BUTTON_Palette3,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&EffectsPanel::OnButton_PaletteNumberClick);
    Connect(ID_BITMAPBUTTON_BUTTON_Palette3,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&EffectsPanel::OnBitmapButton_Palette3Click);
    Connect(ID_CHECKBOX_Palette4,wxEVT_COMMAND_CHECKBOX_CLICKED,(wxObjectEventFunction)&EffectsPanel::OnCheckBox_PaletteClick);
    Connect(ID_BUTTON_Palette4,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&EffectsPanel::OnButton_PaletteNumberClick);
    Connect(ID_BITMAPBUTTON_BUTTON_Palette4,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&EffectsPanel::OnBitmapButton_Palette4Click);
    Connect(ID_CHECKBOX_Palette5,wxEVT_COMMAND_CHECKBOX_CLICKED,(wxObjectEventFunction)&EffectsPanel::OnCheckBox_PaletteClick);
    Connect(ID_BUTTON_Palette5,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&EffectsPanel::OnButton_PaletteNumberClick);
    Connect(ID_BITMAPBUTTON_BUTTON_Palette5,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&EffectsPanel::OnBitmapButton_Palette5Click);
    Connect(ID_CHECKBOX_Palette6,wxEVT_COMMAND_CHECKBOX_CLICKED,(wxObjectEventFunction)&EffectsPanel::OnCheckBox_PaletteClick);
    Connect(ID_BUTTON_Palette6,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&EffectsPanel::OnButton_PaletteNumberClick);
    Connect(ID_BITMAPBUTTON_BUTTON_Palette6,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&EffectsPanel::OnBitmapButton_Palette6Click);
    Connect(ID_SLIDER_Speed,wxEVT_SCROLL_TOP|wxEVT_SCROLL_BOTTOM|wxEVT_SCROLL_LINEUP|wxEVT_SCROLL_LINEDOWN|wxEVT_SCROLL_PAGEUP|wxEVT_SCROLL_PAGEDOWN|wxEVT_SCROLL_THUMBTRACK|wxEVT_SCROLL_THUMBRELEASE|wxEVT_SCROLL_CHANGED,(wxObjectEventFunction)&EffectsPanel::OnSlider_SpeedCmdScroll);
    Connect(ID_SLIDER_Speed,wxEVT_COMMAND_SLIDER_UPDATED,(wxObjectEventFunction)&EffectsPanel::OnSlider_SpeedCmdScroll);
    Connect(ID_BITMAPBUTTON_SLIDER_Speed,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&EffectsPanel::OnBitmapButton_SpeedClick);
    Connect(ID_BITMAPBUTTON_TEXTCTRL_Fadein,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&EffectsPanel::OnBitmapButton_FadeOutClick);
    Connect(ID_BITMAPBUTTON_TEXTCTRL_Fadeout,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&EffectsPanel::OnBitmapButton_FadeInClick);
    Connect(ID_BITMAPBUTTON_CHECKBOX_FitToTime,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&EffectsPanel::OnBitmapButton_FitToTimeClick);
    Connect(ID_BITMAPBUTTON_OverlayBkg,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&EffectsPanel::OnBitmapButton_OverlayBkgClick);
    //*)

    this->SetName(name);
}

EffectsPanel::~EffectsPanel()
{
    //(*Destroy(EffectsPanel)
    //*)
}

// idx is 1-6
wxColour EffectsPanel::GetPaletteColor(int idx)
{
    switch (idx)
    {
    case 1:
        return Button_Palette1->GetBackgroundColour();
    case 2:
        return Button_Palette2->GetBackgroundColour();
    case 3:
        return Button_Palette3->GetBackgroundColour();
    case 4:
        return Button_Palette4->GetBackgroundColour();
    case 5:
        return Button_Palette5->GetBackgroundColour();
    case 6:
        return Button_Palette6->GetBackgroundColour();
    }
    return *wxBLACK;
}

// idx is 1-6
void EffectsPanel::SetPaletteColor(int idx, const wxColour* c)
{
    switch (idx)
    {
    case 1:
        SetButtonColor(Button_Palette1,c);
        break;
    case 2:
        SetButtonColor(Button_Palette2,c);
        break;
    case 3:
        SetButtonColor(Button_Palette3,c);
        break;
    case 4:
        SetButtonColor(Button_Palette4,c);
        break;
    case 5:
        SetButtonColor(Button_Palette5,c);
        break;
    case 6:
        SetButtonColor(Button_Palette6,c);
        break;
    }
}

//allow array-like access to checkboxes also -DJ
wxCheckBox* EffectsPanel::GetPaletteCheckbox(int idx)
{
    switch (idx)   // idx is 1-6
    {
    case 1:
        return CheckBox_Palette1;
    case 2:
        return CheckBox_Palette2;
    case 3:
        return CheckBox_Palette3;
    case 4:
        return CheckBox_Palette4;
    case 5:
        return CheckBox_Palette5;
    case 6:
        return CheckBox_Palette6;
    }
    return CheckBox_Palette1; //0;
}

//allow array-like access to buttons also -DJ
wxButton* EffectsPanel::GetPaletteButton(int idx)
{
    switch (idx)   // idx is 1-6
    {
    case 1:
        return Button_Palette1;
    case 2:
        return Button_Palette2;
    case 3:
        return Button_Palette3;
    case 4:
        return Button_Palette4;
    case 5:
        return Button_Palette5;
    case 6:
        return Button_Palette6;
    }
    return Button_Palette1; //0;
}

void EffectsPanel::SetButtonColor(wxButton* btn, const wxColour* c)
{
    btn->SetBackgroundColour(*c);
    int test=c->Red()*0.299 + c->Green()*0.587 + c->Blue()*0.114;
    btn->SetForegroundColour(test < 186 ? *wxWHITE : *wxBLACK);

#ifdef __WXOSX__
    //OSX does NOT allow active buttons to have a color other than the default.
    //We'll use an image of the appropriate color instead
    wxImage image(15, 15);
    image.SetRGB(wxRect(0, 0, 15, 15),
                 c->Red(), c->Green(), c->Blue());
    wxBitmap bmp(image);

    btn->SetBitmap(bmp);
    btn->SetLabelText("");
#endif
}

void EffectsPanel::SetDefaultPalette()
{
    SetButtonColor(Button_Palette1,wxRED);
    SetButtonColor(Button_Palette2,wxGREEN);
    SetButtonColor(Button_Palette3,wxBLUE);
    SetButtonColor(Button_Palette4,wxYELLOW);
    SetButtonColor(Button_Palette5,wxWHITE);
    SetButtonColor(Button_Palette6,wxBLACK);
}

int EffectsPanel::GetRandomSliderValue(wxSlider* slider)
{
    return isRandom(slider)? rand() % (slider->GetMax()-slider->GetMin()) + slider->GetMin(): slider->GetValue(); //want random? -DJ
}

// returns ",E1_" or ",E2_"
wxString EffectsPanel::GetAttrPrefix()
{
    wxString WinName=this->GetName();
    return ",E" + WinName.Right(1) + "_";
}

// this is recursive
wxString EffectsPanel::GetEffectStringFromWindow(wxWindow *ParentWin)
{
    wxString s,ChildName,AttrName;
    int i;
    wxString prefix=GetAttrPrefix();
    wxWindowList &ChildList = ParentWin->GetChildren();
    for ( wxWindowList::Node *node = ChildList.GetFirst(); node; node = node->GetNext() )
    {
        wxWindow *ChildWin = (wxWindow *)node->GetData();
        ChildName=ChildWin->GetName();
        AttrName=prefix+ChildName.Mid(3)+"=";
        if (ChildName.StartsWith("ID_SLIDER"))
        {
            wxSlider* ctrl=(wxSlider*)ChildWin;
            s+=AttrName+wxString::Format("%d",ctrl->GetValue());
        }
        else if (ChildName.StartsWith("ID_TEXTCTRL"))
        {
            wxTextCtrl* ctrl=(wxTextCtrl*)ChildWin;
            wxString v = ctrl->GetValue();
            v.Replace(",", "&comma;", true); //kludge: need to escape commas; parser doesn't handle them -DJ
            s += AttrName + v;
        }
        else if (ChildName.StartsWith("ID_CHOICE"))
        {
            wxChoice* ctrl=(wxChoice*)ChildWin;
            s+=AttrName+ctrl->GetStringSelection();
        }
        else if (ChildName.StartsWith("ID_CHECKBOX"))
        {
            wxCheckBox* ctrl=(wxCheckBox*)ChildWin;
            wxString v=(ctrl->IsChecked()) ? "1" : "0";
            s+=AttrName+v;
        }
        else if (ChildName.StartsWith("ID_NOTEBOOK"))
        {
            wxNotebook* ctrl=(wxNotebook*)ChildWin;
            for(i=0; i<ctrl->GetPageCount(); i++)
            {
                s+=GetEffectStringFromWindow(ctrl->GetPage(i));
            }
        }
    }
    return s;
}

// assumes effidx does not refer to Text effect
//modifed for partially random, allow random colors also -DJ
//void djdebug(const char* fmt, ...); //_DJ
wxString EffectsPanel::GetRandomEffectString(int effidx)
{
    wxString s,ChildName,AttrName;
    wxString prefix=GetAttrPrefix();

//    djdebug("GetRandomEffectString: %s rnd? %d", (const char*)Slider_Speed->GetName().c_str(), isRandom(Slider_Speed));

    // get speed
    s = prefix + wxString::Format("SLIDER_Speed=%d", GetRandomSliderValue(Slider_Speed));

    // get effect controls
    wxWindowList &ChildList = Choicebook1->GetPage(effidx)->GetChildren();
    for ( wxWindowList::Node *node = ChildList.GetFirst(); node; node = node->GetNext() )
    {
        wxWindow *ChildWin = (wxWindow *)node->GetData();
        ChildName = ChildWin->GetName();
        AttrName = prefix + ChildName.Mid(3) + "=";

        if (ChildName.StartsWith("ID_SLIDER"))
        {
            wxSlider* ctrl=(wxSlider*)ChildWin;
            if (ChildName.Contains("Spirograph_r"))
            {
                // always set little radius, r, to its minimum value
                s += AttrName + wxString::Format("%d", 0);
            }
            else
            {
                s += AttrName + wxString::Format("%d", GetRandomSliderValue(ctrl));
            }
        }
        else if (ChildName.StartsWith("ID_TEXTCTRL"))
        {
            wxTextCtrl* ctrl=(wxTextCtrl*)ChildWin;
            wxString v = ctrl->GetValue();
            v.Replace(",", "&comma;", true); //kludge: need to escape commas; parser doesn't handle them -DJ
            s += AttrName + v;
        }
        else if (ChildName.StartsWith("ID_CHOICE"))
        {
            wxChoice* ctrl=(wxChoice*)ChildWin;
            s += AttrName + ctrl->GetString(isRandom(ctrl)? rand()%ctrl->GetCount(): ctrl->GetSelection()); //-DJ
        }
        else if (ChildName.StartsWith("ID_CHECKBOX"))
        {
            if(ChildName.Contains("Spirograph_Animate"))
            {
                // always animate spirograph
                s+=AttrName+wxString::Format("%d", 1 );
            }
            else
            {
                wxCheckBox* ctrl = (wxCheckBox*)ChildWin;
                wxString v = (isRandom(ctrl)? (rand()%2): ctrl->GetValue())? "1" : "0"; //want random? -DJ
                s += AttrName + v;
            }
        }
    }

    // get palette
    wxColour color;
    for (int i=1; i<=PALETTE_SIZE; i++)
    {
        color = GetPaletteColor(i);
        AttrName.Printf("BUTTON_Palette%d=", i);
        s += prefix+AttrName + color.GetAsString(wxC2S_HTML_SYNTAX);
        wxString v = (isRandom(GetPaletteButton(i))? rand() % 2: GetPaletteCheckbox(i)->GetValue()) ? "1" : "0";
        AttrName.Printf("CHECKBOX_Palette%d=", i);
        s += prefix + AttrName + v;
    }
    return s;
}


wxString EffectsPanel::GetEffectString()
{
    wxString s,ChildName,AttrName;
    wxString prefix=GetAttrPrefix();
    s=prefix+wxString::Format("SLIDER_Speed=%d",Slider_Speed->GetValue());
    s+=prefix+"TEXTCTRL_Fadein="+TextCtrl_Fadein->GetValue();
    s+=prefix+"TEXTCTRL_Fadeout="+TextCtrl_Fadeout->GetValue();
    s+=prefix+wxString::Format("CHECKBOX_FitToTime=%d",CheckBox_FitToTime->GetValue()?1:0);
    s+=prefix+wxString::Format("CHECKBOX_OverlayBkg=%d",CheckBox_OverlayBkg->GetValue()?1:0);

    // get effect controls
    s+=GetEffectStringFromWindow(Choicebook1->GetCurrentPage());

    // get palette
    wxColour color;
    for (int i=1; i<=PALETTE_SIZE; i++)
    {
        color=GetPaletteColor(i);
        AttrName.Printf("BUTTON_Palette%d=",i);
        s+=prefix+AttrName+color.GetAsString(wxC2S_HTML_SYNTAX);
        AttrName.Printf("CHECKBOX_Palette%d",i);
        wxCheckBox* ctrl=(wxCheckBox*)wxWindow::FindWindowByName("ID_"+AttrName,this);
        wxString v=(ctrl->IsChecked()) ? "1" : "0";
        s+=prefix+AttrName+"="+v;
    }
    return s;
}

//selectable clear canvas before render: -DJ
//this allows multiple effects to be overlayed for composite models
//also provides useful "bread crumbs" for some effects such as Spirograph
bool EffectsPanel::WantOverlayBkg(void)
{
    return CheckBox_OverlayBkg->GetValue();
}


void EffectsPanel::OnButton_Pictures_FilenameClick(wxCommandEvent& event)
{
    wxString filename = wxFileSelector( "Choose Image", *CurrentDir, "", "", wxImage::GetImageExtWildcard(), wxFD_OPEN );
    if (!filename.IsEmpty()) TextCtrl_Pictures_Filename->SetValue(filename);
}




void EffectsPanel::OnButton_Glediator_FilenameClick(wxCommandEvent& event)
{
    wxString filename = wxFileSelector( "Choose Glediator *.gled", *CurrentDir, "", "", "Glediator files (*.gled)|*.gled", wxFD_OPEN );
    if (!filename.IsEmpty()) TextCtrl_Glediator_Filename->SetValue(filename);
}


void EffectsPanel::OnButton_Piano_CueFilenameClick(wxCommandEvent& event)
{
    wxString filename = wxFileSelector( "Choose Notes Cue File", *CurrentDir, "", "", "Notes cue files (*notes.txt;*.not)|*notes.txt;*.not", wxFD_OPEN );
    if (!filename.IsEmpty()) TextCtrl_Piano_CueFilename->SetValue(filename);
}

void EffectsPanel::OnButton_Piano_MapFilenameClick(wxCommandEvent& event)
{
    wxString filename = wxFileSelector( "Choose Cue Map File", *CurrentDir, "", "", "Cue map files (*map.txt;*.map)|*map.txt;*.map", wxFD_OPEN );
    if (!filename.IsEmpty()) TextCtrl_Piano_MapFilename->SetValue(filename);
}

void EffectsPanel::OnButton_Piano_ShapeFilenameClick(wxCommandEvent& event)
{
    wxString filename = wxFileSelector( "Choose Shapes File", *CurrentDir, "", "", "Key shapes files " + wxImage::GetImageExtWildcard(), wxFD_OPEN );
    if (!filename.IsEmpty()) TextCtrl_Piano_ShapeFilename->SetValue(filename);
}

void EffectsPanel::UpdateFont(wxTextCtrl* FontCtrl)
{
    wxFont oldfont,newfont;
    oldfont.SetNativeFontInfoUserDesc(FontCtrl->GetValue());
    newfont=wxGetFontFromUser(this,oldfont);
    if (newfont.IsOk())
    {
        wxString FontDesc=newfont.GetNativeFontInfoUserDesc();
        FontDesc.Replace(" unknown-90","");
        FontCtrl->SetValue(FontDesc);
    }
}

void EffectsPanel::OnButton_Text_Font1Click(wxCommandEvent& event)
{
    UpdateFont(TextCtrl_Text_Font1);
}

void EffectsPanel::OnButton_Text_Font2Click(wxCommandEvent& event)
{
    UpdateFont(TextCtrl_Text_Font2);
}

void EffectsPanel::OnButton_Text_Font3Click(wxCommandEvent& event)
{
    UpdateFont(TextCtrl_Text_Font3);
}

void EffectsPanel::OnButton_Text_Font4Click(wxCommandEvent& event)
{
    UpdateFont(TextCtrl_Text_Font4);
}



void EffectsPanel::OnCheckBox_PaletteClick(wxCommandEvent& event)
{
    PaletteChanged=true;
}

void EffectsPanel::OnButton_PaletteNumberClick(wxCommandEvent& event)
{
    wxButton* w=(wxButton*)event.GetEventObject();
    wxColour color = w->GetBackgroundColour();
    colorData.SetColour(color);
    wxColourDialog dialog(this, &colorData);
    if (dialog.ShowModal() == wxID_OK)
    {
        wxColourData retData = dialog.GetColourData();
        wxColour color = retData.GetColour();
        SetButtonColor(w, &color);
        PaletteChanged=true;
    }
}

void EffectsPanel::OnChoicebook1PageChanged(wxChoicebookEvent& event)
{
    EffectChanged=true;
}

void EffectsPanel::UpdateSpeedText()
{
    TextCtrl_Speed->SetValue(wxString::Format("%d",Slider_Speed->GetValue()));
}

void EffectsPanel::OnSlider_SpeedCmdScroll(wxScrollEvent& event)
{
    UpdateSpeedText();
}


//new Piano effect UI handlers: -DJ
void EffectsPanel::OnPiano_StyleSelect(wxCommandEvent& event)
{
//TODO?
}

//#define WANT_DEBUG 99
//#include "djdebug.cpp"
void EffectsPanel::OnSlider_Piano_NumKeysCmdScroll(wxScrollEvent& event)
{
//    debug(1, "#keys %d", Slider_Piano_NumKeys->GetValue());
//    TextCtrl_Speed->SetValue(wxString::Format("%d",Slider_Speed->GetValue()));
//    txtCtrlSparkleFreq->SetValue(wxString::Format("%d",Slider_SparkleFrequency->GetValue()));
//wxMessageBox(wxString::Format("# Keys (%d):", Slider_Piano_NumKeys->GetValue()));
//broken, use wxsl_lables instead for now:
    StaticText_Piano_NumKeys->SetLabel(wxString::Format("# Keys (%d):", Slider_Piano_NumKeys->GetValue())); //show slider value for user friendliness
//    Layout();
}

void EffectsPanel::OnSlider_Piano_NumRowsCmdScroll(wxScrollEvent& event)
{
//    debug(1, "#rows %d", Slider_Piano_NumRows->GetValue());
//broken, use wxsl_lables instead for now:
    StaticText_Piano_NumRows->SetLabel(wxString::Format("# Rows (%d):", Slider_Piano_NumRows->GetValue())); //show slider value for user friendliness
//    Layout();
//wxMessageBox(wxString::Format("# Rows (%d):", Slider_Piano_NumRows->GetValue()));
}


//add lock/unlock/random state flags -DJ
//these could be used to make fields read-only, but initially they are just used for partially random effects
//void djdebug(const char* fmt, ...); //_DJ
//typedef enum { Normal, Locked, Random } EditState;
void EffectsPanel::setlock(wxButton* button) //, EditState& islocked)
{
    wxString parent = button->GetName();
    if (parent.StartsWith("ID_BITMAPBUTTON_")) parent = "ID_" + parent.substr(16); //map to associated control
    EditState& islocked = buttonState[std::string(parent)]; //creates entry if not there
//    djdebug("ctl %s was %d", (const char*)parent.c_str(), islocked);
    switch (islocked) //cycle thru states
    {
    case Locked:
        islocked = Random;
        button->SetBitmapLabel(BitmapButton_random->GetBitmapLabel());
        break;
//        case Random:
//            islocked = Normal;
//            button->SetBitmapLabel(BitmapButton_normal->GetBitmapLabel());
//            break;
    default:
        islocked = Locked;
        button->SetBitmapLabel(BitmapButton_locked->GetBitmapLabel());
        break;
    }
}
//#define isRandom(ctl)  (buttonState[std::string(ctl->GetName())] == Random)
bool EffectsPanel::isRandom_(wxControl* ctl, const char*debug)
{
//    if (!ctl->GetName().length()) djdebug("NO NAME FOR %s", debug);
    bool retval = (buttonState[std::string(ctl->GetName())] != Locked); //== Random);
//    djdebug("isRandom(%s) = %d", (const char*)ctl->GetName().c_str(), retval);
    return retval;
}
bool EffectsPanel::isRandom_(void)
{
    return isRandom(Choicebook1);
}

#define showlock(name)  \
/*EditState isLockedFx_##name = Normal;*/ \
void EffectsPanel::OnBitmapButton_##name##Click(wxCommandEvent& event) \
{ \
    setlock(BitmapButton_##name/*, isLockedFx_##name*/); \
}
showlock(LayerEffect)
showlock(Palette1)
showlock(Palette2)
showlock(Palette3)
showlock(Palette4)
showlock(Palette5)
showlock(Palette6)
showlock(Speed)
showlock(FadeOut)
showlock(FadeIn)
showlock(FitToTime)
showlock(OverlayBkg)
showlock(PaletteRep)
showlock(Direction)
showlock(Highlight)
showlock(3D)
showlock(ButterflyColors)
showlock(ButterflyStyle)
showlock(ButterflyChunks)
showlock(ButterflySkip)
showlock(CirclesCount)
showlock(CirclesSize)
showlock(CirclesBounce)
showlock(CirclesRadial)
showlock(CirclesRadial3D)
showlock(CirclesCollide)
showlock(CirclesPlasma)
showlock(RandomMotion)

showlock(ColorWashCount)
showlock(ColorWashHFade)
showlock(ColorWashVFade)
showlock(CurtainEdge)
showlock(CurtainEffect)
showlock(CurtainSwag)
showlock(CurtainRepeat)
showlock(FireHeight)
showlock(FireWidth)
showlock(FireGrow)
showlock(FireworksNumberExplosions)
showlock(FireworksVelocity)
showlock(FireworksFade)
showlock(FireworksCount)
showlock(GarlandsType)
showlock(GarlandsSpacing)
showlock(LifeCount)
showlock(LifeSpeed)
showlock(MeteorsType)
showlock(MeteorsEffect)
showlock(MeteorsCount)
showlock(MeteorsLength)
showlock(MeteorsSwirlIntensity)
showlock(Piano_Style)
showlock(Piano_NumKeys)
showlock(Piano_NumRows)
showlock(Piano_KeyPlacement)
showlock(Piano_Clipping)
showlock(Piano_CueFilename)
showlock(Piano_MapFilename)
showlock(Piano_ShapeFilename)
showlock(PicturesFilename)
showlock(PicturesDirection)
showlock(PicturesSpeed)
showlock(SnowflakesCount)
showlock(SnowflakesType)
showlock(SnowstormCount)
showlock(SnowstormLength)
showlock(SpiralsCount)
showlock(SpiralsRotation)
showlock(SpiralsThickness)
showlock(SpiralsDirection)
showlock(Spirals3D)
showlock(SpiralsShrink)
showlock(SpiralsBlend)
showlock(SpiralsGrow)
showlock(SpirographR)
showlock(Spirographr)
showlock(SpirographAnimate)
showlock(Spirographd)

showlock(TextLine1)
showlock(TextLine2)
showlock(TextLine3)
showlock(TextLine4)


showlock(TextFont1)
showlock(TextDir1)
showlock(TextToCenter1)
showlock(TextEffect1)
showlock(TextCount1)
showlock(TextPosition1)

showlock(TextFont2)
showlock(TextDir2)
showlock(TextToCenter2)
showlock(TextEffect2)
showlock(TextCount2)
showlock(TextPosition2)

showlock(TextFont3)
showlock(TextDir3)
showlock(TextToCenter3)
showlock(TextEffect3)
showlock(TextCount3)
showlock(TextPosition3)

showlock(TextFont4)
showlock(TextDir4)
showlock(TextToCenter4)
showlock(TextEffect4)
showlock(TextCount4)
showlock(TextPosition4)


showlock(TreeBranches)
showlock(TwinkleCount)
showlock(TwinkleSteps)
showlock(TwinkleStrobe)
//lock buttons for Single Strand:
showlock(SingleStrand_Colors)
showlock(Number_Chases)
showlock(Color_Mix1)
showlock(Chase_Spacing1)
showlock(Chase_Type1)
showlock(Chase_3dFade1)
showlock(Chase_Group_All)

showlock(WaveType)
showlock(Wave_FillColors)
showlock(Wave_MirrorWave)
showlock(NumberWaves)
showlock(Wave_Thickness)
showlock(Wave_Height)
showlock(Wave_Direction)

showlock(Glediator_Filename)





void EffectsPanel::OnChoice_Bars_DirectionSelect(wxCommandEvent& event)
{
}


void EffectsPanel::OnTextCtrl2Text(wxCommandEvent& event)
{
}

void EffectsPanel::OnTextCtrl1Text(wxCommandEvent& event)
{
}
