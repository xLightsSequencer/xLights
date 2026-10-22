#ifndef FPPUPLOADPROGRESSDIALOG_H
#define FPPUPLOADPROGRESSDIALOG_H

//(*Headers(FPPUploadProgressDialog)
#include <wx/button.h>
#include <wx/dialog.h>
#include <wx/scrolwin.h>
#include <wx/sizer.h>
#include <wx/stattext.h>
//*)

#include <wx/stopwatch.h>
#include <wx/timer.h>

#include <string>
#include <vector>

class wxGauge;
class wxTreeCtrl;

class FPPUploadProgressDialog: public wxDialog
{
    public:

        FPPUploadProgressDialog(wxWindow* parent,wxWindowID id=wxID_ANY);
        virtual ~FPPUploadProgressDialog();

        //(*Declarations(FPPUploadProgressDialog)
        wxButton* CancelButton;
        wxFlexGridSizer* scrolledWindowSizer;
        wxScrolledWindow* scrolledWindow;
        wxStaticText* ActionLabel;
        //*)

        enum class SequenceState { Uploaded, UpToDate, Failed, Cancelled };
        struct ControllerResult {
            std::string name;
            std::string status;
            bool failed = false;
            std::vector<std::string> details;
        };

        bool isCancelled() const {return cancelled;}
        void setActionLabel(const std::string &action);
        wxGauge *addGauge(const std::string &name);
        void RequestCancel();
        // Call before ShowModal; indexes below follow the order of names.
        void setSequences(const std::vector<std::string>& names);
        void startSequence(size_t index);
        // A non-sequence step (checking models, sending settings) shown in the header,
        // then added to the completed list when done.
        void startStep(const std::string& title);
        void finishStep(const std::string& title, const std::string& status, bool warning, const std::vector<ControllerResult>& controllers);
        // Moves the sequence to the completed list with one child row per controller.
        void finishSequence(size_t index, SequenceState state, const std::string& detail, const std::vector<ControllerResult>& controllers);
        // Upload is over: show the summary and turn Cancel into Close, which
        // ends the dialog with returnCode.
        void finish(bool wasCancelled, int returnCode);
    protected:

        //(*Identifiers(FPPUploadProgressDialog)
        static const long ID_STATICTEXT1;
        static const long ID_SCROLLEDWINDOW1;
        static const long ID_BUTTON1;
        //*)

    private:
    bool cancelled = false;
    void updateOverall();
    void addCompleted(const wxString& text, const wxColour& colour, const std::vector<ControllerResult>& controllers, bool expand);
    void OnClose(wxCloseEvent& event);

    wxStaticText* _sequenceHeader = nullptr;
    wxStaticText* _completedLabel = nullptr;
    wxTreeCtrl* _completedTree = nullptr;
    wxStaticText* _overallLabel = nullptr;
    std::vector<std::string> _sequenceNames;
    std::vector<bool> _sequenceDone;
    wxStopWatch _totalTime;
    wxTimer _elapsedTimer;
    wxStopWatch _sequenceTime;
    long _firstSequenceStartMs = -1;
    int _uploadedCount = 0;
    int _upToDateCount = 0;
    int _failedCount = 0;
    int _cancelledCount = 0;
    int _completedCount = 0;
    bool _finished = false;
    int _returnCode = 0;
    
        //(*Handlers(FPPUploadProgressDialog)
        void OnCancelButtonClick(wxCommandEvent& event);
        //*)

        DECLARE_EVENT_TABLE()
};

#endif
