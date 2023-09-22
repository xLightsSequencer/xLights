/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/smeighan/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/smeighan/xLights/blob/master/License.txt
 **************************************************************/

#include "../../include/curl/curl.h"

#include <wx/wx.h>
#include <wx/progdlg.h>
#include <wx/file.h>
#include <wx/filename.h>
#include <wx/url.h>

#include <string>
#include <algorithm>

#include <log4cpp/Category.hh>

#include "Curl.h"

static std::string DecodeResponseCode(int rc)
{
    switch (rc) {
    case 100:
        return "Continue";
    case 101:
        return "Switching Protocols";
    case 102:
        return "Processing(WebDAV)";
    case 103:
        return "Early Hints";
    case 200:
        return "OK";
    case 201:
        return "Created";
    case 202:
        return "Accepted";
    case 203:
        return "Non - Authoritative Information";
    case 204:
        return "No Content";
    case 205:
        return "Reset Content";
    case 206:
        return "Partial Content";
    case 207:
        return "Multi - Status(WebDAV)";
    case 208:
        return "Already Reported(WebDAV)";
    case 226:
        return "IM Used(HTTP Delta encoding)";
    case 300:
        return "Multiple Choices";
    case 301:
        return "Moved Permanently";
    case 302:
        return "Found";
    case 303:
        return "See Other";
    case 304:
        return "Not Modified";
    case 305:
        return "Use Proxy Deprecated";
    case 306:
        return "unused";
    case 307:
        return "Temporary Redirect";
    case 308:
        return "Permanent Redirect";
    case 400:
        return "Bad Request";
    case 401:
        return "Unauthorized";
    case 402:
        return "Payment Required Experimental";
    case 403:
        return "Forbidden";
    case 404:
        return "Not Found";
    case 405:
        return "Method Not Allowed";
    case 406:
        return "Not Acceptable";
    case 407:
        return "Proxy Authentication Required";
    case 408:
        return "Request Timeout";
    case 409:
        return "Conflict";
    case 410:
        return "Gone";
    case 411:
        return "Length Required";
    case 412:
        return "Precondition Failed";
    case 413:
        return "Payload Too Large";
    case 414:
        return "URI Too Long";
    case 415:
        return "Unsupported Media Type";
    case 416:
        return "Range Not Satisfiable";
    case 417:
        return "Expectation Failed";
    case 418:
        return "I'm a teapot";
    case 421:
        return "Misdirected Request";
    case 422:
        return "Unprocessable Entity(WebDAV)";
    case 423:
        return "Locked(WebDAV)";
    case 424:
        return "Failed Dependency(WebDAV)";
    case 425:
        return "Too Early Experimental";
    case 426:
        return "Upgrade Required";
    case 428:
        return "Precondition Required";
    case 429:
        return "Too Many Requests";
    case 431:
        return "Request Header Fields Too Large";
    case 451:
        return "Unavailable For Legal Reasons";
    case 500:
        return "Internal Server Error";
    case 501:
        return "Not Implemented";
    case 502:
        return "Bad Gateway";
    case 503:
        return "Service Unavailable";
    case 504:
        return "Gateway Timeout";
    case 505:
        return "HTTP Version Not Supported";
    case 506:
        return "Variant Also Negotiates";
    case 507:
        return "Insufficient Storage(WebDAV)";
    case 508:
        return "Loop Detected(WebDAV)";
    case 510:
        return "Not Extended";
    case 511:
        return "Network Authentication Required";
    default:
        break;
    }
    return "Unknown repsonse code";
}

static size_t writeFunction(void* ptr, size_t size, size_t nmemb, std::string* data)
{
    if (data != nullptr) {
        data->append((char*)ptr, size * nmemb);
    }
    return size * nmemb;
}

static size_t writeFileFunction(void* ptr, size_t size, size_t nmemb, FILE* stream)
{
    if (ptr == nullptr || stream == nullptr)
        return 0;

    size_t written = fwrite(ptr, size, nmemb, stream);
    return written;
}

static int progressFunction(void* bar,
                            double t, /* dltotal */
                            double d, /* dlnow */
                            double ultotal,
                            double ulnow)
{
    wxProgressDialog* dlg = (wxProgressDialog*)bar;

    if (dlg != nullptr) {
        static double pos = 0;
        if (t > 0) {
            pos = d * dlg->GetRange() / t;
        } else {
            pos += 10;
            if (pos > dlg->GetRange())
                pos = 0;
        }

        // if the client data has been set to 1 then we must not hit the top range
        void *cd = dlg->GetClientData();
        size_t cdi = (size_t)cd;
        if (cdi == 1 && pos == dlg->GetRange())
        {
            pos = dlg->GetRange() - 1;
        }

        dlg->Update(pos);
    }

    return 0;
}
#ifdef _DEBUG
static size_t headerFunction(char* buffer, size_t size, size_t nitems, void* userdata)
{
    static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    if (buffer != nullptr) {
        logger_base.debug(buffer);
    }
    return size * nitems;
}
#endif

std::string Curl::HTTPSPost(const std::string& url, const wxString& body, const std::string& user, const std::string& password, const std::string& contentType, int timeout, const std::vector<std::pair<std::string, std::string>>& customHeaders, int* responseCode)
{
    static log4cpp::Category& logger_curl = log4cpp::Category::getInstance(std::string("log_curl"));
    logger_curl.info("URL: %s", url.c_str());

    CURL* curl = curl_easy_init();

    if (curl != nullptr) {
#ifdef _DEBUG
        curl_easy_setopt(curl, CURLOPT_DEBUGFUNCTION, CurlDebug);
        curl_easy_setopt(curl, CURLOPT_VERBOSE, 1);
#endif
        struct curl_slist* headerlist = nullptr;
        static const char buf[] = "Expect:";
        headerlist = curl_slist_append(headerlist, buf);

        logger_curl.info("CONTENTTYPE: %s", contentType.c_str());
        if (contentType == "JSON") {
            static const char buf2[] = "Content-Type: application/json";
            headerlist = curl_slist_append(headerlist, buf2);
        } else if (contentType == "XML") {
            static const char buf2[] = "Content-Type: application/xml";
            headerlist = curl_slist_append(headerlist, buf2);
        } else if (contentType == "TEXT XML") {
            static const char buf2[] = "Content-Type: text/xml";
            headerlist = curl_slist_append(headerlist, buf2);
        } else if (contentType == "HTML") {
            static const char buf2[] = "Content-Type: text/html";
            headerlist = curl_slist_append(headerlist, buf2);
        } else if (contentType == "TEXT") {
            static const char buf2[] = "Content-Type: text/plain";
            headerlist = curl_slist_append(headerlist, buf2);
        } else {
            static const char buf2[] = "Content-Type: application/x-www-form-urlencoded";
            headerlist = curl_slist_append(headerlist, buf2);
        }

        logger_curl.info("HEADER START ----------");
        for (const auto& it : customHeaders) {
            std::string s = it.first + ": " + it.second;
            headerlist = curl_slist_append(headerlist, s.c_str());
            logger_curl.info("    %s", (const char*)s.c_str());
        }
        logger_curl.info("HEADER END ----------");

        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headerlist);

        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        if (user != "" || password != "") {
            std::string sAuth = user + ":" + password;
            curl_easy_setopt(curl, CURLOPT_HTTPAUTH, CURLAUTH_BASIC);
            curl_easy_setopt(curl, CURLOPT_USERPWD, sAuth.c_str());
        }

        // This prevents us verifying the remote site certificate ... not thrilled about that but without it https calls are failing on windows.
        // This may be because of the library we are including ... really not sure. Right now RemoteFalcon will not work without this.
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, false);

        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeFunction);
        std::string buffer = "";
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &buffer);

        curl_easy_setopt(curl, CURLOPT_POST, 1L);

        logger_curl.info("BODY START ----------");
        logger_curl.info("%s", (const char*)body.c_str());
        logger_curl.info("BODY END ----------");
        curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, (long)body.size());
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, (const char*)body.c_str());
        curl_easy_setopt(curl, CURLOPT_TIMEOUT, timeout);
        curl_easy_setopt(curl, CURLOPT_ACCEPT_ENCODING, "");

        //#ifdef _DEBUG
        //            curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, headerFunction);
        //#endif

        CURLcode res = curl_easy_perform(curl);
        if (responseCode) {
            long rc = 0;
            curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &rc);
            *responseCode = rc;
        }
        curl_easy_cleanup(curl);
        if (headerlist != nullptr) {
            curl_slist_free_all(headerlist);
        }
        if (res == CURLE_OK) {
            logger_curl.debug("RESPONSE START ------");
            logger_curl.debug("%s", (const char*)buffer.c_str());
            logger_curl.debug("RESPONSE END ------");
            return buffer;
        } else {
            logger_curl.error("Curl post failed: %d", res);
        }
    }

    return "";
}

std::string Curl::HTTPSPost(const std::string& url, const std::vector<Var>& vars, const std::string& user, const std::string& password, int timeout, const std::vector<std::pair<std::string, std::string>>& customHeaders)
{
    static log4cpp::Category& logger_curl = log4cpp::Category::getInstance(std::string("log_curl"));
    logger_curl.info("URL: %s", url.c_str());

    CURL* curl = curl_easy_init();

    if (curl != nullptr) {
        struct curl_httppost* formpost = nullptr;
        struct curl_httppost* lastptr = nullptr;

        logger_curl.info("FORM START ------");
        for (const auto& it : vars) {
            curl_formadd(&formpost,
                         &lastptr,
                         CURLFORM_COPYNAME, it.key.c_str(),
                         CURLFORM_COPYCONTENTS, it.value.c_str(),
                         CURLFORM_END);
            logger_curl.info("    %s : %s", it.key.c_str(), it.value.c_str());
        }
        logger_curl.info("FORM END ------");

        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        if (user != "" || password != "") {
            std::string sAuth = user + ":" + password;
            curl_easy_setopt(curl, CURLOPT_HTTPAUTH, CURLAUTH_BASIC);
            curl_easy_setopt(curl, CURLOPT_USERPWD, sAuth.c_str());
        }

        struct curl_slist* headerlist = nullptr;
        static const char buf[] = "Expect:";
        headerlist = curl_slist_append(headerlist, buf);
        logger_curl.info("HEADER START ----------");
        for (const auto& it : customHeaders) {
            auto s = wxString::Format("%s: %s", it.first, it.second);
            headerlist = curl_slist_append(headerlist, s.c_str());
            logger_curl.info("    %s", (const char*)s.c_str());
        }
        logger_curl.info("HEADER END ----------");
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headerlist);

        curl_easy_setopt(curl, CURLOPT_HTTPPOST, formpost);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeFunction);
        curl_easy_setopt(curl, CURLOPT_TIMEOUT, timeout);
        curl_easy_setopt(curl, CURLOPT_ACCEPT_ENCODING, "");
        std::string buffer = "";
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &buffer);

        //#ifdef _DEBUG
        //            curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, headerFunction);
        //#endif

        CURLcode res = curl_easy_perform(curl);

        curl_easy_cleanup(curl);
        curl_formfree(formpost);
        if (headerlist != nullptr) {
            curl_slist_free_all(headerlist);
        }
        if (res == CURLE_OK) {
            logger_curl.debug("RESPONSE START ----------");
            logger_curl.debug("%s", (const char*)buffer.c_str());
            logger_curl.debug("RESPONSE END ----------");
            return buffer;
        } else {
            const char* err = curl_easy_strerror(res);
            logger_curl.error("Curl post failed: %d : %s", res, err);
        }
    }

    return "";
}

std::string Curl::HTTPSGet(const std::string& s, const std::string& user, const std::string& password, int timeout, const std::vector<std::pair<std::string, std::string>>& customHeaders, int* responseCode)
{
    static log4cpp::Category& logger_curl = log4cpp::Category::getInstance(std::string("log_curl"));
    static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    logger_curl.info("URL: %s", s.c_str());

    std::string res;
    CURL* curl = curl_easy_init();
    if (curl) {
        curl_easy_setopt(curl, CURLOPT_URL, s.c_str());
        if (user != "" || password != "") {
            std::string sAuth = user + ":" + password;
            curl_easy_setopt(curl, CURLOPT_HTTPAUTH, CURLAUTH_BASIC);
            curl_easy_setopt(curl, CURLOPT_USERPWD, sAuth.c_str());
        }
        curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 1L);
        curl_easy_setopt(curl, CURLOPT_USERAGENT, "curl/7.42.0");
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
        curl_easy_setopt(curl, CURLOPT_MAXREDIRS, 50L);
        curl_easy_setopt(curl, CURLOPT_TCP_KEEPALIVE, 1L);
        curl_easy_setopt(curl, CURLOPT_TIMEOUT, timeout);
        curl_easy_setopt(curl, CURLOPT_ACCEPT_ENCODING, "");

#ifdef __WXMSW__
        // Temporarily adding this in order to try to catch ongoing curl crashes
        // curl_easy_setopt(curl, CURLOPT_DEBUGFUNCTION, CurlDebug);
        // curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
#endif

        struct curl_slist* headerlist = nullptr;
        logger_curl.info("HEADER START ----------");
        for (const auto& it : customHeaders) {
            auto s = wxString::Format("%s: %s", it.first, it.second);
            headerlist = curl_slist_append(headerlist, s.c_str());
            logger_curl.info("    %s", (const char*)s.c_str());
        }
        logger_curl.info("HEADER END ----------");
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headerlist);

        std::string response_string;
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeFunction);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response_string);

#ifdef __WXMSW__
        // curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, headerFunction);
#endif

        /* Perform the request, res will get the return code */
        logger_curl.debug("Curl: %s", (const char*)s.c_str());
        CURLcode r = curl_easy_perform(curl);
        logger_curl.debug(" Curl => %d", r);

        if (headerlist != nullptr) {
            curl_slist_free_all(headerlist);
        }

        if (r != CURLE_OK) {
            const char* err = curl_easy_strerror(r);
            if (err == nullptr) {
                logger_base.error("Failure to access %s: %d.", (const char*)s.c_str(), r);
            } else {
                logger_base.error("Failure to access %s: %d: %s.", (const char*)s.c_str(), r, err);
            }
        } else {
            if (responseCode) {
                long rc = 0;
                curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &rc);
                *responseCode = rc;
                logger_curl.debug("  Response code %d -> %s", *responseCode, (const char*)DecodeResponseCode(*responseCode).c_str());
            }

            res = response_string;
            logger_curl.debug("RESPONSE START ----------");
            logger_curl.debug("%s", (const char*)res.substr(0, 4096).c_str());
            logger_curl.debug("RESPONSE END ----------");
        }

        /* always cleanup */
        curl_easy_cleanup(curl);
    }
    return res;
}

// TODO once I have what i need from this move to logger_curl
int Curl::CurlDebug(CURL* handle, curl_infotype type, char* data, size_t size, void* userp)
{
    static log4cpp::Category& logger_curl = log4cpp::Category::getInstance(std::string("log_curl"));
    static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    switch (type) {
    case CURLINFO_TEXT:
        // strip off the cr
        if (strlen(data) > 0 && data[strlen(data) - 1] == '\n')
            data[strlen(data) - 1] = 0x00;
        logger_base.debug("== Info: %s", data);
        /* FALLTHROUGH */
    default: /* in case a new one is introduced to shock us */
        return 0;

    case CURLINFO_HEADER_OUT:
        logger_base.debug("=> Send header %lu", size);
        break;
    case CURLINFO_DATA_OUT:
        logger_base.debug("=> Send data %lu", size);
        break;
    case CURLINFO_SSL_DATA_OUT:
        logger_base.debug("=> Send SSL data %lu", size);
        break;
    case CURLINFO_HEADER_IN:
        logger_curl.debug("<= Recv header %lu %s", size, data);
        logger_base.debug("<= Recv header %lu", size);
        break;
    case CURLINFO_DATA_IN:
        logger_base.debug("<= Recv data %lu", size);
        break;
    case CURLINFO_SSL_DATA_IN:
        logger_base.debug("<= Recv SSL data %lu", size);
        break;
    }

    return 0;
}

bool Curl::HTTPSGetFile(const std::string& s, const std::string& filename, const std::string& user, const std::string& password, int timeout, wxProgressDialog* prog, bool keepProgress)
{
    static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));
#ifdef _DEBUG
    logger_base.debug("%s", (const char*)s.c_str());
#endif

    void* ocd = nullptr;
    
    if (prog != nullptr) {
        ocd = prog->GetClientData();
        if (keepProgress) {
            prog->SetClientData((wxClientData*)1);
        }
        else
        {
            prog->SetClientData((wxClientData*)0);
        }
    }

    bool res = true;

    FILE* fp = fopen(filename.c_str(), "wb");

    if (fp != nullptr) {
        CURL* curl = curl_easy_init();
        if (curl != nullptr) {
            curl_easy_setopt(curl, CURLOPT_URL, s.c_str());
            if (user != "" || password != "") {
                std::string sAuth = user + ":" + password;
                curl_easy_setopt(curl, CURLOPT_HTTPAUTH, CURLAUTH_BASIC);
                curl_easy_setopt(curl, CURLOPT_USERPWD, sAuth.c_str());
            }
            curl_easy_setopt(curl, CURLOPT_USERAGENT, "curl/7.42.0");
            curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
            curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
            curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
            curl_easy_setopt(curl, CURLOPT_MAXREDIRS, 50L);
            curl_easy_setopt(curl, CURLOPT_TCP_KEEPALIVE, 1L);
            curl_easy_setopt(curl, CURLOPT_TIMEOUT, timeout);
            curl_easy_setopt(curl, CURLOPT_ACCEPT_ENCODING, "");

#ifdef __WXMSW__
            // Temporarily adding this in order to try to catch ongoing curl crashes
            // curl_easy_setopt(curl, CURLOPT_DEBUGFUNCTION, CurlDebug);
            // curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
#endif

            struct curl_slist* chunk = nullptr;
            chunk = curl_slist_append(chunk, "User-Agent: Mozilla/5.0 (Windows NT 5.1; rv:21.0) Gecko/20130401 Firefox/21.0");
            curl_easy_setopt(curl, CURLOPT_HTTPHEADER, chunk);

            if (prog != nullptr) {
                curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 0L);
                curl_easy_setopt(curl, CURLOPT_PROGRESSDATA, prog);
                curl_easy_setopt(curl, CURLOPT_PROGRESSFUNCTION, progressFunction);
            } else {
                curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 1L);
            }

            curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeFileFunction);
            curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp);

            //#ifdef _DEBUG
            // curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, headerFunction);
            //#endif

            /* Perform the request, res will get the return code */
            CURLcode r = curl_easy_perform(curl);

            if (r != CURLE_OK) {
                logger_base.error("Failure to access %s -> %s: %s.", (const char*)s.c_str(), (const char*)filename.c_str(), curl_easy_strerror(r));
                res = false;
            } else {
                long response_code = 0;
                curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response_code);
                if (response_code >= 400) {
                    // not found or server error or similar
                    logger_base.error("Error getting file %s -> %s: %d -> %d.", (const char*)s.c_str(), (const char*)filename.c_str(), response_code, (const char*)DecodeResponseCode(response_code).c_str());
                    res = false;
                }
            }

            /* always cleanup */
            curl_easy_cleanup(curl);
            if (chunk != nullptr) {
                curl_slist_free_all(chunk);
            }
        }

        fclose(fp);
        if (!res) {
            remove(filename.c_str());
        }
    } else {
        logger_base.error("HTTPSGetFile: Failure to create file %s.", (const char*)filename.c_str());
        res = false;
    }

    if (prog != nullptr) {
        prog->SetClientData((wxClientData*)ocd);
    }

    return res;
}

static inline void addString(wxMemoryBuffer& buffer, const std::string& str)
{
    buffer.AppendData(str.c_str(), str.length());
}

struct HTTPFileUploadData {
    HTTPFileUploadData()
    {}

    uint8_t* data = nullptr;
    size_t dataSize = 0;
    size_t curPos = 0;

    wxFile* file = nullptr;

    uint8_t* postData = nullptr;
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
                return copy_this_much; /* we copied this many bytes */
            } else {
                // done reading from the memory data
                curPos = 0;
                if (file == nullptr) {
                    data = postData;
                    dataSize = postDataSize;
                } else {
                    data = nullptr;
                    dataSize = 0;
                }
            }
        }
        if (file != nullptr) {
            size_t t = file->Read(ptr, buffer_size);
            totalWritten += t;

            if (progress != nullptr) {
                size_t donePct = totalWritten;
                donePct *= 1000;
                donePct /= file->Length();
                if (donePct != lastDone) {
                    lastDone = donePct;
                    cancelled = !progress(donePct, progressString);
                    wxYield();
                }
            }
            if (file->Eof()) {
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

static size_t http_file_upload_callback(void* ptr, size_t size, size_t nmemb, void* userp)
{
    size_t buffer_size = size * nmemb;
    struct HTTPFileUploadData* dt = (struct HTTPFileUploadData*)userp;
    return dt->readData(ptr, buffer_size);
}

bool Curl::HTTPUploadFile(const std::string& url, const std::string& filename, const std::string& file, std::function<bool(int, std::string)> dlg, const std::string& username, const std::string& password)
{
    static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));

    bool res = true;
    wxString fn;
    wxString ext;

    for (int a = 0; a < filename.length(); a++) {
        wxChar ch = filename[a];
        if (ch == '"') {
            fn.Append("\\\"");
        } else {
            fn.Append(ch);
        }
    }

    bool cancelled = false;
    logger_base.debug("Upload via http of %s to %s.", (const char*)filename.c_str(), (const char *)url.c_str());
    //dlg->SetTitle("HTTP Upload");
    cancelled |= !dlg(0, "Transferring " + wxFileName(file).GetFullName() + " to " + wxURL(url).GetServer());
    int lastDone = 0;

    CURL* curl = curl_easy_init();
    if (curl == nullptr) {
        cancelled = true;
    } else {
        std::string curlInputBuffer;
        curl_easy_reset(curl);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeFunction);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &curlInputBuffer);
        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
        curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT_MS, 5000);
        curl_easy_setopt(curl, CURLOPT_TCP_FASTOPEN, 1L);
        curl_easy_setopt(curl, CURLOPT_ACCEPT_ENCODING, "");

#ifdef __WXMSW__
        // Temporarily adding this in order to try to catch ongoing curl crashes
        //curl_easy_setopt(curl, CURLOPT_DEBUGFUNCTION, CurlDebug);
        //curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
#endif

        std::string fullFileName = file;

        // if we cannot upload it in 15 minutes, we have serious issues or a really large file
        curl_easy_setopt(curl, CURLOPT_TIMEOUT_MS, 1000 * 15 * 60);

        curlInputBuffer.clear();
        char error[1024];

        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_ERRORBUFFER, &error);
        if (username != "") {
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
        curl_easy_setopt(curl, CURLOPT_POST, 1L);

        wxMemoryBuffer memBuffPost;
        wxMemoryBuffer memBuffPre;

        addString(memBuffPost, "\r\n--");
        addString(memBuffPost, bound);
        addString(memBuffPost, "--\r\n");

        std::string ct = "Content-Type: application/octet-stream";
        std::string cd = "Content-Disposition: form-data; name=\"avatar\"; filename=\"";
        cd += file;
        cd += "\"\r\n";
        addString(memBuffPre, "--");
        addString(memBuffPre, bound);
        addString(memBuffPre, "\r\n");
        addString(memBuffPre, cd);
        addString(memBuffPre, ct);
        addString(memBuffPre, "\r\n\r\n");

        HTTPFileUploadData data;
        wxFile fileobj;
        fileobj.Open(fn);
        logger_base.debug("File Size: %s. Content Length %s.", (const char*)std::to_string(fileobj.Length()).c_str(), (const char*)std::to_string(fileobj.Length() + memBuffPre.GetDataLen() + memBuffPost.GetDataLen()).c_str());
        // While this looks odd only by setting this can we avoid the chunked transfer. Setting CURLOPT_INFILESIZE would seem more logical but it does not work
        curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, fileobj.Length() + memBuffPre.GetDataLen() + memBuffPost.GetDataLen());
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, chunk);

        fileobj.Seek(0);
        data.data = (uint8_t*)memBuffPre.GetData();
        data.dataSize = memBuffPre.GetDataLen();
        data.progress = dlg;
        data.file = &fileobj;
        data.postData = (uint8_t*)memBuffPost.GetData();
        data.postDataSize = memBuffPost.GetDataLen();
        curl_easy_setopt(curl, CURLOPT_READFUNCTION, http_file_upload_callback);
        curl_easy_setopt(curl, CURLOPT_READDATA, &data);

        data.progressString = "Transferring " + wxFileName(file).GetFullName() + " to " + wxURL(url).GetServer();
        data.lastDone = lastDone;

        int i = curl_easy_perform(curl);
        curl_slist_free_all(chunk);

        long response_code = 0;
        if (i == CURLE_OK) {
            curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response_code);
            if (response_code == 200) {
            } else {
                logger_base.warn("Did not get 200 resonse code: %d -> %s", response_code, (const char*)DecodeResponseCode(response_code).c_str());
                cancelled = true;
            }
        } else {
            curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response_code);
            logger_base.warn("Curl did not upload file:  %d -> %s   %s", response_code, (const char*)DecodeResponseCode(response_code).c_str(), error);
            cancelled = true;
        }
        cancelled |= !dlg(1000, "");
        logger_base.info("HTTP File Upload file %s  - Return: %d - RC: %d -> %s - File: %s", url.c_str(), i, response_code, (const char*)DecodeResponseCode(response_code).c_str(), filename.c_str());
        res = !(data.cancelled || cancelled);
    }

    return res;
}
