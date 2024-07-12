
#include "xlFontInfo.h"

#include <log4cpp/Category.hh>
#include <vector>
#include <map>

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
        CASEFONT(10);
        CASEFONT(12);
        CASEFONT(14);
        CASEFONT(16);
        CASEFONT(18);
        CASEFONT(20);
        CASEFONT(24);
        CASEFONT(28);
        CASEFONT(40);
        CASEFONT(44);
        CASEFONT(56);
        CASEFONT(88);
        default:
            return false;
        /*default:
            printf("No FONT!!!! %d\n", size);
            ForceCreate(size);
            break;
         */
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

size_t xlFontInfo::CountChar(const std::string& text, char termChar) const {
    size_t count = 0;
	for (size_t i = 0; i < text.size(); ++i) {
		if (text[i] == termChar) {
			++count;
		}
	}
	return count;
}

void xlFontInfo::populate(xlVertexTextureAccumulator &va, float x, float yBase, const std::string &text, float factor, bool invert, float z) const {

    if (widths.size() == 0 || text.empty()) {
        return;
    }

    float origX = x;

    va.PreAlloc(6 * text.size());
    float textureHeight = image.GetHeight();
    float textureWidth = image.GetWidth();
    int ls = CountChar(text, '\n') + 1;
    int outputLine = ls - 1;
    for (int idx = 0; idx < text.size(); idx++) {
        char ch = text[idx];
        if (ch == '\n') {
            outputLine--;
            x = origX;
            continue;
        }
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

        float lineh = maxH / factor;
        float y = yBase + (outputLine * lineh);
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

        if (invert) {
            std::swap(ty, ty2);
        }
        va.AddVertex(x, y, z, tx, ty);
        va.AddVertex(x, y2, z, tx, ty2);
        va.AddVertex(x2, y2, z, tx2, ty2);
        va.AddVertex(x2, y2, z, tx2, ty2);
        va.AddVertex(x2, y, z, tx2, ty);
        va.AddVertex(x, y, z, tx, ty);

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
        static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));
        logger_base.warn("xlFontTexture had trouble creating font size %d ... this could end badly.", size);
    }

    return FONTS[tsize];
}

float xlFontInfo::ComputeFontSize(int& toffset, const float height, const float factor) {
    double fontSize = height - 10;
    toffset = 0;
    if (fontSize < 10) {
        if (factor > 1.5) {
            fontSize = 9;
            toffset = 1;
        } else {
            fontSize = 10;
            toffset = 2;
        }
    }
    return fontSize;
}
