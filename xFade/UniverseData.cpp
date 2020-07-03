#include "UniverseData.h"

std::string UniverseData::__leftTag = "";
std::string UniverseData::__rightTag = "";

UniverseData::UniverseData(int universe, const std::string& targetIP, const std::string& targetProtocol, std::list<int> excludedChannels) :
    _universe(universe),
    _targetIP(targetIP),
    _excludedChannels(excludedChannels)
{
    if (targetProtocol == "As per input")
    {
        _targetProtocol = 0;
    }
    else if (targetProtocol == "E1.31")
    {
        _targetProtocol = E131PORT;
    }
    else if (targetProtocol == "ArtNET")
    {
        _targetProtocol = ARTNETPORT;
    }
    else
    {
        wxASSERT(false);
    }
}

int UniverseData::GetLeftSequenceNum()
{
    std::unique_lock<std::mutex> lock(_lock);
    return _left.GetSequenceNum();
}

int UniverseData::GetRightSequenceNum()
{
    std::unique_lock<std::mutex> lock(_lock);
    return _right.GetSequenceNum();
}

bool UniverseData::UpdateLeft(int type, uint8_t* buffer, int size)
{
    std::unique_lock<std::mutex> lock(_lock);
    return _left.Update(type, buffer, size);
}

bool UniverseData::UpdateRight(int type, uint8_t* buffer, int size)
{
    std::unique_lock<std::mutex> lock(_lock);
    return _right.Update(type, buffer, size);
}

PacketData* UniverseData::GetOutput(PacketData* output, int leftBrightness, int rightBrightness, float pos)
{
    std::unique_lock<std::mutex> lock(_lock);

    if (_left._length == 0 && _right._length > 0)
    {
        _left.InitialiseLength(_right._type, _right._length, _universe);
    }
    else if (_right._length == 0 && _left._length > 0)
    {
        _right.InitialiseLength(_left._type, _left._length, _universe);
    }

    if (pos == 0.0)
    {
        PrepareData(output, &_left, _targetProtocol);
        output->ApplyBrightness(leftBrightness, _excludedChannels);
    }
    else if (pos == 1.0)
    {
        PrepareData(output, &_right, _targetProtocol);
        output->ApplyBrightness(rightBrightness, _excludedChannels);
    }
    else
    {
        int sz = std::min(_left.GetDataLength(), _right.GetDataLength());

        PacketData l = _left;
        PacketData r = _right;
        l.ApplyBrightness(leftBrightness, _excludedChannels);
        r.ApplyBrightness(rightBrightness, _excludedChannels);

        Blend(l.GetDataPtr(), r.GetDataPtr(), sz, pos, _excludedChannels);
        PrepareData(output, &l, _targetProtocol);
    }
    return output;
}

void UniverseData::Blend(uint8_t* buffer, uint8_t* blendBuffer, size_t channels, float pos, std::list<int> excludeChannels)
{
    float inv = 1.0 - pos;
    for (size_t i = 0; i < channels; ++i)
    {
        if (std::find(excludeChannels.begin(), excludeChannels.end(), i + 1) != excludeChannels.end())
        {
            if (pos < 0.5)
            {
                // dont change anything
            }
            else
            {
                *(buffer + i) = *(blendBuffer + i);
            }
        }
        else
        {
            *(buffer + i) = (uint8_t)((float) * (buffer + i) * inv + (float) * (blendBuffer + i) * pos);
        }
    }
}

void UniverseData::PrepareData(PacketData* target, PacketData* source, int protocol)
{
    if (protocol == 0 || source->_type == protocol)
    {
        // no conversion required
        target->CopyFrom(source, source->_type);
    }
    else
    {
        // conversion required
        target->CopyFrom(source, protocol);
    }
}