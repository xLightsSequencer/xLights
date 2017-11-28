#include "MusicXML.h"
#include <wx/xml/xml.h>
#include <wx/zipstrm.h>
#include <wx/wfstream.h>
#include <log4cpp/Category.hh>

bool MusicXML::IsOk()
{
    return _doc.IsOk();
}

void MusicXmlNote::Dump()
{
    static log4cpp::Category &logger_pianodata = log4cpp::Category::getInstance(std::string("log_pianodata"));
    logger_pianodata.info("%d, %d, %d", startMS, startMS + durationMS, midi);
}

MusicXML::MusicXML(std::string file)
{
    if (file != "" && wxFile::Exists(file))
    {
        static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));
        logger_base.info("Loading music XML file: %s", (const char *)file.c_str());
        wxFileInputStream fin(file);
        wxZipInputStream zin(fin);
        wxZipEntry *ent = zin.GetNextEntry();

        while (ent != nullptr && (ent->GetName().Contains("\\") || ent->GetName().Contains("/")))
        {
            logger_base.info("    Found in zip file %s ... skipping", (const char *)ent->GetName().c_str());
            ent = zin.GetNextEntry();
        }

        if (ent != nullptr)
        {
            logger_base.info("    Found in zip file %s ... loading", (const char *)ent->GetName().c_str());
            _doc.Load(zin);
        }

        if (!IsOk())
        {
            logger_base.warn("    Error loading music xml file.");
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

    int timeperduration = 500;

    std::list<MusicXmlNote> res;

    wxXmlNode* root = _doc.GetRoot();

    if (root->GetName() == "score-partwise")
    {
        for (wxXmlNode* n1 = root->GetChildren(); n1 != nullptr; n1 = n1->GetNext())
        {
            if (n1->GetName() == "part" && (n1->GetAttribute("id").ToStdString() == partid || track == "All"))
            {
                int current = 0;
                for (wxXmlNode* n2 = n1->GetChildren(); n2 != nullptr; n2 = n2->GetNext())
                {
                    if (n2->GetName() == "measure")
                    {
                        for (wxXmlNode* n3 = n2->GetChildren(); n3 != nullptr; n3 = n3->GetNext())
                        {
                            if (n3->GetName() == "direction")
                            {
                                for (wxXmlNode* n4 = n3->GetChildren(); n4 != nullptr; n4 = n4->GetNext())
                                {
                                    if (n4->GetName() == "sound" && n4->HasAttribute("tempo"))
                                    {
                                        timeperduration = 60 * 1000 / wxAtoi(n4->GetAttribute("tempo"));
                                    }
                                }
                            }
                            else if(n3->GetName() == "note")
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

    static log4cpp::Category &logger_pianodata = log4cpp::Category::getInstance(std::string("log_pianodata"));
    logger_pianodata.info("BeatTime %dms", timeperduration);
    logger_pianodata.info("StartMS, EndMS, Note");
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
        wxXmlNode* root = _doc.GetRoot();

        if (root->GetName() == "score-partwise")
        {
            for (wxXmlNode* n1 = root->GetChildren(); n1 != nullptr; n1= n1->GetNext())
            {
                if (n1->GetName() == "part-list")
                {
                    for (wxXmlNode* n2 = n1->GetChildren(); n2 != nullptr; n2 = n2->GetNext())
                    {
                        if (n2->GetName() == "score-part")
                        {
                            std::string id = n2->GetAttribute("id").ToStdString();
                            for (wxXmlNode* n3 = n2->GetChildren(); n3 != nullptr; n3 = n3->GetNext())
                            {
                                if (n3->GetName() == "part-name")
                                {
                                    std::string pn = n3->GetNodeContent().ToStdString();
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

MusicXmlNote::MusicXmlNote(wxXmlNode* node, int s) : duration(0), durationMS(0), midi(0), start(0), startMS(0)
{
    start = s;
    for (wxXmlNode* n1 = node->GetChildren(); n1 != nullptr; n1 = n1->GetNext())
    {
        if (n1->GetName() == "pitch")
        {
            wxString Note = "";
            int Octave = 4;
            int Adjust = 0;
            for (wxXmlNode* n2 = n1->GetChildren(); n2 != nullptr; n2 = n2->GetNext())
            {
                if (n2->GetName() == "step")
                {
                    Note = n2->GetNodeContent();
                }
                else if (n2->GetName() == "octave")
                {
                    Octave = wxAtoi(n2->GetNodeContent());
                }
                else if (n2->GetName() == "alter")
                {
                    Adjust = wxAtoi(n2->GetNodeContent());
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
        else if (n1->GetName() == "duration")
        {
            duration = wxAtoi(n1->GetNodeContent());
        }
        else if (n1->GetName() == "rest")
        {
            midi = -1;
        }
    }
}
