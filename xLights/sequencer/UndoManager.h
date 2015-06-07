#ifndef UNDOMANAGER_H
#define UNDOMANAGER_H

#include "wx/wx.h"
#include <vector>

class SequenceElements;

enum UNDO_ACTIONS
{
    UNDO_MARKER,   // use this to mark the beginning of a group of actions that should be undone together
    UNDO_EFFECT_ADDED,
    UNDO_EFFECT_DELETED,
    UNDO_EFFECT_MODIFIED,
    UNDO_EFFECT_MOVED
};

class DeletedEffectInfo
{
public:
    wxString element_name;
    int layer_index;
    wxString name;
    wxString settings;
    wxString palette;
    double startTime;
    double endTime;
    int Selected;
    bool Protected;
    DeletedEffectInfo( const wxString &element_name_, int layer_index_, const wxString &name_, const wxString &settings_,
                       const wxString &palette_, double &startTime_, double &endTime_, int Selected_, bool Protected_ );
};

class AddedEffectInfo
{
public:
    wxString element_name;
    int layer_index;
    int id;
    AddedEffectInfo( const wxString &element_name_, int layer_index_, int id_ );
};

class MovedEffectInfo
{
public:
    wxString element_name;
    int layer_index;
    int id;
    double startTime;
    double endTime;
    MovedEffectInfo( const wxString &element_name_, int layer_index_, int id_, double &startTime_, double &endTime_ );
};

class UndoStep
{
public:
    explicit UndoStep( UNDO_ACTIONS action );
    UndoStep( UNDO_ACTIONS action, DeletedEffectInfo* effect_info );
    UndoStep( UNDO_ACTIONS action, AddedEffectInfo* effect_info );
    UndoStep( UNDO_ACTIONS action, MovedEffectInfo* effect_info );

    UNDO_ACTIONS undo_action;
    std::vector<DeletedEffectInfo*> deleted_effect_info;
    std::vector<AddedEffectInfo*> added_effect_info;
    std::vector<MovedEffectInfo*> moved_effect_info;
};

class UndoManager
{
    public:
        explicit UndoManager(SequenceElements* parent);
        virtual ~UndoManager();

        void RemoveUnusedMarkers();
        void UndoLastStep();
        void CreateUndoStep();
        bool CanUndo();
        void SetCaptureUndo( bool value );
        bool GetCaptureUndo() { return mCaptureUndo; }

        void CaptureEffectToBeDeleted( const wxString &element_name, int layer_index, const wxString &name, const wxString &settings,
                                       const wxString &palette, double startTime, double endTime, int Selected, bool Protected );

        void CaptureAddedEffect( const wxString &element_name, int layer_index, int id );

        void CaptureEffectToBeMoved( const wxString &element_name, int layer_index, int id, double startTime, double endTime );
    protected:

    private:
        std::vector<UndoStep*> mUndoSteps;
        SequenceElements* mParentSequence;
        bool mCaptureUndo;

};

#endif // UNDOMANAGER_H
