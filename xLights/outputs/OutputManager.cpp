#include "OutputManager.h"

#include <wx/xml/xml.h>

#include <log4cpp/Category.hh>
#include <wx/filename.h>

#include "E131Output.h"
#include "ArtNetOutput.h"
#include "TestPreset.h"

OutputManager::OutputManager()
{
    _syncEnabled = false;
    _dirty = false;
    _syncUniverse = 0;
    _outputting = false;
}

OutputManager::OutputManager(const std::string& showdir, bool syncEnabled)
{
    _syncEnabled = syncEnabled;
    _dirty = false;
    _syncUniverse = 0;
    _outputting = false;

    Load(showdir, syncEnabled);
}

OutputManager::~OutputManager()
{
    
}

void OutputManager::Load(const std::string& showdir, bool syncEnabled)
{
    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));

    wxFileName fn(showdir + "/" + NETWORKSFILE);
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
            else if (e->GetName() == "testpreset")
            {
                _testPresets.push_back(new TestPreset(e));
            }
        }
    }
    else
    {
        logger_base.warn("Error loading networks file: %s.", (const char *)fn.GetFullPath().c_str());
        throw;
    }

    ResetOutputNumbers();
}

// get an output based on an output number
Output* OutputManager::GetOutput(int outputNumber) const
{
    if (outputNumber > _outputs.size()) return nullptr;

    auto it = _outputs.begin();
    for (int i = 0; i < outputNumber; i++)
    {
        ++it;
    }

    return *it;
}

// get an output based on an absolute channel number
Output* OutputManager::GetOutput(int absoluteChannel, int& startChannel) const
{
    for (auto it = _outputs.begin(); it != _outputs.end(); ++it)
    {
        if (absoluteChannel >= (*it)->GetStartChannel() && absoluteChannel <= (*it)->GetEndChannel())
        {
            startChannel = absoluteChannel - (*it)->GetStartChannel();
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
    }

    return nullptr;
}

int OutputManager::GetTotalChannels() const
{
    if (_outputs.size() == 0) return 0;

    return _outputs.back()->GetEndChannel();
}

int OutputManager::GetAbsoluteChannel(int outputNumber, int startChannel) const
{
    if (outputNumber >= _outputs.size()) return 1;

    auto it = _outputs.begin();
    for (int i = 0; i < outputNumber-1; i++)
    {
        ++it;
    }

    return (*it)->GetStartChannel() + startChannel - 1;
}

int OutputManager::GetAbsoluteChannel(const std::string& ip, int universe, int startChannel) const
{
    auto it = _outputs.begin();

    while (it != _outputs.end())
    {
        if (universe == (*it)->GetUniverse() && (ip == "" || ip == (*it)->GetIP()))
        {
            break;
        }
        ++it;
    }

    if (it == _outputs.end()) return 1;

    return (*it)->GetStartChannel() + startChannel - 1;
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
        for (auto it = _outputs.begin(); it != _outputs.end(); ++it)
        {
            cnt++;
            if (cnt == pos)
            {
                newoutputs.push_back(output);
            }
            newoutputs.push_back(*it);
        }
        _outputs = newoutputs;
    }
    ResetOutputNumbers();

    _dirty = true;
}

void OutputManager::DeleteOutput(Output* output)
{
    if (std::find(_outputs.begin(), _outputs.end(), output) == _outputs.end()) return;

    _dirty = true;
    _outputs.remove(output);
    ResetOutputNumbers();
}

void OutputManager::DeleteAllOutputs()
{
    _dirty = true;
    _outputs.clear();
    ResetOutputNumbers();
}

void OutputManager::Save()
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

    for(auto it = _outputs.begin(); it != _outputs.end(); ++it)
    {
        root->AddChild((*it)->Save());
    }

    for (auto it = _testPresets.begin(); it != _testPresets.end(); ++it)
    {
        root->AddChild((*it)->Save());
    }

    doc.Save(_filename + ".xml");

    _dirty = false;
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
            if (std::find(res.begin(), res.end(), (*it)->GetUniverse()) == res.end())
            {
                res.push_back((*it)->GetUniverse());
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

void OutputManager::EndFrame()
{
    if (!_outputting) return;
    if (!_outputCriticalSection.TryEnter()) return;

    for (auto it = _outputs.begin(); it != _outputs.end(); ++it)
    {
        (*it)->EndFrame();
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
    }
    _outputCriticalSection.Leave();
}

bool OutputManager::StartOutput()
{
    if (_outputting) return false;
    if (!_outputCriticalSection.TryEnter()) return false;

    bool ok = true;
    _outputting = true;

    for (auto it = _outputs.begin(); it != _outputs.end(); ++it)
    {
        ok = (*it)->Open() && ok;
    }
    _outputCriticalSection.Leave();

    return ok;
}

void OutputManager::StopOutput()
{
    if (!_outputting) return;
    if (!_outputCriticalSection.TryEnter()) return;

    for (auto it = _outputs.begin(); it != _outputs.end(); ++it)
    {
        (*it)->Close();
    }

    _outputting = false;
    _outputCriticalSection.Leave();
}

void OutputManager::AllOff()
{
    for (auto it = _outputs.begin(); it != _outputs.end(); ++it)
    {
        (*it)->AllOff();
    }
}

void OutputManager::SendHeartbeat()
{
    for (auto it = _outputs.begin(); it != _outputs.end(); ++it)
    {
        (*it)->SendHeartbeat();
    }
}

void OutputManager::MoveOutput(Output* output, int toOutputNumber)
{
    std::list<Output*> res;
    int i = 1;
    for(auto it = _outputs.begin(); it != _outputs.end(); ++it)
    {
        if (i == toOutputNumber)
        {
            res.push_back(output);
        }
        if (*it == output)
        {
            // do nothing we are removing this
        }
        else
        {
            res.push_back(*it);
        }
        i++;
    }
    _outputs = res;
    ResetOutputNumbers();
}

bool OutputManager::AreAllIPOutputs(std::list<int> outputNumbers)
{
    auto outputs = GetOutputs(outputNumbers);

    for (auto it=  outputs.begin(); it != outputs.end(); ++it)
    {
        if (!(*it)->IsIpOutput())
        {
            return false;
        }
    }

    return true;
}

std::list<Output*> OutputManager::GetOutputs(const std::string& ip) const
{
    std::list<Output*> res;

    for (auto it = _outputs.begin(); it != _outputs.end(); ++it)
    {
        if ((*it)->GetIP() == ip)
        {
            res.push_back(*it);
        }
    }

    return res;
}

std::list<Output*> OutputManager::GetOutputs(std::list<int>& outputNumbers) const
{
    std::list<Output*> res;

    for (auto it = outputNumbers.begin(); it != outputNumbers.end(); ++it)
    {
        Output* o = GetOutput(*it);
        if (o != nullptr)
        {
            res.push_back(o);
        }
    }

    return res;
}

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

void OutputManager::ResetOutputNumbers() const
{
    int nullcnt = 0;
    int cnt = 0;
    int start = 1;
    for(auto it = _outputs.begin(); it != _outputs.end(); ++it)
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

        start += (*it)->GetChannels();
    }
}

// Need to call this whenever something may have changed in an output to ensure all the transient data it updated
void OutputManager::SomethingChanged() const
{
    ResetOutputNumbers();
}

void OutputManager::SetOneChannel(int channel, unsigned char data)
{
    int sc = 0;
    Output* output = GetOutput(channel+1, sc);
    if (output != nullptr)
    {
        output->SetOneChannel(sc, data);
    }
}

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
    }

    auto p = new TestPreset(preset);
    _testPresets.push_back(p);
    return p;
}
