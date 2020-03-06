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

class FPP : public BaseController
{
    public:
    FPP() : BaseController("", ""), majorVersion(0), minorVersion(0), outputFile(nullptr), parent(nullptr), curl(nullptr), isFPP(true) {}
    FPP(const std::string &ip, const std::string &proxy, const std::string &model);
    FPP(const std::string &address);
    FPP(const FPP &c);
    virtual ~FPP();

    std::string hostName;
    std::string description;
    std::string ipAddress;
    std::string fullVersion;
    std::string platform;
    std::string model;
    uint32_t majorVersion;
    uint32_t minorVersion;
    std::string ranges;
    std::string mode;
    std::string pixelControllerType;
    std::string panelSize;
    
    std::string proxy;
    std::set<std::string> proxies;

    std::string username;
    std::string password;
    bool isFPP;

    wxWindow *parent;
    wxProgressDialog *progressDialog = nullptr;

    std::map<int, int> GetExpansionPorts(ControllerCaps* caps) const;
    bool AuthenticateAndUpdateVersions();
    void LoadPlaylists(std::list<std::string> &playlists);
    void probePixelControllerType();

    bool IsVersionAtLeast(uint32_t maj, uint32_t min);
    bool IsDrive();

    bool PrepareUploadSequence(const FSEQFile &file,
                               const std::string &seq,
                               const std::string &media,
                               int type);
    bool AddFrameToUpload(uint32_t frame, uint8_t *data);
    bool FinalizeUploadSequence();


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
    bool Restart(const std::string &mode = "");
    void SetDescription(const std::string &st);

    static void Discover(const std::list<std::string> &forcedAddresses, std::list<FPP*> &instances, bool doBroadcast = true, bool allPlatforms = false);
    static void Probe(const std::list<std::string> &addresses, std::list<FPP*> &instances);

    static wxJSONValue CreateModelMemoryMap(ModelManager* allmodels);
    static std::string CreateVirtualDisplayMap(ModelManager* allmodels, bool center0);
    static wxJSONValue CreateUniverseFile(const std::list<ControllerEthernet*>& controllers, bool input);
    static wxJSONValue CreateUniverseFile(ControllerEthernet* controller, bool input);
    static std::string GetVendor(const std::string& type);
    static std::string GetModel(const std::string& type);

#pragma region Getters and Setters
    virtual bool SetInputUniverses(ControllerEthernet* controller, wxWindow* parent) override;
    virtual bool SetOutputs(ModelManager* allmodels, OutputManager* outputManager, ControllerEthernet* controller, wxWindow* parent) override;
    virtual bool UploadForImmediateOutput(ModelManager* allmodels, OutputManager* outputManager, ControllerEthernet* controller, wxWindow* parent) override;

    virtual bool UsesHTTP() const override { return false; } // returning false here because i dont think you can uypload through a FPP proxy to another FPP
#pragma endregion

private:
    void FillRanges(std::map<int, int> &rngs);
    void SetNewRanges(const std::map<int, int> &rngs);
    void DumpJSON(const wxJSONValue& json);

    bool GetPathAsJSON(const std::string &path, wxJSONValue &val);
    bool GetURLAsJSON(const std::string& url, wxJSONValue& val);
    bool GetURLAsString(const std::string& url, std::string& val);

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


    std::map<std::string, std::string> sequences;
    std::string tempFileName;
    std::string baseSeqName;
    FSEQFile *outputFile = nullptr;

    void setupCurl();
    CURL *curl = nullptr;
    std::string curlInputBuffer;
    
    bool restartNeeded = false;
    std::string curMode = "";
};
