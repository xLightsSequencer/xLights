/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include "PlayListItemESEQ.h"
#include "PlayListItemESEQPanel.h"
#include "../../xLights/UtilFunctions.h"
#include "wx/xml/xml.h"
#include <wx/notebook.h>

PlayListItemESEQ::PlayListItemESEQ(wxXmlNode* node) :
    PlayListItem(node) {
    _ESEQFile = nullptr;
    _ESEQFileName = "";
    _applyMethod = APPLYMETHOD::METHOD_OVERWRITE;
    PlayListItemESEQ::Load(node);
}

void PlayListItemESEQ::Load(wxXmlNode* node) {
    PlayListItem::Load(node);
    _ESEQFileName = node->GetAttribute("ESEQFile", "");
    _ESEQFileName = FixFile("", _ESEQFileName);
    _applyMethod = (APPLYMETHOD)wxAtoi(node->GetAttribute("ApplyMethod", ""));
}

PlayListItemESEQ::PlayListItemESEQ() :
    PlayListItem() {
    _type = "PLIESEQ";
    _ESEQFile = nullptr;
    _ESEQFileName = "";
    _applyMethod = APPLYMETHOD::METHOD_OVERWRITE;
}

size_t PlayListItemESEQ::GetDurationMS(size_t frameMS) const {
    if (_ESEQFile != nullptr) {
        return _ESEQFile->GetLengthFrames() * frameMS;
    }

    return 0;
}

PlayListItem* PlayListItemESEQ::Copy(const bool isClone) const {
    PlayListItemESEQ* res = new PlayListItemESEQ();
    res->_ESEQFileName = _ESEQFileName;
    res->_applyMethod = _applyMethod;
    PlayListItem::Copy(res, isClone);

    return res;
}

wxXmlNode* PlayListItemESEQ::Save() {
    wxXmlNode* node = new wxXmlNode(nullptr, wxXML_ELEMENT_NODE, GetType());

    node->AddAttribute("ESEQFile", _ESEQFileName);
    _ESEQFileName = FixFile("", _ESEQFileName);
    node->AddAttribute("ApplyMethod", wxString::Format(wxT("%i"), (int)_applyMethod));

    PlayListItem::Save(node);

    return node;
}

std::string PlayListItemESEQ::GetTitle() const {
    return "ESEQ";
}

void PlayListItemESEQ::Configure(wxNotebook* notebook) {
    notebook->AddPage(new PlayListItemESEQPanel(notebook, this), GetTitle(), true);
}

std::string PlayListItemESEQ::GetNameNoTime() const {
    wxFileName fn(_ESEQFileName);
    if (fn.GetName() == "") {
        return "ESEQ";
    } else {
        return fn.GetName().ToStdString();
    }
}

void PlayListItemESEQ::SetESEQFileName(const std::string& ESEQFileName) {
    if (ESEQFileName != _ESEQFileName) {
        _ESEQFileName = ESEQFileName;
        _changeCount++;
    }
}

void PlayListItemESEQ::Frame(uint8_t* buffer, size_t size, size_t ms, size_t framems, bool outputframe) {
    if (outputframe) {
        if (_ESEQFile != nullptr) {
            _ESEQFile->ReadData(buffer, size, ms / framems, _applyMethod);
        }
    }
}

void PlayListItemESEQ::Start(long stepLengthMS) {
    PlayListItem::Start(stepLengthMS);

    // load the FSEQ
    LoadFiles();
}

void PlayListItemESEQ::Stop() {
    // load the FSEQ
    CloseFiles();
}

void PlayListItemESEQ::LoadFiles() {
    CloseFiles();

    if (wxFile::Exists(_ESEQFileName)) {
        _ESEQFile = new ESEQFile();
        _ESEQFile->Load(_ESEQFileName);
    }
}

void PlayListItemESEQ::CloseFiles() {
    if (_ESEQFile != nullptr) {
        _ESEQFile->Close();
        delete _ESEQFile;
        _ESEQFile = nullptr;
    }
}

std::list<std::string> PlayListItemESEQ::GetMissingFiles() {
    std::list<std::string> res;
    if (!wxFile::Exists(GetESEQFileName())) {
        res.push_back(GetESEQFileName());
    }

    return res;
}
