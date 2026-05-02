#include "QtTextDrawingContext.h"
#include "../../src-core/utils/Color.h"

#include <QPainter>
#include <QFontMetrics>

#include <algorithm>
#include <cctype>
#include <sstream>

// ── Helpers ───────────────────────────────────────────────────────────────────

static std::string toLowerStr(std::string s) {
    std::transform(s.begin(), s.end(), s.begin(),
                   [](unsigned char c){ return std::tolower(c); });
    return s;
}

// Parse common xLights/wx font descriptor strings into TextFontInfo.
// Typical formats (from wx NativeFontInfoUserDesc on Windows/macOS):
//   "Arial 12"
//   "Arial 12 bold"
//   "Arial 12 bold italic"
//   "12pt Arial bold italic"
static TextFontInfo ParseFontString(const std::string& fontString) {
    TextFontInfo info;
    if (fontString.empty()) {
        info.pixelSize = 12;
        return info;
    }

    // Check for "Npt Face..." format first
    bool ptPrefix = false;
    std::string work = fontString;
    {
        std::istringstream ss(fontString);
        std::string tok;
        ss >> tok;
        std::string lo = toLowerStr(tok);
        if (!lo.empty() && lo.back() == 't') {
            // Strip trailing 't' and try to parse size
            lo.pop_back();
            char* end;
            long sz = std::strtol(lo.c_str(), &end, 10);
            if (end != lo.c_str() && sz > 0) {
                info.pixelSize = int(sz);
                ptPrefix = true;
                // Remaining stream has: Face bold italic etc.
                std::string rest;
                std::getline(ss, rest);
                work = rest;
            }
        }
    }

    // Tokenise the remaining string
    std::istringstream ss2(work);
    std::vector<std::string> tokens;
    {
        std::string tok;
        while (ss2 >> tok) tokens.push_back(tok);
    }

    // Pull out style keywords; collect face-name words and one size word
    std::vector<std::string> faceWords;
    for (const auto& t : tokens) {
        std::string lo = toLowerStr(t);
        if (lo == "bold")        { info.bold        = true; continue; }
        if (lo == "italic")      { info.italic       = true; continue; }
        if (lo == "slant")       { info.slant        = true; continue; }
        if (lo == "underlined" || lo == "underline")
                                  { info.underlined   = true; continue; }
        if (lo == "strikethrough")
                                  { info.strikethrough= true; continue; }
        if (lo == "light")       { info.light        = true; continue; }
        if (lo == "normal" || lo == "regular") continue;  // weight/style keywords

        // Is it a plain integer? → pixel size (only if not already set via "Npt")
        if (!ptPrefix || info.pixelSize == 0) {
            char* end;
            long sz = std::strtol(t.c_str(), &end, 10);
            if (end != t.c_str() && sz > 0 && info.pixelSize == 0) {
                info.pixelSize = int(sz);
                continue;
            }
        }

        faceWords.push_back(t);
    }

    // Build face name from remaining tokens
    if (!faceWords.empty()) {
        info.faceName = faceWords[0];
        for (size_t i = 1; i < faceWords.size(); ++i)
            info.faceName += ' ' + faceWords[i];
    }

    if (info.pixelSize <= 0) info.pixelSize = 12;
    return info;
}

// ── Factory ───────────────────────────────────────────────────────────────────

TextDrawingContext* QtTextDrawingContext::Create(int w, int h, bool) {
    return new QtTextDrawingContext(w > 0 ? w : 10, h > 0 ? h : 10);
}

TextFontInfo QtTextDrawingContext::ParseTextFont(const std::string& fontString) {
    return ParseFontString(fontString);
}

TextFontInfo QtTextDrawingContext::ParseShapeFont(const std::string& fontString) {
    TextFontInfo info = ParseFontString(fontString);
    info.light = true;
    return info;
}

// ── Constructor ───────────────────────────────────────────────────────────────

QtTextDrawingContext::QtTextDrawingContext(int w, int h)
    : _image(qMax(1, w), qMax(1, h), QImage::Format_ARGB32_Premultiplied)
{
    _image.fill(Qt::transparent);
}

// ── QFont helper ─────────────────────────────────────────────────────────────

QFont QtTextDrawingContext::ToQFont(const TextFontInfo& fi) {
    QFont f(fi.faceName.empty() ? QString("Arial") : QString::fromStdString(fi.faceName));
    f.setPixelSize(fi.pixelSize > 0 ? fi.pixelSize : 12);
    f.setBold(fi.bold);
    f.setItalic(fi.italic || fi.slant);
    f.setUnderline(fi.underlined);
    f.setStrikeOut(fi.strikethrough);
    if (fi.light) f.setWeight(QFont::Light);
    return f;
}

// ── Buffer lifecycle ──────────────────────────────────────────────────────────

void QtTextDrawingContext::ResetSize(int w, int h) {
    if (_image.width() != w || _image.height() != h) {
        _image = QImage(qMax(1, w), qMax(1, h), QImage::Format_ARGB32_Premultiplied);
    }
    _image.fill(Qt::transparent);
    _rgba.clear();
}

void QtTextDrawingContext::Clear() {
    _image.fill(Qt::transparent);
    _rgba.clear();
}

const uint8_t* QtTextDrawingContext::FlushAndGetImage(int* width, int* height) {
    if (width)  *width  = _image.width();
    if (height) *height = _image.height();

    const int n = _image.width() * _image.height();
    _rgba.resize(size_t(n) * 4);

    // QImage ARGB32_Premultiplied is stored as 0xAARRGGBB in native byte order.
    // TextEffect expects RGBA (R at offset 0).  Un-premultiply alpha and repack.
    const QRgb* src = reinterpret_cast<const QRgb*>(_image.constBits());
    uint8_t* dst = _rgba.data();
    for (int i = 0; i < n; ++i, ++src, dst += 4) {
        int a = qAlpha(*src);
        if (a > 0) {
            // Un-premultiply
            dst[0] = uint8_t(qRed(*src)   * 255 / a);
            dst[1] = uint8_t(qGreen(*src) * 255 / a);
            dst[2] = uint8_t(qBlue(*src)  * 255 / a);
        } else {
            dst[0] = dst[1] = dst[2] = 0;
        }
        dst[3] = uint8_t(a);
    }
    return _rgba.data();
}

// ── Font / color ──────────────────────────────────────────────────────────────

void QtTextDrawingContext::SetFont(const TextFontInfo& fi, const xlColor& color) {
    _font  = ToQFont(fi);
    _color = QColor(color.red, color.green, color.blue);
}

// ── Drawing ───────────────────────────────────────────────────────────────────

void QtTextDrawingContext::DrawText(const std::string& msg, int x, int y, double rotation) {
    QPainter p(&_image);
    p.setFont(_font);
    p.setPen(_color);
    p.setRenderHint(QPainter::TextAntialiasing, false);
    if (rotation != 0.0) {
        p.translate(x, y);
        p.rotate(-rotation);   // xLights rotation is counter-clockwise
        p.drawText(0, 0, QString::fromUtf8(msg.c_str()));
    } else {
        p.drawText(x, y + QFontMetrics(_font).ascent(), QString::fromUtf8(msg.c_str()));
    }
}

void QtTextDrawingContext::DrawText(const std::string& msg, int x, int y) {
    DrawText(msg, x, y, 0.0);
}

// ── Measurement ──────────────────────────────────────────────────────────────

void QtTextDrawingContext::GetTextExtent(const std::string& msg,
                                         double* width, double* height) {
    QFontMetrics fm(_font);
    QRect r = fm.boundingRect(QString::fromUtf8(msg.c_str()));
    if (width)  *width  = r.width();
    if (height) *height = fm.height();
}

void QtTextDrawingContext::GetTextExtents(const std::string& msg,
                                          std::vector<double>& extents) {
    QFontMetrics fm(_font);
    QString text = QString::fromUtf8(msg.c_str());
    extents.clear();
    extents.reserve(text.size());
    double x = 0.0;
    for (QChar c : text) {
        x += fm.horizontalAdvance(c);
        extents.push_back(x);
    }
}
