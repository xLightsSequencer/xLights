#include <algorithm>
#include <cstring>
#include <list>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

#include <log4cpp/Category.hh>

#include "CurlManager.h"


#include <wx/string.h>
#include <wx/app.h>
#include "../xLightsVersion.h"
#include "string_utils.h"


CurlManager CurlManager::INSTANCE;

CurlManager::CurlManager() {
}

CurlManager::~CurlManager() {
    if (curlMulti) {
        curl_multi_cleanup(curlMulti);
    }
}

void CurlManager::addCURL(const std::string& furl, CURL* curl, std::function<void(CURL*)>&& callback, bool autoCleanCurl) {
    std::unique_lock<std::mutex> l(lock);
    if (curlMulti == nullptr) {
        curlMulti = curl_multi_init();
        curl_multi_setopt(curlMulti, CURLMOPT_PIPELINING, CURLPIPE_MULTIPLEX);
    }

    CurlInfo* i = new CurlInfo;
    i->host = getHost(furl);
    i->url = furl;
    i->callback = callback;
    i->curl = curl;
    i->cleanCurl = autoCleanCurl;
    curl_multi_add_handle(curlMulti, curl);
    numCurls++;
    for (int x = 0; x < curls.size(); x++) {
        if (curls[x] == nullptr) {
            curls[x] = i;
            return;
        }
    }
    curls.push_back(i);
}

static size_t urlWriteData(void* buffer, size_t size, size_t nmemb, void* userp) {
    std::vector<uint8_t>* str = (std::vector<uint8_t>*)userp;
    size_t pos = str->size();
    str->resize(str->size() + size * nmemb);
    memcpy(&str->data()[pos], buffer, size * nmemb);
    return size * nmemb;
}
static size_t urlReadData(void* ptr, size_t size, size_t nmemb, void* userp) {
    size_t buffer_size = size * nmemb;
    CurlManager::CurlPrivateData* dt = (CurlManager::CurlPrivateData*)userp;
    int numb = dt->req->size() - dt->curPos;
    if (numb > buffer_size) {
        numb = buffer_size;
    }
    memcpy(ptr, &(*dt->req)[dt->curPos], numb);
    dt->curPos += numb;
    return numb;
}
static int urlSeekData(void *userp, curl_off_t offset, int origin) {
    CurlManager::CurlPrivateData* dt = (CurlManager::CurlPrivateData*)userp;
    dt->curPos = offset;
    return CURL_SEEKFUNC_OK;
}
CURL* CurlManager::createCurl(const std::string& fullUrl, CurlPrivateData** cpd, bool upload) {
    static std::string USERAGENT = wxAppConsole::GetInstance()->GetAppName().ToStdString() + "-" + xlights_version_string;

    const std::string host = getHost(fullUrl);
    HostData* hd = getHostData(host);
    CURL* c = curl_easy_init();
    curl_easy_setopt(c, CURLOPT_URL, fullUrl.c_str());
    curl_easy_setopt(c, CURLOPT_USERAGENT, USERAGENT.c_str());
    curl_easy_setopt(c, CURLOPT_CONNECTTIMEOUT_MS, 4000L);
    curl_easy_setopt(c, CURLOPT_TIMEOUT_MS, 12000L);
    curl_easy_setopt(c, CURLOPT_ACCEPT_ENCODING, "");
    curl_easy_setopt(c, CURLOPT_NOSIGNAL, 1L);
    //curl_easy_setopt(c, CURLOPT_VERBOSE, 2L);
    curl_easy_setopt(c, CURLOPT_UPKEEP_INTERVAL_MS, 5000L);

    curl_easy_setopt(c, CURLOPT_WRITEFUNCTION, urlWriteData);
    CurlPrivateData* data = new CurlPrivateData();
    data->host = host;
    if (hd->allowHTTP0_9) {
        curl_easy_setopt(c, CURLOPT_HTTP09_ALLOWED, 1L);
        data->isHTTP_0_9 = true;
    }
    curl_easy_setopt(c, CURLOPT_ERRORBUFFER, data->errorResp);
    curl_easy_setopt(c, CURLOPT_WRITEDATA, &data->resp);
    curl_easy_setopt(c, CURLOPT_PRIVATE, data);
    if (hd->username != "") {
        curl_easy_setopt(c, CURLOPT_USERNAME, hd->username.c_str());
        curl_easy_setopt(c, CURLOPT_PASSWORD, hd->password.c_str());
        curl_easy_setopt(c, CURLOPT_HTTPAUTH, CURLAUTH_BASIC);
    }
    if (upload) {
        data->req = new std::vector<uint8_t>();
        curl_easy_setopt(c, CURLOPT_UPLOAD_BUFFERSIZE, 1024 * 1024);
        curl_easy_setopt(c, CURLOPT_BUFFERSIZE, 1024 * 1024);
        curl_easy_setopt(c, CURLOPT_READDATA, data);
        curl_easy_setopt(c, CURLOPT_READFUNCTION, urlReadData);
        curl_easy_setopt(c, CURLOPT_SEEKDATA, data);
        curl_easy_setopt(c, CURLOPT_SEEKFUNCTION, urlSeekData);
    }
    if (cpd) {
        *cpd = data;
    }
    return c;
}

void CurlManager::add(const std::string& furl, const std::string& method, const std::string& data,
                      const std::list<std::string>& extraHeaders,
                      std::function<void(int rc, const std::string& resp)>&& callback) {
    static log4cpp::Category& logger_curl = log4cpp::Category::getInstance(std::string("log_curl"));
    logger_curl.info("Adding CURL - URL: %s    Method: %s", furl.c_str(), method.c_str());
    
    CURL* curl = createCurl(furl);

    if (method == "POST") {
        curl_easy_setopt(curl, CURLOPT_POST, 1);
    } else if (method == "PUT") {
        curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "PUT");
    } else if (method == "PATCH") {
        curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "PATCH");
    } else if (method == "DELETE") {
        curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "DELETE");
    }

    struct curl_slist* headers = NULL;
    for (auto& h : extraHeaders) {
        headers = curl_slist_append(headers, h.c_str());
    }

    if ((method == "POST") || (method == "PUT")) {
        curl_easy_setopt(curl, CURLOPT_COPYPOSTFIELDS, data.c_str());
        // turn off the Expect continue thing
        headers = curl_slist_append(headers, "Expect:");
    } else if (method == "PATCH") {
        headers = curl_slist_append(headers, "Expect:");
    }

    if (headers) {
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    }

    addCURL(furl, curl, [furl, callback, headers](CURL* c) {
        CurlPrivateData* data = nullptr;
        long rc = 0;
        curl_easy_getinfo(c, CURLINFO_PRIVATE, &data);
        curl_easy_getinfo(c, CURLINFO_RESPONSE_CODE, &rc);

        static log4cpp::Category& logger_curl = log4cpp::Category::getInstance(std::string("log_curl"));
        logger_curl.info("    CURL Callback - URL: %s    Response Code: %d", furl.c_str(), rc);

        char* urlp = nullptr;
        curl_easy_getinfo(c, CURLINFO_EFFECTIVE_URL, &urlp);
        if (headers) {
            curl_slist_free_all(headers);
        }
        std::string resp(reinterpret_cast<char*>(data->resp.data()), data->resp.size());
        callback(rc, resp);
    });
}
void CurlManager::addGet(const std::string& url, std::function<void(int rc, const std::string& resp)>&& callback) {
    add(url, "GET", "", {}, [callback](int rc, const std::string& resp) { callback(rc, resp); });
}

void CurlManager::addPost(const std::string& url, const std::string& data, const std::string& contentType, std::function<void(int rc, const std::string& resp)>&& callback) {
    add(url, "POST", data, { "Content-Type: " + contentType }, [callback](int rc, const std::string& resp) { callback(rc, resp); });
}
void CurlManager::addPut(const std::string& url, const std::string& data, const std::string& contentType, std::function<void(int rc, const std::string& resp)>&& callback) {
    add(url, "PUT", data, { "Content-Type: " + contentType }, [callback](int rc, const std::string& resp) { callback(rc, resp); });
}

std::string CurlManager::doGet(const std::string& furl, int& rc) {
    static log4cpp::Category& logger_curl = log4cpp::Category::getInstance(std::string("log_curl"));
    CURL* curl = createCurl(furl);

    logger_curl.info("Adding Synchronous CURL - URL: %s    Method: GET", furl.c_str());
    
    bool done = false;
    addCURL(furl, curl, [&done](CURL* c) { done = true; }, false);
    while (!done && processCurls()) {
        wxYieldIfNeeded();
    }

    CurlPrivateData* data = nullptr;
    long rc2 = 0;
    curl_easy_getinfo(curl, CURLINFO_PRIVATE, &data);
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &rc2);
    rc = rc2;
    std::string resp;
    if (rc || data->isHTTP_0_9) {
        resp.assign(reinterpret_cast<char*>(data->resp.data()), data->resp.size());
    } else {
        resp = data->errorResp;
    }
    logger_curl.info("    CURL Synchronous Response - URL: %s    Response Code: %d     Size: %d", furl.c_str(), rc, rc ? data->resp.size() : 0);
    delete data;
    curl_easy_cleanup(curl);

    return resp;
}

struct ReadDataInfo {
    const std::vector<uint8_t>* data;
    size_t curPos = 0;
};
static size_t read_callback(void* ptr, size_t size, size_t nmemb, void* userp) {
    size_t buffer_size = size * nmemb;
    struct ReadDataInfo* dt = (struct ReadDataInfo*)userp;
    size_t numb = dt->data->size() - dt->curPos;
    if (numb > buffer_size) {
        numb = buffer_size;
    }
    if (numb > 0) {
        memcpy(ptr, &(*dt->data)[dt->curPos], numb);
    }
    dt->curPos += numb;
    return numb;
}

static int seek_callback(void *userp, curl_off_t offset, int origin) {
    struct ReadDataInfo* dt = (struct ReadDataInfo*)userp;
    dt->curPos = offset;
    return CURL_SEEKFUNC_OK;
}
std::string CurlManager::doPost(const std::string& furl, const std::string& contentType, const std::vector<uint8_t>& data, int& rc) {
    static log4cpp::Category& logger_curl = log4cpp::Category::getInstance(std::string("log_curl"));
    logger_curl.info("Adding Synchronous CURL - URL: %s    Method: POST", furl.c_str());

    CURL* curl = createCurl(furl);

    struct curl_slist* head = nullptr;
    std::string ct = "Content-Type: " + contentType;
    head = curl_slist_append(head, ct.c_str());
    std::string cl = "Content-Length: " + std::to_string(data.size());
    head = curl_slist_append(head, cl.c_str());
    head = curl_slist_append(head, "Expect:");

    curl_easy_setopt(curl, CURLOPT_POST, 1L);
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, head);

    ReadDataInfo dta;
    dta.data = &data;
    dta.curPos = 0;

    curl_easy_setopt(curl, CURLOPT_READDATA, &dta);
    curl_easy_setopt(curl, CURLOPT_READFUNCTION, read_callback);
    curl_easy_setopt(curl, CURLOPT_SEEKDATA, &dta);
    curl_easy_setopt(curl, CURLOPT_SEEKFUNCTION, seek_callback);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, (long)data.size());

    bool done = false;
    addCURL(furl, curl, [&done](CURL* c) { done = true; }, false);
    while (!done && processCurls()) {
        wxYieldIfNeeded();
    }

    CurlPrivateData* cdata = nullptr;
    long rc2 = 0;
    curl_easy_getinfo(curl, CURLINFO_PRIVATE, &cdata);
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &rc2);
    rc = rc2;
    std::string resp;
    if (rc || cdata->isHTTP_0_9) {
        resp.assign(reinterpret_cast<char*>(cdata->resp.data()), cdata->resp.size());
    } else {
        resp = cdata->errorResp;
    }
    logger_curl.info("    CURL Synchronous Response - URL: %s    Response Code: %d     Size: %d", furl.c_str(), rc, rc ? cdata->resp.size() : 0);
    delete cdata;
    curl_slist_free_all(head);
    curl_easy_cleanup(curl);
    

    return resp;
}
std::string CurlManager::doPost(const std::string& furl, const std::string& contentType, const std::string &data, int& rc) {
    std::vector<uint8_t> vdata;
    vdata.resize(data.size());
    memcpy((void*)&vdata[0], (void*)data.c_str(), data.size());
    return doPost(furl, contentType, vdata,  rc);
}

std::string CurlManager::doPut(const std::string& furl, const std::string& contentType, const std::vector<uint8_t>& data, int& rc) {
    static log4cpp::Category& logger_curl = log4cpp::Category::getInstance(std::string("log_curl"));
    logger_curl.info("Adding Synchronous CURL - URL: %s    Method: PUT", furl.c_str());
    CURL* curl = createCurl(furl);

    struct curl_slist* head = nullptr;
    std::string ct = "Content-Type: " + contentType;
    head = curl_slist_append(head, ct.c_str());
    std::string cl = "Content-Length: " + std::to_string(data.size());
    head = curl_slist_append(head, cl.c_str());
    head = curl_slist_append(head, "Expect:");

    curl_easy_setopt(curl, CURLOPT_PUT, 1L);
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, head);

    ReadDataInfo dta;
    dta.data = &data;
    dta.curPos = 0;

    curl_easy_setopt(curl, CURLOPT_READDATA, &dta);
    curl_easy_setopt(curl, CURLOPT_READFUNCTION, read_callback);
    curl_easy_setopt(curl, CURLOPT_SEEKDATA, &dta);
    curl_easy_setopt(curl, CURLOPT_SEEKFUNCTION, seek_callback);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, (long)data.size());

    addCURL(furl, curl, [](CURL* c) {}, false);
    while (processCurls()) {
        wxYieldIfNeeded();
    }

    CurlPrivateData* cdata = nullptr;
    long rc2 = 0;
    curl_easy_getinfo(curl, CURLINFO_PRIVATE, &cdata);
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &rc2);
    rc = rc2;
    std::string resp;
    if (rc || cdata->isHTTP_0_9) {
        resp.assign(reinterpret_cast<char*>(cdata->resp.data()), cdata->resp.size());
    } else {
        resp = cdata->errorResp;
    }
    logger_curl.info("    CURL Synchronous Response - URL: %s    Response Code: %d     Size: %d", furl.c_str(), rc, rc ? cdata->resp.size() : 0);
    delete cdata;
    curl_slist_free_all(head);
    curl_easy_cleanup(curl);
    return resp;
}
std::string CurlManager::doPut(const std::string& furl, const std::string& contentType, const std::string &data, int& rc) {
    std::vector<uint8_t> vdata;
    vdata.resize(data.size());
    memcpy((void*)&vdata[0], (void*)data.c_str(), data.size());
    return doPut(furl, contentType, vdata,  rc);
}

bool CurlManager::doProcessCurls() {
    CURLMcode mc;
    int stillRunning = 0;
    std::unique_lock<std::mutex> l(lock);
    mc = curl_multi_perform(curlMulti, &stillRunning);
    if (stillRunning != numCurls) {
        CURLMsg* m = nullptr;
        do {
            int msgq = 0;
            m = curl_multi_info_read(curlMulti, &msgq);
            if (m && (m->msg == CURLMSG_DONE)) {
                CURL* e = m->easy_handle;
                curl_multi_remove_handle(curlMulti, e);
                CurlInfo* ci = nullptr;
                for (int x = 0; x < curls.size(); x++) {
                    if (curls[x] && curls[x]->curl == e) {
                        ci = curls[x];
                        curls[x] = nullptr;
                        --numCurls;
                        break;
                    }
                }
                if (ci) {
                    l.unlock();
                    ci->callback(e);
                    l.lock();
                    if (ci->cleanCurl) {
                        CurlPrivateData* data = nullptr;
                        curl_easy_getinfo(e, CURLINFO_PRIVATE, &data);
                        if (data) {
                            delete data;
                        }
                        curl_easy_cleanup(e);
                    }
                    delete ci;
                }
            }
        } while (m);
    }
    return numCurls > 0;
}

CurlManager::HostData* CurlManager::getHostData(const std::string& host) {
    HostData* h = hostData[host];
    if (h == nullptr) {
        h = new HostData();
        hostData[host] = h;
    }
    return h;
}

void CurlManager::setHostUsernamePassword(const std::string& host, const std::string& username, const std::string password) {
    std::unique_lock<std::mutex> l(lock);
    HostData* h = getHostData(host);
    h->username = username;
    h->password = password;
}
void CurlManager::setHostAllowHTTP_0_9(const std::string &host, bool v) {
    std::unique_lock<std::mutex> l(lock);
    HostData* h = getHostData(host);
    h->allowHTTP0_9 = v;
}

std::string CurlManager::getHost(const std::string& url) {
    int idx = url.find("://");
    std::string host = url.substr(idx + 3);
    idx = host.find("/");
    host = host.substr(0, idx);
    return host;
}
