#include "FontManager.h"
#include "../../../include/xLightsFontImages.h"

#define FONT_BITMAP_COLUMNS 8
#define FONT_BITMAP_ROWS 16

xlFont::xlFont(wxBitmap& bitmap_)
: bitmap(bitmap_)
{
    char_width = ((bitmap.GetWidth()-1) / FONT_BITMAP_COLUMNS)-1;
    char_height = ((bitmap.GetHeight()-1) / FONT_BITMAP_ROWS)-1;
    caps_height = char_height;
}

xlFont::~xlFont()
{
}

void xlFont::GatherInfo()
{
    int x_pos, y_pos;
    int red, green, blue;
    int index = 0;
    for( int i = 0; i < 128; i++ )
    {
        widths[i] = char_width;
    }
    wxImage image = bitmap.ConvertToImage();
    for( int y = 0; y < FONT_BITMAP_ROWS; y++)
    {
        y_pos = (y * (char_height + 1)) + 1;
        for( int x = 0; x < FONT_BITMAP_COLUMNS; x++)
        {
            x_pos = (x * (char_width + 1)) + 1;
            for( int z = 0; z < char_width; z++ )
            {
                red = image.GetRed(x_pos+z, y_pos);
                green = image.GetGreen(x_pos+z, y_pos);
                blue = image.GetBlue(x_pos+z, y_pos);
                if( red == 0 && green == 255 && blue == 255 )
                {
                    widths[index] = z;
                    break;
                }
            }
            index++;
        }
    }
}

FontManager::FontManager()
{
}

std::vector<wxBitmap> FontManager::bitmaps;
std::vector<xlFont> FontManager::fonts;
bool FontManager::initialized = false;
wxArrayString FontManager::names;

FontManager::~FontManager()
{
}

void FontManager::init()
{
    if( !initialized )
    {
        bitmaps.push_back(wxBITMAP_PNG_FROM_DATA(font_6_5x6_thin_system));
        bitmaps.push_back(wxBITMAP_PNG_FROM_DATA(font_6_5x6_thin_vertical_system));
        bitmaps.push_back(wxBITMAP_PNG_FROM_DATA(font_6_6x6_thin_system));
        bitmaps.push_back(wxBITMAP_PNG_FROM_DATA(font_6_6x6_thin_vertical_system));
        bitmaps.push_back(wxBITMAP_PNG_FROM_DATA(font_8_8x8_thin_system));
        bitmaps.push_back(wxBITMAP_PNG_FROM_DATA(font_8_8x8_thin_vertical_system));
        bitmaps.push_back(wxBITMAP_PNG_FROM_DATA(font_10_12x12_bold_system));
        bitmaps.push_back(wxBITMAP_PNG_FROM_DATA(font_10_12x12_bold_vertical_system));
        bitmaps.push_back(wxBITMAP_PNG_FROM_DATA(font_10_12x12_thin_system));
        bitmaps.push_back(wxBITMAP_PNG_FROM_DATA(font_10_12x12_thin_vertical_system));
        bitmaps.push_back(wxBITMAP_PNG_FROM_DATA(font_12_15x15_bold_system));
        bitmaps.push_back(wxBITMAP_PNG_FROM_DATA(font_12_15x15_bold_vertical_system));

        for( int i = 0; i < bitmaps.size(); i++ )
        {
            fonts.push_back(xlFont(bitmaps[i]));
            fonts[i].GatherInfo();
        }

        for( int i = 0; i < fonts.size(); i++ )
        {
            wxArrayString parts = wxSplit(names[i], '-');
            fonts[i].SetCapsHeight(wxAtoi(parts[0]));
        }

        initialized = true;
    }
}

wxArrayString FontManager::get_font_names()
{
    if( names.size() == 0 )
    {
        names.Add("6-5x6 Thin");
        names.Add("6-5x6 Thin Vertical");
        names.Add("6-6x6 Thin");
        names.Add("6-6x6 Thin Vertical");
        names.Add("8-8x8 Thin");
        names.Add("8-8x8 Thin Vertical");
        names.Add("10-12x12 Bold");
        names.Add("10-12x12 Bold Vertical");
        names.Add("10-12x12 Thin");
        names.Add("10-12x12 Thin Vertical");
        names.Add("12-15x15 Bold");
        names.Add("12-15x15 Bold Vertical");
    }

    return names;
}

xlFont* FontManager::get_font(wxString font_name)
{
    for( int i = 0; i < bitmaps.size(); i++ )
    {
        if( names[i] == font_name )
        {
            return &fonts[i];
        }
    }
    return nullptr;
}

int FontManager::get_length(xlFont* font, wxString& text)
{
    if( text == "" ) return 0;
    int length = 0;
    int ascii;
    for( int i = 0; i < text.length(); i++ ) {
        ascii = (int)text[i];
        length += font->GetCharWidth(ascii);
    }
    return length;
}
