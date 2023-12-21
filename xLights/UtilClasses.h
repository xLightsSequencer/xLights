#pragma once

/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/smeighan/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/smeighan/xLights/blob/master/License.txt
 **************************************************************/

#include <map>
#include <string>
#include <algorithm>

#include <wx/filepicker.h>
#include "UtilFunctions.h"

class EffectManager;


class MapStringString: public std::map<std::string,std::string> {
public:
    MapStringString(): std::map<std::string,std::string>() {
    }
    virtual ~MapStringString() {}

    const std::string &operator[](const std::string &key) const {
        return Get(key, xlEMPTY_STRING);
    }
    std::string &operator[](const std::string &key) {
        return std::map<std::string, std::string>::operator[](key);
    }
    int GetInt(const std::string &key, const int def = 0) const {
        std::map<std::string,std::string>::const_iterator i(find(key));
        if (i == end() || i->second.length() == 0 || i->second.at(0) == ' ') {
            return def;
        }
        try {
            return stoi(i->second);
        } catch ( ... ) {
            return def;
        }
    }
    float GetFloat(const std::string& key, const float def = 0.0) const
    {
        std::map<std::string, std::string>::const_iterator i(find(key));
        if (i == end() || i->second.length() == 0 || i->second.at(0) == ' ') {
            return def;
        }
        try {
            return stof(i->second);
        }
        catch (...) {
            return def;
        }
    }
    double GetDouble(const std::string& key, const double def = 0.0) const
    {
        std::map<std::string, std::string>::const_iterator i(find(key));
        if (i == end() || i->second.length() == 0 || i->second.at(0) == ' ') {
            return def;
        }
        try {
            return stod(i->second);
        }
        catch (...) {
            return def;
        }
    }
    bool GetBool(const std::string& key, const bool def = false) const
    {
        std::map<std::string, std::string>::const_iterator i(find(key));
        if (i == end()) {
            return def;
        }
        return i->second.length() >= 1 && i->second.at(0) == '1';
    }
    const std::string& Get(const std::string& key, const std::string& def) const
    {
        std::map<std::string, std::string>::const_iterator i(find(key));
        if (i == end()) {
            return def;
        }
        return i->second;
    }

    std::string Get(const std::string& key, const char* def) const
    {
        std::map<std::string, std::string>::const_iterator i(find(key));
        if (i == end()) {
            return def;
        }
        return i->second;
    }

    bool Contains(const std::string& key) const
    {
        std::map<std::string, std::string>::const_iterator i(find(key));
        if (i == end()) {
            return false;
        }
        return true;
    }

    const std::string& operator[](const char* key) const
    {
        return Get(key, xlEMPTY_STRING);
    }
    std::string& operator[](const char* ckey)
    {
        std::string key(ckey);
        return std::map<std::string, std::string>::operator[](key);
    }
    int GetInt(const char* ckey, const int def = 0) const
    {
        return GetInt(std::string(ckey), def);
    }
    double GetDouble(const char* ckey, const double& def = 0.0) const
    {
        return GetDouble(std::string(ckey), def);
    }
    float GetFloat(const char* ckey, const float& def = 0.0) const
    {
        return GetFloat(std::string(ckey), def);
    }
    bool GetBool(const char* ckey, const bool def = false) const
    {
        return GetBool(std::string(ckey), def);
    }
    const std::string& Get(const char* ckey, const std::string& def) const
    {
        return Get(std::string(ckey), def);
    }
    bool Contains(const char* ckey) const
    {
        std::string key(ckey);
        return Contains(key);
    }

    std::string Get(const char* ckey, const char* def) const
    {
        std::string key(ckey);
        std::map<std::string, std::string>::const_iterator i(find(key));
        if (i == end()) {
            return def;
        }
        return i->second;
    }
    size_type erase(const char* ckey)
    {
        std::string key(ckey);
        return std::map<std::string, std::string>::erase(key);
    }
    size_type erase(const std::string& key)
    {
        return std::map<std::string, std::string>::erase(key);
    }

    void ParseJson(EffectManager* effectManager, const std::string& str, const std::string& effectName);
    void Parse(EffectManager* effectManager, const std::string& str, const std::string& effectName);

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
    [[nodiscard]]std::string AsJSON() const
    {
        std::string ret ;
        for (std::map<std::string, std::string>::const_iterator it = begin(); it != end(); ++it) {
            if (ret.length() != 0) {
                ret += ",";
            }
            std::string value = it->second;
            ReplaceAll(value, "&", "&amp;");   // need to escape the amps
            ReplaceAll(value, ",", "&comma;"); // need to escape the commas
            ret += "\"" + it->first + "\":\"" + value + "\"";
        }
        ret.insert(0,"{");
        ret.append("}");
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

    static void Trim(std::string& s)
    {
        s.erase(s.begin(), std::find_if_not(s.begin(), s.end(), [](char c) { return std::isspace(c); }));
        s.erase(std::find_if_not(s.rbegin(), s.rend(), [](char c) { return std::isspace(c); }).base(), s.end());
    }
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
        wxFilePickerCtrl(parent, id, path, message, "Image files|*.png;*.bmp;*.jpg;*.gif;*.jpeg"
                                                    ";*.webp"
                                                    "|All files (*.*)|*.*",
                         pos, size, style, validator, name)
    {
    }
};

class LogarithmicScale {
public:
	static int GetLogSum(int to)
	{
		static std::vector<double> logarithmicX = {
			18.17223207,
			10.63007432,
			7.542157755,
			5.850152051,
			4.779922266,
			4.041366691,
			3.500791064,
			3.087916561,
			2.76223549,
			2.498745944,
			2.281176321,
			2.098478794,
			1.942887898,
			1.808785359,
			1.692005705,
			1.589395049,
			1.498521512,
			1.41747975,
			1.344755739,
			1.279131202,
			1.219614742,
			1.165391374,
			1.115784947,
			1.070229785,
			1.028249009,
			0.989437767,
			0.95345013,
			0.919988749,
			0.88879661,
			0.859650427,
			0.832355277,
			0.80674024,
			0.782654809,
			0.759965938,
			0.738555574,
			0.718318607,
			0.699161143,
			0.680999044,
			0.663756696,
			0.647365955,
			0.631765247,
			0.616898794,
			0.602715949,
			0.589170617,
			0.576220758,
			0.563827948,
			0.551956999,
			0.540575628,
			0.529654157,
			0.519165264,
			0.509083745,
			0.49938632,
			0.490051447,
			0.481059168,
			0.472390962,
			0.46402962,
			0.455959129,
			0.448164573,
			0.440632038,
			0.433348529,
			0.426301898,
			0.419480775,
			0.412874503,
			0.406473089,
			0.40026715,
			0.394247867,
			0.388406942,
			0.382736565,
			0.377229373,
			0.371878421,
			0.366677153,
			0.361619376,
			0.356699232,
			0.351911178,
			0.347249965,
			0.34271062,
			0.338288424,
			0.3339789,
			0.329777796,
			0.325681071,
			0.321684884,
			0.317785577,
			0.31397967,
			0.310263847,
			0.306634947,
			0.303089955,
			0.299625994,
			0.296240317,
			0.2929303,
			0.289693435,
			0.286527323,
			0.28342967,
			0.280398278,
			0.277431045,
			0.274525954,
			0.271681075,
			0.268894553,
			0.266164612,
			0.263489545,
			0.260867715,
			0.258297548,
			0.255777532,
			0.253306213,
			0.250882193,
			0.248504127,
			0.24617072,
			0.243880727,
			0.241632946,
			0.239426222,
			0.237259439,
			0.235131523,
			0.233041437,
			0.230988182,
			0.228970792,
			0.226988336,
			0.225039915,
			0.223124658,
			0.221241727,
			0.21939031,
			0.217569623,
			0.215778906,
			0.214017426,
			0.212284472,
			0.210579358,
			0.208901417,
			0.207250005,
			0.0
		};

		double sum = 0;
		for (int i = 0; i < to && i < 127; i++)
		{
			sum += logarithmicX[i];
		}

		return sum;
	}
};
