#pragma once

#include <curl/curl.h>
#include <functional>
#include <list>
#include <map>
#include <mutex>
#include <thread>
#include <vector>

class CurlManager {
public:
    static CurlManager INSTANCE;

    struct Var {
        std::string key;
        std::string value;
        Var(const std::string& k, const std::string& v)
            : key(k)
            , value(v)
        {
        }
    };

    // Compatibility static API replacing the old Curl facade.
    static std::string HTTPSPost(const std::string& url, const std::string& body, const std::string& user = "", const std::string& password = "", const std::string& contentType = "", int timeout = 10, const std::vector<std::pair<std::string, std::string>>& customHeaders = {}, int* responseCode = nullptr);
    static std::string HTTPSPost(const std::string& url, const std::vector<Var>& vars, const std::string& user = "", const std::string& password = "", int timeout = 10, const std::vector<std::pair<std::string, std::string>>& customHeaders = {});
    static std::string HTTPSDelete(const std::string& url, const std::string& body, const std::string& user = "", const std::string& password = "", const std::string& contentType = "", int timeout = 10, const std::vector<std::pair<std::string, std::string>>& customHeaders = {}, int* responseCode = nullptr);
    static std::string HTTPSGet(const std::string& s, const std::string& user = "", const std::string& password = "", int timeout = 10, const std::vector<std::pair<std::string, std::string>>& customHeaders = {}, int* responseCode = nullptr);
    static bool HTTPSGetFile(const std::string& s, const std::string& filename, const std::string& user = "", const std::string& password = "", int timeout = 10, std::function<bool(int)> progress = nullptr);
    static bool HTTPUploadFile(const std::string& url, const std::string& filename, const std::string& file, std::function<bool(int, std::string)> progress = nullptr, const std::string& username = "", const std::string& password = "");

    // Callback invoked during synchronous curl operations to keep the
    // caller responsive (e.g. pump the UI event loop).  The UI layer
    // should set this to something like wxYieldIfNeeded(); non-UI
    // callers can leave the default which just yields the CPU.
    using YieldFunction = std::function<void()>;
    void setYieldFunction(YieldFunction fn) { yieldFn = std::move(fn); }
    void yield() { if (yieldFn) yieldFn(); }

    class CurlPrivateData {
    public:
        CurlPrivateData() {}
        virtual ~CurlPrivateData() {
            if (req) {
                delete req;
            }
        }

        std::string host;
        std::vector<uint8_t> resp;
        char errorResp[CURL_ERROR_SIZE] = { 0 };

        std::vector<uint8_t>* req = nullptr;
        size_t curPos = 0;
        bool isHTTP_0_9 = false;
    };

    // Raw methods for working directly with the CURL* objects
    CURL* createCurl(const std::string& fullUrl, CurlPrivateData** data = nullptr, bool upload = false);
    void addCURL(const std::string& furl, CURL* curl, std::function<void(CURL*)>&& callback, bool autoCleanCurl = true);

    // Asynchronous methods for string content
    void add(const std::string& furl, const std::string& method, const std::string& data,
             const std::list<std::string>& extraHeaders,
             std::function<void(int rc, const std::string& resp)>&& callback);
    void addGet(const std::string& furl, std::function<void(int rc, const std::string& resp)>&& callback);
    void addPost(const std::string& furl, const std::string& data, const std::string& contentType, std::function<void(int rc, const std::string& resp)>&& callback);
    void addPut(const std::string& furl, const std::string& data, const std::string& contentType, std::function<void(int rc, const std::string& resp)>&& callback);

    class SyncRequestOptions {
    public:
        std::string method = "GET";
        std::string body;
        std::string contentType;
        std::list<std::string> headers;
        std::string username;
        std::string password;
        int timeoutSeconds = 10;
        bool disableSSLVerification = false;
        int* responseCode = nullptr;
    };

    // Synchronous methods
    std::string doRequest(const std::string& furl, const SyncRequestOptions& options);
    bool doGetFile(const std::string& url,
                   const std::string& filename,
                   const std::string& user = "",
                   const std::string& password = "",
                   int timeoutSeconds = 10,
                   std::function<bool(int)> progress = nullptr);
    bool doUploadFile(const std::string& url,
                      const std::string& filename,
                      const std::string& file,
                      std::function<bool(int, std::string)> progress = nullptr,
                      const std::string& username = "",
                      const std::string& password = "");
    std::string doGet(const std::string& furl, int& rc);
    std::string doPost(const std::string& furl, const std::string& contentType, const std::vector<uint8_t>& data, int& rc);
    std::string doPut(const std::string& furl, const std::string& contentType, const std::vector<uint8_t>& data, int& rc);
    std::string doPost(const std::string& furl, const std::string& contentType, const std::string &data, int& rc);
    std::string doPut(const std::string& furl, const std::string& contentType, const std::string &data, int& rc);

    void setHostUsernamePassword(const std::string& host, const std::string& username, const std::string password);
    void setHostAllowHTTP_0_9(const std::string &host, bool v = true);
    bool processCurls() {
        if (numCurls) {
            return doProcessCurls();
        }
        return false;
    }

    void setProcessCallbacks(bool b) {
        doCurlCallbacks = b;
    }
private:
    CurlManager();
    ~CurlManager();

    bool doProcessCurls();
    std::string getHost(const std::string& url);

    CURLM* curlMulti = nullptr;
    int numCurls = 0;
    bool doCurlCallbacks = true;

    class CurlInfo {
    public:
        CurlInfo() {}
        ~CurlInfo() {}
        std::string host;
        std::string url;
        CURL* curl = nullptr;
        bool cleanCurl = true;
        std::function<void(CURL*)> callback;
    };
    std::vector<CurlInfo*> curls;
    std::recursive_mutex lock;

    class HostData {
    public:
        HostData() {}
        ~HostData() {}
        std::string username;
        std::string password;
        int authStatus = 0;
        bool allowHTTP0_9 = false;
    };
    std::map<std::string, HostData*> hostData;
    HostData* getHostData(const std::string& host);

    YieldFunction yieldFn = [] { std::this_thread::yield(); };
};
