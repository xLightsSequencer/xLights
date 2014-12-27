#ifndef ROWHEADING_H
#define ROWHEADING_H
#include "wx/window.h"
#include <wx/xml/xml.h>
#include "wx/wx.h"

#define HORIZONTAL_PADDING          10
#define PIXELS_PER_MAJOR_HASH       100
#define DEFAULT_ROW_HEADING_HEIGHT  22
#define DEFAULT_ROW_HEADING_MARGIN  16

class RowHeading : public wxWindow
{
    public:
        RowHeading(wxWindow* parent);
        virtual ~RowHeading();

        void SetCanvasSize(int width,int height);

        void SetElements(wxXmlNode* displayElements);

    protected:
    private:
        DECLARE_EVENT_TABLE()
        void render( wxPaintEvent& event );
        void DrawHeading(wxPaintDC* dc, wxXmlNode* model,int width,int row);
        wxColour* GetHeaderColor(wxString headerType);

        wxColour * mHeaderColorModel;
        wxColour * mHeaderColorView;
        wxColour * mHeaderColorTiming;
        wxXmlNode* mElements;
};

#endif // ROWHEADING_H
