#include "ModelRemap.h"

//(*InternalHeaders(ModelRemap)
#include <wx/intl.h>
#include <wx/string.h>
//*)

#include <wx/msgdlg.h>

//(*IdInit(ModelRemap)
const long ModelRemap::ID_STATICTEXT1 = wxNewId();
const long ModelRemap::ID_STATICTEXT2 = wxNewId();
const long ModelRemap::ID_FILEPICKERCTRL1 = wxNewId();
const long ModelRemap::ID_STATICTEXT3 = wxNewId();
const long ModelRemap::ID_FILEPICKERCTRL2 = wxNewId();
const long ModelRemap::ID_TEXTCTRL1 = wxNewId();
const long ModelRemap::ID_BUTTON1 = wxNewId();
//*)

BEGIN_EVENT_TABLE(ModelRemap,wxDialog)
	//(*EventTable(ModelRemap)
	//*)
END_EVENT_TABLE()

ModelRemap::ModelRemap(wxWindow* parent,wxWindowID id,const wxPoint& pos,const wxSize& size)
{
	//(*Initialize(ModelRemap)
	wxFlexGridSizer* FlexGridSizer1;
	wxFlexGridSizer* FlexGridSizer2;

	Create(parent, id, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxCAPTION|wxRESIZE_BORDER|wxCLOSE_BOX, _T("id"));
	SetClientSize(wxDefaultSize);
	Move(wxDefaultPosition);
	FlexGridSizer1 = new wxFlexGridSizer(0, 1, 0, 0);
	FlexGridSizer1->AddGrowableCol(0);
	StaticText1 = new wxStaticText(this, ID_STATICTEXT1, _("Set in code"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT1"));
	FlexGridSizer1->Add(StaticText1, 1, wxALL|wxEXPAND, 5);
	FlexGridSizer2 = new wxFlexGridSizer(0, 2, 0, 2);
	FlexGridSizer2->AddGrowableCol(1);
	StaticText2 = new wxStaticText(this, ID_STATICTEXT2, _("Custom xModel File With the faces/states/submodels:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT2"));
	FlexGridSizer2->Add(StaticText2, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	FilePickerCtrl_Original = new wxFilePickerCtrl(this, ID_FILEPICKERCTRL1, wxEmptyString, _("Select a file"), _T("*.xmodel"), wxDefaultPosition, wxDefaultSize, wxFLP_FILE_MUST_EXIST|wxFLP_OPEN|wxFLP_USE_TEXTCTRL, wxDefaultValidator, _T("ID_FILEPICKERCTRL1"));
	FlexGridSizer2->Add(FilePickerCtrl_Original, 1, wxALL|wxEXPAND, 5);
	StaticText3 = new wxStaticText(this, ID_STATICTEXT3, _("Custom xModel File with the correct wiring layout:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT3"));
	FlexGridSizer2->Add(StaticText3, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	FilePickerCtrl_NewWiring = new wxFilePickerCtrl(this, ID_FILEPICKERCTRL2, wxEmptyString, _("Select a file"), _T("*.xmodel"), wxDefaultPosition, wxDefaultSize, wxFLP_FILE_MUST_EXIST|wxFLP_OPEN|wxFLP_USE_TEXTCTRL, wxDefaultValidator, _T("ID_FILEPICKERCTRL2"));
	FlexGridSizer2->Add(FilePickerCtrl_NewWiring, 1, wxALL|wxEXPAND, 5);
	FlexGridSizer1->Add(FlexGridSizer2, 1, wxALL|wxEXPAND, 5);
	TextCtrl_Messages = new wxTextCtrl(this, ID_TEXTCTRL1, _("Select xModel files."), wxDefaultPosition, wxSize(0,200), wxTE_MULTILINE|wxTE_READONLY, wxDefaultValidator, _T("ID_TEXTCTRL1"));
	FlexGridSizer1->Add(TextCtrl_Messages, 1, wxALL|wxEXPAND, 5);
	Button_Generate = new wxButton(this, ID_BUTTON1, _("Generate xModel"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON1"));
	FlexGridSizer1->Add(Button_Generate, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	SetSizer(FlexGridSizer1);
	FlexGridSizer1->Fit(this);
	FlexGridSizer1->SetSizeHints(this);

	Connect(ID_FILEPICKERCTRL1,wxEVT_COMMAND_FILEPICKER_CHANGED,(wxObjectEventFunction)&ModelRemap::OnFilePickerCtrl_OriginalFileChanged);
	Connect(ID_FILEPICKERCTRL2,wxEVT_COMMAND_FILEPICKER_CHANGED,(wxObjectEventFunction)&ModelRemap::OnFilePickerCtrl_NewWiringFileChanged);
	Connect(ID_BUTTON1,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&ModelRemap::OnButton_GenerateClick);
	//*)

    StaticText1->SetLabelText("This tool is used to take a model with valid/useful face definitions, state definitions and submodels and remap them onto a model with a reordered wiring layout.\n\n"
                              "To use the tool you must have 2 xmodel files.\n\n"
                              "The first xmodel is the custom model with the original wiring layout but the faces/states/submodels you want.\n"
                              "The second is an identically dimensioned custom model with pixels in all the same positions but renumbered that you want to load the submodels into.\n\n"
                              "As part of this process all existing faces/states/submodels on the second xmodel file will be removed.\n\n"
                              "When you click generate a new xmodel file will be created combining the second models wiring layout with the faces/states/submodels of the first model.");

    Fit();
	ValidateWindow();
}

ModelRemap::~ModelRemap()
{
	//(*Destroy(ModelRemap)
	//*)
}


void ModelRemap::OnButton_GenerateClick(wxCommandEvent& event)
{
    newWiringProperties.Remap(originalProperties);
    newWiringProperties.Save(this);
    EndDialog(wxCLOSE);
}

void ModelRemap::OnFilePickerCtrl_OriginalFileChanged(wxFileDirPickerEvent& event)
{
    ValidateWindow();
}

void ModelRemap::OnFilePickerCtrl_NewWiringFileChanged(wxFileDirPickerEvent& event)
{
    ValidateWindow();
}

void ModelRemap::ValidateWindow()
{
    std::string message;
    originalProperties.Load(FilePickerCtrl_Original->GetFileName().GetFullPath());
    newWiringProperties.Load(FilePickerCtrl_NewWiring->GetFileName().GetFullPath());

    if (!originalProperties.IsOk() && !newWiringProperties.IsOk()) {
        message = "Select valid xmodel files.";
    } else if (!originalProperties.IsOk()) {
        message = newWiringProperties.Message();
        message += "\n\nSelect valid original xmodel.";
    } else if (!newWiringProperties.IsOk()) {
        message = originalProperties.Message();
        message += "\n\nSelect valid new wiring xmodel.";
    } else {
        newWiringProperties.Compare(originalProperties);
        message = originalProperties.Message() + "\n\n" + newWiringProperties.Message();
        if (!originalProperties.ContainsFSSM()) {
            message += "\n\nOriginal model contains no faces/states/submodels to import.";
        }
    }

    TextCtrl_Messages->SetValue(message);

    if (originalProperties.IsOk() && newWiringProperties.IsOk()) {
        Button_Generate->Enable();
    } else {
        Button_Generate->Disable();
	}
}

void RemapModelProperties::RemapNodes(wxXmlNode* n, const std::string& attr, const std::map<uint32_t, uint32_t>& mapping)
{
    auto str = n->GetAttribute(attr, "");

    std::vector<uint32_t> nl;

    auto rs = wxSplit(str, ',');
    for (const auto& r : rs) {
        if (r.Contains("-")) {
            auto hl = wxSplit(r, '-');
            if (hl.size() == 2) {
                uint32_t a1 = wxAtoi(hl[0]);
                uint32_t a2 = wxAtoi(hl[1]);
                if (a1 <= a2) {
                    for (uint32_t i = a1; i <= a2; ++i) {
                        nl.push_back(i);
                    }
                } else {
                    for (uint32_t i = a1; i >= a2; --i) {
                        nl.push_back(i);
                    }
                }
            }
        } else if (r
            == "")
            {
            nl.push_back(0);
        } else {
            nl.push_back(wxAtoi(r));
        }
    }

    std::vector<uint32_t> nnl;
    for (const auto it : nl) {
        if (mapping.find(it) == mapping.end()) {
            // this should not happen
        } else {
            nnl.push_back(mapping.at(it));
        }
    }

    // now recreate the string
    std::string stro;
    uint32_t runstart = 0;
    uint32_t rundir = 0;
    uint32_t runlast = 0xFFFFFFFF;
    uint32_t runlen = 0;

    for (const auto it : nnl) {
        if (it == 0) {
            if (runlast == 0xFFFFFFFF) {
                stro += ",";
            } else {
                if (runlen != 0) {
                    if (runlen == 1) {
                        stro += std::to_string(runstart) + ",";
                    } else {
                        // need to close off the run
                        if (rundir == 1) {
                            // run up
                            stro += std::to_string(runstart) + "-" + std::to_string(runstart + (runlen - 1)) + ",";
                        } else if (rundir == 2) {
                            stro += std::to_string(runstart) + "-" + std::to_string(runstart - (runlen - 1)) + ",";
                        }
                    }
                    runstart = it;
                    runlast = it;
                    runlen = 0;
                    rundir = 0;
                }
            }
        } else {
            // we could be in a run
            bool runbroke = false;
            if (rundir == 0) {
                if (runstart == 0) {
                    runstart = it;
                    runlast = it;
                    runlen = 1;
                } else {
                    if (it == runlast + 1) {
                        rundir = 1;
                        runlast = it;
                        ++runlen;
                    } else if (it == runlast - 1) {
                        rundir = 2;
                        runlast = it;
                        ++runlen;
                    } else {
                        runbroke = true;
                    }
                }
            } else if (rundir == 1) {
                if (it == runlast + 1) {
                    runlast = it;
                    ++runlen;
                } else {
                    runbroke = true;
                }
            } else if (rundir == 2) {
                if (it == runlast - 1) {
                    runlast = it;
                    ++runlen;
                } else {
                    runbroke = true;
                }
            }
            if (runbroke) {
                if (runlen == 1) {
                    stro += std::to_string(runstart) + ",";
                } else {
                    // need to close off the run
                    if (rundir == 1) {
                        // run up
                        stro += std::to_string(runstart) + "-" + std::to_string(runstart + (runlen - 1)) + ",";
                    } else if (rundir == 2) {
                        stro += std::to_string(runstart) + "-" + std::to_string(runstart - (runlen - 1)) + ",";
                    }
                }

                runstart = it;
                runlast = it;
                runlen = 1;
                rundir = 0;
            }
        }
    }

    if (runlen != 0) {
        if (runlen == 1) {
            stro += std::to_string(runstart);
        } else {
            // need to close off the run
            if (rundir == 1) {
                // run up
                stro += std::to_string(runstart) + "-" + std::to_string(runstart + (runlen - 1));
            } else if (rundir == 2) {
                stro += std::to_string(runstart) + "-" + std::to_string(runstart - (runlen - 1));
            }
        }
    }

    n->DeleteAttribute(attr);
    n->AddAttribute(attr, stro);
}

void RemapModelProperties::ParseData(const std::string& data)
{
    _data.clear();

	auto layers = wxSplit(data, '|');
    for (const auto& l : layers) {
        auto rows = wxSplit(l, ';');
        for (const auto& r : rows) {
            auto cells = wxSplit(r, ',');
            for (const auto& c : cells) {
                if (c == "") {
                    _data.push_back(0);
                } else {
                    _data.push_back(wxAtoi(c));
                }
            }
        }
	}
}

void RemapModelProperties::Load(const std::string& filename)
{
    _ok = false;
    _faces = 0;
    _states = 0;
    _submodels = 0;
    _w = 0;
    _h = 0;
    _d = 0;
    _filename = filename;
    _message = "";
    if (wxFile::Exists(filename)) {
        _xmodel.Load(filename);
        if (_xmodel.IsOk()) {
			// check it is a custom model
            _message = filename;

			if (_xmodel.GetRoot()->GetName() != "custommodel") {
                _message += "\nModel is not a custom model: " + _xmodel.GetRoot()->GetName();
			} else {
                // extract model properties
                // dimensions
                _w = wxAtoi(_xmodel.GetRoot()->GetAttribute("parm1", "0"));
                _h = wxAtoi(_xmodel.GetRoot()->GetAttribute("parm2", "0"));
                _d = wxAtoi(_xmodel.GetRoot()->GetAttribute("Depth", "1"));

				_message += "\nW: " + std::to_string(_w) + " H: " + std::to_string(_h) + " D: " + std::to_string(_d);

				if (_w == 0 || _h == 0 || _d == 0) {
                    _message += "\nAt least one dimension invalid.";
				} else {
                    // number of pixels
                    ParseData(_xmodel.GetRoot()->GetAttribute("CustomModel"));

					uint32_t cnt = 0;
                    for (const auto& it: _data) {
                        if (it != 0) {
                            cnt++;
                        }
					}

					if (cnt == 0) {
                        _message += "\nNo pixels.";
                    }
					else {
                        _message += "\nPixels: " + std::to_string(cnt);
                        _ok = true;

                        for (auto n = _xmodel.GetRoot()->GetChildren(); n != nullptr; n = n->GetNext()) {
                            if (n->GetName() == "faceInfo")
                                _faces++;
                            else if (n->GetName() == "subModel")
                                _submodels++;
                            else if (n->GetName() == "stateInfo")
                                _states++;
                        }
                        _message += "\nFaces: " + std::to_string(_faces) + " States: " + std::to_string(_states) + " Submodels: " + std::to_string(_submodels);
                    }
				}
            }
		}
	}
}

void RemapModelProperties::Compare(RemapModelProperties& original)
{
    // compare dimensions
    if (_w != original.GetWidth() || _h != original.GetHeight() || _d != original.GetDepth()) {
        _ok = false;
        _message += "\n\nModel dimensions do not match.";
    } else {
        uint32_t remapped = 0;

        // compare pixels are all in the same place
        auto it1 = _data.begin();
        auto it2 = original.GetData().begin();

        while (it1 != _data.end() && it2 != original.GetData().end()) {
            if ((*it1 == 0 && *it2 != 0) || (*it1 != 0 && *it2 == 0)) {
                _message += "\n\nAt least one pixel is located in a different cell in the model grid Original: "+std::to_string(*it2)+" New: "+std::to_string(*it1)+".";
                _ok = false;
                break;
            }

            if (*it1 != *it2)
                ++remapped;

            ++it1;
            ++it2;
        }
        if (remapped == 0) {
            _message += "\n\nNo pixels are different between the models. Did you choose the correct model files?";
            _ok = false;
        } else {
            _message += "\n\nProcess will remap " + std::to_string(remapped) + " pixels.";
        }
    }
}

void RemapModelProperties::Remap(RemapModelProperties& original)
{
    // remove existing faces, states and submodels

    std::list<wxXmlNode*> toremove;
    for (auto n = _xmodel.GetRoot()->GetChildren(); n != nullptr; n = n->GetNext()) {
        if (n->GetName() == "faceInfo" || n->GetName() == "subModel" || n->GetName() == "stateInfo") {
            toremove.push_back(n);
        }
    }
    for (const auto& it : toremove) {
        _xmodel.GetRoot()->RemoveChild(it);
    }

    std::map<uint32_t, uint32_t> mapping;
    auto it1 = _data.begin();
    auto it2 = original.GetData().begin();

    while (it1 != _data.end() && it2 != original.GetData().end()) {

        if (*it1 != 0 && *it2 != 0) {
            mapping[*it2] = *it1;
        }

        ++it1;
        ++it2;
    }

    for (auto n = original.GetXml()->GetChildren(); n != nullptr; n = n->GetNext()) {
        if (n->GetName() == "faceInfo") {
            wxXmlNode* nn = new wxXmlNode(*n);

            RemapNodes(nn, "Eyes-Closed", mapping);
            RemapNodes(nn, "Eyes-Closed2", mapping);
            RemapNodes(nn, "Eyes-Closed3", mapping);
            RemapNodes(nn, "Eyes-Open", mapping);
            RemapNodes(nn, "Eyes-Open2", mapping);
            RemapNodes(nn, "Eyes-Open3", mapping);
            RemapNodes(nn, "FaceOutline", mapping);
            RemapNodes(nn, "FaceOutline2", mapping);
            RemapNodes(nn, "Mouth-AI", mapping);
            RemapNodes(nn, "Mouth-E", mapping);
            RemapNodes(nn, "Mouth-FV", mapping);
            RemapNodes(nn, "Mouth-L", mapping);
            RemapNodes(nn, "Mouth-MBP", mapping);
            RemapNodes(nn, "Mouth-O", mapping);
            RemapNodes(nn, "Mouth-U", mapping);
            RemapNodes(nn, "Mouth-WQ", mapping);
            RemapNodes(nn, "Mouth-etc", mapping);
            RemapNodes(nn, "Mouth-rest", mapping);

            _xmodel.GetRoot()->AddChild(nn);
        } else if (n->GetName() == "stateInfo") {
            wxXmlNode* nn = new wxXmlNode(*n);

            for (uint32_t i = 0; i < 400; ++i) {
                auto attr = "s" + std::to_string(i);
                if (nn->HasAttribute(attr)) {
                    RemapNodes(nn, attr, mapping);
                }
            }

            _xmodel.GetRoot()->AddChild(nn);
        } else if (n->GetName() == "subModel") {
            wxXmlNode* nn = new wxXmlNode(*n);

            for (uint32_t i = 0; i < 1000; ++i) {
                auto attr = "line" + std::to_string(i);
                if (nn->HasAttribute(attr)) {
                    RemapNodes(nn, attr, mapping);
                }
            }

            _xmodel.GetRoot()->AddChild(nn);
        }
    }
}

void RemapModelProperties::Save(wxWindow* parent)
{
    wxFileName fn(_filename);
    auto newfn = fn.GetPath() + "/" + fn.GetName() + "_REMAP." + fn.GetExt();

    _xmodel.Save(newfn);

    wxMessageBox("xModel file " + newfn + " created.", "xModel file saved", 5L, parent);
}
