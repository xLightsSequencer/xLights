#ifndef ROWHEADING_H
#define ROWHEADING_H
#include "wx/window.h"
#include <wx/xml/xml.h>
#include "SequenceElements.h"
#include "wx/wx.h"
#include "Color.h"

#define HORIZONTAL_PADDING          10
#define PIXELS_PER_MAJOR_HASH       100
#define DEFAULT_ROW_HEADING_HEIGHT  RowHeading::rowHeadingSize
#define DEFAULT_ROW_HEADING_MARGIN  16
#define INDENT_ROW_HEADING_MARGIN   24

wxDECLARE_EVENT(EVT_ROW_HEADINGS_CHANGED, wxCommandEvent);
wxDECLARE_EVENT(EVT_SHOW_DISPLAY_ELEMENTS, wxCommandEvent);
class MainSequencer;

class RowHeading : public wxWindow
{
    public:
        //RowHeading(wxScrolledWindow* parent);
        RowHeading(MainSequencer* parent, wxWindowID id, const wxPoint &pos=wxDefaultPosition, const wxSize &size=wxDefaultSize,
                       long style=0, const wxString &name=wxPanelNameStr);
        virtual ~RowHeading();

        void SetSequenceElements(SequenceElements* elements);
        static const xlColor* GetTimingColor(int index);
        int getWidth();
        int getHeight();
        int GetMaxRows();
    
    static int rowHeadingSize;

    protected:
    private:
        DECLARE_EVENT_TABLE()
        void render( wxPaintEvent& event );
        void mouseLeftDown( wxMouseEvent& event);
        void rightClick( wxMouseEvent& event);
        void OnLayerPopup(wxCommandEvent& event);
        void DrawHeading(wxPaintDC* dc, wxXmlNode* model,int width,int row);
        bool HitTestCollapseExpand(int row,int x, bool* IsCollapsed);
        bool HitTestTimingActive(int row,int x, bool* IsActive);
        const xlColor* GetHeaderColor(Row_Information_Struct* info);
        const xlColor * mHeaderColorModel;
        const xlColor * mHeaderColorView;
        const xlColor * mHeaderColorTiming;
        const xlColor * mHeaderSelectedColor;

        int mSelectedRow;
        SequenceElements* mSequenceElements;

        static const long ID_ROW_MNU_ADD_LAYER;
        static const long ID_ROW_MNU_DELETE_LAYER;
        static const long ID_ROW_MNU_LAYER;
        static const long ID_ROW_MNU_PLAY_MODEL;
        static const long ID_ROW_MNU_EXPORT_MODEL;
        static const long ID_ROW_MNU_EDIT_DISPLAY_ELEMENTS;
        static const long ID_ROW_MNU_TOGGLE_STRANDS;
        static const long ID_ROW_MNU_TOGGLE_NODES;
    

        static const long ID_ROW_MNU_ADD_TIMING_TRACK;
        static const long ID_ROW_MNU_DELETE_TIMING_TRACK;
        static const long ID_ROW_MNU_IMPORT_TIMING_TRACK;


};

#endif // ROWHEADING_H
