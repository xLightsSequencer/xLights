#ifndef CURL_H
#define CURL_H

#include "../../include/curl/curl.h"
#include <wx/wx.h>
#include <string>
#include <log4cpp/Category.hh>

class Curl
{
    static size_t writeFunction(void *ptr, size_t size, size_t nmemb, std::string* data) {
        data->append((char*)ptr, size * nmemb);
        return size * nmemb;
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

    static std::string HTTPSPost(const std::string& url, const wxString& body, const std::string& user = "", const std::string& password = "")
    {
        CURL* curl = curl_easy_init();

        if (curl != nullptr)
        {
            struct curl_slist *headerlist = nullptr;
            static const char buf[] = "Expect:";
            headerlist = curl_slist_append(headerlist, buf);

            curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
            if (user != "" || password != "")
            {
                std::string sAuth = user + ":" + password;
                curl_easy_setopt(curl, CURLOPT_HTTPAUTH, CURLAUTH_BASIC);
                curl_easy_setopt(curl, CURLOPT_USERPWD, sAuth.c_str());
            }
            curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headerlist);

            curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeFunction);
            std::string buffer = "";
            curl_easy_setopt(curl, CURLOPT_WRITEDATA, &buffer);

            curl_easy_setopt(curl, CURLOPT_POST, 1);

            curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, (long)body.size());
            curl_easy_setopt(curl, CURLOPT_POSTFIELDS, (const char*)body.c_str());

            CURLcode res = curl_easy_perform(curl);
            curl_easy_cleanup(curl);
            curl_slist_free_all(headerlist);
            if (res == CURLE_OK)
            {
                return buffer;
            }
        }

        return "";
    }

    static std::string HTTPSPost(const std::string& url, const std::vector<Var>& vars, const std::string& user = "", const std::string& password = "")
    {
        CURL* curl = curl_easy_init();

        if (curl != nullptr)
        {
            struct curl_httppost *formpost = nullptr;
            struct curl_httppost *lastptr = nullptr;

            for (auto it : vars)
            {
                curl_formadd(&formpost,
                    &lastptr,
                    CURLFORM_COPYNAME, it.key.c_str(),
                    CURLFORM_COPYCONTENTS, it.value.c_str(),
                    CURLFORM_END);
            }

            struct curl_slist *headerlist = nullptr;
            static const char buf[] = "Expect:";
            headerlist = curl_slist_append(headerlist, buf);

            curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
            if (user != "" || password != "")
            {
                std::string sAuth = user + ":" + password;
                curl_easy_setopt(curl, CURLOPT_HTTPAUTH, CURLAUTH_BASIC);
                curl_easy_setopt(curl, CURLOPT_USERPWD, sAuth.c_str());
            }
            curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headerlist);
            curl_easy_setopt(curl, CURLOPT_HTTPPOST, formpost);
            curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeFunction);
            std::string buffer = "";
            curl_easy_setopt(curl, CURLOPT_WRITEDATA, &buffer);
            CURLcode res = curl_easy_perform(curl);
            curl_easy_cleanup(curl);
            curl_formfree(formpost);
            curl_slist_free_all(headerlist);
            if (res == CURLE_OK)
            {
                return buffer;
            }
        }

        return "";
    }

    static std::string HTTPSGet(const std::string s, const std::string& user = "", const std::string& password = "")
    {
        static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));
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

            std::string response_string;
            std::string header_string;
            curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeFunction);
            curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response_string);
            curl_easy_setopt(curl, CURLOPT_HEADERDATA, &header_string);

            /* Perform the request, res will get the return code */
            CURLcode r = curl_easy_perform(curl);

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
};
#endif
