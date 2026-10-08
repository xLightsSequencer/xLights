#ifndef WXCHECKEDLISTCTRL_H
#define WXCHECKEDLISTCTRL_H

#include <wx/listctrl.h>
#include <wx/imaglist.h>
#include <wx/icon.h>

wxDECLARE_EVENT(EVT_LISTITEM_CHECKED, wxCommandEvent);

class wxCheckedListCtrl : public wxListCtrl
{
    public:
        wxCheckedListCtrl();
        wxCheckedListCtrl(wxWindow* parent, wxWindowID id, const wxPoint& pt,
                          const wxSize& sz, long style,const wxValidator &validator,
                          const wxString &name);
        virtual ~wxCheckedListCtrl();

        void OnMouseEvent(wxMouseEvent& event);
        bool IsChecked(long item) const;
        void SetChecked(long item, bool checked);
        void SetImages( char** ImageChecked,char** ImageUnchecked);
        void AddImage( char** ImageXPM);
        // Strike through the text of unchecked rows, for lists where unchecked
        // means hidden.
        void SetStrikeUnchecked(bool strike) { m_strikeUnchecked = strike; }

    private:
        wxImageList m_imageList;
        bool m_strikeUnchecked = false;
        char** mImageChecked;
        char** mImageUnchecked;

   		DECLARE_EVENT_TABLE()

};

#endif // WXCHECKEDLISTCTRL_H
