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
        //RowHeading(wxScrolledWindow* parent);
        RowHeading(wxScrolledWindow* parent, wxWindowID id, const wxPoint &pos=wxDefaultPosition, const wxSize &size=wxDefaultSize,
                       long style=0, const wxString &name=wxPanelNameStr);
        virtual ~RowHeading();

        void SetCanvasSize(int width,int height);

        void SetSequenceElements(SequenceElements* elements);
        static const wxColour* GetTimingColor(int index);
        int getWidth();
        int getHeight();

    protected:
    private:
        DECLARE_EVENT_TABLE()
        void render( wxPaintEvent& event );
        void mouseLeftDown( wxMouseEvent& event);
        void DrawHeading(wxPaintDC* dc, wxXmlNode* model,int width,int row);
        bool HitTestCollapseExpand(int row,int x, bool* IsCollapsed);
        bool HitTestTimingActive(int row,int x, bool* IsActive);
        const wxColour* GetHeaderColor(Row_Information_Struct* info);
        const wxColour * mHeaderColorModel;
        const wxColour * mHeaderColorView;
        const wxColour * mHeaderColorTiming;

        SequenceElements* mSequenceElements;

};

#endif // ROWHEADING_H
