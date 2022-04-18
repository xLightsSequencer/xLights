#pragma once

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

#include <log4cpp/Category.hh>

class Curl
{
    static size_t writeFunction(void *ptr, size_t size, size_t nmemb, std::string* data) {
        if (data != nullptr) {
            data->append((char*)ptr, size * nmemb);
        }
        return size * nmemb;
    }

	static size_t writeFileFunction(void *ptr, size_t size, size_t nmemb, FILE *stream) {
        
        if (ptr == nullptr || stream == nullptr) return 0;

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
            }
            else {
                pos += 10;
                if (pos > dlg->GetRange()) pos = 0;
            }
            dlg->Update(pos);
        }

        return 0;
    }

public:

    struct Var
    {
        std::string key;
        std::string value;
        Var(const std::string& k, const std::string& v)
        {
            key = k;
            value = v;
        }
    };
    typedef struct Var Var;

//#ifdef _DEBUG
//    static size_t headerFunction(char* buffer, size_t size, size_t nitems, void* userdata)
//    {
//        static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));
//        if (buffer != nullptr) {
//            logger_base.debug(buffer);
//        }
//        return size * nitems;
//    }
//#endif

    static std::string HTTPSPost(const std::string& url, const wxString& body, const std::string& user = "", const std::string& password = "", const std::string& contentType = "", int timeout = 10, const std::vector<std::pair<std::string, std::string>>& customHeaders = {}, int *responseCode = nullptr)
    {
        static log4cpp::Category& logger_curl = log4cpp::Category::getInstance(std::string("log_curl"));
        logger_curl.info("URL: %s", url.c_str());

        CURL* curl = curl_easy_init();

        if (curl != nullptr)
        {
#ifdef _DEBUG
            curl_easy_setopt(curl, CURLOPT_DEBUGFUNCTION, CurlDebug);
            curl_easy_setopt(curl, CURLOPT_VERBOSE, 1);
#endif
            struct curl_slist *headerlist = nullptr;
            static const char buf[] = "Expect:";
            headerlist = curl_slist_append(headerlist, buf);

            logger_curl.info("CONTENTTYPE: %s", contentType.c_str());
            if (contentType == "JSON")
            {
                static const char buf2[] = "Content-Type: application/json";
                headerlist = curl_slist_append(headerlist, buf2);
            }
            else if (contentType == "XML")
            {
                static const char buf2[] = "Content-Type: application/xml";
                headerlist = curl_slist_append(headerlist, buf2);
            }
            else if (contentType == "TEXT XML")
            {
                static const char buf2[] = "Content-Type: text/xml";
                headerlist = curl_slist_append(headerlist, buf2);
            }
            else if (contentType == "HTML")
            {
                static const char buf2[] = "Content-Type: text/html";
                headerlist = curl_slist_append(headerlist, buf2);
            }
            else if (contentType == "TEXT")
            {
                static const char buf2[] = "Content-Type: text/plain";
                headerlist = curl_slist_append(headerlist, buf2);
            }
            else
            {
                static const char buf2[] = "Content-Type: application/x-www-form-urlencoded";
                headerlist = curl_slist_append(headerlist, buf2);
            }

            logger_curl.info("HEADER START ----------");
            for (const auto& it : customHeaders) {
                auto s = wxString::Format("%s: %s", it.first, it.second);
                headerlist = curl_slist_append(headerlist, s.c_str());
                logger_curl.info("    %s", (const char *)s.c_str());
            }
            logger_curl.info("HEADER END ----------");

            curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headerlist);

            curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
            if (user != "" || password != "")
            {
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

            curl_easy_setopt(curl, CURLOPT_POST, 1);

            logger_curl.info("BODY START ----------");
            logger_curl.info(body.c_str());
            logger_curl.info("BODY END ----------");
            curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, (long)body.size());
            curl_easy_setopt(curl, CURLOPT_POSTFIELDS, (const char*)body.c_str());
            curl_easy_setopt(curl, CURLOPT_TIMEOUT, timeout);

//#ifdef _DEBUG
//            curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, headerFunction);
//#endif 

            CURLcode res = curl_easy_perform(curl);
            curl_easy_cleanup(curl);
            if (headerlist != nullptr)
            {
                curl_slist_free_all(headerlist);
            }
            if (res == CURLE_OK)
            {
                logger_curl.debug("RESPONSE START ------");
                logger_curl.debug(buffer.c_str());
                logger_curl.debug("RESPONSE END ------");
                
                if (responseCode) {
                    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, responseCode);
                }
                return buffer;
            }
            else {
                logger_curl.error("Curl post failed: %d", res);
            }
        }

        return "";
    }

    static std::string HTTPSPost(const std::string& url, const std::vector<Var>& vars, const std::string& user = "", const std::string& password = "", int timeout = 10, const std::vector<std::pair<std::string, std::string>>& customHeaders = {})
    {
        static log4cpp::Category& logger_curl = log4cpp::Category::getInstance(std::string("log_curl"));
        logger_curl.info("URL: %s", url.c_str());

        CURL* curl = curl_easy_init();

        if (curl != nullptr)
        {
            struct curl_httppost *formpost = nullptr;
            struct curl_httppost *lastptr = nullptr;

            logger_curl.info("FORM START ------");
            for (const auto& it : vars)
            {
                curl_formadd(&formpost,
                    &lastptr,
                    CURLFORM_COPYNAME, it.key.c_str(),
                    CURLFORM_COPYCONTENTS, it.value.c_str(),
                    CURLFORM_END);
                logger_curl.info("    %s : %s", it.key.c_str(), it.value.c_str());
            }
            logger_curl.info("FORM END ------");

            curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
            if (user != "" || password != "")
            {
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
                logger_curl.info("    %s", (const char *)s.c_str());
            }
            logger_curl.info("HEADER END ----------");
            curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headerlist);

            curl_easy_setopt(curl, CURLOPT_HTTPPOST, formpost);
            curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeFunction);
            curl_easy_setopt(curl, CURLOPT_TIMEOUT, timeout);
            std::string buffer = "";
            curl_easy_setopt(curl, CURLOPT_WRITEDATA, &buffer);

//#ifdef _DEBUG
//            curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, headerFunction);
//#endif 

            CURLcode res = curl_easy_perform(curl);
            
            curl_easy_cleanup(curl);
            curl_formfree(formpost);
            if (headerlist != nullptr)
            {
                curl_slist_free_all(headerlist);
            }
            if (res == CURLE_OK)
            {
                logger_curl.debug("RESPONSE START ----------");
                logger_curl.debug(buffer.c_str());
                logger_curl.debug("RESPONSE END ----------");
                return buffer;
            } else {
                logger_curl.error("Curl post failed: %d", res);
            }
        }

        return "";
    }

    static std::string HTTPSGet(const std::string& s, const std::string& user = "", const std::string& password = "", int timeout = 10, const std::vector<std::pair<std::string, std::string>>& customHeaders = {}, int *responseCode = nullptr)
    {
        static log4cpp::Category& logger_curl = log4cpp::Category::getInstance(std::string("log_curl"));
        static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));
        logger_curl.info("URL: %s", s.c_str());

        std::string res;
        CURL* curl = curl_easy_init();
        if (curl)
        {
            curl_easy_setopt(curl, CURLOPT_URL, s.c_str());
            if (user != "" || password != "")
            {
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
            curl_easy_setopt(curl, CURLOPT_ACCEPT_ENCODING, "gzip");

#ifdef __WXMSW__
            // Temporarily adding this in order to try to catch ongoing curl crashes
            //curl_easy_setopt(curl, CURLOPT_DEBUGFUNCTION, CurlDebug);
            //curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L); 
#endif

            struct curl_slist* headerlist = nullptr;
            logger_curl.info("HEADER START ----------");
            for (const auto& it : customHeaders) {
                auto s = wxString::Format("%s: %s", it.first, it.second);
                headerlist = curl_slist_append(headerlist, s.c_str());
                logger_curl.info("    %s", (const char *)s.c_str());
            }
            logger_curl.info("HEADER END ----------");
            curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headerlist);

            std::string response_string;
            curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeFunction);
            curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response_string);

//#ifdef _DEBUG
//            curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, headerFunction);
//#endif 

            /* Perform the request, res will get the return code */
            CURLcode r = curl_easy_perform(curl);

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
                    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, responseCode);
                }

                res = response_string;
                logger_curl.debug("RESPONSE START ----------");
                logger_curl.debug(res.substr(0, 4096).c_str());
                logger_curl.debug("RESPONSE END ----------");
            }

            /* always cleanup */
            curl_easy_cleanup(curl);
        }
        return res;
    }

    // TODO once I have what i need from this move to logger_curl
    static int CurlDebug(CURL* handle, curl_infotype type, char* data, size_t size, void* userp)
    {
        static log4cpp::Category& logger_curl = log4cpp::Category::getInstance(std::string("log_curl"));
        static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));
        switch (type) {
        case CURLINFO_TEXT:
            // strip off the cr
            if (strlen(data) > 0 && data[strlen(data) - 1] == '\n') data[strlen(data) - 1] = 0x00;
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

    static bool HTTPSGetFile(const std::string& s, const std::string& filename, const std::string& user = "", const std::string& password = "", int timeout = 10, wxProgressDialog* prog = nullptr)
    {
        static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));
#ifdef _DEBUG
        logger_base.debug("%s", (const char*)s.c_str());
#endif

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
                curl_easy_setopt(curl, CURLOPT_ACCEPT_ENCODING, "gzip,deflate");

#ifdef __WXMSW__
                // Temporarily adding this in order to try to catch ongoing curl crashes
                curl_easy_setopt(curl, CURLOPT_DEBUGFUNCTION, CurlDebug);
                curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
#endif

                struct curl_slist* chunk = nullptr;
                chunk = curl_slist_append(chunk, "User-Agent: Mozilla/5.0 (Windows NT 5.1; rv:21.0) Gecko/20130401 Firefox/21.0");
                curl_easy_setopt(curl, CURLOPT_HTTPHEADER, chunk);

                if (prog != nullptr) {
                    curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 0L);
                    curl_easy_setopt(curl, CURLOPT_PROGRESSDATA, prog);
                    curl_easy_setopt(curl, CURLOPT_PROGRESSFUNCTION, progressFunction);
                }
                else {
                    curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 1L);
                }

                curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeFileFunction);
                curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp);

                //#ifdef _DEBUG
                //curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, headerFunction);
                //#endif 

                /* Perform the request, res will get the return code */
                CURLcode r = curl_easy_perform(curl);

                if (r != CURLE_OK) {
                    logger_base.error("Failure to access %s -> %s: %s.", (const char*)s.c_str(), (const char*)filename.c_str(), curl_easy_strerror(r));
                    res = false;
                }
                else {
                    int response_code = 0;
                    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response_code);
                    if (response_code >= 400) {
                        //not found or server error or similar
                        logger_base.error("Error getting file %s -> %s: %d.", (const char*)s.c_str(), (const char*)filename.c_str(), response_code);
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
        }
        else {
            logger_base.error("HTTPSGetFile: Failure to create file %s.", (const char*)filename.c_str());
            res = false;
        }
        return res;
    }

    static inline void addString(wxMemoryBuffer& buffer, const std::string& str)
    {
        buffer.AppendData(str.c_str(), str.length());
    }

    struct HTTPFileUploadData {

        HTTPFileUploadData() { }

        uint8_t* data = nullptr;
        size_t dataSize = 0;
        size_t curPos = 0;

        wxFile* file = nullptr;

        uint8_t* postData = nullptr;
        size_t postDataSize = 0;

        wxProgressDialog* progress = nullptr;
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
                }
                else {
                    //done reading from the memory data
                    curPos = 0;
                    if (file == nullptr) {
                        data = postData;
                        dataSize = postDataSize;
                    }
                    else {
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
                        cancelled = !progress->Update(donePct, progressString);
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

    static bool HTTPUploadFile(const std::string& url, const std::string& filename, const std::string& file, wxProgressDialog* dlg, const std::string& username = "", const std::string& password = "")
    {
        static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));

        bool res = true;
        wxString fn;
        wxString ext;

        for (int a = 0; a < filename.length(); a++) {
            wxChar ch = filename[a];
            if (ch == '"') {
                fn.Append("\\\"");
            }
            else {
                fn.Append(ch);
            }
        }

        bool cancelled = false;
        logger_base.debug("Upload via http of %s.", (const char*)filename.c_str());
        dlg->SetTitle("HTTP Upload");
        cancelled |= !dlg->Update(0, "Transferring " + wxFileName(file).GetFullName() + " to " + wxURL(url).GetServer());
        int lastDone = 0;

        CURL* curl = curl_easy_init();
        if (curl == nullptr) {
            cancelled = true;
        }
        else         {
            std::string curlInputBuffer;
            curl_easy_reset(curl);
            curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeFunction);
            curl_easy_setopt(curl, CURLOPT_WRITEDATA, &curlInputBuffer);
            curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
            curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT_MS, 5000);
            curl_easy_setopt(curl, CURLOPT_TCP_FASTOPEN, 1L);

            std::string fullFileName = file;

            //if we cannot upload it in 5 minutes, we have serious issues
            curl_easy_setopt(curl, CURLOPT_TIMEOUT_MS, 1000 * 5 * 60);

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
            std::string cl = "Content-Length: " + std::to_string(fileobj.Length() + memBuffPre.GetDataLen() + memBuffPost.GetDataLen());
            logger_base.debug("File Size: %s. %s.", (const char*)std::to_string(fileobj.Length()).c_str(), (const char*)cl.c_str());
            chunk = curl_slist_append(chunk, cl.c_str());
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

            data.progressString = "Transferring " + wxFileName(filename).GetFullName() + " to " + wxURL(url).GetServer();
            data.lastDone = lastDone;

            int i = curl_easy_perform(curl);
            curl_slist_free_all(chunk);

            long response_code = 0;
            if (i == CURLE_OK) {
                curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response_code);
                if (response_code == 200) {
                }
                else {
                    logger_base.warn("Did not get 200 resonse code:  %d", response_code);
                    cancelled = true;
                }
            }
            else {
                curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response_code);
                logger_base.warn("Curl did not upload file:  %d   %s", response_code, error);
            }
            cancelled |= !dlg->Update(1000);
            logger_base.info("HTTP File Upload file %s  - Return: %d - RC: %d - File: %s", url.c_str(), i, response_code, filename.c_str());
            res = !(data.cancelled || cancelled);
        }

        return res;
    }
};
