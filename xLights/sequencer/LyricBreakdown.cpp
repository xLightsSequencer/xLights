/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include <cassert>
#include <string>
#include <vector>

#include "LyricBreakdown.h"
#include "../render/EffectLayer.h"
#include "../render/RenderUtils.h"
#include "../xLightsMain.h"

#include <wx/arrstr.h>

static bool removechar(std::string& word, char remove)
{
    auto pos = word.find(remove);
    if (pos != std::string::npos)
    {
        word.erase(pos, 1);
        return true;
    }
    return false;
}

void BreakdownPhrase(EffectLayer* word_layer, int start_time, int end_time,
                     const std::string& phrase, double frequency, UndoManager& undo_mgr)
{
    if (!phrase.empty())
    {
        static const std::string delims = " \t:;,.-_!?{}[]()<>+=|";
        std::vector<std::string> words;
        size_t start = 0;
        while (start < phrase.size()) {
            auto pos = phrase.find_first_of(delims, start);
            if (pos != start) {
                std::string w = phrase.substr(start, pos - start);
                if (!w.empty()) {
                    words.push_back(std::move(w));
                }
            }
            if (pos == std::string::npos) break;
            start = pos + 1;
        }
        int num_words = words.size();
        if (num_words == 0) return;
        double interval_ms = (end_time-start_time) / num_words;
        int word_start_time = start_time;
        for( int i = 0; i < num_words; i++ )
        {
            int word_end_time = RoundToMultipleOfPeriod(start_time+(interval_ms * (i + 1)), frequency);
            if( i == num_words - 1  || word_end_time > end_time)
            {
                word_end_time = end_time;
            }
            Effect* ef = word_layer->AddEffect(0,words[i],"","",word_start_time,word_end_time,EFFECT_NOT_SELECTED,false);
            undo_mgr.CaptureAddedEffect(word_layer->GetParentElement()->GetName(), word_layer->GetIndex(), ef->GetID());
            word_start_time = word_end_time;
        }
    }
}

void BreakdownWord(EffectLayer* phoneme_layer, int start_time, int end_time,
                   const std::string& word, double frequency, xLightsFrame* xframe, UndoManager& undo_mgr)
{
    xframe->dictionary.LoadDictionaries(xframe->CurrentDir, xframe);
    wxArrayString wxPhonemes;
    xframe->dictionary.BreakdownWord(word, wxPhonemes);

    // Convert to std::vector for easier use
    std::vector<std::string> phonemes;
    phonemes.reserve(wxPhonemes.size());
    for (const auto& p : wxPhonemes) {
        phonemes.push_back(p.ToStdString());
    }

    if (!phonemes.empty())
    {
        int countShort = 0;
        for (const auto& it : phonemes) {
            if (it == "etc" || it == "MBP") countShort++;
        }

        int minPeriod = (int)(1000.0 / frequency);
        double default_interval_ms = (end_time - start_time) / (double)phonemes.size();
        double short_interval = 50; // our preferred interval for MBP/etc
        if (default_interval_ms < 50) {
            short_interval = minPeriod;
        }
        double adjusted_interval = default_interval_ms;
        if (phonemes.size() > 1) {
            adjusted_interval = (end_time - start_time - countShort * short_interval) / (phonemes.size() - countShort);
        } else {
            short_interval = default_interval_ms;
        }

        int phoneme_start_time = start_time;
        int shorts = 0;
        int longs = 0;
        for (const auto& phoneme : phonemes)
        {
            if (phoneme == "etc" || phoneme == "MBP") {
                shorts++;
            } else {
                longs++;
            }
            int phoneme_end_time = RoundToMultipleOfPeriod(start_time + longs * adjusted_interval + shorts * short_interval, frequency);
            if (phoneme_end_time > end_time) {
                phoneme_end_time = end_time;
            }
            assert(phoneme_start_time < phoneme_end_time);

            if (phoneme_end_time > phoneme_start_time) {
                Effect* ef = phoneme_layer->AddEffect(0, phoneme, "", "", phoneme_start_time, phoneme_end_time, EFFECT_NOT_SELECTED, false);
                undo_mgr.CaptureAddedEffect(phoneme_layer->GetParentElement()->GetName(), phoneme_layer->GetIndex(), ef->GetID());
            }
            phoneme_start_time = phoneme_end_time;
        }
    }
}
