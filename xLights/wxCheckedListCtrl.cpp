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
   wxListCtrl(parent, id, pt, sz, style), m_imageList(16, 16)
   {
   SetImageList(&m_imageList, wxIMAGE_LIST_SMALL);

   //InsertColumn(0, _("Item"), wxLIST_FORMAT_LEFT, 200);
   //InsertColumn(1, _("Value"), wxLIST_FORMAT_LEFT, 80);
   }

wxCheckedListCtrl::~wxCheckedListCtrl()
{
    //dtor
}

void wxCheckedListCtrl::OnMouseEvent(wxMouseEvent& event)
{
  if (event.LeftDown())
  {
     int flags;
     long item = HitTest(event.GetPosition(), flags);
     if (item > -1 && (flags & wxLIST_HITTEST_ONITEMICON))
     {
         SetChecked(item, !IsChecked(item));
        wxCommandEvent eventChecked(EVT_LISTITEM_CHECKED);
        eventChecked.SetClientData((wxClientData*)GetItemData(item));
        wxPostEvent(GetParent(), eventChecked);
     }
     else
        event.Skip();
  }
  else
  {
     event.Skip();
  }
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
