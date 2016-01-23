#ifndef UTILCLASSES_H
#define UTILCLASSES_H

#include <map>
#include <string>
#include <algorithm>

/* for now, to determine if we need the wxString variants */
#include <wx/string.h>

#ifdef __WXMSW__
/* the c++11 runtime in mingw is broken.  It doesn't include these as the spec says it should */
#include <cstdlib>
#define stod(x) atof(x.c_str())
#define stof(x) atof(x.c_str())
#define stoi(x) strtol(x.c_str(), nullptr, 10)
#endif

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
    std::string Get(const char *ckey, const char *def) const {
        std::string key(ckey);
        std::map<std::string,std::string>::const_iterator i(find(key));
        if (i == end()) {
            return def;
        }
        return i->second;
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

#ifndef wxUSE_STD_STRING
    /* deprecated wxString versions, need to rebuild wxWidgets with --with-stl */
    /* On platforms that use the std::string, we don't need these */
    const std::string &operator[](const wxString &wkey) const {
        std::map<std::string,std::string>::const_iterator i(find(wkey.ToStdString()));
        if (i == end()) {
            return EMPTY_STRING;
        }
        return i->second;
    }
    std::string &operator[](const wxString &wkey) {
        return std::map<std::string, std::string>::operator[](wkey.ToStdString());
    }
    int GetInt(const wxString &wkey, const int def) const {
        return GetInt(wkey.ToStdString(), def);
    }
    double GetDouble(const wxString &wkey, const double &def) const {
        return GetDouble(wkey.ToStdString(), def);
    }
    bool GetBool(const wxString &wkey, const bool def = false) const {
        return GetBool(wkey.ToStdString(), def);
    }
    const std::string Get(const wxString &wkey, const wxString &def) const {
        std::map<std::string,std::string>::const_iterator i(find(wkey.ToStdString()));
        if (i == end()) {
            return def.ToStdString();
        }
        return i->second;
    }
    std::string Get(const wxString &wkey, const char *def) const {
        return Get(wkey.ToStdString(), def);
    }
#endif

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


#endif
