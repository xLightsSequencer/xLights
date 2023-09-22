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

// Protocol specification can be found here: http://openpixelcontrol.org/

#include "IPOutput.h"

#include <wx/socket.h>

#pragma region E1.31 Constants
#define OPC_PACKET_HEADERLEN 4
#define OPC_PORT 7890
#pragma endregion 

class OPCOutput : public IPOutput
{
    #pragma region Member Variables
    uint8_t* _data = nullptr;
    wxIPV4address _remoteAddr;
    wxSocketClient*_socket = nullptr;
    #pragma endregion

    #pragma region Private Functiona
    // this is used to create any sub universes in this output
    void OpenSocket();
    #pragma endregion

public:

    #pragma region Constructors and Destructors
    OPCOutput(wxXmlNode* node, bool isActive);
    OPCOutput();
    OPCOutput(const OPCOutput& from);
    virtual ~OPCOutput() override;
    virtual wxXmlNode* Save() override;
    virtual Output* Copy() override
    {
        return new OPCOutput(*this);
    }
#pragma endregion 

    #pragma region Getters and Setters
    virtual std::string GetType() const override { return OUTPUT_OPC; }

    virtual std::string GetLongDescription() const override;

    virtual void SetChannels(int32_t channels) override;
    virtual int GetMaxChannels() const override { return 65535; }
    virtual bool IsValidChannelCount(int32_t channelCount) const override { return channelCount > 0 && channelCount <= GetMaxChannels(); }

    virtual std::string GetExport() const override;
    
    virtual void SetTransientData(int32_t& startChannel, int nullnumber) override;
    #pragma region 

    #pragma region Start and Stop
    virtual bool Open() override;
    virtual void Close() override;
    #pragma endregion 

    #pragma region Frame Handling
    virtual void StartFrame(long msec) override;
    virtual void EndFrame(int suppressFrames) override;
    virtual void ResetFrame() override;
    #pragma endregion 

    #pragma region Data Setting
    virtual void SetOneChannel(int32_t channel, unsigned char data) override;
    virtual void SetManyChannels(int32_t channel, unsigned char* data, size_t size) override;
    virtual void AllOff() override;
    #pragma endregion
    
    
#pragma region UI
#ifndef EXCLUDENETWORKUI
    virtual void UpdateProperties(wxPropertyGrid* propertyGrid, Controller* c, ModelManager* modelManager, std::list<wxPGProperty*>& expandProperties) override;
    virtual void AddProperties(wxPropertyGrid* propertyGrid, wxPGProperty *before, Controller* c, bool allSameSize, std::list<wxPGProperty*>& expandProperties) override;
    virtual bool HandlePropertyEvent(wxPropertyGridEvent& event, OutputModelManager* outputModelManager, Controller* c) override;
    virtual void RemoveProperties(wxPropertyGrid* propertyGrid) override;
#endif
#pragma endregion

};
