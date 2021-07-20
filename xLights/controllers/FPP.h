#pragma once

#include <list>
#include <map>
#include <set>
#include <algorithm>

#include "../models/ModelManager.h"
#include "ControllerUploadData.h"
#include "BaseController.h"

class wxJSONValue;
class FSEQFile;
class wxMemoryBuffer;
typedef void CURL;
class wxWindow;
class wxProgressDialog;
class Discovery;

class FPP : public BaseController
{
    public:
    FPP() : BaseController("", ""), majorVersion(0), minorVersion(0), outputFile(nullptr), parent(nullptr), curl(nullptr), isFPP(true) {}
    FPP(const std::string &ip, const std::string &proxy, const std::string &model);
    FPP(const std::string &address);
    FPP(const FPP &c);
    virtual ~FPP();

    void setIPAddress(const std::string &ip);
    
    std::string hostName;
    std::string description;
    std::string ipAddress;
    std::string fullVersion;
    std::string platform;
    std::string model;
    uint32_t majorVersion = 0;
    uint32_t minorVersion = 0;
    std::string ranges;
    std::string mode;
    std::string pixelControllerType;
    std::string panelSize;
    int type = 0;
    
    std::string proxy;
    std::set<std::string> proxies;

    std::string username;
    std::string password;
    bool isFPP =  false;
    bool iszlib = false;
    
    std::string controllerVendor;
    std::string controllerModel;
    std::string controllerVariant;

    wxWindow *parent = nullptr;
    wxProgressDialog *progressDialog = nullptr;
    std::list<std::string> messages;
    int defaultConnectTimeout = 2000;

    std::map<int, int> GetExpansionPorts(ControllerCaps* caps) const;
    bool AuthenticateAndUpdateVersions();
    void LoadPlaylists(std::list<std::string> &playlists);
    void probePixelControllerType();
    
    void UpdateChannelRanges();
    void FillRanges(std::map<int, int> &rngs);
    void SetNewRanges(const std::map<int, int> &rngs);
    bool IsMultiSyncEnabled();
    bool IsDDPInputEnabled();

    bool IsVersionAtLeast(uint32_t maj, uint32_t min);
    bool IsDrive();

#ifndef DISCOVERYONLY
    bool PrepareUploadSequence(const FSEQFile &file,
                               const std::string &seq,
                               const std::string &media,
                               int type);
    bool WillUploadSequence() const;
    bool AddFrameToUpload(uint32_t frame, uint8_t *data);
    bool FinalizeUploadSequence();
    std::string GetTempFile() const { return tempFileName; }
    void ClearTempFile() { tempFileName = ""; }
#endif

    bool UploadUDPOutputsForProxy(OutputManager* outputManager);
    
    bool UploadPlaylist(const std::string &playlist);
    bool UploadModels(const wxJSONValue &models);
    bool UploadDisplayMap(const std::string &displayMap);
    bool UploadUDPOut(const wxJSONValue &udp);

    bool UploadPixelOutputs(ModelManager* allmodels,
                            OutputManager* outputManager,
                            Controller* controller);
    bool SetInputUniversesBridge(Controller* controller);

    bool SetRestartFlag();
    bool Restart(const std::string &mode = "", bool ifNeeded = false);
    void SetDescription(const std::string &st);    
    
    static void PrepareDiscovery(Discovery &discovery);
    static void PrepareDiscovery(Discovery &discovery, const std::list<std::string> &addresses, bool broadcastPing = true);
    static void MapToFPPInstances(Discovery &discovery, std::list<FPP*> &instances, OutputManager* outputManager);
    
    
#ifndef DISCOVERYONLY
    static wxJSONValue CreateModelMemoryMap(ModelManager* allmodels);
    static std::string CreateVirtualDisplayMap(ModelManager* allmodels, bool center0);
    static wxJSONValue CreateUniverseFile(const std::list<Controller*>& controllers, bool input, std::map<int, int> *rngs = nullptr);
    static wxJSONValue CreateUniverseFile(ControllerEthernet* controller, bool input);
#endif
    static std::string GetVendor(const std::string& type);
    static std::string GetModel(const std::string& type);

#pragma region Getters and Setters
#ifndef DISCOVERYONLY
    virtual bool SetInputUniverses(ControllerEthernet* controller, wxWindow* parent) override;
    virtual bool SetOutputs(ModelManager* allmodels, OutputManager* outputManager, ControllerEthernet* controller, wxWindow* parent) override;
    virtual bool UploadForImmediateOutput(ModelManager* allmodels, OutputManager* outputManager, ControllerEthernet* controller, wxWindow* parent) override;
    virtual bool ResetAfterOutput(OutputManager* outputManager, ControllerEthernet* controller, wxWindow* parent) override;
#endif

    virtual bool UsesHTTP() const override { return false; } // returning false here because i dont think you can uypload through a FPP proxy to another FPP
#pragma endregion

private:
    void DumpJSON(const wxJSONValue& json);

    bool GetPathAsJSON(const std::string &path, wxJSONValue &val);
    bool GetURLAsJSON(const std::string& url, wxJSONValue& val, bool recordError = true);
    bool GetURLAsString(const std::string& url, std::string& val, bool recordError = true);

    bool WriteJSONToPath(const std::string& path, const wxJSONValue& val);
    int PostJSONToURL(const std::string& url, const wxJSONValue& val);
    int PostJSONToURLAsFormData(const std::string& url, const std::string &extra, const wxJSONValue& val);
    int PostToURL(const std::string& url, const std::string &val, const std::string &contentType = "application/octet-stream");
    int PostToURL(const std::string& url, const wxMemoryBuffer &val, const std::string &contentType = "application/octet-stream");
    bool uploadOrCopyFile(const std::string &filename,
                          const std::string &file,
                          const std::string &dir);
    bool uploadFile(const std::string &filename,
                    const std::string &file);
    bool copyFile(const std::string &filename,
                  const std::string &file,
                  const std::string &dir);

    bool parseSysInfo(wxJSONValue& v);
    void parseControllerType(wxJSONValue& v);
    void parseConfig(const std::string& v);
    void parseProxies(wxJSONValue& v);


    class PlaylistEntry {
    public:
        std::string sequence;
        std::string media;
        float duration = 0;
    };
    std::map<std::string, PlaylistEntry> sequences;
    std::string tempFileName;
    std::string baseSeqName;
    FSEQFile *outputFile = nullptr;

    void setupCurl();
    CURL *curl = nullptr;
    std::string curlInputBuffer;
    
    bool restartNeeded = false;
    std::string curMode = "";
};
