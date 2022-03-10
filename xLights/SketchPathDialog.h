#pragma once

#include <wx/dialog.h>

class SketchPathDialog: public wxDialog
{
public:
	SketchPathDialog(wxWindow* parent,wxWindowID id=wxID_ANY,const wxPoint& pos=wxDefaultPosition,const wxSize& size=wxDefaultSize);
    virtual ~SketchPathDialog() = default;

private:
	DECLARE_EVENT_TABLE()

    void OnButton_Ok(wxCommandEvent& event);
    void OnButton_Cancel(wxCommandEvent& event);
};
