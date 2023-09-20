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

// https://tsp.esta.org/tsp/documents/docs/E1-31-2016.pdf
// https://tsp.esta.org/tsp/documents/docs/ANSI_E1-31-2018.pdf

#include "IPOutput.h"

#include <wx/socket.h>

#pragma region E1.31 Constants
#define E131_PACKET_HEADERLEN 126
#define E131_PACKET_LEN (E131_PACKET_HEADERLEN + 512)
#define E131_SYNCPACKET_LEN 49
#define E131_PORT 5568
#define E131_DEFAULT_PRIORITY 100
#define XLIGHTS_UUID "c0de0080-c69b-11e0-9572-0800200c9a66"
#pragma endregion

class E131Output : public IPOutput
{
    #pragma region Member Variables
    uint8_t _data[E131_PACKET_LEN];
    uint8_t _sequenceNum = 0;
    uint8_t _priority = E131_DEFAULT_PRIORITY;
    wxIPV4address _remoteAddr;
    wxDatagramSocket *_datagram = nullptr;

    // Deprecated properties only accessed for conversion
    int _numUniverses_CONVERT = 1;
    std::list<Output*> _outputs_CONVERT;
    #pragma endregion

    #pragma region Private Functiona
    // this is used to create any sub universes in this output
    void CreateMultiUniverses_CONVERT(int num);
    void OpenDatagram();
    #pragma endregion

public:

    #pragma region Constructors and Destructors
    E131Output(wxXmlNode* node, bool isActive);
    E131Output();
    E131Output(const E131Output& from);
    virtual ~E131Output() override;
    virtual wxXmlNode* Save() override;
    virtual Output* Copy() override
    {
        return new E131Output(*this);
    }
    #pragma endregion

    #pragma region Static Functions
    static void SendSync(int syncUniverse, const std::string& localIP);
    static std::string GetTag();
    #pragma endregion

    #pragma region Getters and Setters
    virtual int GetPriority() const { return _priority; }
    virtual void SetPriority(int priority) { if (_priority != priority) { _priority = priority; _dirty = true; } }

    virtual std::string GetType() const override { return OUTPUT_E131; }

    virtual std::string GetLongDescription() const override;

    virtual int GetMaxChannels() const override { return 512; }
    virtual bool IsValidChannelCount(int32_t channelCount) const override { return channelCount > 0 && channelCount <= 512; }

    virtual std::string GetExport() const override;

    // These are required because one e1.31 output can actually be multiple
    virtual std::list<Output*> GetOutputs_CONVERT() const override { return _outputs_CONVERT; }
    virtual bool IsOutputCollection_CONVERT() const override { return _numUniverses_CONVERT > 1; }
    virtual int GetUniverses_CONVERT() const override { return _numUniverses_CONVERT; }

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
    #pragma endregion UI
};
