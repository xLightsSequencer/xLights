/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/smeighan/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/smeighan/xLights/blob/master/License.txt
 **************************************************************/

#include <algorithm>
#include <vector>

#include "EffectLayer.h"
#include "EffectsGrid.h"
#include "Effect.h"
#include "RowHeading.h"
#include "../models/Model.h"
#include "../effects/EffectManager.h"
#include "../effects/RenderableEffect.h"
#include "Element.h"
#include "../xLightsMain.h"
#include "../xLightsApp.h"

#include <log4cpp/Category.hh>
#include "effects/DMXEffect.h"

std::atomic_int EffectLayer::exclusive_index(0);
const std::string NamedLayer::NO_NAME("");

EffectLayer::EffectLayer(Element* parent)
{
    mParentElement = parent;
    mIndex = exclusive_index++;
}

EffectLayer::~EffectLayer()
{
    std::unique_lock<std::recursive_mutex> locker(lock);
    for (int x = 0; x < mEffects.size(); x++) {
        delete mEffects[x];
    }
    while (!mEffectsToDelete.empty()) {
        delete *mEffectsToDelete.begin();
        mEffectsToDelete.pop_front();
    }
}

void EffectLayer::CleanupAfterRender() {
    std::unique_lock<std::recursive_mutex> locker(lock);
    auto it = mEffectsToDelete.begin();
    while (it != mEffectsToDelete.end())
    {
        if ((*it)->IsTimeToDelete())
        {
            delete *it;
            it = mEffectsToDelete.erase(it);
        }
        else
        {
            ++it;
        }
    }
}

int EffectLayer::GetIndex() const
{
    return mIndex;
}

Effect* EffectLayer::GetEffect(int index) const
{
    if(index < mEffects.size())
    {
        return mEffects[index];
    }
    else
    {
        return nullptr;
    }
}

Effect* EffectLayer::GetEffectByTime(int timeMS) {
    std::unique_lock<std::recursive_mutex> locker(lock);
    for(const auto& it : mEffects) {
        if (timeMS >= it->GetStartTimeMS() &&
            timeMS <= it->GetEndTimeMS()) {
            return it;
        }
    }
    return nullptr;
}


Effect* EffectLayer::GetEffectFromID(int id)
{
    Effect* eff = nullptr;
    for (int x = 0; x < mEffects.size(); x++) {
        if( mEffects[x]->GetID() == id )
        {
            eff = mEffects[x];
            break;
        }
    }
    return eff;
}

int EffectLayer::GetFirstSelectedEffectStartMS() const
{
    for (int x = 0; x < mEffects.size(); x++) {
        if (mEffects[x]->GetSelected() != EFFECT_NOT_SELECTED)
        {
            return mEffects[x]->GetStartTimeMS();
        }
    }
    return -1;
}

int EffectLayer::GetLastSelectedEffectEndMS() const
{
    for (int x = mEffects.size() - 1; x >= 0; x--) {
        if (mEffects[x]->GetSelected() != EFFECT_NOT_SELECTED)
        {
            return mEffects[x]->GetEndTimeMS();
        }
    }
    return -1;
}

void EffectLayer::RemoveEffect(int index)
{
    std::unique_lock<std::recursive_mutex> locker(lock);
    if(index<mEffects.size())
    {
        Effect *e = mEffects[index];
        if (!e->IsLocked())
        {
            mEffects.erase(mEffects.begin() + index);
            IncrementChangeCount(e->GetStartTimeMS(), e->GetEndTimeMS());
            e->SetTimeToDelete();
            mEffectsToDelete.push_back(e);
            NumberEffects();
        }
    }
}

void EffectLayer::DeleteEffect(int id)
{
    std::unique_lock<std::recursive_mutex> locker(lock);
    for (int i = 0; i<mEffects.size(); i++)
    {
        if (mEffects[i]->GetID() == id)
        {
            IncrementChangeCount(mEffects[i]->GetStartTimeMS(), mEffects[i]->GetEndTimeMS());
            mEffects[i]->SetTimeToDelete();
            mEffectsToDelete.push_back(mEffects[i]);
            mEffects.erase(mEffects.begin() + i);
            NumberEffects();
            return;
        }
    }
    wxASSERT(false);
}

void EffectLayer::RemoveAllEffects(UndoManager *undo_mgr)
{
    std::unique_lock<std::recursive_mutex> locker(lock);
    std::vector<Effect*> newEffects;
    for (int x = 0; x < mEffects.size(); x++) {
        if (!mEffects[x]->IsLocked()) {
            IncrementChangeCount(mEffects[x]->GetStartTimeMS(), mEffects[x]->GetEndTimeMS());
            if (undo_mgr) {
                undo_mgr->CaptureEffectToBeDeleted(mParentElement->GetModelName(), mIndex, mEffects[x]->GetEffectName(),
                    mEffects[x]->GetSettingsAsString(), mEffects[x]->GetPaletteAsString(),
                    mEffects[x]->GetStartTimeMS(), mEffects[x]->GetEndTimeMS(),
                    mEffects[x]->GetSelected(), mEffects[x]->GetProtected());
            }
            mEffects[x]->SetTimeToDelete();
            mEffectsToDelete.push_back(mEffects[x]);
        }
        else             {
            newEffects.push_back(mEffects[x]);
        }
    }

    mEffects = newEffects;

    // renumber the remaining effects
    NumberEffects();
}

Effect* EffectLayer::AddEffect(int id, const std::string &n, const std::string &settings, const std::string &palette,
                               int startTimeMS, int endTimeMS, int Selected, bool Protected, bool suppress_sort, bool importing)
{
    std::unique_lock<std::recursive_mutex> locker(lock);
    std::string name(n);
    EffectManager* em = nullptr;
    if (GetParentElement() != nullptr)
    {
        em = &(GetParentElement()->GetSequenceElements()->GetEffectManager());
    } else {
        em = &(xLightsApp::GetFrame()->GetEffectManager());
    }
    wxASSERT(em != nullptr);

    // really dont want to add effects which look invalid - some imports result in this
    if (startTimeMS > endTimeMS) return nullptr;

    if (GetParentElement() != nullptr && GetParentElement()->GetType() == ElementType::ELEMENT_TYPE_MODEL) {
        if (name == "") {
            name = "Off";
        }
        if ((em->GetEffectIndex(name) == -1) &&
            (name != "Random"))
        {
            log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));
            logger_base.warn("Unknown effect: " + name + ". Not loaded. " + GetParentElement()->GetModelName());
            return nullptr;
        }
    }

    // KW - I am putting this here because in the past we have forgotten to prevent this and it has caused overlapping effects
    //      with this here debug runs a bit slower but any overlap will ASSERT but it wont impact release build
    //wxASSERT(!HasEffectsInTimeRange(startTimeMS, endTimeMS));

    if (startTimeMS < 0 && endTimeMS <= 0)
    {
        // This effect is not visible ... so lets not load it
        return nullptr;
    }

    // make sure they dont hang over the left side
    if (startTimeMS < 0) startTimeMS = 0;

    Effect* e = new Effect(em, this, id, name, settings, palette, startTimeMS, endTimeMS, Selected, Protected, importing);
    wxASSERT(e != nullptr);
    mEffects.push_back(e);
    if (!suppress_sort)
    {
        SortEffects();
    }
    IncrementChangeCount(startTimeMS, endTimeMS);
    return e;
}

void EffectLayer::NumberEffects()
{
    for (int x = 0; x < mEffects.size(); x++) {
        mEffects[x]->SetID(x);
    }
}

void EffectLayer::SortEffects()
{
    std::sort(mEffects.begin(), mEffects.end(), SortEffectByStartTime);
    NumberEffects();
}

bool EffectLayer::IsStartTimeLinked(int index) const
{
    if (index < mEffects.size() && index > 0) {
        return mEffects[index - 1]->GetEndTimeMS() == mEffects[index]->GetStartTimeMS();
    } else {
        return false;
    }
}

bool EffectLayer::IsEndTimeLinked(int index) const
{
    if (index < mEffects.size() - 1) {
        return mEffects[index]->GetEndTimeMS() == mEffects[index + 1]->GetStartTimeMS();
    } else {
        return false;
    }
}

int EffectLayer::GetMaximumEndTimeMS(int index, bool allow_collapse, int min_period) const
{
    if (index + 1 >= mEffects.size()) {
        return NO_MIN_MAX_TIME;
    } else {
        if (mEffects[index]->GetEndTimeMS() == mEffects[index + 1]->GetStartTimeMS() && allow_collapse) {
            return mEffects[index + 1]->GetEndTimeMS() - min_period;
        } else {
            return mEffects[index + 1]->GetStartTimeMS();
        }
    }
}

int EffectLayer::GetMinimumStartTimeMS(int index, bool allow_collapse, int min_period) const
{
    if (index == 0) {
        return NO_MIN_MAX_TIME;
    } else {
        if (mEffects[index - 1]->GetEndTimeMS() == mEffects[index]->GetStartTimeMS() && allow_collapse) {
            return mEffects[index - 1]->GetStartTimeMS() + min_period;
        } else {
            return mEffects[index - 1]->GetEndTimeMS();
        }
    }
}

int EffectLayer::GetEffectCount() const
{
    return mEffects.size();
}

bool EffectLayer::IsTimingLayer()
{
    TimingElement* te = dynamic_cast<TimingElement*>(GetParentElement());
    return !(te == nullptr);
}

bool EffectLayer::IsFixedTimingLayer()
{
    TimingElement* te = dynamic_cast<TimingElement*>(GetParentElement());
    return !(te == nullptr || !te->IsFixedTiming());
}

bool EffectLayer::HitTestEffectByTime(int timeMS, int& index) const
{
    for (int i = 0; i < mEffects.size(); i++) {
        if (timeMS >= mEffects[i]->GetStartTimeMS() &&
            timeMS <= mEffects[i]->GetEndTimeMS()) {
            index = i;
            return true;
        }
    }
    return false;
}

bool EffectLayer::HitTestEffectBetweenTime(int t1MS, int t2MS) const
{
    for (int i = 0; i < mEffects.size(); i++)
    {
        if ((mEffects[i]->GetStartTimeMS() > t1MS && mEffects[i]->GetStartTimeMS() < t2MS) ||
            (mEffects[i]->GetEndTimeMS() > t1MS && mEffects[i]->GetEndTimeMS() < t2MS) ||
            (mEffects[i]->GetStartTimeMS() == t1MS && mEffects[i]->GetEndTimeMS() == t2MS))
        {
            return true;
        }
    }
    return false;
}

Effect* EffectLayer::GetEffectBeforeTime(int ms) const
{
    int i;
    for (i = 0; i < mEffects.size(); i++)
    {
        if (mEffects[i]->GetStartTimeMS() >= ms)
        {
            break;
        }
    }
    if (i == 0)
    {
        return nullptr;
    }
    else
    {
        return mEffects[i - 1];
    }
}

Effect* EffectLayer::GetEffectAfterTime(int ms) const
{
    int i;
    for (i = 0; i < mEffects.size(); i++)
    {
        if (mEffects[i]->GetStartTimeMS() > ms)
        {
            break;
        }
    }
    if (i >= mEffects.size())
    {
        return nullptr;
    }
    else
    {
        return mEffects[i];
    }
}

Effect* EffectLayer::GetEffectAtTime(int timeMS) const
{
    for (int i = 0; i < mEffects.size(); i++) {
        if (timeMS >= mEffects[i]->GetStartTimeMS() &&
            timeMS <= mEffects[i]->GetEndTimeMS()) {
            return mEffects[i];
        }
    }
    return nullptr;
}

Effect* EffectLayer::GetEffectStartingAtTime(int timeMS) const
{
    for (int i = 0; i < mEffects.size(); i++) {
        if (timeMS == mEffects[i]->GetStartTimeMS()) {
            return mEffects[i];
        }
    }
    return nullptr;
}

Effect* EffectLayer::GetEffectBeforeEmptyTime(int ms) const
{
    int i;
    for (i = mEffects.size() - 1; i >= 0; i--)
    {
        if (mEffects[i]->GetEndTimeMS() < ms)
        {
            break;
        }
    }
    if (i < 0)
    {
        return nullptr;
    }
    else
    {
        return mEffects[i];
    }
}

Effect* EffectLayer::GetEffectAfterEmptyTime(int ms) const
{
    int i;
    for (i = 0; i < mEffects.size(); i++)
    {
        if (mEffects[i]->GetStartTimeMS() > ms)
        {
            break;
        }
    }
    if (i == mEffects.size())
    {
        return nullptr;
    }
    else
    {
        return mEffects[i];
    }
}

std::list<Effect*> EffectLayer::GetAllEffects() const
{
    std::list<Effect*> res;

    for (int i = 0; i < mEffects.size(); i++)
    {
        res.push_back(mEffects[i]);
    }

    return res;
}

bool EffectLayer::GetRangeIsClearMS(int startTimeMS, int endTimeMS, bool ignore_selected)
{
    for (int i = 0; i < mEffects.size(); i++)
    {
        if (ignore_selected)
        {
            if (mEffects[i]->GetSelected())
            {
                continue;
            }
        }
        // check if start is between effect range
        if ((startTimeMS > mEffects[i]->GetStartTimeMS()) && (startTimeMS < mEffects[i]->GetEndTimeMS()))
        {
            return false;
        }
        // check if end is between effect range
        if ((endTimeMS > mEffects[i]->GetStartTimeMS()) && (endTimeMS < mEffects[i]->GetEndTimeMS()))
        {
            return false;
        }
        // check effect is between start and end
        if ((mEffects[i]->GetStartTimeMS() >= startTimeMS) && (mEffects[i]->GetEndTimeMS() <= endTimeMS))
        {
            return false;
        }
    }
    return true;
}

bool EffectLayer::HasEffectsInTimeRange(int startTimeMS, int endTimeMS) {
    for (int i = 0; i < mEffects.size(); i++)
    {
        if (mEffects[i]->OverlapsWith(startTimeMS, endTimeMS)) return true;
    }
    return false;
}

bool EffectLayer::HasEffects()
{
    return mEffects.size() > 0;
}

bool EffectLayer::HasEffectsByType(const std::string& type) const
{
    for (int i = 0; i < mEffects.size(); i++)
    {
        if (mEffects[i]->GetEffectName() == type)
        {
            return true;
        }
    }
    return false;
}

int EffectLayer::SelectEffectsInTimeRange(int startTimeMS, int endTimeMS)
{
    int num_selected = 0;
    for (int i = 0; i < mEffects.size(); i++)
    {
        int midpoint = mEffects[i]->GetStartTimeMS() + ((mEffects[i]->GetEndTimeMS() - mEffects[i]->GetStartTimeMS()) / 2);
        if (mEffects[i]->GetStartTimeMS() >= startTimeMS && mEffects[i]->GetStartTimeMS() < endTimeMS)
        {
            if (endTimeMS < midpoint)
            {
                mEffects[i]->SetSelected(EFFECT_LT_SELECTED);
            }
            else
            {
                mEffects[i]->SetSelected(EFFECT_SELECTED);
            }
            num_selected++;
        }
        else if (mEffects[i]->GetEndTimeMS() <= endTimeMS && mEffects[i]->GetEndTimeMS() > startTimeMS)
        {
            if (startTimeMS > midpoint)
            {
                mEffects[i]->SetSelected(EFFECT_RT_SELECTED);
            }
            else
            {
                mEffects[i]->SetSelected(EFFECT_SELECTED);
            }
            num_selected++;
        }
        else if (mEffects[i]->GetEndTimeMS() > endTimeMS &&  mEffects[i]->GetStartTimeMS() < startTimeMS)
        {
            mEffects[i]->SetSelected(EFFECT_SELECTED);
            num_selected++;
        }
    }
    return num_selected;
}

std::vector<Effect*> EffectLayer::GetEffectsByTypeAndTime(const std::string &type, int startTimeMS, int endTimeMS)
{
    std::vector<Effect*> effs = std::vector<Effect*>();
    for (int i = 0; i < mEffects.size(); i++)
    {
        if (mEffects[i]->GetEffectName() == type)
        {
            if (mEffects[i]->GetStartTimeMS() >= startTimeMS && mEffects[i]->GetStartTimeMS() < endTimeMS)
            {
                effs.push_back(mEffects[i]);
            }
            else if (mEffects[i]->GetEndTimeMS() <= endTimeMS && mEffects[i]->GetEndTimeMS() > startTimeMS)
            {
                effs.push_back(mEffects[i]);
            }
            else if (mEffects[i]->GetEndTimeMS() > endTimeMS &&  mEffects[i]->GetStartTimeMS() < startTimeMS)
            {
                effs.push_back(mEffects[i]);
            }
        }
    }
    return effs;
}

std::vector<Effect*> EffectLayer::GetAllEffectsByTime(int startTimeMS, int endTimeMS)
{
    std::vector<Effect*> effs = std::vector<Effect*>();
    for (int i = 0; i < mEffects.size(); i++)
    {
        if (mEffects[i]->GetStartTimeMS() >= startTimeMS && mEffects[i]->GetStartTimeMS() < endTimeMS)
        {
            effs.push_back(mEffects[i]);
        }
        else if (mEffects[i]->GetEndTimeMS() <= endTimeMS && mEffects[i]->GetEndTimeMS() > startTimeMS)
        {
            effs.push_back(mEffects[i]);
        }
        else if (mEffects[i]->GetEndTimeMS() > endTimeMS &&  mEffects[i]->GetStartTimeMS() < startTimeMS)
        {
            effs.push_back(mEffects[i]);
        }
    }
    return effs;
}

void EffectLayer::PlayEffect(Effect* effect)
{
    EventPlayEffectArgs playArgs;
    wxCommandEvent eventPlayModelEffect(EVT_PLAY_MODEL_EFFECT);
    playArgs.element = GetParentElement();
    playArgs.effect = effect;
    playArgs.renderEffect = false;
    eventPlayModelEffect.SetClientData(&playArgs);
    // Pass it this way to prevent risk of effect being deleted before pointer is used
    GetParentElement()->GetSequenceElements()->GetXLightsFrame()->GetEventHandler()->ProcessEvent(eventPlayModelEffect);
}

Effect* EffectLayer::SelectEffectUsingDescription(std::string description)
{
    for (int i = 0; i < mEffects.size(); i++)
    {
        if (mEffects[i]->GetDescription() == description)
        {
            mEffects[i]->SetSelected(EFFECT_SELECTED);
            PlayEffect(mEffects[i]);
            return mEffects[i];
        }
    }

    return nullptr;
}

bool EffectLayer::IsEffectValid(Effect* e) const
{
    for (int i = 0; i < mEffects.size(); i++)
    {
        if ((void*)mEffects[i] == (void*)e)
        {
            return true;
        }
    }

    return false;
}

Effect* EffectLayer::SelectEffectUsingTime(int time)
{
    for (int i = 0; i < mEffects.size(); i++)
    {
        if (time >= mEffects[i]->GetStartTimeMS() && time < mEffects[i]->GetEndTimeMS())
        {
            mEffects[i]->SetSelected(EFFECT_SELECTED);
            PlayEffect(mEffects[i]);
            return mEffects[i];
        }
    }

    return nullptr;
}

int EffectLayer::GetLayerNumber() const {
    return GetParentElement()->GetLayerNumberFromIndex(GetIndex());
}

void EffectLayer::UnSelectAllEffects()
{
    for (int i = 0; i < mEffects.size(); i++)
    {
        mEffects[i]->SetSelected(EFFECT_NOT_SELECTED);
    }
}

void EffectLayer::SelectAllEffects()
{
    for (int i = 0; i < mEffects.size(); i++)
    {
        mEffects[i]->SetSelected(EFFECT_SELECTED);
    }
}

void EffectLayer::ConvertEffectsToPerModel(UndoManager& undo_manager)
{
    for (auto& it : mEffects) {
        auto buffer = it->GetSettings()["B_CHOICE_BufferStyle"];
        if (buffer == "Per Preview" || buffer == "Default" || buffer == "Single Line") {
            undo_manager.CaptureModifiedEffect(GetParentElement()->GetName(), GetIndex(), it->GetID(), it->GetSettingsAsString(), it->GetPaletteAsString());
            it->GetSettings()["B_CHOICE_BufferStyle"] = "Per Model " + buffer;
        }
        else if (buffer == "") {
            undo_manager.CaptureModifiedEffect(GetParentElement()->GetName(), GetIndex(), it->GetID(), it->GetSettingsAsString(), it->GetPaletteAsString());
            it->GetSettings()["B_CHOICE_BufferStyle"] = "Per Model Default";
        }
    }
}

Element* EffectLayer::GetParentElement() const
{
    return mParentElement;
}

void EffectLayer::SetParentElement(Element* parent)
{
    mParentElement = parent;
}

void EffectLayer::GetMaximumRangeOfMovementForSelectedEffects(int &toLeft, int &toRight)
{
    toLeft = NO_MAX;
    toRight = NO_MAX;
    for (int i = 0; i < mEffects.size(); i++) {
        if (mEffects[i]->GetSelected() != EFFECT_NOT_SELECTED) {
            int l = 0, r = 0;
            GetMaximumRangeOfMovementForEffect(i, l, r);
            toLeft = toLeft < l ? toLeft : l;
            toRight = toRight < r ? toRight : r;
        }
    }
}

void EffectLayer::GetMaximumRangeOfMovementForEffect(int index, int &toLeft, int &toRight)
{
    switch (mEffects[index]->GetSelected())
    {
    case EFFECT_LT_SELECTED:
        GetMaximumRangeWithLeftMovement(index, toLeft, toRight);
        break;
    case EFFECT_RT_SELECTED:
        GetMaximumRangeWithRightMovement(index, toLeft, toRight);
        break;
    case EFFECT_SELECTED: {
        int l1 = 0, l2 = 0, r1 = 0, r2 = 0;
        GetMaximumRangeWithLeftMovement(index, l1, r1);
        GetMaximumRangeWithRightMovement(index, l2, r2);
        toLeft = l1 < l2 ? l1 : l2;
        toRight = r1 < r2 ? r1 : r2;
        break;
    }
    default:
        toLeft = NO_MAX;
        toRight = NO_MAX;
        break;
    }
}

void EffectLayer::GetMaximumRangeWithLeftMovement(int index, int &toLeft, int &toRight)
{
    SequenceElements* seq_elem = mParentElement->GetSequenceElements();
    if (mEffects[index]->GetSelected() == EFFECT_SELECTED)
    {
        toRight = NO_MAX;
    }
    else
    {
        toRight = mEffects[index]->GetEndTimeMS() - mEffects[index]->GetStartTimeMS() - seq_elem->GetMinPeriod();
    }
    if (index == 0)
    {
        toLeft = mEffects[index]->GetStartTimeMS();
    }
    else
    {
        if (mEffects[index - 1]->GetSelected() == EFFECT_NOT_SELECTED ||
            ((index > 0) && mEffects[index - 1]->GetSelected() == EFFECT_LT_SELECTED))
        {
            toLeft = mEffects[index]->GetStartTimeMS() - mEffects[index - 1]->GetEndTimeMS();
        }
        else if (mEffects[index - 1]->GetSelected() == EFFECT_RT_SELECTED)
        {
            toLeft = mEffects[index]->GetStartTimeMS() - mEffects[index - 1]->GetStartTimeMS();
        }
        else if (mEffects[index - 1]->GetSelected() == EFFECT_SELECTED)
        {
            // Do not know so set to maximum. Let the effects to left decide
            toLeft = mEffects[index]->GetStartTimeMS();
        }
    }
}

void EffectLayer::GetMaximumRangeWithRightMovement(int index, int &toLeft, int &toRight)
{
    SequenceElements* seq_elem = mParentElement->GetSequenceElements();
    if (mEffects[index]->GetSelected() == EFFECT_SELECTED)
    {
        toLeft = mEffects[index]->GetStartTimeMS();
    }
    else
    {
        toLeft = mEffects[index]->GetEndTimeMS() - mEffects[index]->GetStartTimeMS() - seq_elem->GetMinPeriod();
    }
    // Last effect, nothing to right to stop movement other then edge of screen.
    // Let grid take care of screen boundary so set to huge number
    if (index == mEffects.size() - 1)
    {
        toRight = NO_MAX;
    }
    else
    {
        if (mEffects[index + 1]->GetSelected() == EFFECT_NOT_SELECTED ||
            ((index < mEffects.size() - 1) && mEffects[index + 1]->GetSelected() == EFFECT_RT_SELECTED))
        {
            toRight = mEffects[index + 1]->GetStartTimeMS() - mEffects[index]->GetEndTimeMS();
        }
        else if (mEffects[index + 1]->GetSelected() == EFFECT_LT_SELECTED)
        {
            toRight = mEffects[index + 1]->GetEndTimeMS() - mEffects[index]->GetEndTimeMS();
        }
        else if (mEffects[index + 1]->GetSelected() == EFFECT_SELECTED)
        {
            // Do not know so set to maximum. Let the effects to right decide
            toRight = NO_MAX;
        }
    }
}

int EffectLayer::GetSelectedEffectCount()
{
    wxString s;
    int count=0;
    for(int i=0; i<mEffects.size();i++)
    {
        if(mEffects[i]->GetSelected() != EFFECT_NOT_SELECTED)
        {
           count++;
        }
    }
    return count;
}

int EffectLayer::GetTaggedEffectCount()
{
    wxString s;
    int count=0;
    for(int i=0; i<mEffects.size();i++)
    {
        if(mEffects[i]->GetTagged())
        {
           count++;
        }
    }
    return count;
}

void EffectLayer::UpdateAllSelectedEffects(const std::string& palette)
{
    for(int i=0; i<mEffects.size();i++)
    {
        if(mEffects[i]->GetSelected() != EFFECT_NOT_SELECTED )
        {
            mEffects[i]->SetPalette(palette);
        }
    }
}

void EffectLayer::MoveAllSelectedEffects(int deltaMS, UndoManager& undo_mgr)
{
    std::unique_lock<std::recursive_mutex> locker(lock);
    for(int i=0; i<mEffects.size();i++)
    {
        if (!mEffects[i]->IsLocked())
        {
            if (mEffects[i]->GetSelected() == EFFECT_LT_SELECTED && mEffects[i]->GetTagged())
            {
                if (undo_mgr.GetCaptureUndo()) {
                    undo_mgr.CaptureEffectToBeMoved(mParentElement->GetModelName(), mIndex, mEffects[i]->GetID(),
                        mEffects[i]->GetStartTimeMS(), mEffects[i]->GetEndTimeMS());
                }
                mEffects[i]->SetStartTimeMS(mEffects[i]->GetStartTimeMS() + deltaMS);
            }
            else if (mEffects[i]->GetSelected() == EFFECT_RT_SELECTED && mEffects[i]->GetTagged())
            {
                if (undo_mgr.GetCaptureUndo()) {
                    undo_mgr.CaptureEffectToBeMoved(mParentElement->GetModelName(), mIndex, mEffects[i]->GetID(),
                        mEffects[i]->GetStartTimeMS(), mEffects[i]->GetEndTimeMS());
                }
                mEffects[i]->SetEndTimeMS(mEffects[i]->GetEndTimeMS() + deltaMS);
            }
            else if (mEffects[i]->GetSelected() == EFFECT_SELECTED && mEffects[i]->GetTagged())
            {
                if (undo_mgr.GetCaptureUndo()) {
                    undo_mgr.CaptureEffectToBeMoved(mParentElement->GetModelName(), mIndex, mEffects[i]->GetID(),
                        mEffects[i]->GetStartTimeMS(), mEffects[i]->GetEndTimeMS());
                }
                mEffects[i]->SetStartTimeMS(mEffects[i]->GetStartTimeMS() + deltaMS);
                mEffects[i]->SetEndTimeMS(mEffects[i]->GetEndTimeMS() + deltaMS);
            }
            mEffects[i]->SetTagged(false);
        }
    }
}

void EffectLayer::ButtUpMoveAllSelectedEffects(bool right, int lengthMS, UndoManager& undo_mgr)
{
    std::unique_lock<std::recursive_mutex> locker(lock);
    for (int i = 0; i<mEffects.size(); i++)
    {
        if (!mEffects[i]->IsLocked())
        {
            int length = mEffects[i]->GetEndTimeMS() - mEffects[i]->GetStartTimeMS();

            if (mEffects[i]->GetSelected() == EFFECT_LT_SELECTED && mEffects[i]->GetTagged())
            {
                if (undo_mgr.GetCaptureUndo()) {
                    undo_mgr.CaptureEffectToBeMoved(mParentElement->GetModelName(), mIndex, mEffects[i]->GetID(),
                        mEffects[i]->GetStartTimeMS(), mEffects[i]->GetEndTimeMS());
                }
                if (!right)
                {
                    Effect* prior = GetEffectBeforeTime(mEffects[i]->GetStartTimeMS());

                    if (prior == nullptr)
                    {
                        mEffects[i]->SetStartTimeMS(0);
                    }
                    else if (prior == mEffects[i])
                    {
                        // do nothing
                    }
                    else
                    {
                        mEffects[i]->SetStartTimeMS(prior->GetEndTimeMS());
                    }
                    mEffects[i]->SetEndTimeMS(mEffects[i]->GetStartTimeMS() + length);
                }
            }
            else if (mEffects[i]->GetSelected() == EFFECT_RT_SELECTED && mEffects[i]->GetTagged())
            {
                if (undo_mgr.GetCaptureUndo()) {
                    undo_mgr.CaptureEffectToBeMoved(mParentElement->GetModelName(), mIndex, mEffects[i]->GetID(),
                        mEffects[i]->GetStartTimeMS(), mEffects[i]->GetEndTimeMS());
                }
                if (right)
                {
                    Effect* next = GetEffectAfterTime(mEffects[i]->GetEndTimeMS() - 1);
                    if (next == nullptr)
                    {
                        mEffects[i]->SetEndTimeMS(lengthMS);
                    }
                    else if (next == mEffects[i])
                    {
                        // do nothing
                    }
                    else
                    {
                        mEffects[i]->SetEndTimeMS(next->GetStartTimeMS());
                    }
                    mEffects[i]->SetStartTimeMS(mEffects[i]->GetEndTimeMS() - length);
                }
            }
            else if (mEffects[i]->GetSelected() == EFFECT_SELECTED && mEffects[i]->GetTagged())
            {
                if (undo_mgr.GetCaptureUndo()) {
                    undo_mgr.CaptureEffectToBeMoved(mParentElement->GetModelName(), mIndex, mEffects[i]->GetID(),
                        mEffects[i]->GetStartTimeMS(), mEffects[i]->GetEndTimeMS());
                }
                if (right)
                {
                    Effect* next = GetEffectAfterTime(mEffects[i]->GetEndTimeMS() - 1);
                    if (next == nullptr)
                    {
                        mEffects[i]->SetEndTimeMS(lengthMS);
                    }
                    else if (next == mEffects[i])
                    {
                        // do nothing
                    }
                    else
                    {
                        mEffects[i]->SetEndTimeMS(next->GetStartTimeMS());
                    }
                    mEffects[i]->SetStartTimeMS(mEffects[i]->GetEndTimeMS() - length);
                }
                else
                {
                    Effect* prior = GetEffectBeforeTime(mEffects[i]->GetStartTimeMS());

                    if (prior == nullptr)
                    {
                        mEffects[i]->SetStartTimeMS(0);
                    }
                    else if (prior == mEffects[i])
                    {
                        // do nothing
                    }
                    else
                    {
                        mEffects[i]->SetStartTimeMS(prior->GetEndTimeMS());
                    }
                    mEffects[i]->SetEndTimeMS(mEffects[i]->GetStartTimeMS() + length);
                }
            }
            mEffects[i]->SetTagged(false);
        }
    }
}

void EffectLayer::StretchAllSelectedEffects(int deltaMS, UndoManager& undo_mgr)
{
    std::unique_lock<std::recursive_mutex> locker(lock);
    for (int i = 0; i<mEffects.size(); i++)
    {
        if (!mEffects[i]->IsLocked())
        {
            if (mEffects[i]->GetSelected() == EFFECT_LT_SELECTED && mEffects[i]->GetTagged())
            {
                if (undo_mgr.GetCaptureUndo()) {
                    undo_mgr.CaptureEffectToBeMoved(mParentElement->GetModelName(), mIndex, mEffects[i]->GetID(),
                        mEffects[i]->GetStartTimeMS(), mEffects[i]->GetEndTimeMS());
                }
                if (deltaMS < 0)
                {
                    mEffects[i]->SetStartTimeMS(mEffects[i]->GetStartTimeMS() + deltaMS);
                }
            }
            else if (mEffects[i]->GetSelected() == EFFECT_RT_SELECTED && mEffects[i]->GetTagged())
            {
                if (undo_mgr.GetCaptureUndo()) {
                    undo_mgr.CaptureEffectToBeMoved(mParentElement->GetModelName(), mIndex, mEffects[i]->GetID(),
                        mEffects[i]->GetStartTimeMS(), mEffects[i]->GetEndTimeMS());
                }
                if (deltaMS > 0)
                {
                    mEffects[i]->SetEndTimeMS(mEffects[i]->GetEndTimeMS() + deltaMS);
                }
            }
            else if (mEffects[i]->GetSelected() == EFFECT_SELECTED && mEffects[i]->GetTagged())
            {
                if (undo_mgr.GetCaptureUndo()) {
                    undo_mgr.CaptureEffectToBeMoved(mParentElement->GetModelName(), mIndex, mEffects[i]->GetID(),
                        mEffects[i]->GetStartTimeMS(), mEffects[i]->GetEndTimeMS());
                }
                if (deltaMS < 0)
                {
                    mEffects[i]->SetStartTimeMS(mEffects[i]->GetStartTimeMS() + deltaMS);
                }
                if (deltaMS > 0)
                {
                    mEffects[i]->SetEndTimeMS(mEffects[i]->GetEndTimeMS() + deltaMS);
                }
            }
            mEffects[i]->SetTagged(false);
        }
    }
}

void EffectLayer::ButtUpStretchAllSelectedEffects(bool right, int lengthMS, UndoManager& undo_mgr)
{
    std::unique_lock<std::recursive_mutex> locker(lock);
    for (int i = 0; i<mEffects.size(); i++)
    {
        if (!mEffects[i]->IsLocked())
        {
            if (mEffects[i]->GetSelected() == EFFECT_LT_SELECTED && mEffects[i]->GetTagged())
            {
                if (undo_mgr.GetCaptureUndo()) {
                    undo_mgr.CaptureEffectToBeMoved(mParentElement->GetModelName(), mIndex, mEffects[i]->GetID(),
                        mEffects[i]->GetStartTimeMS(), mEffects[i]->GetEndTimeMS());
                }
                if (!right)
                {
                    Effect* prior = GetEffectBeforeTime(mEffects[i]->GetStartTimeMS());

                    if (prior == nullptr)
                    {
                        mEffects[i]->SetStartTimeMS(0);
                    }
                    else if (prior == mEffects[i])
                    {
                        // do nothing
                    }
                    else
                    {
                        mEffects[i]->SetStartTimeMS(prior->GetEndTimeMS());
                    }
                }
            }
            else if (mEffects[i]->GetSelected() == EFFECT_RT_SELECTED && mEffects[i]->GetTagged())
            {
                if (undo_mgr.GetCaptureUndo()) {
                    undo_mgr.CaptureEffectToBeMoved(mParentElement->GetModelName(), mIndex, mEffects[i]->GetID(),
                        mEffects[i]->GetStartTimeMS(), mEffects[i]->GetEndTimeMS());
                }
                if (right)
                {
                    Effect* next = GetEffectAfterTime(mEffects[i]->GetEndTimeMS() - 1);
                    if (next == nullptr)
                    {
                        mEffects[i]->SetEndTimeMS(lengthMS);
                    }
                    else if (next == mEffects[i])
                    {
                        // do nothing
                    }
                    else
                    {
                        mEffects[i]->SetEndTimeMS(next->GetStartTimeMS());
                    }
                }
            }
            else if (mEffects[i]->GetSelected() == EFFECT_SELECTED && mEffects[i]->GetTagged())
            {
                if (undo_mgr.GetCaptureUndo()) {
                    undo_mgr.CaptureEffectToBeMoved(mParentElement->GetModelName(), mIndex, mEffects[i]->GetID(),
                        mEffects[i]->GetStartTimeMS(), mEffects[i]->GetEndTimeMS());
                }
                if (right)
                {
                    Effect* next = GetEffectAfterTime(mEffects[i]->GetEndTimeMS() - 1);
                    if (next == nullptr)
                    {
                        mEffects[i]->SetEndTimeMS(lengthMS);
                    }
                    else if (next == mEffects[i])
                    {
                        // do nothing
                    }
                    else
                    {
                        mEffects[i]->SetEndTimeMS(next->GetStartTimeMS());
                    }
                }
                else
                {
                    Effect* prior = GetEffectBeforeTime(mEffects[i]->GetStartTimeMS());

                    if (prior == nullptr)
                    {
                        mEffects[i]->SetStartTimeMS(0);
                    }
                    else if (prior == mEffects[i])
                    {
                        // do nothing
                    }
                    else
                    {
                        mEffects[i]->SetStartTimeMS(prior->GetEndTimeMS());
                    }
                }
            }
            mEffects[i]->SetTagged(false);
        }
    }
}

void EffectLayer::TagAllSelectedEffects()
{
    std::unique_lock<std::recursive_mutex> locker(lock);
    for(int i=0; i<mEffects.size();i++)
    {
        if( (mEffects[i]->GetSelected() == EFFECT_LT_SELECTED) ||
            (mEffects[i]->GetSelected() == EFFECT_RT_SELECTED) ||
            (mEffects[i]->GetSelected() == EFFECT_SELECTED) )
        {
            mEffects[i]->SetTagged(true);
        }
    }
}

std::vector<std::string> EffectLayer::GetUsedColours(bool selectedOnly)
{
    std::unique_lock<std::recursive_mutex> locker(lock);
    std::vector<std::string> res;

    for (int i = 0; i < mEffects.size(); i++) {
        if (!selectedOnly ||
            (mEffects[i]->GetSelected() == EFFECT_LT_SELECTED) ||
            (mEffects[i]->GetSelected() == EFFECT_RT_SELECTED) ||
            (mEffects[i]->GetSelected() == EFFECT_SELECTED)) {
            for (const auto& it : mEffects[i]->GetPalette()) {
                auto s = std::string(it);
                if (std::find(begin(res), end(res), s) == end(res)) {
                    res.push_back(s);
                }
            }
        }
    }
    return res;
}

int EffectLayer::ReplaceColours(xLightsFrame* frame, const std::string& from, const std::string& to, bool selectedOnly, UndoManager& undo_mgr)
{
    std::unique_lock<std::recursive_mutex> locker(lock);

    int replaced = 0;

    for (int i = 0; i < mEffects.size(); i++) {
        if (!selectedOnly ||
            (mEffects[i]->GetSelected() == EFFECT_LT_SELECTED) ||
            (mEffects[i]->GetSelected() == EFFECT_RT_SELECTED) ||
            (mEffects[i]->GetSelected() == EFFECT_SELECTED)) {
            if (mEffects[i]->UsesColour(from)) {
                undo_mgr.CaptureModifiedEffect(GetParentElement()->GetName(), GetIndex(), mEffects[i]->GetID(), mEffects[i]->GetSettingsAsString(), mEffects[i]->GetPaletteAsString());
                replaced += mEffects[i]->ReplaceColours(frame, from, to);
            }
        }
    }

    return replaced;
}

int EffectLayer::GetSelectedEffectCount(const std::string effectName)
{
    std::unique_lock<std::recursive_mutex> locker(lock);
    int count = 0;
    for (int i = 0; i<mEffects.size(); i++)
    {
        if ((effectName == "" || effectName == mEffects[i]->GetEffectName(-1)) &&
            ((mEffects[i]->GetSelected() == EFFECT_LT_SELECTED) ||
             (mEffects[i]->GetSelected() == EFFECT_RT_SELECTED) ||
             (mEffects[i]->GetSelected() == EFFECT_SELECTED))
           )
        {
            count++;
        }
    }
    return count;
}

void EffectLayer::ApplyEffectSettingToSelected(EffectsGrid* grid, UndoManager& undo_manager, const std::string& effectName, const std::string id, const std::string value, ValueCurve* vc, const std::string& vcid, EffectManager& effectManager, RangeAccumulator& rangeAccumulator)
{
    log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));

    for (int i = 0; i<mEffects.size(); i++)
    {
        RenderableEffect* eff1 = effectManager.GetEffect(effectName);
        if (eff1 == nullptr && effectName != "")
        {
            logger_base.error("Effect not found: '%s'", (const char *)effectName.c_str());
            wxASSERT(false);
        }
        RenderableEffect* eff2 = effectManager.GetEffect(mEffects[i]->GetEffectName());
        if (eff2 == nullptr)
        {
            // this cant happen
            logger_base.error("Effect not found when scanning effects: '%s'", (const char *)mEffects[i]->GetEffectName().c_str());
            wxASSERT(false);
        }
        if ((effectName == "" || eff1 == nullptr || eff1->GetId() == eff2->GetId()) &&
            ((mEffects[i]->GetSelected() == EFFECT_LT_SELECTED) ||
             (mEffects[i]->GetSelected() == EFFECT_RT_SELECTED) ||
             (mEffects[i]->GetSelected() == EFFECT_SELECTED))
           )
        {
            undo_manager.CaptureModifiedEffect(GetParentElement()->GetName(), GetIndex(), mEffects[i]->GetID(), mEffects[i]->GetSettingsAsString(), mEffects[i]->GetPaletteAsString());
            mEffects[i]->ApplySetting(id, value, vc, vcid);

            rangeAccumulator.Add(mEffects[i]->GetStartTimeMS(), mEffects[i]->GetEndTimeMS());
        }
    }
}

void EffectLayer::ApplyButtonPressToSelected(EffectsGrid* grid, UndoManager& undo_manager, const std::string& effectName, const std::string id, EffectManager& effectManager, RangeAccumulator& rangeAccumulator)
{
    log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));

    for (int i = 0; i < mEffects.size(); i++) {
        RenderableEffect* eff1 = effectManager.GetEffect(effectName);
        if (eff1 == nullptr && effectName != "") {
            logger_base.error("Effect not found: '%s'", (const char*)effectName.c_str());
            wxASSERT(false);
        }
        RenderableEffect* eff2 = effectManager.GetEffect(mEffects[i]->GetEffectName());
        if (eff2 == nullptr) {
            // this cant happen
            logger_base.error("Effect not found when scanning effects: '%s'", (const char*)mEffects[i]->GetEffectName().c_str());
            wxASSERT(false);
        }
        if ((effectName == "" || eff1 == nullptr || eff2 == nullptr || eff1->GetId() == eff2->GetId()) &&
            ((mEffects[i]->GetSelected() == EFFECT_LT_SELECTED) ||
                (mEffects[i]->GetSelected() == EFFECT_RT_SELECTED) ||
                (mEffects[i]->GetSelected() == EFFECT_SELECTED))
            ) {
            undo_manager.CaptureModifiedEffect(GetParentElement()->GetName(), GetIndex(), mEffects[i]->GetID(), mEffects[i]->GetSettingsAsString(), mEffects[i]->GetPaletteAsString());
            mEffects[i]->PressButton(eff2, id);

            rangeAccumulator.Add(mEffects[i]->GetStartTimeMS(), mEffects[i]->GetEndTimeMS());
        }
    }
}

void EffectLayer::RemapSelectedDMXEffectValues(EffectsGrid* effects_grid, UndoManager& undo_manager, const std::vector<std::tuple<int, int, float, int>>& dmxmappings, const EffectManager& effectManager, RangeAccumulator& rangeAccumulator)
{
    DMXEffect* dmx = static_cast<DMXEffect*>(effectManager.GetEffect("DMX"));

    for (int i = 0; i < mEffects.size(); i++) {
        if (mEffects[i]->GetEffectName() == "DMX" &&
            ((mEffects[i]->GetSelected() == EFFECT_LT_SELECTED) ||
                (mEffects[i]->GetSelected() == EFFECT_RT_SELECTED) ||
                (mEffects[i]->GetSelected() == EFFECT_SELECTED))) {
            undo_manager.CaptureModifiedEffect(GetParentElement()->GetName(), GetIndex(), mEffects[i]->GetID(), mEffects[i]->GetSettingsAsString(), mEffects[i]->GetPaletteAsString());
            dmx->RemapSelectedDMXEffectValues(mEffects[i], dmxmappings);

            rangeAccumulator.Add(mEffects[i]->GetStartTimeMS(), mEffects[i]->GetEndTimeMS());
        }
    }
}

void EffectLayer::ConvertSelectedEffectsTo(EffectsGrid* grid, UndoManager& undo_manager, const std::string& effectName, EffectManager& effectManager, RangeAccumulator& rangeAccumulator)
{
    for (int i = 0; i < mEffects.size(); i++) {
        if ((mEffects[i]->GetSelected() == EFFECT_LT_SELECTED) ||
            (mEffects[i]->GetSelected() == EFFECT_RT_SELECTED) ||
            (mEffects[i]->GetSelected() == EFFECT_SELECTED)
            ) {
            undo_manager.CaptureModifiedEffect(GetParentElement()->GetName(), GetIndex(), mEffects[i]->GetID(), mEffects[i]->GetSettingsAsString(), mEffects[i]->GetPaletteAsString());
            mEffects[i]->ConvertTo(effectManager.GetEffectIndex(effectName));

            rangeAccumulator.Add(mEffects[i]->GetStartTimeMS(), mEffects[i]->GetEndTimeMS());
        }
    }
}

void EffectLayer::UnTagAllEffects()
{
    std::unique_lock<std::recursive_mutex> locker(lock);
    for (int i = 0; i < mEffects.size(); i++) {
        mEffects[i]->SetTagged(false);
    }
}

void EffectLayer::DeleteSelectedEffects(UndoManager& undo_mgr)
{
    std::unique_lock<std::recursive_mutex> locker(lock);
    for (const auto& it : mEffects) {
        if (!it->IsLocked())
        {
            if (it->GetSelected() != EFFECT_NOT_SELECTED) {
                IncrementChangeCount(it->GetStartTimeMS(), it->GetEndTimeMS());
                undo_mgr.CaptureEffectToBeDeleted(mParentElement->GetModelName(), mIndex, it->GetEffectName(),
                    it->GetSettingsAsString(), it->GetPaletteAsString(),
                    it->GetStartTimeMS(), it->GetEndTimeMS(),
                    it->GetSelected(), it->GetProtected());
                it->SetTimeToDelete();
                mEffectsToDelete.push_back(it);
            }
        }
    }
    mEffects.erase(std::remove_if(mEffects.begin(), mEffects.end(), ShouldDeleteSelected),mEffects.end());
}

void EffectLayer::DeleteAllEffects()
{
    std::unique_lock<std::recursive_mutex> locker(lock);

    for (const auto& it : mEffects) {
        if (!it->IsLocked()) {
            IncrementChangeCount(it->GetStartTimeMS(), it->GetEndTimeMS());
            it->SetTimeToDelete();
            mEffectsToDelete.push_back(it);
        }
    }
    mEffects.erase(std::remove_if(mEffects.begin(), mEffects.end(), ShouldDeleteNotLocked), mEffects.end());
}

void EffectLayer::DeleteEffectByIndex(int idx) {
    std::unique_lock<std::recursive_mutex> locker(lock);
    if (!mEffects[idx]->IsLocked())
    {
        IncrementChangeCount(mEffects[idx]->GetStartTimeMS(), mEffects[idx]->GetEndTimeMS());
        mEffects[idx]->SetTimeToDelete();
        mEffectsToDelete.push_back(mEffects[idx]);
        mEffects.erase(mEffects.begin() + idx);
    }
}

bool EffectLayer::ShouldDeleteSelected(Effect *eff)
{
    return !eff->IsLocked() && eff->GetSelected() != EFFECT_NOT_SELECTED;
}

bool EffectLayer::ShouldDeleteNotLocked(Effect* eff)
{
    return !eff->IsLocked();
}

bool EffectLayer::SortEffectByStartTime(Effect *e1,Effect *e2)
{
    return e1->GetStartTimeMS() < e2->GetStartTimeMS();
}

void EffectLayer::IncrementChangeCount(int startMS, int endMS)
{
    if (mParentElement) {
        mParentElement->IncrementChangeCount(startMS, endMS);
    }
}

std::list<std::string> EffectLayer::GetFileReferences(Model* model, EffectManager& em) const
{
    std::list<std::string> res;

    for (int k = 0; k < GetEffectCount(); k++)
    {
        Effect* ef = GetEffect(k);

        if (ef->GetEffectIndex() >= 0)
        {
            RenderableEffect *eff = em[ef->GetEffectIndex()];
            res.splice(end(res), eff->GetFileReferences(model, ef->GetSettings()));
        }
    }

    return res;
}

std::list<std::string> EffectLayer::GetFacesUsed(EffectManager& em) const
{
    std::list<std::string> res;

    for (int k = 0; k < GetEffectCount(); k++)
    {
        Effect* ef = GetEffect(k);

        if (ef->GetEffectIndex() >= 0)
        {
            RenderableEffect *eff = em[ef->GetEffectIndex()];
            res.splice(end(res), eff->GetFacesUsed(ef->GetSettings()));
        }
    }

    return res;
}

bool EffectLayer::CleanupFileLocations(xLightsFrame* frame, EffectManager& em)
{
    bool rc = false;

    for (int k = 0; k < GetEffectCount(); k++)
    {
        Effect* ef = GetEffect(k);

        if (ef->GetEffectIndex() >= 0)
        {
            RenderableEffect *eff = em[ef->GetEffectIndex()];
            rc = eff->CleanupFileLocations(frame, ef->GetSettings()) || rc;
        }
    }

    return rc;
}
