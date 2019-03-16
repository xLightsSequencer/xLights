#ifndef FPP_H
#define FPP_H

#include <list>
#include <map>
#include <curl/curl.h>
#include "models/ModelManager.h"

class OutputManager;
class wxJSONValue;
class FSEQFile;
class wxMemoryBuffer;

class FPP {
    public:
    FPP() : majorVersion(0), minorVersion(0), outputFile(nullptr), parent(nullptr), curl(nullptr) {}
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
    
    std::string username;
    std::string password;
    
    wxWindow *parent;
    
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
    
    bool UploadPlaylist(const std::string &playlist);
    bool UploadModels(const std::string &models);
    bool UploadUDPOut(const wxJSONValue &udp);

    bool UploadPixelOutputs(ModelManager* allmodels,
                            OutputManager* outputManager,
                            const std::list<int>& selected = std::list<int>());
    bool SetInputUniversesBridge(std::list<int>& selected, OutputManager* outputManager);
    
    bool SetRestartFlag();
    
    static void Discover(const std::list<std::string> &forcedAddresses, std::list<FPP*> &instances, bool doBroadcast = true);
    static void Probe(const std::list<std::string> &addresses, std::list<FPP*> &instances);
    
    static std::string CreateModelMemoryMap(ModelManager* allmodels);
    static wxJSONValue CreateOutputUniverseFile(OutputManager* outputManager);
private:
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
                          bool compress,
                          const std::string &dir);
    bool uploadFile(const std::string &filename,
                    const std::string &file,
                    bool compress);
    bool copyFile(const std::string &filename,
                  const std::string &file,
                  const std::string &dir);
    
    bool parseSysInfo(wxJSONValue& v);
    void parseControllerType(wxJSONValue& v);

    
    std::map<std::string, std::string> sequences;
    std::string tempFileName;
    std::string baseSeqName;
    bool uploadCompressed;
    FSEQFile *outputFile;

    void setupCurl();
    CURL *curl;
    std::string curlInputBuffer;
};

#endif
