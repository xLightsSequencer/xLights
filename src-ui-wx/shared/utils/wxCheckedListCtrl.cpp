#include "wxCheckedListCtrl.h"

wxDEFINE_EVENT(EVT_LISTITEM_CHECKED, wxCommandEvent);


BEGIN_EVENT_TABLE(wxCheckedListCtrl, wxListCtrl)
  EVT_LEFT_DOWN(wxCheckedListCtrl::OnMouseEvent)
END_EVENT_TABLE()

wxCheckedListCtrl::wxCheckedListCtrl()
{
    //ctor
}

wxCheckedListCtrl::wxCheckedListCtrl(wxWindow* parent, wxWindowID id, const wxPoint& pt,
   const wxSize& sz, long style,const wxValidator &validator=wxDefaultValidator, const wxString &name=wxListCtrlNameStr):
   wxListCtrl(parent, id, pt, sz, style), m_imageList(16, 16) {

   //InsertColumn(0, _("Item"), wxLIST_FORMAT_LEFT, 200);
   //InsertColumn(1, _("Value"), wxLIST_FORMAT_LEFT, 80);
}

wxCheckedListCtrl::~wxCheckedListCtrl()
{
    //dtor
}

void wxCheckedListCtrl::OnMouseEvent(wxMouseEvent& event)
{
    if (!event.LeftDown()) {
        event.Skip();
        return;
    }

    int flags = 0;
    long item = HitTest(event.GetPosition(), flags);

    bool iconHit = (item > -1) && (flags & wxLIST_HITTEST_ONITEMICON);

    if (!iconHit && item > -1 && (flags & wxLIST_HITTEST_ONITEM)) {
        const int col0Width = GetColumnWidth(0);
        const int x = event.GetX();
        const int hScrollPos = GetScrollPos(wxHORIZONTAL);
        if (hScrollPos == 0 && col0Width > 0 && x >= 0 && x < col0Width) {
            iconHit = true;
        }
    }

    if (!iconHit) {
        event.Skip();
        return;
    }

    SetChecked(item, !IsChecked(item));
    wxCommandEvent eventChecked(EVT_LISTITEM_CHECKED);
    eventChecked.SetClientData((wxClientData*)GetItemData(item));
    eventChecked.SetExtraLong(item);
    wxPostEvent(GetParent(), eventChecked);
}

void wxCheckedListCtrl::SetImages( char** ImageCheckedXPM,char** ImageUncheckedXPM)
{
   mImageChecked = ImageCheckedXPM;
   mImageUnchecked = ImageUncheckedXPM;

   wxImage imageChecked(ImageCheckedXPM);
   wxBitmap bitmapChecked(imageChecked);

   wxImage imageUnchecked(ImageUncheckedXPM);
   wxBitmap bitmapUnchecked(imageUnchecked);

   m_imageList.Add(bitmapUnchecked);
   m_imageList.Add(bitmapChecked);
    
    SetImageList(&m_imageList, wxIMAGE_LIST_SMALL);
}

void wxCheckedListCtrl::AddImage( char** ImageXPM)
{
   wxImage imageNew(ImageXPM);
   wxBitmap bitmapNew(imageNew);
   m_imageList.Add(bitmapNew);
    
    SetImageList(&m_imageList, wxIMAGE_LIST_SMALL);
}

bool wxCheckedListCtrl::IsChecked(long item) const
{
   wxListItem info;
   info.m_mask = wxLIST_MASK_IMAGE ;
   info.m_itemId = item;

   if (GetItem(info))
   {
      return (info.m_image == 1);
   }
   else
      return FALSE;
}

void wxCheckedListCtrl::SetChecked(long item, bool checked)
{
   SetItemImage(item, (checked ? 1 : 0), -1);
}
