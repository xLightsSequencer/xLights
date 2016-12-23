#ifndef E131OUTPUT_H
#define E131OUTPUT_H

#include "IPOutput.h"
#include <wx/socket.h>

// ******************************************************
// * This class represents a single universe for E1.31
// * Methods should be called with: 0 <= chindex <= 511
// ******************************************************

#define E131_PACKET_HEADERLEN 126
#define E131_PACKET_LEN (E131_PACKET_HEADERLEN + 512)
#define E131_SYNCPACKET_LEN 49
#define E131_PORT 5568
#define XLIGHTS_UUID "c0de0080-c69b-11e0-9572-0800200c9a66"

class E131Output : public IPOutput
{
    wxByte _data[E131_PACKET_LEN];
    wxByte _sequenceNum;
    wxIPV4address _remoteAddr;
    wxDatagramSocket *_datagram;

    // in case it is a multi universe e131
    int _numUniverses;
    std::list<Output*> _outputs;

    void CreateMultiUniverses();

public:
    E131Output(wxXmlNode* node);
    E131Output();
    virtual std::list<Output*> GetOutputs() const override { return _outputs; }
    virtual ~E131Output() override;
    virtual wxXmlNode* Save() override;
    virtual bool Open() override;
    virtual void Close() override;
    virtual void StartFrame(long msec) override;
    virtual void EndFrame() override;
    virtual void ResetFrame() override;
    virtual void SetOneChannel(int channel, unsigned char data) override;
    virtual void SetManyChannels(int channel, unsigned char* data, size_t size) override;
    virtual void AllOff() override;
    virtual std::string GetType() const override { return OUTPUT_E131; }
    virtual std::string GetLongDescription() const override;
    static void SendSync(int syncUniverse);
    virtual std::string GetChannelMapping(int ch) const override;
    virtual bool IsOutputCollection() const override { return _numUniverses > 1; }
    virtual int GetMaxChannels() const override { return 512; }
    virtual bool IsValidChannelCount(int channelCount) const override { return channelCount > 0 && channelCount <= 512; }
    virtual int GetUniverses() const override { return _numUniverses; }
    virtual void SetTransientData(int on, int startChannel, int nullnumber) override;
    virtual int GetEndChannel() const override;

#ifndef EXCLUDENETWORKUI
    virtual int Configure(wxWindow* parent, OutputManager& outputManager) override;
#endif
};

 #endif