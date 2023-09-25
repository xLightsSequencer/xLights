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

#include "IPOutput.h"

#include <wx/socket.h>

#pragma region KINET Constants
#define KINET_V1_PACKET_HEADERLEN 21
#define KINET_V2_PACKET_HEADERLEN 24
#define KINET_PACKET_LEN (std::max(KINET_V1_PACKET_HEADERLEN, KINET_V2_PACKET_HEADERLEN) + 512)
#define KINET_PORT 6038
#pragma endregion 

class KinetOutput : public IPOutput
{
    #pragma region Member Variables
    uint8_t _data[KINET_PACKET_LEN];
    uint32_t _sequenceNum = 0;
    wxIPV4address _remoteAddr;
    wxDatagramSocket *_datagram = nullptr;
    int _version = 2;
    #pragma endregion

    #pragma region Private Functiona
    void OpenDatagram();
	int GetHeaderPacketLength() const {
		if (_version == 1) return KINET_V1_PACKET_HEADERLEN;
		return KINET_V2_PACKET_HEADERLEN;
	}
	void PopulateHeader();
    #pragma endregion

public:

    #pragma region Constructors and Destructors
    KinetOutput(wxXmlNode* node, bool isActive);
    KinetOutput();
    KinetOutput(const KinetOutput& from);
    virtual ~KinetOutput() override;
    virtual wxXmlNode* Save() override;
    virtual Output* Copy() override
    {
        return new KinetOutput(*this);
    }
    #pragma endregion 

    #pragma region Static Functions
    #pragma endregion 

    #pragma region Getters and Setters

    virtual int GetVersion() const { return _version; }
    virtual void SetVersion(int version) { if (_version != version) { _version = version; _dirty = true; } }

    virtual std::string GetType() const override { return OUTPUT_KINET; }

    virtual std::string GetLongDescription() const override;

    virtual int GetMaxChannels() const override { return 512; }
    virtual bool IsValidChannelCount(int32_t channelCount) const override { return channelCount > 0 && channelCount <= 512; }

    virtual std::string GetExport() const override;
    #pragma region 

    #pragma region Start and Stop
    virtual bool Open() override;
    virtual void Close() override;
    #pragma endregion 

    #pragma region Frame Handling
    virtual void StartFrame(long msec) override;
    virtual void EndFrame(int suppressFrames) override;
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
    virtual void RemoveProperties(wxPropertyGrid* propertyGrid) override;
    #endif
    #pragma endregion UI
};
