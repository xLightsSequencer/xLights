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

#include "wx/wx.h"
#include <vector>

class SequenceElements;
class Effect;

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
    std::string element_name;
    int layer_index;
    std::string name;
    std::string settings;
    std::string palette;
    int startTimeMS;
    int endTimeMS;
    int Selected;
    bool Protected;
    DeletedEffectInfo( const std::string &element_name_, int layer_index_, const std::string &name_, const std::string &settings_,
                       const std::string &palette_, int startTimeMS_, int endTimeMS_, int Selected_, bool Protected_ );
};

class AddedEffectInfo
{
public:
    std::string element_name;
    int layer_index;
    int id;
    AddedEffectInfo( const std::string &element_name_, int layer_index_, int id_ );
};

class MovedEffectInfo
{
public:
    std::string element_name;
    int layer_index;
    int id;
    int startTimeMS;
    int endTimeMS;
    MovedEffectInfo( const std::string &element_name_, int layer_index_, int id_, int startTimeMS_, int endTimeMS_ );
};

class ModifiedEffectInfo
{
public:
    std::string element_name;
    int layer_index;
    int id;
    std::string settings;
    std::string palette;
    std::string effectName;
    int effectType;
    
    ModifiedEffectInfo( const std::string &element_name_, int layer_index_, int id_, const std::string &settings_, const std::string &palette_ );
    ModifiedEffectInfo( const std::string &element_name_, int layer_index_, Effect *ef);
};

class UndoStep
{
public:
    explicit UndoStep( UNDO_ACTIONS action );
    UndoStep( UNDO_ACTIONS action, DeletedEffectInfo* effect_info );
    UndoStep( UNDO_ACTIONS action, AddedEffectInfo* effect_info );
    UndoStep( UNDO_ACTIONS action, MovedEffectInfo* effect_info );
    UndoStep( UNDO_ACTIONS action, ModifiedEffectInfo* effect_info );

    UNDO_ACTIONS undo_action;
    std::vector<DeletedEffectInfo*> deleted_effect_info;
    std::vector<AddedEffectInfo*> added_effect_info;
    std::vector<MovedEffectInfo*> moved_effect_info;
    std::vector<ModifiedEffectInfo*> modified_effect_info;
};

class UndoManager
{
    public:
        explicit UndoManager(SequenceElements* parent);
        virtual ~UndoManager();
    
        void Clear();
        void ClearRedo();
        void RemoveUnusedMarkers();
        bool ChangeCaptured();
        void UndoLastStep();
        void RedoLastStep();
        void CreateUndoStep();
        bool CanUndo();
        bool CanRedo();
        void SetCaptureUndo( bool value );
        bool GetCaptureUndo() { return mCaptureUndo; }
        std::string GetUndoString();
        std::string GetRedoString();

        void CaptureEffectToBeDeleted( const std::string &element_name, int layer_index, const std::string &name, const std::string &settings,
                                       const std::string &palette, int startTimeMS, int endTimeMS, int Selected, bool Protected );

        void CaptureAddedEffect( const std::string &element_name, int layer_index, int id );

        void CaptureEffectToBeMoved( const std::string &element_name, int layer_index, int id, int startTimeMS, int endTimeMS );
        void CaptureModifiedEffect( const std::string &element_name, int layer_index, int id, const std::string &settings, const std::string &palette );
        void CaptureModifiedEffect( const std::string &element_name, int layer_index, Effect *ef);
    protected:
        void ProcessUndoStep(std::vector<UndoStep*> &fromList, std::vector<UndoStep*> &toList);

    private:
        std::vector<UndoStep*> mUndoSteps;
        std::vector<UndoStep*> mRedoSteps;
        SequenceElements* mParentSequence;
        bool mCaptureUndo;

};
