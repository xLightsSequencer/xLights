#ifndef ROWHEADING_H
#define ROWHEADING_H
#include "wx/window.h"
#include <wx/xml/xml.h>
#include "SequenceElements.h"
#include "wx/wx.h"

#define HORIZONTAL_PADDING          10
#define PIXELS_PER_MAJOR_HASH       100
#define DEFAULT_ROW_HEADING_HEIGHT  22
#define DEFAULT_ROW_HEADING_MARGIN  16
#define INDENT_ROW_HEADING_MARGIN   24

wxDECLARE_EVENT(EVT_ROW_HEADINGS_CHANGED, wxCommandEvent);

class RowHeading : public wxWindow
{
    public:
        RowHeading(wxWindow* parent);
        virtual ~RowHeading();

        void SetCanvasSize(int width,int height);

        void SetSequenceElements(SequenceElements* elements);

    protected:
    private:
        DECLARE_EVENT_TABLE()
        void render( wxPaintEvent& event );
        void mouseLeftDown( wxMouseEvent& event);
        void DrawHeading(wxPaintDC* dc, wxXmlNode* model,int width,int row);
        bool HitTestCollapseExpand(int row,int x, bool* IsCollapsed);
        wxColour* GetHeaderColor(Row_Information_Struct* info);
        wxColour * mHeaderColorModel;
        wxColour * mHeaderColorView;
        wxColour * mHeaderColorTiming;
        SequenceElements* mSequenceElements;

};

#endif // ROWHEADING_H
