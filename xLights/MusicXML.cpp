/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include "MusicXML.h"
#include <wx/zipstrm.h>
#include <wx/wfstream.h>
#include <wx/log.h>
#include "spdlog/spdlog.h"
#include "ExternalHooks.h"

bool MusicXML::IsOk()
{
    return _loaded;
}

void MusicXmlNote::Dump()
{
    spdlog::info("{}, {}, {}", startMS, startMS + durationMS, midi);
}

MusicXML::MusicXML(std::string file)
{
    wxLogNull logNo; //kludge: avoid zip errors

    if (file != "" && FileExists(file))
    {
        spdlog::info("Loading music XML file: {}", file);
        wxFileInputStream fin(file);
        if (fin.IsOk())
        {
            wxZipInputStream zin(fin);

            if (zin.IsOk())
            {
                wxZipEntry* ent = zin.GetNextEntry();

                while (ent != nullptr && (ent->GetName().Contains("\\") || ent->GetName().Contains("/")))
                {
                    spdlog::info("    Found in zip file {} ... skipping", ent->GetName().ToStdString());
                    ent = zin.GetNextEntry();
                }

                if (ent != nullptr)
                {
                    spdlog::info("    Found in zip file {} ... loading", ent->GetName().ToStdString());
                    // Read zip entry into buffer for pugixml
                    std::string buf;
                    char chunk[4096];
                    while (!zin.Eof()) {
                        size_t read = zin.Read(chunk, sizeof(chunk)).LastRead();
                        buf.append(chunk, read);
                    }
                    _loaded = _doc.load_buffer(buf.data(), buf.size());
                }
            }
        }

        if (!IsOk())
        {
            spdlog::info("    Not a zip file {} ... loading as xml", file);
            _loaded = (bool)_doc.load_file(file.c_str());
        }

        if (!IsOk())
        {
            spdlog::warn("    Error loading music xml file.");
        }
    }
}


MusicXML::~MusicXML()
{
}

std::list<MusicXmlNote> MusicXML::GetNotes(std::string track)
{
    // prime the parts
    if (_parts.size() == 0)
    {
        GetTracks();
    }

    std::string partid = "All";
    if (_parts.find(track) != _parts.end())
    {
        partid = _parts[track];
    }

    std::list<MusicXmlNote> res;

    pugi::xml_node root = _doc.document_element();

    int tempo = 60;
    int divisions = 100;

    if (std::string_view(root.name()) == "score-partwise")
    {
        for (pugi::xml_node n1 = root.first_child(); n1; n1 = n1.next_sibling())
        {
            if (std::string_view(n1.name()) == "part" && (std::string_view(n1.attribute("id").as_string()) == partid || track == "All"))
            {
                int current = 0;
                for (pugi::xml_node n2 = n1.first_child(); n2; n2 = n2.next_sibling())
                {
                    if (std::string_view(n2.name()) == "measure")
                    {
                        for (pugi::xml_node n3 = n2.first_child(); n3; n3 = n3.next_sibling())
                        {
                            if (std::string_view(n3.name()) == "direction")
                            {
                                for (pugi::xml_node n4 = n3.first_child(); n4; n4 = n4.next_sibling())
                                {
                                    if (std::string_view(n4.name()) == "sound" && n4.attribute("tempo"))
                                    {
                                        tempo = n4.attribute("tempo").as_int();
                                    }
                                }
                            }
                            else if (std::string_view(n3.name()) == "attributes")
                            {
                                for (pugi::xml_node n4 = n3.first_child(); n4; n4 = n4.next_sibling())
                                {
                                    if (std::string_view(n4.name()) == "divisions")
                                    {
                                        divisions = n4.text().as_int();
                                    }
                                }
                            }
                            else if(std::string_view(n3.name()) == "note")
                            {
                                MusicXmlNote note(n3, current);
                                res.push_back(note);
                                current += note.duration;
                            }
                        }
                    }
                }
            }
        }
    }

    // tempo is beats per minute
    // divisions is divisions per quarter beat (hence multiplying it by 4)
    if (tempo == 0) tempo = 60;
    if (divisions == 0) divisions = 1;
    float timeperduration = ((60.0 * 1000.0) / tempo) / (4.0 * divisions);

    spdlog::info("BeatTime {:.2f}ms", timeperduration);
    spdlog::info("StartMS, EndMS, Note");
    for (auto it = res.begin(); it != res.end(); ++it)
    {
        it->ApplyTiming(timeperduration);
        it->Dump();
    }

    return res;
}

std::list<std::string> MusicXML::GetTracks()
{
    std::list<std::string> res;

    if (IsOk())
    {
        _parts.clear();
        pugi::xml_node root = _doc.document_element();

        if (std::string_view(root.name()) == "score-partwise")
        {
            for (pugi::xml_node n1 = root.first_child(); n1; n1 = n1.next_sibling())
            {
                if (std::string_view(n1.name()) == "part-list")
                {
                    for (pugi::xml_node n2 = n1.first_child(); n2; n2 = n2.next_sibling())
                    {
                        if (std::string_view(n2.name()) == "score-part")
                        {
                            std::string id = n2.attribute("id").as_string();
                            for (pugi::xml_node n3 = n2.first_child(); n3; n3 = n3.next_sibling())
                            {
                                if (std::string_view(n3.name()) == "part-name")
                                {
                                    std::string pn = n3.text().get();
                                    if (pn == "")
                                    {
                                        _parts[id] = id;
                                        pn = id;
                                    }
                                    else
                                    {
                                        _parts[pn] = id;
                                    }
                                    res.push_back(pn);
                                }
                            }
                        }
                    }
                    break;
                }
            }
        }
    }

    return res;
}

MusicXmlNote::MusicXmlNote(pugi::xml_node node, int s) : duration(0), durationMS(0), midi(0), start(0), startMS(0)
{
    start = s;
    for (pugi::xml_node n1 = node.first_child(); n1; n1 = n1.next_sibling())
    {
        if (std::string_view(n1.name()) == "pitch")
        {
            std::string Note = "";
            int Octave = 4;
            int Adjust = 0;
            for (pugi::xml_node n2 = n1.first_child(); n2; n2 = n2.next_sibling())
            {
                if (std::string_view(n2.name()) == "step")
                {
                    Note = n2.text().get();
                }
                else if (std::string_view(n2.name()) == "octave")
                {
                    Octave = n2.text().as_int();
                }
                else if (std::string_view(n2.name()) == "alter")
                {
                    Adjust = n2.text().as_int();
                }
            }

            if (Note == "A")
            {
                midi = 9;
            }
            else if (Note == "B")
            {
                midi = 11;
            }
            else if (Note == "C")
            {
                midi = 0;
            }
            else if (Note == "D")
            {
                midi = 2;
            }
            else if (Note == "E")
            {
                midi = 4;
            }
            else if (Note == "F")
            {
                midi = 5;
            }
            else if (Note == "G")
            {
                midi = 7;
            }
            midi += Adjust;
            midi += (Octave + 1) * 12;
        }
        else if (std::string_view(n1.name()) == "duration")
        {
            duration = n1.text().as_int();
        }
        else if (std::string_view(n1.name()) == "rest")
        {
            midi = -1;
        }
    }
}
