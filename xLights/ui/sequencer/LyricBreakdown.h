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

#include <string>

class EffectLayer;
class UndoManager;
class xLightsFrame;

void BreakdownPhrase(EffectLayer* word_layer, int start_time, int end_time,
                     const std::string& phrase, double frequency, UndoManager& undo_mgr);

void BreakdownWord(EffectLayer* phoneme_layer, int start_time, int end_time,
                   const std::string& word, double frequency, xLightsFrame* xframe, UndoManager& undo_mgr);
