#ifndef PLAYBACKOPTIONSDIALOG_H
#define PLAYBACKOPTIONSDIALOG_H

//(*Headers(PlaybackOptionsDialog)
#include <wx/checkbox.h>
#include <wx/dialog.h>
#include <wx/sizer.h>
//*)

class PlaybackOptionsDialog: public wxDialog
{
public:

    PlaybackOptionsDialog(wxWindow* parent,wxWindowID id=wxID_ANY,const wxPoint& pos=wxDefaultPosition,const wxSize& size=wxDefaultSize);
    virtual ~PlaybackOptionsDialog();

    //(*Declarations(PlaybackOptionsDialog)
    wxCheckBox* CheckBoxLastItem;
    wxCheckBox* CheckBoxFirstItem;
    wxCheckBox* CheckBoxRandom;
    wxCheckBox* CheckBoxLightsOff;
    wxCheckBox* CheckBoxRepeat;
    //*)

protected:

    //(*Identifiers(PlaybackOptionsDialog)
    static const long ID_CHECKBOX_REPEAT;
    static const long ID_CHECKBOX_FIRSTITEM;
    static const long ID_CHECKBOX_LASTITEM;
    static const long ID_CHECKBOX_RANDOM;
    static const long ID_CHECKBOX1;
    //*)

private:

    //(*Handlers(PlaybackOptionsDialog)
    //*)

    DECLARE_EVENT_TABLE()
};

#endif
