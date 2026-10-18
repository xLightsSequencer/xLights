/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include "MovingHeadSongDialog.h"

#include <algorithm>
#include <cmath>
#include <set>

#include <wx/msgdlg.h>
#include <wx/settings.h>
#include <wx/stattext.h>

#include "../xLightsMain.h"
#include "effects/MovingHeadSongGenerator.h"
#include "models/DMX/DmxMotor.h"
#include "models/DMX/DmxMovingHeadComm.h"
#include "models/Model.h"
#include "models/ModelManager.h"
#include "render/Effect.h"
#include "render/EffectLayer.h"
#include "render/Element.h"
#include "render/SequenceElements.h"
#include "MainSequencer.h"
#include "RenderCommandEvent.h"
#include "settings/XLightsConfigAdapter.h"

namespace {
bool ContainsNoCase(const std::string& s, const char* needle) {
    return wxString(s).Lower().Contains(needle);
}
}

MovingHeadSongDialog::MovingHeadSongDialog(wxWindow* parent, SequenceElements& elements, ModelManager& models) :
    wxDialog(parent, wxID_ANY, _("Generate Moving Head Effects"), wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER),
    _elements(elements) {
    auto* top = new wxBoxSizer(wxVERTICAL);

    auto* intro = new wxStaticText(this, wxID_ANY, _("Adds sparse white Moving Head effects (sweeps on section changes, ping-pong on chorus beats, an outro) driven by your bars and sections timing tracks. Existing effects on the selected heads are removed."));
    intro->Wrap(540);
    top->Add(intro, 0, wxALL, 8);

    auto* tips = new wxStaticText(this, wxID_ANY,
                                  _("Tips: create the timing tracks first with the audio analysis (Vamp) plugins.\n"
                                    "Bars: QM Bar and Beat Tracker, Bars output. Check it follows the music - beats are assumed to be 4 per bar.\n"
                                    "Sections: QM Segmenter. Its labels (1, 2, 3...) only group similar-sounding sections. Listen, then tick Chorus on the label that repeats as the chorus and Peak on the loudest ones. Lowering its number of segment types (e.g. 4-5) gives fewer labels to choose from."));
    tips->Wrap(540);
    tips->SetForegroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_GRAYTEXT));
    top->Add(tips, 0, wxLEFT | wxRIGHT | wxBOTTOM, 8);

    auto* grid = new wxFlexGridSizer(2, 5, 5);
    grid->AddGrowableCol(1);

    wxArrayString timings;
    int barsSel = 0;
    int sectionsSel = 0; // 0 = none
    wxArrayString sectionChoices;
    sectionChoices.Add(_("(none)"));
    for (int i = 0; i < _elements.GetNumberOfTimingElements(); ++i) {
        TimingElement* t = _elements.GetTimingElement(i);
        if (t == nullptr) {
            continue;
        }
        timings.Add(t->GetName());
        sectionChoices.Add(t->GetName());
        if (barsSel == 0 && ContainsNoCase(t->GetName(), "bar")) {
            barsSel = (int)timings.size() - 1;
        }
        if (sectionsSel == 0 && (ContainsNoCase(t->GetName(), "segment") || ContainsNoCase(t->GetName(), "section"))) {
            sectionsSel = (int)sectionChoices.size() - 1;
        }
    }

    grid->Add(new wxStaticText(this, wxID_ANY, _("Bars timing track:")), 0, wxALIGN_CENTER_VERTICAL);
    _bars = new wxChoice(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, timings);
    if (!timings.empty()) {
        _bars->SetSelection(barsSel);
    }
    _bars->SetToolTip(_("One mark per bar, e.g. QM Bar and Beat Tracker (Bars output)."));
    grid->Add(_bars, 1, wxEXPAND);

    grid->Add(new wxStaticText(this, wxID_ANY, _("Sections timing track:")), 0, wxALIGN_CENTER_VERTICAL);
    _sections = new wxChoice(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, sectionChoices);
    _sections->SetSelection(sectionsSel);
    _sections->SetToolTip(_("Optional. Section boundaries with labels, e.g. QM Segmenter. Without it every bar is treated as a verse bar."));
    grid->Add(_sections, 1, wxEXPAND);

    grid->Add(new wxStaticText(this, wxID_ANY, _("Move-into-position time (ms):")), 0, wxALIGN_CENTER_VERTICAL);
    _leadIn = new wxSpinCtrl(this, wxID_ANY, "1000", wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 0, 5000, 1000);
    _leadIn->SetToolTip(_("Dark time before each beam for the head to reach its position. When the gap is shorter, the end of the previous beam is trimmed (never below 40% of its length) to make room."));
    grid->Add(_leadIn, 0);
    grid->AddSpacer(0);
    _useSlew = new wxCheckBox(this, wxID_ANY, _("Size the move time from each head's tilt slew limit"));
    _useSlew->SetValue(true);
    _useSlew->SetToolTip(_("Uses degrees-per-second from the model's tilt motor. Heads with no limit set (0) use the time above."));
    grid->Add(_useSlew, 0);
    grid->Add(new wxStaticText(this, wxID_ANY, _("Chorus flash pattern:")), 0, wxALIGN_CENTER_VERTICAL);
    wxArrayString pairings;
    pairings.Add(_("Alternate odd / even heads"));
    pairings.Add(_("Left half, then right half"));
    _pairing = new wxChoice(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, pairings);
    _pairing->SetSelection(0);
    grid->Add(_pairing, 1, wxEXPAND);
    grid->Add(new wxStaticText(this, wxID_ANY, _("Peak burst every N bars:")), 0, wxALIGN_CENTER_VERTICAL);
    _burstEvery = new wxSpinCtrl(this, wxID_ANY, "4", wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 1, 32, 4);
    _burstEvery->SetToolTip(_("In sections with Peak ticked, the all-heads burst plays on the bar after the section starts (which opens with a sweep) and then every N bars. 1 = every bar."));
    grid->Add(_burstEvery, 0);
    grid->Add(new wxStaticText(this, wxID_ANY, _("Chorus flash rhythm:")), 0, wxALIGN_CENTER_VERTICAL);
    wxArrayString rhythms;
    rhythms.Add(_("Every beat (busy)"));
    rhythms.Add(_("Every 2 beats"));
    rhythms.Add(_("Every bar (sparse)"));
    _rhythm = new wxChoice(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, rhythms);
    _rhythm->SetSelection(1);
    grid->Add(_rhythm, 1, wxEXPAND);
    top->Add(grid, 0, wxEXPAND | wxALL, 8);

    auto* labelHelp = new wxStaticText(this, wxID_ANY, _("Section labels - tick Chorus for the chorus sections, Peak for the ones that should get a flash burst:"));
    labelHelp->Wrap(540);
    top->Add(labelHelp, 0, wxLEFT | wxRIGHT, 8);
    _labelInfo = new wxStaticText(this, wxID_ANY, wxEmptyString);
    top->Add(_labelInfo, 0, wxLEFT | wxRIGHT, 8);
    _labelPanel = new wxScrolledWindow(this, wxID_ANY, wxDefaultPosition, wxSize(-1, 120), wxVSCROLL | wxBORDER_THEME);
    _labelPanel->SetScrollRate(0, 10);
    _labelSizer = new wxFlexGridSizer(3, 4, 12);
    _labelPanel->SetSizer(_labelSizer);
    top->Add(_labelPanel, 1, wxEXPAND | wxALL, 8);

    top->Add(new wxStaticText(this, wxID_ANY, _("Moving head models (ordered left to right by layout position):")), 0, wxLEFT | wxRIGHT, 8);
    struct Head {
        std::string name;
        float x;
    };
    std::vector<Head> heads;
    std::vector<std::string> notInSequence;
    for (const auto& it : models) {
        Model* m = it.second;
        if (m == nullptr) {
            continue;
        }
        if (m->GetDisplayAs() != DisplayAsType::DmxMovingHeadAdv && m->GetDisplayAs() != DisplayAsType::DmxMovingHead) {
            continue;
        }
        Element* e = _elements.GetElement(m->GetName());
        if (e == nullptr || e->GetType() != ElementType::ELEMENT_TYPE_MODEL) {
            notInSequence.push_back(m->GetName());
            continue;
        }
        heads.push_back({ m->GetName(), m->GetBaseObjectScreenLocation().GetWorldPos_X() });
    }
    std::stable_sort(heads.begin(), heads.end(), [](const Head& a, const Head& b) { return a.x < b.x; });
    wxArrayString names;
    for (const auto& h : heads) {
        _headNames.push_back(h.name);
        names.Add(h.name);
    }
    _heads = new wxCheckListBox(this, wxID_ANY, wxDefaultPosition, wxSize(-1, 120), names);
    for (unsigned int i = 0; i < names.size(); ++i) {
        _heads->Check(i, true);
    }
    top->Add(_heads, 1, wxEXPAND | wxALL, 8);
    if (!notInSequence.empty()) {
        std::sort(notInSequence.begin(), notInSequence.end());
        wxString list;
        for (size_t i = 0; i < notInSequence.size(); ++i) {
            list += (i == 0 ? "" : ", ") + wxString(notInSequence[i]);
        }
        auto* missing = new wxStaticText(this, wxID_ANY, wxString::Format(_("Not listed because they are not in this sequence (add them in the sequence's Display Elements / Master View): %s"), list));
        missing->Wrap(540);
        missing->SetForegroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_GRAYTEXT));
        top->Add(missing, 0, wxLEFT | wxRIGHT | wxBOTTOM, 8);
    }
    if (names.empty()) {
        top->Add(new wxStaticText(this, wxID_ANY, _("No moving head models are in this sequence.")), 0, wxLEFT | wxRIGHT | wxBOTTOM, 8);
    }

    top->Add(CreateStdDialogButtonSizer(wxOK | wxCANCEL), 0, wxEXPAND | wxALL, 8);
    SetSizerAndFit(top);
    SetMinSize(wxSize(580, 640));
    SetSize(wxSize(620, 720));

    _sections->Bind(wxEVT_CHOICE, &MovingHeadSongDialog::OnSectionsChanged, this);
    LoadSettings();
    RebuildLabelRows();
}

namespace {
const char* const kCfgBars = "MHSongBarsTrack";
const char* const kCfgSections = "MHSongSectionsTrack";
const char* const kCfgLeadIn = "MHSongLeadIn";
const char* const kCfgPairing = "MHSongPairing";
const char* const kCfgRhythm = "MHSongRhythm";
const char* const kCfgSlew = "MHSongUseSlew";
const char* const kCfgBurst = "MHSongBurstEvery";
const char* const kCfgHeads = "MHSongHeads";
const char* const kCfgLabels = "MHSongLabels";

std::vector<std::string> SplitCsv(const std::string& s, char sep) {
    std::vector<std::string> out;
    size_t start = 0;
    while (start <= s.size()) {
        size_t pos = s.find(sep, start);
        if (pos == std::string::npos) {
            pos = s.size();
        }
        if (pos > start) {
            out.push_back(s.substr(start, pos - start));
        }
        start = pos + 1;
    }
    return out;
}
}

void MovingHeadSongDialog::LoadSettings() {
    auto* config = GetXLightsConfig();
    if (config == nullptr) {
        return;
    }
    auto select = [](wxChoice* c, const std::string& name) {
        int idx = c->FindString(wxString(name));
        if (idx != wxNOT_FOUND) {
            c->SetSelection(idx);
        }
    };
    std::string v = config->Read(kCfgBars, std::string());
    if (!v.empty()) {
        select(_bars, v);
    }
    v = config->Read(kCfgSections, std::string());
    if (!v.empty()) {
        select(_sections, v);
    }
    _leadIn->SetValue((int)config->ReadLong(kCfgLeadIn, _leadIn->GetValue()));
    _pairing->SetSelection((int)std::clamp<long>(config->ReadLong(kCfgPairing, _pairing->GetSelection()), 0, (long)_pairing->GetCount() - 1));
    _rhythm->SetSelection((int)std::clamp<long>(config->ReadLong(kCfgRhythm, _rhythm->GetSelection()), 0, (long)_rhythm->GetCount() - 1));
    _useSlew->SetValue(config->ReadBool(kCfgSlew, _useSlew->GetValue()));
    _burstEvery->SetValue((int)config->ReadLong(kCfgBurst, _burstEvery->GetValue()));

    v = config->Read(kCfgHeads, std::string());
    if (!v.empty()) {
        std::set<std::string> ticked;
        for (const auto& n : SplitCsv(v, '	')) {
            ticked.insert(n);
        }
        for (unsigned int i = 0; i < _heads->GetCount(); ++i) {
            _heads->Check(i, ticked.count(_headNames[i]) > 0);
        }
    }

    // "<sections track>	<label>:<flags>	<label>:<flags>..." where flags are c (chorus) and/or p (peak)
    v = config->Read(kCfgLabels, std::string());
    auto parts = SplitCsv(v, '	');
    if (!parts.empty()) {
        _savedLabelTrack = parts[0];
        for (size_t i = 1; i < parts.size(); ++i) {
            size_t colon = parts[i].rfind(':');
            if (colon == std::string::npos) {
                continue;
            }
            LabelFlags f;
            std::string flags = parts[i].substr(colon + 1);
            f.chorus = flags.find('c') != std::string::npos;
            f.peak = flags.find('p') != std::string::npos;
            _savedLabels[parts[i].substr(0, colon)] = f;
        }
    }
}

void MovingHeadSongDialog::SaveSettings() const {
    auto* config = GetXLightsConfig();
    if (config == nullptr) {
        return;
    }
    config->Write(kCfgBars, GetBarsTrack());
    config->Write(kCfgSections, GetSectionsTrack());
    config->Write(kCfgLeadIn, (long)GetLeadInMS());
    config->Write(kCfgPairing, (long)_pairing->GetSelection());
    config->Write(kCfgRhythm, (long)_rhythm->GetSelection());
    config->Write(kCfgSlew, GetUseSlew());
    config->Write(kCfgBurst, (long)GetBurstEveryBars());

    std::string heads;
    for (const auto& n : GetSelectedModels()) {
        heads += n + "	";
    }
    config->Write(kCfgHeads, heads);

    std::string labels = GetSectionsTrack();
    for (const auto& r : _rows) {
        labels += "	" + r.label + ":" + (r.chorus->GetValue() ? "c" : "") + (r.peak->GetValue() ? "p" : "");
    }
    config->Write(kCfgLabels, labels);
    config->Flush();
}

std::string MovingHeadSongDialog::GetBarsTrack() const {
    return _bars->GetSelection() == wxNOT_FOUND ? std::string() : _bars->GetStringSelection().ToStdString();
}

std::string MovingHeadSongDialog::GetSectionsTrack() const {
    return _sections->GetSelection() <= 0 ? std::string() : _sections->GetStringSelection().ToStdString();
}

long MovingHeadSongDialog::GetLeadInMS() const {
    return _leadIn->GetValue();
}

std::map<std::string, MovingHeadSongDialog::LabelFlags> MovingHeadSongDialog::GetLabelFlags() const {
    std::map<std::string, LabelFlags> res;
    for (const auto& r : _rows) {
        res[r.label] = { r.chorus->GetValue(), r.peak->GetValue() };
    }
    return res;
}

std::vector<std::string> MovingHeadSongDialog::GetSelectedModels() const {
    std::vector<std::string> res;
    for (unsigned int i = 0; i < _heads->GetCount(); ++i) {
        if (_heads->IsChecked(i)) {
            res.push_back(_headNames[i]);
        }
    }
    return res;
}

void MovingHeadSongDialog::OnSectionsChanged(wxCommandEvent&) {
    RebuildLabelRows();
}

std::vector<std::string> MovingHeadSongDialog::LabelsOf(const std::string& track) const {
    std::vector<std::string> labels;
    if (track.empty()) {
        return labels;
    }
    TimingElement* t = _elements.GetTimingElement(track);
    if (t == nullptr || t->GetEffectLayerCount() == 0) {
        return labels;
    }
    std::set<std::string> seen;
    for (size_t li = 0; li < t->GetEffectLayerCount(); ++li)
    for (Effect* ef : t->GetEffectLayer((int)li)->GetAllEffects()) {
        const std::string& l = ef->GetEffectName();
        if (seen.insert(l).second) {
            labels.push_back(l);
        }
    }
    return labels;
}

void MovingHeadSongDialog::RebuildLabelRows() {
    _labelSizer->Clear(true);
    _rows.clear();
    for (const auto& l : LabelsOf(GetSectionsTrack())) {
        auto* text = new wxStaticText(_labelPanel, wxID_ANY, l.empty() ? wxString(_("(blank)")) : wxString(l));
        auto* chorus = new wxCheckBox(_labelPanel, wxID_ANY, _("Chorus"));
        auto* peak = new wxCheckBox(_labelPanel, wxID_ANY, _("Peak"));
        chorus->SetValue(ContainsNoCase(l, "chorus") || ContainsNoCase(l, "hook"));
        if (GetSectionsTrack() == _savedLabelTrack) {
            auto saved = _savedLabels.find(l);
            if (saved != _savedLabels.end()) {
                chorus->SetValue(saved->second.chorus);
                peak->SetValue(saved->second.peak);
            }
        }
        _labelSizer->Add(text, 0, wxALIGN_CENTER_VERTICAL);
        _labelSizer->Add(chorus, 0);
        _labelSizer->Add(peak, 0);
        _rows.push_back({ l, chorus, peak });
    }
    if (GetSectionsTrack().empty()) {
        _labelInfo->SetLabel(_("No sections track selected - every bar will be treated as a verse bar."));
    } else if (_rows.empty()) {
        _labelInfo->SetLabel(_("No marks found in that track."));
    } else {
        _labelInfo->SetLabel(wxString::Format(_("%d distinct labels found:"), (int)_rows.size()));
    }
    _labelPanel->FitInside();
    _labelPanel->Layout();
    Layout();
}

void xLightsFrame::OnMenuItem_GenerateMovingHeadSongSelected(wxCommandEvent& /*event*/) {
    if (!IsSequenceLoaded()) {
        wxMessageBox(_("Open a sequence first."), _("Generate Moving Head Effects"), wxOK | wxICON_INFORMATION, this);
        return;
    }

    MovingHeadSongDialog dlg(this, _sequenceElements, AllModels);
    if (dlg.ShowModal() != wxID_OK) {
        return;
    }

    auto marks = [this](const std::string& track) {
        std::vector<Effect*> res;
        TimingElement* t = _sequenceElements.GetTimingElement(track);
        if (t != nullptr && t->GetEffectLayerCount() > 0) {
            for (Effect* e : t->GetEffectLayer(0)->GetAllEffects()) {
                res.push_back(e);
            }
            std::sort(res.begin(), res.end(), [](Effect* a, Effect* b) { return a->GetStartTimeMS() < b->GetStartTimeMS(); });
        }
        return res;
    };

    std::vector<std::string> models = dlg.GetSelectedModels();
    if (models.empty()) {
        wxMessageBox(_("No moving head models selected. Only moving heads that are in the sequence (Master View) can be used."), _("Generate Moving Head Effects"), wxOK | wxICON_INFORMATION, this);
        return;
    }
    auto barMarks = marks(dlg.GetBarsTrack());
    if (barMarks.size() < 2) {
        wxMessageBox(_("The bars timing track needs at least two marks."), _("Generate Moving Head Effects"), wxOK | wxICON_INFORMATION, this);
        return;
    }

    std::vector<long> bars;
    for (Effect* e : barMarks) {
        bars.push_back(e->GetStartTimeMS());
    }
    // Vamp tracks often stretch the last mark to the end of the song; keep the final bar a normal length in that case.
    long lastEnd = barMarks.back()->GetEndTimeMS();
    long lastLen = bars[bars.size() - 1] - bars[bars.size() - 2];
    bars.push_back((lastEnd > bars.back() && (lastLen <= 0 || lastEnd - bars.back() <= lastLen * 3 / 2)) ? lastEnd : bars.back() + lastLen);

    const long avgBar = (bars.back() - bars.front()) / (long)(bars.size() - 1);
    if (avgBar < 1000) {
        if (wxMessageBox(wxString::Format(_("The bars track averages only %ld ms per bar. It may be a beats track rather than a bars track, which would create a very large number of effects. Continue anyway?"), avgBar),
                         _("Generate Moving Head Effects"), wxYES_NO | wxNO_DEFAULT | wxICON_WARNING, this) != wxYES) {
            return;
        }
    }

    std::vector<MovingHeadSong::Section> sections;
    auto flags = dlg.GetLabelFlags();
    for (Effect* e : marks(dlg.GetSectionsTrack())) {
        MovingHeadSong::Section s;
        s.startMS = e->GetStartTimeMS();
        s.endMS = e->GetEndTimeMS();
        auto it = flags.find(e->GetEffectName());
        if (it != flags.end()) {
            s.chorus = it->second.chorus;
            s.peak = it->second.peak;
        }
        sections.push_back(s);
    }

    dlg.SaveSettings();

    MovingHeadSong::Params params;
    params.leadInMS = dlg.GetLeadInMS();
    params.chorusBeats = dlg.GetChorusBeats();
    params.burstEveryBars = dlg.GetBurstEveryBars();
    params.chorusPairing = dlg.GetChorusLeftRight() ? MovingHeadSong::ChorusPairing::LeftRight : MovingHeadSong::ChorusPairing::OddEven;
    const std::vector<std::string>& allHeads = dlg.GetAllModels();
    std::vector<int> active;
    for (const auto& name : models) {
        active.push_back((int)(std::find(allHeads.begin(), allHeads.end(), name) - allHeads.begin()));
    }
    if (dlg.GetUseSlew()) {
        for (const auto& name : allHeads) {
            auto* mh = dynamic_cast<DmxMovingHeadComm*>(AllModels.GetModel(name));
            auto* tilt = mh == nullptr ? nullptr : mh->GetTiltMotor();
            params.tiltSlewDegPerSec.push_back(tilt == nullptr ? 0.0f : tilt->GetSlewLimit());
        }
    }
    auto generated = MovingHeadSong::Generate(bars, sections, (int)allHeads.size(), params, active);

    // Only touch heads that can actually receive effects, so nothing is deleted without being replaced.
    std::vector<std::string> usable;
    std::vector<std::string> skipped;
    for (const auto& name : models) {
        Element* el = _sequenceElements.GetElement(name);
        auto* mh = dynamic_cast<DmxMovingHeadComm*>(AllModels.GetModel(name));
        if (el == nullptr || el->GetEffectLayerCount() == 0 || mh == nullptr || mh->GetFixtureVal() < 1 || mh->GetFixtureVal() > 8) {
            skipped.push_back(name);
        } else {
            usable.push_back(name);
        }
    }
    if (usable.empty()) {
        wxMessageBox(_("None of the selected models can take moving head effects (check each model's fixture number is 1-8)."), _("Generate Moving Head Effects"), wxOK | wxICON_INFORMATION, this);
        return;
    }
    if (generated.empty()) {
        wxMessageBox(_("No effects could be generated from the selected timing tracks. Nothing was changed."), _("Generate Moving Head Effects"), wxOK | wxICON_INFORMATION, this);
        return;
    }

    auto& undo = _sequenceElements.get_undo_mgr();
    undo.CreateUndoStep();

    for (const auto& name : usable) {
        Element* el = _sequenceElements.GetElement(name);
        if (el == nullptr) {
            continue;
        }
        for (size_t l = 0; l < el->GetEffectLayerCount(); ++l) {
            EffectLayer* layer = el->GetEffectLayer((int)l);
            for (Effect* ef : layer->GetAllEffects()) {
                if (ef->IsLocked()) {
                    continue;
                }
                undo.CaptureEffectToBeDeleted(el->GetName(), (int)l, ef->GetEffectName(), ef->GetSettingsAsString(), ef->GetPaletteAsString(),
                                              ef->GetStartTimeMS(), ef->GetEndTimeMS(), ef->GetSelected(), ef->GetProtected());
                layer->DeleteEffect(ef->GetID());
            }
        }
    }

    const long frame = std::max(1, (int)_seqData.FrameTime());
    auto snap = [frame](long ms) { return (ms + frame / 2) / frame * frame; };
    const std::string palette = MovingHeadSong::BuildPalette();
    int added = 0;
    int failed = 0;
    long endMS = 0;
    for (const auto& g : generated) {
        long s = snap(g.startMS);
        long e = snap(g.endMS);
        if (e <= s) {
            continue;
        }
        Element* el = _sequenceElements.GetElement(allHeads[g.head]);
        auto* mh = dynamic_cast<DmxMovingHeadComm*>(AllModels.GetModel(allHeads[g.head]));
        if (el == nullptr || mh == nullptr || el->GetEffectLayerCount() == 0 ||
            std::find(usable.begin(), usable.end(), allHeads[g.head]) == usable.end()) {
            continue;
        }
        EffectLayer* layer = el->GetEffectLayer(0);
        if (layer == nullptr) {
            ++failed;
            continue;
        }
        Effect* ef = layer->AddEffect(0, "Moving Head", MovingHeadSong::BuildSettings(g, mh->GetFixtureVal()), palette, (int)s, (int)e, EFFECT_NOT_SELECTED, false);
        if (ef == nullptr) {
            ++failed;
            continue;
        }
        undo.CaptureAddedEffect(el->GetName(), 0, ef->GetID());
        endMS = std::max(endMS, e);
        ++added;
    }

    MarkEffectsFileDirty();
    for (const auto& name : usable) {
        RenderEffectForModel(name, 0, (int)endMS, true);
    }
    mainSequencer->PanelEffectGrid->Refresh();

    wxString msg = wxString::Format(_("Added %d Moving Head effects to %d models."), added, (int)usable.size());
    if (!skipped.empty()) {
        msg += _("\nSkipped (no valid fixture number 1-8): ");
        for (size_t i = 0; i < skipped.size(); ++i) {
            msg += (i == 0 ? "" : ", ") + wxString(skipped[i]);
        }
    }
    if (failed > 0) {
        msg += wxString::Format(_("\n%d could not be added (overlap with locked effects?)."), failed);
    }
    wxMessageBox(msg, _("Generate Moving Head Effects"), wxOK | wxICON_INFORMATION, this);
}
