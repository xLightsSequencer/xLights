/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include <wx/wx.h>
#ifdef __WXMAC__
    #include "OpenGL/gl.h"
#else
    #include <GL/gl.h>
#endif
#include <wx/glcanvas.h>
#include <wx/event.h>
#include <wx/bitmap.h>
#include <wx/intl.h>
#include <wx/image.h>
#include <wx/string.h>

#include "shared/utils/xLightsTimer.h"
#include "Waveform.h"
#include "TimeLine.h"
#include "sequencer/RenderCommandEvent.h"
#include <wx/file.h>
#include "color/ColorManager.h"
#include "xLightsApp.h"
#include "xLightsMain.h"
#include "MainSequencer.h"
#include "sequencer/NoteRangeDialog.h"
#include "media/OnsetDetector.h"
#include "media/PitchDetector.h"
#include "media/Spectrogram.h"
#include "media/AIModelStore.h"
#ifdef __APPLE__
#include "media/SoundClassifier.h"
#include <wx/zipstrm.h>
#include <wx/wfstream.h>
#endif
#if defined(__APPLE__) || defined(HAVE_OPENVINO) || defined(HAVE_ORT)
#include "media/StemSeparator.h"
#include <wx/progdlg.h>
#include "utils/CurlManager.h"
#include <atomic>
#include <filesystem>
#include <thread>
#endif

#include <log.h>

wxDEFINE_EVENT(EVT_WAVE_FORM_MOVED, wxCommandEvent);
wxDEFINE_EVENT(EVT_WAVE_FORM_HIGHLIGHT, wxCommandEvent);

BEGIN_EVENT_TABLE(Waveform, GRAPHICS_BASE_CLASS)
EVT_MOTION(Waveform::mouseMoved)
EVT_LEFT_DOWN(Waveform::mouseLeftDown)
EVT_LEFT_UP(Waveform::mouseLeftUp)
EVT_LEFT_DCLICK(Waveform::OnLeftDClick)
EVT_RIGHT_DOWN(Waveform::rightClick)
EVT_MOUSE_CAPTURE_LOST(Waveform::OnLostMouseCapture)
EVT_LEAVE_WINDOW(Waveform::mouseLeftWindow)
EVT_SIZE(Waveform::Resized)
EVT_MOUSEWHEEL(Waveform::mouseWheelMoved)
EVT_PAINT(Waveform::Paint)
END_EVENT_TABLE()

const long Waveform::ID_WAVE_MNU_RENDER = wxNewId();
const long Waveform::ID_WAVE_MNU_RAW = wxNewId();
const long Waveform::ID_WAVE_MNU_BASS = wxNewId();
const long Waveform::ID_WAVE_MNU_ALTO = wxNewId();
const long Waveform::ID_WAVE_MNU_TREBLE = wxNewId();
const long Waveform::ID_WAVE_MNU_CUSTOM = wxNewId();
const long Waveform::ID_WAVE_MNU_NONVOCALS = wxNewId();
const long Waveform::ID_WAVE_MNU_LUFS = wxNewId();
const long Waveform::ID_WAVE_MNU_VOCALS = wxNewId();
const long Waveform::ID_WAVE_MNU_DOUBLEHEIGHT = wxNewId();
const long Waveform::ID_WAVE_MNU_SHOW_ONSETS = wxNewId();
const long Waveform::ID_WAVE_MNU_SHOW_PITCH = wxNewId();
const long Waveform::ID_WAVE_MNU_SHOW_SPECTROGRAM = wxNewId();
#if defined(__APPLE__) || defined(HAVE_OPENVINO) || defined(HAVE_ORT)
const long Waveform::ID_WAVE_MNU_STEM_DRUMS = wxNewId();
const long Waveform::ID_WAVE_MNU_STEM_BASS = wxNewId();
const long Waveform::ID_WAVE_MNU_STEM_OTHER = wxNewId();
const long Waveform::ID_WAVE_MNU_STEM_VOCALS = wxNewId();
#endif
#ifdef __APPLE__
const long Waveform::ID_WAVE_MNU_CLASSIFY = wxNewId();
const long Waveform::ID_WAVE_MNU_CLASSIFY_CLEAR = wxNewId();
#endif
// Reserve 32 IDs for audio track submenu items (base + 0..31)
const long Waveform::ID_WAVE_MNU_AUDIO_TRACK_BASE = wxNewId();
static long _audioTrackIdPool[32];
static bool _audioTrackIdPoolInited = false;
static void EnsureAudioTrackIds() {
    if (!_audioTrackIdPoolInited) {
        for (int i = 0; i < 32; i++) _audioTrackIdPool[i] = wxNewId();
        _audioTrackIdPoolInited = true;
    }
}

Waveform::Waveform(wxPanel* parent, wxWindowID id, const wxPoint &pos, const wxSize &size,
                   long style, const wxString &name):
    GRAPHICS_BASE_CLASS(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, 0, "WaveForm")
{
    
    spdlog::debug("                Creating Waveform");
    m_dragging = false;
    m_drag_mode = DRAG_NORMAL;
    mParent = parent;
    mCurrentWaveView = NO_WAVE_VIEW_SELECTED;
    mZoomLevel=0;
    mStartPixelOffset = 0;
    mFrequency = 40;
    _media = nullptr;
    mTimeline = nullptr;
}

Waveform::~Waveform()
{
    CloseMedia();
    if (_spectrogramTexture) {
        delete _spectrogramTexture;
        _spectrogramTexture = nullptr;
    }
}

void Waveform::ResetAnalysisState()
{
    _showOnsets = false;
    _onsetsComputed = false;
    _onsetMS.clear();
    _showPitchContour = false;
    _pitchComputed = false;
    _pitchContour.clear();
    _showSpectrogram = false;
    _spectrogramComputed = false;
    _spectrogram = Spectrogram{};
    if (_spectrogramTexture) {
        delete _spectrogramTexture;
        _spectrogramTexture = nullptr;
    }
    _spectrogramTexW = 0;
    _spectrogramTexH = 0;
    _spectrogramRangeStartMS = -1;
    _spectrogramRangeEndMS = -1;
#ifdef __APPLE__
    _soundClasses.clear();
    _selectedSoundClass.clear();
    _soundClassTimeStep = 1.0f;
#endif
    _doubleHeight = false;
}

void Waveform::CloseMedia()
{
    views.clear();
    mCurrentWaveView = NO_WAVE_VIEW_SELECTED;
    _type = AUDIOSAMPLETYPE::RAW;
    _lowNote = -1;
    _highNote = -1;
    _media = nullptr;
    ResetAnalysisState();
    mParent->Refresh();
}

void Waveform::OnLostMouseCapture(wxMouseCaptureLostEvent& event)
{
    m_dragging = false;
}

void Waveform::SetTimeline(TimeLine* timeLine)
{
    mTimeline = timeLine;
}

void Waveform::OnLeftDClick(wxMouseEvent& event)
{
    if(!mIsInitialized){return;}
    // Zoom on double click
    wxCommandEvent eventZoom(EVT_ZOOM);

    if (event.ShiftDown() || event.ControlDown()) {
        eventZoom.SetInt(ZOOM_OUT);
    } else {
        eventZoom.SetInt(ZOOM_IN);
    }

    wxPostEvent(GetParent(), eventZoom);
}

void Waveform::UpdatePlayMarker()
{
    render();
}

void Waveform::CheckNeedToScroll() const
{
    int StartTime;
    int EndTime;
    mTimeline->GetViewableTimeRange(StartTime, EndTime);
    int scroll_point = mTimeline->GetPositionFromTimeMS(EndTime) * 0.99;
    if (mTimeline->GetPlayMarker() > scroll_point) {
        wxCommandEvent eventScroll(EVT_SCROLL_RIGHT);
        wxPostEvent(mParent, eventScroll);
    }
}

void Waveform::mouseLeftDown( wxMouseEvent& event)
{
    if(!mIsInitialized) return;

    if (!m_dragging) {
        m_dragging = true;
        CaptureMouse();
    }
    if (m_drag_mode == DRAG_NORMAL) {
        mTimeline->SetSelectedPositionStart(event.GetX());
    }
    SetFocus();
    Refresh(false);

    wxCommandEvent eventSelected(EVT_WAVE_FORM_HIGHLIGHT);
    eventSelected.SetInt(0);
    wxPostEvent(mParent, eventSelected);
}

void Waveform::mouseLeftUp( wxMouseEvent& event)
{
    if (m_dragging) {
        ReleaseMouse();
        m_dragging = false;
    }

    mTimeline->LatchSelectedPositions();
    Refresh(false);

    wxCommandEvent eventSelected(EVT_WAVE_FORM_HIGHLIGHT);
    if (mTimeline->GetNewEndTimeMS() == -1) {
        eventSelected.SetInt(0);
    } else {
        eventSelected.SetInt(abs(mTimeline->GetNewStartTimeMS() - mTimeline->GetNewEndTimeMS()));
    }
    wxPostEvent(mParent, eventSelected);
}

void Waveform::rightClick(wxMouseEvent& event)
{
    wxMenu mnuWave;
    if( (mTimeline->GetSelectedPositionStartMS() != -1 ) &&
        (mTimeline->GetSelectedPositionEndMS() != -1 ) ) {
        mnuWave.Append(ID_WAVE_MNU_RENDER,"Render Selected Region");
    }
    if (_media != nullptr) {
        if (mnuWave.GetMenuItemCount() > 0) {
            mnuWave.AppendSeparator();
        }

        mnuWave.AppendRadioItem(ID_WAVE_MNU_RAW, "Raw waveform")->Check(_type == AUDIOSAMPLETYPE::RAW);
        mnuWave.AppendRadioItem(ID_WAVE_MNU_BASS, "Bass waveform")->Check(_type == AUDIOSAMPLETYPE::BASS);
        mnuWave.AppendRadioItem(ID_WAVE_MNU_TREBLE, "Treble waveform")->Check(_type == AUDIOSAMPLETYPE::TREBLE);
        mnuWave.AppendRadioItem(ID_WAVE_MNU_ALTO, "Alto waveform")->Check(_type == AUDIOSAMPLETYPE::ALTO);
        mnuWave.AppendRadioItem(ID_WAVE_MNU_CUSTOM, "Custom filtered waveform")->Check(_type == AUDIOSAMPLETYPE::CUSTOM);
        mnuWave.AppendRadioItem(ID_WAVE_MNU_NONVOCALS, "Non Vocals waveform")->Check(_type == AUDIOSAMPLETYPE::NONVOCALS);
        mnuWave.AppendRadioItem(ID_WAVE_MNU_VOCALS, "Vocals waveform (center extract)")->Check(_type == AUDIOSAMPLETYPE::VOCALS);
        mnuWave.AppendRadioItem(ID_WAVE_MNU_LUFS, "Perceptual (LUFS)")->Check(_type == AUDIOSAMPLETYPE::LUFS);
#ifdef __APPLE__
        // A8 — HTDemucs stems. Each in the same radio group so its
        // checkmark tracks the active `_type`. First activation runs
        // the download + separator; subsequent picks are instant
        // cache hits. Only offered on macOS 12+ — the Float16
        // MLMultiArray I/O the model uses isn't available before.
        if (__builtin_available(macOS 12.0, *)) {
            mnuWave.AppendRadioItem(ID_WAVE_MNU_STEM_DRUMS, "Stem — Drums")
                ->Check(_type == AUDIOSAMPLETYPE::STEM_DRUMS);
            mnuWave.AppendRadioItem(ID_WAVE_MNU_STEM_BASS, "Stem — Bass")
                ->Check(_type == AUDIOSAMPLETYPE::STEM_BASS);
            mnuWave.AppendRadioItem(ID_WAVE_MNU_STEM_OTHER, "Stem — Other")
                ->Check(_type == AUDIOSAMPLETYPE::STEM_OTHER);
            mnuWave.AppendRadioItem(ID_WAVE_MNU_STEM_VOCALS, "Stem — Vocals (ML)")
                ->Check(_type == AUDIOSAMPLETYPE::STEM_VOCALS);
        }
#elif defined(HAVE_OPENVINO) || defined(HAVE_ORT)
        mnuWave.AppendRadioItem(ID_WAVE_MNU_STEM_DRUMS, "Stem — Drums")
            ->Check(_type == AUDIOSAMPLETYPE::STEM_DRUMS);
        mnuWave.AppendRadioItem(ID_WAVE_MNU_STEM_BASS, "Stem — Bass")
            ->Check(_type == AUDIOSAMPLETYPE::STEM_BASS);
        mnuWave.AppendRadioItem(ID_WAVE_MNU_STEM_OTHER, "Stem — Other")
            ->Check(_type == AUDIOSAMPLETYPE::STEM_OTHER);
        mnuWave.AppendRadioItem(ID_WAVE_MNU_STEM_VOCALS, "Stem — Vocals (ML)")
            ->Check(_type == AUDIOSAMPLETYPE::STEM_VOCALS);
#endif
#ifdef __APPLE__
        // Keep the classify entry inside the same radio group so its
        // checkmark moves with `_type == CLASSIFIED` — otherwise the
        // radio defaults to showing "Raw" as selected while the
        // classified filter is active.
        wxString classifyLabel = _selectedSoundClass.empty()
            ? wxString("Classify Audio…")
            : wxString::Format("Classified: %s…", _selectedSoundClass);
        mnuWave.AppendRadioItem(ID_WAVE_MNU_CLASSIFY, classifyLabel)
            ->Check(_type == AUDIOSAMPLETYPE::CLASSIFIED);
#endif
        mnuWave.AppendSeparator();
        mnuWave.AppendCheckItem(ID_WAVE_MNU_DOUBLEHEIGHT, "Double height waveform")->Check(_doubleHeight);
        mnuWave.AppendCheckItem(ID_WAVE_MNU_SHOW_ONSETS, "Show onsets")->Check(_showOnsets);
        mnuWave.AppendCheckItem(ID_WAVE_MNU_SHOW_PITCH, "Show pitch contour")->Check(_showPitchContour);
        mnuWave.AppendCheckItem(ID_WAVE_MNU_SHOW_SPECTROGRAM, "View as spectrogram")->Check(_showSpectrogram);
#ifdef __APPLE__
        if (!_selectedSoundClass.empty()) {
            mnuWave.AppendSeparator();
            mnuWave.Append(ID_WAVE_MNU_CLASSIFY_CLEAR, "Clear sound-class gating");
        }
#endif
    }

    // Audio Track submenu — outside the _media guard so the user can always switch
    // back to Main even when the current track has a broken/missing file (_media == nullptr).
    {
        auto* frame = xLightsApp::GetFrame();
        if (frame != nullptr && frame->CurrentSeqXmlFile != nullptr &&
            (frame->CurrentSeqXmlFile->GetAltTrackCount() > 0 || _activeAudioTrackIndex != 0)) {
            EnsureAudioTrackIds();
            mnuWave.AppendSeparator();
            wxMenu* trackMenu = new wxMenu();
            // Main track
            wxString mainLabel = "Main";
            if (frame->CurrentSeqXmlFile->GetMedia() != nullptr) {
                wxFileName fn(frame->CurrentSeqXmlFile->GetMedia()->FileName());
                mainLabel = "Main  [" + fn.GetFullName() + "]";
            }
            trackMenu->AppendRadioItem(_audioTrackIdPool[0], mainLabel)->Check(_activeAudioTrackIndex == 0);
            // Alt tracks
            int altCount = frame->CurrentSeqXmlFile->GetAltTrackCount();
            for (int i = 0; i < altCount && i + 1 < 32; i++) {
                std::string dispName = frame->CurrentSeqXmlFile->GetAltTrackDisplayName(i);
                wxFileName fn(frame->CurrentSeqXmlFile->GetAltTrack(i).path);
                wxString label = wxString(dispName) + "  [" + fn.GetFullName() + "]";
                trackMenu->AppendRadioItem(_audioTrackIdPool[i + 1], label)->Check(_activeAudioTrackIndex == i + 1);
            }
            mnuWave.AppendSubMenu(trackMenu, "Audio Track");
        }
    }
    if (mnuWave.GetMenuItemCount() > 0) {
        mnuWave.Connect(wxEVT_COMMAND_MENU_SELECTED, (wxObjectEventFunction)& Waveform::OnGridPopup, nullptr, this);
        render();
        PopupMenu(&mnuWave);
    }
}

void Waveform::OnGridPopup(wxCommandEvent& event)
{
    
    int id = event.GetId();
    if(id == ID_WAVE_MNU_RENDER) {
        spdlog::debug("OnGridPopup - ID_WAVE_MNU_RENDER");
        RenderCommandEvent rcEvent("", mTimeline->GetSelectedPositionStartMS(), mTimeline->GetSelectedPositionEndMS(), true, false);
        wxPostEvent(mParent, rcEvent);
    } else if (id == ID_WAVE_MNU_RAW) {
        _type = AUDIOSAMPLETYPE::RAW;
    } else if (id == ID_WAVE_MNU_BASS) {
        _type = AUDIOSAMPLETYPE::BASS;
    } else if (id == ID_WAVE_MNU_TREBLE) {
        _type = AUDIOSAMPLETYPE::TREBLE;
    } else if (id == ID_WAVE_MNU_ALTO) {
        _type = AUDIOSAMPLETYPE::ALTO;
    } else if (id == ID_WAVE_MNU_NONVOCALS) {
        _type = AUDIOSAMPLETYPE::NONVOCALS;
    } else if (id == ID_WAVE_MNU_VOCALS) {
        _type = AUDIOSAMPLETYPE::VOCALS;
    } else if (id == ID_WAVE_MNU_LUFS) {
        _type = AUDIOSAMPLETYPE::LUFS;
    } else if (id == ID_WAVE_MNU_CUSTOM) {
        int origLow = _lowNote;
        int origHigh = _highNote;
        if (_lowNote == -1) _lowNote = 0;
        if (_highNote == -1) _highNote = 127;
        NoteRangeDialog dlg(GetParent(), _lowNote, _highNote);
        if (dlg.ShowModal() == wxID_CANCEL) {
            _lowNote = origLow;
            _highNote = origHigh;
            return;
        }
        _type = AUDIOSAMPLETYPE::CUSTOM;
    } else if (id == ID_WAVE_MNU_DOUBLEHEIGHT) {
        _doubleHeight = !_doubleHeight;
    } else if (id == ID_WAVE_MNU_SHOW_ONSETS) {
        _showOnsets = !_showOnsets;
        if (_showOnsets && !_onsetsComputed && _media != nullptr) {
            wxSetCursor(wxCURSOR_WAIT);
            _onsetMS = DetectOnsets(_media);
            _onsetsComputed = true;
            wxSetCursor(wxCURSOR_ARROW);
        }
        ForceRedraw();
        Refresh();
        return;
    } else if (id == ID_WAVE_MNU_SHOW_SPECTROGRAM) {
        _showSpectrogram = !_showSpectrogram;
        if (_showSpectrogram && !_spectrogramComputed && _media != nullptr) {
            wxSetCursor(wxCURSOR_WAIT);
            _spectrogram = ComputeSpectrogram(_media);
            _spectrogramComputed = _spectrogram.frames > 0;
            wxSetCursor(wxCURSOR_ARROW);
            if (!_spectrogramComputed) _showSpectrogram = false;
        }
        _spectrogramRangeStartMS = -1;
        _spectrogramRangeEndMS = -1;
        ForceRedraw();
        Refresh();
        return;
    } else if (id == ID_WAVE_MNU_SHOW_PITCH) {
        _showPitchContour = !_showPitchContour;
        if (_showPitchContour && !_pitchComputed && _media != nullptr) {
            wxSetCursor(wxCURSOR_WAIT);
            PitchContour c = DetectPitch(_media);
            _pitchContour.clear();
            _pitchContour.reserve(c.samples.size() * 3);
            for (const auto& s : c.samples) {
                _pitchContour.push_back(float(s.timeMS));
                _pitchContour.push_back(s.frequency);
                _pitchContour.push_back(s.confidence);
            }
            _pitchComputed = true;
            wxSetCursor(wxCURSOR_ARROW);
        }
        ForceRedraw();
        Refresh();
        return;
#if defined(__APPLE__) || defined(HAVE_OPENVINO) || defined(HAVE_ORT) 
    } else if (id == ID_WAVE_MNU_STEM_DRUMS ||
               id == ID_WAVE_MNU_STEM_BASS ||
               id == ID_WAVE_MNU_STEM_OTHER ||
               id == ID_WAVE_MNU_STEM_VOCALS) {
        if (_media == nullptr) return;
        if (!_media->HasStemData()) {
            if (!PrepareStemData()) {
                // User cancelled or error — bail without switching.
                ForceRedraw();
                Refresh();
                return;
            }
        }
        AUDIOSAMPLETYPE stem =
            id == ID_WAVE_MNU_STEM_DRUMS  ? AUDIOSAMPLETYPE::STEM_DRUMS :
            id == ID_WAVE_MNU_STEM_BASS   ? AUDIOSAMPLETYPE::STEM_BASS :
            id == ID_WAVE_MNU_STEM_OTHER  ? AUDIOSAMPLETYPE::STEM_OTHER :
                                             AUDIOSAMPLETYPE::STEM_VOCALS;
        _type = stem;
        views.clear();
        mCurrentWaveView = NO_WAVE_VIEW_SELECTED;
        // Fall through to the shared SwitchTo / rebuild path below.
#endif // __APPLE__ || HAVE_OPENVINO || HAVE_ORT
#ifdef __APPLE__
    } else if (id == ID_WAVE_MNU_CLASSIFY) {
        if (_media == nullptr) return;
        if (_soundClasses.empty()) {
            wxSetCursor(wxCURSOR_WAIT);
            SoundClassification r = ClassifySound(_media);
            wxSetCursor(wxCURSOR_ARROW);
            _soundClassTimeStep = r.timeStepSeconds > 0 ? r.timeStepSeconds : 1.0f;
            _soundClasses.clear();
            for (auto& c : r.classes) {
                _soundClasses.emplace_back(c.name, c.confidence);
            }
        }
        if (_soundClasses.empty()) {
            wxMessageBox("No sound classes detected above the confidence threshold.",
                         "Classify Audio", wxOK | wxICON_INFORMATION);
            return;
        }
        wxArrayString names;
        for (const auto& c : _soundClasses) {
            names.Add(wxString::FromUTF8(c.first));
        }
        wxSingleChoiceDialog dlg(this,
            "Pick a sound class to gate the waveform by.\n"
            "Only moments where the class is present will show amplitude.",
            "Classify Audio", names);
        if (dlg.ShowModal() == wxID_OK) {
            _selectedSoundClass = names[dlg.GetSelection()].ToStdString();
            // Push the chosen class's confidence curve down to
            // AudioManager so the CLASSIFIED filter type can gate
            // both display and playback. Then route through the
            // regular filter-switch path so playback follows.
            std::vector<float> curve;
            for (const auto& c : _soundClasses) {
                if (c.first == _selectedSoundClass) { curve = c.second; break; }
            }
            if (_media) {
                _media->SetClassifyGate(_selectedSoundClass, curve, _soundClassTimeStep);
            }
            _type = AUDIOSAMPLETYPE::CLASSIFIED;
            views.clear();
            mCurrentWaveView = NO_WAVE_VIEW_SELECTED;
            // Fall through to the common switch-and-rebuild path so
            // SwitchTo runs.
        } else {
            ForceRedraw();
            Refresh();
            return;
        }
    } else if (id == ID_WAVE_MNU_CLASSIFY_CLEAR) {
        _selectedSoundClass.clear();
        if (_media) {
            _media->SetClassifyGate("", {}, 1.0f);
        }
        _type = AUDIOSAMPLETYPE::RAW;
        views.clear();
        mCurrentWaveView = NO_WAVE_VIEW_SELECTED;
        // Fall through so SwitchTo restores raw playback.
#endif
    } else {
        // Check if this is an audio track selection
        EnsureAudioTrackIds();
        for (int i = 0; i < 32; i++) {
            if (id == _audioTrackIdPool[i]) {
                _activeAudioTrackIndex = i;
                auto* frame = xLightsApp::GetFrame();
                if (frame != nullptr && frame->CurrentSeqXmlFile != nullptr) {
                    wxString err;
                    AudioManager* newMedia = (i == 0)
                        ? frame->CurrentSeqXmlFile->GetMedia()
                        : frame->CurrentSeqXmlFile->GetAltTrackMedia(i - 1);
                    OpenfileMedia(newMedia, err);
                }
                return;
            }
        }
    }

    wxSetCursor(wxCURSOR_WAIT);

    mCurrentWaveView = NO_WAVE_VIEW_SELECTED;
    for (size_t i = 0; i < views.size(); i++) {
        if (views[i].GetZoomLevel() == mZoomLevel && views[i].GetType() == _type) {
            if (_type == AUDIOSAMPLETYPE::CUSTOM) {
                if (views[i].GetLowNote() == _lowNote && views[i].GetHighNote() == _highNote) {
                    mCurrentWaveView = i;
                    break;
                }
            } else {
                mCurrentWaveView = i;
                break;
            }
        }
    }
    if (_media) {
        _media->SwitchTo(_type, _lowNote, _highNote);
    }
    if (mCurrentWaveView == NO_WAVE_VIEW_SELECTED) {
        float samplesPerLine = GetSamplesPerLineFromZoomLevel(mZoomLevel);
        views.emplace_back(mZoomLevel, samplesPerLine, _media, _type, _lowNote, _highNote);
        mCurrentWaveView = views.size() - 1;
    }

    wxSetCursor(wxCURSOR_ARROW);

    ForceRedraw();
    Refresh();
}

void Waveform::SetSelectedInterval(int startMS, int endMS)
{
    mTimeline->SetSelectedPositionStart(mTimeline->GetPositionFromTimeMS(startMS), false);
    mTimeline->SetSelectedPositionEnd(mTimeline->GetPositionFromTimeMS(endMS));
    mTimeline->LatchSelectedPositions();

    wxCommandEvent eventSelected(EVT_WAVE_FORM_HIGHLIGHT);
    eventSelected.SetInt(abs(mTimeline->GetNewStartTimeMS() - mTimeline->GetNewEndTimeMS()));
    wxPostEvent(mParent, eventSelected);

    Refresh(false);
}

void Waveform::mouseMoved(wxMouseEvent& event)
{
    if (!mIsInitialized) { return; }
    if (m_dragging) {
        if (m_drag_mode == DRAG_LEFT_EDGE) {
            mTimeline->SetSelectedPositionStart(event.GetX(), false);
        } else {
            mTimeline->SetSelectedPositionEnd(event.GetX());
        }
        Refresh(false);
        wxCommandEvent eventSelected(EVT_WAVE_FORM_HIGHLIGHT);
        eventSelected.SetInt(abs(mTimeline->GetNewStartTimeMS() - mTimeline->GetNewEndTimeMS()));
        wxPostEvent(mParent, eventSelected);
    } else {
        int selected_x1 = mTimeline->GetSelectedPositionStart();
        int selected_x2 = mTimeline->GetSelectedPositionEnd();
        if (event.GetX() >= selected_x1 && event.GetX() < selected_x1 + 6) {
            SetCursor(wxCURSOR_POINT_LEFT);
            m_drag_mode = DRAG_LEFT_EDGE;
        } else if (event.GetX() > selected_x2 - 6 && event.GetX() <= selected_x2) {
            SetCursor(wxCURSOR_POINT_RIGHT);
            m_drag_mode = DRAG_RIGHT_EDGE;
        } else {
            SetCursor(wxCURSOR_ARROW);
            m_drag_mode = DRAG_NORMAL;
        }
    }
    int mouseTimeMS = mTimeline->GetAbsoluteTimeMSfromPosition(event.GetX());
    UpdateMousePosition(mouseTimeMS);

    // Scrubbing
    if (event.LeftIsDown() && event.ControlDown()) {
        if (_media != nullptr) {
            int msperpixel = std::max(1000 / GetTimeFrequency(), mTimeline->TimePerMajorTickInMS() / mTimeline->PixelsPerMajorTick());
            _media->Play(mouseTimeMS, msperpixel);
        }

        wxCommandEvent eventScrub(EVT_SCRUB);
        eventScrub.SetInt(mouseTimeMS);
        wxPostEvent(mParent, eventScrub);
    }
}

void Waveform::mouseWheelMoved(wxMouseEvent& event)
{
    if(event.CmdDown()) {
        int i = event.GetWheelRotation();
        if (i < 0) {
            wxCommandEvent eventZoom(EVT_ZOOM);
            eventZoom.SetInt(ZOOM_OUT);
            wxPostEvent(mParent, eventZoom);
        } else {
            wxCommandEvent eventZoom(EVT_ZOOM);
            eventZoom.SetInt(ZOOM_IN);
            wxPostEvent(mParent, eventZoom);
        }
    } else if (event.ShiftDown()) {
        int i = event.GetWheelRotation();
        if (i < 0) {
            wxCommandEvent eventScroll(EVT_GSCROLL);
            eventScroll.SetInt(SCROLL_RIGHT);
            wxPostEvent(mParent, eventScroll);
        } else {
            wxCommandEvent eventScroll(EVT_GSCROLL);
            eventScroll.SetInt(SCROLL_LEFT);
            wxPostEvent(mParent, eventScroll);
        }
    } else {
        // Need to process the event directly and not post it
        // If posted, it will be queued to proces later, but then
        // it will get disassociated with the underlying OS
        // level mouse event
        GetParent()->GetEventHandler()->SafelyProcessEvent(event);
    }
}

#if defined(HAVE_OPENVINO) || defined(HAVE_ORT)
// Non-Apple PrepareStemData:
//    ONNX Runtime or OpenVINO
bool Waveform::PrepareStemData()
{
    if (_media == nullptr) return false;
    auto* frame = xLightsApp::GetFrame();
    if (frame == nullptr) return false;

    std::vector<std::string> roots;
    if (!xLightsFrame::CurrentDir.empty())
        roots.push_back(xLightsFrame::CurrentDir.ToStdString());
    for (const auto& m : frame->GetMediaFolders())
        roots.push_back(m);
    auto modelDirs = AIModelStore::CandidateModelDirs(roots);

    // ONNX Runtime / OpenVINO path ─────────────────────────────────────────────────
    std::string modelPath = AIModelStore::FindModel(AIModelStore::kDemucsOnnxModelName, modelDirs);

    if (modelPath.empty()) {
        // No cached model — confirm with user, let them pick install
        // location, download.
        if (roots.empty()) {
            DisplayError("No show or media folders configured — cannot install the stem-separation model.");
            return false;
        }
        wxArrayString choices;
        for (const auto& r : roots) {
            choices.Add(wxString::FromUTF8(r));
        }
        wxSingleChoiceDialog locDlg(this,
                                    "The HTDemucs stem-separation model isn't installed yet.\n"
                                    "Download (~300 MB) and install it to:",
                                    "Install Stem Separation Model", choices);
        if (locDlg.ShowModal() != wxID_OK)
            return false;
        const std::string chosenRoot = roots[locDlg.GetSelection()];
        const std::string destDir = chosenRoot + "\\" + AIModelStore::kModelsSubdir;
        if (!AIModelStore::EnsureDirectory(destDir)) {
            DisplayError("Couldn't create directory: " + destDir);
            return false;
        }

        const std::string local_Path = destDir + "\\" + AIModelStore::kDemucsOnnxModelName;
        {
            wxProgressDialog prog("Downloading Model",
                                    "Downloading HTDemucs stem-separation model…",
                                    1000, this,
                                  wxPD_APP_MODAL | wxPD_AUTO_HIDE | wxPD_CAN_ABORT | wxPD_ELAPSED_TIME | wxPD_REMAINING_TIME);
            std::atomic<bool> dlDone(false);
            std::atomic<bool> dlOk(false);
            std::atomic<int>  dlPct(0);
            std::atomic<bool> dlAbort(false);
            std::thread dlWorker([&] {
                auto cb = [&dlPct, &dlAbort](int pos) -> bool {
                    dlPct.store(pos);
                    return !dlAbort.load();
                };
                dlOk = CurlManager::HTTPSGetFile(AIModelStore::kDemucsOnnxDownloadURL, local_Path, {}, {}, 300, cb);
                dlDone.store(true);
            });
            bool dlCancelled = false;
            while (!dlDone.load()) {
                if (!prog.Update(dlPct.load())) {
                    dlAbort.store(true);
                    dlCancelled = true;
                    break;
                }
                wxMilliSleep(50);
                wxTheApp->Yield(true);
            }
            dlWorker.join();
            if (dlCancelled || !dlOk.load()) {
                if (!dlCancelled)
                    DisplayError("Download failed. Check your internet connection and try again.");
                return false;
            }
        }
        modelPath = local_Path;
        if (modelPath.empty() || !std::filesystem::exists(modelPath)) {
            DisplayError("Model wasn't found anywhere under " + destDir);
            return false;
        }
    }

    StemOutput stems;
    {
        wxProgressDialog prog("Separating Stems",
            "Running HTDemucs — drums, bass, vocals, other…",
            100, this,
                              wxPD_APP_MODAL | wxPD_AUTO_HIDE | wxPD_ELAPSED_TIME | wxPD_REMAINING_TIME | wxPD_CAN_ABORT);
        std::atomic<bool> done(false);
        std::atomic<bool> ok(false);
        std::atomic<int>  pct(0);
        std::thread worker([&] {
            ok = SeparateStems(_media, modelPath, stems,
                               StemSeparatorOptions{},
                               [&pct](int p) { pct.store(p); });
            done.store(true);
        });
        wxSetCursor(wxCURSOR_WAIT);
        while (!done.load()) {
            prog.Update(pct.load());
            wxMilliSleep(50);
            wxTheApp->Yield(true);
        }
        worker.join();
        wxSetCursor(wxCURSOR_ARROW);
        if (!ok) {
            DisplayError("Stem separation failed. Check the log for details.");
            return false;
        }
    }

    _media->SetStemData(
        stems.drumsL, stems.drumsR,
        stems.bassL,  stems.bassR,
        stems.otherL, stems.otherR,
        stems.vocalsL, stems.vocalsR);
    return true;
}
#endif // HAVE_OPENVINO  || HAVE_ORT 

#ifdef __APPLE__
// A8 first-run helper: make sure the HTDemucs model is present
// under one of the configured roots, downloading it (with user
// confirmation) if not, then run the separator and hand the result
// off to `AudioManager::SetStemData`. Returns true if stem data is
// now available.
bool Waveform::PrepareStemData()
{
    if (_media == nullptr) return false;
    // The CoreML model uses Float16 MLMultiArray I/O — macOS 12+.
    // Hitting this on pre-12 only happens if the stem menu items
    // somehow leaked through; surface a clean error either way.
    if (!__builtin_available(macOS 12.0, *)) {
        DisplayError("Stem separation requires macOS 12 or newer.");
        return false;
    }
    auto* frame = xLightsApp::GetFrame();
    if (frame == nullptr) return false;

    // Build the list of candidate install roots: show folder first,
    // then each configured media folder in preference order.
    std::vector<std::string> roots;
    if (!xLightsFrame::CurrentDir.empty()) {
        roots.push_back(xLightsFrame::CurrentDir.ToStdString());
    }
    for (const auto& m : frame->GetMediaFolders()) {
        roots.push_back(m);
    }
    auto modelDirs = AIModelStore::CandidateModelDirs(roots);

    std::string modelPath = AIModelStore::FindModel(
        AIModelStore::kDemucsModelName, modelDirs);

    // Lambda: recursively scan an ai-models dir for the .mlpackage
    // wherever the zip nested it, then lift it up to
    // `<dir>/<modelName>` so subsequent shallow FindModel lookups
    // succeed. Returns the normalised path, or empty on miss.
    auto liftNestedModel = [](const std::string& rootDir) -> std::string {
        std::error_code ec;
        if (!std::filesystem::exists(rootDir, ec)) return {};
        std::string found;
        for (auto& entry : std::filesystem::recursive_directory_iterator(rootDir, ec)) {
            if (ec) break;
            if (entry.is_directory(ec) &&
                entry.path().filename().string() == AIModelStore::kDemucsModelName) {
                found = entry.path().string();
                break;
            }
        }
        if (found.empty()) return {};
        std::string target = rootDir + "/" + AIModelStore::kDemucsModelName;
        if (found == target) return target;
        std::filesystem::rename(found, target, ec);
        if (ec) return {};
        // Clean up emptied intermediate dirs up to rootDir.
        std::filesystem::path p(found);
        while (p.has_parent_path()) {
            p = p.parent_path();
            if (p.string() == rootDir) break;
            std::error_code rmErr;
            if (!std::filesystem::remove(p, rmErr)) break;
        }
        return target;
    };

    // Check for nested-but-already-extracted installs before prompting
    // for a download — handles the john-rocky zip layout where the
    // .mlpackage lives inside `creative_apps/DemucsDemo/DemucsDemo/`.
    if (modelPath.empty()) {
        for (const auto& d : modelDirs) {
            modelPath = liftNestedModel(d);
            if (!modelPath.empty()) break;
        }
    }

    if (modelPath.empty()) {
        // No cached model — confirm with user, let them pick install
        // location, download + unzip.
        if (roots.empty()) {
            DisplayError("No show or media folders configured — cannot install the stem-separation model.");
            return false;
        }
        wxArrayString choices;
        for (const auto& r : roots) {
            choices.Add(wxString::FromUTF8(r));
        }
        wxSingleChoiceDialog locDlg(this,
            "The HTDemucs stem-separation model isn't installed yet.\n"
            "Download (~65 MB zip, ~180 MB expanded) and install it to:",
            "Install Stem Separation Model", choices);
        if (locDlg.ShowModal() != wxID_OK) return false;
        const std::string chosenRoot = roots[locDlg.GetSelection()];
        const std::string destDir = chosenRoot + "/" + AIModelStore::kModelsSubdir;
        if (!AIModelStore::EnsureDirectory(destDir)) {
            DisplayError("Couldn't create directory: " + destDir);
            return false;
        }

        // Download the zip.
        const std::string zipPath = destDir + "/HTDemucs_SourceSeparation_F32.mlpackage.zip";
        {
            wxProgressDialog prog("Downloading Model",
                "Fetching HTDemucs stem-separation model…",
                100, this,
                wxPD_APP_MODAL | wxPD_AUTO_HIDE | wxPD_CAN_ABORT);
            if (!CurlManager::HTTPSGetFile(AIModelStore::kDemucsDownloadURL, zipPath)) {
                DisplayError("Download failed. Check your internet connection and try again.");
                return false;
            }
        }

        // Unzip preserving directory structure (`.mlpackage` is a
        // directory bundle). Entries are written relative to destDir.
        {
            wxProgressDialog prog("Installing Model",
                "Extracting the stem-separation model…",
                100, this, wxPD_APP_MODAL | wxPD_AUTO_HIDE);
            wxFileInputStream fin(zipPath);
            if (!fin.IsOk()) {
                DisplayError("Couldn't open downloaded zip.");
                return false;
            }
            wxZipInputStream zin(fin);
            while (wxZipEntry* ent = zin.GetNextEntry()) {
                wxString entryName = ent->GetName();
                wxString outPath = wxString::FromUTF8(destDir) + "/" + entryName;
                if (ent->IsDir()) {
                    ::wxMkdir(outPath, wxS_DIR_DEFAULT);
                } else {
                    wxFileName(outPath).Mkdir(wxS_DIR_DEFAULT, wxPATH_MKDIR_FULL);
                    wxFileOutputStream fout(outPath);
                    if (fout.IsOk()) {
                        zin.Read(fout);
                    }
                }
                delete ent;
            }
        }
        ::wxRemoveFile(wxString::FromUTF8(zipPath));

        // The john-rocky release zip nests the model inside
        // `creative_apps/DemucsDemo/DemucsDemo/`. Walk the extracted
        // tree, find the `.mlpackage`, and lift it to
        // `<ai-models>/HTDemucs_SourceSeparation_F32.mlpackage` so
        // `FindModel` (shallow scan) will hit it on the next run.
        modelPath = liftNestedModel(destDir);
        if (modelPath.empty()) {
            DisplayError("Unzip finished but the model wasn't found anywhere under " + destDir);
            return false;
        }
    }

    // Run the separator — this is slow (tens of seconds to a few
    // minutes depending on hardware + track length). CoreML warns
    // when called from the main thread ("may lead to UI
    // unresponsiveness"), so we offload to a std::thread and pump
    // wxWidgets events from main while polling for completion. The
    // progress callback updates an atomic; the main loop reads it
    // and forwards to wxProgressDialog.
    StemOutput stems;
    {
        wxProgressDialog prog("Separating Stems",
            "Running HTDemucs — drums, bass, vocals, other…",
            100, this,
            wxPD_APP_MODAL | wxPD_AUTO_HIDE | wxPD_ELAPSED_TIME | wxPD_CAN_ABORT);
        std::atomic<bool> done(false);
        std::atomic<bool> ok(false);
        std::atomic<bool> cancelRequested(false);
        std::atomic<int> pct(0);
        std::thread worker([&]{
            ok = SeparateStems(_media, modelPath, stems,
                               StemSeparatorOptions{},
                               [&pct, &cancelRequested](int p) {
                                   pct.store(p);
                                   // SeparateStems honours false as
                                   // "cancel"; void-returning
                                   // callback doesn't, so the worker
                                   // always runs to completion. Good
                                   // enough — inference for a 3-min
                                   // track is ≤ a few minutes on
                                   // Apple Silicon.
                               });
            done.store(true);
        });
        wxSetCursor(wxCURSOR_WAIT);
        while (!done.load()) {
            if (!prog.Update(pct.load())) {
                cancelRequested.store(true);
                // CoreML predictions can't be interrupted mid-chunk,
                // but we fall out of the wait loop once the in-flight
                // chunk returns, then discard the result below.
            }
            wxMilliSleep(50);
            wxTheApp->Yield(true);
        }
        worker.join();
        wxSetCursor(wxCURSOR_ARROW);
        if (!ok.load()) {
            DisplayError("Stem separation failed. See log for details.");
            return false;
        }
        if (cancelRequested.load()) return false;
    }

    _media->SetStemData(
        stems.drumsL, stems.drumsR,
        stems.bassL,  stems.bassR,
        stems.otherL, stems.otherR,
        stems.vocalsL, stems.vocalsR);
    return true;
}
#endif

// Open Media file and return elapsed time in milliseconds
int Waveform::OpenfileMedia(AudioManager* media, wxString& error)
{
    _type = AUDIOSAMPLETYPE::RAW;
    _lowNote = -1;
    _highNote = -1;
    _media = media;
    views.clear();
    ResetAnalysisState();
	if (_media != nullptr) {
        _media->SwitchTo(_type);
		float samplesPerLine = GetSamplesPerLineFromZoomLevel(mZoomLevel);
		views.emplace_back(mZoomLevel, samplesPerLine, media, _type, _lowNote, _highNote);
		mCurrentWaveView = 0;
		return media->LengthMS();
    } else {
        mCurrentWaveView = NO_WAVE_VIEW_SELECTED;
        SetZoomLevel(GetZoomLevel());
	    return 0;
	}
}


xlColor Waveform::ClearBackgroundColor() const {
    if (AudioManager::GetAudioManager()->IsNoAudio()) {
        return xlRED;
    }
    return ColorManager::instance()->GetColor(ColorManager::COLOR_WAVEFORM_BACKGROUND);
}

void Waveform::Paint(wxPaintEvent& event)
{
    wxPaintDC(this);
    render();
}

void Waveform::render()
{
    if(!IsShownOnScreen()) return;
    if(!mIsInitialized) {
        PrepareCanvas();
        SetZoomLevel(mZoomLevel);
    }

    xlGraphicsContext *ctx = PrepareContextForDrawing();
    if (ctx == nullptr) {
        return;
    }
    ctx->SetViewport(0, 0, mWindowWidth, mWindowHeight);

    if (mCurrentWaveView >= 0) {
		DrawWaveView(ctx, views[mCurrentWaveView]);
	}

    FinishDrawing(ctx);
}

float Waveform::translateOffset(float f) {
    if (drawingUsingLogicalSize()) {
        return f;
    }
    return translateToBacking(f);
}


void Waveform::ForceRedraw()
{
    if (mCurrentWaveView >= 0) {
        views[mCurrentWaveView].ForceRedraw();
    }
}

double DoubleHeight(double v, bool dh, int ht)
{
    if (dh) {
        return (v) * (double)ht;
    }
    return v * ((double)ht / 2.0);
}

void Waveform::DrawWaveView(xlGraphicsContext* ctx, const WaveView& wv)
{
    if (!border) {
        border = ctx->createVertexAccumulator();
        border->PreAlloc(5);
        border->AddVertex(0.25, 0.25, 0);
        border->AddVertex(mWindowWidth - 0.5, 0.25, 0);
        border->AddVertex(mWindowWidth - 0.5, mWindowHeight - 0.5, 0);
        border->AddVertex(0.25, mWindowHeight - 0.5, 0);
        border->AddVertex(0.25, 0.25, 0);
        border->Finalize(true);
    } else if (mWindowResized) {
        border->SetVertex(1, mWindowWidth - 0.5, 0.25, 0);
        border->SetVertex(2, mWindowWidth - 0.5, mWindowHeight - 0.5, 0);
        border->SetVertex(3, 0.25, mWindowHeight - 0.5, 0);
        border->FlushRange(1, 3);
    }
    xlColor color;
    color.Set(128, 128, 128);
    // Draw Outside rectangle
    ctx->drawLineStrip(border, color);

    // Get selection positions from timeline
    int selected_x1 = mTimeline->GetSelectedPositionStart();
    int selected_x2 = mTimeline->GetSelectedPositionEnd();

    // draw shaded region if needed
    if (selected_x1 != -1 && selected_x2 != -1) {
        float x1 = translateOffset(selected_x1);
        float x2 = translateOffset(selected_x2);
        color = xLightsApp::GetFrame()->color_mgr.GetColor(ColorManager::COLOR_WAVEFORM_SELECTED);
        color.SetAlpha(45);
        xlVertexAccumulator* selection = ctx->createVertexAccumulator();
        selection->PreAlloc(4);
        selection->AddVertex(x1, 1, 0);
        selection->AddVertex(x2, 1, 0);
        selection->AddVertex(x1, mWindowHeight - 1, 0);
        selection->AddVertex(x2, mWindowHeight - 1, 0);
        ctx->enableBlending();
        ctx->drawTriangleStrip(selection, color);
        ctx->disableBlending();
        delete selection;
    }

    int max_wave_ht = mWindowHeight - VERTICAL_PADDING;

    // A6: spectrogram view — render the cached STFT as a texture
    // covering the waveform strip instead of the peak polygons.
    // Re-upload the BGRA image whenever the visible time range or
    // the strip size changes. Overlays (onsets / pitch) still draw
    // on top further down in this function.
    bool spectrogramDrawn = false;
    if (_showSpectrogram && _spectrogramComputed && _media != nullptr && mTimeline != nullptr) {
        int texW = std::max(1, int(mWindowWidth));
        int texH = std::max(1, int(mWindowHeight));
        // Timeline position math is in LOGICAL pixels, but on Metal
        // `mWindowWidth` is in BACKING pixels (2x on retina). Convert
        // the right edge back to logical before feeding it into
        // `GetTimeMSfromPosition`, otherwise we ask for a time twice
        // as far into the track as the viewport actually shows and
        // the right half of the spectrogram ends up pointing at
        // frames past the end (= silent = black).
        int logicalWidth = drawingUsingLogicalSize()
            ? mWindowWidth
            : int(GetSize().GetWidth());
        long visStartMS = long(mTimeline->GetTimeMSfromPosition(mStartPixelOffset));
        long visEndMS = long(mTimeline->GetTimeMSfromPosition(mStartPixelOffset + logicalWidth));
        if (visEndMS > visStartMS && texW > 0 && texH > 0) {
            bool rebuild = (_spectrogramTexture == nullptr) ||
                           texW != _spectrogramTexW || texH != _spectrogramTexH ||
                           visStartMS != _spectrogramRangeStartMS ||
                           visEndMS != _spectrogramRangeEndMS;
            if (rebuild) {
                if (_spectrogramTexture == nullptr ||
                    texW != _spectrogramTexW || texH != _spectrogramTexH) {
                    if (_spectrogramTexture) {
                        delete _spectrogramTexture;
                        _spectrogramTexture = nullptr;
                    }
                    _spectrogramTexture = ctx->createTexture(texW, texH, true, true);
                    _spectrogramTexture->SetName("WaveSpectrogram");
                    _spectrogramTexW = texW;
                    _spectrogramTexH = texH;
                }
                std::vector<uint8_t> buf;
                RenderSpectrogramBGRA(_spectrogram, visStartMS, visEndMS, texW, texH, buf);
                _spectrogramTexture->UpdateData(buf.data(), true, true);
                _spectrogramRangeStartMS = visStartMS;
                _spectrogramRangeEndMS = visEndMS;
            }
            ctx->drawTexture(_spectrogramTexture, 0, 0, mWindowWidth, mWindowHeight);
            spectrogramDrawn = true;
        }
    }

    if (_media != nullptr && !spectrogramDrawn) {
        xlColor c(130, 178, 207, 255);
        if (xLightsApp::GetFrame() != nullptr) {
            c = xLightsApp::GetFrame()->color_mgr.GetColor(ColorManager::COLOR_WAVEFORM);
        }

        int max = std::min(mWindowWidth, wv.MinMaxs.size());
        if (mStartPixelOffset != wv.lastRenderStart || max != wv.lastRenderSize || _doubleHeight != wv._doubleHeight) {
            float pixelOffset = translateOffset(mStartPixelOffset);

            if (wv.background.get() == nullptr) {
                wv.background = std::unique_ptr<xlVertexAccumulator>(ctx->createVertexAccumulator()->SetName("WaveFill"));
                wv.outline = std::unique_ptr<xlVertexAccumulator>(ctx->createVertexAccumulator()->SetName("WaveLines"));
                wv.rmsFill = std::unique_ptr<xlVertexAccumulator>(ctx->createVertexAccumulator()->SetName("WaveRMS"));
            }
            wv.background->Reset();
            wv.outline->Reset();
            wv.rmsFill->Reset();
            wv.background->PreAlloc((mWindowWidth + 2) * 2);
            wv.outline->PreAlloc((mWindowWidth + 2) + 4);
            wv.rmsFill->PreAlloc((mWindowWidth + 2) * 2);

            std::vector<double> vertexes;
            vertexes.resize((mWindowWidth + 2));

            for (size_t x = 0; x < mWindowWidth && x < wv.MinMaxs.size(); x++) {
                int index = x;
                index += pixelOffset;
                if (index >= 0 && index < (int)wv.MinMaxs.size()) {
                    double y1 = DoubleHeight(wv.MinMaxs[index].min, _doubleHeight, max_wave_ht) + (mWindowHeight / 2);
                    double y2 = DoubleHeight(wv.MinMaxs[index].max, _doubleHeight, max_wave_ht) + (mWindowHeight / 2);

                    wv.background->AddVertex(x, y1);
                    wv.background->AddVertex(x, y2);

                    // A10: RMS band centred on the axis, mirrored ±rms.
                    double r = wv.MinMaxs[index].rms;
                    double ry1 = DoubleHeight(-r, _doubleHeight, max_wave_ht) + (mWindowHeight / 2);
                    double ry2 = DoubleHeight( r, _doubleHeight, max_wave_ht) + (mWindowHeight / 2);
                    wv.rmsFill->AddVertex(x, ry1);
                    wv.rmsFill->AddVertex(x, ry2);

                    wv.outline->AddVertex(x, y1);
                    vertexes[x] = y2;
                }
            }
            for (int x = std::min(mWindowWidth, wv.MinMaxs.size()) - 1; x >= 0; x--) {
                int index = x;
                index += pixelOffset;
                if (index >= 0 && index < (int)wv.MinMaxs.size()) {
                    wv.outline->AddVertex(x, vertexes[x]);
                }
            }
            wv._doubleHeight = _doubleHeight;
            wv.lastRenderSize = max;
            wv.lastRenderStart = mStartPixelOffset;
            wv.background->FlushRange(0, wv.background->getCount());
            wv.outline->FlushRange(0, wv.outline->getCount());
            wv.rmsFill->FlushRange(0, wv.rmsFill->getCount());
        }
        if (wv.background.get() && wv.background->getCount()) {
            ctx->drawTriangleStrip(wv.background.get(), c);
        }
        if (wv.rmsFill.get() && wv.rmsFill->getCount()) {
            // Lighter tint on top of the peak fill — makes the
            // "average energy" band read independently of transients.
            xlColor rmsC(c.red + (255 - c.red) * 3 / 5,
                         c.green + (255 - c.green) * 3 / 5,
                         c.blue + (255 - c.blue) * 3 / 5,
                         220);
            ctx->enableBlending();
            ctx->drawTriangleStrip(wv.rmsFill.get(), rmsC);
            ctx->disableBlending();
        }
        if (wv.outline.get() && wv.outline->getCount()) {
            ctx->drawLineStrip(wv.outline.get(), xlWHITE);
        }
    }

    // A2 onset overlay — faint amber verticals across the waveform
    // strip at each detected onset position.
    if (_showOnsets && !_onsetMS.empty() && mTimeline != nullptr) {
        xlVertexColorAccumulator* onsets = ctx->createVertexColorAccumulator();
        xlColor onsetColor(255, 170, 60, 200);
        for (long ms : _onsetMS) {
            int px = mTimeline->GetPositionFromTimeMS(int(ms));
            if (px < 0 || px > mWindowWidth) continue;
            float f = translateOffset(px);
            onsets->AddVertex(f, 1, 0, onsetColor);
            onsets->AddVertex(f, mWindowHeight - 1, 0, onsetColor);
        }
        if (onsets->getCount() > 0) {
            ctx->enableBlending();
            ctx->drawLines(onsets);
            ctx->disableBlending();
        }
        delete onsets;
    }

    // A5 pitch contour overlay. Line segments colour-coded by pitch
    // class (12 hues around the wheel) with frequency mapped to a log
    // y-scale. Unvoiced frames (freq=0) break the polyline so silence
    // doesn't produce a spurious slope.
    if (_showPitchContour && _pitchContour.size() >= 6 && mTimeline != nullptr) {
        // HSL → RGB helper for colour-by-note.
        auto hueToRGB = [](float h, float s, float l, uint8_t& R, uint8_t& G, uint8_t& B) {
            auto hue2rgb = [](float p, float q, float t) {
                if (t < 0) t += 1;
                if (t > 1) t -= 1;
                if (t < 1.0f / 6) return p + (q - p) * 6 * t;
                if (t < 0.5f)     return q;
                if (t < 2.0f / 3) return p + (q - p) * (2.0f / 3 - t) * 6;
                return p;
            };
            float q = l < 0.5f ? l * (1 + s) : l + s - l * s;
            float p = 2 * l - q;
            R = uint8_t(255 * hue2rgb(p, q, h + 1.0f / 3));
            G = uint8_t(255 * hue2rgb(p, q, h));
            B = uint8_t(255 * hue2rgb(p, q, h - 1.0f / 3));
        };
        // Range choice: detector runs 75–1200 Hz, display adds a bit
        // of headroom (60–1800 Hz) so pitches near the detector's
        // upper bound don't pin the line to the top of the strip.
        const float logMin = std::log2(60.0f);
        const float logMax = std::log2(1800.0f);
        const float logRange = logMax - logMin;
        const float yTop = 2.0f;
        const float yBottom = float(mWindowHeight) - 2.0f;
        const float yRange = yBottom - yTop;
        // Draw each segment as a thickened ribbon by stacking
        // parallel line pairs offset in y by ±1 backing-pixel. On
        // retina that's ~3 physical pixels of stroke, visible at
        // normal zoom.
        const float thicknessStep = translateOffset(1.0f) - translateOffset(0.0f);
        const int thicknessLines = 3; // centre + one above + one below

        xlVertexColorAccumulator* pitch = ctx->createVertexColorAccumulator();
        const int triples = int(_pitchContour.size() / 3);
        bool havePrev = false;
        float prevX = 0, prevY = 0;
        uint8_t prevR = 0, prevG = 0, prevB = 0;
        for (int i = 0; i < triples; i++) {
            float t = _pitchContour[i * 3];
            float freq = _pitchContour[i * 3 + 1];
            int px = mTimeline->GetPositionFromTimeMS(int(t));
            if (px < -8 || px > mWindowWidth + 8) { havePrev = false; continue; }
            if (freq <= 0) { havePrev = false; continue; }
            float x = translateOffset(px);
            float logF = std::log2(freq);
            float norm = (logF - logMin) / logRange;
            if (norm < 0) norm = 0;
            if (norm > 1) norm = 1;
            float y = yBottom - norm * yRange;
            float semitones = 12.0f * (logF - std::log2(440.0f));
            int noteClass = ((int(std::round(semitones)) % 12) + 12) % 12;
            float hue = float(noteClass) / 12.0f;
            uint8_t R, G, B;
            hueToRGB(hue, 0.7f, 0.55f, R, G, B);
            xlColor col(R, G, B, 240);
            if (havePrev) {
                xlColor prevCol(prevR, prevG, prevB, 240);
                for (int k = -(thicknessLines / 2); k <= thicknessLines / 2; k++) {
                    float offset = float(k) * thicknessStep;
                    pitch->AddVertex(prevX, prevY + offset, 0, prevCol);
                    pitch->AddVertex(x, y + offset, 0, col);
                }
            }
            prevX = x; prevY = y; prevR = R; prevG = G; prevB = B;
            havePrev = true;
        }
        if (pitch->getCount() > 0) {
            ctx->enableBlending();
            ctx->drawLines(pitch);
            ctx->disableBlending();
        }
        delete pitch;
    }

    xlVertexColorAccumulator* vac = ctx->createVertexColorAccumulator();
    // draw selection line if not a range
    if (selected_x1 != -1 && selected_x2 == -1) {
        color.Set(0, 0, 0, 128);
        float f = translateOffset(selected_x1);
        vac->AddVertex(f, 1, 0, color);
        vac->AddVertex(f, mWindowHeight - 1, 0, color);
    }

    // draw mouse position line
    int mouse_marker = mTimeline->GetMousePosition();

    if (xLightsApp::GetFrame() != nullptr && xLightsApp::GetFrame()->_timelineZooming == 1) { // 1 = Mouse Position, 0 = Play Marker
        
        //
        // preference : use waveform mouse marker for centered zoom 
        //
        int aSelectedPlayMarkerStartMS = mTimeline->GetAbsoluteTimeMSfromPosition(mouse_marker);

        mTimeline->SetZoomMarkerMS(aSelectedPlayMarkerStartMS);

    }

    if (mouse_marker != -1) {
        color.Set(0, 0, 255, 255);
        
        if (xLightsApp::GetFrame() != nullptr) {
            color = xLightsApp::GetFrame()->color_mgr.GetColor(ColorManager::COLOR_WAVEFORM_MOUSE_MARKER);
        }        

        float f = translateOffset(mouse_marker);
        vac->AddVertex(f, 1, 0, color);
        vac->AddVertex(f, mWindowHeight - 1, 0, color);
    }

    // draw play marker line
    int play_marker = mTimeline->GetPlayMarker();
    if (play_marker != -1) {
        color.Set(0, 0, 0, 255);
        float f = translateOffset(play_marker);
        vac->AddVertex(f, 1, 0, color);
        vac->AddVertex(f, mWindowHeight - 1, 0, color);
    }

    if (xLightsApp::GetFrame() != nullptr) {
        Effect* selectedEffect = xLightsApp::GetFrame()->GetMainSequencer()->GetSelectedEffect();
        if (selectedEffect != nullptr) {
            color = ColorManager::instance()->GetColor(ColorManager::COLOR_WAVEFORM_SELECTEDEFFECT);
            int start = translateOffset(mTimeline->GetPositionFromTimeMS(selectedEffect->GetStartTimeMS()));
            int end = translateOffset(mTimeline->GetPositionFromTimeMS(selectedEffect->GetEndTimeMS()));
            vac->AddVertex(start, 1, 0, color);
            vac->AddVertex(start, (mWindowHeight - 1) / 4, 0, color);
            vac->AddVertex(end, 1, 0, color);
            vac->AddVertex(end, (mWindowHeight - 1) / 4, 0, color);
            vac->AddVertex(start, (mWindowHeight - 1) / 8, 0, color);
            vac->AddVertex(end, (mWindowHeight - 1) / 8, 0, color);
        }
    }
    if (vac->getCount() > 0) {
        ctx->drawLines(vac);
    }
    delete vac;
    mWindowResized = false;
}

void Waveform::SetZoomLevel(int level)
{
    mZoomLevel = level;

    if (!mIsInitialized) return;

    mCurrentWaveView = NO_WAVE_VIEW_SELECTED;
    for (size_t i = 0; i < views.size(); i++) {
        if (views[i].GetZoomLevel() == mZoomLevel && views[i].GetType() == _type) {
            mCurrentWaveView = i;
        }
    }
    if (mCurrentWaveView == NO_WAVE_VIEW_SELECTED) {
        float samplesPerLine = GetSamplesPerLineFromZoomLevel(mZoomLevel);
        views.emplace_back(mZoomLevel, samplesPerLine, _media, _type, _lowNote, _highNote);
        mCurrentWaveView = views.size() - 1;
    }
}

int Waveform::GetZoomLevel() const
{
    return  mZoomLevel;
}

int Waveform::SetStartPixelOffset(int offset)
{
    if (mCurrentWaveView != NO_WAVE_VIEW_SELECTED) {
       mStartPixelOffset = offset;
    } else {
        mStartPixelOffset = 0;
    }
    return mStartPixelOffset;
}

int Waveform::GetStartPixelOffset() const
{
    return  mStartPixelOffset;
}

void Waveform::SetTimeFrequency(int frequency)
{
    mFrequency = frequency;
}

int Waveform::GetTimeFrequency() const
{
    return  mFrequency;
}

float Waveform::GetSamplesPerLineFromZoomLevel(int ZoomLevel) const
{
    // The number of periods for each Zoomlevel is held in ZoomLevelValues array
    int periodsPerMajorHash = TimeLine::ZoomLevelValues[mZoomLevel];
    float timePerPixel = ((float)periodsPerMajorHash/(float)mFrequency)/(float)PIXELS_PER_MAJOR_HASH;
    if (!drawingUsingLogicalSize()) {
        timePerPixel /= GetContentScaleFactor();
    }
	if (_media != nullptr) {
		return (float)timePerPixel * (float)_media->GetRate();
    } else {
		return 0.0f;
	}
}

void Waveform::SetWaveFormSize(int h)
{
    int w = -1;
    SetMinSize(wxSize(w, h));

    wxSize size = GetSize();
    if (w == -1) w = size.GetWidth();
    if (h == -1) h = size.GetHeight();

    SetSize(w, h);
    mWindowHeight = h;
    mWindowWidth = w;
    mWindowResized = true;

    // All of our existing views are invalid ... so erase them
    views.clear();

    if (_media != nullptr) {
        float samplesPerLine = GetSamplesPerLineFromZoomLevel(mZoomLevel);
        views.emplace_back(0, samplesPerLine, _media, _type, _lowNote, _highNote);
    }

    mCurrentWaveView = NO_WAVE_VIEW_SELECTED;
    SetZoomLevel(mZoomLevel);

    Refresh(false);
}

Waveform::WaveView::~WaveView() {
}

void Waveform::WaveView::SetMinMaxSampleSet(float SamplesPerPixel, AudioManager* media, AUDIOSAMPLETYPE type, int lowNote, int highNote)
{
	MinMaxs.clear();

	if (media != nullptr) {
		float minimum=1;
		float maximum=-1;
		long trackSize = media->GetTrackSize();
		int totalMinMaxs = (int)((float)trackSize/SamplesPerPixel)+1;

		for (int i = 0; i < totalMinMaxs; i++) {
			// Use float calculation to minimize compounded rounding of position
			long start = (long)((float)i*SamplesPerPixel);
			if (start >= trackSize) {
				return;
			}
			long end = start + SamplesPerPixel;
			if (end >= trackSize) {
				end = trackSize;
			}
			minimum = 1;
			maximum = -1;
            float rms = 0.0f;
            media->GetLeftDataMinMax(start, end, minimum, maximum, type, lowNote, highNote, &rms);
			MINMAX mm;
			mm.min = minimum;
			mm.max = maximum;
			mm.rms = rms;
			MinMaxs.push_back(mm);
		}
    }
}

void Waveform::mouseLeftWindow(wxMouseEvent& event)
{
    UpdateMousePosition(-1);
}

void Waveform::UpdateMousePosition(int time)
{
    // Update time selection
    wxCommandEvent eventMousePos(EVT_MOUSE_POSITION);
    eventMousePos.SetInt(time);
    wxPostEvent(mParent, eventMousePos);
}
