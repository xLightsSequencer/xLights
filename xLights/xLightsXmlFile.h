#ifndef XLIGHTSXMLFILE_H
#define XLIGHTSXMLFILE_H

#include <wx/filename.h>
#include <wx/xml/xml.h>

class xLightsXmlFile : public wxFileName
{
    public:
        xLightsXmlFile();
        virtual ~xLightsXmlFile();

        void Load();
        void Clear();
        bool IsLoaded() { return is_loaded; }
        int GetNumModels() { return models.GetCount(); }
        const wxString GetVersion();
    protected:
    private:
        wxArrayString models;
        wxArrayString timing_protection;
        wxArrayString timing;
        wxArrayString label_protection;
        wxArrayString labels;
        wxArrayString effect_protection;
        wxArrayString effects;
        wxXmlDocument seqDocument;
        bool is_loaded;
        int major_version;
        wxString version_string;

};

#endif // XLIGHTSXMLFILE_H
