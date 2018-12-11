#ifndef UTILCLASSES_H
#define UTILCLASSES_H

#include <map>
#include <string>
#include <algorithm>
#include <wx/filepicker.h>


class MapStringString: public std::map<std::string,std::string> {
public:
    MapStringString(): std::map<std::string,std::string>() {
    }
    virtual ~MapStringString() {}

    const std::string &operator[](const std::string &key) const {
        return Get(key, EMPTY_STRING);
    }
    std::string &operator[](const std::string &key) {
        return std::map<std::string, std::string>::operator[](key);
    }
    int GetInt(const std::string &key, const int def = 0) const {
        std::map<std::string,std::string>::const_iterator i(find(key));
        if (i == end() || i->second.length() == 0) {
            return def;
        }
        try {
            return stoi(i->second);
        } catch ( ... ) {
            return def;
        }
    }
    float GetFloat(const std::string &key, const float def = 0.0) const {
        std::map<std::string,std::string>::const_iterator i(find(key));
        if (i == end() || i->second.length() == 0) {
            return def;
        }
        try {
            return stof(i->second);
        } catch ( ... ) {
            return def;
        }
    }
    double GetDouble(const std::string &key, const double def = 0.0) const {
        std::map<std::string,std::string>::const_iterator i(find(key));
        if (i == end() || i->second.length() == 0) {
            return def;
        }
        try {
            return stod(i->second);
        } catch ( ... ) {
            return def;
        }
    }
    bool GetBool(const std::string &key, const bool def = false) const {
        std::map<std::string,std::string>::const_iterator i(find(key));
        if (i == end()) {
            return def;
        }
        return i->second.length() >= 1 && i->second.at(0) == '1';
    }
    const std::string &Get(const std::string &key, const std::string &def) const {
        std::map<std::string,std::string>::const_iterator i(find(key));
        if (i == end()) {
            return def;
        }
        return i->second;
    }
    std::string Get(const std::string &key, const char *def) const {
        std::map<std::string,std::string>::const_iterator i(find(key));
        if (i == end()) {
            return def;
        }
        return i->second;
    }
    bool Contains(const std::string &key) const {
        std::map<std::string, std::string>::const_iterator i(find(key));
        if (i == end()) {
            return false;
        }
        return true;
    }
    



    const std::string &operator[](const char *key) const {
        return Get(key, EMPTY_STRING);
    }
    std::string &operator[](const char *ckey) {
        std::string key(ckey);
        return std::map<std::string, std::string>::operator[](key);
    }
    int GetInt(const char * ckey, const int def = 0) const {
        return GetInt(std::string(ckey), def);
    }
    double GetDouble(const char *ckey, const double &def = 0.0) const {
        return GetDouble(std::string(ckey), def);
    }
    float GetFloat(const char *ckey, const float &def = 0.0) const {
        return GetFloat(std::string(ckey), def);
    }
    bool GetBool(const char *ckey, const bool def = false) const {
        return GetBool(std::string(ckey), def);
    }
    const std::string &Get(const char *ckey, const std::string &def) const {
        return Get(std::string(ckey), def);
    }
    bool Contains(const char* ckey) const {
        std::string key(ckey);
        return Contains(key);
    }

    std::string Get(const char *ckey, const char *def) const {
        std::string key(ckey);
        std::map<std::string,std::string>::const_iterator i(find(key));
        if (i == end()) {
            return def;
        }
        return i->second;
    }
    size_type erase(const char *ckey) {
        std::string key(ckey);
        return std::map<std::string,std::string>::erase(key);
    }
    size_type erase(const std::string &key) {
        return std::map<std::string,std::string>::erase(key);
    }


    void Parse(const std::string &str) {
        clear();
        std::string before,after,name,value;
        std::string settings(str);
        while (!settings.empty()) {
            size_t start_pos = settings.find(',');
            if (start_pos != std::string::npos) {
                before = settings.substr(0, start_pos);
                settings = settings.substr(start_pos + 1);
            } else {
                before = settings;
                settings = "";
            }

            start_pos = before.find('=');
            name = before.substr(0, start_pos);
            value = before.substr(start_pos + 1);
            ReplaceAll(value, "&comma;", ","); //unescape the commas
            ReplaceAll(value, "&amp;", "&"); //unescape the amps

            RemapKey(name, value);
            if (!name.empty()) {
                (*this)[name]=value;
            }
        }
    }

    virtual void RemapKey(std::string &n, std::string &value) {};
    std::string AsString() const {
        std::string ret;
        for (std::map<std::string,std::string>::const_iterator it=begin(); it!=end(); ++it) {
            if (ret.length() != 0) {
                ret += ",";
            }
            std::string value = it->second;
            ReplaceAll(value, "&", "&amp;"); //need to escape the amps
            ReplaceAll(value, ",", "&comma;"); //need to escape the commas
            ret += it->first + "=" + value;
        }
        return ret;
    }

private:

    void ReplaceAll(std::string &str, const std::string& from, const std::string& to) const {
        size_t start_pos = 0;
        while((start_pos = str.find(from, start_pos)) != std::string::npos) {
            str.replace(start_pos, from.length(), to);
            start_pos += to.length();
        }
    }

    static const std::string EMPTY_STRING;
};

class SettingsMap: public MapStringString {
public:
    SettingsMap(): MapStringString() {
    }
    virtual ~SettingsMap() {}

    virtual void RemapKey(std::string &n, std::string &value) {
        RemapChangedSettingKey(n, value);
    }
private:
    static void RemapChangedSettingKey(std::string &n,  std::string &value);
};

class RangeAccumulator
{
    std::list<std::pair<int,int>> _ranges;
    void ResolveOverlaps(int minSeparation);

public:
    RangeAccumulator() {}
    virtual ~RangeAccumulator() {}
    void Add(int low, int high);
    std::list<std::pair<int, int>>::iterator begin() {
        return _ranges.begin(); 
    }
    std::list<std::pair<int, int>>::iterator end() { return _ranges.end(); }
    size_t size() const { return _ranges.size(); }
    void clear() { _ranges.clear(); }
    void Consolidate(int minSeparation = 0) { ResolveOverlaps(minSeparation); }
    std::pair<int, int> front() const { return _ranges.front(); }
    std::pair<int, int> back() const { return _ranges.back(); }
};

class ImageFilePickerCtrl : public wxFilePickerCtrl
{
public:
    ImageFilePickerCtrl(wxWindow *parent, wxWindowID id, const wxString& path, const wxString& message, const wxString& wildcard, const wxPoint &pos, const wxSize &size, long style, const wxValidator &validator, const wxString &name) :
        wxFilePickerCtrl(parent, id, path, message, "Image files|*.png;*.bmp;*.jpg;*.gif|All files (*.*)|*.*", pos, size, style, validator, name)
    {
    }
};
#endif
