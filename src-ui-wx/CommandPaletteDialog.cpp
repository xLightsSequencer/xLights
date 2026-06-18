
/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include "CommandPaletteDialog.h"
#include "effects/EffectManager.h"
#include "effects/RenderableEffect.h"
#include "effectpanels/EffectIconCache.h"

#include <wx/menu.h>
#include <wx/vlbox.h>
#include <wx/dcclient.h>
#include <wx/settings.h>

#ifdef __WXMAC__
#include <objc/objc.h>
#include <objc/message.h>
#endif

#include <algorithm>
#include <map>

// --- Dark theme colors ---
namespace {
    const wxColour CP_BG(30, 30, 30);
    const wxColour CP_INPUT_BG(55, 55, 55);
    const wxColour CP_BORDER(60, 60, 60);
    const wxColour CP_SELECTED_BG(2, 80, 130);
    const wxColour CP_TEXT(212, 212, 212);
    const wxColour CP_TEXT_DIM(118, 118, 118);
    const wxColour CP_TEXT_FAINT(80, 80, 80);
    const wxColour CP_MATCH_HL(229, 192, 75);
    const wxColour CP_ACCENT(0, 120, 212);
    const wxColour CP_BADGE_BG(55, 55, 55);
    const wxColour CP_EFFECT_DOT(80, 200, 120);
    const wxColour CP_HOVER_BG(45, 45, 45);
    constexpr int CP_ROW_HEIGHT = 34;
    constexpr int CP_H_PAD = 16;
}

// --- Fuzzy match position tracking ---
static std::vector<size_t> GetFuzzyMatchPositions(const wxString& text, const wxString& pattern)
{
    std::vector<size_t> positions;
    if (pattern.IsEmpty()) return positions;

    wxString lowerText = text.Lower();
    wxString lowerPattern = pattern.Lower();

    int pos = lowerText.Find(lowerPattern);
    if (pos != wxNOT_FOUND) {
        for (size_t i = 0; i < lowerPattern.length(); i++) {
            positions.push_back(static_cast<size_t>(pos) + i);
        }
        return positions;
    }

    size_t patIdx = 0;
    for (size_t i = 0; i < lowerText.length() && patIdx < lowerPattern.length(); i++) {
        if (lowerText[i] == lowerPattern[patIdx]) {
            positions.push_back(i);
            patIdx++;
        }
    }

    if (patIdx != lowerPattern.length()) {
        positions.clear();
    }
    return positions;
}

// --- Owner-drawn list box ---
class CommandPaletteListBox : public wxVListBox {
public:
    CommandPaletteListBox(wxWindow* parent, CommandPaletteDialog* dlg)
        : wxVListBox(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxBORDER_NONE)
        , _dialog(dlg)
    {
        SetBackgroundColour(CP_BG);
        Bind(wxEVT_MOTION, &CommandPaletteListBox::OnMouseMove, this);
        Bind(wxEVT_LEAVE_WINDOW, &CommandPaletteListBox::OnMouseLeave, this);
    }

    int GetHoveredRow() const { return _hoveredRow; }

protected:
    void OnDrawItem(wxDC& dc, const wxRect& rect, size_t n) const override
    {
        if (n >= _dialog->_filteredIndices.size()) return;
        const auto& cmd = _dialog->_allCommands[_dialog->_filteredIndices[n]];
        wxString searchText = _dialog->_searchCtrl->GetValue();

        wxFont nameFont = GetFont();
        wxFont boldFont = nameFont;
        boldFont.SetWeight(wxFONTWEIGHT_BOLD);
        wxFont dimFont = nameFont;
        dimFont.SetPointSize(std::max(dimFont.GetPointSize() - 1, 9));

        dc.SetFont(nameFont);
        int textH = dc.GetCharHeight();
        int y = rect.y + (rect.height - textH) / 2;

        // Accent bar on selected row
        if (IsSelected(n)) {
            dc.SetPen(*wxTRANSPARENT_PEN);
            dc.SetBrush(wxBrush(CP_ACCENT));
            dc.DrawRectangle(rect.x, rect.y + 6, 3, rect.height - 12);
        }

        // --- Right side: shortcut badge, then category ---
        int rightEdge = rect.GetRight() - CP_H_PAD;

        if (!cmd.shortcut.IsEmpty()) {
            dc.SetFont(dimFont);
            int sw = dc.GetTextExtent(cmd.shortcut).GetWidth();
            int sh = dc.GetTextExtent(cmd.shortcut).GetHeight();
            int bx = rightEdge - sw - 8;
            int by = y + (textH - sh) / 2 - 2;

            dc.SetPen(*wxTRANSPARENT_PEN);
            dc.SetBrush(wxBrush(CP_BADGE_BG));
            dc.DrawRoundedRectangle(bx, by, sw + 8, sh + 4, 3);
            dc.SetTextForeground(CP_TEXT_DIM);
            dc.DrawText(cmd.shortcut, bx + 4, by + 2);
            rightEdge = bx - 12;
        }

        dc.SetFont(dimFont);
        dc.SetTextForeground(IsSelected(n) ? CP_TEXT_DIM : CP_TEXT_FAINT);
        int catW = dc.GetTextExtent(cmd.category).GetWidth();
        dc.DrawText(cmd.category, rightEdge - catW, y + (textH - dc.GetCharHeight()) / 2);

        // --- Left side: icon + name with highlights ---
        int x = rect.x + CP_H_PAD;

        if (cmd.isEffect) {
            constexpr int iconColW = 28;
            constexpr int logicalIconSize = 16;
            double scale = GetDPIScaleFactor();
            int physSize = (int)std::round(logicalIconSize * scale);
            wxBitmap icon = GetCachedBundle(cmd.effectName).GetBitmap(wxSize(physSize, physSize));
            if (icon.IsOk()) {
                icon.SetScaleFactor(scale);
                int iconX = x + (iconColW - logicalIconSize) / 2;
                int iconY = rect.y + (rect.height - logicalIconSize) / 2;
                dc.DrawBitmap(icon, iconX, iconY, true);
            }
            x += iconColW;
        }

        auto matchPositions = GetFuzzyMatchPositions(cmd.name, searchText);
        size_t matchIdx = 0;
        size_t spanStart = 0;
        dc.SetFont(nameFont);

        while (spanStart < cmd.name.length()) {
            bool isMatch = (matchIdx < matchPositions.size() && matchPositions[matchIdx] == spanStart);
            size_t spanEnd = spanStart + 1;

            if (isMatch) {
                matchIdx++;
                while (spanEnd < cmd.name.length() &&
                       matchIdx < matchPositions.size() &&
                       matchPositions[matchIdx] == spanEnd) {
                    matchIdx++;
                    spanEnd++;
                }
            } else {
                size_t nextMatch = (matchIdx < matchPositions.size()) ? matchPositions[matchIdx] : cmd.name.length();
                spanEnd = nextMatch;
            }

            wxString span = cmd.name.Mid(spanStart, spanEnd - spanStart);
            if (isMatch) {
                dc.SetFont(boldFont);
                dc.SetTextForeground(CP_MATCH_HL);
            } else {
                dc.SetFont(nameFont);
                dc.SetTextForeground(CP_TEXT);
            }
            dc.DrawText(span, x, y);
            x += dc.GetTextExtent(span).GetWidth();

            spanStart = spanEnd;
        }
    }

    wxCoord OnMeasureItem(size_t) const override
    {
        return CP_ROW_HEIGHT;
    }

    void OnDrawBackground(wxDC& dc, const wxRect& rect, size_t n) const override
    {
        wxColour bg = CP_BG;
        if (IsSelected(n))
            bg = CP_SELECTED_BG;
        else if (static_cast<int>(n) == _hoveredRow)
            bg = CP_HOVER_BG;

        dc.SetPen(*wxTRANSPARENT_PEN);
        dc.SetBrush(wxBrush(bg));
        dc.DrawRectangle(rect);
    }

private:
    void OnMouseMove(wxMouseEvent& event)
    {
        int row = VirtualHitTest(event.GetPosition().y);
        if (row != _hoveredRow) {
            _hoveredRow = row;
            Refresh();
        }
        event.Skip();
    }

    void OnMouseLeave(wxMouseEvent& event)
    {
        if (_hoveredRow != -1) {
            _hoveredRow = -1;
            Refresh();
        }
        event.Skip();
    }

    CommandPaletteDialog* _dialog;
    int _hoveredRow = -1;

    mutable std::map<wxString, wxBitmapBundle> _iconBundleCache;

    const wxBitmapBundle& GetCachedBundle(const wxString& effectName) const
    {
        auto it = _iconBundleCache.find(effectName);
        if (it != _iconBundleCache.end()) return it->second;

        wxBitmapBundle bundle;
        if (_dialog->_effectManager) {
            auto* eff = _dialog->_effectManager->GetEffect(effectName.ToStdString());
            if (eff)
                bundle = EffectIconCache::GetEffectIcon(eff, 16);
        }
        return _iconBundleCache.emplace(effectName, std::move(bundle)).first->second;
    }
};

// --- CommandPaletteDialog ---

CommandPaletteDialog::CommandPaletteDialog(wxWindow* parent, wxMenuBar* menuBar, EffectManager* effectManager)
    : wxDialog(parent, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize(650, 420), wxBORDER_NONE)
{
    SetBackgroundColour(CP_BORDER);

    // Position top-center of parent, ~20% from top
    if (parent) {
        wxPoint pp = parent->GetScreenPosition();
        wxSize ps = parent->GetSize();
        SetPosition(wxPoint(pp.x + (ps.x - 650) / 2, pp.y + ps.y / 5));
    }

    // Outer sizer with 1px margin creates the border effect
    auto* outerSizer = new wxBoxSizer(wxVERTICAL);
    auto* mainPanel = new wxPanel(this, wxID_ANY);
    mainPanel->SetBackgroundColour(CP_BG);
    outerSizer->Add(mainPanel, 1, wxEXPAND | wxALL, 1);
    SetSizer(outerSizer);

#ifdef __WXMAC__
    // Round the window corners via NSWindow/NSView layer
    {
        id contentView = (id)GetHandle();
        if (contentView) {
            id window = ((id (*)(id, SEL))objc_msgSend)(contentView, sel_registerName("window"));
            if (window) {
                ((void (*)(id, SEL, BOOL))objc_msgSend)(window, sel_registerName("setOpaque:"), NO);
                ((void (*)(id, SEL, id))objc_msgSend)(window, sel_registerName("setBackgroundColor:"),
                    ((id (*)(id, SEL))objc_msgSend)((id)objc_getClass("NSColor"), sel_registerName("clearColor")));
                ((void (*)(id, SEL, BOOL))objc_msgSend)(window, sel_registerName("setHasShadow:"), YES);
            }
            ((void (*)(id, SEL, BOOL))objc_msgSend)(contentView, sel_registerName("setWantsLayer:"), YES);
            id layer = ((id (*)(id, SEL))objc_msgSend)(contentView, sel_registerName("layer"));
            if (layer) {
                ((void (*)(id, SEL, double))objc_msgSend)(layer, sel_registerName("setCornerRadius:"), 12.0);
                ((void (*)(id, SEL, BOOL))objc_msgSend)(layer, sel_registerName("setMasksToBounds:"), YES);
            }
        }
    }
#endif

    auto* innerSizer = new wxBoxSizer(wxVERTICAL);

    // Search container - provides padding and vertical centering
    auto* searchContainer = new wxPanel(mainPanel, wxID_ANY);
    searchContainer->SetBackgroundColour(CP_INPUT_BG);

    _searchCtrl = new wxTextCtrl(searchContainer, wxID_ANY, wxEmptyString,
                                  wxDefaultPosition, wxDefaultSize,
                                  wxTE_PROCESS_ENTER | wxBORDER_NONE);
    _searchCtrl->SetBackgroundColour(CP_INPUT_BG);
    _searchCtrl->SetForegroundColour(CP_TEXT);
    wxFont searchFont = _searchCtrl->GetFont();
    searchFont.SetPointSize(searchFont.GetPointSize() + 2);
    _searchCtrl->SetFont(searchFont);
    _searchCtrl->SetHint("Search commands and effects...");

#ifdef __WXMAC__
    // Remove the macOS focus ring
    id nsView = (id)_searchCtrl->GetHandle();
    if (nsView) {
        ((void (*)(id, SEL, long))objc_msgSend)(nsView, sel_registerName("setFocusRingType:"), 1L);
    }
#endif

    auto* searchSizer = new wxBoxSizer(wxHORIZONTAL);
    searchSizer->Add(_searchCtrl, 1, wxALIGN_CENTER_VERTICAL | wxLEFT | wxRIGHT, 12);
    searchContainer->SetSizer(searchSizer);
    searchContainer->SetMinSize(wxSize(-1, 48));

#ifdef __WXMAC__
    // Round the search input container corners
    {
        id searchView = (id)searchContainer->GetHandle();
        if (searchView) {
            ((void (*)(id, SEL, BOOL))objc_msgSend)(searchView, sel_registerName("setWantsLayer:"), YES);
            id layer = ((id (*)(id, SEL))objc_msgSend)(searchView, sel_registerName("layer"));
            if (layer) {
                ((void (*)(id, SEL, double))objc_msgSend)(layer, sel_registerName("setCornerRadius:"), 8.0);
                ((void (*)(id, SEL, BOOL))objc_msgSend)(layer, sel_registerName("setMasksToBounds:"), YES);
            }
        }
    }
#endif

    innerSizer->Add(searchContainer, 0, wxEXPAND | wxLEFT | wxRIGHT | wxTOP, 8);

    // Separator
    auto* sep = new wxPanel(mainPanel, wxID_ANY, wxDefaultPosition, wxSize(-1, 1));
    sep->SetBackgroundColour(CP_BORDER);
    innerSizer->Add(sep, 0, wxEXPAND | wxTOP, 4);

    // Results list
    _listBox = new CommandPaletteListBox(mainPanel, this);
    innerSizer->Add(_listBox, 1, wxEXPAND | wxTOP, 4);

    mainPanel->SetSizer(innerSizer);

    _effectManager = effectManager;
    BuildCommandList(menuBar);
    AddEffectEntries(effectManager);
    UpdateResults();

    _searchCtrl->Bind(wxEVT_TEXT, &CommandPaletteDialog::OnSearchTextChanged, this);
    _searchCtrl->Bind(wxEVT_KEY_DOWN, &CommandPaletteDialog::OnSearchKeyDown, this);
    _listBox->Bind(wxEVT_LISTBOX_DCLICK, &CommandPaletteDialog::OnListDoubleClick, this);
    _listBox->Bind(wxEVT_KEY_DOWN, &CommandPaletteDialog::OnListKeyDown, this);

    _searchCtrl->SetFocus();
}

void CommandPaletteDialog::EndModal(int retCode)
{
#ifdef __WXMAC__
    // Immediately yank the window off screen to avoid flash from transparent backing
    id contentView = (id)GetHandle();
    if (contentView) {
        id window = ((id (*)(id, SEL))objc_msgSend)(contentView, sel_registerName("window"));
        if (window) {
            ((void (*)(id, SEL, double))objc_msgSend)(window, sel_registerName("setAlphaValue:"), 0.0);
        }
    }
#endif
    wxDialog::EndModal(retCode);
}

void CommandPaletteDialog::BuildCommandList(wxMenuBar* menuBar)
{
    if (!menuBar) return;

    for (size_t i = 0; i < menuBar->GetMenuCount(); ++i) {
        wxString label = menuBar->GetMenuLabelText(i);
        CollectMenuItems(menuBar->GetMenu(i), label);
    }
}

static wxString GetAccelText(const wxMenuItem* item)
{
    wxString label = item->GetItemLabel();
    int tabPos = label.Find('\t');
    if (tabPos != wxNOT_FOUND) {
        return label.Mid(tabPos + 1);
    }
    return wxEmptyString;
}

void CommandPaletteDialog::CollectMenuItems(wxMenu* menu, const wxString& category)
{
    if (!menu) return;

    for (size_t i = 0; i < menu->GetMenuItemCount(); ++i) {
        wxMenuItem* item = menu->FindItemByPosition(i);
        if (!item) continue;
        if (item->IsSeparator()) continue;
        if (!item->IsEnabled()) continue;

        if (item->GetSubMenu()) {
            wxString subCategory = category + " > " + item->GetItemLabelText();
            CollectMenuItems(item->GetSubMenu(), subCategory);
            continue;
        }

        CommandEntry entry;
        entry.name = item->GetItemLabelText();
        entry.category = category;
        entry.id = item->GetId();
        entry.shortcut = GetAccelText(item);

        if (!entry.name.IsEmpty()) {
            _allCommands.push_back(entry);
        }
    }
}

void CommandPaletteDialog::AddEffectEntries(EffectManager* effectManager)
{
    if (!effectManager) return;

    for (size_t i = 0; i < effectManager->size(); ++i) {
        const std::string& name = effectManager->GetEffectName(i);
        if (name.empty()) continue;

        CommandEntry entry;
        entry.name = name;
        entry.category = "Add Effect";
        entry.id = wxID_NONE;
        entry.isEffect = true;
        entry.effectName = name;
        _allCommands.push_back(entry);
    }

}

bool CommandPaletteDialog::FuzzyMatch(const wxString& text, const wxString& pattern, int& score) const
{
    if (pattern.IsEmpty()) {
        score = 0;
        return true;
    }

    wxString lowerText = text.Lower();
    wxString lowerPattern = pattern.Lower();

    int pos = lowerText.Find(lowerPattern);
    if (pos != wxNOT_FOUND) {
        score = (pos == 0) ? 100 : 50;
        return true;
    }

    size_t patIdx = 0;
    int matchScore = 0;
    bool prevMatched = false;

    for (size_t i = 0; i < lowerText.length() && patIdx < lowerPattern.length(); ++i) {
        if (lowerText[i] == lowerPattern[patIdx]) {
            patIdx++;
            matchScore += prevMatched ? 5 : 1;
            if (i == 0 || lowerText[i - 1] == ' ' || lowerText[i - 1] == '_' ||
                lowerText[i - 1] == '-' || lowerText[i - 1] == '>' || lowerText[i - 1] == '/') {
                matchScore += 10;
            }
            prevMatched = true;
        } else {
            prevMatched = false;
        }
    }

    if (patIdx == lowerPattern.length()) {
        score = matchScore;
        return true;
    }

    score = 0;
    return false;
}

void CommandPaletteDialog::UpdateResults()
{
    _filteredIndices.clear();

    wxString searchText = _searchCtrl->GetValue();

    struct ScoredIndex {
        size_t index;
        int nameScore;
        int catScore;
    };
    std::vector<ScoredIndex> scored;

    for (size_t i = 0; i < _allCommands.size(); ++i) {
        int nameScore = 0;
        int catScore = 0;
        bool nameMatch = FuzzyMatch(_allCommands[i].name, searchText, nameScore);
        bool catMatch = FuzzyMatch(_allCommands[i].category, searchText, catScore);

        if (nameMatch || catMatch) {
            scored.push_back({ i, nameScore, catScore });
        }
    }

    std::sort(scored.begin(), scored.end(), [](const ScoredIndex& a, const ScoredIndex& b) {
        int scoreA = std::max(a.nameScore, a.catScore);
        int scoreB = std::max(b.nameScore, b.catScore);
        if (scoreA != scoreB) return scoreA > scoreB;
        if (a.nameScore != b.nameScore) return a.nameScore > b.nameScore;
        return a.index < b.index;
    });

    for (const auto& s : scored) {
        _filteredIndices.push_back(s.index);
    }

    _listBox->SetItemCount(_filteredIndices.size());
    if (!_filteredIndices.empty()) {
        _listBox->SetSelection(0);
    }
    _listBox->Refresh();
}

void CommandPaletteDialog::ActivateSelection()
{
    int sel = _listBox->GetSelection();
    if (sel != wxNOT_FOUND && sel < static_cast<int>(_filteredIndices.size())) {
        const auto& cmd = _allCommands[_filteredIndices[sel]];
        _selectedCommandId = cmd.id;
        _isEffectSelected = cmd.isEffect;
        _selectedEffectName = cmd.effectName;
        _selectedCommandName = cmd.name;
        EndModal(wxID_OK);
    }
}

void CommandPaletteDialog::OnSearchTextChanged(wxCommandEvent& event)
{
    UpdateResults();
}

void CommandPaletteDialog::OnSearchKeyDown(wxKeyEvent& event)
{
    int key = event.GetKeyCode();

    if (key == WXK_DOWN) {
        int count = static_cast<int>(_filteredIndices.size());
        if (count > 0) {
            int sel = _listBox->GetSelection();
            int next = (sel < 0) ? 0 : std::min(sel + 1, count - 1);
            _listBox->SetSelection(next);
        }
    } else if (key == WXK_UP) {
        int count = static_cast<int>(_filteredIndices.size());
        if (count > 0) {
            int sel = _listBox->GetSelection();
            int next = (sel <= 0) ? 0 : sel - 1;
            _listBox->SetSelection(next);
        }
    } else if (key == WXK_RETURN || key == WXK_NUMPAD_ENTER) {
        ActivateSelection();
    } else if (key == WXK_ESCAPE) {
        EndModal(wxID_CANCEL);
    } else {
        event.Skip();
    }
}

void CommandPaletteDialog::OnListDoubleClick(wxCommandEvent& event)
{
    ActivateSelection();
}

void CommandPaletteDialog::OnListKeyDown(wxKeyEvent& event)
{
    int key = event.GetKeyCode();
    if (key == WXK_RETURN || key == WXK_NUMPAD_ENTER) {
        ActivateSelection();
    } else if (key == WXK_ESCAPE) {
        EndModal(wxID_CANCEL);
    } else if (key >= 32 && key < 127) {
        _searchCtrl->SetFocus();
        _searchCtrl->SetInsertionPointEnd();
        _searchCtrl->WriteText(wxString(static_cast<wxChar>(key)));
    } else {
        event.Skip();
    }
}
