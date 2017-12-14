#include "OutputManager.h"

#include <wx/xml/xml.h>

#include <log4cpp/Category.hh>
#include <wx/filename.h>

#include "E131Output.h"
#include "ArtNetOutput.h"
#include "DDPOutput.h"
#include "TestPreset.h"
#include <wx/msgdlg.h>
#include "../osxMacUtils.h"
#include <wx/config.h>

int OutputManager::_lastSecond = -10;
int OutputManager::_currentSecond = -10;
int OutputManager::_lastSecondCount = 0;
int OutputManager::_currentSecondCount = 0;

#pragma region Constructors and Destructors
OutputManager::OutputManager()
{
    _syncEnabled = false;
    _dirty = false;
    _syncUniverse = 0;
    _outputting = false;
    _suppressFrames = 0;
}

OutputManager::~OutputManager()
{
    // destroy all out output objects
    DeleteAllOutputs();
}
#pragma endregion Constructors and Destructors

#pragma region Save and Load
bool OutputManager::Load(const std::string& showdir, bool syncEnabled)
{
    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));

    // Remove any existing outputs
    DeleteAllOutputs();

    wxFileName fn(showdir + "/" + GetNetworksFileName());
    _filename = fn.GetFullPath();

    wxXmlDocument doc;
    doc.Load(fn.GetFullPath());

    if (doc.IsOk())
    {
        for (auto e = doc.GetRoot()->GetChildren(); e != nullptr; e = e->GetNext())
        {
            if (e->GetName() == "network")
            {
                _outputs.push_back(Output::Create(e));
            }
            else if (e->GetName() == "e131sync")
            {
                _syncUniverse = wxAtoi(e->GetAttribute("universe"));
            }
            else if (e->GetName() == "suppressframes")
            {
                _suppressFrames = wxAtoi(e->GetAttribute("frames"));
            }
            else if (e->GetName() == "testpreset")
            {
                TestPreset* tp = new TestPreset(e);

                bool exists = false;
                for (auto it = _testPresets.begin(); it != _testPresets.end() && !exists; ++it)
                {
                    if ((*it)->GetName() == tp->GetName())
                    {
                        exists = true;
                    }
                }

                if (exists)
                {
                    // dont load this preset ... it is a duplicate
                    delete tp;
                }
                else
                {
                    _testPresets.push_back(tp);
                }
            }
        }
    }
    else
    {
        logger_base.warn("Error loading networks file: %s.", (const char *)fn.GetFullPath().c_str());
        return false;
    }

    SomethingChanged();

    return true;
}

bool OutputManager::Save()
{
    wxXmlDocument doc;
    wxXmlNode* root = new wxXmlNode(wxXML_ELEMENT_NODE, "Networks");
    root->AddAttribute("computer", wxGetHostName());

    doc.SetRoot(root);

    if (_syncUniverse != 0)
    {
        wxXmlNode* newNode = new wxXmlNode(wxXmlNodeType::wxXML_ELEMENT_NODE, "e131sync");
        newNode->AddAttribute("universe", wxString::Format("%d", _syncUniverse));
        root->AddChild(newNode);
    }

    if (_suppressFrames != 0)
    {
        wxXmlNode* newNode = new wxXmlNode(wxXmlNodeType::wxXML_ELEMENT_NODE, "suppressframes");
        newNode->AddAttribute("frames", wxString::Format("%d", _suppressFrames));
        root->AddChild(newNode);
    }

    for (auto it = _outputs.begin(); it != _outputs.end(); ++it)
    {
        root->AddChild((*it)->Save());
    }

    for (auto it = _testPresets.begin(); it != _testPresets.end(); ++it)
    {
        root->AddChild((*it)->Save());
    }

    if (doc.Save(_filename))
    {
        _dirty = false;
    }

    return (_dirty == false);
}
#pragma endregion Save and Load

#pragma region Controller Discovery
bool OutputManager::Discover()
{
    bool found = false;
    auto artnet = ArtNetOutput::Discover();
    auto ddp = DDPOutput::Discover();
    auto e131 = E131Output::Discover();

    auto outputs = GetAllOutputs("");

    for (auto it =  artnet.begin(); it != artnet.end(); ++it)
    {
        if (std::find(outputs.begin(), outputs.end(), *it) == outputs.end())
        {
            _outputs.push_back(*it);
            found = true;
        }
    }

    for (auto it = ddp.begin(); it != ddp.end(); ++it)
    {
        if (std::find(outputs.begin(), outputs.end(), *it) == outputs.end())
        {
            _outputs.push_back(*it);
            found = true;
        }
    }

    for (auto it = e131.begin(); it != e131.end(); ++it)
    {
        if (std::find(outputs.begin(), outputs.end(), *it) == outputs.end())
        {
            _outputs.push_back(*it);
            found = true;
        }
    }

    return found;
}
#pragma endregion Controller Discovery

#pragma region Getters and Setters
// get an output based on an output number - zero based
Output* OutputManager::GetOutput(int outputNumber) const
{
    if (outputNumber >= (int)_outputs.size() || outputNumber < 0)
    {
        return nullptr;
    }

    auto iter = _outputs.begin();
    std::advance(iter, outputNumber);
    return *iter;
}

void OutputManager::SetShowDir(const std::string& showDir)
{
    wxFileName fn(showDir + "/" + GetNetworksFileName());
    _filename = fn.GetFullPath();
}

// get an output based on an absolute channel number
Output* OutputManager::GetOutput(long absoluteChannel, long& startChannel) const
{
    for (auto it = _outputs.begin(); it != _outputs.end(); ++it)
    {
        if (absoluteChannel >= (*it)->GetStartChannel() && absoluteChannel <= (*it)->GetEndChannel())
        {
            startChannel = absoluteChannel - (*it)->GetStartChannel() + 1;
            return *it;
        }
    }

    return nullptr;
}

// get an output based on a universe number
Output* OutputManager::GetOutput(int universe, const std::string& ip) const
{
    for (auto it = _outputs.begin(); it != _outputs.end(); ++it)
    {
        if (universe == (*it)->GetUniverse() && (ip == "" || ip == (*it)->GetIP()))
        {
            return (*it);
        }

        if ((*it)->IsOutputCollection())
        {
            auto outputs = (*it)->GetOutputs();

            for (auto it2 = outputs.begin(); it2 != outputs.end(); ++it2)
            {
                if (universe == (*it2)->GetUniverse() && (ip == "" || ip == (*it2)->GetIP()))
                {
                    return (*it2);
                }
            }
        }
    }

    return nullptr;
}

long OutputManager::GetTotalChannels() const
{
    if (_outputs.size() == 0) return 0;

    return _outputs.back()->GetEndChannel();
}

std::string OutputManager::GetChannelName(long channel)
{
    long startChannel = 0;
    ++channel;
    Output* o = GetOutput(channel, startChannel);

    if (o == nullptr)
    {
        return wxString::Format(wxT("Ch %ld: invalid"), channel).ToStdString();
    }
    else
    {
        return wxString::Format(wxT("Ch %ld: Net %i #%ld"),
                                channel,
                                o->GetOutputNumber(),
                                (long)(channel - o->GetStartChannel() + 1)).ToStdString();
    }
}

long OutputManager::GetAbsoluteChannel(int outputNumber, int startChannel) const
{
    if (outputNumber >= (int)_outputs.size()) return -1;

    auto it = _outputs.begin();
    for (int i = 0; i < outputNumber; i++)
    {
        ++it;
    }

    return (*it)->GetStartChannel() + startChannel;
}

long OutputManager::GetAbsoluteChannel(const std::string& ip, int universe, int startChannel) const
{
    auto o = GetAllOutputs(ip);
    auto it = o.begin();

    while (it != o.end())
    {
        if (universe+1 == (*it)->GetUniverse() && (ip == "" || ip == (*it)->GetIP()))
        {
            break;
        }
        ++it;
    }

    if (it == o.end()) return -1;

    return (*it)->GetStartChannel() + startChannel;
}

long OutputManager::DecodeStartChannel(const std::string& startChannelString)
{
    // Decodes Absolute, Output:StartChannel, #Universe:StartChannel, and #IP:Universe:StartChannel
    // If there is an error 0 is returned

    if (startChannelString == "") return 0;

    if (startChannelString.find(':') != std::string::npos)
    {
        if (startChannelString[0] == '#')
        {
            auto parts = wxSplit(&startChannelString[1], ':');
            if (parts.size() > 3) return 0;
            if (parts.size() == 2)
            {
                int uni = wxAtoi(parts[0]);
                long sc = wxAtol(parts[1]);
                if (uni < 1) return 0;
                if (sc < 1) return 0;
                Output* o = GetOutput(uni, "");
                if (o == nullptr) return 0;
                return o->GetStartChannel() + sc - 1;
            }
            else
            {
                std::string ip = parts[0].ToStdString();
                int uni = wxAtoi(parts[1]);
                long sc = wxAtol(parts[2]);
                if (ip == "") return 0;
                if (uni < 1) return 0;
                if (sc < 1) return 0;
                Output* o = GetOutput(uni, ip);
                if (o == nullptr) return 0;
                return o->GetStartChannel() + sc - 1;
            }
        }
        else
        {
            auto parts = wxSplit(startChannelString, ':');
            if (parts.size() > 2) return 0;
            int output = wxAtoi(parts[0]);
            long sc = wxAtol(parts[1]);
            if (output < 1) return 0;
            if (sc < 1) return 0;
            Output* o = GetOutput(output-1);
            if (o == nullptr) return 0;
            return o->GetStartChannel() + sc - 1;
        }
    }
    else
    {
        return wxAtol(startChannelString);
    }
}

bool OutputManager::IsDirty() const
{
    if (_dirty) return _dirty;

    for (auto it = _outputs.begin(); it != _outputs.end(); ++it)
    {
        if ((*it)->IsDirty())
        {
            return true;
        }
    }

    return false;
}

std::list<int> OutputManager::GetIPUniverses(const std::string& ip) const
{
    std::list<int> res;

    for (auto it = _outputs.begin(); it != _outputs.end(); ++it)
    {
        if ((*it)->IsIpOutput() && (ip == "" || ip == (*it)->GetIP()))
        {
            if ((*it)->IsOutputCollection())
            {
                auto coll = (*it)->GetOutputs();
                for (auto it2 = coll.begin(); it2 != coll.end(); ++it2)
                {
                    if (std::find(res.begin(), res.end(), (*it2)->GetUniverse()) == res.end())
                    {
                        res.push_back((*it2)->GetUniverse());
                    }
                }
            }
            else
            {
                if (std::find(res.begin(), res.end(), (*it)->GetUniverse()) == res.end())
                {
                    res.push_back((*it)->GetUniverse());
                }
            }
        }
    }

    return res;
}

std::list<std::string> OutputManager::GetIps() const
{
    std::list<std::string> res;

    for (auto it = _outputs.begin(); it != _outputs.end(); ++it)
    {
        if ((*it)->IsIpOutput())
        {
            if (std::find(res.begin(), res.end(), (*it)->GetIP()) == res.end())
            {
                res.push_back((*it)->GetIP());
            }
        }
    }

    return res;
}

size_t OutputManager::TxNonEmptyCount()
{
    size_t res = 0;
    for (auto it = _outputs.begin(); it != _outputs.end(); ++it)
    {
        res += (*it)->TxNonEmptyCount();
    }

    return res;
}
bool OutputManager::TxEmpty()
{
    for (auto it = _outputs.begin(); it != _outputs.end(); ++it)
    {
        if (!(*it)->TxEmpty()) return false;
    }

    return true;
}

// Need to call this whenever something may have changed in an output to ensure all the transient data it updated
void OutputManager::SomethingChanged() const
{
    int nullcnt = 0;
    int cnt = 0;
    int start = 1;
    for (auto it = _outputs.begin(); it != _outputs.end(); ++it)
    {
        cnt++;
        if ((*it)->GetType() == OUTPUT_NULL)
        {
            nullcnt++;
            (*it)->SetTransientData(cnt, start, nullcnt);
        }
        else
        {
            (*it)->SetTransientData(cnt, start, -1);
        }

        start += (*it)->GetChannels() * (*it)->GetUniverses();
    }
}

void OutputManager::SetForceFromIP(const std::string& forceFromIP)
{
    IPOutput::SetLocalIP(forceFromIP);
}
#pragma endregion Getters and Setters

#pragma region Output Management
void OutputManager::AddOutput(Output* output, Output* after)
{
    if (after == nullptr)
    {
        _outputs.push_back(output);
    }
    else
    {
        std::list<Output*> newoutputs;
        bool added = false;
        for (auto it = _outputs.begin(); it != _outputs.end(); ++it)
        {
            newoutputs.push_back(*it);
            if (*it == after)
            {
                newoutputs.push_back(output);
                added = true;
            }
        }
        if (!added)
        {
            newoutputs.push_back(output);
        }
        _outputs = newoutputs;
    }

    SomethingChanged();

    _dirty = true;
}

void OutputManager::AddOutput(Output* output, int pos)
{
    if (pos == -1)
    {
        _outputs.push_back(output);
    }
    else
    {
        std::list<Output*> newoutputs;
        int cnt = 0;
        bool added = false;
        for (auto it = _outputs.begin(); it != _outputs.end(); ++it)
        {
            cnt++;
            if (cnt == pos+1)
            {
                newoutputs.push_back(output);
                added = true;
            }
            newoutputs.push_back(*it);
        }
        if (!added)
        {
            newoutputs.push_back(output);
        }
        _outputs = newoutputs;
    }

    SomethingChanged();

    _dirty = true;
}

// This will actually delete the output object so it should not be accessed after this call
void OutputManager::DeleteOutput(Output* output)
{
    if (std::find(_outputs.begin(), _outputs.end(), output) == _outputs.end()) return;

    _dirty = true;
    _outputs.remove(output);
    delete output;
    
    SomethingChanged();
}

// This will actually delete the outputs objects so they should not be accessed after this call
void OutputManager::DeleteAllOutputs()
{
    _dirty = true;

    for (auto it=  _outputs.begin(); it != _outputs.end(); ++it)
    {
        delete (*it);
    }

    _outputs.clear();
    
    SomethingChanged();
}

void OutputManager::MoveOutput(Output* output, int toOutputNumber)
{
    std::list<Output*> res;
    int i = 1;
    bool added = false;
    for (auto it = _outputs.begin(); it != _outputs.end(); ++it)
    {
        if (i == toOutputNumber + 1)
        {
            res.push_back(output);
            added = true;
            i++;
        }
        if (*it == output)
        {
            // do nothing we are moving this
        }
        else
        {
            res.push_back(*it);
            i++;
        }
    }

    if (!added)
    {
        res.push_back(output);
    }

    _outputs = res;

    SomethingChanged();
}

bool OutputManager::AreAllIPOutputs(std::list<int> outputNumbers)
{
    auto outputs = GetAllOutputs(outputNumbers);

    for (auto it = outputs.begin(); it != outputs.end(); ++it)
    {
        if (!(*it)->IsIpOutput())
        {
            return false;
        }
    }

    return true;
}

std::list<Output*> OutputManager::GetAllOutputs(const std::string& ip, const std::list<int>& selected) const
{
    std::list<Output*> res;

    std::list<Output*> sel = GetAllOutputs(selected);

    for (auto it = _outputs.begin(); it != _outputs.end(); ++it)
    {
        if (ip == "" || (*it)->GetIP() == ip)
        {
            if ((*it)->IsOutputCollection())
            {
                auto o = (*it)->GetOutputs();
                for (auto it2 = o.begin(); it2 != o.end(); ++it2)
                {
                    res.push_back(*it2);
                }
            }
            else
            {
                res.push_back(*it);
            }
        }
        else
        {
            for (auto it2 = sel.begin(); it2 != sel.end(); ++it2)
            {
                if ((*it2)->GetOutputNumber() == (*it)->GetOutputNumber())
                {
                    if ((*it2)->IsOutputCollection())
                    {
                        auto o = (*it2)->GetOutputs();
                        for (auto it3 = o.begin(); it3 != o.end(); ++it3)
                        {
                            res.push_back(*it3);
                        }
                    }
                    else
                    {
                        res.push_back(*it2);
                    }
                }
            }
        }
    }

    return res;
}

std::list<Output*> OutputManager::GetAllOutputs(const std::list<int>& outputNumbers) const
{
    std::list<Output*> res;

    for (auto it = outputNumbers.begin(); it != outputNumbers.end(); ++it)
    {
        Output* o = GetOutput(*it);
        if (o != nullptr)
        {
            if (o->IsOutputCollection())
            {
                auto o2 = o->GetOutputs();
                for (auto it2 = o2.begin(); it2 != o2.end(); ++it2)
                {
                    res.push_back(*it2);
                }
            }
            else
            {
                res.push_back(o);
            }
        }
    }

    return res;
}

void OutputManager::Replace(Output* replacethis, Output* withthis)
{
    std::list<Output*> newoutputs;

    for (auto it = _outputs.begin(); it != _outputs.end(); ++it)
    {
        if ((*it) == replacethis)
        {
            newoutputs.push_back(withthis);
        }
        else
        {
            newoutputs.push_back(*it);
        }
    }
    _outputs = newoutputs;
}
#pragma endregion Output Management

#pragma region Frame Handling
void OutputManager::StartFrame(long msec)
{
    if (!_outputting) return;
    if (!_outputCriticalSection.TryEnter()) return;
    for (auto it = _outputs.begin(); it != _outputs.end(); ++it)
    {
        (*it)->StartFrame(msec);
    }
    _outputCriticalSection.Leave();
}

void OutputManager::ResetFrame()
{
    if (!_outputting) return;
    if (!_outputCriticalSection.TryEnter()) return;
    for (auto it = _outputs.begin(); it != _outputs.end(); ++it)
    {
        (*it)->ResetFrame();
    }
    _outputCriticalSection.Leave();
}

void OutputManager::EndFrame()
{
    if (!_outputting) return;
    if (!_outputCriticalSection.TryEnter()) return;

    for (auto it = _outputs.begin(); it != _outputs.end(); ++it)
    {
        (*it)->EndFrame(_suppressFrames);
    }

    if (IsSyncEnabled())
    {
        if (_syncUniverse != 0)
        {
            if (UseE131())
            {
                E131Output::SendSync(_syncUniverse);
            }
        }

        if (UseArtnet())
        {
            ArtNetOutput::SendSync();
        }

        if (UseDDP())
        {
            DDPOutput::SendSync();
        }
    }
    _outputCriticalSection.Leave();
}
#pragma endregion Frame Handling

#pragma region Start and Stop
bool OutputManager::StartOutput()
{
    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));

    if (_outputting) return false;
    if (!_outputCriticalSection.TryEnter()) return false;

    logger_base.debug("Starting light output.");

    int started = 0;
    bool ok = true;
    bool err = false;

    for (auto it = _outputs.begin(); it != _outputs.end(); ++it)
    {
        bool preok = ok;
        ok = (*it)->Open() && ok;
        if (!ok && ok != preok)
        {
            logger_base.error("An error occured opening output %d (%s). Do you want to continue trying to start output?", started + 1, (const char *)(*it)->GetDescription().c_str());
            if (wxMessageBox(wxString::Format(wxT("An error occured opening output %d (%s). Do you want to continue trying to start output?"), started+1, (*it)->GetDescription()), "Continue?", wxYES_NO) == wxNO) return _outputting;
            err = true;
        }
        if (ok) started++;
        _outputting = (started > 0);
    }

    if (err && !_outputting)
    {
        // fake it so we dont keep getting error messages
        _outputting = true;
    }

    _outputCriticalSection.Leave();

    if (_outputting)
    {
        DisableSleepModes();
        SetGlobalOutputtingFlag(true);
    }

    return _outputting; // even partially started is ok
}

void OutputManager::StopOutput()
{
    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));

    if (!_outputting) return;
    if (!_outputCriticalSection.TryEnter()) return;

    logger_base.debug("Stopping light output.");

    for (auto it = _outputs.begin(); it != _outputs.end(); ++it)
    {
        (*it)->Close();
    }

    _outputting = false;
    SetGlobalOutputtingFlag(false);
    _outputCriticalSection.Leave();

    EnableSleepModes();
}
#pragma endregion Start and Stop

#pragma region Data Setting
void OutputManager::AllOff()
{
    if (!_outputCriticalSection.TryEnter()) return;
    for (auto it = _outputs.begin(); it != _outputs.end(); ++it)
    {
        (*it)->AllOff();
        (*it)->EndFrame(_suppressFrames);
    }
    _outputCriticalSection.Leave();
}

// channel here is zero based
void OutputManager::SetOneChannel(long channel, unsigned char data)
{
    long sc = 0;
    Output* output = GetOutput(channel + 1, sc);
    if (output != nullptr)
    {
        if (output->IsEnabled())
        {
            output->SetOneChannel(sc - 1, data);
        }
    }
}

// channel here is zero based
void OutputManager::SetManyChannels(long channel, unsigned char* data, long size)
{
    if (size == 0) return;

    long stch;
    Output* o = GetOutput(channel + 1, stch);
    wxASSERT(o != nullptr);

    long left = size;

    while (left > 0 && o != nullptr)
    {
#ifdef _MSC_VER
        long send = min(left, (o->GetChannels() * o->GetUniverses()) - stch + 1);
#else
        long send = std::min(left, (o->GetChannels() * o->GetUniverses()) - stch + 1);
#endif
        if (o->IsEnabled())
        {
            o->SetManyChannels(stch - 1, &data[size - left], send);
        }
        stch = 1;
        left -= send;
        o = GetOutput(o->GetOutputNumber()); // get the next output
    }
}
#pragma endregion Data Setting

#pragma region Sync
bool OutputManager::UseE131() const
{
    for (auto it = _outputs.begin(); it != _outputs.end(); ++it)
    {
        if ((*it)->GetType() == OUTPUT_E131)
        {
            return true;
        }
    }

    return false;
}

bool OutputManager::UseArtnet() const
{
    for (auto it = _outputs.begin(); it != _outputs.end(); ++it)
    {
        if ((*it)->GetType() == OUTPUT_ARTNET)
        {
            return true;
        }
    }

    return false;
}

bool OutputManager::UseDDP() const
{
    for (auto it = _outputs.begin(); it != _outputs.end(); ++it)
    {
        if ((*it)->GetType() == OUTPUT_DDP)
        {
            return true;
        }
    }

    return false;
}
#pragma endregion Sync

void OutputManager::SendHeartbeat()
{
    for (auto it = _outputs.begin(); it != _outputs.end(); ++it)
    {
        (*it)->SendHeartbeat();
    }
}

#pragma region Test Presets
std::list<std::string> OutputManager::GetTestPresets()
{
    std::list<std::string> res;

    for (auto it = _testPresets.begin(); it != _testPresets.end(); ++it)
    {
        res.push_back((*it)->GetName());
    }

    return res;
}

TestPreset* OutputManager::GetTestPreset(std::string preset)
{
    for (auto it = _testPresets.begin(); it != _testPresets.end(); ++it)
    {
        if (preset == (*it)->GetName())
        {
            return *it;
        }
    }

    return nullptr;
}

// create a preset. If one with the same name exists then it is erased
TestPreset* OutputManager::CreateTestPreset(std::string preset)
{
    auto apreset = GetTestPreset(preset);
    if (apreset != nullptr)
    {
        _testPresets.remove(apreset);
        delete apreset;
    }

    auto p = new TestPreset(preset);
    _testPresets.push_back(p);
    return p;
}
#pragma endregion Test Presets

bool OutputManager::IsOutputOpenInAnotherProcess()
{
    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    wxConfig *xlconfig = new wxConfig(_("xLights"));
    if (xlconfig != nullptr)
    {
        if (xlconfig->HasEntry(_("OutputActive")))
        {
            bool state;
            xlconfig->Read(_("OutputActive"), &state);
            delete xlconfig;

            if (state)
            {
                logger_base.warn("Output already seems to be happening. This may generate odd results.");
            }

            return state;
        }
    }

    return false;
}

bool OutputManager::SetGlobalOutputtingFlag(bool state, bool force)
{
    if (state != _outputting && !force)
    {
        return false;
    }

    wxConfig *xlconfig = new wxConfig(_("xLights"));
    if (xlconfig != nullptr)
    {
        xlconfig->Write(_("OutputActive"), state);
        delete xlconfig;
        return true;
    }

    return false;
}

int OutputManager::GetPacketsPerSecond() const
{
    if (IsOutputting())
    {
        return _lastSecondCount;
    }

    return 0;
}

void OutputManager::RegisterSentPacket()
{
    int second = wxGetLocalTime() % 60;

    if (second == _currentSecond)
    {
        _currentSecondCount++;
    }
    else
    {
        if (second == _currentSecond + 1 || (second == 0 && _currentSecond == 59))
        {
            _lastSecond = _currentSecond;
            _lastSecondCount = _currentSecondCount;
        }
        else
        {
            _lastSecond = second - 1;
            if (_lastSecond < 0) _lastSecond = 59;
            _lastSecondCount = 0;
        }
        _currentSecond = second;
        _currentSecondCount = 1;
    }
}
