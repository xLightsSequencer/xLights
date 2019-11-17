#ifndef FPP_H
#define FPP_H

#include <list>
#include <map>
#include <set>
#include <algorithm>

#include "../models/ModelManager.h"
#include "ControllerUploadData.h"

class OutputManager;
class wxJSONValue;
class FSEQFile;
class wxMemoryBuffer;
typedef void CURL;
class wxWindow;
class wxProgressDialog;

class PixelCapeInfo : public ControllerRules {
public:
    PixelCapeInfo(const std::string &i, const std::string &d, int s, int dmx) : PixelCapeInfo(i, d, s, dmx, {}) {}
    PixelCapeInfo(const std::string &d, int s, int dmx) : PixelCapeInfo(d, d, s, dmx, {}) {}
    PixelCapeInfo(const std::string &i, const std::string &d, int s, int dmx, const std::map<int, int> &expansions)
        : ControllerRules(), id(i), description(d), maxStrings(s), maxDMX(dmx), expansionPorts(expansions) {}

    PixelCapeInfo() : ControllerRules(), maxStrings(0), maxDMX(0) {}
    PixelCapeInfo(const PixelCapeInfo&pci) : ControllerRules(), description(pci.description), id(pci.id), maxStrings(pci.maxStrings), maxDMX(pci.maxDMX), expansionPorts(pci.expansionPorts) {}
    
    const std::string id;
    const std::string description;
    const int maxStrings;
    const int maxDMX;
    const std::map<int, int> expansionPorts;

    virtual const std::string GetControllerId() const override {
        return id;
    }
    virtual const std::string GetControllerDescription() const override {
        return "FPP " + description;
    }
    virtual const std::string GetControllerManufacturer() const override { return "FPP"; };
    virtual bool SupportsLEDPanelMatrix() const override {
        return maxStrings == 0 && maxDMX == 0;
    }

    virtual int GetMaxPixelPortChannels() const override {
        return 1400 * 3;
    }
    virtual int GetMaxPixelPort() const override {
        return maxStrings;
    }
    virtual int GetMaxSerialPortChannels() const override {
        return 4096;
    }
    virtual int GetMaxSerialPort() const override {
        return maxDMX;
    }
    virtual bool SupportsVirtualStrings() const override { return true; }
    virtual bool MergeConsecutiveVirtualStrings() const override { return false; }
    virtual bool IsValidPixelProtocol(const std::string protocol) const override {
        std::string p(protocol);
        std::transform(p.begin(), p.end(), p.begin(), ::tolower);
        if (p == "ws2811") return true;
        return false;
    }
    virtual bool IsValidSerialProtocol(const std::string protocol) const override {
        std::string p(protocol);
        std::transform(p.begin(), p.end(), p.begin(), ::tolower);
        if (p == "dmx") return true;
        if (p == "pixelnet") return true;
        if (p == "renard") return false;
        return false;
    }
    virtual bool SupportsMultipleProtocols() const override {
        return false;
    }
    virtual bool SupportsSmartRemotes() const override {
        return true;
    }
    virtual bool SupportsMultipleInputProtocols() const override {
        return true;

    }
    virtual bool AllUniversesSameSize() const override {
        return false;
    }
    virtual std::set<std::string> GetSupportedInputProtocols() const override {
        std::set<std::string> res = {"E131", "ARTNET", "DDP"};
        return res;
    }
    virtual bool UniversesMustBeSequential() const override {
        return false;
    }
};

class FPP {
    public:
    FPP() : majorVersion(0), minorVersion(0), outputFile(nullptr), parent(nullptr), curl(nullptr), isFPP(true) {}
    FPP(const std::string &address);
    FPP(const FPP &c);
    virtual ~FPP();

    static PixelCapeInfo& GetCapeRules(const std::string& type);
    static void RegisterCapes();
    
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

    bool AuthenticateAndUpdateVersions();
    void LoadPlaylists(std::list<std::string> &playlists);
    void probePixelControllerType();

    bool IsVersionAtLeast(uint32_t maj, uint32_t min);
    bool IsDrive();

    const std::string &PixelContollerDescription() const;

    bool PrepareUploadSequence(const FSEQFile &file,
                               const std::string &seq,
                               const std::string &media,
                               int type);
    bool AddFrameToUpload(uint32_t frame, uint8_t *data);
    bool FinalizeUploadSequence();


    bool UploadUDPOutputsForProxy(OutputManager* outputManager);
    
    bool UploadPlaylist(const std::string &playlist);
    bool UploadModels(const std::string &models);
    bool UploadDisplayMap(const std::string &displayMap);
    bool UploadUDPOut(const wxJSONValue &udp);

    bool UploadPixelOutputs(ModelManager* allmodels,
                            OutputManager* outputManager,
                            const std::list<int>& selected = std::list<int>());
    bool SetInputUniversesBridge(std::list<int>& selected, OutputManager* outputManager);

    bool SetRestartFlag();
    void SetDescription(const std::string &st);

    static void Discover(const std::list<std::string> &forcedAddresses, std::list<FPP*> &instances, bool doBroadcast = true, bool allPlatforms = false);
    static void Probe(const std::list<std::string> &addresses, std::list<FPP*> &instances);

    static std::string CreateModelMemoryMap(ModelManager* allmodels);
    static std::string CreateVirtualDisplayMap(ModelManager* allmodels, bool center0);
    static wxJSONValue CreateOutputUniverseFile(OutputManager* outputManager);
private:
    void FillRanges(std::map<int, int> &rngs);
    void SetNewRanges(const std::map<int, int> &rngs);
    static wxJSONValue CreateUniverseFile(OutputManager* outputManager, const std::string &onlyip, const std::list<int>& selected, bool input);

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
};

#endif
