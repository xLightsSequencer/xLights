#ifndef TESTPRESET_H
#define TESTPRESET_H

#include <vector>
#include <string>

class wxXmlNode;

class TestPreset
{
protected:

    #pragma region Member Variables
	std::string _name;
	std::vector<int> _channels;
    #pragma endregion Member Variables

public:

    #pragma region Constructors and Destructors
    TestPreset(wxXmlNode* node);
    TestPreset(const std::string& name);
    virtual ~TestPreset() {};
    #pragma endregion Constructors and Destructors

    wxXmlNode* Save();

    #pragma region Getters and Setters
    std::string GetName() { return _name; }
    std::vector<int> GetChannels() const { return _channels; }
    void AddChannel(int ch);
    void AddChannelRange(int start, int end);
    #pragma endregion Getters and Setters
};

 #endif
