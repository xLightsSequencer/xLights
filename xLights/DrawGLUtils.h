
#ifndef __XL_DRAWGLUTILS
#define __XL_DRAWGLUTILS

#include <wx/thread.h>
#include "wx/glcanvas.h"
#include "Color.h"

namespace DrawGLUtils
{
    class DisplayListItem {
    public:
        DisplayListItem() : valid(true), usage(GL_POINT), x(0.0), y(0.0) {};
        bool valid;
        int usage;  //POINT, TRIANGLE, QUAD, ETC...
        xlColor color;
        double x, y;
    };
    class xlDisplayList : public std::vector<DisplayListItem> {
    public:
        xlDisplayList() : iconSize(2) {};
        int iconSize;
        mutable wxMutex lock;
    };

    void DrawPoint(const xlColor &color, double x, double y);

    void DrawCircle(const xlColor &color, double x, double y, double r, int ctransparency = 0, int etransparency = 0);

    /* Methods to hold vertex informaton (x, y, color) in an array until End is called where they are all
       draw out to the context in very few calls) */
    void AddVertex(wxDouble x, wxDouble y, const xlColor &c, int transparency = 0);
    void End(int type, bool reset = true);

    void DrawLine(const xlColor &color, wxByte alpha,int x1, int y1,int x2, int y2,float width);
    void DrawRectangle(const xlColor &color, bool dashed, int x1, int y1,int x2, int y2);
    void DrawFillRectangle(const xlColor &color, wxByte alpha, int x, int y,int width, int height);

    void DrawHBlendedRectangle(const xlColor &lcolor, const xlColor &rcolor, int x, int y, int x2, int y2);
    void DrawHBlendedRectangle(const xlColorVector &colors, int x, int y, int x2, int y2, int offset = 0);
    void CreateOrUpdateTexture(const wxBitmap &bmp48,    //will scale to 64x64 for base
                               const wxBitmap &bmp32,
                               const wxBitmap &bmp16,
                               GLuint* texture);
    void DrawTexture(GLuint* texture,double x, double y, double x2, double y2);

    void DrawRectangleArray(double y1, double y2, double x, std::vector<double> &xs, std::vector<xlColor> & colors, bool flush = true);

    void DrawDisplayList(double xOffset, double yOffset,
                         double width, double height,
                         const xlDisplayList & dl);
}

#endif
