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

void ModelRemap::OnButton_GenerateClick(wxCommandEvent& event) {
    newWiringProperties.Remap(originalProperties);
    newWiringProperties.Save(this);
    EndDialog(wxCLOSE);
}

void ModelRemap::OnFilePickerCtrl_OriginalFileChanged(wxFileDirPickerEvent& event) {
    ValidateWindow();
}

void ModelRemap::OnFilePickerCtrl_NewWiringFileChanged(wxFileDirPickerEvent& event) {
    ValidateWindow();
}

void ModelRemap::ValidateWindow() {
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

void RemapModelProperties::RemapNodes(pugi::xml_node n, const std::string& attr, const std::map<uint32_t, uint32_t>& mapping) {
    wxString str = n.attribute(attr).as_string();

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
        } else if (r == "") {
            nl.push_back(0);
        } else {
            nl.push_back(wxAtoi(r));
        }
    }

    std::vector<uint32_t> nnl;
    for (const auto it : nl) {
        if (mapping.find(it) == mapping.end()) {
            // didnt find value, check if a placeholder
            if (it == 0) {
                nnl.push_back(0);
            }
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

    n.remove_attribute(attr);
    n.append_attribute(attr) = stro;
}

void RemapModelProperties::ParseData(const std::string& data) {
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

void RemapModelProperties::ParseCompressedData(const std::string& data) {
    _data.assign((size_t)_w * _h * _d, 0);

    // format: node,row,col[,layer];...  (only non-empty cells are stored)
    auto nodeStrings = wxSplit(data, ';');
    for (const auto& n : nodeStrings) {
        auto parts = wxSplit(n, ',');
        if (parts.size() >= 3) {
            uint32_t node = wxAtoi(parts[0]);
            uint32_t row = wxAtoi(parts[1]);
            uint32_t col = wxAtoi(parts[2]);
            uint32_t layer = (parts.size() >= 4) ? wxAtoi(parts[3]) : 0;
            if (row < _h && col < _w && layer < _d) {
                _data[layer * _h * _w + row * _w + col] = node;
            }
        }
    }
}

void RemapModelProperties::Load(const std::string& filename) {
    _ok = false;
    _faces = 0;
    _states = 0;
    _submodels = 0;
    _w = 0;
    _h = 0;
    _d = 0;
    _filename = filename;
    _message = "";
    _modelNode = pugi::xml_node();
    if (wxFile::Exists(filename)) {
        _xmodel.load_file(filename.c_str());
        pugi::xml_node root = _xmodel.document_element();
        if (root) {
            _message = filename;

            // Resolve the actual model node — two supported file shapes:
            //   Old: <custommodel .../>  (root is the model)
            //   New: <models type="exported"><model DisplayAs="Custom" .../>
            pugi::xml_node modelNode;
            if (std::string_view(root.name()) == "custommodel") {
                modelNode = root;
            } else if (std::string_view(root.name()) == "models") {
                for (pugi::xml_node child = root.first_child(); child; child = child.next_sibling()) {
                    if (std::string_view(child.name()) == "model" &&
                        std::string_view(child.attribute("DisplayAs").as_string()) == "Custom") {
                        modelNode = child;
                        break;
                    }
                }
                if (!modelNode) {
                    _message += "\nNo Custom model found in file.";
                }
            } else {
                _message += std::string("\nModel is not a custom model: ") + root.name();
            }

            if (modelNode) {
                _modelNode = modelNode;
                // extract model properties
                // dimensions
                _w = !modelNode.attribute("CustomWidth").empty() ? modelNode.attribute("CustomWidth").as_int(0) : modelNode.attribute("parm1").as_int(0);
                _h = !modelNode.attribute("CustomHeight").empty() ? modelNode.attribute("CustomHeight").as_int(0) : modelNode.attribute("parm2").as_int(0);
                _d = modelNode.attribute("Depth").as_int(1);

                _message += "\nW: " + std::to_string(_w) + " H: " + std::to_string(_h) + " D: " + std::to_string(_d);

                if (_w == 0 || _h == 0 || _d == 0) {
                    _message += "\nAt least one dimension invalid.";
                } else {
                    // number of pixels — support both old (CustomModel) and new (CustomModelCompressed) formats
                    std::string compressedData = modelNode.attribute("CustomModelCompressed").as_string();
                    if (!compressedData.empty()) {
                        ParseCompressedData(compressedData);
                    } else {
                        ParseData(modelNode.attribute("CustomModel").as_string());
                    }

                    uint32_t cnt = 0;
                    for (const auto& it : _data) {
                        if (it != 0) {
                            cnt++;
                        }
                    }

                    if (cnt == 0) {
                        _message += "\nNo pixels.";
                    } else {
                        _message += "\nPixels: " + std::to_string(cnt);
                        _ok = true;

                        for (pugi::xml_node n = modelNode.first_child(); n; n = n.next_sibling()) {
                            std::string_view nname = n.name();
                            if (nname == "faceInfo")
                                _faces++;
                            else if (nname == "subModel")
                                _submodels++;
                            else if (nname == "stateInfo")
                                _states++;
                        }
                        _message += "\nFaces: " + std::to_string(_faces) + " States: " + std::to_string(_states) + " Submodels: " + std::to_string(_submodels);
                    }
                }
            }
        }
    }
}

void RemapModelProperties::Compare(RemapModelProperties& original) {
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
                _message += "\n\nAt least one pixel is located in a different cell in the model grid Original: " + std::to_string(*it2) + " New: " + std::to_string(*it1) + ".";
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

void RemapModelProperties::Remap(RemapModelProperties& original) {
    // remove existing faces, states and submodels

    pugi::xml_node root = _modelNode;
    std::vector<pugi::xml_node> toremove;
    for (pugi::xml_node n = root.first_child(); n; n = n.next_sibling()) {
        std::string_view nname = n.name();
        if (nname == "faceInfo" || nname == "subModel" || nname == "stateInfo") {
            toremove.push_back(n);
        }
    }
    for (const auto& it : toremove) {
        root.remove_child(it);
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

    pugi::xml_node origXml = original.GetXml();
    for (pugi::xml_node n = origXml.first_child(); n; n = n.next_sibling()) {
        std::string_view nname = n.name();
        if (nname == "faceInfo") {
            pugi::xml_node nn = root.append_copy(n);

            static const std::vector<std::string> faceAttrs = {
                "Eyes-Closed", "Eyes-Closed2", "Eyes-Closed3",
                "Eyes-Open", "Eyes-Open2", "Eyes-Open3",
                "FaceOutline", "FaceOutline2",
                "Mouth-AI", "Mouth-E", "Mouth-FV", "Mouth-L", "Mouth-MBP",
                "Mouth-O", "Mouth-U", "Mouth-WQ", "Mouth-etc", "Mouth-rest"
            };
            for (const auto& fa : faceAttrs) {
                if (!nn.attribute(fa.c_str()).empty()) {
                    RemapNodes(nn, fa, mapping);
                }
            }
        } else if (nname == "stateInfo") {
            pugi::xml_node nn = root.append_copy(n);

            // Collect names first — RemapNodes removes+reappends the attribute, which
            // would cause a by-handle iterator to visit the reappended copy a second time.
            std::vector<std::string> toRemap;
            for (pugi::xml_attribute attr = nn.first_attribute(); attr; attr = attr.next_attribute()) {
                std::string attrName = attr.name();
                if (attrName.size() >= 2 && attrName[0] == 's') {
                    bool allDigits = true;
                    for (size_t k = 1; k < attrName.size(); ++k) {
                        if (!std::isdigit((unsigned char)attrName[k])) {
                            allDigits = false;
                            break;
                        }
                    }
                    if (allDigits)
                        toRemap.push_back(attrName);
                }
            }
            for (const auto& attrName : toRemap) {
                RemapNodes(nn, attrName, mapping);
            }
        } else if (nname == "subModel") {
            pugi::xml_node nn = root.append_copy(n);

            for (uint32_t i = 0; i < 1000; ++i) {
                auto attr = "line" + std::to_string(i);
                if (!nn.attribute(attr).empty()) {
                    RemapNodes(nn, attr, mapping);
                }
            }
        }
    }
}

void RemapModelProperties::Save(wxWindow* parent) {
    wxFileName fn(_filename);
    auto newfn = fn.GetPath() + "/" + fn.GetName() + "_REMAP." + fn.GetExt();

    _xmodel.save_file(newfn.ToStdString().c_str());

    wxMessageBox("xModel file " + newfn + " created.", "xModel file saved", 5L, parent);
}
