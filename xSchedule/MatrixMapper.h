#pragma once

/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include <string>
#include <vector>

class wxXmlNode;
class OutputManager;

typedef enum { HORIZONTAL,
               VERTICAL } MMORIENTATION;
typedef enum { BOTTOM_LEFT,
               BOTTOM_RIGHT,
               TOP_LEFT,
               TOP_RIGHT } MMSTARTLOCATION;

class MatrixMapper {
    OutputManager* _outputManager;
    int _changeCount;
    int _lastSavedChangeCount;
    std::string _name;
    std::string _fromModel = "";
    int _strings;
    int _strandsPerString;
    int _stringLength;
    MMORIENTATION _orientation;
    MMSTARTLOCATION _startLocation;
    std::string _startChannel;
    std::string _displayAs = "";
    bool _valid = true;
    std::vector<std::vector<std::vector<int>>> _customModelData;

    static std::vector<std::vector<std::vector<int>>> ParseCustomModel(const std::string& customModel);
    static std::vector<std::vector<std::vector<int>>> ParseCompressed(const std::string& compressed);
    bool LoadModel();
    size_t MapCustom(int x, int y) const;

public:
    static std::vector<std::vector<std::vector<int>>> ParseCustomModelDataFromXml(const wxXmlNode* node);
    static MMORIENTATION EncodeOrientation(const std::string orientation);
    static std::string DecodeOrientation(MMORIENTATION orientation);
    static MMSTARTLOCATION EncodeStartLocation(const std::string startLocation);
    static std::string DecodeStartLocation(MMSTARTLOCATION startLocation);
    static bool IsSupportedDisplayAs(const std::string& displayAs);

    MatrixMapper(OutputManager* outputManager, wxXmlNode* n);
    MatrixMapper(OutputManager* outputManager, int strings, int strandsPerString, int stringLength, MMORIENTATION orientation, MMSTARTLOCATION startLocation, const std::string& startChannel, const std::string& name, const std::string& fromModel);
    MatrixMapper(OutputManager* outputManager, int strings, int strandsPerString, int stringLength, const std::string& orientation, const std::string& startLocation, const std::string& startChannel, const std::string& name, const std::string& fromModel);
    MatrixMapper(OutputManager* outputManager);
    virtual ~MatrixMapper() {
    }
    size_t Map(int x, int y) const;
    size_t GetChannels() const;
    int GetWidth() const;
    int GetHeight() const;
    std::string GetStartChannel() const {
        return _startChannel;
    }
    long GetStartChannelAsNumber() const;
    std::string GetName() const {
        return _name;
    }
    int GetStrings() const {
        return _strings;
    }
    int GetStringLength() const {
        return _stringLength;
    }
    int GetStrandsPerString() const {
        return _strandsPerString;
    }
    std::string GetStartLocation() const {
        return DecodeStartLocation(_startLocation);
    }
    std::string GetOrientation() const {
        return DecodeOrientation(_orientation);
    }
    bool IsDirty() const {
        return _lastSavedChangeCount != _changeCount;
    }
    void ClearDirty() {
        _lastSavedChangeCount = _changeCount;
    }
    wxXmlNode* Save();
    std::string GetFromModel() const {
        return _fromModel;
    }
    void SetName(const std::string& name) {
        if (name != _name) {
            _name = name;
            _changeCount++;
        }
    }
    void SetStrings(const int strings) {
        if (strings != _strings) {
            _strings = strings;
            _changeCount++;
        }
    }
    void SetStringLength(const int stringLength) {
        if (stringLength != _stringLength) {
            _stringLength = stringLength;
            _changeCount++;
        }
    }
    void SetStrandsPerString(const int strandsPerString) {
        if (strandsPerString != _strandsPerString) {
            _strandsPerString = strandsPerString;
            if (_strandsPerString < 1)
                _strandsPerString = 1;
            _changeCount++;
        }
    }
    void SetFromModel(const std::string& fromModel) {
        if (fromModel != _fromModel) {
            _fromModel = fromModel;
            _changeCount++;
        }
    }
    bool IsValid() const {
        return _valid;
    }
    static void Test(OutputManager* outputManager);
    std::string GetConfigDescription() const;
};
