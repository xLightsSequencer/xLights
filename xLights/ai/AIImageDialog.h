#ifndef HEADER_85D1CC366A7F7DAE
#define HEADER_85D1CC366A7F7DAE

#pragma once

class ServiceManager;
class CropPanel;

//(*Headers(AIImageDialog)
#include <wx/button.h>
#include <wx/dialog.h>
#include <wx/panel.h>
#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/textctrl.h>
//*)

#include "ai/aiBase.h"

class AIImageDialog: public wxDialog
{
    public:

        AIImageDialog(wxWindow* parent, aiBase* service, wxWindowID id = wxID_ANY);
        virtual ~AIImageDialog();


        //(*Declarations(AIImageDialog)
        CropPanel* cropPanel;
        wxButton* CloseButton;
        wxButton* CropButton;
        wxButton* GenerateButton;
        wxButton* ResetButton;
        wxButton* ResizeButton;
        wxButton* SaveButton;
        wxFlexGridSizer* MainSizer;
        wxFlexGridSizer* ParametersSizer;
        wxFlexGridSizer* ResultBoxSizer;
        wxPanel* ImagePanel;
        wxStaticBoxSizer* ParametersBox;
        wxStaticBoxSizer* ResultsBox;
        wxStaticText* SizeLabel;
        wxStaticText* StaticText1;
        wxTextCtrl* ErrorText;
        wxTextCtrl* PromptBox;
        //*)


    protected:

        //(*Identifiers(AIImageDialog)
        static const wxWindowID ID_STATICTEXT1;
        static const wxWindowID ID_PROMPT;
        static const wxWindowID ID_PANEL2;
        static const wxWindowID ID_STATICTEXT4;
        static const wxWindowID ID_RESIZE;
        static const wxWindowID ID_CROP;
        static const wxWindowID ID_RESET;
        static const wxWindowID ID_PANEL1;
        static const wxWindowID ID_STATICTEXT2;
        static const wxWindowID ID_GENERATE;
        static const wxWindowID ID_BUTTON1;
        static const wxWindowID ID_OK;
        //*)

    private:

        //(*Handlers(AIImageDialog)
        void OnGenerateButtonClick(wxCommandEvent& event);
        void OnResize(wxSizeEvent& event);
        void OnResizeButtonClick(wxCommandEvent& event);
        void OnCropButtonClick(wxCommandEvent& event);
        void OnResetButtonClick(wxCommandEvent& event);
        void OnSaveButtonClicked(wxCommandEvent& event);
        void OnCloseButtonClick(wxCommandEvent& event);
        //*)

        DECLARE_EVENT_TABLE()

        aiBase::AIImageGenerator *generator = nullptr;
        wxImage _originalImage;
        wxImage _currentImage;
        
        void UpdateSizeLabel();
        void ShowInteractiveCrop();
};

// Helper class for interactive crop selection
class CropPanel : public wxPanel
{
public:
    CropPanel(wxWindow* parent, wxWindowID id = wxID_ANY,
              const wxPoint& pos = wxDefaultPosition,
              const wxSize& size = wxDefaultSize,
              long style = wxTAB_TRAVERSAL | wxNO_BORDER,
              const wxString& name = wxASCII_STR(wxPanelNameStr));
    
    void SetImage(const wxImage& image);
    bool GetCropRect(wxRect& rect) const;
    bool HasSelection() const { return _hasSelection; }
    void ClearSelection();
    
    std::function<void(bool)> OnSelectionChanged;
    
private:
    void OnPaint(wxPaintEvent& event);
    void OnLeftDown(wxMouseEvent& event);
    void OnLeftUp(wxMouseEvent& event);
    void OnMouseMove(wxMouseEvent& event);
    
    wxImage _image;
    wxBitmap _bitmap;
    wxPoint _startPoint;
    wxPoint _endPoint;
    bool _isDragging;
    bool _hasSelection;
    double _scaleFactor;
    wxPoint _offset;
    
    wxDECLARE_EVENT_TABLE();
};
#endif // header guard 

