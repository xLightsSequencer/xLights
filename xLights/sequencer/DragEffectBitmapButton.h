#ifndef DRAGEFFECTBITMAPBUTTON_H
#define DRAGEFFECTBITMAPBUTTON_H
#include "wx/wx.h"

class DragEffectBitmapButton : public wxBitmapButton
{
    public:
//        DragEffectBitmapButton();

        DragEffectBitmapButton (wxWindow *parent, wxWindowID id, const wxBitmap &bitmap, const wxPoint &pos=wxDefaultPosition,
                                const wxSize &size=wxDefaultSize, long style=wxBU_AUTODRAW, const wxValidator &validator=wxDefaultValidator,
                                const wxString &name=wxButtonNameStr);

        virtual ~DragEffectBitmapButton();
        void OnMouseLeftDown (wxMouseEvent& event);
        void SetBitmap(const char** xpm);
    protected:
    private:
        const char** mDragIconBuffer;

};

#endif // DRAGEFFECTBITMAPBUTTON_H
