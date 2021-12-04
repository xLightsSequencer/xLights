#include "TwinklyOutput.h"

#pragma region Constructors and Destructors
TwinklyOutput::TwinklyOutput(wxXmlNode* node) :
    IPOutput(node)
{
}
TwinklyOutput::TwinklyOutput()
{
}
TwinklyOutput::~TwinklyOutput()
{
}
wxXmlNode* TwinklyOutput::Save()
{
    return IPOutput::Save();
}
#pragma endregion

#pragma region Output implementation
inline std::string TwinklyOutput::GetType() const
{
    return OUTPUT_TWINKLY;
}

inline int TwinklyOutput::GetMaxChannels() const
{
    return MAX_CHANNELS;
}

inline bool TwinklyOutput::IsValidChannelCount(int32_t channelCount) const
{
    return channelCount > 0 && channelCount <= MAX_CHANNELS;
}

std::string TwinklyOutput::GetLongDescription() const
{
    std::string res = "Twinkly ";
    res += "[" + std::string(wxString::Format(wxT("%i"), _channels)) + "] ";
    return res;
}
#pragma endregion

#pragma region Start and Stop
bool TwinklyOutput::Open()
{
    return IPOutput::Open();
}
void TwinklyOutput::Close()
{
    IPOutput::Close();
}
#pragma endregion

#pragma region Frame Handling
void TwinklyOutput::StartFrame(long msec)
{
}

void TwinklyOutput::EndFrame(int suppressFrames)
{
}

void TwinklyOutput::ResetFrame()
{
}
#pragma endregion

#pragma region Frame Handling
void TwinklyOutput::SetOneChannel(int32_t channel, unsigned char data)
{
}
void TwinklyOutput::SetManyChannels(int32_t channel, unsigned char* data, size_t size)
{
}
void TwinklyOutput::AllOff()
{
}
#pragma endregion
