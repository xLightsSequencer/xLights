#ifndef MODELLISTDIALOG_H
#define MODELLISTDIALOG_H

#include <map>

//(*Headers(ModelListDialog)
#include <wx/sizer.h>
#include <wx/listbox.h>
#include <wx/button.h>
#include <wx/dialog.h>
//*)

#include <wx/html/htmprint.h>
#include "ModelClass.h"
#include "sequencer/SequenceElements.h"

class xLightsFrame;

class ModelListDialog: public wxDialog
{
public:

    ModelListDialog(wxWindow* parent,wxWindowID id=wxID_ANY,const wxPoint& pos=wxDefaultPosition,const wxSize& size=wxDefaultSize);
    virtual ~ModelListDialog();

    wxHtmlEasyPrinting* HtmlEasyPrint;
    void SetSequenceElements(SequenceElements* elements);
    void SetNetInfo(NetInfoClass *ni) {netInfo = ni;};
    void SetModelGroupsNode(wxXmlNode *mg) {modelGroups = mg;};
    void AddModel(const wxString &name, wxXmlNode *nd);
    wxXmlNode *GetXMLForModel(const wxString &name);
    
    //(*Declarations(ModelListDialog)
    wxButton* Button_ExportCsv;
    wxButton* Button_Modify;
    wxButton* Button_Layout;
    wxButton* Button_Delete;
    wxButton* Button_New;
    wxButton* Button_Copy;
    wxButton* Button_Rename;
    wxListBox* ListBox1;
    //*)

protected:

    //(*Identifiers(ModelListDialog)
    static const long ID_LISTBOX1;
    static const long ID_BUTTON1;
    static const long ID_BUTTON3;
    static const long ID_BUTTON4;
    static const long ID_BUTTON2;
    static const long ID_BUTTON5;
    static const long ID_BUTTON_LAYOUT;
    static const long ID_BUTTON_ExportCsv;
    //*)

private:

    //(*Handlers(ModelListDialog)
    void OnButton_NewClick(wxCommandEvent& event);
    void OnButton_ModifyClick(wxCommandEvent& event);
    void OnButton_DeleteClick(wxCommandEvent& event);
    void OnButton_RenameClick(wxCommandEvent& event);
    void OnButton_LayoutClick(wxCommandEvent& event);
    void OnButton_CopyClick(wxCommandEvent& event);
    void OnButton_ExportCsvClick(wxCommandEvent& event);
    void OnListBox_Modellist_ModelsSelect(wxCommandEvent& event);
    void OnListBox_ListBox1(wxCommandEvent& event);
    //*)

    bool ValidateModelName(const wxString& name);
    SequenceElements* mSequenceElements;
    xLightsFrame* mParent;
    NetInfoClass *netInfo;
    wxXmlNode *modelGroups;
    std::map<wxString, wxXmlNode*> models;

    DECLARE_EVENT_TABLE()
};

#endif
