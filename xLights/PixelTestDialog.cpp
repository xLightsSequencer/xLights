/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/smeighan/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/smeighan/xLights/blob/master/License.txt
 **************************************************************/

#include "PixelTestDialog.h"

#include <wx/choicdlg.h>
#include <wx/msgdlg.h>
#include <wx/textdlg.h>
#include <wx/settings.h>
#include <wx/dataview.h>
#include <wx/confbase.h>
#include <wx/numdlg.h>

//(*InternalHeaders(PixelTestDialog)
#include <wx/intl.h>
#include <wx/string.h>
//*)

#include "models/Model.h"
#include "models/ModelGroup.h"
#include "models/SubModel.h"
#include <log4cpp/Category.hh>
#include "xLightsXmlFile.h"
#include "outputs/TestPreset.h"
#include "outputs/Output.h"
#include "UtilFunctions.h"
#include "outputs/ControllerSerial.h"
#include "xLightsMain.h"
#include "controllers/ControllerUploadData.h"
#include "controllers/ControllerCaps.h"
#include "ModelPreview.h"
#include "support/VectorMath.h"

#pragma region ChannelTracker
bool CompareRange(const wxLongLong& a, const wxLongLong& b)
{
    return (ChannelTracker::GetStart(a) < ChannelTracker::GetStart(b));
}

void ChannelTracker::FixOverlaps()
{
    _ranges.sort(CompareRange);

    bool ok = true;
    do
    {
        ok = true;

        auto first = _ranges.begin();
        if (first == _ranges.end()) return;
        auto second = first;
        ++second;

        while (second != _ranges.end())
        {
            long fe = GetEnd(*first);
            long fs = GetStart(*first);
            long ss = GetStart(*second);
            long se = GetEnd(*second);

            if (fe >= ss - 1 && fe <= se)
            {
                // first runs into second ... extend first and delete second
                _ranges.push_front(SetBoth(fs, se));
                _ranges.erase(first);
                _ranges.erase(second);
                ok = false;
                break;
            }
            else if (fe > se)
            {
                // second is totally unnecessary ... remove it
                _ranges.erase(second);
                ok = false;
                break;
            }
            else if (fe < ss - 1)
            {
                // there is a gap between first and second so move on
                ++first;
                ++second;
            }
            else
            {
                wxASSERT(false);
            }
        }
        _ranges.sort(CompareRange);
    } while (!ok);
}

void ChannelTracker::Dump()
{
    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    logger_base.debug("Selected channels dump:");
    for (const auto& it : _ranges)
    {
        logger_base.debug("   %ld-%ld", GetStart(it), GetEnd(it));
    }
}

void ChannelTracker::AddRange(long start, long end)
{
    wxASSERT(start >= 0 && end >= 0 && start != end);

    ClearLast();
    for (auto it = _ranges.begin(); it != _ranges.end(); ++it)
    {
        long s = GetStart(*it);
        long e = GetEnd(*it);

        if (end < s - 1 || start > e + 1)
        {
            // this is not connected to this range
        }
        else if (end <= e && start < s)
        {
            // extend this range at the start
            _ranges.push_back(SetBoth(start, GetEnd(*it)));
            _ranges.erase(it);
            _ranges.sort(CompareRange);
            _changeCount++;
            FixOverlaps();
            return;
        }
        else if (start >= s && end > e)
        {
            // extend this range at the end
            _ranges.push_back(SetBoth(GetStart(*it), end));
            _ranges.erase(it);
            _ranges.sort(CompareRange);
            _changeCount++;
            FixOverlaps();
            return;
        }
        else if (start >= s && end <= e)
        {
            // already contained within this range ... I can ignore this request
            return;
        }
        else if (start < s && end > e)
        {
            // this new range fully encapsulates the existing range
            _ranges.push_back(SetBoth(start, end));
            _ranges.erase(it);
            _ranges.sort(CompareRange);
            _changeCount++;
            FixOverlaps();
            return;
        }
        else
        {
            // this should never fire
            wxASSERT(false);
        }
    }

    // at this point it must be an entirely new range
    _changeCount++;
    _ranges.push_back(SetBoth(start, end));
    _ranges.sort(CompareRange);
}

void ChannelTracker::RemoveRange(long start, long end)
{
    ClearLast();

    auto it = _ranges.begin();

    while (it != _ranges.end())
    {
        long s = GetStart(*it);
        long e = GetEnd(*it);

        if (start <= s && end >= e)
        {
            // remove the whole item
            auto temp = it;
            ++temp;
            _ranges.remove(*it);
            it = temp;
            _changeCount++;
        }
        else if (end < s || start > e)
        {
            // do nothing this one does not overlap
            ++it;
        }
        else if (start <= s && end < e)
        {
            _ranges.push_back(SetBoth(end + 1, GetEnd(*it)));
            it = _ranges.erase(it);
            _changeCount++;
        }
        else if (start <= e && end >= e)
        {
            _ranges.push_back(SetBoth(GetStart(*it), start - 1));
            it = _ranges.erase(it);
            _changeCount++;
        }
        else if (start > s &&  end < e)
        {
            _ranges.push_back(SetBoth(GetStart(*it), start - 1));
            _ranges.push_back(SetBoth(end + 1, e));
            it = _ranges.erase(it);
            _changeCount++;
        }
        else
        {
            wxASSERT(false);
        }
    }
    _ranges.sort(CompareRange);
}
#pragma endregion

#pragma region TestItems

#pragma region TestItemCommon
class TestItemBase
{
protected:
    std::string _name; // what to display on the tree

    // either range is contiguous start->end or non contiguous ... but it can't be both
    long _absoluteStartChannel;
    long _absoluteEndChannel;
    std::vector<int> _nonContiguousChannels;

    // if contiguous then this is the prior channel returned
    // if non contiguous then this is the offset within the list
    long _lastChannel;
    wxTreeListItem _treeListItem;

public:
    virtual ~TestItemBase() {}
    TestItemBase()
    {
        _name = "";
        _absoluteStartChannel = -1;
        _absoluteEndChannel = -1;
        _lastChannel = -1;
    }
    long GetChannelCount() const
    {
        if (_nonContiguousChannels.size() > 0)
        {
            return _nonContiguousChannels.size();
        }
        else
        {
            return _absoluteEndChannel - _absoluteStartChannel + 1;
        }
    }

    wxCheckBoxState GetState(const ChannelTracker& tracker)
    {
        long offCount = 0;
        long onCount = 0;

        if (_nonContiguousChannels.size() == 0)
        {
            for (long i = _absoluteStartChannel; i <= _absoluteEndChannel; i++)
            {
                if (tracker.IsChannelOn(i))
                {
                    onCount++;
                    if (offCount > 0) break;
                }
                else
                {
                    offCount++;
                    if (onCount > 0) break;
                }
            }
        }
        else
        {
            for (const auto it : _nonContiguousChannels)
            {
                if (tracker.IsChannelOn(it))
                {
                    onCount++;
                    if (offCount > 0) break;
                }
                else
                {
                    offCount++;
                    if (onCount > 0) break;
                }
            }
        }
        if (onCount == 0 && offCount != 0)
        {
            return wxCheckBoxState::wxCHK_UNCHECKED;
        }
        else if (offCount == 0 && onCount != 0)
        {
            return wxCheckBoxState::wxCHK_CHECKED;
        }
        else
        {
            return wxCheckBoxState::wxCHK_UNDETERMINED;
        }
    }

    std::string GetName() const { return _name; }
    void SetTreeListItem(wxTreeListItem tli) { _treeListItem = tli; };
    wxTreeListItem GetTreeListItem() const { return _treeListItem; };
    bool IsContiguous() const { return _nonContiguousChannels.size() == 0; }
    long GetChannelOffset(long ch)
    {
        long fc;
        if (_nonContiguousChannels.size() > 0)
        {
            fc = _nonContiguousChannels.front();
        }
        else
        {
            fc = _absoluteStartChannel;
        }
        return ch - fc + 1;
    }
    long GetFirstChannel()
    {
        if (_nonContiguousChannels.size() > 0)
        {
            _lastChannel = 0;
            return _nonContiguousChannels.front();
        }
        else
        {
            _lastChannel = _absoluteStartChannel;
            return _lastChannel;
        }
    }
    long GetLastChannel()
    {
        if (_nonContiguousChannels.size() > 0)
        {
            return _nonContiguousChannels.back();
        }
        else
        {
            return _absoluteEndChannel;
        }
    }

    long GetNextChannel()
    {
        if (_lastChannel == -1) return -1;

        if (_nonContiguousChannels.size() > 0)
        {
            _lastChannel++;
            if (_lastChannel >= _nonContiguousChannels.size()) {
                _lastChannel = -1;
                return -1;
            }
            return _nonContiguousChannels[_lastChannel];
        }
        else
        {
            _lastChannel++;
            if (_lastChannel > _absoluteEndChannel)
            {
                _lastChannel = -1;
            }
            return _lastChannel;
        }
    }

    bool ContainsChannel(long ch)
    {
        if (_absoluteStartChannel > 0)
        {
            return (ch >= _absoluteStartChannel && ch <= _absoluteEndChannel);
        }

        for (const auto it : _nonContiguousChannels)
        {
            if (ch == it) return true;
        }
        return false;
    }
    virtual bool IsClickable() const = 0;
    virtual std::string GetType() const = 0;
};

class ChannelTestItem : public TestItemBase
{
    char _colour;
    long _channel;
    bool _channelAvailable;

public:
    virtual ~ChannelTestItem()
    {}
    ChannelTestItem(long channelOffset, long absoluteChannel, char colour, bool channelAvailable) :
        TestItemBase()
    {
        _channelAvailable = channelAvailable;
        _colour = colour;
        _absoluteStartChannel = absoluteChannel;
        _absoluteEndChannel = absoluteChannel;
        _channel = channelOffset;
        _name = "Channel ";
        if (_colour != ' ') {
            _name += "{";
            _name += _colour;
            _name += "} ";
        }
        if (_channel >= 0) {
            _name += "[" + std::string(wxString::Format(wxT("%ld"), _channel)) + "] ";
        }
        _name += "(" + std::string(wxString::Format(wxT("%ld"), _absoluteStartChannel)) + ")";
    }
    virtual bool IsClickable() const override
    {
        return _channelAvailable;
    }
    virtual std::string GetType() const override
    {
        return "Channel";
    }
};

class NodeTestItem : public TestItemBase
{
    long _nodeNumber;
    bool _channelsAvailable;

public:
    virtual ~NodeTestItem()
    {}
    NodeTestItem(long node, long absoluteChannel, int channelspernode, bool channelsAvailable) :
        TestItemBase()
    {
        _channelsAvailable = channelsAvailable;
        _nodeNumber = node;
        _absoluteStartChannel = absoluteChannel;
        _absoluteEndChannel = absoluteChannel + channelspernode - 1;
        _name = "Node {" + std::string(wxString::Format(wxT("%ld"), _nodeNumber)) + "} ";
        _name += "(" + std::string(wxString::Format(wxT("%ld"), _absoluteStartChannel)) + "-" + std::string(wxString::Format(wxT("%ld"), _absoluteEndChannel)) + ")";
    }
    virtual bool IsClickable() const override
    {
        return _channelsAvailable;
    }
    virtual std::string GetType() const override
    {
        return "Node";
    }
};

class NodesTestItem : public TestItemBase{
public:
    virtual ~NodesTestItem()
    {}
    NodesTestItem() :
        TestItemBase()
    {
        _absoluteStartChannel = -1;
        _absoluteEndChannel = -1;
        _name = "Nodes";
    }
    virtual bool IsClickable() const override
    {
        return true;
    }
    virtual std::string GetType() const override
    {
        return "Nodes";
    }
};
#pragma endregion

#pragma region OutputTestItems
class OutputRootTestItem : public TestItemBase
{
public:
    virtual ~OutputRootTestItem() {}
    OutputRootTestItem(long channels) : TestItemBase()
    {
        _absoluteStartChannel = 1;
        _absoluteEndChannel = channels;
        _name = wxString::Format("[%ld-%ld]", _absoluteStartChannel, _absoluteEndChannel).ToStdString();
    }
    virtual bool IsClickable() const override { return _absoluteEndChannel > 0; }
    virtual std::string GetType() const override { return "ControllerRoot"; }
};

class ControllerTestItem : public TestItemBase
{
    bool _inactive; // true if controller has been deactivated
    std::string _type;
    std::string _port;

public:
    virtual ~ControllerTestItem()
    {}
    ControllerTestItem(Controller* controller) :
        TestItemBase()
    {
        _type = controller->GetType();
        _inactive = !controller->IsEnabled();
        _absoluteStartChannel = controller->GetStartChannel();
        _absoluteEndChannel = controller->GetEndChannel();
        _name = controller->GetLongDescription();

        if (dynamic_cast<ControllerSerial*>(controller) != nullptr) {
            _port = dynamic_cast<ControllerSerial*>(controller)->GetPort();
        }
    }
    bool IsOutputable() const
    {
        return !_inactive && _type != "Null" && _port != "NotConnected";
    }
    virtual std::string GetType() const override
    {
        return "Controller";
    }
    virtual bool IsClickable() const override
    {
        return IsOutputable();
    }
};

class OutputTestItem : public TestItemBase
{
    bool _inactive; // true if controller has been deactivated
    std::string _type;
    std::string _port;

public:
    virtual ~OutputTestItem() {}
    OutputTestItem(Output* output) : TestItemBase()
    {
        _type = output->GetType();
        _inactive = !output->IsEnabled();
        _absoluteStartChannel = output->GetStartChannel();
        _absoluteEndChannel = output->GetEndChannel();
        _name = output->GetLongDescription();
        _port = output->GetCommPort();
    }
    bool IsOutputable() const
    {
        return !_inactive && _type != "NULL" && _port != "NotConnected";
    }
    virtual std::string GetType() const override { return "Output"; }
    virtual bool IsClickable() const override { return IsOutputable(); }
};
#pragma endregion

#pragma region ModelTestItems
class SubModelTestItem : public TestItemBase
{
    SubModel* _subModel;
    std::string _modelName;
    std::string _subModelName;
    long _nodes;
    bool _channelsAvailable;

public:
    virtual ~SubModelTestItem()
    {}
    SubModelTestItem(const std::string& name, SubModel* subModel, bool channelsAvailable, bool useLongName) :
        TestItemBase()
    {
        if (useLongName) {
            _subModelName = subModel->GetFullName();
        } else {
            _subModelName = subModel->GetName();
        }
        _subModel = subModel;
        _modelName = subModel->GetParent()->GetName();
        _channelsAvailable = channelsAvailable;

        _nodes = _subModel->GetNodeCount();
        _absoluteStartChannel = _subModel->GetFirstChannel() + 1;
        _absoluteEndChannel = _subModel->GetLastChannel() + 1;

        if (_absoluteEndChannel - _absoluteStartChannel + 1 != _nodes * _subModel->GetChanCountPerNode()) {
            // channels are not contiguous
            for (int i = 0; i < _nodes; i++) {
                // I am not sure this is right
                int32_t sc = _subModel->NodeStartChannel(i);
                for (int j = 0; j < _subModel->GetChanCountPerNode(); j++) {
                    _nonContiguousChannels.push_back(sc + 1 + j);
                }
            }
            _absoluteStartChannel = -1;
            _absoluteEndChannel = -1;
        }

        _name = "";
        if (!_channelsAvailable) {
            _name += "UNAVAILABLE ";
        }
        _name += _subModelName;
        if (_nodes > 0) {
            if (_nodes == 1) {
                _name += " [1]";
            } else {
                _name += " [1-" + std::string(wxString::Format(wxT("%ld"), _nodes)) + "]";
            }
        }
        if (_absoluteStartChannel < 1) {
            // dont add anything
        } else if (_absoluteEndChannel <= _absoluteStartChannel) {
            _name += " (" + std::string(wxString::Format(wxT("%ld"), _absoluteStartChannel)) + ")";
        } else {
            _name += " (" + std::string(wxString::Format(wxT("%ld"), _absoluteStartChannel)) + "-" + std::string(wxString::Format(wxT("%ld"), _absoluteEndChannel)) + ")";
        }
    }
    virtual bool IsClickable() const override
    {
        return _subModel != nullptr && _channelsAvailable;
    }
    virtual std::string GetType() const override
    {
        return "SubModel";
    }
};

inline bool SubModelTICompare(const SubModelTestItem* a, const SubModelTestItem* b)
{
    return NumberAwareStringCompare(a->GetName(), b->GetName()) == -1;
}

class ModelTestItem : public TestItemBase
{
    std::string _modelName;
    long _nodes = 0;
    long _nodeOffset = 0;
    bool _channelsAvailable = 0;
    std::list<SubModelTestItem*> _subModels;
    int _channelsPerNode = 0;
    std::string _channelColours;

public:
    virtual ~ModelTestItem()
    {
        while (_subModels.size() > 0) {
            delete _subModels.front();
            _subModels.pop_front();
        }
    }
    ModelTestItem(const std::string& name, const std::string& modelSuffix, ModelManager& modelManager, bool channelsAvailable, int nodes = -1, long startChannel = -1, long endChannel = -1, long nodeOffset = 0) :
        TestItemBase()
    {
        _modelName = name;
        _channelsAvailable = channelsAvailable;
        Model* model = modelManager[_modelName];

        if (model != nullptr) {
            _channelsPerNode = model->GetChanCountPerNode();
            if (_channelsPerNode == 1) {
                _channelColours = wxString(Model::EncodeColour(model->GetNodeMaskColor(0))).ToStdString() + "   ";
            } else if (_channelsPerNode == 4) {
                auto rgb = model->GetRGBOrder();
                auto wrgb = "W" + rgb;
                auto rgbw = rgb + "W";
                auto st = model->GetStringType();
                if (Contains(st, wrgb)) {
                    _channelColours = "W" + rgb + "  ";
                } else {
                    _channelColours = rgb + "W  ";
                }
            } else {
                _channelColours = model->GetRGBOrder() + "   ";
            }

            if (nodes == -1) {
                for (const auto& it : model->GetSubModels()) {
                    _subModels.push_back(new SubModelTestItem(it->GetFullName(), (SubModel*)it, channelsAvailable, false));
                }
                _subModels.sort(SubModelTICompare);
            }

            if (nodes == -1) {
                _nodes = model->GetNodeCount();
                _absoluteStartChannel = model->GetFirstChannel() + 1;
                _absoluteEndChannel = model->GetLastChannel() + 1;
                _nodeOffset = 0;
            } else {
                _nodeOffset = nodeOffset;
                _nodes = nodes;
                _absoluteStartChannel = startChannel;
                _absoluteEndChannel = endChannel;
            }

            if (_absoluteEndChannel - _absoluteStartChannel + 1 != _nodes * model->GetChanCountPerNode()) {
                // channels are not contiguous
                for (long i = 0; i < _nodes; i++) {
                    // I am not sure this is right
                    long sc = model->NodeStartChannel(i + _nodeOffset);
                    for (int j = 0; j < model->GetChanCountPerNode(); j++) {
                        _nonContiguousChannels.push_back(sc + j + 1);
                    }
                }
                _absoluteStartChannel = -1;
                _absoluteEndChannel = -1;
            }

            _name = "";
            if (!_channelsAvailable) {
                _name += "UNAVAILABLE ";
            }
            _name += _modelName;
            if (modelSuffix != "") {
                _name += " : " + modelSuffix;
            }

            if (_nodes > 0) {
                if (_nodes == 1) {
                    _name += " [1]";
                } else {
                    _name += " [1-" + std::string(wxString::Format(wxT("%ld"), _nodes)) + "]";
                }
            }
            if (_absoluteStartChannel < 1) {
                // dont add anything
            } else if (_absoluteEndChannel <= _absoluteStartChannel) {
                _name += " (" + std::string(wxString::Format(wxT("%ld"), _absoluteStartChannel)) + ")";
            } else {
                _name += " (" + std::string(wxString::Format(wxT("%ld"), _absoluteStartChannel)) + "-" + std::string(wxString::Format(wxT("%ld"), _absoluteEndChannel)) + ")";
            }
        } else {
            _channelsAvailable = false;
            _name = "Unknown model '" + _modelName + "'";
            _channelColours = "    ";
        }
    }
    std::list<SubModelTestItem*> GetSubModels() const
    {
        return _subModels;
    }
    char GetModelAbsoluteChannelColour(long ch)
    {
        if (ch < _absoluteStartChannel || ch > _absoluteEndChannel)
            return ' ';

        ch -= _absoluteStartChannel;
        return _channelColours[ch % _channelsPerNode];
    }
    std::string GetModelName() const
    {
        return _modelName;
    }
    long GetNodes() const
    {
        return _nodes;
    }
    int GetChannelsPerNode() const
    {
        return _channelsPerNode;
    }
    bool ContainsChannelRange(long start, long end) const
    {
        if (_nonContiguousChannels.size() == 0) {
            return (_absoluteStartChannel <= end && _absoluteEndChannel >= start);
        } else {
            for (const auto it : _nonContiguousChannels) {
                if (it >= start && it <= end)
                    return true;
            }
            return false;
        }
    }

    long GetNodeAbsoluteChannel(long node) const
    {
        if (_nonContiguousChannels.size() != 0) {
            return _nonContiguousChannels[node * _channelsPerNode];
        } else {
            return _absoluteStartChannel + node * _channelsPerNode;
        }
    }
    virtual bool IsClickable() const override
    {
        return _channelsAvailable;
    }
    virtual std::string GetType() const override
    {
        return "Model";
    }
};

inline bool ModelTICompare(const ModelTestItem* a, const ModelTestItem* b)
{
    return NumberAwareStringCompare(a->GetName(), b->GetName()) == -1;
}
#pragma endregion

#pragma region ModelGroupTestItems
class ModelGroupTestItem;
inline bool ModelGroupTICompare(const ModelGroupTestItem* a, const ModelGroupTestItem* b);

class ModelGroupTestItem : public TestItemBase
{
    std::string _modelGroupName;
    bool _channelsAvailable;
    std::list<ModelTestItem*> _models;
    std::list<ModelGroupTestItem*> _modelGroups;
    std::list<SubModelTestItem*> _subModels;

public:
    virtual ~ModelGroupTestItem()
    {
        while (_models.size() > 0) {
            delete _models.front();
            _models.pop_front();
        }
        // while (_subModels.size() > 0)
        //{
        //     delete _subModels.front();
        //     _subModels.pop_front();
        // }
        // while (_modelGroups.size() > 0)
        //{
        //     delete _modelGroups.front();
        //     _modelGroups.pop_front();
        // }
    }
    ModelGroupTestItem(const std::string name, ModelManager& modelManager, bool channelsAvailable) :
        TestItemBase()
    {
        _modelGroupName = name;
        _channelsAvailable = channelsAvailable;
        ModelGroup* modelGroup = (ModelGroup*)modelManager[_modelGroupName];

        if (modelGroup != nullptr) {
            // channels are not likely contiguous
            for (const auto& it : modelGroup->Models()) {
                if (it->GetDisplayAs() != "ModelGroup" && it->GetDisplayAs() != "SubModel") {
                    _models.push_back(new ModelTestItem(it->GetName(), "", modelManager, channelsAvailable));

                    long nodes = it->GetNodeCount();
                    for (int i = 0; i < nodes; i++) {
                        // I am not sure this is right
                        long sc = it->NodeStartChannel(i);
                        for (int j = 0; j < it->GetChanCountPerNode(); j++) {
                            _nonContiguousChannels.push_back(sc + 1 + j);
                        }
                    }
                } else if (it->GetDisplayAs() == "ModelGroup") {
                    _modelGroups.push_back(new ModelGroupTestItem(it->GetName(), modelManager, channelsAvailable));
                    long ch = _modelGroups.back()->GetFirstChannel();
                    while (ch > 0) {
                        _nonContiguousChannels.push_back(ch);
                        ch = _modelGroups.back()->GetNextChannel();
                    }
                } else {
                    _subModels.push_back(new SubModelTestItem(it->GetFullName(), (SubModel*)it, channelsAvailable, true));
                    long ch = _subModels.back()->GetFirstChannel();
                    while (ch > 0) {
                        _nonContiguousChannels.push_back(ch);
                        ch = _subModels.back()->GetNextChannel();
                    }
                }
            }

            _name = "";
            if (!_channelsAvailable) {
                _name += "UNAVAILABLE ";
            }
            _name += _modelGroupName;
        } else {
            _channelsAvailable = false;
            _name = "Unknown model group '" + _modelGroupName + "'";
        }
    }
    std::list<ModelGroupTestItem*> GetModelGroups() const
    {
        return _modelGroups;
    }
    std::list<SubModelTestItem*> GetSubModels() const
    {
        return _subModels;
    }
    std::list<ModelTestItem*> GetModels() const
    {
        return _models;
    }
    virtual bool IsClickable() const override
    {
        return _channelsAvailable;
    }
    virtual std::string GetType() const override
    {
        return "ModelGroup";
    }
};

inline bool ModelGroupTICompare(const ModelGroupTestItem* a, const ModelGroupTestItem* b)
{
    return NumberAwareStringCompare(a->GetName(), b->GetName()) == -1;
}
#pragma endregion

#pragma region ControllerTestItems

class CPR_SRTestItem : public TestItemBase
{
    bool _channelsAvailable = false;
    std::list<ModelTestItem*> _models;
    std::string _srName;
    char _letter;

public:
    CPR_SRTestItem(char letter, UDControllerPort* pud, ModelManager& modelManager, bool channelsAvailable, int nodes, long startChannel, long endChannel) :
        TestItemBase()
    {
        _letter = letter;
        _srName = "No Remote";
        if (letter != ' ') {
            _srName = "Smart Remote " + wxString(letter);
        }
        _channelsAvailable = channelsAvailable;
        _name = "";
        if (!_channelsAvailable) {
            _name += "UNAVAILABLE ";
        }
        _name += _srName;
        if (_channelsAvailable) {
            _name += wxString::Format(" (%ld-%ld)", startChannel, endChannel);
        }

        _absoluteStartChannel = startChannel;
        _absoluteEndChannel = endChannel;
    }

    virtual ~CPR_SRTestItem()
    {
    }
    virtual bool IsClickable() const override
    {
        return _channelsAvailable;
    }
    virtual std::string GetType() const override
    {
        return "SR";
    }
    std::list<ModelTestItem*> GetModels() const
    {
        return _models;
    }
    char GetLetter() const
    {
        return _letter;
    }
    void AddModel(ModelTestItem* m)
    {
        _models.push_back(m);
    }
};

class CPR_PortTestItem : public TestItemBase
{
    bool _channelsAvailable = false;
    std::list<CPR_SRTestItem*> _remotes;
    std::string _portName;
    uint16_t _port = 0xFFFF;

    CPR_SRTestItem* GetSmartRemote(char srl, UDControllerPort* pud, ModelManager& modelManager, bool channelsAvailable)
    {
        for (auto& it : _remotes) {
            if (it->GetLetter() == srl)
                return it;
        }

        // sr does not exist so we need to create it
        int srNodes = 0;
        long srStartChannel = -1;
        long srEndChannel = -1;

        for (const auto& it : pud->GetModels()) {
            int nodes = it->Channels() / it->GetChannelsPerPixel();
            long startChannel = it->GetStartChannel();
            long endChannel = it->GetEndChannel();
            int sr = it->GetSmartRemoteLetter();

            if (sr == srl) {
                srNodes += nodes;
                if (srStartChannel == -1 || startChannel < srStartChannel) {
                    srStartChannel = startChannel;
                }
                if (srEndChannel == -1 || endChannel > srEndChannel) {
                    srEndChannel = endChannel;
                }
            }
        }
        _remotes.push_back(new CPR_SRTestItem(srl, pud, modelManager, channelsAvailable, srNodes, srStartChannel, srEndChannel));

        return _remotes.back();
    }

public:
    CPR_PortTestItem(const std::string name, UDControllerPort* pud, ModelManager& modelManager, bool channelsAvailable) :
        TestItemBase()
    {
        _portName = name;
        _channelsAvailable = channelsAvailable && pud != nullptr && pud->GetFirstModel() != nullptr;
        _name = "";
        if (!_channelsAvailable) {
            _name += "UNAVAILABLE ";
        }
        _name += _portName;
        if (_channelsAvailable) {
            _name += wxString::Format(" (%ld-%ld)", pud->GetStartChannel(), pud->GetEndChannel());
        }

        _absoluteStartChannel = pud->GetStartChannel();
        _absoluteEndChannel = pud->GetEndChannel();
        _port = pud->GetPort();

        for (const auto& it : pud->GetModels()) {
            int nodes = it->Channels() / it->GetChannelsPerPixel();
            long startChannel = it->GetStartChannel();
            long endChannel = it->GetEndChannel();
            long nodeOffset = (startChannel - it->GetModel()->GetFirstChannel() - 1) / it->GetChannelsPerPixel();
            char srl = it->GetSmartRemoteLetter();

            auto sr = GetSmartRemote(srl, pud, modelManager, _channelsAvailable);

            sr->AddModel(new ModelTestItem(it->GetModel()->GetName(), it->GetName(), modelManager, channelsAvailable, nodes, startChannel, endChannel, nodeOffset));
        }
    }

    virtual ~CPR_PortTestItem(){
    }
    virtual bool IsClickable() const override
    {
        return _channelsAvailable;
    }
    virtual std::string GetType() const override
    {
        return "Port";
    }
    std::list<CPR_SRTestItem*> GetRemotes() const
    {
        return _remotes;
    }
    uint16_t GetPort() const
    {
        return _port;
    }
};

class CPR_ControllerTestItem : public TestItemBase
{
    bool _channelsAvailable = false;
    std::string _controllerName;
    std::list<CPR_PortTestItem*> _ports;

public:
    virtual ~CPR_ControllerTestItem()
    {
    }
    CPR_ControllerTestItem(const std::string name, ControllerCaps* caps, UDController* cud, OutputManager& outputManager, ModelManager& modelManager) :
        TestItemBase()
    {
        _controllerName = name;
        Controller* controller = outputManager.GetController(name);

        if (controller != nullptr) {
            if (caps != nullptr) {
                _channelsAvailable = controller->IsActive() && (caps->GetMaxPixelPort() > 0 || caps->GetMaxSerialPort());
                for (auto p = 0; p < caps->GetMaxPixelPort(); p++) {
                    auto pud = cud->GetControllerPixelPort(p + 1);
                    _ports.push_back(new CPR_PortTestItem(wxString::Format("Pixel Port %d", p + 1), pud, modelManager, _channelsAvailable));
                }
                for (auto p = 0; p < caps->GetMaxSerialPort(); p++) {
                    auto pud = cud->GetControllerSerialPort(p + 1);
                    _ports.push_back(new CPR_PortTestItem(wxString::Format("Serial Port %d", p + 1), pud, modelManager, _channelsAvailable));
                }
            } else {
                _channelsAvailable = controller->IsActive() && (cud->GetMaxPixelPort() > 0 || cud->GetMaxSerialPort());
                for (auto p = 0; p < cud->GetMaxPixelPort(); ++p) {
                    auto pud = cud->GetControllerPixelPort(p + 1);
                    _ports.push_back(new CPR_PortTestItem(wxString::Format("Pixel Port %d", p + 1), pud, modelManager, _channelsAvailable));
                }
                for (auto p = 0; p < cud->GetMaxSerialPort(); ++p) {
                    auto pud = cud->GetControllerSerialPort(p + 1);
                    _ports.push_back(new CPR_PortTestItem(wxString::Format("Serial Port %d", p + 1), pud, modelManager, _channelsAvailable));
                }
            }

            _name = "";
            if (!_channelsAvailable) {
                _name += "UNAVAILABLE ";
            }
            _name += controller->GetLongDescription();
            _absoluteStartChannel = controller->GetStartChannel();
            _absoluteEndChannel = controller->GetEndChannel();

        } else {
            _channelsAvailable = false;
            _name = "Unknown controller '" + _controllerName + "'";
        }
    }
    std::list<CPR_PortTestItem*> GetPorts() const
    {
        return _ports;
    }
    virtual bool IsClickable() const override
    {
        return _channelsAvailable;
    }
    virtual std::string GetType() const override
    {
        return "Controller";
    }
};

#pragma endregion
#pragma endregion

#pragma region ConstDest
// Test Dialog Methods
const long PixelTestDialog::ID_TREELISTCTRL_Outputs = wxNewId();
const long PixelTestDialog::ID_TREELISTCTRL_ModelGroups = wxNewId();
const long PixelTestDialog::ID_TREELISTCTRL_Models = wxNewId();
const long PixelTestDialog::ID_TREELISTCTRL_Controllers = wxNewId();
const long PixelTestDialog::ID_MNU_TEST_SELECTALL = wxNewId();
const long PixelTestDialog::ID_MNU_TEST_DESELECTALL = wxNewId();
const long PixelTestDialog::ID_MNU_TEST_SELECTN = wxNewId();
const long PixelTestDialog::ID_MNU_TEST_DESELECTN = wxNewId();
const long PixelTestDialog::ID_MNU_TEST_NUMBER = wxNewId();

//(*IdInit(PixelTestDialog)
const long PixelTestDialog::ID_BUTTON_Load = wxNewId();
const long PixelTestDialog::ID_BUTTON_Save = wxNewId();
const long PixelTestDialog::ID_PANEL3 = wxNewId();
const long PixelTestDialog::ID_PANEL6 = wxNewId();
const long PixelTestDialog::ID_PANEL7 = wxNewId();
const long PixelTestDialog::ID_STATICTEXT8 = wxNewId();
const long PixelTestDialog::ID_CHOICE1 = wxNewId();
const long PixelTestDialog::ID_PANEL11 = wxNewId();
const long PixelTestDialog::ID_PANEL5 = wxNewId();
const long PixelTestDialog::ID_PANEL4 = wxNewId();
const long PixelTestDialog::ID_NOTEBOOK1 = wxNewId();
const long PixelTestDialog::ID_PANEL1 = wxNewId();
const long PixelTestDialog::ID_CHECKBOX_OutputToLights = wxNewId();
const long PixelTestDialog::ID_CHECKBOX1 = wxNewId();
const long PixelTestDialog::ID_STATICTEXT2 = wxNewId();
const long PixelTestDialog::ID_RADIOBUTTON_Standard_Off = wxNewId();
const long PixelTestDialog::ID_RADIOBUTTON_Standard_Chase = wxNewId();
const long PixelTestDialog::ID_RADIOBUTTON_Standard_Chase13 = wxNewId();
const long PixelTestDialog::ID_RADIOBUTTON_Standard_Chase14 = wxNewId();
const long PixelTestDialog::ID_RADIOBUTTON_Standard_Chase15 = wxNewId();
const long PixelTestDialog::ID_RADIOBUTTON_Standard_Alternate = wxNewId();
const long PixelTestDialog::ID_RADIOBUTTON_Standard_Twinke5 = wxNewId();
const long PixelTestDialog::ID_RADIOBUTTON_Standard_Twinkle10 = wxNewId();
const long PixelTestDialog::ID_RADIOBUTTON_Standard_Twinkle25 = wxNewId();
const long PixelTestDialog::ID_RADIOBUTTON_Standard_Twinkle50 = wxNewId();
const long PixelTestDialog::ID_RADIOBUTTON_Standard_Shimmer = wxNewId();
const long PixelTestDialog::ID_RADIOBUTTON_Standard_Background = wxNewId();
const long PixelTestDialog::ID_STATICTEXT3 = wxNewId();
const long PixelTestDialog::ID_SLIDER_Standard_Background = wxNewId();
const long PixelTestDialog::ID_STATICTEXT4 = wxNewId();
const long PixelTestDialog::ID_SLIDER_Standard_Highlight = wxNewId();
const long PixelTestDialog::ID_PANEL8 = wxNewId();
const long PixelTestDialog::ID_STATICTEXT5 = wxNewId();
const long PixelTestDialog::ID_RADIOBUTTON_RGB_Off = wxNewId();
const long PixelTestDialog::ID_RADIOBUTTON_RGB_Chase = wxNewId();
const long PixelTestDialog::ID_RADIOBUTTON_RGB_Chase13 = wxNewId();
const long PixelTestDialog::ID_RADIOBUTTON_RGB_Chase14 = wxNewId();
const long PixelTestDialog::ID_RADIOBUTTON_RGB_Chase15 = wxNewId();
const long PixelTestDialog::ID_RADIOBUTTON_RGB_Alternate = wxNewId();
const long PixelTestDialog::ID_RADIOBUTTON_RGB_Twinkle5 = wxNewId();
const long PixelTestDialog::ID_RADIOBUTTON_RGB_Twinkle10 = wxNewId();
const long PixelTestDialog::ID_RADIOBUTTON_RGB_Twinkle25 = wxNewId();
const long PixelTestDialog::ID_RADIOBUTTON_RGB_Twinkle50 = wxNewId();
const long PixelTestDialog::ID_RADIOBUTTON_RGB_Shimmer = wxNewId();
const long PixelTestDialog::ID_RADIOBUTTON_RGB_Background = wxNewId();
const long PixelTestDialog::ID_SLIDER1 = wxNewId();
const long PixelTestDialog::ID_SLIDER2 = wxNewId();
const long PixelTestDialog::ID_SLIDER3 = wxNewId();
const long PixelTestDialog::ID_SLIDER4 = wxNewId();
const long PixelTestDialog::ID_SLIDER5 = wxNewId();
const long PixelTestDialog::ID_SLIDER6 = wxNewId();
const long PixelTestDialog::ID_PANEL9 = wxNewId();
const long PixelTestDialog::ID_STATICTEXT6 = wxNewId();
const long PixelTestDialog::ID_RADIOBUTTON_RGBCycle_Off = wxNewId();
const long PixelTestDialog::ID_RADIOBUTTON_RGBCycle_ABC = wxNewId();
const long PixelTestDialog::ID_RADIOBUTTON_RGBCycle_ABCAll = wxNewId();
const long PixelTestDialog::ID_RADIOBUTTON_RGBCycle_ABCAllNone = wxNewId();
const long PixelTestDialog::ID_RADIOBUTTON_RGBCycle_MixedColors = wxNewId();
const long PixelTestDialog::ID_RADIOBUTTON_RGBCycle_RGBW = wxNewId();
const long PixelTestDialog::ID_CHECKBOX2 = wxNewId();
const long PixelTestDialog::ID_PANEL10 = wxNewId();
const long PixelTestDialog::ID_NOTEBOOK2 = wxNewId();
const long PixelTestDialog::ID_STATICTEXT1 = wxNewId();
const long PixelTestDialog::ID_SLIDER_Speed = wxNewId();
const long PixelTestDialog::ID_PANEL2 = wxNewId();
const long PixelTestDialog::ID_SPLITTERWINDOW1 = wxNewId();
const long PixelTestDialog::ID_STATICTEXT7 = wxNewId();
const long PixelTestDialog::ID_TIMER1 = wxNewId();
//*)

BEGIN_EVENT_TABLE(PixelTestDialog,wxDialog)
	//(*EventTable(PixelTestDialog)
	//*)
END_EVENT_TABLE()

// Constructor

PixelTestDialog::PixelTestDialog(xLightsFrame* parent, OutputManager* outputManager, wxFileName networkFile, ModelManager* modelManager, wxWindowID id) :
    mPointSize(PIXEL_SIZE_ON_DIALOGS)
{
    _lastModel = nullptr;
    _outputManager = outputManager;
    _networkFile = networkFile;
    _modelManager = modelManager;
    _checkChannelList = false;
    _cascading = false;

    //(*Initialize(PixelTestDialog)
    wxBoxSizer* BoxSizer1;
    wxBoxSizer* BoxSizer2;
    wxBoxSizer* BoxSizer3;
    wxBoxSizer* BoxSizer4;
    wxFlexGridSizer* FlexGridSizer10;
    wxFlexGridSizer* FlexGridSizer11;
    wxFlexGridSizer* FlexGridSizer12;
    wxFlexGridSizer* FlexGridSizer13;
    wxFlexGridSizer* FlexGridSizer14;
    wxFlexGridSizer* FlexGridSizer15;
    wxFlexGridSizer* FlexGridSizer16;
    wxFlexGridSizer* FlexGridSizer1;
    wxFlexGridSizer* FlexGridSizer2;
    wxFlexGridSizer* FlexGridSizer3;
    wxFlexGridSizer* FlexGridSizer44;
    wxFlexGridSizer* FlexGridSizer4;
    wxFlexGridSizer* FlexGridSizer5;
    wxFlexGridSizer* FlexGridSizer6;
    wxFlexGridSizer* FlexGridSizer7;
    wxFlexGridSizer* FlexGridSizer8;
    wxFlexGridSizer* FlexGridSizer9;
    wxStaticBoxSizer* StaticBoxSizer1;
    wxStaticBoxSizer* StaticBoxSizer2;

    Create(parent, wxID_ANY, _("Test Lights"), wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE|wxRESIZE_BORDER|wxMAXIMIZE_BOX, _T("wxID_ANY"));
    SetClientSize(wxDefaultSize);
    FlexGridSizer1 = new wxFlexGridSizer(2, 1, 0, 0);
    FlexGridSizer1->AddGrowableCol(0);
    FlexGridSizer1->AddGrowableRow(0);
    SplitterWindow1 = new wxSplitterWindow(this, ID_SPLITTERWINDOW1, wxDefaultPosition, wxDefaultSize, wxSP_3D, _T("ID_SPLITTERWINDOW1"));
    SplitterWindow1->SetSashGravity(0.5);
    Panel1 = new wxPanel(SplitterWindow1, ID_PANEL1, wxPoint(95,46), wxDefaultSize, wxTAB_TRAVERSAL, _T("ID_PANEL1"));
    FlexGridSizer2 = new wxFlexGridSizer(0, 1, 0, 0);
    FlexGridSizer2->AddGrowableCol(0);
    FlexGridSizer2->AddGrowableRow(0);
    Notebook1 = new wxNotebook(Panel1, ID_NOTEBOOK1, wxDefaultPosition, wxDefaultSize, 0, _T("ID_NOTEBOOK1"));
    Notebook1->SetMinSize(wxDLG_UNIT(Panel1,wxSize(200,200)));
    Panel_Outputs = new wxPanel(Notebook1, ID_PANEL3, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("ID_PANEL3"));
    FlexGridSizer_Outputs = new wxFlexGridSizer(0, 1, 0, 0);
    FlexGridSizer_Outputs->AddGrowableCol(0);
    FlexGridSizer4 = new wxFlexGridSizer(0, 2, 0, 0);
    Button_Load = new wxButton(Panel_Outputs, ID_BUTTON_Load, _("Load"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON_Load"));
    FlexGridSizer4->Add(Button_Load, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
    Button_Save = new wxButton(Panel_Outputs, ID_BUTTON_Save, _("Save"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON_Save"));
    FlexGridSizer4->Add(Button_Save, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizer_Outputs->Add(FlexGridSizer4, 1, wxALL|wxEXPAND, 5);
    Panel_Outputs->SetSizer(FlexGridSizer_Outputs);
    FlexGridSizer_Outputs->Fit(Panel_Outputs);
    FlexGridSizer_Outputs->SetSizeHints(Panel_Outputs);
    Panel_ModelGroups = new wxPanel(Notebook1, ID_PANEL6, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("ID_PANEL6"));
    FlexGridSizer_ModelGroups = new wxFlexGridSizer(0, 1, 0, 0);
    FlexGridSizer_ModelGroups->AddGrowableCol(0);
    FlexGridSizer_ModelGroups->AddGrowableRow(0);
    Panel_ModelGroups->SetSizer(FlexGridSizer_ModelGroups);
    FlexGridSizer_ModelGroups->Fit(Panel_ModelGroups);
    FlexGridSizer_ModelGroups->SetSizeHints(Panel_ModelGroups);
    Panel_Models = new wxPanel(Notebook1, ID_PANEL7, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("ID_PANEL7"));
    FlexGridSizer_Models = new wxFlexGridSizer(0, 1, 0, 0);
    FlexGridSizer_Models->AddGrowableCol(0);
    FlexGridSizer_Models->AddGrowableRow(0);
    Panel_Models->SetSizer(FlexGridSizer_Models);
    FlexGridSizer_Models->Fit(Panel_Models);
    FlexGridSizer_Models->SetSizeHints(Panel_Models);
    Panel_Model = new wxPanel(Notebook1, ID_PANEL5, wxPoint(210,20), wxDefaultSize, wxTAB_TRAVERSAL, _T("ID_PANEL5"));
    FlexGridSizer15 = new wxFlexGridSizer(2, 1, 0, 0);
    FlexGridSizer15->AddGrowableCol(0);
    FlexGridSizer15->AddGrowableRow(1);
    FlexGridSizer16 = new wxFlexGridSizer(0, 2, 0, 0);
    FlexGridSizer16->AddGrowableCol(1);
    StaticText7 = new wxStaticText(Panel_Model, ID_STATICTEXT8, _("Model:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT8"));
    FlexGridSizer16->Add(StaticText7, 1, wxALL|wxEXPAND, 5);
    Choice_VisualModel = new wxChoice(Panel_Model, ID_CHOICE1, wxDefaultPosition, wxDefaultSize, 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE1"));
    FlexGridSizer16->Add(Choice_VisualModel, 1, wxALL|wxEXPAND, 5);
    FlexGridSizer15->Add(FlexGridSizer16, 1, wxALL|wxEXPAND, 5);
    FlexGridSizer44 = new wxFlexGridSizer(1, 1, 0, 0);
    FlexGridSizer44->AddGrowableCol(0);
    FlexGridSizer44->AddGrowableRow(0);
    Panel_VisualModel = new wxPanel(Panel_Model, ID_PANEL11, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("ID_PANEL11"));
    FlexGridSizer_VisualModelSizer = new wxFlexGridSizer(0, 1, 0, 0);
    FlexGridSizer_VisualModelSizer->AddGrowableCol(0);
    FlexGridSizer_VisualModelSizer->AddGrowableRow(0);
    Panel_VisualModel->SetSizer(FlexGridSizer_VisualModelSizer);
    FlexGridSizer_VisualModelSizer->Fit(Panel_VisualModel);
    FlexGridSizer_VisualModelSizer->SetSizeHints(Panel_VisualModel);
    FlexGridSizer44->Add(Panel_VisualModel, 1, wxALL|wxEXPAND, 5);
    FlexGridSizer15->Add(FlexGridSizer44, 1, wxALL|wxEXPAND, 5);
    Panel_Model->SetSizer(FlexGridSizer15);
    FlexGridSizer15->Fit(Panel_Model);
    FlexGridSizer15->SetSizeHints(Panel_Model);
    Panel_Controllers = new wxPanel(Notebook1, ID_PANEL4, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("ID_PANEL4"));
    FlexGridSizer_Controllers = new wxFlexGridSizer(0, 1, 0, 0);
    FlexGridSizer_Controllers->AddGrowableCol(0);
    FlexGridSizer_Controllers->AddGrowableRow(0);
    Panel_Controllers->SetSizer(FlexGridSizer_Controllers);
    FlexGridSizer_Controllers->Fit(Panel_Controllers);
    FlexGridSizer_Controllers->SetSizeHints(Panel_Controllers);
    Notebook1->AddPage(Panel_Outputs, _("Outputs"), false);
    Notebook1->AddPage(Panel_ModelGroups, _("Model Groups"), false);
    Notebook1->AddPage(Panel_Models, _("Models"), false);
    Notebook1->AddPage(Panel_Model, _("Model"), false);
    Notebook1->AddPage(Panel_Controllers, _("Controllers"), false);
    FlexGridSizer2->Add(Notebook1, 1, wxALL|wxEXPAND, 5);
    Panel1->SetSizer(FlexGridSizer2);
    FlexGridSizer2->Fit(Panel1);
    FlexGridSizer2->SetSizeHints(Panel1);
    Panel2 = new wxPanel(SplitterWindow1, ID_PANEL2, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("ID_PANEL2"));
    FlexGridSizer3 = new wxFlexGridSizer(0, 1, 0, 0);
    FlexGridSizer3->AddGrowableCol(0);
    FlexGridSizer3->AddGrowableRow(1);
    FlexGridSizer14 = new wxFlexGridSizer(0, 1, 0, 0);
    CheckBox_OutputToLights = new wxCheckBox(Panel2, ID_CHECKBOX_OutputToLights, _("Output to lights"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX_OutputToLights"));
    CheckBox_OutputToLights->SetValue(false);
    FlexGridSizer14->Add(CheckBox_OutputToLights, 1, wxALL|wxEXPAND, 2);
    CheckBox_SuppressUnusedOutputs = new wxCheckBox(Panel2, ID_CHECKBOX1, _("Don\'t send data to unused outputs"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX1"));
    CheckBox_SuppressUnusedOutputs->SetValue(false);
    FlexGridSizer14->Add(CheckBox_SuppressUnusedOutputs, 1, wxALL|wxEXPAND, 2);
    FlexGridSizer3->Add(FlexGridSizer14, 1, wxALL|wxEXPAND, 5);
    Notebook2 = new wxNotebook(Panel2, ID_NOTEBOOK2, wxDefaultPosition, wxDefaultSize, 0, _T("ID_NOTEBOOK2"));
    PanelStandard = new wxPanel(Notebook2, ID_PANEL8, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("ID_PANEL8"));
    FlexGridSizer6 = new wxFlexGridSizer(1, 4, 0, 0);
    FlexGridSizer6->AddGrowableCol(3);
    FlexGridSizer6->AddGrowableRow(0);
    FlexGridSizer7 = new wxFlexGridSizer(0, 1, 0, 0);
    FlexGridSizer7->AddGrowableCol(0);
    StaticText2 = new wxStaticText(PanelStandard, ID_STATICTEXT2, _("Function"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT2"));
    FlexGridSizer7->Add(StaticText2, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
    RadioButton_Standard_Off = new wxRadioButton(PanelStandard, ID_RADIOBUTTON_Standard_Off, _("Off"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_RADIOBUTTON_Standard_Off"));
    FlexGridSizer7->Add(RadioButton_Standard_Off, 1, wxALL|wxEXPAND, 5);
    RadioButton_Standard_Chase = new wxRadioButton(PanelStandard, ID_RADIOBUTTON_Standard_Chase, _("Chase"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_RADIOBUTTON_Standard_Chase"));
    FlexGridSizer7->Add(RadioButton_Standard_Chase, 1, wxALL|wxEXPAND, 5);
    RadioButton_Standard_Chase13 = new wxRadioButton(PanelStandard, ID_RADIOBUTTON_Standard_Chase13, _("Chase 1/3"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_RADIOBUTTON_Standard_Chase13"));
    FlexGridSizer7->Add(RadioButton_Standard_Chase13, 1, wxALL|wxEXPAND, 5);
    RadioButton_Standard_Chase14 = new wxRadioButton(PanelStandard, ID_RADIOBUTTON_Standard_Chase14, _("Chase 1/4"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_RADIOBUTTON_Standard_Chase14"));
    FlexGridSizer7->Add(RadioButton_Standard_Chase14, 1, wxALL|wxEXPAND, 5);
    RadioButton_Standard_Chase15 = new wxRadioButton(PanelStandard, ID_RADIOBUTTON_Standard_Chase15, _("Chase 1/5"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_RADIOBUTTON_Standard_Chase15"));
    FlexGridSizer7->Add(RadioButton_Standard_Chase15, 1, wxALL|wxEXPAND, 5);
    RadioButton_Standard_Alternate = new wxRadioButton(PanelStandard, ID_RADIOBUTTON_Standard_Alternate, _("Alternate"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_RADIOBUTTON_Standard_Alternate"));
    FlexGridSizer7->Add(RadioButton_Standard_Alternate, 1, wxALL|wxEXPAND, 5);
    RadioButton_Standard_Twinkle5 = new wxRadioButton(PanelStandard, ID_RADIOBUTTON_Standard_Twinke5, _("Twinkle 5%"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_RADIOBUTTON_Standard_Twinke5"));
    FlexGridSizer7->Add(RadioButton_Standard_Twinkle5, 1, wxALL|wxEXPAND, 5);
    RadioButton_Standard_Twinkle10 = new wxRadioButton(PanelStandard, ID_RADIOBUTTON_Standard_Twinkle10, _("Twinkle 10%"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_RADIOBUTTON_Standard_Twinkle10"));
    FlexGridSizer7->Add(RadioButton_Standard_Twinkle10, 1, wxALL|wxEXPAND, 5);
    RadioButton_Standard_Twinkle25 = new wxRadioButton(PanelStandard, ID_RADIOBUTTON_Standard_Twinkle25, _("Twinkle 25%"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_RADIOBUTTON_Standard_Twinkle25"));
    FlexGridSizer7->Add(RadioButton_Standard_Twinkle25, 1, wxALL|wxEXPAND, 5);
    RadioButton_Standard_Twinkle50 = new wxRadioButton(PanelStandard, ID_RADIOBUTTON_Standard_Twinkle50, _("Twinkle 50%"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_RADIOBUTTON_Standard_Twinkle50"));
    FlexGridSizer7->Add(RadioButton_Standard_Twinkle50, 1, wxALL|wxEXPAND, 5);
    RadioButton_Standard_Shimmer = new wxRadioButton(PanelStandard, ID_RADIOBUTTON_Standard_Shimmer, _("Shimmer"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_RADIOBUTTON_Standard_Shimmer"));
    FlexGridSizer7->Add(RadioButton_Standard_Shimmer, 1, wxALL|wxEXPAND, 5);
    RadioButton_Standard_Background = new wxRadioButton(PanelStandard, ID_RADIOBUTTON_Standard_Background, _("Background Only"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_RADIOBUTTON_Standard_Background"));
    FlexGridSizer7->Add(RadioButton_Standard_Background, 1, wxALL|wxEXPAND, 5);
    FlexGridSizer6->Add(FlexGridSizer7, 1, wxALL|wxEXPAND, 2);
    FlexGridSizer6->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
    BoxSizer1 = new wxBoxSizer(wxHORIZONTAL);
    FlexGridSizer8 = new wxFlexGridSizer(2, 1, 0, 0);
    FlexGridSizer8->AddGrowableCol(0);
    FlexGridSizer8->AddGrowableRow(1);
    StaticText3 = new wxStaticText(PanelStandard, ID_STATICTEXT3, _("Background\nIntensity"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT3"));
    FlexGridSizer8->Add(StaticText3, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
    Slider_Standard_Background = new wxSlider(PanelStandard, ID_SLIDER_Standard_Background, 0, 0, 255, wxDefaultPosition, wxDefaultSize, wxSL_VERTICAL|wxSL_LABELS|wxSL_INVERSE|wxBORDER_SIMPLE, wxDefaultValidator, _T("ID_SLIDER_Standard_Background"));
    FlexGridSizer8->Add(Slider_Standard_Background, 1, wxALL|wxEXPAND, 5);
    BoxSizer1->Add(FlexGridSizer8, 1, wxALL|wxEXPAND, 5);
    FlexGridSizer9 = new wxFlexGridSizer(2, 1, 0, 0);
    FlexGridSizer9->AddGrowableCol(0);
    FlexGridSizer9->AddGrowableRow(1);
    StaticText4 = new wxStaticText(PanelStandard, ID_STATICTEXT4, _("Highlight\nIntensity"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT4"));
    FlexGridSizer9->Add(StaticText4, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
    Slider_Standard_Highlight = new wxSlider(PanelStandard, ID_SLIDER_Standard_Highlight, 255, 0, 255, wxDefaultPosition, wxDefaultSize, wxSL_VERTICAL|wxSL_LABELS|wxSL_INVERSE|wxBORDER_SIMPLE, wxDefaultValidator, _T("ID_SLIDER_Standard_Highlight"));
    FlexGridSizer9->Add(Slider_Standard_Highlight, 1, wxALL|wxEXPAND, 5);
    BoxSizer1->Add(FlexGridSizer9, 1, wxALL|wxEXPAND, 5);
    FlexGridSizer6->Add(BoxSizer1, 1, wxALL|wxEXPAND, 2);
    FlexGridSizer6->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    PanelStandard->SetSizer(FlexGridSizer6);
    FlexGridSizer6->Fit(PanelStandard);
    FlexGridSizer6->SetSizeHints(PanelStandard);
    PanelRGB = new wxPanel(Notebook2, ID_PANEL9, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("ID_PANEL9"));
    FlexGridSizer10 = new wxFlexGridSizer(1, 4, 0, 0);
    FlexGridSizer10->AddGrowableCol(3);
    FlexGridSizer10->AddGrowableRow(0);
    FlexGridSizer11 = new wxFlexGridSizer(0, 1, 0, 0);
    FlexGridSizer11->AddGrowableCol(0);
    StaticText5 = new wxStaticText(PanelRGB, ID_STATICTEXT5, _("Function"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT5"));
    FlexGridSizer11->Add(StaticText5, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
    RadioButton_RGB_Off = new wxRadioButton(PanelRGB, ID_RADIOBUTTON_RGB_Off, _("Off"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_RADIOBUTTON_RGB_Off"));
    FlexGridSizer11->Add(RadioButton_RGB_Off, 1, wxALL|wxEXPAND, 5);
    RadioButton_RGB_Chase = new wxRadioButton(PanelRGB, ID_RADIOBUTTON_RGB_Chase, _("Chase"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_RADIOBUTTON_RGB_Chase"));
    FlexGridSizer11->Add(RadioButton_RGB_Chase, 1, wxALL|wxEXPAND, 5);
    RadioButton_RGB_Chase13 = new wxRadioButton(PanelRGB, ID_RADIOBUTTON_RGB_Chase13, _("Chase 1/3"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_RADIOBUTTON_RGB_Chase13"));
    FlexGridSizer11->Add(RadioButton_RGB_Chase13, 1, wxALL|wxEXPAND, 5);
    RadioButton_RGB_Chase14 = new wxRadioButton(PanelRGB, ID_RADIOBUTTON_RGB_Chase14, _("Chase 1/4"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_RADIOBUTTON_RGB_Chase14"));
    FlexGridSizer11->Add(RadioButton_RGB_Chase14, 1, wxALL|wxEXPAND, 5);
    RadioButton_RGB_Chase15 = new wxRadioButton(PanelRGB, ID_RADIOBUTTON_RGB_Chase15, _("Chase 1/5"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_RADIOBUTTON_RGB_Chase15"));
    FlexGridSizer11->Add(RadioButton_RGB_Chase15, 1, wxALL|wxEXPAND, 5);
    RadioButton_RGB_Alternate = new wxRadioButton(PanelRGB, ID_RADIOBUTTON_RGB_Alternate, _("Alternate"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_RADIOBUTTON_RGB_Alternate"));
    FlexGridSizer11->Add(RadioButton_RGB_Alternate, 1, wxALL|wxEXPAND, 5);
    RadioButton_RGB_Twinkle5 = new wxRadioButton(PanelRGB, ID_RADIOBUTTON_RGB_Twinkle5, _("Twinkle 5%"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_RADIOBUTTON_RGB_Twinkle5"));
    FlexGridSizer11->Add(RadioButton_RGB_Twinkle5, 1, wxALL|wxEXPAND, 5);
    RadioButton_RGB_Twinkle10 = new wxRadioButton(PanelRGB, ID_RADIOBUTTON_RGB_Twinkle10, _("Twinkle 10%"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_RADIOBUTTON_RGB_Twinkle10"));
    FlexGridSizer11->Add(RadioButton_RGB_Twinkle10, 1, wxALL|wxEXPAND, 5);
    RadioButton_RGB_Twinkle25 = new wxRadioButton(PanelRGB, ID_RADIOBUTTON_RGB_Twinkle25, _("Twinkle 25%"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_RADIOBUTTON_RGB_Twinkle25"));
    FlexGridSizer11->Add(RadioButton_RGB_Twinkle25, 1, wxALL|wxEXPAND, 5);
    RadioButton_RGB_Twinkle50 = new wxRadioButton(PanelRGB, ID_RADIOBUTTON_RGB_Twinkle50, _("Twinkle 50%"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_RADIOBUTTON_RGB_Twinkle50"));
    FlexGridSizer11->Add(RadioButton_RGB_Twinkle50, 1, wxALL|wxEXPAND, 5);
    RadioButton_RGB_Shimmer = new wxRadioButton(PanelRGB, ID_RADIOBUTTON_RGB_Shimmer, _("Shimmer"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_RADIOBUTTON_RGB_Shimmer"));
    FlexGridSizer11->Add(RadioButton_RGB_Shimmer, 1, wxALL|wxEXPAND, 5);
    RadioButton_RGB_Background = new wxRadioButton(PanelRGB, ID_RADIOBUTTON_RGB_Background, _("Background Only"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_RADIOBUTTON_RGB_Background"));
    FlexGridSizer11->Add(RadioButton_RGB_Background, 1, wxALL|wxEXPAND, 5);
    FlexGridSizer10->Add(FlexGridSizer11, 1, wxALL|wxEXPAND, 2);
    FlexGridSizer10->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    BoxSizer2 = new wxBoxSizer(wxVERTICAL);
    BoxSizer3 = new wxBoxSizer(wxHORIZONTAL);
    StaticBoxSizer1 = new wxStaticBoxSizer(wxHORIZONTAL, PanelRGB, _("Background Color"));
    Slider_RGB_BG_R = new wxSlider(PanelRGB, ID_SLIDER1, 0, 0, 255, wxDefaultPosition, wxDefaultSize, wxSL_VERTICAL|wxSL_LABELS|wxSL_INVERSE, wxDefaultValidator, _T("ID_SLIDER1"));
    StaticBoxSizer1->Add(Slider_RGB_BG_R, 1, wxALL|wxEXPAND, 5);
    Slider_RGB_BG_G = new wxSlider(PanelRGB, ID_SLIDER2, 0, 0, 255, wxDefaultPosition, wxDefaultSize, wxSL_VERTICAL|wxSL_LABELS|wxSL_INVERSE, wxDefaultValidator, _T("ID_SLIDER2"));
    StaticBoxSizer1->Add(Slider_RGB_BG_G, 1, wxALL|wxEXPAND, 5);
    Slider_RGB_BG_B = new wxSlider(PanelRGB, ID_SLIDER3, 0, 0, 255, wxDefaultPosition, wxDefaultSize, wxSL_VERTICAL|wxSL_LABELS|wxSL_INVERSE, wxDefaultValidator, _T("ID_SLIDER3"));
    StaticBoxSizer1->Add(Slider_RGB_BG_B, 1, wxALL|wxEXPAND, 5);
    BoxSizer3->Add(StaticBoxSizer1, 1, wxALL|wxEXPAND, 5);
    BoxSizer2->Add(BoxSizer3, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    BoxSizer4 = new wxBoxSizer(wxHORIZONTAL);
    StaticBoxSizer2 = new wxStaticBoxSizer(wxHORIZONTAL, PanelRGB, _("Highlight Color"));
    Slider_RGB_H_R = new wxSlider(PanelRGB, ID_SLIDER4, 255, 0, 255, wxDefaultPosition, wxDefaultSize, wxSL_VERTICAL|wxSL_LABELS|wxSL_INVERSE, wxDefaultValidator, _T("ID_SLIDER4"));
    StaticBoxSizer2->Add(Slider_RGB_H_R, 1, wxALL|wxEXPAND, 5);
    Slider_RGB_H_G = new wxSlider(PanelRGB, ID_SLIDER5, 255, 0, 255, wxDefaultPosition, wxDefaultSize, wxSL_VERTICAL|wxSL_LABELS|wxSL_INVERSE, wxDefaultValidator, _T("ID_SLIDER5"));
    StaticBoxSizer2->Add(Slider_RGB_H_G, 1, wxALL|wxEXPAND, 5);
    Slider_RGB_H_B = new wxSlider(PanelRGB, ID_SLIDER6, 255, 0, 255, wxDefaultPosition, wxDefaultSize, wxSL_VERTICAL|wxSL_LABELS|wxSL_INVERSE, wxDefaultValidator, _T("ID_SLIDER6"));
    StaticBoxSizer2->Add(Slider_RGB_H_B, 1, wxALL|wxEXPAND, 5);
    BoxSizer4->Add(StaticBoxSizer2, 1, wxALL|wxEXPAND, 5);
    BoxSizer2->Add(BoxSizer4, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizer10->Add(BoxSizer2, 1, wxALL|wxEXPAND, 5);
    PanelRGB->SetSizer(FlexGridSizer10);
    FlexGridSizer10->Fit(PanelRGB);
    FlexGridSizer10->SetSizeHints(PanelRGB);
    PanelRGBCycle = new wxPanel(Notebook2, ID_PANEL10, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("ID_PANEL10"));
    FlexGridSizer12 = new wxFlexGridSizer(0, 2, 0, 0);
    FlexGridSizer12->AddGrowableRow(0);
    FlexGridSizer13 = new wxFlexGridSizer(0, 1, 0, 0);
    StaticText6 = new wxStaticText(PanelRGBCycle, ID_STATICTEXT6, _("Function"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT6"));
    FlexGridSizer13->Add(StaticText6, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    RadioButton_RGBCycle_Off = new wxRadioButton(PanelRGBCycle, ID_RADIOBUTTON_RGBCycle_Off, _("Off"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_RADIOBUTTON_RGBCycle_Off"));
    FlexGridSizer13->Add(RadioButton_RGBCycle_Off, 1, wxALL|wxEXPAND, 5);
    RadioButton_RGBCycle_ABC = new wxRadioButton(PanelRGBCycle, ID_RADIOBUTTON_RGBCycle_ABC, _("A-B-C"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_RADIOBUTTON_RGBCycle_ABC"));
    FlexGridSizer13->Add(RadioButton_RGBCycle_ABC, 1, wxALL|wxEXPAND, 5);
    RadioButton_RGBCycle_ABCAll = new wxRadioButton(PanelRGBCycle, ID_RADIOBUTTON_RGBCycle_ABCAll, _("A-B-C-All"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_RADIOBUTTON_RGBCycle_ABCAll"));
    FlexGridSizer13->Add(RadioButton_RGBCycle_ABCAll, 1, wxALL|wxEXPAND, 5);
    RadioButton_RGBCycle_ABCAllNone = new wxRadioButton(PanelRGBCycle, ID_RADIOBUTTON_RGBCycle_ABCAllNone, _("A-B-C-All-None"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_RADIOBUTTON_RGBCycle_ABCAllNone"));
    FlexGridSizer13->Add(RadioButton_RGBCycle_ABCAllNone, 1, wxALL|wxEXPAND, 5);
    RadioButton_RGBCycle_MixedColors = new wxRadioButton(PanelRGBCycle, ID_RADIOBUTTON_RGBCycle_MixedColors, _("Mixed Colors"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_RADIOBUTTON_RGBCycle_MixedColors"));
    FlexGridSizer13->Add(RadioButton_RGBCycle_MixedColors, 1, wxALL|wxEXPAND, 5);
    RadioButton_RGBCycle_RGBW = new wxRadioButton(PanelRGBCycle, ID_RADIOBUTTON_RGBCycle_RGBW, _("R-G-B-W"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_RADIOBUTTON_RGBCycle_RGBW"));
    FlexGridSizer13->Add(RadioButton_RGBCycle_RGBW, 1, wxALL|wxEXPAND, 5);
    CheckBox_Tag50th = new wxCheckBox(PanelRGBCycle, ID_CHECKBOX2, _("Tag every 50th node white @ 50%"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX2"));
    CheckBox_Tag50th->SetValue(false);
    CheckBox_Tag50th->SetToolTip(_("This is really only useful if you are testing a single model"));
    FlexGridSizer13->Add(CheckBox_Tag50th, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizer12->Add(FlexGridSizer13, 1, wxALL|wxEXPAND, 5);
    PanelRGBCycle->SetSizer(FlexGridSizer12);
    FlexGridSizer12->Fit(PanelRGBCycle);
    FlexGridSizer12->SetSizeHints(PanelRGBCycle);
    Notebook2->AddPage(PanelStandard, _("Standard"), false);
    Notebook2->AddPage(PanelRGB, _("RGB"), false);
    Notebook2->AddPage(PanelRGBCycle, _("RGB Cycle"), false);
    FlexGridSizer3->Add(Notebook2, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizer5 = new wxFlexGridSizer(0, 2, 0, 0);
    FlexGridSizer5->AddGrowableCol(1);
    StaticText1 = new wxStaticText(Panel2, ID_STATICTEXT1, _("Speed"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT1"));
    FlexGridSizer5->Add(StaticText1, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    Slider_Speed = new wxSlider(Panel2, ID_SLIDER_Speed, 50, 0, 100, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_Speed"));
    FlexGridSizer5->Add(Slider_Speed, 1, wxALL|wxEXPAND, 2);
    FlexGridSizer3->Add(FlexGridSizer5, 1, wxALL|wxEXPAND, 5);
    Panel2->SetSizer(FlexGridSizer3);
    FlexGridSizer3->Fit(Panel2);
    FlexGridSizer3->SetSizeHints(Panel2);
    SplitterWindow1->SplitVertically(Panel1, Panel2);
    FlexGridSizer1->Add(SplitterWindow1, 1, wxALL|wxEXPAND, 2);
    StatusBar1 = new wxStaticText(this, ID_STATICTEXT7, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxBORDER_DOUBLE, _T("ID_STATICTEXT7"));
    FlexGridSizer1->Add(StatusBar1, 1, wxALL|wxEXPAND, 2);
    SetSizer(FlexGridSizer1);
    Timer1.SetOwner(this, ID_TIMER1);
    FlexGridSizer1->Fit(this);
    FlexGridSizer1->SetSizeHints(this);

    Connect(ID_BUTTON_Load,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&PixelTestDialog::OnButton_LoadClick);
    Connect(ID_BUTTON_Save,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&PixelTestDialog::OnButton_SaveClick);
    Connect(ID_CHOICE1,wxEVT_COMMAND_CHOICE_SELECTED,(wxObjectEventFunction)&PixelTestDialog::OnChoice_VisualModelSelect);
    Connect(ID_NOTEBOOK1,wxEVT_COMMAND_NOTEBOOK_PAGE_CHANGED,(wxObjectEventFunction)&PixelTestDialog::OnNotebook1PageChanged);
    Connect(ID_CHECKBOX_OutputToLights,wxEVT_COMMAND_CHECKBOX_CLICKED,(wxObjectEventFunction)&PixelTestDialog::OnCheckBox_OutputToLightsClick);
    Connect(ID_CHECKBOX1,wxEVT_COMMAND_CHECKBOX_CLICKED,(wxObjectEventFunction)&PixelTestDialog::OnCheckBox_SuppressUnusedOutputsClick);
    Connect(ID_CHECKBOX2,wxEVT_COMMAND_CHECKBOX_CLICKED,(wxObjectEventFunction)&PixelTestDialog::OnCheckBox_Tag50thClick);
    Connect(ID_TIMER1,wxEVT_TIMER,(wxObjectEventFunction)&PixelTestDialog::OnTimer1Trigger);
    Connect(wxID_ANY,wxEVT_CLOSE_WINDOW,(wxObjectEventFunction)&PixelTestDialog::OnClose);
    //*)

    SetSize(wxSystemSettings::GetMetric(wxSYS_SCREEN_X) * 3 / 4, wxSystemSettings::GetMetric(wxSYS_SCREEN_Y) * 3 / 4);

    SplitterWindow1->SetMinimumPaneSize(100);

    TreeListCtrl_Outputs = new wxTreeListCtrl(Panel_Outputs, ID_TREELISTCTRL_Outputs, wxPoint(0, 0), Panel_Outputs->GetSize(), wxTR_FULL_ROW_HIGHLIGHT | wxTR_DEFAULT_STYLE | wxTL_CHECKBOX | wxTL_USER_3STATE, _T("ID_TREELISTCTRL_Outputs"));
    FlexGridSizer_Outputs->Add(TreeListCtrl_Outputs, 1, wxALL | wxEXPAND, 5);
    FlexGridSizer_Outputs->AddGrowableRow(1);
    TreeListCtrl_Outputs->AppendColumn(L"Select channels ...", 500);
    FlexGridSizer_Outputs->Layout();
    TreeListCtrl_Models = new wxTreeListCtrl(Panel_Models, ID_TREELISTCTRL_Models, wxPoint(0, 0), Panel_Models->GetSize(), wxTR_FULL_ROW_HIGHLIGHT | wxTR_DEFAULT_STYLE | wxTL_CHECKBOX | wxTL_USER_3STATE, _T("ID_TREELISTCTRL_Models"));
    FlexGridSizer_Models->Add(TreeListCtrl_Models, 1, wxALL | wxEXPAND, 5);
    TreeListCtrl_Models->AppendColumn(L"Select channels ...", 500);
    FlexGridSizer_Models->Layout();
    TreeListCtrl_ModelGroups = new wxTreeListCtrl(Panel_ModelGroups, ID_TREELISTCTRL_ModelGroups, wxPoint(0, 0), Panel_ModelGroups->GetSize(), wxTR_FULL_ROW_HIGHLIGHT | wxTR_DEFAULT_STYLE | wxTL_CHECKBOX | wxTL_USER_3STATE, _T("ID_TREELISTCTRL_ModelGroups"));
    FlexGridSizer_ModelGroups->Add(TreeListCtrl_ModelGroups, 1, wxALL | wxEXPAND, 5);
    TreeListCtrl_ModelGroups->AppendColumn(L"Select channels ...", 500);
    FlexGridSizer_ModelGroups->Layout();
    TreeListCtrl_Controllers = new wxTreeListCtrl(Panel_Controllers, ID_TREELISTCTRL_Controllers, wxPoint(0, 0), Panel_Controllers->GetSize(), wxTR_FULL_ROW_HIGHLIGHT | wxTR_DEFAULT_STYLE | wxTL_CHECKBOX | wxTL_USER_3STATE, _T("ID_TREELISTCTRL_Controllers"));
    FlexGridSizer_Controllers->Add(TreeListCtrl_Controllers, 1, wxALL | wxEXPAND, 5);
    TreeListCtrl_Controllers->AppendColumn(L"Select channels ...", 500);
    FlexGridSizer_Controllers->Layout();

    // add checkbox events
    Connect(ID_TREELISTCTRL_Outputs, wxEVT_COMMAND_CHECKLISTBOX_TOGGLED, (wxObjectEventFunction)&PixelTestDialog::OnTreeListCtrlCheckboxtoggled);
    Connect(ID_TREELISTCTRL_Outputs, wxEVT_COMMAND_TREELIST_ITEM_CHECKED, (wxObjectEventFunction)&PixelTestDialog::OnTreeListCtrlCheckboxtoggled);
    Connect(ID_TREELISTCTRL_Outputs, wxEVT_COMMAND_TREELIST_SELECTION_CHANGED, (wxObjectEventFunction)&PixelTestDialog::OnTreeListCtrlItemSelected);
    Connect(ID_TREELISTCTRL_Outputs, wxEVT_COMMAND_TREELIST_ITEM_EXPANDING, (wxObjectEventFunction)&PixelTestDialog::OnTreeListCtrlItemExpanding);
    Connect(ID_TREELISTCTRL_Outputs, wxEVT_TREELIST_ITEM_CONTEXT_MENU, (wxObjectEventFunction)&PixelTestDialog::OnContextMenu);
    Connect(ID_TREELISTCTRL_ModelGroups, wxEVT_COMMAND_CHECKLISTBOX_TOGGLED, (wxObjectEventFunction)&PixelTestDialog::OnTreeListCtrlCheckboxtoggled);
    Connect(ID_TREELISTCTRL_ModelGroups, wxEVT_COMMAND_TREELIST_ITEM_CHECKED, (wxObjectEventFunction)&PixelTestDialog::OnTreeListCtrlCheckboxtoggled);
    Connect(ID_TREELISTCTRL_ModelGroups, wxEVT_COMMAND_TREELIST_SELECTION_CHANGED, (wxObjectEventFunction)&PixelTestDialog::OnTreeListCtrlItemSelected);
    Connect(ID_TREELISTCTRL_ModelGroups, wxEVT_COMMAND_TREELIST_ITEM_EXPANDING, (wxObjectEventFunction)&PixelTestDialog::OnTreeListCtrlItemExpanding);
    Connect(ID_TREELISTCTRL_ModelGroups, wxEVT_TREELIST_ITEM_CONTEXT_MENU, (wxObjectEventFunction)&PixelTestDialog::OnContextMenu);
    Connect(ID_TREELISTCTRL_Models, wxEVT_COMMAND_CHECKLISTBOX_TOGGLED, (wxObjectEventFunction)&PixelTestDialog::OnTreeListCtrlCheckboxtoggled);
    Connect(ID_TREELISTCTRL_Models, wxEVT_COMMAND_TREELIST_ITEM_CHECKED, (wxObjectEventFunction)&PixelTestDialog::OnTreeListCtrlCheckboxtoggled);
    Connect(ID_TREELISTCTRL_Models, wxEVT_COMMAND_TREELIST_SELECTION_CHANGED, (wxObjectEventFunction)&PixelTestDialog::OnTreeListCtrlItemSelected);
    Connect(ID_TREELISTCTRL_Models, wxEVT_COMMAND_TREELIST_ITEM_EXPANDING, (wxObjectEventFunction)&PixelTestDialog::OnTreeListCtrlItemExpanding);
    Connect(ID_TREELISTCTRL_Models, wxEVT_TREELIST_ITEM_CONTEXT_MENU, (wxObjectEventFunction)&PixelTestDialog::OnContextMenu);
    Connect(ID_TREELISTCTRL_Controllers, wxEVT_COMMAND_CHECKLISTBOX_TOGGLED, (wxObjectEventFunction)&PixelTestDialog::OnTreeListCtrlCheckboxtoggled);
    Connect(ID_TREELISTCTRL_Controllers, wxEVT_COMMAND_TREELIST_ITEM_CHECKED, (wxObjectEventFunction)&PixelTestDialog::OnTreeListCtrlCheckboxtoggled);
    Connect(ID_TREELISTCTRL_Controllers, wxEVT_COMMAND_TREELIST_SELECTION_CHANGED, (wxObjectEventFunction)&PixelTestDialog::OnTreeListCtrlItemSelected);
    Connect(ID_TREELISTCTRL_Controllers, wxEVT_COMMAND_TREELIST_ITEM_EXPANDING, (wxObjectEventFunction)&PixelTestDialog::OnTreeListCtrlItemExpanding);
    Connect(ID_TREELISTCTRL_Controllers, wxEVT_TREELIST_ITEM_CONTEXT_MENU, (wxObjectEventFunction)&PixelTestDialog::OnContextMenu);
#ifdef __WXOSX__
    Connect(ID_TREELISTCTRL_Outputs, wxEVT_COMMAND_TREELIST_ITEM_ACTIVATED, (wxObjectEventFunction)&PixelTestDialog::OnTreeListCtrlItemActivated);
    Connect(ID_TREELISTCTRL_ModelGroups, wxEVT_COMMAND_TREELIST_ITEM_ACTIVATED, (wxObjectEventFunction)&PixelTestDialog::OnTreeListCtrlItemActivated);
    Connect(ID_TREELISTCTRL_Models, wxEVT_COMMAND_TREELIST_ITEM_ACTIVATED, (wxObjectEventFunction)&PixelTestDialog::OnTreeListCtrlItemActivated);
    Connect(ID_TREELISTCTRL_Controllers, wxEVT_COMMAND_TREELIST_ITEM_ACTIVATED, (wxObjectEventFunction)&PixelTestDialog::OnTreeListCtrlItemActivated);
#endif

    PopulateOutputTree(_outputManager);
    PopulateModelTree(_modelManager);
    PopulateVisualModelTree(_modelManager);
    PopulateModelGroupTree(_modelManager);
    PopulateControllerTree(_outputManager, _modelManager);
    DeactivateNotClickableModels(TreeListCtrl_Outputs);
    DeactivateNotClickableModels(TreeListCtrl_Models);
    DeactivateNotClickableModels(TreeListCtrl_ModelGroups);
    DeactivateNotClickableModels(TreeListCtrl_Controllers);

    wxConfigBase* config = wxConfigBase::Get();
    DeserialiseSettings(config->Read("xLightsTestSettings").ToStdString());

    SetSuspend(false);

    _starttime = wxDateTime::UNow();

    CheckBox_OutputToLights->SetValue(true);

    SetSize(1200, 800);
    wxPoint loc;
    wxSize sz;
    LoadWindowPosition("xLightsTestDialogPosition", sz, loc);
    if (loc.x != -1) {
        if (sz.GetWidth() < 400)
            sz.SetWidth(400);
        if (sz.GetHeight() < 300)
            sz.SetHeight(300);
        SetPosition(loc);
        SetSize(sz);
        Layout();
    }
    EnsureWindowHeaderIsOnScreen(this);

    if (_outputManager->IsOutputOpenInAnotherProcess()) {
        DisplayWarning("Another process seems to be outputting to lights right now. This may not generate the result expected.", this);
    }

    if (!parent->ForceEnableOutputs(false)) {
        DisplayWarning("At least one output could not be started. See log file for details.", this);
    }
    Timer1.Start(50, wxTIMER_CONTINUOUS);
}

// Destructor

PixelTestDialog::~PixelTestDialog()
{
    SetSuspend(false);

    // need to delete all the TreeController Objects
    wxTreeListItem root = TreeListCtrl_Outputs->GetRootItem();
    DestroyTreeControllerData(TreeListCtrl_Outputs, root);
    root = TreeListCtrl_ModelGroups->GetRootItem();
    DestroyTreeControllerData(TreeListCtrl_ModelGroups, root);
    root = TreeListCtrl_Models->GetRootItem();
    DestroyTreeControllerData(TreeListCtrl_Models, root);
    root = TreeListCtrl_Controllers->GetRootItem();
    DestroyTreeControllerData(TreeListCtrl_Controllers, root);

    // need to delete the TreeController.
    Panel_Outputs->RemoveChild(TreeListCtrl_Outputs);
    Panel_Models->RemoveChild(TreeListCtrl_Models);
    Panel_ModelGroups->RemoveChild(TreeListCtrl_ModelGroups);
    Panel_Controllers->RemoveChild(TreeListCtrl_Controllers);

    SaveWindowPosition("xLightsTestDialogPosition", this);

    //(*Destroy(PixelTestDialog)
    //*)
}
#pragma endregion

void PixelTestDialog::DumpSelected()
{
    _channelTracker.Dump();
}

bool PixelTestDialog::AreChannelsAvailable(ModelGroup* modelGroup)
{
    for (const auto& it : modelGroup->Models()) {
        if (!AreChannelsAvailable(it)) {
            return false;
        }
    }

    return true;
}

bool PixelTestDialog::AreChannelsAvailable(Model* model)
{
    int32_t sc = model->GetFirstChannel() + 1;
    int32_t ec = model->GetLastChannel() + 1;
    int32_t current = sc;

    while (current <= ec) {
        int32_t offset;
        auto c = _outputManager->GetController(current, offset);

        if (c == nullptr || c->GetType() == "NULL")
            return false;
        current += c->GetChannels();
    }

    return true;
}

std::list<std::string> PixelTestDialog::GetModelsOnChannels(int start, int end)
{
    std::list<std::string> res;

    for (const auto& it : *_modelManager) {
        Model* m = it.second;
        if (m->GetDisplayAs() != "ModelGroup") {
            int st = m->GetFirstChannel() + 1;
            int en = m->GetLastChannel() + 1;
            if (start <= en && end >= st) {
                res.push_back(it.first);
            }
        }
    }

    return res;
}

// Populate the tree functions

#pragma region OutputTab
void PixelTestDialog::AddOutput(wxTreeListItem root, Output* output)
{
    OutputTestItem* oti = new OutputTestItem(output);

    wxTreeListItem c = TreeListCtrl_Outputs->AppendItem(root, oti->GetName(), -1, -1, (wxClientData*)oti);
    oti->SetTreeListItem(c);
    if (oti->IsClickable()) {
        TreeListCtrl_Outputs->AppendItem(c, "Dummy");
    }
}

wxTreeListItem PixelTestDialog::AddController(wxTreeListItem root, Controller* controller)
{
    ControllerTestItem* cti = new ControllerTestItem(controller);

    wxTreeListItem c = TreeListCtrl_Outputs->AppendItem(root, cti->GetName(), -1, -1, (wxClientData*)cti);
    cti->SetTreeListItem(c);
    // if (cti->IsClickable())
    // {
    //     TreeListCtrl_Outputs->AppendItem(c, "Dummy");
    // }
    return c;
}

void PixelTestDialog::PopulateOutputTree(OutputManager* outputManager)
{
    OutputRootTestItem* root = new OutputRootTestItem(outputManager->GetTotalChannels());
    wxTreeListItem r = TreeListCtrl_Outputs->AppendItem(TreeListCtrl_Outputs->GetRootItem(), root->GetName(), -1, -1, (wxClientData*)root);
    root->SetTreeListItem(r);

    for (const auto& c : outputManager->GetControllers()) {
        auto cti = AddController(r, c);
        for (const auto& o : c->GetOutputs()) {
            AddOutput(cti, o);
        }
        TreeListCtrl_Outputs->Expand(cti);
    }

    TreeListCtrl_Outputs->Expand(r);
}
#pragma endregion

#pragma region ModelGroupTab
void PixelTestDialog::AddModelGroup(wxTreeListItem parent, Model* m)
{
    ModelGroupTestItem* modelgroupcontroller = new ModelGroupTestItem(m->GetName(), *_modelManager, AreChannelsAvailable(m));
    wxTreeListItem modelgroupitem = TreeListCtrl_ModelGroups->AppendItem(parent, modelgroupcontroller->GetName(), -1, -1, (wxClientData*)modelgroupcontroller);
    modelgroupcontroller->SetTreeListItem(modelgroupitem);

    if (modelgroupcontroller->IsClickable()) {
        auto models = modelgroupcontroller->GetModels();
        for (const auto& it2 : models) {
            wxTreeListItem modelitem = TreeListCtrl_ModelGroups->AppendItem(modelgroupitem, it2->GetName(), -1, -1, (wxClientData*)it2);
            it2->SetTreeListItem(modelitem);
        }

        auto submodels = modelgroupcontroller->GetSubModels();
        for (const auto& it2 : submodels) {
            wxTreeListItem submodelitem = TreeListCtrl_ModelGroups->AppendItem(modelgroupitem, it2->GetName(), -1, -1, (wxClientData*)it2);
            it2->SetTreeListItem(submodelitem);
        }

        auto groups = modelgroupcontroller->GetModelGroups();
        for (const auto& it : groups) {
            AddModelGroup(modelgroupitem, it);
        }
    }
}

void PixelTestDialog::AddModelGroup(wxTreeListItem parent, ModelGroupTestItem* mgti)
{
    wxTreeListItem modelgroupitem = TreeListCtrl_ModelGroups->AppendItem(parent, mgti->GetName(), -1, -1, (wxClientData*)mgti);
    mgti->SetTreeListItem(modelgroupitem);

    if (mgti->IsClickable()) {
        auto models = mgti->GetModels();
        for (const auto& it2 : models) {
            wxTreeListItem modelitem = TreeListCtrl_ModelGroups->AppendItem(modelgroupitem, it2->GetName(), -1, -1, (wxClientData*)it2);
            it2->SetTreeListItem(modelitem);
        }

        auto submodels = mgti->GetSubModels();
        for (const auto& it2 : submodels) {
            wxTreeListItem submodelitem = TreeListCtrl_ModelGroups->AppendItem(modelgroupitem, it2->GetName(), -1, -1, (wxClientData*)it2);
            it2->SetTreeListItem(submodelitem);
        }

        auto groups = mgti->GetModelGroups();
        for (const auto& it : groups) {
            AddModelGroup(modelgroupitem, it);
        }
    }
}

void PixelTestDialog::PopulateModelGroupTree(ModelManager* modelManager)
{
    for (const auto& it : *_modelManager) {
        Model* m = it.second;

        if (m->GetDisplayAs() == "ModelGroup") {
            // we found a model group
            AddModelGroup(TreeListCtrl_ModelGroups->GetRootItem(), m);
        }
    }
}
#pragma endregion

#pragma region ControllerTab
void PixelTestDialog::PopulateControllerTree(OutputManager* outputManager, ModelManager* modelManager)
{
    std::list<std::string> controllerNames;
    for (const auto& it : outputManager->GetControllers()) {
        controllerNames.push_back(it->GetName());
    }
    controllerNames.sort(stdlistNumberAwareStringCompare);

    for (const auto& it : controllerNames) {
        Controller* c = outputManager->GetController(it);
        auto caps = c->GetControllerCaps();
        auto cud = new UDController(c, outputManager, modelManager, false);
        if (cud->IsValid()) {
            // we found a controller
            CPR_ControllerTestItem* cti = new CPR_ControllerTestItem(it, caps, cud, *outputManager, *modelManager);
            wxTreeListItem item = TreeListCtrl_Controllers->AppendItem(TreeListCtrl_Controllers->GetRootItem(), cti->GetName(), -1, -1, (wxClientData*)cti);
            cti->SetTreeListItem(item);

            if (cti->GetPorts().size() > 0) {
                for (const auto& it : cti->GetPorts()) {
                    wxTreeListItem portitem = TreeListCtrl_Controllers->AppendItem(item, it->GetName(), -1, -1, (wxClientData*)it);
                    it->SetTreeListItem(portitem);
                    if (it->GetRemotes().size() > 0) {
                        if (it->GetRemotes().size() == 1 && it->GetRemotes().front()->GetLetter() == ' ') {
                            auto models = it->GetRemotes().front()->GetModels();
                            for (const auto& it2 : models) {
                                wxTreeListItem modelitem = TreeListCtrl_Controllers->AppendItem(portitem, it2->GetName(), -1, -1, (wxClientData*)it2);
                                it2->SetTreeListItem(modelitem);
                                if (it2->GetChannelCount() > 0) {
                                    TreeListCtrl_Controllers->AppendItem(modelitem, "Dummy");
                                }
                            }
                        } else {
                            for (const auto& it2 : it->GetRemotes()) {
                                wxTreeListItem sritem = TreeListCtrl_Controllers->AppendItem(portitem, it2->GetName(), -1, -1, (wxClientData*)it2);
                                it2->SetTreeListItem(sritem);
                                auto models = it2->GetModels();
                                for (const auto& it3 : models) {
                                    wxTreeListItem modelitem = TreeListCtrl_Controllers->AppendItem(sritem, it3->GetName(), -1, -1, (wxClientData*)it3);
                                    it3->SetTreeListItem(modelitem);
                                    if (it3->GetChannelCount() > 0) {
                                        TreeListCtrl_Controllers->AppendItem(modelitem, "Dummy");
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}
#pragma endregion

#pragma region VisualModelTab
void PixelTestDialog::PopulateVisualModelTree(ModelManager* modelManager)
{
    _modelPreview = new ModelPreview(Panel_VisualModel);
    _modelPreview->SetMinSize(wxSize(150, 150));
    FlexGridSizer_VisualModelSizer->Add(_modelPreview, 1, wxALL | wxEXPAND, 0);
    FlexGridSizer_VisualModelSizer->Fit(Panel_VisualModel);
    FlexGridSizer_VisualModelSizer->SetSizeHints(Panel_VisualModel);

    _modelPreview->Connect(wxEVT_LEFT_DOWN, (wxObjectEventFunction)&PixelTestDialog::OnPreviewLeftDown, nullptr, this);
    _modelPreview->Connect(wxEVT_LEFT_UP, (wxObjectEventFunction)&PixelTestDialog::OnPreviewLeftUp, nullptr, this);
    _modelPreview->Connect(wxEVT_MOTION, (wxObjectEventFunction)&PixelTestDialog::OnPreviewMouseMove, nullptr, this);
    _modelPreview->Connect(wxEVT_LEAVE_WINDOW, (wxObjectEventFunction)&PixelTestDialog::OnPreviewMouseLeave, nullptr, this);
    _modelPreview->Connect(wxEVT_LEFT_DCLICK, (wxObjectEventFunction)&PixelTestDialog::OnPreviewLeftDClick, nullptr, this);

    std::list<std::string> modelNames;
    for (const auto& it : *_modelManager) {
        Model* m = it.second;

        if (m->GetDisplayAs() != "ModelGroup") {
            modelNames.push_back(m->GetName());
        }
    }
    modelNames.sort(stdlistNumberAwareStringCompare);

    Choice_VisualModel->Clear();
    for (const auto& it : modelNames) {
        Choice_VisualModel->AppendString(it);
    }
    if (Choice_VisualModel->GetCount() > 0) {
        Choice_VisualModel->SetSelection(0);
        SelectVisualModel(modelNames.front());
    }
}

void PixelTestDialog::SelectVisualModel(const std::string& model)
{
    Model* m = _modelManager->GetModel(model);
    _modelPreview->SetModel(m);

    UpdateVisualModelFromTracker();
    RenderModel();
}

void PixelTestDialog::UpdateVisualModelFromTracker()
{
    Model* m = _modelManager->GetModel(Choice_VisualModel->GetStringSelection());

    if (m != nullptr) {
        xlColor c(xlDARK_GREY);
        xlColor cc(xlWHITE);
        int nn = m->GetNodeCount();
        for (int node = 0; node < nn; node++) {
            auto n = m->GetNode(node);
            bool on = false;
            if (n != nullptr) {
                for (uint8_t c = 0; c < m->GetChanCountPerNode() && !on; ++c) {
                    on = on || _channelTracker.IsChannelOn(n->ActChan + c + 1);
                }
            }
            if (on) {
                m->SetNodeColor(node, cc);
            } else {
                m->SetNodeColor(node, c);
            }
        }
    }
}

void PixelTestDialog::OnPreviewLeftUp(wxMouseEvent& event)
{
    if (m_creating_bound_rect) {
        glm::vec3 ray_origin;
        glm::vec3 ray_direction;
        GetMouseLocation(event.GetX(), event.GetY(), ray_origin, ray_direction);
        m_bound_end_x = ray_origin.x;
        m_bound_end_y = ray_origin.y;

        m_creating_bound_rect = false;
        SelectAllInBoundingRect(event.ShiftDown());

        _modelPreview->ReleaseMouse();
    }
}

void PixelTestDialog::OnPreviewMouseLeave(wxMouseEvent& event)
{
    RenderModel();
}

void PixelTestDialog::OnPreviewLeftDown(wxMouseEvent& event)
{
    m_creating_bound_rect = true;
    glm::vec3 ray_origin;
    glm::vec3 ray_direction;
    GetMouseLocation(event.GetX(), event.GetY(), ray_origin, ray_direction);
    m_bound_start_x = ray_origin.x;
    m_bound_start_y = ray_origin.y;
    m_bound_end_x = m_bound_start_x;
    m_bound_end_y = m_bound_start_y;

    // Capture the mouse; this will keep it selecting even if the
    //  user temporarily leaves the preview area...
    _modelPreview->CaptureMouse();
}

void PixelTestDialog::OnPreviewLeftDClick(wxMouseEvent& event)
{
    Model* model = _modelManager->GetModel(Choice_VisualModel->GetStringSelection());
    if (model != nullptr) {
        glm::vec3 ray_origin;
        glm::vec3 ray_direction;
        GetMouseLocation(event.GetX(), event.GetY(), ray_origin, ray_direction);
        int x = ray_origin.x;
        int y = ray_origin.y;

        auto stNode = model->GetNodeNear(_modelPreview, wxPoint(x, y), false);
        if (stNode.IsEmpty())
            return;

        auto node = wxAtoi(stNode) - 1;
        auto n = model->GetNode(node);

        if (n != nullptr) {
            bool on = false;
            for (uint8_t c = 0; c < model->GetChanCountPerNode() && !on; ++c) {
                on = on || _channelTracker.IsChannelOn(n->ActChan + c + 1);
            }

            if (on) {
                _channelTracker.RemoveRange(n->ActChan + 1, n->ActChan + model->GetChanCountPerNode());
            }
            else {
                _channelTracker.AddRange(n->ActChan + 1, n->ActChan + model->GetChanCountPerNode());
            }
            UpdateVisualModelFromTracker();
            RenderModel();
            _checkChannelList = true;
        }
    }
}

void PixelTestDialog::OnPreviewMouseMove(wxMouseEvent& event)
{
    event.ResumePropagation(1);
    event.Skip();
    if (m_creating_bound_rect) {
        glm::vec3 ray_origin;
        glm::vec3 ray_direction;
        GetMouseLocation(event.GetX(), event.GetY(), ray_origin, ray_direction);
        m_bound_end_x = ray_origin.x;
        m_bound_end_y = ray_origin.y;
        RenderModel();
    }
}

void PixelTestDialog::RenderModel()
{
    if (_modelPreview == nullptr || !_modelPreview->StartDrawing(mPointSize))
        return;

    Model* model = _modelManager->GetModel(Choice_VisualModel->GetStringSelection());
    if (model != nullptr) {
        if (m_creating_bound_rect) {
            _modelPreview->AddBoundingBoxToAccumulator(m_bound_start_x, m_bound_start_y, m_bound_end_x, m_bound_end_y);
        }
        model->DisplayEffectOnWindow(_modelPreview, mPointSize);
        _modelPreview->EndDrawing();
    }
}

void PixelTestDialog::GetMouseLocation(int x, int y, glm::vec3& ray_origin, glm::vec3& ray_direction)
{
    // Trim the mouse location to the preview area
    //   (It can go outside this area if the button is down and the mouse
    //    has been captured.)
    x = std::max(x, 0);
    y = std::max(y, 0);
    x = std::min(x, _modelPreview->getWidth());
    y = std::min(y, _modelPreview->getHeight());

    VectorMath::ScreenPosToWorldRay(
        x, _modelPreview->getHeight() - y,
        _modelPreview->getWidth(), _modelPreview->getHeight(),
        _modelPreview->GetProjViewMatrix(),
        ray_origin,
        ray_direction);
}

void PixelTestDialog::SelectAllInBoundingRect(bool shiftDwn)
{
    Model* model = _modelManager->GetModel(Choice_VisualModel->GetStringSelection());
    if (model != nullptr) {
        std::vector<wxRealPoint> pts;
        std::vector<int> nodes = model->GetNodesInBoundingBox(_modelPreview, wxPoint(m_bound_start_x, m_bound_start_y), wxPoint(m_bound_end_x, m_bound_end_y));
        if (nodes.size() == 0)
            return;

        for (auto const& n : nodes) {
            auto nn = model->GetNode(n-1);
            if (nn != nullptr) {
                if (shiftDwn) {
                    _channelTracker.RemoveRange(nn->ActChan + 1, nn->ActChan + model->GetChanCountPerNode());
                } else {
                    _channelTracker.AddRange(nn->ActChan + 1, nn->ActChan + model->GetChanCountPerNode());
                }
            }
        }

        UpdateVisualModelFromTracker();
        RenderModel();
        _checkChannelList = true;
    }
}
#pragma endregion

#pragma region ModelTab
void PixelTestDialog::PopulateModelTree(ModelManager* modelManager)
{
    std::list<std::string> modelNames;
    for (const auto& it : *_modelManager) {
        Model* m = it.second;

        if (m->GetDisplayAs() != "ModelGroup") {
            modelNames.push_back(m->GetName());
        }
    }
    modelNames.sort(stdlistNumberAwareStringCompare);

    for (const auto& it : modelNames) {
        Model* m = modelManager->GetModel(it);

        if (m != nullptr && m->GetDisplayAs() != "ModelGroup") {
            // we found a model
            ModelTestItem* modelcontroller = new ModelTestItem(m->GetName(), "", *modelManager, AreChannelsAvailable(m));
            _models.push_back(modelcontroller);
            wxTreeListItem modelitem = TreeListCtrl_Models->AppendItem(TreeListCtrl_Models->GetRootItem(), modelcontroller->GetName(), -1, -1, (wxClientData*)modelcontroller);
            modelcontroller->SetTreeListItem(modelitem);
            if (modelcontroller->IsClickable()) {
                auto submodels = modelcontroller->GetSubModels();
                if (submodels.size() > 0) {
                    for (const auto& it2 : submodels) {
                        wxTreeListItem submodelitem = TreeListCtrl_Models->AppendItem(modelitem, it2->GetName(), -1, -1, (wxClientData*)it2);
                        it2->SetTreeListItem(submodelitem);
                    }
                    NodesTestItem* nti = new NodesTestItem();
                    wxTreeListItem nodesitem = TreeListCtrl_Models->AppendItem(modelitem, nti->GetName(), -1, -1, (wxClientData*)nti);
                    nti->SetTreeListItem(nodesitem);
                    TreeListCtrl_Models->AppendItem(nodesitem, "Dummy");
                } else {
                    TreeListCtrl_Models->AppendItem(modelitem, "Dummy");
                }
            }
        } else {
            wxASSERT(false);
        }
    }
}
#pragma endregion

#pragma region GenericTreeEvents
void PixelTestDialog::CascadeSelected(wxTreeListCtrl* tree, const wxTreeListItem& item, wxCheckBoxState state)
{
    tree->CheckItemRecursively(item, state);
}

void PixelTestDialog::DestroyTreeControllerData(wxTreeListCtrl* tree, wxTreeListItem& item)
{
    wxTreeListItem i = tree->GetFirstChild(item);
    while (i != nullptr) {
        DestroyTreeControllerData(tree, i);
        i = tree->GetNextSibling(i);
    }

    TestItemBase* tc = (TestItemBase*)tree->GetItemData(item);
    if (tc != nullptr) {
        if ((tree == TreeListCtrl_ModelGroups && tc->GetType() == "Model") ||
            (tree == TreeListCtrl_Models && tc->GetType() == "SubModel")) {
            // dont delete these
        } else {
            delete tc;
        }
    }
}

void PixelTestDialog::DeactivateNotClickableModels(wxTreeListCtrl* tree)
{
    wxTreeListItem i = tree->GetFirstChild(tree->GetRootItem());
    while (i != nullptr) {
        TestItemBase* tc = (TestItemBase*)tree->GetItemData(i);
        if (!tc->IsClickable()) {
            tree->SetItemText(i, tc->GetName());
        }
        i = tree->GetNextSibling(i);
    }
}

void PixelTestDialog::SetTreeTooltip(wxTreeListCtrl* tree, wxTreeListItem& item)
{
    if (tree == TreeListCtrl_Outputs) {
        TestItemBase* tib = (TestItemBase*)tree->GetItemData(item);
        if (tib != nullptr) {
            if (tib->GetType() == "Controller" || tib->GetType() == "Channel") {
                std::string tt = "";
                for (const auto& it : _models) {
                    if (it->ContainsChannelRange(tib->GetFirstChannel(), tib->GetLastChannel())) {
                        if (tt != "") {
                            tt += "\n";
                        }
                        tt = tt + it->GetModelName();
                    }
                }
                if (tt != "") {
                    if (tib->GetFirstChannel() == tib->GetLastChannel()) {
                        tt = "[" + std::string(wxString::Format(wxT("%ld"), tib->GetFirstChannel())) + "] maps to\n" + tt;
                    } else {
                        tt = "[" + std::string(wxString::Format(wxT("%ld"), tib->GetFirstChannel())) + "-" + std::string(wxString::Format(wxT("%ld"), tib->GetLastChannel())) + "] maps to\n" + tt;
                    }
#ifdef __WXOSX__
                    tree->SetToolTip(tt);
#else
                    tree->GetView()->SetToolTip(tt);
#endif
                } else {
#ifdef __WXOSX__
                    tree->UnsetToolTip();
#else
                    tree->GetView()->UnsetToolTip();
#endif
                }
            }
        } else {
#ifdef __WXOSX__
            tree->UnsetToolTip();
#else
            tree->GetView()->UnsetToolTip();
#endif
        }
    } else {
#ifdef __WXOSX__
        tree->UnsetToolTip();
#else
        tree->GetView()->UnsetToolTip();
#endif
    }
}

void PixelTestDialog::OnTreeListCtrlItemSelected(wxTreeListEvent& event)
{
    wxTreeListCtrl* tree = (wxTreeListCtrl*)event.GetEventObject();
    wxTreeListItem item = event.GetItem();
    SetTreeTooltip(tree, item);
}

void PixelTestDialog::AddChannel(wxTreeListCtrl* tree, wxTreeListItem parent, long absoluteChannel, long relativeChannel, char colour)
{
    ChannelTestItem* cti = new ChannelTestItem(relativeChannel, absoluteChannel, colour, true);
    wxTreeListItem c = tree->AppendItem(parent, cti->GetName(), -1, -1, (wxClientData*)cti);
    tree->CheckItem(c, cti->GetState(_channelTracker));
    cti->SetTreeListItem(c);
}

void PixelTestDialog::AddNode(wxTreeListCtrl* tree, wxTreeListItem parent, ModelTestItem* model, long node)
{
    NodeTestItem* nti = new NodeTestItem(node + 1, model->GetNodeAbsoluteChannel(node), model->GetChannelsPerNode(), true);
    wxTreeListItem c = tree->AppendItem(parent, nti->GetName(), -1, -1, (wxClientData*)nti);
    tree->CheckItem(c, nti->GetState(_channelTracker));
    nti->SetTreeListItem(c);

    if (model->GetChannelsPerNode() > 1) {
        tree->AppendItem(c, "Dummy");
    }
}

void PixelTestDialog::OnTreeListCtrlItemExpanding(wxTreeListEvent& event)
{
    wxTreeListItem item = event.GetItem();
    wxTreeListCtrl* tree = (wxTreeListCtrl*)event.GetEventObject();

    if (tree->GetItemText(tree->GetFirstChild(item)) == "Dummy") {
        if (tree == TreeListCtrl_Outputs) {
            ControllerTestItem* controller = (ControllerTestItem*)tree->GetItemData(item);
            long ch = controller->GetFirstChannel();
            while (ch > 0) {
                long offset = controller->GetChannelOffset(ch);
                char c = GetChannelColour(ch);
                AddChannel(tree, item, ch, offset, c);
                ch = controller->GetNextChannel();
            }
        } else if (tree == TreeListCtrl_ModelGroups) {
        } else if (tree == TreeListCtrl_Models || tree == TreeListCtrl_Controllers) {
            TestItemBase* tib = (TestItemBase*)tree->GetItemData(item);
            if (tib->GetType() == "Nodes" || tib->GetType() == "Model") {
                ModelTestItem* mti = nullptr;

                if (tib->GetType() == "Nodes") {
                    mti = (ModelTestItem*)tree->GetItemData(TreeListCtrl_Models->GetItemParent(item));
                } else {
                    mti = (ModelTestItem*)tree->GetItemData(item);
                }
                long nodes = mti->GetNodes();
                for (int i = 0; i < nodes; i++) {
                    AddNode(tree, item, mti, i);
                }
            } else if (tib->GetType() == "Node") {
                NodeTestItem* node = (NodeTestItem*)tree->GetItemData(item);
                long ch = node->GetFirstChannel();
                while (ch > 0) {
                    long offset = node->GetChannelOffset(ch);
                    char c = GetChannelColour(ch);
                    AddChannel(tree, item, ch, offset, c);
                    ch = node->GetNextChannel();
                }
            }
        }

        tree->DeleteItem(tree->GetFirstChild(item));
    }
}

void PixelTestDialog::OnTreeListCtrlItemActivated(wxTreeListEvent& event)
{
    // On Mac, the checkboxes aren't working, we'll fake it with the double click activations
    wxTreeListItem item = event.GetItem();
    wxTreeListCtrl* tree = (wxTreeListCtrl*)event.GetEventObject();
    wxCheckBoxState checked = tree->GetCheckedState(item);
    if (checked != wxCHK_CHECKED) {
        tree->CheckItem(item, wxCHK_CHECKED);
    } else {
        tree->CheckItem(item, wxCHK_UNCHECKED);
    }
    OnTreeListCtrlCheckboxtoggled(event);

    SetTreeTooltip(tree, item);
}

void PixelTestDialog::OnContextMenu(wxTreeListEvent& event)
{
    _rcItem = event.GetItem();
    _rcTree = (wxTreeListCtrl*)event.GetEventObject();
    wxMenu mnuContext;
    mnuContext.Append(ID_MNU_TEST_SELECTALL, "Select All");
    mnuContext.Append(ID_MNU_TEST_DESELECTALL, "Deselect All");
    mnuContext.Append(ID_MNU_TEST_SELECTN, "Select Many");
    mnuContext.Append(ID_MNU_TEST_DESELECTN, "Deselect Many");
    if (_rcTree == TreeListCtrl_Controllers)
        mnuContext.Append(ID_MNU_TEST_NUMBER, "Number");

    mnuContext.Connect(wxEVT_COMMAND_MENU_SELECTED, (wxObjectEventFunction)&PixelTestDialog::OnListPopup, nullptr, this);
    PopupMenu(&mnuContext);
}

void PixelTestDialog::OnListPopup(wxCommandEvent& event)
{
    // static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));

    wxTreeListCtrl* tree = _rcTree;
    wxTreeListItem selected = _rcItem;
    wxTreeListItem root = tree->GetFirstChild(tree->GetRootItem());
    long id = event.GetId();
    if (id == ID_MNU_TEST_SELECTALL) {
        wxTreeListItem curr = root;
        while (curr.IsOk()) {
            TestItemBase* tc = (TestItemBase*)tree->GetItemData(curr);
            if (tc != nullptr) {
                if (tc->IsContiguous()) {
                    _channelTracker.AddRange(tc->GetFirstChannel(), tc->GetLastChannel());
                } else {
                    long ch = tc->GetFirstChannel();
                    while (ch != -1) {
                        _channelTracker.AddRange(ch, ch);
                        ch = tc->GetNextChannel();
                    }
                }
            }
            curr = tree->GetNextSibling(curr);
        }

        tree->CheckItem(tree->GetRootItem(), wxCHK_CHECKED);
        CascadeSelected(tree, tree->GetRootItem(), wxCHK_CHECKED);
    } else if (id == ID_MNU_TEST_DESELECTALL) {
        wxTreeListItem curr = root;
        while (curr.IsOk()) {
            TestItemBase* tc = (TestItemBase*)tree->GetItemData(curr);
            if (tc != nullptr) {
                if (tc->IsContiguous()) {
                    _channelTracker.RemoveRange(tc->GetFirstChannel(), tc->GetLastChannel());
                } else {
                    long ch = tc->GetFirstChannel();
                    while (ch != -1) {
                        _channelTracker.RemoveRange(ch, ch);
                        ch = tc->GetNextChannel();
                    }
                }
            }
            curr = tree->GetNextSibling(curr);
        }
        tree->CheckItem(tree->GetRootItem(), wxCHK_UNCHECKED);
        CascadeSelected(tree, tree->GetRootItem(), wxCHK_UNCHECKED);
    } else if (id == ID_MNU_TEST_SELECTN) {
        if (selected.IsOk()) {
            wxNumberEntryDialog dlg(this, "Number to select", "", "", 2, 1, 1000);
            if (dlg.ShowModal() == wxID_OK) {
                int count = dlg.GetValue();

                while (count > 0 && selected.IsOk()) {
                    TestItemBase* tc = (TestItemBase*)tree->GetItemData(selected);
                    if (tc->IsContiguous()) {
                        _channelTracker.AddRange(tc->GetFirstChannel(), tc->GetLastChannel());
                    } else {
                        long ch = tc->GetFirstChannel();
                        while (ch != -1) {
                            _channelTracker.AddRange(ch, ch);
                            ch = tc->GetNextChannel();
                        }
                    }

                    tree->CheckItem(selected, wxCHK_CHECKED);

                    RollUpAll(tree, selected);

                    selected = tree->GetNextSibling(selected);
                    count--;
                }
            }
        }
    } else if (id == ID_MNU_TEST_DESELECTN) {
        if (selected.IsOk()) {
            wxNumberEntryDialog dlg(this, "Number to deselect", "", "", 2, 1, 1000);
            if (dlg.ShowModal() == wxID_OK) {
                int count = dlg.GetValue();

                while (count > 0 && selected.IsOk()) {
                    TestItemBase* tc = (TestItemBase*)tree->GetItemData(selected);
                    if (tc->IsContiguous()) {
                        _channelTracker.RemoveRange(tc->GetFirstChannel(), tc->GetLastChannel());
                    } else {
                        long ch = tc->GetFirstChannel();
                        while (ch != -1) {
                            _channelTracker.RemoveRange(ch, ch);
                            ch = tc->GetNextChannel();
                        }
                    }

                    tree->CheckItem(selected, wxCHK_UNCHECKED);

                    RollUpAll(tree, selected);

                    selected = tree->GetNextSibling(selected);
                    count--;
                }
            }
        }
    } else if (id == ID_MNU_TEST_NUMBER) {
        if (selected.IsOk()) {
            TestItemBase* tc = (TestItemBase*)tree->GetItemData(selected);

            if (tc->IsClickable()) {
                if (tc->GetType() == "Controller") {
                    // controller
                    for (auto p = tree->GetFirstChild(selected); p != nullptr; p = tree->GetNextSibling(p)) {
                        tc = (TestItemBase*)tree->GetItemData(p);
                        uint16_t port = ((CPR_PortTestItem*)tc)->GetPort();
                        uint16_t pixel = 0;
                        for (auto srporm = tree->GetFirstChild(p); srporm != nullptr; srporm = tree->GetNextSibling(srporm)) {
                            tc = (TestItemBase*)tree->GetItemData(srporm);
                            if (tc->GetType() == "SR") {
                                for (auto m = tree->GetFirstChild(srporm); m != nullptr; m = tree->GetNextSibling(m)) {
                                    for (auto px = tree->GetFirstChild(m); px != nullptr; px = tree->GetNextSibling(px)) {
                                        tc = (TestItemBase*)tree->GetItemData(px);
                                        if (tc != nullptr) {
                                            if (pixel < port) {
                                                tree->CheckItem(px, wxCHK_CHECKED);
                                                _channelTracker.AddRange(tc->GetFirstChannel(), tc->GetLastChannel());
                                                RollUpAll(tree, px);
                                            } else {
                                                tree->CheckItem(px, wxCHK_UNCHECKED);
                                                _channelTracker.RemoveRange(tc->GetFirstChannel(), tc->GetLastChannel());
                                                RollUpAll(tree, px);
                                            }
                                            ++pixel;
                                        } else {
                                            ModelTestItem* tm = (ModelTestItem*)tree->GetItemData(m);
                                            if (pixel < port) {
                                                tree->CheckItem(px, wxCHK_CHECKED);
                                                auto ep = std::min(tm->GetLastChannel(), tm->GetFirstChannel() + (port - pixel) * tm->GetChannelsPerNode() - 1);
                                                _channelTracker.AddRange(tm->GetFirstChannel(), ep);
                                                if (ep != tm->GetLastChannel()) {
                                                    _channelTracker.RemoveRange(ep + 1, tm->GetLastChannel());
                                                }
                                                pixel += (ep - tm->GetFirstChannel() + 1) / tm->GetChannelsPerNode();
                                                RollUpAll(tree, px);
                                            }
                                        }
                                    }
                                }
                            } else {
                                for (auto px = tree->GetFirstChild(srporm); px != nullptr; px = tree->GetNextSibling(px)) {
                                    tc = (TestItemBase*)tree->GetItemData(px);
                                    if (tc != nullptr) {
                                        if (pixel < port) {
                                            tree->CheckItem(px, wxCHK_CHECKED);
                                            _channelTracker.AddRange(tc->GetFirstChannel(), tc->GetLastChannel());
                                            RollUpAll(tree, px);
                                        } else {
                                            tree->CheckItem(px, wxCHK_UNCHECKED);
                                            _channelTracker.RemoveRange(tc->GetFirstChannel(), tc->GetLastChannel());
                                            RollUpAll(tree, px);
                                        }
                                        ++pixel;
                                    } else {
                                        ModelTestItem* tm = (ModelTestItem*)tree->GetItemData(srporm);
                                        if (pixel < port) {
                                            tree->CheckItem(px, wxCHK_CHECKED);
                                            auto ep = std::min(tm->GetLastChannel(), tm->GetFirstChannel() + (port - pixel) * tm->GetChannelsPerNode() - 1);
                                            _channelTracker.AddRange(tm->GetFirstChannel(), ep);
                                            if (ep != tm->GetLastChannel()) {
                                                _channelTracker.RemoveRange(ep + 1, tm->GetLastChannel());
                                            }
                                            pixel += (ep - tm->GetFirstChannel() + 1) / tm->GetChannelsPerNode();
                                            RollUpAll(tree, px);
                                        }
                                    }
                                }
                            }
                        }
                    }
                } else {
                    // port / Node
                    // move up to the port
                    while (tc != nullptr && tc->GetType() != "Port") {
                        selected = tree->GetItemParent(selected);
                        tc = (TestItemBase*)tree->GetItemData(selected);
                    }
                    uint16_t port = ((CPR_PortTestItem*)tc)->GetPort();
                    uint16_t pixel = 0;

                    for (auto srporm = tree->GetFirstChild(selected); srporm != nullptr; srporm = tree->GetNextSibling(srporm)) {
                        tc = (TestItemBase*)tree->GetItemData(srporm);
                        if (tc->GetType() == "SR") {
                            for (auto m = tree->GetFirstChild(srporm); m != nullptr; m = tree->GetNextSibling(m)) {
                                for (auto px = tree->GetFirstChild(m); px != nullptr; px = tree->GetNextSibling(px)) {
                                    tc = (TestItemBase*)tree->GetItemData(px);
                                    if (tc != nullptr) {
                                        if (pixel < port) {
                                            tree->CheckItem(px, wxCHK_CHECKED);
                                            _channelTracker.AddRange(tc->GetFirstChannel(), tc->GetLastChannel());
                                            RollUpAll(tree, px);
                                        } else {
                                            tree->CheckItem(px, wxCHK_UNCHECKED);
                                            _channelTracker.RemoveRange(tc->GetFirstChannel(), tc->GetLastChannel());
                                            RollUpAll(tree, px);
                                        }
                                        ++pixel;
                                    } else {
                                        ModelTestItem* tm = (ModelTestItem*)tree->GetItemData(m);
                                        if (pixel < port) {
                                            tree->CheckItem(px, wxCHK_CHECKED);
                                            auto ep = std::min(tm->GetLastChannel(), tm->GetFirstChannel() + (port - pixel) * tm->GetChannelsPerNode() - 1);
                                            _channelTracker.AddRange(tm->GetFirstChannel(), ep);
                                            if (ep != tm->GetLastChannel()) {
                                                _channelTracker.RemoveRange(ep + 1, tm->GetLastChannel());
                                            }
                                            pixel += (ep - tm->GetFirstChannel() + 1) / tm->GetChannelsPerNode();
                                            RollUpAll(tree, px);
                                        }
                                    }
                                }
                            }
                        } else {
                            for (auto px = tree->GetFirstChild(srporm); px != nullptr; px = tree->GetNextSibling(px)) {
                                tc = (TestItemBase*)tree->GetItemData(px);
                                if (tc != nullptr) {
                                    if (pixel < port) {
                                        tree->CheckItem(px, wxCHK_CHECKED);
                                        _channelTracker.AddRange(tc->GetFirstChannel(), tc->GetLastChannel());
                                        RollUpAll(tree, px);
                                    } else {
                                        tree->CheckItem(px, wxCHK_UNCHECKED);
                                        _channelTracker.RemoveRange(tc->GetFirstChannel(), tc->GetLastChannel());
                                        RollUpAll(tree, px);
                                    }
                                    ++pixel;
                                } else {
                                    ModelTestItem* tm = (ModelTestItem*)tree->GetItemData(srporm);
                                    if (pixel < port) {
                                        tree->CheckItem(px, wxCHK_CHECKED);
                                        auto ep = std::min(tm->GetLastChannel(), tm->GetFirstChannel() + (port - pixel) * tm->GetChannelsPerNode() - 1);
                                        _channelTracker.AddRange(tm->GetFirstChannel(), ep);
                                        if (ep != tm->GetLastChannel()) {
                                            _channelTracker.RemoveRange(ep + 1, tm->GetLastChannel());
                                        }
                                        pixel += (ep - tm->GetFirstChannel() + 1) / tm->GetChannelsPerNode();
                                        RollUpAll(tree, px);
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}

void PixelTestDialog::OnTreeListCtrlCheckboxtoggled(wxTreeListEvent& event)
{
    wxTreeListItem item = event.GetItem();
    wxTreeListCtrl* tree = (wxTreeListCtrl*)event.GetEventObject();

    TestItemBase* tc = (TestItemBase*)tree->GetItemData(item);

    // You cannot check these items
    if (!tc->IsClickable()) {
        tree->CheckItem(item, wxCHK_UNCHECKED);
        wxBell();
        return;
    }

    wxCheckBoxState checked = tree->GetCheckedState(item);
    if (checked == wxCheckBoxState::wxCHK_UNDETERMINED) {
        wxCheckBoxState state = event.GetOldCheckedState() == wxCHK_CHECKED ? wxCHK_UNCHECKED : wxCHK_CHECKED;
        tree->CheckItem(item, state);
        checked = state;
    }

    if (checked == wxCheckBoxState::wxCHK_CHECKED) {
        if (tc->IsContiguous()) {
            _channelTracker.AddRange(tc->GetFirstChannel(), tc->GetLastChannel());
        } else {
            long ch = tc->GetFirstChannel();
            while (ch != -1) {
                _channelTracker.AddRange(ch, ch);
                ch = tc->GetNextChannel();
            }
        }
        CascadeSelected(tree, item, wxCheckBoxState::wxCHK_CHECKED);
    } else if (checked == wxCheckBoxState::wxCHK_UNCHECKED) {
        if (tc->IsContiguous()) {
            _channelTracker.RemoveRange(tc->GetFirstChannel(), tc->GetLastChannel());
        } else {
            long ch = tc->GetFirstChannel();
            while (ch != -1) {
                _channelTracker.RemoveRange(ch, ch);
                ch = tc->GetNextChannel();
            }
        }
        CascadeSelected(tree, item, wxCheckBoxState::wxCHK_UNCHECKED);
    }

    RollUpAll(tree, item);

    _checkChannelList = true;

    // handle multiple selected items
    wxTreeListItems selections;
    tree->GetSelections(selections);
    if (selections.size() > 1) {
        for (int i = 0; i < selections.size(); i++) {
            // dont double process the item that was passed into the event
            if (selections[i] != item) {
                tc = (TestItemBase*)tree->GetItemData(selections[i]);
                if (tree->GetCheckedState(selections[i]) == wxCHK_UNCHECKED) {
                    // check the items
                    tree->CheckItem(selections[i], wxCheckBoxState::wxCHK_CHECKED);
                    if (tc->IsContiguous()) {
                        _channelTracker.AddRange(tc->GetFirstChannel(), tc->GetLastChannel());
                    } else {
                        long ch = tc->GetFirstChannel();
                        while (ch != -1) {
                            _channelTracker.AddRange(ch, ch);
                            ch = tc->GetNextChannel();
                        }
                    }
                    CascadeSelected(tree, selections[i], wxCheckBoxState::wxCHK_CHECKED);
                } else if (tree->GetCheckedState(selections[i]) == wxCHK_CHECKED) {
                    // uncheck the items
                    tree->CheckItem(selections[i], wxCheckBoxState::wxCHK_UNCHECKED);
                    if (tc->IsContiguous()) {
                        _channelTracker.RemoveRange(tc->GetFirstChannel(), tc->GetLastChannel());
                    } else {
                        long ch = tc->GetFirstChannel();
                        while (ch != -1) {
                            _channelTracker.RemoveRange(ch, ch);
                            ch = tc->GetNextChannel();
                        }
                    }
                    CascadeSelected(tree, selections[i], wxCheckBoxState::wxCHK_UNCHECKED);
                }
            }
        }
    }

    _checkChannelList = true;

    SetTreeTooltip(tree, item);

    SetCheckBoxItemFromTracker(tree, tree->GetRootItem(), wxCheckBoxState::wxCHK_UNCHECKED);

    RollUpAll(tree, item);

    DumpSelected();
}

void PixelTestDialog::RollUpAll(wxTreeListCtrl* tree, wxTreeListItem start)
{
    tree->UpdateItemParentStateRecursively(start);
}

void PixelTestDialog::Clear(wxTreeListCtrl* tree, wxTreeListItem& item)
{
    tree->CheckItem(item, wxCHK_UNCHECKED);

    wxTreeListItem i = tree->GetFirstChild(item);
    while (i != nullptr) {
        Clear(tree, i);
        i = tree->GetNextSibling(i);
    }
}
#pragma endregion

#pragma region TestPresets
void PixelTestDialog::OnButton_LoadClick(wxCommandEvent& event)
{
    auto presets = _outputManager->GetTestPresets();

    if (presets.size() == 0) {
        DisplayError("No test configurations found", this);
        return;
    }

    // get user selection
    presets.sort();
    wxArrayString PresetNames;
    for (const auto& it : presets) {
        PresetNames.Add(wxString(it.c_str()));
    }
    wxSingleChoiceDialog dialog(this, _("Select test configuration"), _("Load Test Settings"), PresetNames);

    if (dialog.ShowModal() != wxID_OK)
        return;

    wxString name = dialog.GetStringSelection();
    long ChCount = _outputManager->GetTotalChannels();
    TestPreset* preset = _outputManager->GetTestPreset(name.ToStdString());

    if (preset == nullptr)
        return; // this should never happen

    _channelTracker.Clear();

    auto chs = preset->GetChannels();
    for (const auto& c : chs) {
        if (c > 0 && c < ChCount) {
            _channelTracker.AddRange(c, c);
        }
    }

    SetCheckBoxItemFromTracker(TreeListCtrl_Outputs, TreeListCtrl_Outputs->GetRootItem(), wxCheckBoxState::wxCHK_UNCHECKED);

    _checkChannelList = true;
}

void PixelTestDialog::OnButton_SaveClick(wxCommandEvent& event)
{
    wxTextEntryDialog NameDialog(this, _("Enter a name for this test configuration"), _("Save Test Settings"));
    if (NameDialog.ShowModal() != wxID_OK)
        return;

    wxString name = NameDialog.GetValue().Trim(true).Trim(false);

    if (name.IsEmpty()) {
        DisplayError("Name cannot be empty", this);
        return;
    } else if (name.Len() > 240) {
        DisplayError("Name is too long", this);
        return;
    } else if (_outputManager->GetTestPreset(name.ToStdString()) != nullptr) {
        if (wxMessageBox(_("Name already exists. Do you want to overwrite it?"), _("Warning"), wxYES_NO) == wxNO) {
            return;
        }
    }

    TestPreset* testPreset = _outputManager->CreateTestPreset(name.ToStdString());

    long start;
    long end;
    _channelTracker.GetFirstRange(start, end);
    while (start > 0) {
        testPreset->AddChannelRange(start, end);
        _channelTracker.GetNextRange(start, end);
    }
    static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    logger_base.debug("Saving test preset: %s", (const char*)name.c_str());
    _outputManager->Save();
    logger_base.debug("   Save done.");
}
#pragma endregion

void PixelTestDialog::GetCheckedItems(wxArrayInt& chArray)
{
    chArray.Clear();
    long ch = _channelTracker.GetFirst();
    while (ch > 0) {
        chArray.Add(ch);
        ch = _channelTracker.GetNext();
    }
}

void PixelTestDialog::GetCheckedItems(wxArrayInt& chArray, char col)
{
    chArray.Clear();

    long ch = _channelTracker.GetFirst();
    while (ch > 0) {
        if (GetChannelColour(ch) == col) {
            chArray.Add(ch);
        }
        ch = _channelTracker.GetNext();
    }
}

void PixelTestDialog::OnTimer1Trigger(wxTimerEvent& event)
{
    wxTimeSpan ts = wxDateTime::UNow() - _starttime;
    long curtime = ts.GetMilliseconds().ToLong();
    _outputManager->StartFrame(curtime);
    OnTimer(curtime);
    _outputManager->EndFrame();
}

void PixelTestDialog::TestButtonsOff()
{
    RadioButton_Standard_Off->SetValue(true);
    RadioButton_Standard_Chase->SetValue(false);
    RadioButton_Standard_Chase13->SetValue(false);
    RadioButton_Standard_Chase14->SetValue(false);
    RadioButton_Standard_Chase15->SetValue(false);
    RadioButton_Standard_Alternate->SetValue(false);
    RadioButton_Standard_Twinkle5->SetValue(false);
    RadioButton_Standard_Twinkle10->SetValue(false);
    RadioButton_Standard_Twinkle25->SetValue(false);
    RadioButton_Standard_Twinkle50->SetValue(false);
    RadioButton_Standard_Shimmer->SetValue(false);
    RadioButton_Standard_Background->SetValue(false);

    RadioButton_RGB_Off->SetValue(true);
    RadioButton_RGB_Chase->SetValue(false);
    RadioButton_RGB_Chase13->SetValue(false);
    RadioButton_RGB_Chase14->SetValue(false);
    RadioButton_RGB_Chase15->SetValue(false);
    RadioButton_RGB_Alternate->SetValue(false);
    RadioButton_RGB_Twinkle5->SetValue(false);
    RadioButton_RGB_Twinkle10->SetValue(false);
    RadioButton_RGB_Twinkle25->SetValue(false);
    RadioButton_RGB_Twinkle50->SetValue(false);
    RadioButton_RGB_Shimmer->SetValue(false);
    RadioButton_RGB_Background->SetValue(false);

    RadioButton_RGBCycle_Off->SetValue(true);
    RadioButton_RGBCycle_ABC->SetValue(false);
    RadioButton_RGBCycle_ABCAll->SetValue(false);
    RadioButton_RGBCycle_ABCAllNone->SetValue(false);
    RadioButton_RGBCycle_MixedColors->SetValue(false);
    RadioButton_RGBCycle_RGBW->SetValue(false);
}

PixelTestDialog::TestFunctions PixelTestDialog::GetTestFunction(int notebookSelection)
{
    switch (notebookSelection)
    {
    case 0:
        if (RadioButton_Standard_Off->GetValue())
        {
            return PixelTestDialog::TestFunctions::OFF;
        }
        else if (RadioButton_Standard_Chase->GetValue())
        {
            _chaseGrouping = std::numeric_limits<int>::max();
            return PixelTestDialog::TestFunctions::CHASE;
        }
        else if (RadioButton_Standard_Chase13->GetValue())
        {
            _chaseGrouping = 3;
            return PixelTestDialog::TestFunctions::CHASE;
        }
        else if (RadioButton_Standard_Chase14->GetValue())
        {
            _chaseGrouping = 4;
            return PixelTestDialog::TestFunctions::CHASE;
        }
        else if (RadioButton_Standard_Chase15->GetValue())
        {
            _chaseGrouping = 5;
            return PixelTestDialog::TestFunctions::CHASE;
        }
        else if (RadioButton_Standard_Alternate->GetValue())
        {
            _chaseGrouping = 2;
            return PixelTestDialog::TestFunctions::CHASE;
        }
        else if (RadioButton_Standard_Twinkle5->GetValue())
        {
            _twinkleRatio = 20;
            return PixelTestDialog::TestFunctions::TWINKLE;
        }
        else if (RadioButton_Standard_Twinkle10->GetValue())
        {
            _twinkleRatio = 10;
            return PixelTestDialog::TestFunctions::TWINKLE;
        }
        else if (RadioButton_Standard_Twinkle25->GetValue())
        {
            _twinkleRatio = 4;
            return PixelTestDialog::TestFunctions::TWINKLE;
        }
        else if (RadioButton_Standard_Twinkle50->GetValue())
        {
            _twinkleRatio = 2;
            return PixelTestDialog::TestFunctions::TWINKLE;
        }
        else if (RadioButton_Standard_Shimmer->GetValue())
        {
            return PixelTestDialog::TestFunctions::SHIMMER;
        }
        else if (RadioButton_Standard_Background->GetValue())
        {
            return PixelTestDialog::TestFunctions::DIM;
        }
        break;
    case 1:
        if (RadioButton_RGB_Off->GetValue())
        {
            return PixelTestDialog::TestFunctions::OFF;
        }
        else if (RadioButton_RGB_Chase->GetValue())
        {
            _chaseGrouping = std::numeric_limits<int>::max();
            return PixelTestDialog::TestFunctions::CHASE;
        }
        else if (RadioButton_RGB_Chase13->GetValue())
        {
            _chaseGrouping = 3;
            return PixelTestDialog::TestFunctions::CHASE;
        }
        else if (RadioButton_RGB_Chase14->GetValue())
        {
            _chaseGrouping = 4;
            return PixelTestDialog::TestFunctions::CHASE;
        }
        else if (RadioButton_RGB_Chase15->GetValue())
        {
            _chaseGrouping = 5;
            return PixelTestDialog::TestFunctions::CHASE;
        }
        else if (RadioButton_RGB_Alternate->GetValue())
        {
            _chaseGrouping = 2;
            return PixelTestDialog::TestFunctions::CHASE;
        }
        else if (RadioButton_RGB_Twinkle5->GetValue())
        {
            _twinkleRatio = 20;
            return PixelTestDialog::TestFunctions::TWINKLE;
        }
        else if (RadioButton_RGB_Twinkle10->GetValue())
        {
            _twinkleRatio = 10;
            return PixelTestDialog::TestFunctions::TWINKLE;
        }
        else if (RadioButton_RGB_Twinkle25->GetValue())
        {
            _twinkleRatio = 4;
            return PixelTestDialog::TestFunctions::TWINKLE;
        }
        else if (RadioButton_RGB_Twinkle50->GetValue())
        {
            _twinkleRatio = 2;
            return PixelTestDialog::TestFunctions::TWINKLE;
        }
        else if (RadioButton_RGB_Shimmer->GetValue())
        {
            return PixelTestDialog::TestFunctions::SHIMMER;
        }
        else if (RadioButton_RGB_Background->GetValue())
        {
            return PixelTestDialog::TestFunctions::DIM;
        }
        break;
    case 2:
        if (RadioButton_RGBCycle_Off->GetValue())
        {
            return PixelTestDialog::TestFunctions::OFF;
        }
        else if (RadioButton_RGBCycle_ABC->GetValue())
        {
            _chaseGrouping = 3;
            return PixelTestDialog::TestFunctions::CHASE;
        }
        else if (RadioButton_RGBCycle_ABCAll->GetValue())
        {
            _chaseGrouping = 4;
            return PixelTestDialog::TestFunctions::CHASE;
        }
        else if (RadioButton_RGBCycle_ABCAllNone->GetValue())
        {
            _chaseGrouping = 5;
            return PixelTestDialog::TestFunctions::CHASE;
        }
        else if (RadioButton_RGBCycle_MixedColors->GetValue())
        {
            return PixelTestDialog::TestFunctions::DIM;
        }
        else if (RadioButton_RGBCycle_RGBW->GetValue())
        {
            return PixelTestDialog::TestFunctions::RGBW;
        }
        break;
    }

    return PixelTestDialog::TestFunctions::OFF;
}

void PixelTestDialog::OnTimer(long curtime)
{
    static int LastNotebookSelection = -1;
    static int LastBgIntensity, LastFgIntensity, LastBgColor[3], LastFgColor[3], *ShimColor, ShimIntensity;
    static int LastSequenceSpeed;
    static int LastTwinkleRatio;
    //static int LastAutomatedTest;
    static long NextSequenceStart = -1;
    static TestFunctions LastFunc = PixelTestDialog::TestFunctions::OFF;
    static unsigned int interval, rgbCycle, TestSeqIdx;
    static wxArrayInt chArray, chArrayR, chArrayG, chArrayB, chArrayW, TwinkleState;
    static float frequency;
    int v, BgColor[3], FgColor[3];
    unsigned int i;
    bool ColorChange;
    bool fiftieth = CheckBox_Tag50th->GetValue();

    int NotebookSelection = Notebook2->GetSelection();
    if (NotebookSelection != LastNotebookSelection) {
        LastNotebookSelection = NotebookSelection;
        _checkChannelList = true;
        TestSeqIdx = 0;
    }

    TestFunctions testFunc = GetTestFunction(NotebookSelection);

    if (testFunc != LastFunc) {
        LastFunc = testFunc;
        rgbCycle = 0;
        _checkChannelList = true;
        NextSequenceStart = -1;
    }

    if (_checkChannelList) {
        SetSuspend(CheckBox_SuppressUnusedOutputs->GetValue());

        NextSequenceStart = -1;

        // get list of checked channels
        _outputManager->AllOff();
        GetCheckedItems(chArray);
        if (RadioButton_RGB_Chase->GetValue() || RadioButton_Standard_Chase->GetValue()) {
            _chaseGrouping = chArray.Count();
            if (_chaseGrouping == 0) {
                _chaseGrouping = std::numeric_limits<int>::max();
            }
        }
        if (RadioButton_RGBCycle_RGBW->GetValue()) {
            GetCheckedItems(chArrayR, 'R');
            GetCheckedItems(chArrayG, 'G');
            GetCheckedItems(chArrayB, 'B');
            GetCheckedItems(chArrayW, 'W');
        }

        LastSequenceSpeed = -1;
        LastBgIntensity = -1;
        LastFgIntensity = -1;
        //LastAutomatedTest = -1;
        LastTwinkleRatio = -1;
        for (i = 0; i < 3; i++) {
            LastBgColor[i] = -1;
            LastFgColor[i] = -1;
        }
        if (testFunc == PixelTestDialog::TestFunctions::OFF) {
            StatusBar1->SetLabelText(_("Testing off"));
        } else {
            StatusBar1->SetLabelText(wxString::Format(_("Testing %ld channels"), static_cast<long>(chArray.Count())));
        }
        _checkChannelList = false;
    }

    if (testFunc != PixelTestDialog::TestFunctions::OFF && chArray.Count() > 0) {
        switch (NotebookSelection) {
        case 0: {
            // standard tests
            v = Slider_Speed->GetValue(); // 0-100
            int BgIntensity = Slider_Standard_Background->GetValue();
            int FgIntensity = Slider_Standard_Highlight->GetValue();
            ColorChange = BgIntensity != LastBgIntensity || FgIntensity != LastFgIntensity;
            LastBgIntensity = BgIntensity;
            LastFgIntensity = FgIntensity;
            interval = 1600 - v * 15;

            switch (testFunc) {
            case PixelTestDialog::TestFunctions::DIM:
                if (ColorChange) {
                    for (i = 0; i < chArray.Count(); i++) {
                        _outputManager->SetOneChannel(chArray[i] - 1, BgIntensity);
                    }
                }
                break;

            case PixelTestDialog::TestFunctions::TWINKLE:
                if (LastSequenceSpeed < 0 || _twinkleRatio != LastTwinkleRatio) {
                    LastSequenceSpeed = 0;
                    TwinkleState.Clear();
                    for (i = 0; i < chArray.Count(); i++) {
                        TestSeqIdx = static_cast<int>(rand01() * (double)_twinkleRatio);
                        TwinkleState.Add(TestSeqIdx == 0 ? -1 : 1);
                    }
                }
                for (i = 0; i < TwinkleState.Count(); i++) {
                    if (TwinkleState[i] < -1) {
                        // background
                        TwinkleState[i]++;
                    } else if (TwinkleState[i] > 1) {
                        // highlight
                        TwinkleState[i]--;
                    } else if (TwinkleState[i] == -1) {
                        // was background, now highlight for random period
                        TwinkleState[i] = static_cast<int>(rand01() * (double)interval + 100.0) / (double)_seqData.FrameTime();
                        _outputManager->SetOneChannel(chArray[i] - 1, FgIntensity);
                    } else {
                        // was on, now go to bg color for random period
                        TwinkleState[i] = -static_cast<int>(rand01() * (double)interval + 100.0) / (double)_seqData.FrameTime() * ((double)_twinkleRatio - 1.0);
                        _outputManager->SetOneChannel(chArray[i] - 1, BgIntensity);
                    }
                }
                break;

            case PixelTestDialog::TestFunctions::SHIMMER:
                if (ColorChange || curtime >= NextSequenceStart) {
                    ShimIntensity = (ShimIntensity == FgIntensity) ? BgIntensity : FgIntensity;
                    for (i = 0; i < chArray.Count(); i++) {
                        _outputManager->SetOneChannel(chArray[i] - 1, ShimIntensity);
                    }
                }
                if (curtime >= NextSequenceStart) {
                    NextSequenceStart = curtime + interval / 2;
                }
                break;

            case PixelTestDialog::TestFunctions::CHASE:
                // StatusBar1->SetStatusText(wxString::Format(_("chase curtime=%ld, NextSequenceStart=%ld"),curtime,NextSequenceStart));
                if (ColorChange || curtime >= NextSequenceStart) {
                    for (i = 0; i < chArray.Count(); i++) {
                        v = (i % _chaseGrouping) == TestSeqIdx ? FgIntensity : BgIntensity;
                        _outputManager->SetOneChannel(chArray[i] - 1, v);
                    }
                }
                if (curtime >= NextSequenceStart) {
                    NextSequenceStart = curtime + interval;
                    TestSeqIdx = (TestSeqIdx + 1) % _chaseGrouping;
                    if (TestSeqIdx >= chArray.Count())
                        TestSeqIdx = 0;
                }
                StatusBar1->SetLabelText(wxString::Format(_("Testing %ld channels; chase now at ch# %d"), static_cast<long>(chArray.Count()), TestSeqIdx)); // show current ch# -DJ
                break;
            default:
                break;
            }
        } break;

        case 1:
            // RGB tests
            v = Slider_Speed->GetValue(); // 0-100
            BgColor[0] = Slider_RGB_BG_R->GetValue();
            BgColor[1] = Slider_RGB_BG_G->GetValue();
            BgColor[2] = Slider_RGB_BG_B->GetValue();
            FgColor[0] = Slider_RGB_H_R->GetValue();
            FgColor[1] = Slider_RGB_H_G->GetValue();
            FgColor[2] = Slider_RGB_H_B->GetValue();

            interval = 1600 - v * 15;
            for (ColorChange = false, i = 0; i < 3; i++) {
                ColorChange |= (BgColor[i] != LastBgColor[i]);
                ColorChange |= (FgColor[i] != LastFgColor[i]);
                LastBgColor[i] = BgColor[i];
                LastFgColor[i] = FgColor[i];
            }
            switch (testFunc) {
            case PixelTestDialog::TestFunctions::DIM:
                if (ColorChange) {
                    for (i = 0; i < chArray.Count(); i++) {
                        _outputManager->SetOneChannel(chArray[i] - 1, BgColor[i % 3]);
                    }
                }
                break;

            case PixelTestDialog::TestFunctions::TWINKLE:
                if (LastSequenceSpeed < 0 || LastTwinkleRatio != _twinkleRatio) {
                    LastSequenceSpeed = 0;
                    TwinkleState.Clear();
                    for (i = 0; i < chArray.Count() - 2; i += 3) {
                        TestSeqIdx = static_cast<int>(rand01() * (double)_twinkleRatio);
                        TwinkleState.Add(TestSeqIdx == 0 ? -1 : 1);
                    }
                }
                for (i = 0; i < TwinkleState.Count(); i++) {
                    if (TwinkleState[i] < -1) {
                        // background
                        TwinkleState[i]++;
                    } else if (TwinkleState[i] > 1) {
                        // highlight
                        TwinkleState[i]--;
                    } else if (TwinkleState[i] == -1) {
                        // was background, now highlight for random period
                        TwinkleState[i] = static_cast<int>(rand01() * (double)interval + 100.0) / (double)_seqData.FrameTime();
                        TestSeqIdx = i * 3;
                        _outputManager->SetOneChannel(chArray[TestSeqIdx] - 1, FgColor[0]);
                        _outputManager->SetOneChannel(chArray[TestSeqIdx + 1] - 1, FgColor[1]);
                        _outputManager->SetOneChannel(chArray[TestSeqIdx + 2] - 1, FgColor[2]);
                    } else {
                        // was on, now go to bg color for random period
                        TwinkleState[i] = -static_cast<int>(rand01() * (double)interval + 100.0) / (double)_seqData.FrameTime() * ((double)_twinkleRatio - 1.0);
                        TestSeqIdx = i * 3;
                        _outputManager->SetOneChannel(chArray[TestSeqIdx] - 1, BgColor[0]);
                        _outputManager->SetOneChannel(chArray[TestSeqIdx + 1] - 1, BgColor[1]);
                        _outputManager->SetOneChannel(chArray[TestSeqIdx + 2] - 1, BgColor[2]);
                    }
                }
                break;
            case PixelTestDialog::TestFunctions::SHIMMER:
                if (ColorChange || curtime >= NextSequenceStart) {
                    ShimColor = (ShimColor == FgColor) ? BgColor : FgColor;
                    for (i = 0; i < chArray.Count(); i++) {
                        _outputManager->SetOneChannel(chArray[i] - 1, ShimColor[i % 3]);
                    }
                }
                if (curtime >= NextSequenceStart) {
                    NextSequenceStart = curtime + interval / 2;
                }
                break;
            case PixelTestDialog::TestFunctions::CHASE:
                if (ColorChange || curtime >= NextSequenceStart) {
                    for (i = 0; i < chArray.Count(); i++) {
                        v = (i / 3 % _chaseGrouping) == TestSeqIdx ? FgColor[i % 3] : BgColor[i % 3];
                        _outputManager->SetOneChannel(chArray[i] - 1, v);
                    }
                }
                if (curtime >= NextSequenceStart) {
                    NextSequenceStart = curtime + interval;
                    TestSeqIdx = (TestSeqIdx + 1) % _chaseGrouping;
                    if (TestSeqIdx >= (chArray.Count() + 2) / 3)
                        TestSeqIdx = 0;
                }
                StatusBar1->SetLabelText(wxString::Format(_("Testing %ld channels; chase now at ch# %d"), static_cast<long>(chArray.Count()), TestSeqIdx)); // show current ch# -DJ
                break;
            default:
                break;
            }
            break;

        case 2:
            // RGB Cycle
            v = Slider_Speed->GetValue(); // 0-100
            if (testFunc == PixelTestDialog::TestFunctions::DIM) {
                // color mixing
                if (v != LastSequenceSpeed) {
                    frequency = v / 1000.0 + 0.05;
                    LastSequenceSpeed = v;
                }
                BgColor[0] = sin(frequency * TestSeqIdx + 0.0) * 127 + 128;
                BgColor[1] = sin(frequency * TestSeqIdx + 2.0) * 127 + 128;
                BgColor[2] = sin(frequency * TestSeqIdx + 4.0) * 127 + 128;
                TestSeqIdx++;
                for (i = 0; i < chArray.Count(); ++i) {
                    _outputManager->SetOneChannel(chArray[i] - 1, BgColor[i % 3]);
                }
            } else if (testFunc == PixelTestDialog::TestFunctions::RGBW) {
                if (v != LastSequenceSpeed) {
                    interval = (101 - v) * 50;
                    NextSequenceStart = curtime + interval;
                    LastSequenceSpeed = v;
                }
                if (curtime >= NextSequenceStart) {
                    // blank everything first
                    for (i = 0; i < chArray.Count(); i++) {
                        _outputManager->SetOneChannel(chArray[i] - 1, 0);
                    }
                    switch (rgbCycle) {
                    case 0: // red
                        for (i = 0; i < chArrayR.Count(); i++) {
                            _outputManager->SetOneChannel(chArrayR[i] - 1, 255);
                        }
                        break;
                    case 1: // green
                        for (i = 0; i < chArrayG.Count(); i++) {
                            _outputManager->SetOneChannel(chArrayG[i] - 1, 255);
                        }
                        break;
                    case 2: // blue
                        for (i = 0; i < chArrayB.Count(); i++) {
                            _outputManager->SetOneChannel(chArrayB[i] - 1, 255);
                        }
                        break;
                    case 3: // white
                        for (i = 0; i < chArrayW.Count(); i++) {
                            _outputManager->SetOneChannel(chArrayW[i] - 1, 255);
                        }
                        break;
                    }
                    rgbCycle = (rgbCycle + 1) % 4;
                    NextSequenceStart += interval;
                }
            } else {
                // RGB cycle
                if (v != LastSequenceSpeed) {
                    interval = (101 - v) * 50;
                    NextSequenceStart = curtime + interval;
                    LastSequenceSpeed = v;
                }
                if (curtime >= NextSequenceStart) {
                    for (i = 0; i < chArray.Count(); i++) {
                        switch (rgbCycle) {
                        case 3:
                            v = 255;
                            break;
                        default:
                            v = (i % 3) == rgbCycle ? 255 : 0;
                            break;
                        }
                        _outputManager->SetOneChannel(chArray[i] - 1, v);
                    }
                    rgbCycle = (rgbCycle + 1) % _chaseGrouping;
                    NextSequenceStart += interval;
                }
            }

            // we set the 50th pixel to white at 50%
            if (fiftieth) {
                for (i = 3 * (50 - 1); i < chArray.Count(); i += 150) {
                    for (uint8_t j = 0; j < 3; j++) {
                        _outputManager->SetOneChannel(chArray[i + j] - 1, 128);
                    }
                }
            }
            break;
        }
    }
}

char PixelTestDialog::GetChannelColour(long ch)
{
    // assume the channel is from the same model as the last channel ... this saves looking through all the models first
    if (_lastModel != nullptr) {
        char c = _lastModel->GetModelAbsoluteChannelColour(ch);
        if (c != ' ') {
            return c;
        }
    }

    for (const auto& it : _models) {
        char c = it->GetModelAbsoluteChannelColour(ch);
        if (c != ' ') {
            _lastModel = it;
            return c;
        }
    }

    return ' ';
}

void PixelTestDialog::OnCheckBox_OutputToLightsClick(wxCommandEvent& event)
{
    if (CheckBox_OutputToLights->IsChecked()) {
        if (_outputManager->IsOutputOpenInAnotherProcess()) {
            DisplayWarning("Another process seems to be outputting to lights right now. This may not generate the result expected.", this);
        }

        xLightsFrame* f = (xLightsFrame*)GetParent();

        if (!f->ForceEnableOutputs(false)) {
            DisplayWarning("At least one output could not be started. See log file for details.", this);
        }
        Timer1.Start(50, wxTIMER_CONTINUOUS);
    } else {
        Timer1.Stop();
        wxTimerEvent ev(Timer1);
        OnTimer1Trigger(ev);
        _outputManager->StopOutput();
    }
}

std::string PixelTestDialog::SerialiseSettings()
{
    int standardFunction = 0;
    if (RadioButton_Standard_Chase->GetValue()) {
        standardFunction = 1;
    } else if (RadioButton_Standard_Chase13->GetValue()) {
        standardFunction = 2;
    } else if (RadioButton_Standard_Chase14->GetValue()) {
        standardFunction = 3;
    } else if (RadioButton_Standard_Chase15->GetValue()) {
        standardFunction = 4;
    } else if (RadioButton_Standard_Alternate->GetValue()) {
        standardFunction = 5;
    } else if (RadioButton_Standard_Twinkle5->GetValue()) {
        standardFunction = 6;
    } else if (RadioButton_Standard_Twinkle10->GetValue()) {
        standardFunction = 7;
    } else if (RadioButton_Standard_Twinkle25->GetValue()) {
        standardFunction = 8;
    } else if (RadioButton_Standard_Twinkle50->GetValue()) {
        standardFunction = 9;
    } else if (RadioButton_Standard_Shimmer->GetValue()) {
        standardFunction = 10;
    } else if (RadioButton_Standard_Background->GetValue()) {
        standardFunction = 11;
    }

    int rgbFunction = 0;
    if (RadioButton_RGB_Chase->GetValue()) {
        rgbFunction = 1;
    } else if (RadioButton_RGB_Chase13->GetValue()) {
        rgbFunction = 2;
    } else if (RadioButton_RGB_Chase14->GetValue()) {
        rgbFunction = 3;
    } else if (RadioButton_RGB_Chase15->GetValue()) {
        rgbFunction = 4;
    } else if (RadioButton_RGB_Alternate->GetValue()) {
        rgbFunction = 5;
    } else if (RadioButton_RGB_Twinkle5->GetValue()) {
        rgbFunction = 6;
    } else if (RadioButton_RGB_Twinkle10->GetValue()) {
        rgbFunction = 7;
    } else if (RadioButton_RGB_Twinkle25->GetValue()) {
        rgbFunction = 8;
    } else if (RadioButton_RGB_Twinkle50->GetValue()) {
        rgbFunction = 9;
    } else if (RadioButton_RGB_Shimmer->GetValue()) {
        rgbFunction = 10;
    } else if (RadioButton_RGB_Background->GetValue()) {
        rgbFunction = 11;
    }

    int rgbCycleFunction = 0;
    if (RadioButton_RGBCycle_ABC->GetValue()) {
        rgbCycleFunction = 1;
    } else if (RadioButton_RGBCycle_ABCAll->GetValue()) {
        rgbCycleFunction = 2;
    } else if (RadioButton_RGBCycle_ABCAllNone->GetValue()) {
        rgbCycleFunction = 3;
    } else if (RadioButton_RGBCycle_MixedColors->GetValue()) {
        rgbCycleFunction = 4;
    } else if (RadioButton_RGBCycle_RGBW->GetValue()) {
        rgbCycleFunction = 5;
    }

    int speed = Slider_Speed->GetValue();

    int standardBackground = Slider_Standard_Background->GetValue();
    int standardHighlight = Slider_Standard_Highlight->GetValue();

    int rgbBackgroundR = Slider_RGB_BG_R->GetValue();
    int rgbBackgroundG = Slider_RGB_BG_G->GetValue();
    int rgbBackgroundB = Slider_RGB_BG_B->GetValue();
    int rgbHighlightR = Slider_RGB_H_R->GetValue();
    int rgbHighlightG = Slider_RGB_H_G->GetValue();
    int rgbHighlightB = Slider_RGB_H_B->GetValue();

    bool suspend = CheckBox_SuppressUnusedOutputs->GetValue();

    int notebookSelection = Notebook2->GetSelection();

    return wxString::Format("%d|%d|%d|%d|%d|%d|%d|%d|%d|%d|%d|%d|%s|%d",
                            speed,
                            standardFunction, standardBackground, standardHighlight,
                            rgbFunction, rgbBackgroundR, rgbBackgroundG, rgbBackgroundB,
                            rgbHighlightR, rgbHighlightG, rgbHighlightB,
                            rgbCycleFunction, suspend ? "T" : "F", notebookSelection)
        .ToStdString();
}

void PixelTestDialog::DeserialiseSettings(const std::string& settings)
{
    if (settings == "") {
        // set defaults for fresh install
        Notebook2->SetSelection(2); // 2 - RGB Cycle
        RadioButton_RGBCycle_ABC->SetValue(true); // RGB Cycle A-B-C
        return;
    }

    wxArrayString values = wxSplit(settings, '|');

    TestButtonsOff();
    RadioButton_Standard_Off->SetValue(false);
    RadioButton_RGB_Off->SetValue(false);
    RadioButton_RGBCycle_Off->SetValue(false);

    if (values.size() >= 12) {
        Slider_Speed->SetValue(wxAtoi(values[0]));
        switch (wxAtoi(values[1])) {
        case 1:
            RadioButton_Standard_Chase->SetValue(true);
            break;
        case 2:
            RadioButton_Standard_Chase13->SetValue(true);
            break;
        case 3:
            RadioButton_Standard_Chase14->SetValue(true);
            break;
        case 4:
            RadioButton_Standard_Chase15->SetValue(true);
            break;
        case 5:
            RadioButton_Standard_Alternate->SetValue(true);
            break;
        case 6:
            RadioButton_Standard_Twinkle5->SetValue(true);
            break;
        case 7:
            RadioButton_Standard_Twinkle10->SetValue(true);
            break;
        case 8:
            RadioButton_Standard_Twinkle25->SetValue(true);
            break;
        case 9:
            RadioButton_Standard_Twinkle50->SetValue(true);
            break;
        case 10:
            RadioButton_Standard_Shimmer->SetValue(true);
            break;
        case 11:
            RadioButton_Standard_Background->SetValue(true);
            break;
        default:
            RadioButton_Standard_Off->SetValue(true);
            break;
        }
        Slider_Standard_Background->SetValue(wxAtoi(values[2]));
        Slider_Standard_Highlight->SetValue(wxAtoi(values[3]));

        switch (wxAtoi(values[4])) {
        case 1:
            RadioButton_RGB_Chase->SetValue(true);
            break;
        case 2:
            RadioButton_RGB_Chase13->SetValue(true);
            break;
        case 3:
            RadioButton_RGB_Chase14->SetValue(true);
            break;
        case 4:
            RadioButton_RGB_Chase15->SetValue(true);
            break;
        case 5:
            RadioButton_RGB_Alternate->SetValue(true);
            break;
        case 6:
            RadioButton_RGB_Twinkle5->SetValue(true);
            break;
        case 7:
            RadioButton_RGB_Twinkle10->SetValue(true);
            break;
        case 8:
            RadioButton_RGB_Twinkle25->SetValue(true);
            break;
        case 9:
            RadioButton_RGB_Twinkle50->SetValue(true);
            break;
        case 10:
            RadioButton_RGB_Shimmer->SetValue(true);
            break;
        case 11:
            RadioButton_RGB_Background->SetValue(true);
            break;
        default:
            RadioButton_RGB_Off->SetValue(true);
            break;
        }
        Slider_RGB_BG_R->SetValue(wxAtoi(values[5]));
        Slider_RGB_BG_G->SetValue(wxAtoi(values[6]));
        Slider_RGB_BG_B->SetValue(wxAtoi(values[7]));
        Slider_RGB_H_R->SetValue(wxAtoi(values[8]));
        Slider_RGB_H_G->SetValue(wxAtoi(values[9]));
        Slider_RGB_H_B->SetValue(wxAtoi(values[10]));

        switch (wxAtoi(values[11])) {
        case 1:
            RadioButton_RGBCycle_ABC->SetValue(true);
            break;
        case 2:
            RadioButton_RGBCycle_ABCAll->SetValue(true);
            break;
        case 3:
            RadioButton_RGBCycle_ABCAllNone->SetValue(true);
            break;
        case 4:
            RadioButton_RGBCycle_MixedColors->SetValue(true);
            break;
        case 5:
            RadioButton_RGBCycle_RGBW->SetValue(true);
            break;
        default:
            RadioButton_RGBCycle_Off->SetValue(true);
            break;
        }

        if (values.size() >= 13) {
            if (values[12] == "T") {
                CheckBox_SuppressUnusedOutputs->SetValue(true);
            }
        }

        if (values.size() >= 14) {
            switch (wxAtoi(values[13])) {
            case 1: 
                Notebook2->SetSelection(1); // 1 - RGB Chase
                break;
            case 2: 
                Notebook2->SetSelection(2); // 2 - RGB Cycle
                break;
            default: 
                Notebook2->SetSelection(0); // 0 - Standard
                break;
            }
        }
    }
}

void PixelTestDialog::OnClose(wxCloseEvent& event)
{
    if (CheckBox_OutputToLights->IsChecked()) {
        Timer1.Stop();
        _outputManager->AllOff();
        _outputManager->StopOutput();
    }

    wxConfigBase* config = wxConfigBase::Get();
    config->Write("xLightsTestSettings", wxString(SerialiseSettings()));

    EndDialog(0);
}

void PixelTestDialog::SetCheckBoxItemFromTracker(wxTreeListCtrl* tree, wxTreeListItem item, wxCheckBoxState parentState)
{
    wxTreeListItem i = tree->GetFirstChild(item);
    while (i != nullptr) {
        if (tree->GetItemText(i) == "Dummy") {
            if (tree->GetCheckedState(i) != parentState) {
                tree->CheckItem(i, parentState);
            }
        } else {
            TestItemBase* tc = (TestItemBase*)tree->GetItemData(i);
            auto state = tc->GetState(_channelTracker);
            if (tree->GetCheckedState(i) != state) {
                tree->CheckItem(i, state);
            }
            SetCheckBoxItemFromTracker(tree, i, tree->GetCheckedState(i));
        }
        i = tree->GetNextSibling(i);
    }
}

void PixelTestDialog::OnCheckBox_SuppressUnusedOutputsClick(wxCommandEvent& event)
{
    SetSuspend(CheckBox_SuppressUnusedOutputs->GetValue());
}

void PixelTestDialog::SetSuspend(bool suspend)
{
    if (suspend) {
        auto outputs = _outputManager->GetOutputs();
        for (const auto& it : outputs) {
            if (_channelTracker.AreAnyIncluded(it->GetStartChannel(), it->GetEndChannel())) {
                it->Suspend(false);
            } else {
                it->Suspend(true);
            }
        }
    } else {
        _outputManager->SuspendAll(false);
    }
}

void PixelTestDialog::OnNotebook1PageChanged(wxNotebookEvent& event)
{
    // need to go through all items in the tree on the selected page and update them based on channels
    wxTreeListCtrl* tree = (event.GetSelection() == 0 ? TreeListCtrl_Outputs : (event.GetSelection() == 1 ? TreeListCtrl_ModelGroups : (event.GetSelection() == 2 ? TreeListCtrl_Models : (event.GetSelection() == 4 ? TreeListCtrl_Controllers : nullptr))));
    if (tree != nullptr) {
        SetCheckBoxItemFromTracker(tree, tree->GetRootItem(), wxCheckBoxState::wxCHK_UNCHECKED);
    }
    else
    {
        UpdateVisualModelFromTracker();
        RenderModel();
    }
}

void PixelTestDialog::OnCheckBox_Tag50thClick(wxCommandEvent& event)
{
}

void PixelTestDialog::OnChoice_VisualModelSelect(wxCommandEvent& event)
{
    SelectVisualModel(Choice_VisualModel->GetStringSelection().ToStdString());
}
