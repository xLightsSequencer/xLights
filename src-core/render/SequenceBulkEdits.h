#pragma once

/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

// Whole-sequence edit tools shared by the desktop and iPad menus. Each
// records its changes as commands, as one undo step named by `label`.

#include <string>
#include <vector>

class SequenceElements;
class EffectLayer;
class ModelElement;

// The layers the Shift Effects tools walk: each element's layers and, for
// models, their strands' node layers and their submodels' and strands'
// layers. Timing tracks are included only with includeTiming.
std::vector<EffectLayer*> ShiftableLayers(const SequenceElements& seq, bool includeTiming);

// Moves the effects on `layers` by `ms`. Effects pushed entirely before 0
// are deleted; effects crossing 0 are truncated. Locked effects are left
// alone. With
// selectedOnly, only selected effects move, and one whose new range would
// overlap another effect on its layer stays where it is. Returns true if it
// recorded an undo step: false when nothing changed (or when it joined a
// batch the caller has open).
bool ShiftEffects(SequenceElements& seq, const std::vector<EffectLayer*>& layers, int ms, bool selectedOnly, const std::string& label);
// The same over all the ShiftableLayers.
bool ShiftEffects(SequenceElements& seq, int ms, bool includeTiming, bool selectedOnly, const std::string& label);

// Promotes node effects to their strand, then strand effects to the model,
// where every node (strand) carries an identical On or Color Wash effect over
// the same time. Returns the number of effects promoted.
int PromoteNodeEffects(SequenceElements& seq, ModelElement* element, const std::string& label = "Promote Node Effects");
