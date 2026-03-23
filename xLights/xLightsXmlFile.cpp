/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include <wx/tokenzr.h>
#include <wx/regex.h>
#include <wx/numdlg.h>
#include <wx/zipstrm.h>
#include <wx/wfstream.h>
#include <wx/dir.h>
#include <wx/textfile.h>
#include <wx/mstream.h>
#include <wx/base64.h>
#include <zstd.h>

#include "../include/spxml-0.5/spxmlparser.hpp"
#include "../include/spxml-0.5/spxmlevent.hpp"

#include "xLightsXmlFile.h"
#include "xLightsMain.h"
#include "OptionChooser.h"
#include "effects/EffectManager.h"
#include "effects/RenderableEffect.h"
#include "xLightsVersion.h"
#include "UtilFunctions.h"
#include "sequencer/TimeLine.h"
#include "Vixen3.h"
#include "ExternalHooks.h"

#include <log4cpp/Category.hh>

#define string_format wxString::Format

//     #define USE_COMPRESSION

const wxString xLightsXmlFile::ERASE_MODE = "<rendered: erase-mode>";
const wxString xLightsXmlFile::CANVAS_MODE = "<rendered: canvas-mode>";


const std::array<std::string, (int)HEADER_INFO_TYPES::NUM_TYPES> HEADER_STRINGS = {
    "author",
    "author-email",
    "author-website",
    "song",
    "artist",
    "album",
    "MusicURL",
    "comment"
};

xLightsXmlFile::xLightsXmlFile(const wxFileName& filename, uint32_t frameMS) :
    wxFileName(filename),
    version_string(wxEmptyString),
    seq_duration(30.0),
    media_file(wxEmptyString),
    seq_type("Animation"), // default to animation
    seq_timing("50 ms"),
    image_dir(wxEmptyString),
    supports_model_blending(true),
    is_open(false),
    was_converted(false),
    sequence_loaded(false),
    audio(nullptr)
{
    if (frameMS != 0) {
        seq_timing = wxString::Format("%d ms", frameMS);
    }
    CreateNew();
}

xLightsXmlFile::~xLightsXmlFile()
{
    models.Clear();
    timing_list.Clear();
    if (audio != nullptr) {
        ValueCurve::SetAudio(nullptr);
        delete audio;
        audio = nullptr;
    }
}

bool xLightsXmlFile::IsXmlSequence(wxFileName& fname)
{
    char buf[1024];
    wxFile file(fname.GetFullPath());
    int i = file.Read(buf, 1024);
    file.Close();
    wxString bufs(buf, i);
    if (bufs.Contains("<xsequence")) {
        return true;
    }
    return false;
}

bool xLightsXmlFile::IsV3Sequence() const
{
    char buf[1024];
    wxFile file(GetFullPath());
    int i = file.Read(buf, 1024);
    file.Close();
    if ((wxString(buf, i).Contains("<xsequence")) &&
        (wxString(buf, i).Contains("<tr>"))) {
        return true;
    }
    return false;
}

bool xLightsXmlFile::NeedsTimesCorrected() const
{
    char buf[1024];
    wxFile file(GetFullPath());
    int i = file.Read(buf, 1024);
    file.Close();
    if ((wxString(buf, i).Contains("<xsequence")) &&
        (wxString(buf, i).Contains("FixedPointTiming"))) {
        return false;
    }
    return true;
}

int xLightsXmlFile::GetLastView() const
{
    wxXmlNode* root = seqDocument.GetRoot();

    for (wxXmlNode* e = root->GetChildren(); e != nullptr; e = e->GetNext()) {
        if (e->GetName() == "lastView") {
            wxString last_view_string = e->GetNodeContent();
            return wxAtoi(last_view_string);
        }
    }
    return 0;
}

wxXmlNode* xLightsXmlFile::GetPalettesNode() const
{
    wxXmlNode* root = seqDocument.GetRoot();

    for (wxXmlNode* e = root->GetChildren(); e != nullptr; e = e->GetNext()) {
        if (e->GetName() == "ColorPalettes") {
            return e;
        }
    }
    return nullptr;
}

bool xLightsXmlFile::SaveCopy() const
{
    wxString archive_dir = xLightsFrame::CurrentDir + GetPathSeparators() + "ArchiveV3";

    if (wxDir::Exists(archive_dir) == false) {
        if (!wxDir::Make(archive_dir)) return false;
    }

    wxRenameFile(GetFullPath(), archive_dir + GetPathSeparators() + GetFullName());

    return true;
}

void xLightsXmlFile::SetSequenceType(const wxString& type)
{
    seq_type = type;

    wxXmlNode* root = seqDocument.GetRoot();

    for (wxXmlNode* e = root->GetChildren(); e != nullptr; e = e->GetNext()) {
        if (e->GetName() == "head") {
            for (wxXmlNode* element = e->GetChildren(); element != nullptr; element = element->GetNext()) {
                if (element->GetName() == "sequenceType") {
                    SetNodeContent(element, seq_type);
                }
            }
        }
    }
    if (type == "Animation") {
        SetMediaFile("", "", false);
        if (audio != nullptr) {
            ValueCurve::SetAudio(nullptr);
            delete audio;
            audio = nullptr;
        }
    }
}

int xLightsXmlFile::GetFrameMS() const
{
    return wxAtoi(seq_timing);
}

int xLightsXmlFile::GetFrequency() const
{
    int freq_ms = wxAtoi(seq_timing);
    return (int)(1000 / freq_ms);
}

void xLightsXmlFile::SetSequenceTiming(const wxString& timing)
{
    static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    logger_base.info("Sequence timing set to " + timing);

    seq_timing = timing;

    wxXmlNode* root = seqDocument.GetRoot();

    // looking to work out if this is why i have seen a crash in this function
    if (root == nullptr) {
        logger_base.crit("SetSequenceTiming is about to crash because sequence XML document has no root. Strange!");
    }

    for (wxXmlNode* e = root->GetChildren(); e != nullptr; e = e->GetNext()) {
        if (e->GetName() == "head") {
            for (wxXmlNode* element = e->GetChildren(); element != nullptr; element = element->GetNext()) {
                if (element->GetName() == "sequenceTiming") {
                    SetNodeContent(element, seq_timing);
                }
            }
        }
    }
}

void xLightsXmlFile::SetMediaFile(const wxString& ShowDir, const wxString& filename, bool overwrite_tags)
{
    static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));

    media_file = FixFile(ShowDir, filename);

    wxXmlNode* root = seqDocument.GetRoot();

    for (wxXmlNode* e = root->GetChildren(); e != nullptr; e = e->GetNext()) {
        if (e->GetName() == "head") {
            for (wxXmlNode* element = e->GetChildren(); element != nullptr; element = element->GetNext()) {
                if (element->GetName() == "mediaFile") {
                    SetNodeContent(element, media_file);
                }
            }
        }
    }

    if (audio != nullptr) {
        ValueCurve::SetAudio(nullptr);
        delete audio;
        audio = nullptr;
    }

    ObtainAccessToURL(filename.ToStdString());
    if ((filename != wxEmptyString) && FileExists(filename) && wxIsReadable(filename)) {
        logger_base.debug("SetMediaFile: Creating audio manager");
        audio = new AudioManager(std::string(filename.c_str()), GetFrameMS());

        if (audio != nullptr) {
            ValueCurve::SetAudio(audio);
            logger_base.info("SetMediaFile: Audio loaded. Audio frame interval %dms. Our frame interval %dms", audio->GetFrameInterval(), GetFrameMS());
            if (audio->GetFrameInterval() < 0 && GetFrameMS() > 0) {
                audio->SetFrameInterval(GetFrameMS());
            }
        }
    }

    if (overwrite_tags) {
        SetMetaMP3Tags();
    }
}

void xLightsXmlFile::ClearMediaFile()
{
    if (audio != nullptr) {
        ValueCurve::SetAudio(nullptr);
        delete audio;
        audio = nullptr;
    }

    wxXmlNode* root = seqDocument.GetRoot();

    bool done = false;
    for (wxXmlNode* e = root->GetChildren(); !done && e != nullptr; e = e->GetNext()) {
        if (e->GetName() == "head") {
            for (wxXmlNode* element = e->GetChildren(); !done && element != nullptr; element = element->GetNext()) {
                if (element->GetName() == "mediaFile") {
                    SetNodeContent(element, "");
                    done = true;
                }
            }
        }
    }

    media_file = "";
    SetHeaderInfo(HEADER_INFO_TYPES::SONG, "");
    SetHeaderInfo(HEADER_INFO_TYPES::ARTIST, "");
    SetHeaderInfo(HEADER_INFO_TYPES::ALBUM, "");
    SetHeaderInfo(HEADER_INFO_TYPES::URL, "");
}

void xLightsXmlFile::SetRenderMode(const wxString& mode)
{
    for (int i = 0; i < mDataLayers.GetNumLayers(); i++) {
        if (mDataLayers.GetDataLayer(i)->GetName() == "Nutcracker") {
            mDataLayers.GetDataLayer(i)->SetDataSource(mode);
            return;
        }
    }
}

wxString xLightsXmlFile::GetRenderMode()
{
    for (int i = 0; i < mDataLayers.GetNumLayers(); i++) {
        if (mDataLayers.GetDataLayer(i)->GetName() == "Nutcracker") {
            return mDataLayers.GetDataLayer(i)->GetDataSource();
        }
    }
    return ERASE_MODE;
}

void xLightsXmlFile::AddDisplayElement(const wxString& name, const wxString& type, const wxString& visible, const wxString& collapsed, const wxString& active, const wxString& renderDisabled)
{
    wxXmlNode* root = seqDocument.GetRoot();

    for (wxXmlNode* e = root->GetChildren(); e != nullptr; e = e->GetNext()) {
        if (e->GetName() == "DisplayElements") {
            wxXmlNode* child = AddChildXmlNode(e, "Element");
            child->AddAttribute("collapsed", collapsed);
            if (renderDisabled == "1") {
                child->AddAttribute("RenderDisabled", renderDisabled);
            }
            child->AddAttribute("active", active);
            child->AddAttribute("visible", visible);
            child->AddAttribute("type", type);
            child->AddAttribute("name", name);
            break;
        }
    }
}

void xLightsXmlFile::AddTimingDisplayElement(const wxString& name, const wxString& visible, const wxString& active, const wxString &subType)
{
    wxXmlNode* root = seqDocument.GetRoot();

    for (wxXmlNode* e = root->GetChildren(); e != nullptr; e = e->GetNext()) {
        if (e->GetName() == "DisplayElements") {
            wxXmlNode* insert_node = nullptr;
            for (wxXmlNode* element = e->GetChildren(); element != nullptr; element = element->GetNext()) {
                wxString attr;
                element->GetAttribute("type", &attr);
                if (attr != "timing") {
                    insert_node = element;
                    break;
                }
            }
            wxXmlNode* child;
            if (insert_node != nullptr) {
                child = InsertChildXmlNode(e, insert_node, "Element");
            }
            else {
                wxXmlNode* first_child = e->GetChildren();
                if (first_child != nullptr) {
                    child = InsertChildXmlNode(e, first_child, "Element");
                }
                else {
                    child = AddChildXmlNode(e, "Element");
                }
            }
            child->AddAttribute("active", active);
            child->AddAttribute("visible", visible);
            child->AddAttribute("type", "timing");
            child->AddAttribute("name", name);
            if (subType != "") {
                child->AddAttribute("subType", subType);
            }
            timing_list.push_back(name);
            break;
        }
    }
}

int xLightsXmlFile::AddColorPalette(StringIntMap& paletteCache, const wxString& palette)
{
    int cnt = 0;
    wxXmlNode* root = seqDocument.GetRoot();

    int size = paletteCache.size();
    int i = paletteCache[palette];
    if (i != 0) {
        return i - 1;
    }

    for (wxXmlNode* e = root->GetChildren(); e != nullptr; e = e->GetNext()) {
        if (e->GetName() == "ColorPalettes") {
            cnt = size;
            AddChildXmlNode(e, "ColorPalette", palette);
            paletteCache[palette] = cnt + 1;
            return cnt;
        }
    }
    wxXmlNode* child = AddChildXmlNode(root, "ColorPalettes");
    AddChildXmlNode(child, "ColorPalette", palette);
    paletteCache[palette] = 1;
    return cnt;
}

wxXmlNode* xLightsXmlFile::AddElement(const wxString& name, const wxString& type)
{
    wxXmlNode* root = seqDocument.GetRoot();
    wxXmlNode* child = nullptr;

    for (wxXmlNode* e = root->GetChildren(); e != nullptr; e = e->GetNext()) {
        if (e->GetName() == "ElementEffects") {
            child = AddChildXmlNode(e, "Element");
            child->AddAttribute("type", type);
            child->AddAttribute("name", name);
            break;
        }
    }

    return child;
}

wxXmlNode* xLightsXmlFile::AddFixedTiming(const wxString& name, const wxString& timing)
{
    wxXmlNode* root = seqDocument.GetRoot();
    wxXmlNode* child = nullptr;

    for (wxXmlNode* e = root->GetChildren(); e != nullptr; e = e->GetNext()) {
        if (e->GetName() == "ElementEffects") {
            child = AddChildXmlNode(e, "Element");
            child->AddAttribute("fixed", timing);
            child->AddAttribute("type", "timing");
            child->AddAttribute("name", name);
            break;
        }
    }

    return child;
}

static wxString SplitPalette(wxString& data)
{
    wxString settings = data;
    data = "";

    wxString color;
    while (!settings.IsEmpty()) {
        wxString before = settings.BeforeFirst(',');
        settings = settings.AfterFirst(',');
        if (before.StartsWith("C_")) {
            color += "," + before;
        }
        else {
            data += "," + before;
        }
    }
    if (!color.IsEmpty()) {
        color = color.AfterFirst(',');
    }
    if (!data.IsEmpty()) {
        data = data.AfterFirst(',');
    }
    return color;
}

void xLightsXmlFile::AddEffect(wxXmlNode* node,
    StringIntMap& paletteCache,
    const wxString& name,
    const wxString& d,
    const wxString& protection,
    const wxString& selected,
    const wxString& id,
    const wxString& start_time,
    const wxString& end_time)
{
    wxString data = d;
    wxString palette = SplitPalette(data);
    int p = AddColorPalette(paletteCache, palette);
    wxXmlNode* effect = AddChildXmlNode(node, "Effect", data);
    effect->AddAttribute("name", name);
    effect->AddAttribute("protected", protection);
    effect->AddAttribute("selected", selected);
    if (id != "0") {
        effect->AddAttribute("id", id);
    }
    effect->AddAttribute("startTime", start_time);
    effect->AddAttribute("endTime", end_time);
    effect->AddAttribute("palette", string_format("%d", p));
}

void xLightsXmlFile::AddTimingEffect(wxXmlNode* node,
    const wxString& label,
    const wxString& protection,
    const wxString& selected,
    const wxString& start_time,
    const wxString& end_time)
{
    wxXmlNode* effect = AddChildXmlNode(node, "Effect");
    effect->AddAttribute("label", label);
    effect->AddAttribute("protected", protection);
    effect->AddAttribute("selected", selected);
    effect->AddAttribute("startTime", start_time);
    effect->AddAttribute("endTime", end_time);
}

void xLightsXmlFile::UpdateNextId(const wxString& value)
{
    wxXmlNode* root = seqDocument.GetRoot();

    for (wxXmlNode* e = root->GetChildren(); e != nullptr; e = e->GetNext()) {
        if (e->GetName() == "nextid") {
            SetNodeContent(e, value);
            break;
        }
    }
}

static wxString GetSetting(const wxString& setting, const wxString& text)
{
    wxString settings = text;
    while (!settings.IsEmpty()) {
        wxString before = settings.BeforeFirst(',');
        if (before.Contains(setting)) {
            wxString val = before.AfterLast('=');
            return val;
        }
        settings = settings.AfterFirst(',');
    }
    return "";
}

wxXmlNode* xLightsXmlFile::AddChildXmlNode(wxXmlNode* node, const wxString& node_name, const wxString& node_data)
{
    wxXmlNode* new_node = new wxXmlNode(wxXML_ELEMENT_NODE, node_name);
    new wxXmlNode(new_node, wxXML_TEXT_NODE, "", node_data);
    node->AddChild(new_node);
    return new_node;
}

wxXmlNode* xLightsXmlFile::AddChildXmlNode(wxXmlNode* node, const wxString& node_name)
{
    wxXmlNode* new_node = new wxXmlNode(wxXML_ELEMENT_NODE, node_name);
    node->AddChild(new_node);
    return new_node;
}

wxXmlNode* xLightsXmlFile::InsertChildXmlNode(wxXmlNode* node, wxXmlNode* following_node, const wxString& node_name)
{
    wxXmlNode* new_node = new wxXmlNode(wxXML_ELEMENT_NODE, node_name);
    node->InsertChild(new_node, following_node);
    return new_node;
}

void xLightsXmlFile::SetNodeContent(wxXmlNode* node, const wxString& content)
{
    wxXmlNode* element = node->GetChildren();
    if (element != nullptr) {
        element->SetContent(content);
    }
    else {
        new wxXmlNode(node, wxXML_TEXT_NODE, "", content);
    }
}

const wxString& xLightsXmlFile::GetHeaderInfo(HEADER_INFO_TYPES node_type) const
{
    return header_info[static_cast<int>(node_type)];
}

void xLightsXmlFile::SetHeaderInfo(HEADER_INFO_TYPES name_name, const wxString& node_value)
{
    wxString clean_node_value(XmlSafe(node_value));
    wxXmlNode* root = seqDocument.GetRoot();

    for (wxXmlNode* e = root->GetChildren(); e != nullptr; e = e->GetNext()) {
        if (e->GetName() == "head") {
            for (wxXmlNode* element = e->GetChildren(); element != nullptr; element = element->GetNext()) {
                if (element->GetName() == HEADER_STRINGS[static_cast<int>(name_name)]) {
                    SetNodeContent(element, clean_node_value);
                    header_info[static_cast<int>(name_name)] = node_value;
                }
            }
        }
    }
}

void xLightsXmlFile::SetTimingSectionName(const std::string& section, const std::string& name)
{
    bool found = false;
    wxXmlNode* root = seqDocument.GetRoot();

    for (wxXmlNode* e = root->GetChildren(); e != nullptr && !found; e = e->GetNext()) {
        if (e->GetName() == "ElementEffects") {
            for (wxXmlNode* element = e->GetChildren(); element != nullptr && !found; element = element->GetNext()) {
                if (element->GetName() == "Element") {
                    wxString name_attr;
                    element->GetAttribute("type", &name_attr);
                    if (name_attr == "timing") {
                        element->GetAttribute("name", &name_attr);
                        if (name_attr == section) {
                            for (wxXmlAttribute* attr = element->GetAttributes(); attr != nullptr; attr = attr->GetNext()) {
                                if (attr->GetValue() == section) {
                                    attr->SetValue(name);
                                    int index = timing_list.Index(section);
                                    timing_list.Remove(section);
                                    timing_list.Insert(name, index);
                                    break;
                                }
                            }
                        }
                    }
                }
            }
        }
        else if (e->GetName() == "DisplayElements") {
            for (wxXmlNode* element = e->GetChildren(); element != nullptr && !found; element = element->GetNext()) {
                if (element->GetName() == "Element") {
                    wxString name_attr;
                    element->GetAttribute("type", &name_attr);
                    if (name_attr == "timing") {
                        element->GetAttribute("name", &name_attr);
                        if (name_attr == section) {
                            for (wxXmlAttribute* attr = element->GetAttributes(); attr != nullptr; attr = attr->GetNext()) {
                                if (attr->GetValue() == section) {
                                    attr->SetValue(name);
                                    break;
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}

void xLightsXmlFile::DeleteTimingSection(const std::string& section)
{
    bool found = false;
    wxXmlNode* root = seqDocument.GetRoot();

    for (wxXmlNode* e = root->GetChildren(); e != nullptr && !found; e = e->GetNext()) {
        if (e->GetName() == "DisplayElements") {
            for (wxXmlNode* element = e->GetChildren(); element != nullptr && !found; element = element ? element->GetNext() : nullptr) {
                if (element->GetName() == "Element") {
                    wxString attr;
                    element->GetAttribute("type", &attr);
                    if (attr == "timing") {
                        element->GetAttribute("name", &attr);
                        if (attr == section) {
                            e->RemoveChild(element);
                            delete element;
                            timing_list.Remove(section);
                            element = nullptr;
                            found = true;
                        }
                    }
                }
            }
        }
    }
    found = false;
    for (wxXmlNode* e = root->GetChildren(); e != nullptr && !found; e = e->GetNext()) {
        if (e->GetName() == "ElementEffects") {
            for (wxXmlNode* element = e->GetChildren(); element != nullptr && !found; element = element ? element->GetNext() : nullptr) {
                if (element->GetName() == "Element") {
                    wxString attr;
                    element->GetAttribute("type", &attr);
                    if (attr == "timing") {
                        element->GetAttribute("name", &attr);
                        if (attr == section) {
                            e->RemoveChild(element);
                            delete element;
                            element = nullptr;
                            found = true;
                        }
                    }
                }
            }
        }
    }
}

void xLightsXmlFile::CreateNew()
{
    // construct the new XML file
    wxXmlNode* root = new wxXmlNode(wxXML_ELEMENT_NODE, "xsequence");
    root->AddAttribute("BaseChannel", "0");
    root->AddAttribute("ChanCtrlBasic", "0");
    root->AddAttribute("ChanCtrlColor", "0");
    root->AddAttribute("FixedPointTiming", "1");
    root->AddAttribute("ModelBlending", "true");
    seqDocument.SetRoot(root);

    wxXmlNode* node = AddChildXmlNode(root, "head");
    AddChildXmlNode(node, "version", xlights_version_string);
    AddChildXmlNode(node, "author", GetHeaderInfo(HEADER_INFO_TYPES::AUTHOR));
    AddChildXmlNode(node, "author-email", GetHeaderInfo(HEADER_INFO_TYPES::AUTHOR_EMAIL));
    AddChildXmlNode(node, "author-website", GetHeaderInfo(HEADER_INFO_TYPES::WEBSITE));
    AddChildXmlNode(node, "song", GetHeaderInfo(HEADER_INFO_TYPES::SONG));
    AddChildXmlNode(node, "artist", GetHeaderInfo(HEADER_INFO_TYPES::ARTIST));
    AddChildXmlNode(node, "album", GetHeaderInfo(HEADER_INFO_TYPES::ALBUM));
    AddChildXmlNode(node, "MusicURL", GetHeaderInfo(HEADER_INFO_TYPES::URL));
    AddChildXmlNode(node, "comment", GetHeaderInfo(HEADER_INFO_TYPES::COMMENT));
    AddChildXmlNode(node, "sequenceTiming", seq_timing);
    AddChildXmlNode(node, "sequenceType", seq_type);
    AddChildXmlNode(node, "mediaFile", media_file);
    AddChildXmlNode(node, "sequenceDuration", GetSequenceDurationString());
    AddChildXmlNode(node, "imageDir", wxEmptyString);
    AddChildXmlNode(root, "DataLayers");
    mDataLayers.AddDataLayer("Nutcracker", "<auto-generated>", ERASE_MODE);
    AddChildXmlNode(root, "ColorPalettes");
    AddChildXmlNode(root, "DisplayElements");
    AddChildXmlNode(root, "ElementEffects");
    AddChildXmlNode(root, "TimingTags");
    AddChildXmlNode(root, "nextid", "1");

    version_string = xlights_version_string;
}

bool xLightsXmlFile::Open(const wxString& ShowDir, bool ignore_audio, const wxFileName &realFilename)
{
    if (!FileExists())
        return false;

    sequence_loaded = false;
    if (IsV3Sequence()) {
        wxMessageBox("Loading of xLights v3 Sequences is no longer supported.", "Error", wxOK | wxCENTRE |wxICON_ERROR, xLightsFrame::GetFrame());
        return false;
    }
    else if (IsXmlSequence(*this)) {
        return LoadSequence(ShowDir, ignore_audio, realFilename);
    }
    return false;
}

void ConvertAllChildrenToFixedPointTiming(wxXmlNode* node)
{
    while (node != nullptr) {
        if (node->GetName() == "Effect") {
            wxString start_time, end_time;
            node->GetAttribute("startTime", &start_time);
            node->GetAttribute("endTime", &end_time);
            double t1, t2;
            start_time.ToDouble(&t1);
            end_time.ToDouble(&t2);
            int new_start_time = (int)(t1 * 1000.0);
            int new_end_time = (int)(t2 * 1000.0);
            node->DeleteAttribute("startTime");
            node->DeleteAttribute("endTime");
            node->AddAttribute("startTime", wxString::Format("%d", new_start_time));
            node->AddAttribute("endTime", wxString::Format("%d", new_end_time));
        }
        else {
            ConvertAllChildrenToFixedPointTiming(node->GetChildren());
        }
        node = node->GetNext();
    }
}

void xLightsXmlFile::ConvertToFixedPointTiming()
{
    wxXmlNode* root = seqDocument.GetRoot();

    for (wxXmlNode* e = root->GetChildren(); e != nullptr; e = e->GetNext()) {
        if (e->GetName() == "ElementEffects") {
            ConvertAllChildrenToFixedPointTiming(e->GetChildren());
        }
    }
}

void xLightsXmlFile::UpdateMediaFileInXML(const wxString& filename)
{
    media_file = filename;

    wxXmlNode* root = seqDocument.GetRoot();

    for (wxXmlNode* e = root->GetChildren(); e != nullptr; e = e->GetNext()) {
        if (e->GetName() == "head") {
            for (wxXmlNode* element = e->GetChildren(); element != nullptr; element = element->GetNext()) {
                if (element->GetName() == "mediaFile") {
                    SetNodeContent(element, media_file);
                    return;
                }
            }
        }
    }
}

bool xLightsXmlFile::LoadSequence(const wxString& ShowDir, bool ignore_audio, const wxFileName &realFilename)
{
    static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    if (realFilename.GetFullPath() != GetFullPath()) {
        logger_base.info("LoadSequence: Loading sequence " + GetFullPath() + " from " + realFilename.GetFullPath());
    } else {
        logger_base.info("LoadSequence: Loading sequence " + GetFullPath());
    }

    if (!seqDocument.Load(realFilename.GetFullPath())) {
        logger_base.error("LoadSequence: XML file load failed.");
        return false;
    }
    is_open = true;

    wxXmlNode* root = seqDocument.GetRoot();
    for (wxXmlNode* e = root->GetChildren(); e != nullptr; e = e->GetNext()) {
        if (e->GetName() == "CompressedData") {
            int size = wxAtoi(e->GetAttribute("size"));
            wxMemoryBuffer memBuffer = wxBase64Decode(e->GetNodeContent());
            uint8_t* bytes = new uint8_t[size + 50];
            int sz = ZSTD_decompress(bytes, size + 50, memBuffer.GetData(), memBuffer.GetDataLen());

            wxMemoryInputStream in(bytes, sz);
            wxXmlDocument doc;
            doc.Load(in);
            wxXmlNode* c = doc.DetachRoot();
            root->InsertChildAfter(c, e);
            root->RemoveChild(e);
            delete e;
            e = c;
            delete[] bytes;
        }
    }
    supports_model_blending = "true" == root->GetAttribute("ModelBlending", "false");

    if (NeedsTimesCorrected()) {
        ConvertToFixedPointTiming();
        root->AddAttribute("FixedPointTiming", "1");
    }

    std::string mediaFileName;
    for (wxXmlNode* e = root->GetChildren(); e != nullptr; e = e->GetNext()) {
        if (e->GetName() == "head") {
            for (wxXmlNode* element = e->GetChildren(); element != nullptr; element = element->GetNext()) {
                if (element->GetName() == "version") {
                    version_string = element->GetNodeContent();
                }
                else if (element->GetName() == "author") {
                    SetHeaderInfo(HEADER_INFO_TYPES::AUTHOR, UnXmlSafe(element->GetNodeContent()));
                }
                else if (element->GetName() == "author-email") {
                    SetHeaderInfo(HEADER_INFO_TYPES::AUTHOR_EMAIL, UnXmlSafe(element->GetNodeContent()));
                }
                else if (element->GetName() == "author-website") {
                    SetHeaderInfo(HEADER_INFO_TYPES::WEBSITE, UnXmlSafe(element->GetNodeContent()));
                }
                else if (element->GetName() == "song") {
                    SetHeaderInfo(HEADER_INFO_TYPES::SONG, UnXmlSafe(element->GetNodeContent()));
                }
                else if (element->GetName() == "artist") {
                    SetHeaderInfo(HEADER_INFO_TYPES::ARTIST, UnXmlSafe(element->GetNodeContent()));
                }
                else if (element->GetName() == "album") {
                    SetHeaderInfo(HEADER_INFO_TYPES::ALBUM, UnXmlSafe(element->GetNodeContent()));
                }
                else if (element->GetName() == "MusicURL") {
                    SetHeaderInfo(HEADER_INFO_TYPES::URL, UnXmlSafe(element->GetNodeContent()));
                }
                else if (element->GetName() == "comment") {
                   SetHeaderInfo(HEADER_INFO_TYPES::COMMENT, UnXmlSafe(element->GetNodeContent()));
                }
                else if (element->GetName() == "sequenceTiming") {
                    seq_timing = element->GetNodeContent();
                    logger_base.debug("LoadSequence: Sequence timing loaded from XML file. %s", (const char*)seq_timing.c_str());
                }
                else if (element->GetName() == "sequenceType") {
                    seq_type = element->GetNodeContent();
                }
                else if (element->GetName() == "mediaFile") {
                    if (!ignore_audio) {
                        logger_base.debug("LoadSequence: mediaFile %s", (const char*)element->GetNodeContent().c_str());
                        media_file = FixFile(ShowDir, element->GetNodeContent());
                        if (media_file != element->GetNodeContent()) {
                            UpdateMediaFileInXML(media_file);
                            logger_base.debug("LoadSequence: mediaFile updated to %s", (const char*)media_file.c_str());
                        }
                        wxFileName mf = media_file;
                        if (audio != nullptr) {
                            logger_base.debug("LoadSequence: removing prior audio.");
                            ValueCurve::SetAudio(nullptr);
                            delete audio;
                            audio = nullptr;
                        }
                        if (::FileExists(mf) && mf.IsFileReadable()) {
                            mediaFileName = media_file.ToStdString();
                        }
                        else {
                            if (!::FileExists(mf)) {
                                logger_base.error("LoadSequence: audio file does not exist.");
                            }
                            else if (!mf.IsFileReadable()) {
                                logger_base.error("LoadSequence: audio file not readable.");
                            }
                        }
                    }
                }
                else if (element->GetName() == "sequenceDuration") {
                    SetSequenceDuration(element->GetNodeContent());
                }
                else if (element->GetName() == "imageDir") {
                    image_dir = FixFile(ShowDir, element->GetNodeContent());
                    if (image_dir != element->GetNodeContent()) element->SetContent(image_dir);
                }
            }
        }
        else if (e->GetName() == "ElementEffects") {
            for (wxXmlNode* element = e->GetChildren(); element != nullptr; element = element->GetNext()) {
                if (element->GetName() == "Element") {
                    wxString attr;
                    element->GetAttribute("type", &attr);
                    if (attr == "model") {
                        element->GetAttribute("name", &attr);
                        models.push_back(attr);
                    }
                    else if (attr == "timing") {
                        element->GetAttribute("name", &attr);
                        timing_list.push_back(attr);
                    }
                }
            }
        }
        else if (e->GetName() == "DataLayers") {
            for (wxXmlNode* element = e->GetChildren(); element != nullptr; element = element->GetNext()) {
                if (element->GetName() == "DataLayer") {
                    wxString name, source, data, num_frames, num_channels, channel_offset, lor_params;
                    element->GetAttribute("name", &name);
                    element->GetAttribute("source", &source);
                    element->GetAttribute("data", &data);
                    element->GetAttribute("num_frames", &num_frames);
                    element->GetAttribute("num_channels", &num_channels);
                    element->GetAttribute("channel_offset", &channel_offset);
                    element->GetAttribute("lor_params", &lor_params);
                    if (!data.StartsWith("<")) {
                        auto oldData = data;
                        data = FixFile("", data);
                        if (data != oldData) {
                            element->DeleteAttribute("data");
                            element->AddAttribute("data", data);
                        }
                    }
                    auto oldSource = source;
                    if (source != "<auto-generated>") {
                        source = FixFile("", source);
                        if (source != oldSource) {
                            element->DeleteAttribute("source");
                            element->AddAttribute("source", source);
                        }
                    }
                    if (name == "Nutcracker") {
                        mDataLayers.RemoveDataLayer(0);
                    }
                    DataLayer* new_data_layer = mDataLayers.AddDataLayer(name, source, data);
                    new_data_layer->SetNumFrames(atoi(num_frames.c_str()));
                    new_data_layer->SetNumChannels(atoi(num_channels.c_str()));
                    new_data_layer->SetChannelOffset(atoi(channel_offset.c_str()));
                    new_data_layer->SetLORConvertParams(atoi(lor_params.c_str()));
                }
            }
        }
    }

    if (mediaFileName != "") {
        ObtainAccessToURL(mediaFileName);
        logger_base.debug("LoadSequence: Creating audio manager");
        audio = new AudioManager(mediaFileName, GetFrameMS());
        ValueCurve::SetAudio(audio);
        logger_base.debug("LoadSequence: audio manager creation done");
    }
    else {
        logger_base.info("LoadSequence: No Audio loaded.");
    }

    logger_base.info("LoadSequence: Sequence timing interval %dms.", GetFrameMS());
    logger_base.info("LoadSequence: Sequence loaded.");

    return is_open;
}

void xLightsXmlFile::CleanUpEffects() const
{
    wxXmlNode* root = seqDocument.GetRoot();
    wxXmlNode* node = nullptr;

    for (wxXmlNode* e = root->GetChildren(); e != nullptr; e = e->GetNext()) {
        if (e->GetName() == "ElementEffects") {
            node = e;
            break;
        }
    }

    // connect timing gaps
    for (wxXmlNode* element = node->GetChildren(); element != nullptr; element = element->GetNext()) {
        for (wxXmlNode* layer = element->GetChildren(); layer != nullptr; layer = layer->GetNext()) {
            wxXmlNode* eff1 = layer->GetChildren();
            if (eff1 == nullptr) continue;
            wxXmlNode* eff2 = eff1->GetNext();
            if (eff2 == nullptr) continue;

            while (eff1 != nullptr && eff2 != nullptr) {
                wxString start_time;
                wxString end_time;
                eff1->GetAttribute("endTime", &end_time);
                eff2->GetAttribute("startTime", &start_time);
                if (end_time != start_time && start_time != "") {
                    for (wxXmlAttribute* attr = eff1->GetAttributes(); attr != nullptr; attr = attr->GetNext()) {
                        if (attr->GetName() == "endTime") {
                            attr->SetValue(start_time);
                            break;
                        }
                    }
                }
                eff1 = eff2;
                eff2 = eff1->GetNext();
            }
        }
    }

    // remove Effect 1 and slider is 0 , then delete effect 2
    // remove Effect 2 and slider is 100 , then delete effect 1
    for (wxXmlNode* e = node->GetChildren(); e != nullptr; e = e->GetNext()) {
        wxXmlNode* layer1 = e->GetChildren();
        if (layer1 == nullptr) continue;
        wxXmlNode* layer2 = layer1->GetNext();
        wxXmlNode* effect1 = layer1->GetChildren();
        wxXmlNode* effect2 = layer2->GetChildren();

        while (effect1 != nullptr && effect2 != nullptr) {
            wxString layer_effect_name;
            effect1->GetAttribute("name", &layer_effect_name);
            layer_effect_name = UnXmlSafe(layer_effect_name);

            // Capture next effects now in case we delete
            wxXmlNode* next_effect1 = effect1->GetNext();
            wxXmlNode* next_effect2 = effect2->GetNext();

            wxString content1 = effect1->GetNodeContent();

            wxString combine = GetSetting("T_CHOICE_LayerMethod", content1);
            wxString morph = GetSetting("T_CHECKBOX_LayerMorph", content1);
            wxString mix = GetSetting("T_SLIDER_EffectLayerMix", content1);

            if ((combine == "Effect 1" && morph == "0" && mix == "0") ||
                (combine == "Effect 2" && morph == "0" && mix == "100")) {
                layer2->RemoveChild(effect2);
                delete effect2;
            }
            else if ((combine == "Effect 2" && morph == "0" && mix == "0") ||
                (combine == "Effect 1" && morph == "0" && mix == "100")) {
                layer1->RemoveChild(effect1);
                delete effect1;
            }

            effect1 = next_effect1;
            effect2 = next_effect2;
        }
    }

    // remove "None" effects
    for (wxXmlNode* e = node->GetChildren(); e != nullptr; e = e->GetNext()) {
        for (wxXmlNode* layer = e->GetChildren(); layer != nullptr; layer = layer->GetNext()) {
            for (wxXmlNode* effect = layer->GetChildren(); effect != nullptr;) {
                wxString layer_effect_name;
                effect->GetAttribute("name", &layer_effect_name);
                layer_effect_name = UnXmlSafe(layer_effect_name);
                if (layer_effect_name == "None") {
                    wxXmlNode* node_to_delete = effect;
                    effect = effect->GetNext();
                    layer->RemoveChild(node_to_delete);
                    delete node_to_delete;
                }
                else {
                    effect = effect->GetNext();
                }
            }
        }
    }

    // remove empty layers
    for (wxXmlNode* e = node->GetChildren(); e != nullptr; e = e->GetNext()) {
        for (wxXmlNode* layer = e->GetChildren(); layer != nullptr;) {
            wxXmlNode* child = layer->GetChildren();
            if (child == nullptr) {
                wxXmlNode* node_to_delete = layer;
                layer = layer->GetNext();
                e->RemoveChild(node_to_delete);
                delete node_to_delete;
            }
            else {
                layer = layer->GetNext();
            }
        }
    }
}

wxString xLightsXmlFile::GetSequenceDurationString() const
{
    return string_format("%.3f", seq_duration);
}

void xLightsXmlFile::SetSequenceDurationMS(int length)
{
    SetSequenceDuration(length / 1000.0f);
}

void xLightsXmlFile::SetSequenceDuration(double length)
{
    SetSequenceDuration(string_format("%.3f",length));
}

void xLightsXmlFile::SetSequenceDuration(const wxString& length)
{
    length.ToDouble(&seq_duration);

    wxXmlNode* root = seqDocument.GetRoot();

    for (wxXmlNode* e = root->GetChildren(); e != nullptr; e = e->GetNext()) {
        if (e->GetName() == "head") {
            for (wxXmlNode* element = e->GetChildren(); element != nullptr; element = element->GetNext()) {
                if (element->GetName() == "sequenceDuration") {
                    SetNodeContent(element, length);
                    return;
                }
            }
        }
    }
}

wxString xLightsXmlFile::GetImageDir(wxWindow* parent)
{
    if (image_dir != wxEmptyString) {
        return image_dir;
    }

    wxDirDialog* dlg = new wxDirDialog(parent, _("Select Directory for storing image files for this sequence"), wxEmptyString, wxDD_DEFAULT_STYLE, wxDefaultPosition, wxDefaultSize, _T("wxDirDialog"));

    if (dlg->ShowModal() != wxID_OK) {
        return wxEmptyString;
    }

    SetImageDir(dlg->GetPath());
    return image_dir;
}

void xLightsXmlFile::SetImageDir(const wxString& dir)
{
    image_dir = dir;

    wxXmlNode* root = seqDocument.GetRoot();

    for (wxXmlNode* e = root->GetChildren(); e != nullptr; e = e->GetNext()) {
        if (e->GetName() == "head") {
            for (wxXmlNode* element = e->GetChildren(); element != nullptr; element = element->GetNext()) {
                if (element->GetName() == "imageDir") {
                    SetNodeContent(element, dir);
                    return;
                }
            }
            // need to create node
            AddChildXmlNode(e, "imageDir", image_dir);
        }
    }
}

void xLightsXmlFile::UpdateVersion()
{
    UpdateVersion(xlights_version_string);
}

void xLightsXmlFile::UpdateVersion(const std::string& version)
{
    wxXmlNode* root = seqDocument.GetRoot();

    for (wxXmlNode* e = root->GetChildren(); e != nullptr; e = e->GetNext()) {
        if (e->GetName() == "head") {
            for (wxXmlNode* element = e->GetChildren(); element != nullptr; element = element->GetNext()) {
                if (element->GetName() == "version") {
                    SetNodeContent(element, version);
                    return;
                }
            }
        }
    }
}

void xLightsXmlFile::ProcessAudacityTimingFiles( const wxArrayString& filenames, xLightsFrame* xLightsParent)
{
    wxTextFile f;
    wxString line;
    int r;

    for (size_t i = 0; i < filenames.Count(); ++i) {
        wxFileName next_file(filenames[i]);

        if (!f.Open(next_file.GetFullPath().c_str())) {
            //Add error dialog if open file failed
            return;
        }

        std::string filename = next_file.GetName().ToStdString();

        while (TimingAlreadyExists(filename, xLightsParent)) {
            filename += "_1";
        }

        EffectLayer* effectLayer = nullptr;
        wxXmlNode* layer = nullptr;
        if (sequence_loaded) {
            Element* element = xLightsParent->AddTimingElement(filename);
            effectLayer = element->GetEffectLayer(0);
        }
        else {
            AddTimingDisplayElement(filename, "1", "0");
            wxXmlNode* node = AddElement(filename, "timing");
            layer = AddChildXmlNode(node, "EffectLayer");
        }

        bool isTab = false;
        // scan the first 30 lines to see if it is tab delimited
        for (r = 0, line = f.GetFirstLine(); !f.Eof() && r < 30 && !isTab; line = f.GetNextLine(), r++) {
            if (line.Contains("\t")) {
                isTab = true;
            }
        }

        wxArrayString start_times;
        wxArrayString end_times;
        std::vector<std::string> labels;

        for (r = 0, line = f.GetFirstLine(); !f.Eof(); line = f.GetNextLine(), r++) {
            // remove comments
            if (line.Contains("#")) {
                int pos = line.Find("#");
                line.Truncate(pos);
            }

            while (!line.empty() && (line.Last() == ' ')) line.RemoveLast(); //trim trailing spaces
            if (line.empty()) {
                --r;    //skip blank lines; don't add grid row
                continue;
            }

            wxStringTokenizer tkz;
            if (isTab) {
                tkz = wxStringTokenizer(line, "\t");
            }
            else {
                tkz = wxStringTokenizer(line, " ");
            }
            start_times.push_back(tkz.GetNextToken()); //first column = start time
            //pull in lyrics or other label text
            end_times.push_back(tkz.GetNextToken()); //second column = end time;
            std::string label = tkz.GetNextToken().ToStdString(); //third column = label/text
            for (;;) //collect remaining tokens into label
            {
                std::string more = tkz.GetNextToken().ToStdString();
                if (more.empty()) break;
                label += " " + more;
            }
            labels.push_back(label); //third column = label/text

        }

        double time1;
        for (size_t j = 0; j < start_times.GetCount(); j++) {
            start_times[j].ToDouble(&time1);
            start_times[j] = string_format("%d", (int)(time1 * 1000.0));
            end_times[j].ToDouble(&time1);
            end_times[j] = string_format("%d", (int)(time1 * 1000.0));
        }

        for (size_t k = 0; k < start_times.GetCount(); ++k) {
            int startTime = TimeLine::RoundToMultipleOfPeriod(wxAtoi(start_times[k]), GetFrequency());
            int endTime = TimeLine::RoundToMultipleOfPeriod(wxAtoi(end_times[k]), GetFrequency());
            if (startTime == endTime) {
                if (k == start_times.GetCount() - 1) // last timing mark
                {
                    endTime = startTime + GetFrequency();
                }
                else {
                    endTime = TimeLine::RoundToMultipleOfPeriod(wxAtoi(start_times[k + 1]), GetFrequency());
                }
            }

            if (sequence_loaded) {
                effectLayer->AddEffect(0, labels[k], "", "", startTime, endTime, EFFECT_NOT_SELECTED, false);
            }
            else {
                AddTimingEffect(layer, labels[k], "0", "0", string_format("%d", startTime), string_format("%d", endTime));
            }
        }
    }
}

void xLightsXmlFile::ProcessLorTiming(const wxArrayString& filenames, xLightsFrame* xLightsParent)
{
    for (size_t i = 0; i < filenames.Count(); ++i )
    {
        wxFileName next_file(filenames[i]);

        wxFile f;
        if (!f.Open(next_file.GetFullPath().c_str()))
        {
            DisplayError(wxString::Format("LOR Timing: Failed to open file: '%s'", next_file.GetFullPath()).ToStdString());
            return;
        }
        f.Close();

        std::string filename = next_file.GetName().ToStdString();

        wxArrayString grid_times;
        wxArrayString timing_options;

        wxXmlDocument input_xml;
        if( !input_xml.Load(next_file.GetFullPath()) )
        {
            DisplayError(wxString::Format("LOR Timing: Failed to load XML file: '%s'", next_file.GetFullPath()).ToStdString());
            return;
        }

        wxXmlNode* input_root=input_xml.GetRoot();

        for(wxXmlNode* e=input_root->GetChildren(); e!=nullptr; e=e->GetNext() )
        {
            if (e->GetName() == "timingGrids")
            {
                for(wxXmlNode* grids=e->GetChildren(); grids!=nullptr; grids=grids->GetNext() )
                {
                    if (grids->GetName() == "timingGrid")
                    {
                        wxString grid_type = grids->GetAttribute("type");
                        if( grid_type == "freeform" )
                        {
                            wxString grid_name = grids->GetAttribute("name");
                            wxString grid_id = grids->GetAttribute("saveID");
                            if( grid_name == "" )
                            {
                                grid_name = "Unnamed" + grid_id;
                            }
                            timing_options.push_back(grid_name);
                        }
                    }
                }
            }
        }

        OptionChooser opt_dialog(xLightsParent);
        opt_dialog.SetInstructionText("Choose Timing Grid to use for timing import:");
        opt_dialog.SetOptions(timing_options);
        wxArrayString timing_grids;
        if (opt_dialog.ShowModal() == wxID_OK)
        {
            opt_dialog.GetSelectedOptions(timing_grids);
        }
        else
        {
            return;
        }

        for(wxXmlNode* e=input_root->GetChildren(); e!=nullptr; e=e->GetNext() )
        {
            if (e->GetName() == "timingGrids")
            {
                for(wxXmlNode* grids=e->GetChildren(); grids!=nullptr; grids=grids->GetNext() )
                {
                    if (grids->GetName() == "timingGrid")
                    {
                        std::string grid_name = grids->GetAttribute("name").ToStdString();
                        std::string grid_id = grids->GetAttribute("saveID").ToStdString();
                        if( grid_name == "" )
                        {
                            grid_name = "Unnamed" + grid_id;
                        }
                        for (size_t i1 = 0; i1 < timing_grids.GetCount(); i1++ )
                        {
                            if( grid_name == timing_grids[i1] )
                            {
                                std::string new_timing_name = UniqueTimingName(xLightsParent,  filename + ": " + grid_name);
                                EffectLayer* effectLayer = nullptr;
                                wxXmlNode* layer = nullptr;
                                if( sequence_loaded )
                                {
                                    Element* element = xLightsParent->AddTimingElement(new_timing_name);
                                    effectLayer = element->GetEffectLayer(0);
                                }
                                else
                                {
                                    AddTimingDisplayElement(new_timing_name, "1", "0" );
                                    wxXmlNode*  node = AddElement( new_timing_name, "timing" );
                                    layer = AddChildXmlNode(node, "EffectLayer");
                                }

                                grid_times.Clear();
                                for(wxXmlNode* effect=grids->GetChildren(); effect!=nullptr; effect=effect->GetNext() )
                                {
                                    wxString t1 = effect->GetAttribute("centisecond");
                                    int time = wxAtoi(t1) * 10;
                                    t1 = string_format("%d",time);
                                    grid_times.push_back(t1);
                                }

                                for (size_t k = 0; k < grid_times.GetCount()-1; ++k )
                                {
                                    int startTime = TimeLine::RoundToMultipleOfPeriod(wxAtoi(grid_times[k]),GetFrequency());
                                    int endTime = TimeLine::RoundToMultipleOfPeriod(wxAtoi(grid_times[k+1]),GetFrequency());
                                    if( sequence_loaded )
                                    {
                                        effectLayer->AddEffect(0,"","","",startTime,endTime,EFFECT_NOT_SELECTED,false);
                                    }
                                    else
                                    {
                                        AddTimingEffect(layer, "", "0", "0", string_format("%d", startTime), string_format("%d", endTime));
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}

wxString xLightsXmlFile::UniqueTimingName(xLightsFrame* xLightsParent, wxString name) const
{
    wxString testname = RemoveUnsafeXmlChars(name);
    int testnamenum = 1;
    bool ok;
    do
    {
        ok = true;
        int num_elements = xLightsParent->GetSequenceElements().GetElementCount();
        for (int i = 0; i < num_elements; ++i)
        {
            Element* element = xLightsParent->GetSequenceElements().GetElement(i);
            if (element->GetType() == ElementType::ELEMENT_TYPE_TIMING)
            {
                if (element->GetName() == testname)
                {
                    testname = name + wxString::Format("_%d", testnamenum++);
                    ok = false;
                    break;
                }
            }
        }
    } while (!ok);
    return testname;
}

void xLightsXmlFile::ProcessXTiming(wxXmlNode* node, xLightsFrame* xLightsParent)
{
    wxString name = UnXmlSafe(node->GetAttribute("name"));
    //wxString v = node->GetAttribute("SourceVersion");
    wxString st = node->GetAttribute("subType");

    name = UniqueTimingName(xLightsParent, name);

    Element* element = nullptr;
    EffectLayer* effectLayer = nullptr;
    wxXmlNode* layer = nullptr;
    wxXmlNode* timing = nullptr;
    if (sequence_loaded)
    {
        element = xLightsParent->AddTimingElement(std::string(name.c_str()), std::string(st.c_str()));
    }
    else
    {
        AddTimingDisplayElement(name, "1", "0");
        timing = AddElement(name, "timing");
    }

    int l = 0;
    for (wxXmlNode* layers = node->GetChildren(); layers != nullptr; layers = layers->GetNext())
    {
        if (layers->GetName() == "EffectLayer")
        {
            l++;
            if (sequence_loaded)
            {
                if (l == 1)
                {
                    effectLayer = element->GetEffectLayer(0);
                }
                else
                {
                    effectLayer = element->AddEffectLayer();
                }
            }
            else
            {
                layer = AddChildXmlNode(timing, "EffectLayer");
            }

            for (wxXmlNode* effects = layers->GetChildren(); effects != nullptr; effects = effects->GetNext())
            {
                if (effects->GetName() == "Effect")
                {
                    wxString label = UnXmlSafe(effects->GetAttribute("label"));
                    wxString start = effects->GetAttribute("starttime");
                    wxString end = effects->GetAttribute("endtime");
                    if (sequence_loaded)
                    {
                        int s = wxAtoi(start);
                        int e = wxAtoi(end);

                        if (s % GetFrameMS() != 0)
                        {
                            s -= s % GetFrameMS();
                        }
                        if (e % GetFrameMS() != 0)
                        {
                            e -= e % GetFrameMS();
                        }
                        effectLayer->AddEffect(0, std::string(label.c_str()), "", "", s, e, EFFECT_NOT_SELECTED, false);
                    }
                    else
                    {
                        AddTimingEffect(layer, std::string(label.c_str()), "0", "0", start, end);
                    }
                }
            }
        }
    }
}

void xLightsXmlFile::ProcessXTiming(const wxArrayString& filenames, xLightsFrame* xLightsParent)
{
    wxTextFile f;

    for (size_t i = 0; i < filenames.Count(); ++i)
    {
        wxFileName next_file(filenames[i]);
        if (!f.Open(next_file.GetFullPath().c_str()))
        {
            DisplayError(wxString::Format("xTiming: Failed to open file: '%s'", next_file.GetFullPath()).ToStdString());
            return;
        }

        std::string filename = next_file.GetName().ToStdString();

        wxXmlDocument input_xml;
        if (!input_xml.Load(next_file.GetFullPath()))
        {
            DisplayError(wxString::Format("xTiming: Failed to load XML file: '%s'", next_file.GetFullPath()).ToStdString());
            return;
        }

        wxXmlNode* e = input_xml.GetRoot();

        if (e->GetName() == "timing")
        {
            ProcessXTiming(e, xLightsParent);
        }
        else if (e->GetName() == "timings")
        {
            for (wxXmlNode* node = e->GetChildren(); node != nullptr; node = node->GetNext())
            {
                if (node->GetName() == "timing")
                {
                    ProcessXTiming(node, xLightsParent);
                }
            }
        }
    }

    if (GetSequenceLoaded()) {
        GetTimingList(xLightsParent->GetSequenceElements());
    }
}

wxString RemoveTabs(const wxString& s, size_t tabs)
{
    wxString res = s;

    for (size_t i = 0; i < tabs; i++)
    {
        if (res[0] == '\t')
        {
            res = res.SubString(1, res.Length() - 1);
        }
    }
    return res;
}

void xLightsXmlFile::ProcessPapagayo( const wxArrayString& filenames, xLightsFrame* xLightsParent)
{
    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    wxTextFile f;

    for (size_t i = 0; i < filenames.Count(); ++i)
    {
        int linenum = 1;
        wxFileName next_file(filenames[i]);
        logger_base.info("Loading papagayo file " + std::string(next_file.GetFullPath().c_str()));

        if (!f.Open(next_file.GetFullPath().c_str()))
        {
            DisplayError("Failed to open file: " + next_file.GetFullPath());
            return;
        }

        wxString line = f.GetFirstLine();
        if (line.CmpNoCase("lipsync version 1"))
        {
            DisplayError(wxString::Format(_("Invalid papagayo file @line %d (header '%s')"), linenum, line.c_str()).ToStdString());
            return;
        }

        line = f.GetNextLine(); // filename which we ignore
        linenum++;

        wxRegEx number("^[0-9]+$");
        int samppersec = number.Matches(line = f.GetNextLine()) ? wxAtoi(line) : -1;
        linenum++;
        if (samppersec < 1)
        {
            DisplayError(wxString::Format(_("Invalid file @line %d ('%s' samples per sec)"), linenum, line.c_str()).ToStdString());
        }
        int ms = 1000 / samppersec;

        int maxframe = 4 * 60 * samppersec;
        if (GetMedia() != nullptr)
        {
            maxframe = GetMedia()->LengthMS() / ms;
        }
        int offset = wxGetNumberFromUser("Enter the number of frames to offset the papagayo data by", "", "Offset", 0, 0, maxframe, xLightsParent);

        int numsamp = number.Matches(line = f.GetNextLine()) ? wxAtoi(line) : -1;
        linenum++;
        if (numsamp < 1)
        {
            DisplayError(wxString::Format(_("Invalid file @line %d ('%s' song samples)"), linenum, line.c_str()).ToStdString());
        }

        int numvoices = number.Matches(line = f.GetNextLine()) ? wxAtoi(line) : -1;
        linenum++;
        if (numvoices < 1)
        {
            DisplayError(wxString::Format(_("Invalid file @line %d ('%s' voices)"), linenum, line.c_str()).ToStdString());
        }
        logger_base.info("    Voices %d", numvoices);

        for (int v = 1; v <= numvoices; ++v)
        {
            wxString name = wxString::Format("Voice %d", v);
            name = UniqueTimingName(xLightsParent, name);
            logger_base.info("    Loading voice %d into timing track %s.", v, (const char *)name.c_str());

            wxString voicename = f.GetNextLine();
            linenum++;
            if (voicename.empty())
            {
                DisplayError(wxString::Format(_("Missing voice# %d of %d"), v, numvoices).ToStdString());
                return;
            }

            f.GetNextLine(); //all phrases for voice, "|" delimiter; TODO: do we need to save this?
            linenum++;
            wxString desc = wxString::Format(_("voice# %d '%s' @line %d"), v, voicename, linenum);

            int numphrases = number.Matches(line = RemoveTabs(f.GetNextLine(),1)) ? wxAtoi(line) : -1;
            linenum++;
            if (numphrases < 0)
            {
                DisplayError(wxString::Format(_("Invalid file @line %d ('%s' phrases for %s)"), linenum, line.c_str(), desc.c_str()).ToStdString());
            }

            Element* element = nullptr;
            wxXmlNode* timing = nullptr, *l1 = nullptr, *l2 = nullptr, *l3 = nullptr;
            EffectLayer *el1 = nullptr, *el2 = nullptr, *el3 = nullptr;

            if (sequence_loaded)
            {
                element = xLightsParent->AddTimingElement(std::string(name.c_str()));
            }
            else
            {
                AddTimingDisplayElement(name, "1", "0");
                timing = AddElement(name, "timing");
            }

            if (sequence_loaded)
            {
                el1 = element->GetEffectLayer(0);
                el2 = element->AddEffectLayer();
                el3 = element->AddEffectLayer();
            }
            else
            {
                l1 = AddChildXmlNode(timing, "EffectLayer");
                l2 = AddChildXmlNode(timing, "EffectLayer");
                l3 = AddChildXmlNode(timing, "EffectLayer");
            }

            for (int p = 1; p <= numphrases; ++p)
            {
                wxString label = RemoveTabs(f.GetNextLine(), 2);
                linenum++;
                if (label == "")
                {
                    DisplayError(wxString::Format(_("Missing phrase# %d of %d for %s"), p, numphrases, desc.c_str()).ToStdString());
                    return;
                }

                //int start = number.Matches(line = RemoveTabs(f.GetNextLine(), 2)) ? TimeLine::RoundToMultipleOfPeriod((offset + wxAtoi(line)) * ms, GetFrequency()) : 0;
                int start = number.Matches(line = RemoveTabs(f.GetNextLine(),2)) ? (offset + wxAtoi(line)) * ms : 0;
                linenum++;

                //int end = number.Matches(line = RemoveTabs(f.GetNextLine(), 2)) ? TimeLine::RoundToMultipleOfPeriod((offset + wxAtoi(line)) * ms, GetFrequency()) : 0;
                int end = number.Matches(line = RemoveTabs(f.GetNextLine(),2)) ? (offset + wxAtoi(line)) * ms : 0;
                linenum++;
                desc = wxString::Format(_("voice# %d, phrase %d '%s', start frame %d end frame %d @line %d"), v, p, label.c_str(), start, end, linenum);

                if (sequence_loaded)
                {
                    el1->AddEffect(0, std::string(label.c_str()), "", "", start, end, EFFECT_NOT_SELECTED, false);
                }
                else
                {
                    AddTimingEffect(l1, std::string(label.c_str()), "0", "0", wxString::Format("%d", start), wxString::Format("%d", end));
                }

                int numwords = number.Matches(line = RemoveTabs(f.GetNextLine(),2)) ? wxAtoi(line) : -1;
                linenum++;
                if (numwords < 0)
                {
                    DisplayError(wxString::Format(_("Invalid file @line %d ('%s' words for %s)"), linenum, line.c_str(), desc.c_str()).ToStdString());
                }

                for (int w = 1; w <= numwords; ++w)
                {
                    line = RemoveTabs(f.GetNextLine(), 3);
                    linenum++;
                    int space1 = line.find(' ');
                    label = line.SubString(0, space1-1);
                    if (label == "")
                    {
                        DisplayError(wxString::Format(_("Missing word# %d of %d for %s"), w, numwords, desc.c_str()).ToStdString());
                        return;
                    }

                    int space2 = line.find(' ', space1 + 1);
                    wxString ss = line.SubString(space1 + 1, space2 - 1);
                    //start = number.Matches(ss) ? TimeLine::RoundToMultipleOfPeriod((offset + wxAtoi(ss)) * ms, GetFrequency()) : 0;
                    start = number.Matches(ss) ? (offset + wxAtoi(ss)) * ms : 0;
                    linenum++;

                    int space3 = line.find(' ', space2 + 1);
                    ss = line.SubString(space2 + 1, space3 - 1);
                    //end = number.Matches(ss) ? TimeLine::RoundToMultipleOfPeriod((offset + wxAtoi(ss)) * ms, GetFrequency()) : 0;
                    end = number.Matches(ss) ? (offset + wxAtoi(ss)) * ms : 0;
                    linenum++;
                    desc = wxString::Format(_("voice# %d, phrase# %d, word %d '%s', start frame %d end frame %d @line %d"), v, p, w, label.c_str(), start, end, linenum);

                    if (sequence_loaded)
                    {
                        el2->AddEffect(0, std::string(label.c_str()), "", "", start, end, EFFECT_NOT_SELECTED, false);
                    }
                    else
                    {
                        AddTimingEffect(l2, std::string(label.c_str()), "0", "0", wxString::Format("%d", start), wxString::Format("%d", end));
                    }

                    ss = line.SubString(space3 + 1, line.Length());
                    int numphonemes = number.Matches(ss) ? wxAtoi(ss) : -1;
                    linenum++;
                    if (numphonemes < 0)
                    {
                        DisplayError(wxString::Format(_("Invalid file @line %d ('%s' phonemes for %s)"), linenum, line.c_str(), desc.c_str()).ToStdString());
                    }

                    int outerend = end;
                    for (int ph = 1; ph <= numphonemes; ++ph)
                    {
                        line = RemoveTabs(f.GetNextLine(), 4);
                        linenum++;
                        int space4 = line.find(' ');

                        ss = line.SubString(0, space4 - 1);
                        //end = number.Matches(ss) ? TimeLine::RoundToMultipleOfPeriod((offset + wxAtoi(ss)) * ms, GetFrequency()) : 0;
                        end = number.Matches(ss) ? (offset + wxAtoi(ss)) * ms : 0;
                        linenum++;

                        if (ph == 1)
                        {
                            // dont do anything
                        }
                        else
                        {
                            if (sequence_loaded)
                            {
                                el3->AddEffect(0, std::string(label.c_str()), "", "", start, end, EFFECT_NOT_SELECTED, false);
                            }
                            else
                            {
                                AddTimingEffect(l3, std::string(label.c_str()), "0", "0", wxString::Format("%d", start), wxString::Format("%d", end));
                            }
                        }
                        label = line.SubString(space4 + 1, line.Length());
                        if (label == "")
                        {
                            DisplayError(wxString::Format(_("Missing phoneme# %d of %d for %s"), ph, numphonemes, desc.c_str()).ToStdString());
                            return;
                        }
                        start = end;

                        if (ph == numphonemes)
                        {
                            end = outerend;
                            if (sequence_loaded)
                            {
                                el3->AddEffect(0, std::string(label.c_str()), "", "", start, end, EFFECT_NOT_SELECTED, false);
                            }
                            else
                            {
                                AddTimingEffect(l3, std::string(label.c_str()), "0", "0", wxString::Format("%d", start), wxString::Format("%d", end));
                            }
                        }
                    }
                }
            }
        }
    }
}

std::string ReadSRTLine(wxTextFile& f, int linenum, long& startMS, long& endMS)
{
    startMS = 0;
    endMS = 0;

    if (f.Eof()) return "";

    int l = 0;
    if (linenum == 1)
    {
        l = wxAtoi(f.GetFirstLine());
    }
    else
    {
        l = wxAtoi(f.GetNextLine());
    }
    while (!f.Eof() && l < linenum)
    {
        l = wxAtoi(f.GetNextLine());
    }
    if (l > linenum)
    {
        return "";
    }

    if (f.Eof()) return "";

    //00:00:06,580 --> 00:00:08,580
    auto times = f.GetNextLine();
    if (Contains(times, "-->"))
    {
        int sH, eH, sM, eM, sS, eS, sMS, eMS;
        wxArrayString c1 = wxSplit(times, ':');
        if (c1.size() == 5)
        {
            sH = wxAtoi(c1[0]);
            sM = wxAtoi(c1[1]);
            wxArrayString c2 = wxSplit(c1[2], ',');
            if (c2.size() == 2)
            {
                sS = wxAtoi(c2[0]);
                wxArrayString c3 = wxSplit(c2[1], ' ');
                if (c3.size() == 3)
                {
                    sMS = wxAtoi(c3[0]);
                    eH = wxAtoi(c3[2]);
                    eM = wxAtoi(c1[3]);
                    wxArrayString c4 = wxSplit(c1[4], ',');
                    if (c4.size() == 2)
                    {
                        eS = wxAtoi(c4[0]);
                        eMS = wxAtoi(c4[1]);
                        startMS = sH * 3600000 + sM * 60000 + sS * 1000 + sMS;
                        endMS = eH * 3600000 + eM * 60000 + eS * 1000 + eMS;
                    }
                }
            }
        }
    }

    if (f.Eof()) return "";

    std::string line = "";
    std::string ll = f.GetNextLine();
    while (!f.Eof() && ll != "")
    {
        if (line != "") line += " ";
        line += Trim(ll);
        ll = f.GetNextLine();
    }
    return line;
}

void xLightsXmlFile::ProcessSRT( const wxArrayString& filenames, xLightsFrame* xLightsParent)
{
    static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    wxTextFile f;

    for (size_t i = 0; i < filenames.Count(); ++i)
    {
        wxFileName next_file(filenames[i]);
        logger_base.info("Loading srt file " + std::string(next_file.GetFullPath().c_str()));

        if (!f.Open(next_file.GetFullPath().c_str()))
        {
            DisplayError("Failed to open file: " + next_file.GetFullPath());
            return;
        }

        wxString name = wxString::Format(next_file.GetName());
        name = UniqueTimingName(xLightsParent, name);
        logger_base.info("    Loading into timing track %s.", (const char*)name.c_str());

        Element* element = nullptr;
        wxXmlNode* timing = nullptr;
        if (sequence_loaded)
        {
            element = xLightsParent->AddTimingElement(std::string(name.c_str()));
        }
        else
        {
            AddTimingDisplayElement(name, "1", "0");
            timing = AddElement(name, "timing");
        }

        wxXmlNode* l1 = nullptr;
        EffectLayer* el1 = nullptr;
        if (sequence_loaded)
        {
            el1 = element->GetEffectLayer(0);
        }
        else
        {
            l1 = AddChildXmlNode(timing, "EffectLayer");
        }

        long startMS;
        long endMS;
        int linenum = 1;

        std::string line = ReadSRTLine(f, linenum++, startMS, endMS);

        do {

            if (line != "" && endMS > startMS)
            {
                if (sequence_loaded)
                {
                    el1->AddEffect(0, line, "", "", startMS, endMS, EFFECT_NOT_SELECTED, false);
                }
                else
                {
                    AddTimingEffect(l1, line, "0", "0", wxString::Format("%ld", startMS), wxString::Format("%ld", endMS));
                }
            }

            line = ReadSRTLine(f, linenum++, startMS, endMS);
        } while (!f.Eof());
    }
}

wxString DecodeLSPTTColour(int att)
{
    switch (att)
    {
    case 1:
        return "x";
    case 2:
        return "R";
    case 4:
        return "G";
    case 8:
        return "B";
    case 16:
        return "Y";
    case 32:
        return "P";
    case 64:
        return "O";
    case 128:
        return "z";
    case 256:
        return "Go";
    case 512:
        return "W";
    case 1024:
        return "System";
    default:
        break;
    }

    return wxString::Format("%d", att);
}

void xLightsXmlFile::ProcessLSPTiming( const wxArrayString& filenames, xLightsFrame* xLightsParent)
{
    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    wxTextFile f;

    xLightsParent->SetCursor(wxCURSOR_WAIT);

    for (size_t i = 0; i < filenames.Count(); ++i)
    {
        wxFileName next_file(filenames[i]);
        logger_base.info("Decompressing LSP file " + std::string(next_file.GetFullPath().c_str()));

        wxFileInputStream fin(next_file.GetFullPath());
        wxZipInputStream zin(fin);
        wxZipEntry *ent = zin.GetNextEntry();

        wxXmlDocument seq_xml;

        while (ent != nullptr)
        {
            if (ent->GetName() == "Sequence")
            {
                logger_base.info("Extracting timing tracks from " + std::string(next_file.GetFullPath().c_str()) + "/" + std::string(ent->GetName().c_str()));
                seq_xml.Load(zin);

                wxXmlNode* e = seq_xml.GetRoot();

                if (e->GetName() == "MusicalSequence")
                {
                    for (wxXmlNode* tts = e->GetChildren(); tts != nullptr; tts = tts->GetNext())
                    {
                        if (tts->GetName() == "TimingTracks")
                        {
                            for (wxXmlNode* t = tts->GetChildren(); t != nullptr; t = t->GetNext())
                            {
                                if (t->GetName() == "Track") {
                                    wxString name = UniqueTimingName(xLightsParent, next_file.GetName());
                                    logger_base.info("  Track: " + std::string(name.c_str()));
                                    EffectLayer* effectLayer = nullptr;
                                    wxXmlNode* layer = nullptr;
                                    int present = 0;
                                    for (wxXmlNode* is = t->GetChildren(); is != nullptr; is = is->GetNext()) {
                                        if (is->GetName() == "Intervals") {
                                            std::list<wxString> atts;
                                            for (wxXmlNode* ti = is->GetChildren(); ti != nullptr; ti = ti->GetNext()) {
                                                if (ti->GetName() == "TimeInterval") {
                                                    if (ti->GetAttribute("eff") == "7") {
                                                        present |= wxAtoi(ti->GetAttribute("att"));
                                                    }
                                                }
                                            }

                                            int mask = 1;
                                            for (size_t i1 = 0; i1 < 10; i1++) {
                                                if (present & mask) {
                                                    wxString tname = UniqueTimingName(xLightsParent, DecodeLSPTTColour(mask) + "-" + name);
                                                    logger_base.info("  Adding timing track " + std::string(tname.c_str()) + "(" + std::string(wxString::Format("%d",mask).c_str()) + ")");
                                                    if (sequence_loaded) {
                                                        Element* element = xLightsParent->AddTimingElement(std::string(tname.c_str()));
                                                        effectLayer = element->GetEffectLayer(0);
                                                    }
                                                    else {
                                                        AddTimingDisplayElement(tname, "1", "0");
                                                        wxXmlNode* timing = AddElement(tname, "timing");
                                                        layer = AddChildXmlNode(timing, "EffectLayer");
                                                    }

                                                    int last = 0;
                                                    bool sevenfound = false;
                                                    bool fourfound = false;
                                                    for (wxXmlNode* ti = is->GetChildren(); ti != nullptr; ti = ti->GetNext()) {
                                                        if (ti->GetName() == "TimeInterval") {
                                                            if (ti->GetAttribute("eff") == "7" && (wxAtoi(ti->GetAttribute("att")) & mask)) {
                                                                sevenfound = true;
                                                                int start = last;
                                                                int end = TimeLine::RoundToMultipleOfPeriod((int)(wxAtof(ti->GetAttribute("pos")) * 50.0 / 4410.0), GetFrequency());
                                                                if (start != end)
                                                                {
                                                                    wxString label = "";
                                                                    if (sequence_loaded) {
                                                                        effectLayer->AddEffect(0, std::string(label.c_str()), "", "", start, end, EFFECT_NOT_SELECTED, false);
                                                                    }
                                                                    else {
                                                                        AddTimingEffect(layer, std::string(label.c_str()), "0", "0", wxString::Format("%d", start), wxString::Format("%d", end));
                                                                    }
                                                                    last = end;
                                                                }
                                                            }
                                                            // we take the only the first 4 after we have found 7s
                                                            else if (ti->GetAttribute("eff") == "4" && sevenfound && !fourfound && (wxAtoi(ti->GetAttribute("att")) & mask)) {
                                                                fourfound = true;
                                                                int start = last;
                                                                int end = TimeLine::RoundToMultipleOfPeriod((int)(wxAtof(ti->GetAttribute("pos")) * 50.0 / 4410.0), GetFrequency());
                                                                if (start != end)
                                                                {
                                                                    wxString label = "";
                                                                    if (sequence_loaded) {
                                                                        effectLayer->AddEffect(0, std::string(label.c_str()), "", "", start, end, EFFECT_NOT_SELECTED, false);
                                                                    }
                                                                    else {
                                                                        AddTimingEffect(layer, std::string(label.c_str()), "0", "0", wxString::Format("%d", start), wxString::Format("%d", end));
                                                                    }
                                                                    last = end;
                                                                }
                                                            }
                                                        }
                                                    }
                                                }
                                                mask = mask << 1;
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
            ent = zin.GetNextEntry();
        }
    }
    xLightsParent->SetCursor(wxCURSOR_ARROW);
}

void xLightsXmlFile::ProcessXLightsTiming( const wxArrayString& filenames, xLightsFrame* xLightsParent) {
    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    wxTextFile f;

    xLightsParent->SetCursor(wxCURSOR_WAIT);
    Element* element = nullptr;
    EffectLayer* effectLayer = nullptr;
    wxXmlNode* layer = nullptr;
    wxXmlNode* timing = nullptr;

    for (size_t i = 0; i < filenames.Count(); ++i)
    {
        wxFileName next_file(filenames[i]);

        logger_base.info("Loading sequence file " + std::string(next_file.GetFullPath().c_str()));
        xLightsXmlFile file(next_file);
        file.LoadSequence(next_file.GetPath(), true, next_file);

        SequenceElements se(xLightsParent);
        se.SetFrequency(file.GetFrequency());
        se.SetViewsManager(xLightsParent->GetViewsManager()); // This must come first before LoadSequencerFile.
        se.LoadSequencerFile(file, xLightsParent->GetShowDirectory());
        file.AdjustEffectSettingsForVersion(se, xLightsParent);

        std::vector<TimingElement *> elements;
        wxArrayString names;
        for (size_t e = 0; e < se.GetElementCount(); e++) {
            Element *el = se.GetElement(e);
            if (el->GetType() == ElementType::ELEMENT_TYPE_TIMING) {
                TimingElement *ti =dynamic_cast<TimingElement*>(el);
                if (ti->GetFixedTiming() == 0) {
                    elements.push_back(ti);
                    names.Add(el->GetName());
                }
            }
        }
        wxMultiChoiceDialog dlg(xLightsParent, "Select timing tracks to import", "Import Timing Tracks", names);
        if (dlg.ShowModal() == wxID_OK) {
            wxArrayInt selections = dlg.GetSelections();

            for (int i1 = 0; i1 < selections.size(); i1++) {
                TimingElement *ti = elements[selections[i1]];
                if (sequence_loaded) {
                    element = xLightsParent->AddTimingElement(ti->GetName());
                } else {
                    AddTimingDisplayElement(ti->GetName(), "1", "0");
                    timing = AddElement(ti->GetName(), "timing");
                }
                for (int x = 0; x < ti->GetEffectLayerCount(); x++) {
                    EffectLayer *src = ti->GetEffectLayer(x);
                    if (sequence_loaded) {
                        effectLayer = element->GetEffectLayer(x);
                        if (effectLayer == nullptr) {
                            effectLayer = element->AddEffectLayer();
                        }
                    } else {
                        layer = AddChildXmlNode(timing, "EffectLayer");
                    }
                    for (int ef = 0; ef < src->GetEffectCount(); ef++) {
                        Effect *effect = src->GetEffect(ef);
                        if (sequence_loaded) {
                            effectLayer->AddEffect(0, effect->GetEffectName(), "", "", effect->GetStartTimeMS(), effect->GetEndTimeMS(), EFFECT_NOT_SELECTED, false);
                        } else {
                            AddTimingEffect(layer, effect->GetEffectName(), "0", "0", wxString::Format("%d", effect->GetStartTimeMS()),
                                            wxString::Format("%d", effect->GetEndTimeMS()));
                        }
                    }
                }
            }
        }
    }
    xLightsParent->SetCursor(wxCURSOR_ARROW);
}

void xLightsXmlFile::AddMarksToLayer(const std::list<VixenTiming>& marks, EffectLayer* effectLayer, int frameMS) {
    int32_t last = 0;
    for (const auto& it : marks)
    {
        int st = Vixen3::ConvertTiming(it.start, frameMS);
        int en = Vixen3::ConvertTiming(it.end, frameMS);

        if (st < last) st = last;
        if (st < en)
        {
            effectLayer->AddEffect(0, it.label, "", "", st, en, EFFECT_NOT_SELECTED, false);
            last = en;
        }
        else
        {
            // Timing mark dropped because we could not fit it in
        }
    }
}

void xLightsXmlFile::ProcessVixen3Timing( const wxArrayString& filenames, xLightsFrame* xLightsParent) {
    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));

    xLightsParent->SetCursor(wxCURSOR_WAIT);

    for (size_t i = 0; i < filenames.Count(); ++i)
    {
        wxFileName next_file(filenames[i]);

        logger_base.info("Loading Vixen 3 file " + std::string(next_file.GetFullPath().c_str()));

        Vixen3 vixenFile(next_file.GetFullPath());

        auto timings = vixenFile.GetTimings();
        wxArrayString markNames;
        for (auto it: timings)
        {
            markNames.push_back(it);
        }

        wxMultiChoiceDialog dlg(xLightsParent, "Select timing tracks to import", "Import Timing Tracks", markNames);

        if (dlg.ShowModal() == wxID_OK) {
            wxArrayInt selections = dlg.GetSelections();

            for (int i1 = 0; i1 < selections.size(); i1++) {
                
                wxString sel = markNames[selections[i1]];

                if (vixenFile.GetTimingType(sel) == "Phrase")
                {
                    TimingElement* element = xLightsParent->AddTimingElement(UniqueTimingName(xLightsParent, sel));
                    EffectLayer* effectLayer = element->GetEffectLayer(0);
                    if (effectLayer == nullptr) {
                        effectLayer = element->AddEffectLayer();
                    }

                    AddMarksToLayer(vixenFile.GetTimings(sel.ToStdString()), effectLayer, GetFrameMS());
                    effectLayer = element->AddEffectLayer();
                    AddMarksToLayer(vixenFile.GetRelatedTiming(sel.ToStdString(), "Word"), effectLayer, GetFrameMS());
                    effectLayer = element->AddEffectLayer();
                    AddMarksToLayer(vixenFile.GetRelatedTiming(sel.ToStdString(), "Phoneme"), effectLayer, GetFrameMS());
                }
                else
                {
                    TimingElement* element = xLightsParent->AddTimingElement(UniqueTimingName(xLightsParent, sel));
                    EffectLayer* effectLayer = element->GetEffectLayer(0);
                    if (effectLayer == nullptr) {
                        effectLayer = element->AddEffectLayer();
                    }

                    AddMarksToLayer(vixenFile.GetTimings(sel.ToStdString()), effectLayer, GetFrameMS());
                }
            }
        }
    }

    xLightsParent->SetCursor(wxCURSOR_ARROW);
}

wxArrayString xLightsXmlFile::GetTimingList(const SequenceElements& seq_elements)
{
    timing_list.Clear();
    int num_elements = seq_elements.GetElementCount();
    for(int i = 0; i < num_elements; ++i)
    {
        Element* element = seq_elements.GetElement(i);
        if( element->GetType() == ElementType::ELEMENT_TYPE_TIMING )
        {
            timing_list.push_back(element->GetName());
        }
    }
    return timing_list;
}

bool xLightsXmlFile::Save()
{
    UpdateVersion();
    return seqDocument.Save(GetFullPath());
}

void xLightsXmlFile::WriteEffects(EffectLayer *layer,
                                  wxXmlNode *effect_layer_node,
                                  StringIntMap &colorPalettes,
                                  wxXmlNode* colorPalette_node,
                                  StringIntMap &effectStrings,
                                  wxXmlNode* effectDB_Node) {
    int num_effects = layer->GetEffectCount();
    for(int k = 0; k < num_effects; ++k)
    {
        Effect* effect = layer->GetEffect(k);
        std::string effectStdString = effect->GetSettingsAsString();
        wxString effectString = ToWXString(effectStdString);
        int size = effectStrings.size();
        int ref = effectStrings[effectString] - 1;
        if (ref == -1) {
            ref = size;
            effectStrings[effectString] = ref + 1;
            AddChildXmlNode(effectDB_Node, "Effect", effectString);
        }


        // Add effect node
        wxXmlNode* effect_node = AddChildXmlNode(effect_layer_node, "Effect");
        effect_node->AddAttribute("ref", string_format("%d", ref));
        effect_node->AddAttribute("name", XmlSafe(effect->GetEffectName()));
        if (effect->GetProtected()) {
            effect_node->AddAttribute("protected", "1");
        }
        if (effect->GetSelected()) {
            effect_node->AddAttribute("selected", "1");
        }
        if (effect->GetID()) {
            effect_node->AddAttribute("id", string_format("%d", effect->GetID()));
        }
        effect_node->AddAttribute("startTime", string_format("%d", effect->GetStartTimeMS()));
        effect_node->AddAttribute("endTime", string_format("%d", effect->GetEndTimeMS()));
        wxString palette = effect->GetPaletteAsString();
        if (palette != "") {
            size = colorPalettes.size();
            int pref = colorPalettes[palette] - 1;
            if (pref == -1) {
                pref = size;
                colorPalettes[palette] = pref + 1;
                AddChildXmlNode(colorPalette_node, "ColorPalette", palette);
            }
            effect_node->AddAttribute("palette", string_format("%d", pref));
        }
    }
}

void xLightsXmlFile::AddJukebox(wxXmlNode* node)
{
    wxXmlNode* root = seqDocument.GetRoot();

    for (wxXmlNode* n = root->GetChildren(); n != nullptr; n = n->GetNext())
    {
        if (n->GetName() == "Jukebox")
        {
            root->RemoveChild(n);
            delete n;
            break;
        }
    }

    root->AddChild(node);
}

// function used to save sequence data
bool xLightsXmlFile::Save(SequenceElements& seq_elements)
{
    if (SaveToDoc(seq_elements)) {
        wxFileOutputStream fout(GetFullPath());
        wxBufferedOutputStream *bout = new wxBufferedOutputStream(fout, 2 * 1024 * 1024);
        if (!seqDocument.Save(*bout)) {
            delete bout;
            return false;
        }
        delete bout;
        if (!fout.Close()) {
            return false;
        }

        MarkNewFileRevision(GetFullPath());
        return true;
    }
    return false;
}
bool xLightsXmlFile::SaveToDoc(SequenceElements& seq_elements) {
    wxXmlNode* root = seqDocument.GetRoot();

    root->DeleteAttribute("ModelBlending");
    root->AddAttribute("ModelBlending", seq_elements.SupportsModelBlending() ? "true" : "false");

    // Delete nodes that will be replaced
    for (wxXmlNode* e = root->GetChildren(); e != nullptr;) {
        if (e->GetName() == "DisplayElements" ||
            e->GetName() == "ElementEffects" ||
            e->GetName() == "DataLayers" ||
            e->GetName() == "ColorPalettes" ||
            e->GetName() == "EffectDB" ||
            e->GetName() == "SequenceMedia" ||
            e->GetName() == "TimingTags" ||
            e->GetName() == "lastView") {
            wxXmlNode* node_to_delete = e;
            e = e->GetNext();
            root->RemoveChild(node_to_delete);
            delete node_to_delete;
        } else {
            e = e->GetNext();
        }
    }

    StringIntMap colorPalettes;
    wxXmlNode* colorPalette_node = AddChildXmlNode(root, "ColorPalettes");
    StringIntMap effectStrings;
    wxXmlNode* effectDB_Node = AddChildXmlNode(root, "EffectDB");

    // Save sequence media
    wxXmlNode* sequenceImages_node = seq_elements.GetSequenceMedia().SaveToXml();
    if (sequenceImages_node != nullptr) {
        root->AddChild(sequenceImages_node);
    }

    // Now add new elements to our xml document
    wxXmlNode* data_layer = AddChildXmlNode(root, "DataLayers");
    wxXmlNode* display_node = AddChildXmlNode(root, "DisplayElements");
    wxXmlNode* elements_node = AddChildXmlNode(root, "ElementEffects");
    wxXmlNode* last_view_node = AddChildXmlNode(root, "lastView");
    wxXmlNode* timing_tags_node = AddChildXmlNode(root, "TimingTags");

    SetNodeContent(last_view_node, wxString::Format("%d", seq_elements.GetCurrentView()));

    int num_data_layers = mDataLayers.GetNumLayers();
    for (int i = 0; i < num_data_layers; ++i) {
        DataLayer* layer = mDataLayers.GetDataLayer(i);
        wxXmlNode* layer_node = AddChildXmlNode(data_layer, "DataLayer");
        layer_node->AddAttribute("lor_params", wxString::Format("%d", layer->GetLORConvertParams()));
        layer_node->AddAttribute("channel_offset", wxString::Format("%d", layer->GetChannelOffset()));
        layer_node->AddAttribute("num_channels", wxString::Format("%d", layer->GetNumChannels()));
        layer_node->AddAttribute("num_frames", wxString::Format("%d", layer->GetNumFrames()));
        layer_node->AddAttribute("data", layer->GetDataSource());
        layer_node->AddAttribute("source", layer->GetSource());
        layer_node->AddAttribute("name", layer->GetName());
    }

    if (seq_elements.GetTimeLine() != nullptr) {
        for (int i = 0; i < 10; ++i) {
            wxXmlNode* tag_node = AddChildXmlNode(timing_tags_node, "Tag");
            tag_node->AddAttribute("number", string_format("%d", i));
            tag_node->AddAttribute("position", string_format("%d", seq_elements.GetTimeLine()->GetTagPosition(i)));
        }
    }

    int num_elements = seq_elements.GetElementCount();
    for (int i = 0; i < num_elements; ++i) {
        Element* element = seq_elements.GetElement(i);

        // Add display elements
        wxXmlNode* display_element_node = AddChildXmlNode(display_node, "Element");
        display_element_node->AddAttribute("collapsed", string_format("%d", element->GetCollapsed()));
        if (element->IsRenderDisabled()) {
            display_element_node->AddAttribute("RenderDisabled", "1");
        }
        display_element_node->AddAttribute("type", element->GetType() == ElementType::ELEMENT_TYPE_TIMING ? "timing" : "model");
        display_element_node->AddAttribute("name", element->GetName());
        if (element->GetType() == ElementType::ELEMENT_TYPE_TIMING) {
            TimingElement *te = dynamic_cast<TimingElement*>(element);
            display_element_node->AddAttribute("visible", string_format("%d", te->GetMasterVisible()));
        } else {
            display_element_node->AddAttribute("visible", string_format("%d", element->GetVisible()));
        }

        // Add element node to ElementEffects
        wxXmlNode* element_effects_node = AddChildXmlNode(elements_node, "Element");
        element_effects_node->AddAttribute("type", element->GetType() == ElementType::ELEMENT_TYPE_TIMING ? "timing" : "model");
        element_effects_node->AddAttribute("name", element->GetName());

        if (element->GetType() == ElementType::ELEMENT_TYPE_TIMING) {
            TimingElement* tm = dynamic_cast<TimingElement*>(element);
            display_element_node->AddAttribute("views", tm->GetViews());
            display_element_node->AddAttribute("active", string_format("%d", tm->GetActive()));
            if (tm->GetSubType() != "") {
                display_element_node->AddAttribute("subType", tm->GetSubType());
            }
            if (tm->GetFixedTiming()) {
                element_effects_node->AddAttribute("fixed", string_format("%d", tm->GetFixedTiming()));
                AddChildXmlNode(element_effects_node, "EffectLayer");
            } else {
                int num_layers = tm->GetEffectLayerCount();
                for (int j = 0; j < num_layers; ++j) {
                    EffectLayer* layer = tm->GetEffectLayer(j);
                    // Add layer node
                    wxXmlNode* effect_layer_node = AddChildXmlNode(element_effects_node, "EffectLayer");

                    // Add effects
                    int num_effects = layer->GetEffectCount();
                    for (int k = 0; k < num_effects; ++k) {
                        Effect* effect = layer->GetEffect(k);
                        // Add effect node
                        wxXmlNode* effect_node = AddChildXmlNode(effect_layer_node, "Effect", effect->GetSettingsAsString());

                        effect_node->AddAttribute("label", effect->GetEffectName());
                        if (effect->GetProtected()) {
                            effect_node->AddAttribute("protected", "1");
                        }
                        if (effect->GetSelected()) {
                            effect_node->AddAttribute("selected", "1");
                        }
                        effect_node->AddAttribute("startTime", string_format("%d", effect->GetStartTimeMS()));
                        effect_node->AddAttribute("endTime", string_format("%d", effect->GetEndTimeMS()));
                    }
                }
            }
        } else if (element->GetType() == ElementType::ELEMENT_TYPE_MODEL) {
            ModelElement* me = dynamic_cast<ModelElement*>(element);
            int num_layers = me->GetEffectLayerCount();
            for (int j = 0; j < num_layers; ++j) {
                EffectLayer* layer = me->GetEffectLayer(j);

                // Add layer node
                wxXmlNode* effect_layer_node = AddChildXmlNode(element_effects_node, "EffectLayer");
                if (!layer->GetLayerName().empty()) {
                    effect_layer_node->AddAttribute("layerName", layer->GetLayerName());
                }
                WriteEffects(layer, effect_layer_node, colorPalettes,
                             colorPalette_node,
                             effectStrings,
                             effectDB_Node);
            }

            int num_strands = me->GetSubModelAndStrandCount();
            for (int strand = 0; strand < num_strands; strand++) {
                SubModelElement* se = me->GetSubModel(strand);
                num_layers = se->GetEffectLayerCount();
                wxXmlNode* effect_layer_node = nullptr;

                StrandElement* strEl = dynamic_cast<StrandElement*>(se);
                for (int j = 0; j < num_layers; ++j) {
                    EffectLayer* layer = se->GetEffectLayer(j);

                    if (layer->GetEffectCount() != 0 || !layer->GetLayerName().empty()) {
                        wxXmlNode* eln = AddChildXmlNode(element_effects_node, strEl == nullptr ? "SubModelEffectLayer" : "Strand");
                        if (strEl != nullptr) {
                            eln->AddAttribute("index", string_format("%d", strEl->GetStrand()));
                            if (j == 0) {
                                effect_layer_node = eln;
                            }
                        }
                        if (!layer->GetLayerName().empty()) {
                            eln->AddAttribute("layerName", layer->GetLayerName());
                        }
                        if (j > 0) {
                            eln->AddAttribute("layer", string_format("%d", j));
                        }
                        if (se->GetName() != "") {
                            eln->AddAttribute("name", se->GetName());
                        }
                        WriteEffects(layer, eln, colorPalettes,
                                     colorPalette_node,
                                     effectStrings,
                                     effectDB_Node);
                    }
                }
                if (strEl != nullptr) {
                    for (int n = 0; n < strEl->GetNodeLayerCount(); n++) {
                        NodeLayer* nlayer = strEl->GetNodeLayer(n);
                        if (nlayer->GetEffectCount() == 0) {
                            continue;
                        }
                        if (effect_layer_node == nullptr) {
                            effect_layer_node = AddChildXmlNode(element_effects_node, "Strand");
                            effect_layer_node->AddAttribute("index", string_format("%d", strEl->GetStrand()));
                            if (se->GetName() != "") {
                                effect_layer_node->AddAttribute("name", se->GetName());
                            }
                        }
                        wxXmlNode* neffect_layer_node = AddChildXmlNode(effect_layer_node, "Node");
                        neffect_layer_node->AddAttribute("index", string_format("%d", n));
                        if (nlayer->GetNodeName() != "") {
                            neffect_layer_node->AddAttribute("name", nlayer->GetNodeName());
                        }
                        WriteEffects(nlayer, neffect_layer_node, colorPalettes,
                                     colorPalette_node,
                                     effectStrings,
                                     effectDB_Node);
                    }
                }
            }
        }
    }
    UpdateVersion();

#ifdef USE_COMPRESSION
    for (wxXmlNode* e = root->GetChildren(); e != nullptr; e = e->GetNext()) {
        wxString name = e->GetName();
        if (name == "ColorPalettes" || name == "EffectDB" || name == "ElementEffects") {
            wxXmlDocument doc;

            wxXmlNode* next = e->GetNext();
            root->RemoveChild(e);

            doc.SetRoot(e);
            wxMemoryOutputStream out;
            doc.Save(out, wxXML_NO_INDENTATION);

            int max = out.GetOutputStreamBuffer()->Tell();
            uint8_t* outBuf = new uint8_t[max];
            int outSize = ZSTD_compress(outBuf, max,
                                        out.GetOutputStreamBuffer()->GetBufferStart(), max,
                                        1);
            // Manipulate data.....
            wxString b64 = wxBase64Encode(outBuf, outSize);
            delete[] outBuf;
            wxXmlNode* newNode = new wxXmlNode(wxXML_ELEMENT_NODE, "CompressedData");
            newNode->AddChild(new wxXmlNode(wxXML_TEXT_NODE, "", b64));
            newNode->AddAttribute("size", std::to_string(max));
            root->InsertChild(newNode, next);

            e = newNode;
        }
    }
#endif

    
    return true;
}

bool xLightsXmlFile::TimingAlreadyExists(const std::string & section, xLightsFrame* xLightsParent)
{
    if( sequence_loaded )
    {
        timing_list = GetTimingList(xLightsParent->GetSequenceElements());
    }
    else
    {
        timing_list = GetTimingList();
    }
    for (size_t i = 0; i < timing_list.size(); ++i )
    {
        if( timing_list[i] == section )
        {
            return true;
        }
    }
    return false;
}

bool xLightsXmlFile::TimingMatchesModelName(const std::string& section, xLightsFrame* xLightsParent) {
    if (sequence_loaded) {
        SequenceElements& mSequenceElements = xLightsParent->GetSequenceElements();
        if (mSequenceElements.ElementExists(section)) {
            return true;
        }
    }
    return false;
}

void xLightsXmlFile::AddNewTimingSection(const std::string& filename, xLightsFrame* xLightsParent,
                                         std::vector<int>& starts, std::vector<int>& ends, std::vector<std::string>& labels)
{
    // some QM plugins dont return items sorted appropriately
    typedef struct tm {
        int start;
        int end;
        std::string label;
        int duration() const
        {
            return end - start;
        }
        static bool sort_func(const struct tm& a, const struct tm& b)
        {
            if (a.start == b.start)
                return a.duration() < b.duration();

            return a.start < b.start;
        }
    } tm;

    std::vector<tm> tms;

    tms.resize(starts.size());
    for (size_t k = 0; k < starts.size(); k++) {
        tms[k] = tm({ TimeLine::RoundToMultipleOfPeriod(starts[k], GetFrequency()),
                   TimeLine::RoundToMultipleOfPeriod(ends[k], GetFrequency()),
                   labels[k] });
    }

    std::sort(begin(tms), end(tms), tm::sort_func);

    EffectLayer* effectLayer = nullptr;
    wxXmlNode* layer = nullptr;
    if (sequence_loaded) {
        Element* element = xLightsParent->AddTimingElement(filename);
        effectLayer = element->GetEffectLayer(0);
    } else {
        AddTimingDisplayElement(filename, "1", "0");
        wxXmlNode* node = AddElement(filename, "timing");
        layer = AddChildXmlNode(node, "EffectLayer");
    }

    int prev_start = -1;
    int prev_end = -1;

    for (size_t k = 0; k < tms.size(); k++) {
        int start = tms[k].start;
        int end = tms[k].end;

        // if this timing mark overlaps the prior one then force it to start at the end of the prior one
        if (start < prev_end) {
            start = prev_end;
        }

        if (k < tms.size() - 1 && tms[k + 1].start < end) // the next timing starts before this one ends ... in which case this ends = that start
        {
            end = tms[k + 1].start;
        }

        // if this timing mark starts before the prior one ended then start it after the prior one
        if (start < prev_end)
        {
            start = prev_end;
        }

        // if it now starts after it ends then skip it as it totally overlapped with the last timing mark
        if (start > end) {
            continue;
        }

        if (start > GetSequenceDurationMS()) {
            continue; // dont add timing marks after the end of the song
        } else if (start == prev_start && end == prev_end) {
            continue;            // skip duplicates
        } else if (start == end) // dont add zero length timing marks
        {
            // zero length timing marks are not valid ... but if the following start is greater than the current start then use that as the end
            if (k == tms.size() - 1) {
                // special case use the end of the sequence
                end = TimeLine::RoundToMultipleOfPeriod(GetSequenceDurationMS(), GetFrequency());
                if (start == end) {
                    continue;
                }
            } else {
                if (tms[k + 1].start > start) {
                    end = tms[k + 1].start;
                } else {
                    // even using the next start would make it zero length so we have to skip this one
                    continue;
                }
            }
        }

        prev_start = start;
        prev_end = end;

        if (sequence_loaded) {
            effectLayer->AddEffect(0, tms[k].label, "", "", start, end, EFFECT_NOT_SELECTED, false);
        } else {
            AddTimingEffect(layer, tms[k].label, "0", "0", string_format("%d", start), string_format("%d", end));
        }
    }
}

void xLightsXmlFile::AddNewTimingSection(const std::string & interval_name, xLightsFrame* xLightsParent, const std::string& subType)
{
    AddTimingDisplayElement( interval_name, "1", "0", subType);

    if( sequence_loaded )
    {
        xLightsParent->AddTimingElement(interval_name, subType);
    }

    wxXmlNode * node = AddElement( interval_name, "timing" );
    AddChildXmlNode(node, "EffectLayer");
}

void xLightsXmlFile::AddFixedTimingSection(const std::string& interval_name, xLightsFrame* xLightsParent)
{
    AddTimingDisplayElement(interval_name, "1", "0");
    wxXmlNode* node;

    if (interval_name == "Empty" || (interval_name != "25ms" && interval_name != "50ms" && interval_name != "100ms" && !EndsWith(interval_name, "ms Metronome")))
    {
        if (sequence_loaded)
        {
            xLightsParent->AddTimingElement(interval_name);
        }
        node = AddElement(interval_name, "timing");
    }
    else
    {
        int interval = wxAtoi(interval_name);
        if (sequence_loaded)
        {
            TimingElement* element = xLightsParent->AddTimingElement(interval_name);
            element->SetFixedTiming(interval);
            EffectLayer* effectLayer = element->GetEffectLayer(0);
            int time = 0;
            int end_time = GetSequenceDurationMS();
            while (time <= end_time)
            {
                int next_time = (time + interval <= end_time) ? time + interval : end_time;
                int startTime = TimeLine::RoundToMultipleOfPeriod(time, GetFrequency());
                int endTime = TimeLine::RoundToMultipleOfPeriod(next_time, GetFrequency());
                effectLayer->AddEffect(0, "", "", "", startTime, endTime, EFFECT_NOT_SELECTED, false);
                time += interval;
            }
        }
        node = AddFixedTiming(interval_name, string_format("%d", interval));
    }

    AddChildXmlNode(node, "EffectLayer");
}

void xLightsXmlFile::AddFixedTimingSection(const std::string& interval_name, int interval, xLightsFrame* xLightsParent)
{
    AddTimingDisplayElement(interval_name, "1", "0");

    if (sequence_loaded)
    {
        TimingElement* element = xLightsParent->AddTimingElement(interval_name);
        element->SetFixedTiming(interval);
        EffectLayer* effectLayer = element->GetEffectLayer(0);
        int time = 0;
        int end_time = GetSequenceDurationMS();
        while (time <= end_time)
        {
            int next_time = (time + interval <= end_time) ? time + interval : end_time;
            int startTime = TimeLine::RoundToMultipleOfPeriod(time, GetFrequency());
            int endTime = TimeLine::RoundToMultipleOfPeriod(next_time, GetFrequency());
            effectLayer->AddEffect(0, "", "", "", startTime, endTime, EFFECT_NOT_SELECTED, false);
            time += interval;
        }
    }

    wxXmlNode* node = AddFixedTiming(interval_name, string_format("%d", interval));
    AddChildXmlNode(node, "EffectLayer");
}

void xLightsXmlFile::AddMetronomeLabelTimingSection(const std::string& interval_name, int _interval, const std::vector<std::string>& tags, xLightsFrame* xLightsParent, int minForRandomRange, bool randomLabels) {
    AddTimingDisplayElement(interval_name, "1", "0");
    wxXmlNode* node;

    std::vector<std::string> effectiveTags = tags;
    if (effectiveTags.empty()) {
        for (int i = 1; i <= 10; ++i) {
            effectiveTags.push_back(std::to_string(i));
        }
    }

    if (sequence_loaded) {
        TimingElement* element = xLightsParent->AddTimingElement(interval_name);
        EffectLayer* effectLayer = element->GetEffectLayer(0);
        int time{ 0 };
        int id{ 0 };
        int end_time = GetSequenceDurationMS();
        int lastRandomState = -1;
        while (time < end_time) {
            int interval = minForRandomRange == -1 ? _interval : intRand(minForRandomRange, _interval);
            int next_time = (time + interval <= end_time) ? time + interval : end_time;
            int startTime = TimeLine::RoundToMultipleOfPeriod(time, GetFrequency());
            int endTime = TimeLine::RoundToMultipleOfPeriod(next_time, GetFrequency());

            std::string label;
            if (randomLabels) {
                int tagIndex;
                do {
                    tagIndex = intRand(0, effectiveTags.size() - 1);
                } while (tagIndex == lastRandomState && effectiveTags.size() > 1);
                lastRandomState = tagIndex;
                label = effectiveTags[tagIndex];
            } else {
                label = effectiveTags[id % effectiveTags.size()];
            }

            effectLayer->AddEffect(0, label, "", "", startTime, endTime, EFFECT_NOT_SELECTED, false);
            time += interval;
            id++;
        }
    } else {
        // sequence not yet loaded - write tag effects directly into XML
        node = AddElement(interval_name, "timing");
        wxXmlNode* effectLayer = AddChildXmlNode(node, "EffectLayer");
        int time = 0;
        int id = 0;
        int end_time = GetSequenceDurationMS();
        int lastRandomState = -1;
        while (time < end_time) {
            int interval = minForRandomRange == -1 ? _interval : intRand(minForRandomRange, _interval);
            int next_time = (time + interval <= end_time) ? time + interval : end_time;
            int startTime = TimeLine::RoundToMultipleOfPeriod(time, GetFrequency());
            int endTime = TimeLine::RoundToMultipleOfPeriod(next_time, GetFrequency());

            std::string label;
            if (randomLabels) {
                int tagIndex;
                do {
                    tagIndex = intRand(0, effectiveTags.size() - 1);
                } while (tagIndex == lastRandomState && effectiveTags.size() > 1);
                lastRandomState = tagIndex;
                label = effectiveTags[tagIndex];
            } else {
                label = effectiveTags[id % effectiveTags.size()];
            }

            AddTimingEffect(effectLayer, label, "0", "0",
                string_format("%d", startTime), string_format("%d", endTime));
            time += interval;
            id++;
        }
        return; // node already handled, skip the AddFixedTiming path
    }

    node = AddElement(interval_name, "timing");
    AddChildXmlNode(node, "EffectLayer");
}

void xLightsXmlFile::SetMetaMP3Tags()
{
    if (audio != nullptr)
    {
        SetHeaderInfo(HEADER_INFO_TYPES::SONG, audio->Title());
        SetHeaderInfo(HEADER_INFO_TYPES::ARTIST, audio->Artist());
        SetHeaderInfo(HEADER_INFO_TYPES::ALBUM, audio->Album());
    }
}

std::string xLightsXmlFile::GetFSEQForXSQ(const std::string& xsq, const std::string& fseqDirectory)
{
    if (!FileExists(xsq))
        return "";

    wxFileName fn(xsq);
    fn.SetExt("fseq");

    if (!FileExists(fn.GetFullPath())) {
        fn.SetPath(fseqDirectory);

        if (!FileExists(fn.GetFullPath())) {
            return "";
        }
    }

    return fn.GetFullPath();
}

std::string xLightsXmlFile::GetMediaForXSQ(const std::string& xsq, const std::string& showDir, const std::list<std::string> mediaFolders)
{
    if (!FileExists(xsq))
        return "";

    static const int BUFFER_SIZE = 1024 * 12;
    std::vector<char> buf(BUFFER_SIZE); //12K buffer

    wxFile doc(xsq);
    SP_XmlPullParser* parser = new SP_XmlPullParser();
    size_t read = doc.Read(&buf[0], BUFFER_SIZE);
    parser->append(&buf[0], read);
    SP_XmlPullEvent* event = parser->getNext();
    int done = 0;
    int count = 0;
    bool isMedia = false;
    std::string mediaName;

    while (!done) {
        if (!event) {
            size_t read2 = doc.Read(&buf[0], BUFFER_SIZE);
            if (read2 == 0) {
                done = true;
            } else {
                parser->append(&buf[0], read2);
            }
        } else {
            switch (event->getEventType()) {
            case SP_XmlPullEvent::eEndDocument:
                done = true;
                break;
            case SP_XmlPullEvent::eStartTag: {
                SP_XmlStartTagEvent* stagEvent = (SP_XmlStartTagEvent*)event;
                wxString NodeName = wxString::FromAscii(stagEvent->getName());
                count++;
                if (NodeName == "mediaFile") {
                    isMedia = true;
                } else {
                    isMedia = false;
                }
                if (count == 100) {
                    //media file will be very early in the file, dont waste time;
                    done = true;
                }
            } break;
            case SP_XmlPullEvent::eCData:
                if (isMedia) {
                    SP_XmlCDataEvent* stagEvent = (SP_XmlCDataEvent*)event;
                    mediaName = wxString::FromAscii(stagEvent->getText()).ToStdString();
                    done = true;
                }
                break;
            }
        }
        if (!done) {
            event = parser->getNext();
        }
    }
    delete parser;

    if (mediaName != "") {
        if (!FileExists(mediaName)) {
            wxFileName fn(mediaName);
            for (const std::string& md : mediaFolders) {
                std::string tmn = md + ::GetPathSeparator() + fn.GetFullName().ToStdString();
                if (FileExists(tmn)) {
                    mediaName = tmn;
                    break;
                }
            }
            if (!FileExists(mediaName)) {
                const std::string fixedMN = FixFile(showDir, mediaName);
                if (!FileExists(fixedMN)) {
                    mediaName = "";
                } else {
                    mediaName = fixedMN;
                }
            }
        }
    }

    return mediaName;
}

void xLightsXmlFile::AdjustEffectSettingsForVersion(SequenceElements& elements, xLightsFrame* xLightsParent)
{
    std::string ver = GetVersion().ToStdString();
    std::vector<RenderableEffect*> effects(xLightsParent->GetEffectManager().size());
    int count = 0;
    for (int x = 0; x < xLightsParent->GetEffectManager().size(); x++) {
        RenderableEffect* eff = xLightsParent->GetEffectManager()[x];
        if (eff->needToAdjustSettings(ver)) {
            effects[x] = eff;
            count++;
        }
    }
    if (count > 0) {
        for (size_t i = 0; i < elements.GetElementCount(); i++) {
            Element* elem = elements.GetElement(i);
            if (elem->GetType() == ElementType::ELEMENT_TYPE_MODEL) {
                ModelElement* me = dynamic_cast<ModelElement*>(elem);
                for (int j = 0; j < elem->GetEffectLayerCount(); j++) {
                    EffectLayer* layer = elem->GetEffectLayer(j);
                    for (int k = 0; k < layer->GetEffectCount(); k++) {
                        Effect* eff = layer->GetEffect(k);
                        if (eff != nullptr && eff->GetEffectIndex() >= 0 && effects[eff->GetEffectIndex()] != nullptr) {
                            effects[eff->GetEffectIndex()]->adjustSettings(ver, eff);
                        }
                    }
                }
                for (int s = 0; s < me->GetSubModelAndStrandCount(); s++) {
                    SubModelElement* se = me->GetSubModel(s);
                    for (int j = 0; j < se->GetEffectLayerCount(); j++) {
                        EffectLayer* layer = se->GetEffectLayer(j);
                        for (int k = 0; k < layer->GetEffectCount(); k++) {
                            Effect* eff = layer->GetEffect(k);
                            if (eff != nullptr && eff->GetEffectIndex() >= 0 && effects[eff->GetEffectIndex()] != nullptr) {
                                effects[eff->GetEffectIndex()]->adjustSettings(ver, eff);
                            }
                        }
                    }
                    if (se->GetType() == ElementType::ELEMENT_TYPE_STRAND) {
                        StrandElement* ste = dynamic_cast<StrandElement*>(se);
                        for (int k = 0; k < ste->GetNodeLayerCount(); k++) {
                            NodeLayer* nlayer = ste->GetNodeLayer(k);
                            for (int l = 0; l < nlayer->GetEffectCount(); l++) {
                                Effect* eff = nlayer->GetEffect(l);
                                if (eff != nullptr && eff->GetEffectIndex() >= 0 && effects[eff->GetEffectIndex()] != nullptr) {
                                    effects[eff->GetEffectIndex()]->adjustSettings(ver, eff);
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}

wxString xLightsXmlFile::InsertMissing(wxString str,wxString missing_array,bool INSERT)
{
    wxStringTokenizer tkz(missing_array, "|");
    wxString token1 = tkz.GetNextToken(); // get first two dummy tokens out
    wxString token2 = tkz.GetNextToken();
    while ( tkz.HasMoreTokens() )
    {
        token1 = tkz.GetNextToken();
        token2 = tkz.GetNextToken();
        int pos = str.find(token1,0);
        wxString replacement = "," + token2 + "</td>";
        if(pos<=0 && INSERT) // if we are INSERT mode we will add token 2 to the end of the xml string
        {
            str.Replace("</td>",replacement);
        }
        else if(pos>0 && !INSERT) // if we are in REPLACE mode (!INSERT), we replace token1 with token 2
        {
            str.Replace(token1,token2);
        }
    }
    return str;
}
