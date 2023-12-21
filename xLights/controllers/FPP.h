#pragma once

#include <list>
#include <map>
#include <set>
#include <algorithm>
#include <string>

#include "../models/ModelManager.h"
#include "ControllerUploadData.h"
#include "BaseController.h"

class wxJSONValue;
class FSEQFile;
typedef void CURL;
class wxWindow;
class wxGauge;
class FPPUploadProgressDialog;
class Discovery;

enum class FPP_TYPE { FPP,
                      FALCONV4V5,
                      ESPIXELSTICK,
                      GENIUS };

class FPP : public BaseController
{
    public:
    FPP() :
            BaseController("", ""), majorVersion(0), minorVersion(0), outputFile(nullptr), parent(nullptr), fppType(FPP_TYPE::FPP) {}
    FPP(const std::string& ip_, const std::string& proxy_, const std::string& model_);
    FPP(const std::string &address);
    FPP(const FPP &c);
    virtual ~FPP();
    
    std::string hostName;
    std::string description;
    std::string ipAddress;
    std::string fullVersion;
    std::string platform;
    std::string model;
    uint32_t majorVersion = 0;
    uint32_t minorVersion = 0;
    uint32_t patchVersion = 0;
    std::string ranges;
    std::string mode;
    std::string pixelControllerType;
    std::string panelSize;
    std::string uuid = "";
    std::list<std::string> playlists;

    std::string proxy;
    std::set<std::string> proxies;

    std::string username;
    std::string password;
    FPP_TYPE fppType = FPP_TYPE::FPP;
    
    std::string controllerVendor;
    std::string controllerModel;
    std::string controllerVariant;

    wxWindow *parent = nullptr;
    void setProgress(FPPUploadProgressDialog*d, wxGauge *g) { progressDialog = d; progress = g; }
    bool updateProgress(int val, bool yield);

    
    std::list<std::string> messages;
    int defaultConnectTimeout = 2000;

    std::map<int, int> GetExpansionPorts(ControllerCaps* caps) const;
    bool AuthenticateAndUpdateVersions();
    void probePixelControllerType();
    
    void UpdateChannelRanges();
    void FillRanges(std::map<int, int> &rngs);
    void SetNewRanges(const std::map<int, int> &rngs);
    bool IsDDPInputEnabled();

    bool IsVersionAtLeast(uint32_t maj, uint32_t min, uint32_t patch = 0) const;

#ifndef DISCOVERYONLY
    bool PrepareUploadSequence(FSEQFile *file,
                               const std::string &seq,
                               const std::string &media,
                               int type);
    bool CheckUploadMedia(const std::string &media, std::string &mediaBaseName);
    bool WillUploadSequence() const;
    bool NeedCustomSequence() const;
    bool AddFrameToUpload(uint32_t frame, uint8_t *data);
    bool FinalizeUploadSequence();
    std::string GetTempFile() const { return tempFileName; }
    void ClearTempFile() { tempFileName = ""; }
#endif
    bool supportedForFPPConnect() const;

    bool UploadUDPOutputsForProxy(OutputManager* outputManager);
    
    bool UploadPlaylist(const std::string &playlist);
    bool UploadModels(const wxJSONValue &models);
    bool UploadDisplayMap(const std::string &displayMap);
    bool UploadUDPOut(const wxJSONValue &udp);

    bool UploadPixelOutputs(ModelManager* allmodels,
                            OutputManager* outputManager,
                            Controller* controller);
    bool UploadPanelOutputs(ModelManager* allmodels,
                            OutputManager* outputManager,
                            Controller* controller);
    bool UploadVirtualMatrixOutputs(ModelManager* allmodels,
                                    OutputManager* outputManager,
                                    Controller* controller);
    bool UploadSerialOutputs(ModelManager* allmodels,
                             OutputManager* outputManager,
                             Controller* controller);
    bool SetInputUniversesBridge(Controller* controller);

    bool UploadControllerProxies(OutputManager* outputManager);

    bool SetRestartFlag();
    bool Restart(bool ifNeeded = false);
    void SetDescription(const std::string &st);
    std::vector<std::string> GetProxies();

    static void PrepareDiscovery(Discovery &discovery);
    static void PrepareDiscovery(Discovery &discovery, const std::list<std::string> &addresses, bool broadcastPing = true);
    static void MapToFPPInstances(Discovery &discovery, std::list<FPP*> &instances, OutputManager* outputManager);
    static bool ValidateProxy(const std::string& to, const std::string& via);

    static void TypeIDtoControllerType(int typeId, FPP* inst);
    static std::list<FPP*> GetInstances(wxWindow* frame, OutputManager* outputManager);

#ifndef DISCOVERYONLY
    wxJSONValue CreateModelMemoryMap(ModelManager* allmodels, int32_t startChan, int32_t endChannel);
    static std::string CreateVirtualDisplayMap(ModelManager* allmodels);
    static wxJSONValue CreateUniverseFile(const std::list<Controller*>& controllers, bool input, std::map<int, int> *rngs = nullptr);
    static wxJSONValue CreateUniverseFile(Controller* controller, bool input);
#endif
    static std::string GetVendor(const std::string& type);
    static std::string GetModel(const std::string& type);

#pragma region Getters and Setters
#ifndef DISCOVERYONLY
    virtual bool SetInputUniverses(Controller* controller, wxWindow* parent) override;
    virtual bool SetOutputs(ModelManager* allmodels, OutputManager* outputManager, Controller* controller, wxWindow* parent) override;
    virtual bool UploadForImmediateOutput(ModelManager* allmodels, OutputManager* outputManager, Controller* controller, wxWindow* parent) override;
#endif
    virtual bool UsesHTTP() const override { return true; }
#pragma endregion

private:
    FPPUploadProgressDialog *progressDialog = nullptr;
    wxGauge *progress = nullptr;
    
    void DumpJSON(const wxJSONValue& json);

    bool GetURLAsJSON(const std::string& url, wxJSONValue& val, bool recordError = true);
    bool GetURLAsString(const std::string& url, std::string& val, bool recordError = true);

    int PostJSONToURL(const std::string& url, const wxJSONValue& val);
    int PostJSONToURLAsFormData(const std::string& url, const std::string &extra, const wxJSONValue& val);
    int PostToURL(const std::string& url, const std::string &val, const std::string &contentType = "application/octet-stream");
    int PostToURL(const std::string& url, const std::vector<uint8_t> &val, const std::string &contentType = "application/octet-stream");
    int PutToURL(const std::string& url, const std::string &val, const std::string &contentType = "application/octet-stream");
    int PutToURL(const std::string& url, const std::vector<uint8_t> &val, const std::string &contentType = "application/octet-stream");
    int TransferToURL(const std::string& url, const std::vector<uint8_t> &val, const std::string &contentType, bool isPost);

    
    bool uploadOrCopyFile(const std::string &filename,
                          const std::string &file,
                          const std::string &dir);
    bool uploadFile(const std::string &filename,
                    const std::string &file);
    bool uploadFileV7(const std::string &filename,
                      const std::string &file,
                      const std::string &dir);
    bool callMoveFile(const std::string &filename);

    bool parseSysInfo(wxJSONValue& v);
    void parseControllerType(wxJSONValue& v);
    void parseConfig(const std::string& v);
    void parseProxies(wxJSONValue& v);

    bool IsCompatible(const ControllerCaps *rules,
                      std::string &origVend, std::string &origMod, std::string origVar, const std::string &origId,
                      std::string &driver);
    
    class PlaylistEntry {
    public:
        std::string sequence;
        std::string media;
        float duration = 0;
    };
    std::map<std::string, PlaylistEntry> sequences;
    wxJSONValue capeInfo;
    std::string tempFileName;
    std::string baseSeqName;
    FSEQFile *outputFile = nullptr;
    bool outputFileIsOriginal = false;

    CURL *setupCurl(const std::string &url, bool isGet = true, int timeout = 30000);
    std::string curlInputBuffer;
    
    bool restartNeeded = false;

    bool sysInfoLoaded = false;
};

static inline int case_insensitive_match(std::string s1, std::string s2)
{
    //convert s1 and s2 into lower case strings
    transform(s1.begin(), s1.end(), s1.begin(), ::tolower);
    transform(s2.begin(), s2.end(), s2.begin(), ::tolower);
    if (s1.compare(s2) == 0)
        return 1; //The strings are same
    return 0;     //not matched
}

static inline bool sortByName(const FPP* i, const FPP* j)
{
    return i->hostName < j->hostName;
}

static inline bool sortByIP(const FPP* i, const FPP* j)
{
    return i->ipAddress < j->ipAddress;
}
