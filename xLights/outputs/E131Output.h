#ifndef E131OUTPUT_H
#define E131OUTPUT_H

#include "IPOutput.h"
#include <wx/socket.h>

// ******************************************************
// * This class represents a single universe for E1.31
// * Methods should be called with: 0 <= chindex <= 511
// ******************************************************

#pragma region E1.31 Constants
#define E131_PACKET_HEADERLEN 126
#define E131_PACKET_LEN (E131_PACKET_HEADERLEN + 512)
#define E131_SYNCPACKET_LEN 49
#define E131_PORT 5568
#define E131_DEFAULT_PRIORITY 100
#define XLIGHTS_UUID "c0de0080-c69b-11e0-9572-0800200c9a66"
#pragma endregion E1.31 Constants

class E131Output : public IPOutput
{
    #pragma region Member Variables
    uint8_t _data[E131_PACKET_LEN];
    uint8_t _sequenceNum = 0;
    uint8_t _priority = E131_DEFAULT_PRIORITY;
    wxIPV4address _remoteAddr;
    wxDatagramSocket *_datagram = nullptr;

    // in case it is a multi universe e131
    int _numUniverses = 1;
    std::list<Output*> _outputs;
	bool _autoStartChannels = false;
    #pragma endregion Member Variables

public:

    #pragma region Constructors and Destructors
    E131Output(wxXmlNode* node);
    E131Output();
    E131Output(E131Output* output);
    virtual ~E131Output() override;

    // this is used to create any sub universes in this output
    void CreateMultiUniverses(int num);
    #pragma endregion Constructors and Destructors

    #pragma region Static Functions
    static void SendSync(int syncUniverse);
    static std::string GetTag();
    #pragma endregion Static Functions

    #pragma region Getters and Setters
    virtual std::string GetType() const override { return OUTPUT_E131; }
    virtual std::string GetLongDescription() const override;
    virtual std::string GetChannelMapping(int32_t ch) const override;
    virtual int GetMaxChannels() const override { return 512; }
    virtual bool IsValidChannelCount(int32_t channelCount) const override { return channelCount > 0 && channelCount <= 512; }
    virtual int32_t GetEndChannel() const override;
    virtual std::string GetUniverseString() const override;
    virtual bool IsLookedUpByControllerName() const override;
	virtual bool IsAutoLayoutModels() const override { return _autoStartChannels; }
    virtual std::string GetExport() const override;

    // These are required because one e1.31 output can actually be multiple
    virtual std::list<Output*> GetOutputs() const override { return _outputs; }
    virtual bool IsOutputCollection() const override { return _numUniverses > 1; }
    virtual int GetUniverses() const override { return _numUniverses; }
    virtual void SetTransientData(int on, int32_t startChannel, int nullnumber) override;
    virtual Output* GetActualOutput(int32_t startChannel) override;
    virtual int GetPriority() const {return _priority; }
    virtual void SetPriority(int priority);

	virtual void SetAutoStartChannels(bool autoMode) { _autoStartChannels = autoMode; }
    #pragma region Getters and Setters

    virtual wxXmlNode* Save() override;

    #pragma region Start and Stop
    virtual bool Open() override;
    virtual void Close() override;
    void OpenDatagram();
    #pragma endregion Start and Stop

    #pragma region Frame Handling
    virtual void StartFrame(long msec) override;
    virtual void EndFrame(int suppressFrames) override;
    virtual void ResetFrame() override;
    #pragma endregion Frame Handling

    #pragma region Data Setting
    virtual void SetOneChannel(int32_t channel, unsigned char data) override;
    virtual void SetManyChannels(int32_t channel, unsigned char* data, size_t size) override;
    virtual void AllOff() override;
    #pragma endregion Data Setting
	
    #pragma region UI
#ifndef EXCLUDENETWORKUI
    virtual Output* Configure(wxWindow* parent, OutputManager* outputManager, ModelManager* modelManager) override;
#endif
    #pragma endregion UI
};
#endif
