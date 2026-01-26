#include "SMSMessage.h"
#include "../../xLights/UtilFunctions.h"

std::set<std::string> SMSMessage::_phoneBlacklist;
std::set<std::string> SMSMessage::_blacklist;
std::set<std::string> SMSMessage::_whitelist;
int SMSMessage::__nextId = 1;

void SMSMessage::Censor(bool reject) {
    if (reject) {
        wxURI url("https://www.purgomalum.com/service/containsprofanity?text=" + _rawMessage);
        auto msg = Curl::HTTPSGet(url.BuildURI().ToStdString());
        if (msg == "false") {
            _message = _rawMessage;
        } else {
            _filtered = true;
        }
    } else {
        wxURI url("https://www.purgomalum.com/service/xml?text=" + _rawMessage);
        auto msg = Curl::HTTPSGet(url.BuildURI().ToStdString());

        wxRegEx regex("result>([^<]*)<\\/result");
        regex.Matches(msg);
        _message = regex.GetMatch(msg, 1);
    }
}

int SMSMessage::GetAgeMins() const {
    wxTimeSpan age = wxDateTime::Now().MakeGMT() - _timestamp;
    return age.GetDays() * 24 * 60 + age.GetHours() * 60 + age.GetMinutes();
}

void SMSMessage::LoadList(std::set<std::string>& set, const wxFileName& file) {
    std::ifstream t;
    t.open(file.GetFullPath().ToStdString());
    std::string line;
    while (t) {
        std::getline(t, line);
        set.insert(Lower(line));
    }
    t.close();
}

void SMSMessage::LoadBlackList() {
    if (_blacklist.size() > 0)
        return;

    wxFileName fn("Blacklist.txt");

    if (!fn.Exists()) {
        spdlog::error("Blacklist file not found {}.", (const char*)fn.GetFullPath().c_str());
        return;
    }

    LoadList(_blacklist, fn);
}

void SMSMessage::LoadPhoneBlackList() {
    if (_phoneBlacklist.size() > 0)
        return;

    wxFileName fn("PhoneBlacklist.txt");

    if (!fn.Exists()) {
        spdlog::error("Phone Blacklist file not found {}.", (const char*)fn.GetFullPath().c_str());
        return;
    }

    LoadList(_phoneBlacklist, fn);
}

void SMSMessage::LoadWhiteList() {
    if (_whitelist.size() > 0)
        return;

    wxFileName fn("Whitelist.txt");

    if (!fn.Exists()) {
        spdlog::error("Whitelist file not found {}.", (const char*)fn.GetFullPath().c_str());
        return;
    }

    LoadList(_whitelist, fn);
}

bool SMSMessage::PassesBlacklist() const {
    LoadBlackList();

    wxStringTokenizer tkz(_rawMessage, wxT(" ,;:.)([]\\/<>-_*&^%$#~`\"?"));
    while (tkz.HasMoreTokens()) {
        wxString token = tkz.GetNextToken().Lower();
        if (_blacklist.find(token.ToStdString()) != _blacklist.end()) {
            spdlog::debug("Blacklist failed on '{}'", (const char*)token.c_str());
            return false;
        }
    }
    return true;
}

bool SMSMessage::PassesPhoneBlacklist() const {
    LoadPhoneBlackList();

    if (_phoneBlacklist.find(_from) == _phoneBlacklist.end()) {
        return true;
    }
    spdlog::debug("Phone blacklist failed on '{}'", (const char*)_from.c_str());
    return false;
}

bool SMSMessage::PassesWhitelist() const {
    LoadWhiteList();

    wxStringTokenizer tkz(_rawMessage, wxT(" ,;:.!|)([]\\/<>-_*&^%$#@~`\"?"));
    while (tkz.HasMoreTokens()) {
        wxString token = tkz.GetNextToken().Lower();
        if (_whitelist.find(token.ToStdString()) == _whitelist.end()) {
            spdlog::debug("Whitelist failed on '{}'", (const char*)token.c_str());
            return false;
        }
    }
    return true;
}