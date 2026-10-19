/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include <algorithm>
#include <cstdlib>

#include "xLightsMain.h"
#include "media/AudioManager.h"
#include "sequencer/ControlSurfaceManager.h"
#include "sequencer/EffectsGrid.h"
#include "sequencer/MainSequencer.h"
#include "sequencer/TimeLine.h"
#include "sequencer/Waveform.h"

namespace {
constexpr int SKIP_MS = 10000;
}

mcu::Status xLightsFrame::GetControlSurfaceStatus() const {
    mcu::Status s;
    if (CurrentSeqXmlFile == nullptr || mainSequencer == nullptr) {
        return s;
    }
    s.sequenceOpen = true;
    if (playType == PLAY_TYPE_MODEL || playType == PLAY_TYPE_EFFECT) {
        s.transport = mcu::TransportState::Playing;
    } else if (playType == PLAY_TYPE_MODEL_PAUSED || playType == PLAY_TYPE_EFFECT_PAUSED) {
        s.transport = mcu::TransportState::Paused;
    }
    s.looping = mLoopAudio;
    if (s.transport == mcu::TransportState::Stopped) {
        s.timeMS = mainSequencer->PanelTimeLine->GetSelectedPositionStartMS();
    } else {
        s.timeMS = mainSequencer->PanelTimeLine->GetCurrentPlayMarkerMS();
    }
    s.timeMS = std::max(0, s.timeMS);
    return s;
}

// Moves the stopped/paused play position and shows that frame, as dragging
// on the waveform with Ctrl held does.
void xLightsFrame::MoveControlSurfaceCursor(int deltaMS, bool scrubAudio) {
    TimeLine* timeline = mainSequencer->PanelTimeLine;
    int pos;
    if (playType == PLAY_TYPE_MODEL_PAUSED) {
        pos = timeline->GetCurrentPlayMarkerMS();
        DoStopSequence();
    } else {
        pos = timeline->GetSelectedPositionStartMS();
    }
    pos = std::max(0, pos);

    int frameMS = std::max(1, CurrentSeqXmlFile->GetFrameMS());
    int lastFrameMS = std::max(0, CurrentSeqXmlFile->GetSequenceDurationMS() - frameMS);
    pos = std::clamp(pos + deltaMS, 0, lastFrameMS);
    pos -= pos % frameMS;

    timeline->SetSelectedPositionStartMS(pos);
    timeline->EnsureTimeVisible(pos, pos);
    mainSequencer->PanelWaveForm->Refresh(false);
    mainSequencer->PanelEffectGrid->ForceRefresh();

    if (scrubAudio) {
        AudioManager* audio = GetPlaybackAudio();
        if (audio != nullptr) {
            audio->Play(pos, std::max(50, std::abs(deltaMS)));
        }
    }

    if (_seqData.NumFrames() > 0) {
        wxCommandEvent scrub(EVT_SCRUB);
        scrub.SetInt(pos);
        Scrub(scrub);
    }
}

void xLightsFrame::ExecuteControlSurfaceAction(const mcu::Action& action) {
    if (CurrentSeqXmlFile == nullptr || mainSequencer == nullptr) {
        return;
    }
    bool playing = playType == PLAY_TYPE_MODEL;
    bool paused = playType == PLAY_TYPE_MODEL_PAUSED;

    auto post = [this](const wxEventType& type, int value = 0) {
        wxCommandEvent e(type);
        e.SetInt(value);
        wxPostEvent(this, e);
    };

    switch (action.command) {
    case mcu::Command::Play:
        if (playing || paused) {
            post(EVT_PAUSE_SEQUENCE);
        } else {
            post(EVT_PLAY_SEQUENCE);
        }
        break;
    case mcu::Command::Stop:
        if (playType != PLAY_TYPE_STOPPED) {
            post(EVT_STOP_SEQUENCE);
        } else {
            post(EVT_SEQUENCE_FIRST_FRAME);
        }
        break;
    case mcu::Command::Rewind:
        if (playing) {
            post(EVT_SEQUENCE_REWIND10);
        } else {
            MoveControlSurfaceCursor(-SKIP_MS, false);
        }
        break;
    case mcu::Command::FastForward:
        if (playing) {
            post(EVT_SEQUENCE_FFORWARD10);
        } else {
            MoveControlSurfaceCursor(SKIP_MS, false);
        }
        break;
    case mcu::Command::Loop:
        if (playing || paused) {
            mLoopAudio = !mLoopAudio;
        } else {
            post(EVT_SEQUENCE_REPLAY_SECTION);
        }
        break;
    case mcu::Command::AddTimingMark:
        mainSequencer->InsertTimingMarkFromRange();
        break;
    case mcu::Command::ZoomIn:
        mainSequencer->PanelTimeLine->ZoomIn();
        break;
    case mcu::Command::ZoomOut:
        mainSequencer->PanelTimeLine->ZoomOut();
        break;
    case mcu::Command::Up:
        mainSequencer->PanelEffectGrid->MoveSelectedEffectUp(false);
        break;
    case mcu::Command::Down:
        mainSequencer->PanelEffectGrid->MoveSelectedEffectDown(false);
        break;
    case mcu::Command::Left:
        mainSequencer->PanelEffectGrid->MoveSelectedEffectLeft(false, false, false);
        break;
    case mcu::Command::Right:
        mainSequencer->PanelEffectGrid->MoveSelectedEffectRight(false, false, false);
        break;
    case mcu::Command::Jog: {
        int deltaMS = action.ticks * std::max(1, CurrentSeqXmlFile->GetFrameMS());
        if (playing) {
            int pos = std::max(0, mainSequencer->PanelTimeLine->GetCurrentPlayMarkerMS() + deltaMS);
            post(EVT_SEQUENCE_SEEKTO, pos);
        } else {
            MoveControlSurfaceCursor(deltaMS, action.scrub);
        }
        break;
    }
    }
}
