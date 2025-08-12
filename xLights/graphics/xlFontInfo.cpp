
#include "xlFontInfo.h"

#include "./utils/spdlog_macros.h"
#include <vector>
#include <map>

// Uncomment to enable the code to create any missing FontInfo's
// #define CREATE_MISSING_FONTS

static int TEXTURE_IDX = 1;
inline int NextFontTextureIdx() {
    TEXTURE_IDX++;
    return TEXTURE_IDX;
}

class FontInfoStruct {
public:
    FontInfoStruct(const char * const *d,
                   float mh, float mw, float md,
                   std::vector<float> w)
        : data(d), maxH(mh), maxW(mw), maxD(md), widths(w) {

    }

    const char * const *data;
    float maxH;
    float maxW;
    float maxD;
    std::vector<float> widths;
};
#include "fonts/fonts.h"

#define CASEFONT(x) case x: load(font##x); return true;

xlFontInfo::xlFontInfo() {
    id = NextFontTextureIdx();
}
xlFontInfo::~xlFontInfo() {
}

bool xlFontInfo::valid() const {
    return image.IsOk();
}

bool xlFontInfo::init(int size) {
    this->size = size;
    switch (size) {
        CASEFONT(8);
        CASEFONT(9);
        CASEFONT(10);
        CASEFONT(11);
        CASEFONT(12);
        CASEFONT(14);
        CASEFONT(16);
        CASEFONT(18);
        CASEFONT(20);
        CASEFONT(22);
        CASEFONT(24);
        CASEFONT(28);
        CASEFONT(32);
        CASEFONT(40);
        CASEFONT(44);
        CASEFONT(56);
        CASEFONT(88);
        default:
            return CreateMissingFont(size);
    }
}

bool xlFontInfo::load(const FontInfoStruct &fi) {
    maxW = fi.maxW;
    maxH = fi.maxH;
    maxD = fi.maxD;
    widths = fi.widths;
    image = wxImage(fi.data);
    if (!image.IsOk()) {
        return false;
    }
    image.InitAlpha();

    for (int x = 0; x < image.GetWidth(); x++) {
        for (int y = 0; y < image.GetHeight(); y++) {
            int alpha = image.GetRed(x, y);
            if (alpha) {
                image.SetRGB(x, y, 0, 0, 0);
                image.SetAlpha(x, y, alpha);
            } else {
                image.SetRGB(x, y, 0, 0, 0);
                image.SetAlpha(x, y, 0);
            }
        }
    }
    return image.IsOk();
}
float xlFontInfo::widthOf(const std::string &text, float factor) const {
    float w = 0;
    for (int idx = 0; idx < text.size(); idx++) {
        char ch = text[idx];
        if (ch < ' ' || ch > '~') {
            ch = '?';
        }
        w += widths[ch - ' '];
    }
    return w / factor;
}

void xlFontInfo::populate(xlVertexTextureAccumulator &va, float x, float yBase, const std::string &text, float factor) const {

    if (widths.size() == 0 || text.empty()) {
        return;
    }

    va.PreAlloc(6 * text.size());
    float textureHeight = image.GetHeight();
    float textureWidth = image.GetWidth();
    for (int idx = 0; idx < text.size(); idx++) {
        char ch = text[idx];
        if (ch < ' ' || ch > '~') {
            ch = '?';
        }
        if (ch == ' ') {
            x += widths[0] / factor;
            continue;
        }
        int linei = ch;
        linei -= ' ';
        linei /= 16.0;
        float line = linei;

        float pos = ch - ' ';
        pos -= linei * 16.0;

        float tx = 1 + (pos * (maxW + 5));
        float tx2 = tx + widths[ch - ' '];

        float x2 = x + float(widths[ch - ' ']) / factor;

        float ty2 = 2 + (line * (maxH + 5));
        float ty = ty2 + maxH;

        float y = yBase;
        float y2 = yBase - float(maxH) / factor;

        //samples need to be from within the pixel
        ty += 0.45f;
        ty2 += 0.45f;
        tx += 0.45f;
        tx2 += 0.45f;

        tx /= textureWidth;
        tx2 /= textureWidth;
        ty2 /= textureHeight;
        ty /= textureHeight;

        y += 0.25f/factor;
        y2 += 0.25f/factor;
        x += 0.25f/factor;
        x2 += 0.25f/factor;

        va.AddVertex(x, y, tx, ty);
        va.AddVertex(x, y2, tx, ty2);
        va.AddVertex(x2, y2, tx2, ty2);
        va.AddVertex(x2, y2, tx2, ty2);
        va.AddVertex(x2, y, tx2, ty);
        va.AddVertex(x, y, tx, ty);

        x += widths[ch - ' '] / factor;
    }
}

static std::map<unsigned int, xlFontInfo> FONTS;
const xlFontInfo &xlFontInfo::FindFont(int size) {
    int tsize = size;
    while (!FONTS[tsize].valid() && tsize > 0) {
        if (FONTS[tsize].init(tsize)) {
            return FONTS[tsize];
        }
        tsize--;
    }
    tsize = size;
    while (!FONTS[tsize].valid() && tsize <= 88) {
        if (FONTS[tsize].init(tsize)) {
            return FONTS[tsize];
        }
        tsize++;
    }

    if (!FONTS[tsize].valid()) {
        
        LOG_WARN("xlFontTexture had trouble creating font size %d ... this could end badly.", size);
    }

    return FONTS[tsize];
}


#define USEAA true
#define NUMLINES 6
#define NUMCHARS 16

bool xlFontInfo::CreateMissingFont(int size) {
#ifdef CREATE_MISSING_FONTS
    wxString faceName = "Gil Sans";
    
    bool useAA = USEAA;
    if (size <= 12)  {
        useAA = false;
    }
    
    wxGraphicsContext *ctx = wxGraphicsContext::Create();
    ctx->SetInterpolationQuality(wxInterpolationQuality::wxINTERPOLATION_BEST);
    ctx->SetCompositionMode(wxCompositionMode::wxCOMPOSITION_OVER);
    if (useAA) {
        ctx->SetAntialiasMode(wxANTIALIAS_DEFAULT);
        ctx->SetFont(ctx->CreateFont(size, faceName, wxFONTFLAG_ANTIALIASED, *wxWHITE));
    } else {
        ctx->SetAntialiasMode(wxANTIALIAS_NONE);
        ctx->SetFont(ctx->CreateFont(size, faceName, wxFONTFLAG_NOT_ANTIALIASED, *wxWHITE));
    }
    maxW = 0;
    maxH = 0;
    maxD = 0;
    for (char c = ' '; c <= '~'; c++) {
        wxString s = c;
        double width, height, desc, el;
        ctx->GetTextExtent(s, &width, &height, &desc, &el);
        maxW = std::max(maxW, (float)width);
        maxH = std::max(maxH, (float)height);
        maxD = std::max(maxD, (float)desc);
    }
    delete ctx;
    
    maxW += 1.0; // allow room to possibly have to handle some kerning
    
    
    int imgh = (maxH + 5) * 6;
    int imgw = (maxW + 5) * 16;
    float power_of_two_that_gives_correct_width=std::log((float)imgw)/std::log(2.0);
    float power_of_two_that_gives_correct_height=std::log((float)imgh)/std::log(2.0);
    imgw=(int)std::pow( 2.0, (int)(std::ceil(power_of_two_that_gives_correct_width)) );
    imgh=(int)std::pow( 2.0, (int)(std::ceil(power_of_two_that_gives_correct_height)) );
    
    
    wxImage cimg(imgw, imgh);
    cimg.InitAlpha();
    
    ctx = wxGraphicsContext::Create(cimg);
    ctx->SetPen( *wxWHITE_PEN );
    ctx->SetInterpolationQuality(wxInterpolationQuality::wxINTERPOLATION_BEST);
    ctx->SetCompositionMode(wxCompositionMode::wxCOMPOSITION_OVER);
    
    if (useAA) {
        ctx->SetAntialiasMode(wxANTIALIAS_DEFAULT);
        ctx->SetFont(ctx->CreateFont(size, faceName, wxFONTFLAG_ANTIALIASED, *wxWHITE));
    } else {
        ctx->SetAntialiasMode(wxANTIALIAS_NONE);
        ctx->SetFont(ctx->CreateFont(size, faceName, wxFONTFLAG_NOT_ANTIALIASED, *wxWHITE));
    }
    
    int count = 0;
    int line = 0;
    widths.clear();
    widths.resize('~' - ' ' + 1);
    for (char c = ' '; c <= '~'; c++) {
        wxString s = c;
        ctx->DrawText(s, 2 + (count * (maxW + 5)), 2 + line * (maxH + 5));
        
        double width, height, desc, el;
        ctx->GetTextExtent(s, &width, &height, &desc, &el);
        widths[c - ' '] = width;
        count++;
        if (count == 16) {
            count = 0;
            line++;
        }
    }
    delete ctx;
    
    
    for (int l = 0; l < NUMLINES; l++) {
        for (int c = 0; c < NUMCHARS; c++) {
            bool kerned = false;
            char ch = (l * NUMCHARS + c);
            for (int y = 0; y < (maxH + 3) && !kerned; y++) {
                int notblack = cimg.GetRed(2 + c * (maxW + 5), y + 1 + l * (maxH + 5));
                if (notblack) {
                    //the antialiasing has caused part of the pixel to bleed to the left
                    //we need to move the entire thing to the right
                    kerned = true;
                    //printf("Kerned: %c\n", (ch + ' '));
                }
            }
            if (kerned) {
                for (int x = (maxW-1); x > 1; x--) {
                    int xpos = c * (maxW + 5) + x;
                    for (int y = 1; y <= (maxH + 4); y++) {
                        int ypos = y  + l * (maxH + 5);
                        int clr = cimg.GetRed(xpos - 1, ypos);
                        cimg.SetRGB(xpos, ypos, clr, clr, clr);
                    }
                }
                for (int y = 1; y <= (maxH + 4); y++) {
                    int ypos = y  + l * (maxH + 5);
                    cimg.SetRGB(c * (maxW + 5) + 1, ypos, 0, 0, 0);
                }
                widths[ch] += 1.0;
            }
            /*
             if ((ch + ' ') == 'j') {
             printf("%f  %f\n", c * (maxW + 5) + 1, widths[ch]);
             }
             */
        }
    }
    
    //used to output data that can be used to static generation above
    
    wxString fn = wxFileName::CreateTempFileName("font_");
    //wxString fn2 = wxString::Format("~/tmp/font_%d.xpm", size);
    cimg.SaveFile(fn, wxBITMAP_TYPE_XPM);
    printf("%s\n", (const char*)fn.c_str());
    
    FILE *f = fopen(fn.c_str(), "r");
    fseek(f, 0, SEEK_END);
    size_t sz = ftell(f);
    fseek(f, 0, SEEK_SET);
    char *buf = (char *)malloc(sz);
    size_t r = 0;
    while (r < sz) {
        size_t i = fread(buf + r, 1, sz - r, f);
        if (i > 0) {
            r += i;
        } else {
            break;
        }
    }
    fclose(f);
    int cnt = 0;
    bool inQuote = false;
    for (int x = 0; x < sz; x++) {
        if (buf[x] == '"') {
            inQuote = !inQuote;
            buf[x] = 0;
        } else if (!inQuote) {
            if (buf[x] == ',') {
                cnt++;
            }
            buf[x] = 0;
        }
    }
    std::vector<char *> data;
    data.resize(cnt + 1);
    cnt = 0;
    for (int x = 0; x < sz; x++) {
        if (buf[x] != 0) {
            data[cnt] = &buf[x];
            printf("%d:  %d\n", cnt, (int)strlen(&buf[x]));
            x += strlen(&buf[x]) - 1;
            cnt++;
        }
    }
    
    printf("#include \"font_%d.xpm\"\n", size);
    if (size == 10) {
        widths['b' - ' '] += 0.5;
        widths['o' - ' '] += 0.5;
        widths['p' - ' '] += 0.5;
        widths['w' - ' '] += 0.5;
        widths['x' - ' '] += 0.75;
        widths['y' - ' '] += 0.75;
        widths['m' - ' '] -= 0.75;
        widths['A' - ' '] += 0.75;
        widths['C' - ' '] -= 0.5;
        widths['W' - ' '] += 0.5;
        widths['P' - ' '] += 0.5;
        widths['|' - ' '] -= 0.5;
    }
    if (size == 12) {
        widths['p' - ' '] += 0.5;
        widths['w' - ' '] += 0.75;
        widths['m' - ' '] -= 0.5;
        widths['e' - ' '] -= 0.5;
        widths['t' - ' '] += 0.5;
    }
    printf("static FontInfoStruct font%d(font_%d_xpm, %ff, %ff, %ff, {\n", size, size, maxH, maxW, maxD);
    printf("%ff", widths[0]);
    for (int x = 1; x < widths.size(); x++) {
        printf(", %ff", widths[x]);
    }
    printf("});\n");

    FontInfoStruct fis(&data[0], maxH, maxW, maxD, widths);
    load(fis);
    return true;
#else
    return false;
#endif
}
