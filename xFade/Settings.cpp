#include "Settings.h"
#include <log4cpp/Category.hh>
#include <wx/string.h>
#include <wx/socket.h>

Settings::Settings(std::string settings)
{
    wxIPV4address addr;
    wxString fullhostname = wxGetFullHostName();
    addr.AnyAddress();
    wxDatagramSocket* testSocket = new wxDatagramSocket(addr, wxSOCKET_NOWAIT);

    if (testSocket) delete testSocket;
    addr.Hostname(fullhostname);

    _defaultIP = addr.IPAddress();

    _localInputIP = "";
    _localOutputIP = "";
    _E131 = true;
    _ArtNET = false;
    _midiDevice = "";
    _frameMS = 50;
    Load(settings);
}

std::string Settings::Safe(std::string s)
{
    wxString res = s;
    res.Replace("|", "");
    res.Replace(":", "");
    res.Replace("^", "");
    res.Replace(",", "");
    return res.ToStdString();
}

void Settings::SetMIDIControl(std::string controlName, int status, int channel, int data1)
{
    // unmap any other keys mapped to the same MIDI codes
    std::list<std::string> todelete;
    auto itc = _midiChannel.begin();
    auto itd = _midiData1.begin();
    for (auto it = _midiStatus.begin(); it != _midiStatus.end(); ++it)
    {
        if (it->second == status)
        {
            if (itc->second == 16 || itc->second == channel)
            {
                if (itd->second == 256 || itd->second == data1)
                {
                    todelete.push_back(it->first);
                }
            }
        }

        ++itc;
        ++itd;
    }

    for (auto it = todelete.begin(); it != todelete.end(); ++it)
    {
        _midiStatus.erase(*it);
        _midiChannel.erase(*it);
        _midiData1.erase(*it);
    }

    _midiStatus[controlName] = status; 
    _midiChannel[controlName] = channel; 
    _midiData1[controlName] = data1;
}

std::string Settings::LookupMIDI(int status, int channel, int data1) const
{
    auto itc = _midiChannel.begin();
    auto itd = _midiData1.begin();
    for (auto it = _midiStatus.begin(); it != _midiStatus.end(); ++it)
    {
        if (it->second == status)
        {
            if (itc->second == 16 || itc->second == channel)
            {
                if (itd->second == 256 || itd->second == data1)
                {
                    return it->first;
                }
            }
        }
        
        ++itc;
        ++itd;
    }

    return "";
}

void Settings::LookupMIDI(std::string controlName, int& status, int& channel, int& data1)
{
    if (_midiStatus.find(controlName) != _midiStatus.end())
    {
        status = _midiStatus[controlName];
        channel = _midiChannel[controlName];
        data1 = _midiData1[controlName];
    }
    else
    {
        status = 0x90;
        channel = 0;
        data1 = 0;
    }
}

std::string Settings::Serialise()
{
    std::string res = "";
    
    if (_midiDevice != "")
    {
        res += "|MIDI:" + _midiDevice;
    }

    if (_localInputIP != _defaultIP)
    {
        res += "|LIIP:" + _localInputIP;
    }

    if (_localOutputIP != _defaultIP)
    {
        res += "|LOIP:" + _localOutputIP;
    }

    if (_frameMS != 50)
    {
        res += "|FRM:" + wxString::Format("%d", _frameMS);
    }

    res += "|TGT:";
    auto itd = _targetDesc.begin();
    auto itp = _targetProtocol.begin();
    for (auto it = _targetIP.begin(); it != _targetIP.end(); ++it)
    {
        res += wxString::Format("%d,%s,%s,%s^", it->first, it->second, Safe(itd->second), itp->second);
        ++itd;
        ++itp;
    }

    res += "|MM:";
    auto itc = _midiChannel.begin();
    auto itd1 = _midiData1.begin();
    for (auto it = _midiStatus.begin(); it != _midiStatus.end(); ++it)
    {
        res += wxString::Format("%s,%d,%d,%d^", it->first, it->second, itc->second, itd1->second);
        ++itc;
        ++itd1;
    }

    if (_E131)
    {
        res += "|E131:";
    }
    if (_ArtNET)
    {
        res += "|ARTNET:";
    }

    if (_fadeExclude.size() > 0)
    {
        res += "|FE:";
        bool first = true;
        for (auto it = _fadeExclude.begin(); it != _fadeExclude.end(); ++it)
        {
            if (first)
            {
                first = false;
            }
            else
            {
                res += ",";
            }
            wxString fix = *it;
            fix.Replace(":", ";");
            res += fix.ToStdString();
        }
    }

    return res;
}

void Settings::Load(std::string settings)
{
    if (settings == "") return;

    _E131 = false;

    auto s1 = wxSplit(settings, '|');

    for (auto it1 = s1.begin(); it1 != s1.end(); ++it1)
    {
        auto s2 = wxSplit(*it1, ':');
        if (s2.size() == 2)
        {
            if (s2[0] == "ARTNET")
            {
                _ArtNET = true;
            }
            else if (s2[0] == "E131")
            {
                _E131 = true;
            }
            else if (s2[0] == "TGT")
            {
                auto s3 = wxSplit(s2[1], '^');
                for (auto it3 = s3.begin(); it3 != s3.end(); ++it3)
                {
                    auto s4 = wxSplit(*it3, ',');
                    if (s4.size() == 4)
                    {
                        int u = wxAtoi(s4[0]);
                        _targetIP[u] = s4[1];
                        _targetDesc[u] = s4[2];
                        _targetProtocol[u] = s4[3];
                    }
                }
            }
            else if (s2[0] == "MM")
            {
                auto s3 = wxSplit(s2[1], '^');
                for (auto it3 = s3.begin(); it3 != s3.end(); ++it3)
                {
                    auto s4 = wxSplit(*it3, ',');
                    if (s4.size() == 4)
                    {
                        _midiStatus[s4[0]] = wxAtoi(s4[1]);
                        _midiChannel[s4[0]] = wxAtoi(s4[2]);
                        _midiData1[s4[0]] = wxAtoi(s4[3]);
                    }
                }
            }
            else if (s2[0] == "FE")
            {
                auto s4 = wxSplit(s2[1], ',');
                for (auto it = s4.begin(); it != s4.end(); ++it)
                {
                    wxString fix = *it;
                    fix.Replace(";", ":");
                    _fadeExclude.push_back(fix.ToStdString());
                }
            }
            else if (s2[0] == "FRM")
            {
                _frameMS = wxAtoi(s2[1]);
            }
            else if (s2[0] == "LOIP")
            {
                _localOutputIP = s2[1];
            }
            else if (s2[0] == "LIIP")
            {
                _localInputIP = s2[1];
            }
            else if (s2[0] == "MIDI")
            {
                _midiDevice = s2[1];
            }            
        }
    }

    if (_localInputIP == "")
    {
        _localInputIP = _defaultIP;
    }
    if (_localOutputIP == "")
    {
        _localOutputIP = _defaultIP;
    }
}

int Settings::GetMIDIDeviceId()
{
    if (_midiDevice == "") return -1;
    return wxAtoi(wxString(_midiDevice).AfterLast(' '));
}

bool Settings::IsFadeExclude(std::string ch)
{
    for (auto it = _fadeExclude.begin(); it != _fadeExclude.end(); ++it)
    {
        if (*it == ch)
        {
            return true;
        }
    }

    return false;
}

bool sortusc(const std::string& first, const std::string& second)
{
    int fu, fsc;
    Settings::DecodeUSC(first, fu, fsc);
    int su, ssc;
    Settings::DecodeUSC(second, su, ssc);

    if (fu == su)
    {
        return fsc < ssc;
    }
    return fu < su;
}

void Settings::DecodeUSC(std::string ch, int& u, int &sc)
{
    auto f = wxSplit(ch, ':');
    if (f.size() == 2)
    {
        u = wxAtoi(f[0].substr(1));
        sc = wxAtoi(f[1]);
    }
}

void Settings::AddFadeExclude(std::string ch)
{
    if (!IsFadeExclude(ch))
    {
        _fadeExclude.push_back(ch);
        _fadeExclude.sort(sortusc);
    }
}

void Settings::DeleteFadeExclude(std::string ch)
{
    if (IsFadeExclude(ch))
    {
        for (auto it = _fadeExclude.begin(); it != _fadeExclude.end(); ++it)
        {
            if (*it == ch)
            {
                _fadeExclude.erase(it);
                return;
            }
        }
    }
}

std::list<int> Settings::GetExcludeChannels(int u)
{
    std::list<int> res;

    for (auto it = _fadeExclude.begin(); it != _fadeExclude.end(); ++it)
    {
        int uu, sc;
        Settings::DecodeUSC(*it, uu, sc);
        if (u == uu)
        {
            res.push_back(sc);
        }
    }

    return res;
}
