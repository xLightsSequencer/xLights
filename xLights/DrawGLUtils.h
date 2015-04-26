#include "Color.h"

namespace DrawGLUtils
{
    void DrawPoint(const xlColor &color, double x, double y);

    void DrawCircle(const xlColor &color, double x, double y, double r, int ctransparency = 0, int etransparency = 0);

    void StartPoints(const xlColor &color, int transparency = 0);
    void AddPoint(wxDouble x, wxDouble y);
    void EndPoints();
    
    void DrawLine(const xlColor &color, wxByte alpha,int x1, int y1,int x2, int y2,float width);
    void DrawRectangle(const xlColor &color, bool dashed, int x1, int y1,int x2, int y2);
    void DrawFillRectangle(const xlColor &color, wxByte alpha, int x, int y,int width, int height);
    
    void DrawHBlendedRectangle(const xlColor &lcolor, const xlColor &rcolor, int x, int y, int x2, int y2);
    void DrawHBlendedRectangle(const xlColorVector &colors, int x, int y, int x2, int y2);
    void CreateOrUpdateTexture(const wxBitmap &bmp48,    //will scale to 64x64 for base
                               const wxBitmap &bmp32,
                               const wxBitmap &bmp16,
                               GLuint* texture);
    
    void DrawRectangleArray(double y1, double y2, double x, std::vector<double> &xs, std::vector<xlColor> & colors);
}

