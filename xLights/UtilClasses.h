#ifndef UTILCLASSES_H
#define UTILCLASSES_H

#include "wx/wx.h"
#include <map>


class MapStringString: public std::map<wxString,wxString> {
public:
    MapStringString(): std::map<wxString,wxString>() {
    }
    virtual ~MapStringString() {}
    
    const wxString &operator[](const wxString &key) const {
        return Get(key, notFound);
    }
    wxString &operator[](const wxString &key) {
        return std::map<wxString, wxString>::operator[](key);
    }
    const wxString &Get(const wxString &key, const wxString &def) const {
        std::map<wxString,wxString>::const_iterator i(find(key));
        if (i == end()) {
            return def;
        }
        return i->second;
    }
    
    void Parse(const wxString &str) {
        clear();
        wxString before,after,name,value;
        wxString settings(str);
        while (!settings.IsEmpty()) {
            before=settings.BeforeFirst(',');
            settings=settings.AfterFirst(',');
            
            name=before.BeforeFirst('=');
            value=before.AfterFirst('=');
            value.Replace("&comma;", ",", true); //unescape the commas
            value.Replace("&amp;", "&", true); //unescape the amps

            RemapKey(name, value);
            if (!name.empty()) {
                (*this)[name]=value;
            }
        }
    }
    
    virtual void RemapKey(wxString &n, wxString &value) {};
    
    wxString AsString() const {
        wxString ret;
        for (std::map<wxString,wxString>::const_iterator it=begin(); it!=end(); ++it) {
            if (ret.Length() != 0) {
                ret += ",";
            }
            wxString value = it->second;
            value.Replace("&", "&amp;", true); //need to escape the amps
            value.Replace(",", "&comma;", true); //need to escape the commas
            ret += it->first + "=" + value;
        }
        return ret;
    }
    
    
private:
    wxString notFound;
};

class SettingsMap: public MapStringString {
public:
    SettingsMap(): MapStringString() {
    }
    virtual ~SettingsMap() {}
    
    virtual void RemapKey(wxString &n, wxString &value) {
        RemapChangedSettingKey(n, value);
    }
private:
    static void RemapChangedSettingKey(wxString &n,  wxString &value);
};


#endif