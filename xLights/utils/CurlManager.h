#pragma once

#include <curl/curl.h>
#include <functional>
#include <list>
#include <map>
#include <mutex>
#include <vector>

class CurlManager {
public:
    static CurlManager INSTANCE;

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

    // Synchronous methods
    std::string doGet(const std::string& furl, int& rc);
    std::string doPost(const std::string& furl, const std::string& contentType, const std::vector<uint8_t>& data, int& rc);
    std::string doPut(const std::string& furl, const std::string& contentType, const std::vector<uint8_t>& data, int& rc);

    void setHostUsernamePassword(const std::string& host, const std::string& username, const std::string password);
    bool processCurls() {
        if (numCurls) {
            return doProcessCurls();
        }
        return false;
    }

private:
    CurlManager();
    ~CurlManager();

    bool doProcessCurls();
    std::string getHost(const std::string& url);

    CURLM* curlMulti = nullptr;
    int numCurls = 0;

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
    std::mutex lock;

    class HostData {
    public:
        HostData() {}
        ~HostData() {}
        std::string username;
        std::string password;
        int authStatus = 0;
    };
    std::map<std::string, HostData*> hostData;
    HostData* getHostData(const std::string& host);
};
