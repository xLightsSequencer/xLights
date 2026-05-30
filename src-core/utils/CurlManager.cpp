#include <algorithm>
#include <cstdio>
#include <cstring>
#include <filesystem>
#include <list>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

#include <log.h>

#include "CurlManager.h"


#include "xLightsVersion.h"
#include "string_utils.h"

static std::string resolveContentType(const std::string& type)
{
    if (type == "JSON")
        return "application/json; charset=utf-8";
    if (type == "XML")
        return "application/xml";
    if (type == "TEXT XML")
        return "text/xml";
    if (type == "HTML")
        return "text/html";
    if (type == "TEXT")
        return "text/plain";
    return "application/x-www-form-urlencoded";
}


std::string CurlManager::HTTPSPost(const std::string& url, const std::string& body, const std::string& user, const std::string& password, const std::string& contentType, int timeout, const std::vector<std::pair<std::string, std::string>>& customHeaders, int* responseCode)
{
    SyncRequestOptions options;
    options.method = "POST";
    options.body = body;
    options.contentType = resolveContentType(contentType);
    options.username = user;
    options.password = password;
    options.timeoutSeconds = timeout;
    options.disableSSLVerification = true;
    options.responseCode = responseCode;
    for (const auto& it : customHeaders) {
        options.headers.push_back(it.first + ": " + it.second);
    }
    return INSTANCE.doRequest(url, options);
}

std::string CurlManager::HTTPSPost(const std::string& url, const std::vector<Var>& vars, const std::string& user, const std::string& password, int timeout, const std::vector<std::pair<std::string, std::string>>& customHeaders)
{
    auto logger_curl = spdlog::get("curl");
    CURL* curl = curl_easy_init();
    if (curl == nullptr) {
        return "";
    }

    struct curl_httppost* formpost = nullptr;
    struct curl_httppost* lastptr = nullptr;
    for (const auto& it : vars) {
        curl_formadd(&formpost,
                     &lastptr,
                     CURLFORM_COPYNAME, it.key.c_str(),
                     CURLFORM_COPYCONTENTS, it.value.c_str(),
                     CURLFORM_END);
    }

    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    if (!user.empty() || !password.empty()) {
        std::string sAuth = user + ":" + password;
        curl_easy_setopt(curl, CURLOPT_HTTPAUTH, CURLAUTH_BASIC);
        curl_easy_setopt(curl, CURLOPT_USERPWD, sAuth.c_str());
    }

    struct curl_slist* headerlist = nullptr;
    headerlist = curl_slist_append(headerlist, "Expect:");
    for (const auto& it : customHeaders) {
        std::string h = it.first + ": " + it.second;
        headerlist = curl_slist_append(headerlist, h.c_str());
    }
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headerlist);

    curl_easy_setopt(curl, CURLOPT_HTTPPOST, formpost);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, timeout);
    curl_easy_setopt(curl, CURLOPT_ACCEPT_ENCODING, "");
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);

    std::string buffer;
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, +[](void* ptr, size_t size, size_t nmemb, void* userdata) {
        auto* out = static_cast<std::string*>(userdata);
        out->append(static_cast<char*>(ptr), size * nmemb);
        return size * nmemb;
    });
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &buffer);

    CURLcode res = curl_easy_perform(curl);
    curl_easy_cleanup(curl);
    curl_formfree(formpost);
    if (headerlist != nullptr) {
        curl_slist_free_all(headerlist);
    }

    if (res == CURLE_OK) {
        return buffer;
    }
    logger_curl->error("Curl post form failed: {}", static_cast<int>(res));
    return "";
}

std::string CurlManager::HTTPSDelete(const std::string& url, const std::string& body, const std::string& user, const std::string& password, const std::string& contentType, int timeout, const std::vector<std::pair<std::string, std::string>>& customHeaders, int* responseCode)
{
    SyncRequestOptions options;
    options.method = "DELETE";
    options.body = body;
    options.contentType = resolveContentType(contentType);
    options.username = user;
    options.password = password;
    options.timeoutSeconds = timeout;
    options.disableSSLVerification = true;
    options.responseCode = responseCode;
    for (const auto& it : customHeaders) {
        options.headers.push_back(it.first + ": " + it.second);
    }
    return INSTANCE.doRequest(url, options);
}

std::string CurlManager::HTTPSGet(const std::string& s, const std::string& user, const std::string& password, int timeout, const std::vector<std::pair<std::string, std::string>>& customHeaders, int* responseCode)
{
    SyncRequestOptions options;
    options.method = "GET";
    options.username = user;
    options.password = password;
    options.timeoutSeconds = timeout;
    options.disableSSLVerification = true;
    options.responseCode = responseCode;
    for (const auto& it : customHeaders) {
        options.headers.push_back(it.first + ": " + it.second);
    }
    return INSTANCE.doRequest(s, options);
}

bool CurlManager::HTTPSGetFile(const std::string& s, const std::string& filename, const std::string& user, const std::string& password, int timeout, std::function<bool(int)> progress)
{
    return INSTANCE.doGetFile(s, filename, user, password, timeout, progress);
}

bool CurlManager::HTTPUploadFile(const std::string& url, const std::string& filename, const std::string& file, std::function<bool(int, std::string)> progress, const std::string& username, const std::string& password)
{
    return INSTANCE.doUploadFile(url, filename, file, progress, username, password);
}


CurlManager CurlManager::INSTANCE;

CurlManager::CurlManager() {
}

CurlManager::~CurlManager() {
    if (curlMulti) {
        curl_multi_cleanup(curlMulti);
    }
}

void CurlManager::addCURL(const std::string& furl, CURL* curl, std::function<void(CURL*)>&& callback, bool autoCleanCurl) {
    std::unique_lock<std::recursive_mutex> l(lock);
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
    for (size_t x = 0; x < curls.size(); x++) {
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
    size_t numb = dt->req->size() - dt->curPos;
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
    static std::string USERAGENT = "xLights-" + xlights_version_string;

    const std::string host = getHost(fullUrl);
    HostData* hd = getHostData(host);
    CURL* c = curl_easy_init();
    curl_easy_setopt(c, CURLOPT_URL, fullUrl.c_str());
    curl_easy_setopt(c, CURLOPT_USERAGENT, USERAGENT.c_str());
    curl_easy_setopt(c, CURLOPT_CONNECTTIMEOUT_MS, 5000L);
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

#ifdef _WIN32
    if (StartsWith(fullUrl, "https")) {
        // FIXME - curl on Windows is broken... Likely need to update to a newer version or one with  proper
        // options compiled in.  Alternatively, drop support for Windows 8 and use curl provided by Win 10/11
        curl_easy_setopt(c, CURLOPT_SSL_VERIFYPEER, 0L);
        curl_easy_setopt(c, CURLOPT_SSL_VERIFYHOST, 0L);
    }
#endif
    
    return c;
}

void CurlManager::add(const std::string& furl, const std::string& method, const std::string& data,
                      const std::list<std::string>& extraHeaders,
                      std::function<void(int rc, const std::string& resp)>&& callback) {
    auto logger_curl = spdlog::get("curl");
    logger_curl->info("Adding CURL - URL: {}    Method: {}", furl.c_str(), method.c_str());
    
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

        auto logger_curl = spdlog::get("curl");
        logger_curl->info("    CURL Callback - URL: {}    Response Code: {}", furl.c_str(), rc);

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

std::string CurlManager::doRequest(const std::string& furl, const SyncRequestOptions& options)
{
    auto logger_curl = spdlog::get("curl");
    logger_curl->info("Adding Synchronous CURL - URL: {}    Method: {}", furl.c_str(), options.method.c_str());

    CURL* curl = createCurl(furl);
    if (curl == nullptr) {
        return "";
    }

    if (options.timeoutSeconds > 0) {
        curl_easy_setopt(curl, CURLOPT_TIMEOUT, options.timeoutSeconds);
    }

    if (!options.username.empty() || !options.password.empty()) {
        std::string sAuth = options.username + ":" + options.password;
        curl_easy_setopt(curl, CURLOPT_HTTPAUTH, CURLAUTH_BASIC);
        curl_easy_setopt(curl, CURLOPT_USERPWD, sAuth.c_str());
    }

    if (options.disableSSLVerification) {
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
    }

    struct curl_slist* headerlist = nullptr;
    if (!options.contentType.empty()) {
        std::string ct = "Content-Type: " + options.contentType;
        headerlist = curl_slist_append(headerlist, ct.c_str());
    }
    for (const auto& h : options.headers) {
        headerlist = curl_slist_append(headerlist, h.c_str());
    }

    if (options.method == "POST") {
        curl_easy_setopt(curl, CURLOPT_POST, 1L);
    } else if (options.method == "PUT" || options.method == "PATCH" || options.method == "DELETE") {
        curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, options.method.c_str());
    }

    if (!options.body.empty()) {
        headerlist = curl_slist_append(headerlist, "Expect:");
        curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, (long)options.body.size());
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, options.body.c_str());
    }

    if (headerlist != nullptr) {
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headerlist);
    }

    CURLcode res = curl_easy_perform(curl);

    CurlPrivateData* data = nullptr;
    long rc = 0;
    curl_easy_getinfo(curl, CURLINFO_PRIVATE, &data);
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &rc);
    if (options.responseCode != nullptr) {
        *options.responseCode = static_cast<int>(rc);
    }

    std::string resp;
    if (res == CURLE_OK && data != nullptr) {
        resp.assign(reinterpret_cast<char*>(data->resp.data()), data->resp.size());
    } else {
        const char* err = curl_easy_strerror(res);
        logger_curl->error("Curl request failed: {} {}", static_cast<int>(res), (err ? err : ""));
    }

    if (headerlist != nullptr) {
        curl_slist_free_all(headerlist);
    }
    if (data != nullptr) {
        delete data;
    }
    curl_easy_cleanup(curl);

    return resp;
}

struct HTTPGetFileProgressData {
    std::function<bool(int)> progress;
    int lastPosition = -1;
};

static int http_get_file_progress_callback(void* userp,
                                           double total,
                                           double now,
                                           double ultotal,
                                           double ulnow)
{
    HTTPGetFileProgressData* data = static_cast<HTTPGetFileProgressData*>(userp);
    if (data != nullptr && data->progress != nullptr) {
        int pos = 0;
        if (total > 0) {
            pos = static_cast<int>((now * 1000.0) / total);
        }
        if (pos < 0) {
            pos = 0;
        }
        if (pos > 1000) {
            pos = 1000;
        }
        if (pos != data->lastPosition) {
            data->lastPosition = pos;
            if (!data->progress(pos)) {
                return 1;
            }
        }
    }
    return 0;
}

static size_t http_get_file_write_callback(void* ptr, size_t size, size_t nmemb, FILE* stream)
{
    if (ptr == nullptr || stream == nullptr) {
        return 0;
    }
    return fwrite(ptr, size, nmemb, stream);
}

bool CurlManager::doGetFile(const std::string& url,
                            const std::string& filename,
                            const std::string& user,
                            const std::string& password,
                            int timeoutSeconds,
                            std::function<bool(int)> progress)
{
    auto logger_curl = spdlog::get("curl");

    FILE* fp = fopen(filename.c_str(), "wb");
    if (fp == nullptr) {
        logger_curl->error("doGetFile: Failure to create file {}.", filename);
        return false;
    }

    bool res = true;
    CURL* curl = createCurl(url);
    if (curl == nullptr) {
        fclose(fp);
        return false;
    }

    if (!user.empty() || !password.empty()) {
        std::string sAuth = user + ":" + password;
        curl_easy_setopt(curl, CURLOPT_HTTPAUTH, CURLAUTH_BASIC);
        curl_easy_setopt(curl, CURLOPT_USERPWD, sAuth.c_str());
    }

    // createCurl sets CURLOPT_TIMEOUT_MS=12000 for short API calls, but that
    // would abort a large-file download after 12 seconds.  Disable it here and
    // use the connection-phase timeout + low-speed detection instead.
    curl_easy_setopt(curl, CURLOPT_TIMEOUT_MS, 0L);
    if (timeoutSeconds > 0) {
        curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, timeoutSeconds);
    }
    curl_easy_setopt(curl, CURLOPT_LOW_SPEED_LIMIT, 1024L);
    curl_easy_setopt(curl, CURLOPT_LOW_SPEED_TIME, 30L);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(curl, CURLOPT_MAXREDIRS, 50L);
    curl_easy_setopt(curl, CURLOPT_TCP_KEEPALIVE, 1L);
    // Force HTTP/1.1 for file fetches.
    //   - The multi handle in `addCURL` enables CURLPIPE_MULTIPLEX,
    //     which on HTTP/2 coalesces sockets across different
    //     hostnames that resolve to overlapping IPs. Misconfigured
    //     servers return 421 "Misdirected Request" when a request
    //     for hostname A arrives over a connection negotiated for
    //     hostname B (seen with mattosdesigns.com /
    //     ledpixelshow.com in the vendor catalog).
    //   - On iOS, ALPN-negotiated h2 over Secure Transport also
    //     triggers TLS handshake aborts on some servers
    //     (efl-designs.com, buildalightshow.com, twinkle-forge.com).
    //   FRESH_CONNECT + FORBID_REUSE keep each file download on
    //   its own connection, eliminating cross-host coalescing
    //   entirely. Cost is negligible — vendor catalog fetches are
    //   one-shot small XML downloads.
    curl_easy_setopt(curl, CURLOPT_HTTP_VERSION, (long)CURL_HTTP_VERSION_1_1);
    curl_easy_setopt(curl, CURLOPT_FRESH_CONNECT, 1L);
    curl_easy_setopt(curl, CURLOPT_FORBID_REUSE, 1L);

    struct curl_slist* headers = nullptr;
    headers = curl_slist_append(headers, "User-Agent: Mozilla/5.0 (Windows NT 5.1; rv:21.0) Gecko/20130401 Firefox/21.0");
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

    HTTPGetFileProgressData progressData;
    progressData.progress = progress;
    if (progress != nullptr) {
        curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 0L);
        curl_easy_setopt(curl, CURLOPT_PROGRESSDATA, &progressData);
        curl_easy_setopt(curl, CURLOPT_PROGRESSFUNCTION, http_get_file_progress_callback);
    } else {
        curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 1L);
    }

    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, http_get_file_write_callback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp);

    CURLcode r = curl_easy_perform(curl);
    if (r != CURLE_OK) {
        logger_curl->error("Failure to access {} -> {}: {}.", url, filename, curl_easy_strerror(r));
        res = false;
    } else {
        long response_code = 0;
        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response_code);
        if (response_code >= 400) {
            logger_curl->error("Error getting file {} -> {}: {}.", url, filename, response_code);
            res = false;
        }
    }

    if (headers != nullptr) {
        curl_slist_free_all(headers);
    }
    CurlPrivateData* data = nullptr;
    curl_easy_getinfo(curl, CURLINFO_PRIVATE, &data);
    if (data != nullptr) {
        delete data;
    }
    curl_easy_cleanup(curl);

    fclose(fp);
    if (!res) {
        remove(filename.c_str());
    }

    return res;
}

static std::string GetServerFromUrl(const std::string& url)
{
    size_t start = 0;
    size_t scheme = url.find("://");
    if (scheme != std::string::npos) {
        start = scheme + 3;
    }

    size_t end = url.find('/', start);
    std::string host = url.substr(start, end == std::string::npos ? std::string::npos : end - start);
    size_t atPos = host.rfind('@');
    if (atPos != std::string::npos) {
        host = host.substr(atPos + 1);
    }

    if (!host.empty() && host.front() == '[') {
        size_t bracketEnd = host.find(']');
        if (bracketEnd != std::string::npos) {
            return host.substr(1, bracketEnd - 1);
        }
        return host;
    }

    size_t colon = host.find(':');
    if (colon != std::string::npos) {
        host = host.substr(0, colon);
    }
    return host;
}

struct HTTPUploadData {
    const uint8_t* data = nullptr;
    size_t dataSize = 0;
    size_t curPos = 0;
    FILE* file = nullptr;
    size_t fileSize = 0;
    const uint8_t* postData = nullptr;
    size_t postDataSize = 0;
    std::function<bool(int, std::string)> progress = nullptr;
    std::string progressString;
    size_t totalWritten = 0;
    size_t lastDone = 0;
    bool cancelled = false;

    size_t readData(void* ptr, size_t buffer_size)
    {
        if (data != nullptr) {
            size_t remaining = dataSize - curPos;
            if (remaining) {
                size_t copy_this_much = remaining;
                if (copy_this_much > buffer_size) {
                    copy_this_much = buffer_size;
                }
                if (copy_this_much > 8 * 1024 * 1024) {
                    copy_this_much = 8 * 1024 * 1024;
                }
                memcpy(ptr, &data[curPos], copy_this_much);
                curPos += copy_this_much;
                return copy_this_much;
            }
            curPos = 0;
            if (file == nullptr) {
                data = postData;
                dataSize = postDataSize;
            } else {
                data = nullptr;
                dataSize = 0;
            }
        }

        if (file != nullptr) {
            size_t t = fread(ptr, 1, buffer_size, file);
            totalWritten += t;

            if (progress != nullptr) {
                size_t donePct = totalWritten;
                donePct *= 1000;
                if (fileSize != 0) {
                    donePct /= fileSize;
                }
                if (donePct != lastDone) {
                    lastDone = donePct;
                    cancelled = !progress(static_cast<int>(donePct), progressString);
                    std::this_thread::yield();
                }
            }

            if (t == 0 || feof(file)) {
                fclose(file);
                curPos = 0;
                data = postData;
                dataSize = postDataSize;
                file = nullptr;
            }

            if (cancelled) {
                return CURL_READFUNC_ABORT;
            }
            return t;
        }
        return 0;
    }
};

static size_t http_upload_read_callback(void* ptr, size_t size, size_t nmemb, void* userp)
{
    size_t buffer_size = size * nmemb;
    HTTPUploadData* dt = static_cast<HTTPUploadData*>(userp);
    return dt->readData(ptr, buffer_size);
}

bool CurlManager::doUploadFile(const std::string& url,
                               const std::string& filename,
                               const std::string& file,
                               std::function<bool(int, std::string)> progress,
                               const std::string& username,
                               const std::string& password)
{
    auto logger_curl = spdlog::get("curl");
    logger_curl->info("URL: {}", url);

    std::string escapedFilename;
    for (char ch : filename) {
        if (ch == '"') {
            escapedFilename += "\\\"";
        } else {
            escapedFilename += ch;
        }
    }

    const std::string fileBaseName = std::filesystem::path(file).filename().string();
    const std::string serverName = GetServerFromUrl(url);

    bool cancelled = false;
    if (progress != nullptr) {
        cancelled |= !progress(0, "Transferring " + fileBaseName + " to " + serverName);
    }

    CURL* curl = curl_easy_init();
    if (curl == nullptr) {
        return false;
    }

    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, urlWriteData);
    std::vector<uint8_t> responseBuffer;
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &responseBuffer);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT_MS, 5000L);
    curl_easy_setopt(curl, CURLOPT_ACCEPT_ENCODING, "");
    curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1L);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT_MS, 1000 * 15 * 60);

    char error[1024] = { 0 };
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_ERRORBUFFER, &error);
    if (!username.empty()) {
        curl_easy_setopt(curl, CURLOPT_USERNAME, username.c_str());
        curl_easy_setopt(curl, CURLOPT_PASSWORD, password.c_str());
        curl_easy_setopt(curl, CURLOPT_HTTPAUTH, CURLAUTH_BASIC | CURLAUTH_DIGEST | CURLAUTH_NEGOTIATE);
    }

    const std::string bound = "----WebKitFormBoundaryb29a7c2fe47b9481";
    struct curl_slist* chunk = nullptr;
    std::string ctMime = "Content-Type: multipart/form-data; boundary=" + bound;
    chunk = curl_slist_append(chunk, ctMime.c_str());
    chunk = curl_slist_append(chunk, "X-Requested-With: FPPConnect");
    chunk = curl_slist_append(chunk, "User-Agent: Mozilla/5.0 (Macintosh; Intel Mac OS X 10_14_1) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/70.0.3538.77 Safari/537.36");
    chunk = curl_slist_append(chunk, "Expect:");

    curl_easy_setopt(curl, CURLOPT_POST, 1L);

    std::string memBuffPost;
    std::string memBuffPre;
    memBuffPost.append("\r\n--").append(bound).append("--\r\n");

    std::string cd = "Content-Disposition: form-data; name=\"avatar\"; filename=\"";
    cd += file;
    cd += "\"\r\n";
    memBuffPre.append("--").append(bound).append("\r\n").append(cd)
             .append("Content-Type: application/octet-stream")
             .append("\r\n\r\n");

    FILE* fileobj = fopen(escapedFilename.c_str(), "rb");
    if (fileobj == nullptr) {
        logger_curl->error("Could not open upload file {}.", escapedFilename);
        curl_slist_free_all(chunk);
        curl_easy_cleanup(curl);
        return false;
    }
    if (fseek(fileobj, 0, SEEK_END) != 0) {
        fclose(fileobj);
        curl_slist_free_all(chunk);
        curl_easy_cleanup(curl);
        return false;
    }
    long fileLength = ftell(fileobj);
    if (fileLength < 0 || fseek(fileobj, 0, SEEK_SET) != 0) {
        fclose(fileobj);
        curl_slist_free_all(chunk);
        curl_easy_cleanup(curl);
        return false;
    }

    size_t contentLength = static_cast<size_t>(fileLength) + memBuffPre.size() + memBuffPost.size();
    curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, contentLength);
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, chunk);

    HTTPUploadData data;
    data.data = reinterpret_cast<const uint8_t*>(memBuffPre.data());
    data.dataSize = memBuffPre.size();
    data.progress = progress;
    data.file = fileobj;
    data.fileSize = static_cast<size_t>(fileLength);
    data.postData = reinterpret_cast<const uint8_t*>(memBuffPost.data());
    data.postDataSize = memBuffPost.size();
    data.progressString = "Transferring " + fileBaseName + " to " + serverName;
    curl_easy_setopt(curl, CURLOPT_READFUNCTION, http_upload_read_callback);
    curl_easy_setopt(curl, CURLOPT_READDATA, &data);

    CURLcode i = curl_easy_perform(curl);
    curl_slist_free_all(chunk);

    long response_code = 0;
    bool ok = true;
    if (i == CURLE_OK) {
        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response_code);
        if (response_code != 200) {
            ok = false;
        }
    } else {
        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response_code);
        logger_curl->warn("Curl did not upload file: {} {}", response_code, error);
        ok = false;
    }

    if (progress != nullptr) {
        cancelled |= !progress(1000, "");
    }

    if (data.file != nullptr) {
        fclose(data.file);
        data.file = nullptr;
    }

    CurlPrivateData* privateData = nullptr;
    curl_easy_getinfo(curl, CURLINFO_PRIVATE, &privateData);
    if (privateData != nullptr) {
        delete privateData;
    }
    curl_easy_cleanup(curl);

    return ok && !data.cancelled && !cancelled;
}

std::string CurlManager::doGet(const std::string& furl, int& rc) {
    auto logger_curl = spdlog::get("curl");
    CURL* curl = createCurl(furl);

    logger_curl->info("Adding Synchronous CURL - URL: {}    Method: GET", furl.c_str());
    
    bool done = false;
    addCURL(furl, curl, [&done](CURL* c) { done = true; }, false);
    while (!done && processCurls()) {
        CurlManager::INSTANCE.yield();
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
    logger_curl->info("    CURL Synchronous Response - URL: {}    Response Code: {}     Size: {}", furl.c_str(), rc, rc ? data->resp.size() : 0);
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
    auto logger_curl = spdlog::get("curl");
    logger_curl->info("Adding Synchronous CURL - URL: {}    Method: POST", furl.c_str());

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
        CurlManager::INSTANCE.yield();
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
    logger_curl->info("    CURL Synchronous Response - URL: {}    Response Code: {}     Size: {}", furl.c_str(), rc, rc ? cdata->resp.size() : 0);
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
    auto logger_curl = spdlog::get("curl");
    logger_curl->info("Adding Synchronous CURL - URL: {}    Method: PUT", furl.c_str());
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
        CurlManager::INSTANCE.yield();
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
    logger_curl->info("    CURL Synchronous Response - URL: {}    Response Code: {}     Size: {}", furl.c_str(), rc, rc ? cdata->resp.size() : 0);
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
    int stillRunning = 0;
    std::unique_lock<std::recursive_mutex> l(lock);
    curl_multi_perform(curlMulti, &stillRunning);
    if (doCurlCallbacks && stillRunning != numCurls) {
        CURLMsg* m = nullptr;
        do {
            int msgq = 0;
            m = curl_multi_info_read(curlMulti, &msgq);
            if (m && (m->msg == CURLMSG_DONE)) {
                CURL* e = m->easy_handle;
                curl_multi_remove_handle(curlMulti, e);
                CurlInfo* ci = nullptr;
                for (size_t x = 0; x < curls.size(); x++) {
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
    std::unique_lock<std::recursive_mutex> l(lock);
    HostData* h = hostData[host];
    if (h == nullptr) {
        h = new HostData();
        hostData[host] = h;
    }
    return h;
}

void CurlManager::setHostUsernamePassword(const std::string& host, const std::string& username, const std::string password) {
    std::unique_lock<std::recursive_mutex> l(lock);
    HostData* h = getHostData(host);
    h->username = username;
    h->password = password;
}
void CurlManager::setHostAllowHTTP_0_9(const std::string &host, bool v) {
    std::unique_lock<std::recursive_mutex> l(lock);
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
