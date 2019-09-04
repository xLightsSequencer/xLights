#ifndef SERIALOUTPUT_H
#define SERIALOUTPUT_H

#include "Output.h"
#include "serial.h"

class SerialOutput : public Output
{

protected:

    #pragma region Member Variables
    SerialPort* _serial;
    char _serialConfig[4];
    #pragma endregion Member Variables

    virtual void Save(wxXmlNode* node) override;

public:

    #pragma region Constructors and Destructors
    SerialOutput(wxXmlNode* node);
    SerialOutput(SerialOutput* output);
    SerialOutput();
    virtual ~SerialOutput() override;
    #pragma endregion Constructors and Destructors

    #pragma region Static Functions
    static std::list<std::string> GetAvailableSerialPorts();
    static std::list<std::string> GetPossibleSerialPorts();
    static std::list<std::string> GetPossibleBaudRates();
    #pragma endregion Static Functions

    virtual wxXmlNode* Save() override;

    #pragma region Getters and Setters
    virtual bool IsIpOutput() const override { return false; }
    virtual bool IsSerialOutput() const override { return true; }
    virtual std::string GetChannelMapping(int32_t ch) const override;
    virtual std::string GetLongDescription() const override;
    virtual std::string GetPingDescription() const override;
    virtual int GetMaxChannels() const override { return 0; }
    virtual std::string GetBaudRateString() const override;
    virtual std::string GetSetupHelp() const = 0;
    virtual bool AllowsBaudRateSetting() const { return true; }
    virtual size_t TxNonEmptyCount() const override;
    virtual bool TxEmpty() const override;
    int GetId() const { return _universe; }
    void SetId(int id) { _universe = id; _dirty = true; }
    virtual std::string GetSortName() const override { return GetCommPort(); }
    #pragma endregion Getters and Setters

    #pragma region Operators
    bool operator==(const SerialOutput& output) const;
    #pragma endregion Operators

    #pragma region Start and Stop
    virtual bool Open() override;
    virtual void Close() override;
    #pragma endregion Start and Stop

    virtual void StartFrame(long msec) override;

    // Create a new serial type of the specified type but copy across this objects settings
    SerialOutput* Mutate(const std::string& newtype);

    PINGSTATE Ping() const override;
    bool CanPing() const override { return true; }

    #pragma region UI
#ifndef EXCLUDENETWORKUI
    virtual Output* Configure(wxWindow* parent, OutputManager* outputManager, ModelManager* modelManager) override;
#endif
    #pragma endregion UI
};

 #endif
