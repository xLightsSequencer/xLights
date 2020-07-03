#pragma once

/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/smeighan/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/smeighan/xLights/blob/master/License.txt
 **************************************************************/

//(*Headers(PaletteMgmtDialog)
#include <wx/dialog.h>
#include <wx/sizer.h>
#include <wx/button.h>
#include <wx/stattext.h>
#include <wx/listbox.h>
//*)

#include <wx/xml/xml.h>
#include <wx/msgdlg.h>
#include <wx/textdlg.h>
#include "EffectsPanel.h"

class PaletteMgmtDialog: public wxDialog
{
public:

    PaletteMgmtDialog(wxWindow* parent,wxWindowID id=wxID_ANY,const wxPoint& pos=wxDefaultPosition,const wxSize& size=wxDefaultSize);
    virtual ~PaletteMgmtDialog();
    void initialize(wxXmlNode* PalNode, EffectsPanel* p1,EffectsPanel* p2);


    //(*Declarations(PaletteMgmtDialog)
    wxStaticText* StaticText14;
    wxListBox* ListBox1;
    wxButton* ButtonSavePalette1;
    wxButton* ButtonDelete;
    wxStaticText* StaticText1;
    wxButton* ButtonSavePalette2;
    wxButton* ButtonLoadPalette1;
    wxButton* ButtonCopy12;
    wxButton* ButtonLoadPalette2;
    //*)

protected:

    //(*Identifiers(PaletteMgmtDialog)
    static const long ID_STATICTEXT14;
    static const long ID_STATICTEXT1;
    static const long ID_LISTBOX1;
    static const long ID_BUTTON5;
    static const long ID_BUTTON3;
    static const long ID_BUTTON4;
    static const long ID_BUTTON1;
    static const long ID_BUTTON2;
    static const long ID_BUTTON6;
    //*)

private:

    //(*Handlers(PaletteMgmtDialog)
    void OnButtonCopyClick(wxCommandEvent& event);
    void OnButtonDeleteClick(wxCommandEvent& event);
    void OnButtonLoadPalette1Click(wxCommandEvent& event);
    void OnButtonLoadPalette2Click(wxCommandEvent& event);
    void OnButtonSavePalette1Click(wxCommandEvent& event);
    void OnButtonSavePalette2Click(wxCommandEvent& event);
    //*)

    void ReloadPaletteList();
    void LoadPalette(wxXmlNode* PaletteNode, EffectsPanel* panel);
    void SavePalette(EffectsPanel* panel);
    int GetSelectedIndex();

    EffectsPanel* panel1;
    EffectsPanel* panel2;
    wxXmlNode* PalettesNode;

    DECLARE_EVENT_TABLE()
};

