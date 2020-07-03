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

#include <string>

#include <log4cpp/Category.hh>

class Curl
{
    static size_t writeFunction(void *ptr, size_t size, size_t nmemb, std::string* data) {
        data->append((char*)ptr, size * nmemb);
        return size * nmemb;
    }

	static size_t writeFileFunction(void *ptr, size_t size, size_t nmemb, FILE *stream) {
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
        static double pos = 0;
        if (t > 0)
        {
            pos = d * dlg->GetRange() / t;
        }
        else
        {
            pos += 10;
            if (pos > dlg->GetRange()) pos = 0;
        }
        dlg->Update(pos);
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

    static std::string HTTPSPost(const std::string& url, const wxString& body, const std::string& user = "", const std::string& password = "", const std::string& contentType = "", int timeout = 10, const std::vector<std::pair<std::string, std::string>>& customHeaders = {})
    {
#ifdef _DEBUG
        static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));
        logger_base.debug(url.c_str());
#endif

        CURL* curl = curl_easy_init();

        if (curl != nullptr)
        {
            struct curl_slist *headerlist = nullptr;
            static const char buf[] = "Expect:";
            headerlist = curl_slist_append(headerlist, buf);

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

            for (const auto& it : customHeaders) {
                auto s = wxString::Format("%s: %s", it.first, it.second);
                headerlist = curl_slist_append(headerlist, s.c_str());
            }

            curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headerlist);

            curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
            if (user != "" || password != "")
            {
                std::string sAuth = user + ":" + password;
                curl_easy_setopt(curl, CURLOPT_HTTPAUTH, CURLAUTH_BASIC);
                curl_easy_setopt(curl, CURLOPT_USERPWD, sAuth.c_str());
            }

            curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeFunction);
            std::string buffer = "";
            curl_easy_setopt(curl, CURLOPT_WRITEDATA, &buffer);

            curl_easy_setopt(curl, CURLOPT_POST, 1);

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
                return buffer;
            }
        }

        return "";
    }

    static std::string HTTPSPost(const std::string& url, const std::vector<Var>& vars, const std::string& user = "", const std::string& password = "", int timeout = 10, const std::vector<std::pair<std::string, std::string>>& customHeaders = {})
    {
#ifdef _DEBUG
        static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));
        logger_base.debug(url.c_str());
#endif

        CURL* curl = curl_easy_init();

        if (curl != nullptr)
        {
            struct curl_httppost *formpost = nullptr;
            struct curl_httppost *lastptr = nullptr;

            for (const auto& it : vars)
            {
                curl_formadd(&formpost,
                    &lastptr,
                    CURLFORM_COPYNAME, it.key.c_str(),
                    CURLFORM_COPYCONTENTS, it.value.c_str(),
                    CURLFORM_END);
            }

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
            for (const auto& it : customHeaders) {
                auto s = wxString::Format("%s: %s", it.first, it.second);
                headerlist = curl_slist_append(headerlist, s.c_str());
            }
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
                return buffer;
            }
        }

        return "";
    }

    static std::string HTTPSGet(const std::string& s, const std::string& user = "", const std::string& password = "", int timeout = 10, const std::vector<std::pair<std::string, std::string>>& customHeaders = {})
    {
        static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));
#ifdef _DEBUG
        logger_base.debug(s.c_str());
#endif

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

            struct curl_slist* headerlist = nullptr;
            for (const auto& it : customHeaders)                 {
                auto s = wxString::Format("%s: %s", it.first, it.second);
                headerlist = curl_slist_append(headerlist, s.c_str());
            }
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

            if (r != CURLE_OK)
            {
                logger_base.error("Failure to access %s: %s.", (const char*)s.c_str(), curl_easy_strerror(r));
            }
            else
            {
                res = response_string;
            }

            /* always cleanup */
            curl_easy_cleanup(curl);
        }
        return res;
    }

    static bool HTTPSGetFile(const std::string& s, const std::string& filename, const std::string& user = "", const std::string& password = "", int timeout = 10, wxProgressDialog * prog = nullptr)
    {
        static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));
#ifdef _DEBUG
        logger_base.debug(s.c_str());
#endif

        bool res = true;

        FILE* fp = fopen(filename.c_str(), "wb");

        if (fp != nullptr)
        {
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
                curl_easy_setopt(curl, CURLOPT_ACCEPT_ENCODING, "gzip,deflate");

                struct curl_slist* chunk = nullptr;
                chunk = curl_slist_append(chunk, "User-Agent: Mozilla/5.0 (Windows NT 5.1; rv:21.0) Gecko/20130401 Firefox/21.0");
                curl_easy_setopt(curl, CURLOPT_HTTPHEADER, chunk);

                if (prog != nullptr)
                {
                    curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 0L);
                    curl_easy_setopt(curl, CURLOPT_PROGRESSDATA, prog);
                    curl_easy_setopt(curl, CURLOPT_PROGRESSFUNCTION, progressFunction);
                }

                curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeFileFunction);
                curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp);

//#ifdef _DEBUG
//                curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, headerFunction);
//#endif 

                /* Perform the request, res will get the return code */
                CURLcode r = curl_easy_perform(curl);

                if (r != CURLE_OK)
                {
                    logger_base.error("Failure to access %s -> %s: %s.", (const char*)s.c_str(), (const char*)filename.c_str(),curl_easy_strerror(r));
                    res = false;
                }
                /* always cleanup */
                curl_easy_cleanup(curl);
                if (chunk != nullptr)
                {
                    curl_slist_free_all(chunk);
                }
            }
            fclose(fp);
        }
        else
        {
            logger_base.error("HTTPSGetFile: Failure to create file %s.", (const char*)filename.c_str());
            res = false;
        }
        return res;
    }
};
