/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include "EffectSymbolDialog.h"
#include "render/SequenceElements.h"
#include "render/EffectSymbol.h"
#include "render/EffectSymbolManager.h"
#include "render/Effect.h"

#include <wx/sizer.h>
#include <wx/button.h>
#include <wx/textdlg.h>
#include <wx/msgdlg.h>

enum {
    ID_SYMBOL_LIST = wxID_HIGHEST + 1,
    ID_DELETE_BUTTON,
    ID_RENAME_BUTTON,
    ID_CLOSE_BUTTON
};

BEGIN_EVENT_TABLE(EffectSymbolDialog, wxDialog)
    EVT_BUTTON(ID_DELETE_BUTTON, EffectSymbolDialog::OnDeleteClick)
    EVT_BUTTON(ID_RENAME_BUTTON, EffectSymbolDialog::OnRenameClick)
    EVT_BUTTON(ID_CLOSE_BUTTON, EffectSymbolDialog::OnCloseClick)
    EVT_LIST_ITEM_SELECTED(ID_SYMBOL_LIST, EffectSymbolDialog::OnSymbolSelected)
    EVT_LIST_ITEM_DESELECTED(ID_SYMBOL_LIST, EffectSymbolDialog::OnSymbolDeselected)
END_EVENT_TABLE()

EffectSymbolDialog::EffectSymbolDialog(wxWindow* parent, SequenceElements* sequenceElements)
    : wxDialog(parent, wxID_ANY, "Effect Symbol Library", wxDefaultPosition, wxSize(500, 400),
               wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER)
    , m_sequenceElements(sequenceElements)
{
    wxBoxSizer* mainSizer = new wxBoxSizer(wxVERTICAL);

    // Create list control
    m_symbolList = new wxListCtrl(this, ID_SYMBOL_LIST, wxDefaultPosition, wxDefaultSize,
                                   wxLC_REPORT | wxLC_SINGLE_SEL);
    m_symbolList->InsertColumn(0, "Symbol Name", wxLIST_FORMAT_LEFT, 180);
    m_symbolList->InsertColumn(1, "Effect Type", wxLIST_FORMAT_LEFT, 120);
    m_symbolList->InsertColumn(2, "Linked Effects", wxLIST_FORMAT_CENTER, 100);

    mainSizer->Add(m_symbolList, 1, wxEXPAND | wxALL, 10);

    // Button sizer
    wxBoxSizer* buttonSizer = new wxBoxSizer(wxHORIZONTAL);

    m_deleteButton = new wxButton(this, ID_DELETE_BUTTON, "Delete");
    m_renameButton = new wxButton(this, ID_RENAME_BUTTON, "Rename");
    m_closeButton = new wxButton(this, ID_CLOSE_BUTTON, "Close");

    buttonSizer->Add(m_deleteButton, 0, wxRIGHT, 5);
    buttonSizer->Add(m_renameButton, 0, wxRIGHT, 5);
    buttonSizer->AddStretchSpacer();
    buttonSizer->Add(m_closeButton, 0);

    mainSizer->Add(buttonSizer, 0, wxEXPAND | wxLEFT | wxRIGHT | wxBOTTOM, 10);

    SetSizer(mainSizer);
    mainSizer->SetSizeHints(this);

    PopulateSymbolList();
    UpdateButtonStates();

    Center();
}

EffectSymbolDialog::~EffectSymbolDialog()
{
}

void EffectSymbolDialog::PopulateSymbolList()
{
    m_symbolList->DeleteAllItems();

    if (m_sequenceElements == nullptr) return;

    EffectSymbolManager& symbolMgr = m_sequenceElements->GetEffectSymbolManager();
    std::vector<EffectSymbol*> symbols = symbolMgr.GetAllSymbols();

    for (size_t i = 0; i < symbols.size(); i++) {
        EffectSymbol* symbol = symbols[i];
        if (symbol == nullptr) continue;

        long itemIndex = m_symbolList->InsertItem(i, symbol->GetName());
        m_symbolList->SetItem(itemIndex, 1, symbol->GetEffectType());

        size_t linkedCount = symbolMgr.GetLinkedEffectCount(symbol->GetId());
        m_symbolList->SetItem(itemIndex, 2, wxString::Format("%zu", linkedCount));

        // Store the symbol's index into the symbols vector — the list is
        // rebuilt from scratch on every change, so the index is stable for
        // the lifetime of any single user interaction.
        m_symbolList->SetItemData(itemIndex, i);
    }
}

void EffectSymbolDialog::OnDeleteClick(wxCommandEvent& event)
{
    long selectedIndex = m_symbolList->GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
    if (selectedIndex == -1) return;

    EffectSymbolManager& symbolMgr = m_sequenceElements->GetEffectSymbolManager();
    std::vector<EffectSymbol*> symbols = symbolMgr.GetAllSymbols();

    long dataIndex = m_symbolList->GetItemData(selectedIndex);
    if (dataIndex < 0 || dataIndex >= static_cast<long>(symbols.size())) return;

    EffectSymbol* symbol = symbols[dataIndex];
    if (symbol == nullptr) return;

    std::string symbolId = symbol->GetId();
    std::string symbolName = symbol->GetName();
    size_t linkedCount = symbolMgr.GetLinkedEffectCount(symbolId);

    // Confirm deletion
    wxString message;
    if (linkedCount > 0) {
        message = wxString::Format(
            "Are you sure you want to delete the symbol '%s'?\n\n"
            "This symbol has %zu linked effect(s). These effects will be "
            "converted to regular effects with their current settings preserved.",
            symbolName.c_str(), linkedCount);
    } else {
        message = wxString::Format(
            "Are you sure you want to delete the symbol '%s'?",
            symbolName.c_str());
    }

    if (wxMessageBox(message, "Confirm Delete Symbol",
                     wxYES_NO | wxNO_DEFAULT | wxICON_QUESTION, this) != wxYES) {
        return;
    }

    // Get all linked effects and unlink them before deleting the symbol
    std::vector<Effect*> linkedEffects = symbolMgr.GetLinkedEffects(symbolId);
    for (Effect* effect : linkedEffects) {
        if (effect != nullptr) {
            // Unlink the effect (it keeps its current settings)
            effect->UnlinkFromSymbol();
        }
    }

    // Now delete the symbol
    symbolMgr.DeleteSymbol(symbolId);

    // Refresh the list
    PopulateSymbolList();
    UpdateButtonStates();
}

void EffectSymbolDialog::OnRenameClick(wxCommandEvent& event)
{
    long selectedIndex = m_symbolList->GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
    if (selectedIndex == -1) return;

    EffectSymbolManager& symbolMgr = m_sequenceElements->GetEffectSymbolManager();
    std::vector<EffectSymbol*> symbols = symbolMgr.GetAllSymbols();

    long dataIndex = m_symbolList->GetItemData(selectedIndex);
    if (dataIndex < 0 || dataIndex >= static_cast<long>(symbols.size())) return;

    EffectSymbol* symbol = symbols[dataIndex];
    if (symbol == nullptr) return;

    wxTextEntryDialog dlg(this, "Enter new name for the symbol:",
                          "Rename Symbol", symbol->GetName());

    if (dlg.ShowModal() != wxID_OK) return;

    std::string newName = dlg.GetValue().ToStdString();
    if (newName.empty()) {
        wxMessageBox("Symbol name cannot be empty.", "Error", wxOK | wxICON_ERROR, this);
        return;
    }

    if (newName == symbol->GetName()) return; // No change

    // Check if name already exists
    if (symbolMgr.SymbolNameExists(newName)) {
        wxMessageBox("A symbol with this name already exists. Please choose a different name.",
                     "Error", wxOK | wxICON_ERROR, this);
        return;
    }

    // Rename the symbol
    symbolMgr.RenameSymbol(symbol->GetId(), newName);

    // Refresh the list
    PopulateSymbolList();
}

void EffectSymbolDialog::OnCloseClick(wxCommandEvent& event)
{
    EndModal(wxID_CLOSE);
}

void EffectSymbolDialog::OnSymbolSelected(wxListEvent& event)
{
    UpdateButtonStates();
}

void EffectSymbolDialog::OnSymbolDeselected(wxListEvent& event)
{
    UpdateButtonStates();
}

void EffectSymbolDialog::UpdateButtonStates()
{
    long selectedIndex = m_symbolList->GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
    bool hasSelection = (selectedIndex != -1);

    m_deleteButton->Enable(hasSelection);
    m_renameButton->Enable(hasSelection);
}
