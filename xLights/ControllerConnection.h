
#pragma once

/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

class Model;

class ControllerConnection
{
public:
    ControllerConnection(Model* model);
    virtual ~ControllerConnection();

    [[nodiscard]] std::string GetName() const { return _name; }
    [[nodiscard]] std::string GetProtocol() const { return _protocol; }
    [[nodiscard]] int GetProtocolSpeed() const {return _protocolSpeed; }
    [[nodiscard]] int GetPort(int string = 1) const;
    [[nodiscard]] int GetBrightness() const { return _brightness; }
    [[nodiscard]] int GetStartNulls() const { return _startNulls; }
    [[nodiscard]] int GetEndNulls() const { return _endNulls; }
    [[nodiscard]] std::string GetColorOrder() const { return _colorOrder; }
    [[nodiscard]] int GetGroupCount() const { return _groupCount; }
    [[nodiscard]] float GetGamma() const { return _gamma; }
    [[nodiscard]] int GetReverse() const { return _reverse; }
    [[nodiscard]] int GetZigZag() const { return _zigzag; }
    [[nodiscard]] int GetDMXChannel() const { return _dmxChannel; }

    [[nodiscard]] bool IsValid() const;
    [[nodiscard]] bool IsBrightnessSet() const { return _brightnessIsSet; }
    [[nodiscard]] bool IsPixelProtocol() const;
    [[nodiscard]] bool IsSerialProtocol() const;
    [[nodiscard]] bool IsMatrixProtocol() const;
    [[nodiscard]] bool IsLEDPanelMatrixProtocol() const;
    [[nodiscard]] bool IsVirtualMatrixProtocol() const;
    [[nodiscard]] bool IsPWMProtocol() const;

    void SetName(std::string const& controller);
    void SetProtocol(std::string const& protocol);
    void SetSerialProtocolSpeed(int speed);
    void SetPort(int port);
    void SetBrightness(int brightness);
    void SetStartNulls(int nulls);
    void SetEndNulls(int nulls);
    void SetColorOrder(std::string const& color_order);
    void SetGroupCount(int grouping);
    void SetGamma(float gamma);
    void SetReverse(int reverse);
    void SetZigZag(int zigzag);
    void SetDMXChannel(int ch);
    void ClearBrightness();
    bool Rename(const std::string& oldName, const std::string& newName);

    // Smart Remote Functions
    void GetPortSR(int string, int& outport, int& outsr) const;
    [[nodiscard]] char GetSmartRemoteLetter() const;
    [[nodiscard]] char GetSmartRemoteLetterForString(int string = 1) const;
    [[nodiscard]] int GetSortableSmartRemote() const;
    [[nodiscard]] int GetSmartTs() const { return _smartRemoteTs; }
    [[nodiscard]] int GetSmartRemoteForString(int string = 1) const;
    void SetSmartRemote(int sr);
    void SetSRCascadeOnPort(bool cascade);
    void SetSRMaxCascade(int max);
    void SetSmartRemoteType(const std::string& type);
    void SetSmartRemoteTs(int ts);
    [[nodiscard]] int GetSmartRemote() const { return _smartRemote; }
    [[nodiscard]] bool GetSRCascadeOnPort() const { return _smartRemoteCascadeOnPort; }
    [[nodiscard]] int GetSRMaxCascade() const { return _smartRemoteMaxCascade; }
    [[nodiscard]] std::vector<std::string> GetSmartRemoteTypes() const;
    [[nodiscard]] std::string GetSmartRemoteType() const;
    [[nodiscard]] int GetSmartRemoteTypeIndex(const std::string& srType) const;
    [[nodiscard]] std::string GetSmartRemoteTypeName(int idx) const;
    [[nodiscard]] int GetSmartRemoteCount() const;

private:
    std::string _name {""};
    std::string _protocol {""};
    int _protocolSpeed {25000};
    int _port {0};
    int _brightness {100};
    int _startNulls {0};
    int _endNulls {0};
    std::string _colorOrder {"RGB"};
    int _groupCount {0};
    float _gamma {1.0};
    int _reverse {0};
    int _zigzag {0};
    int _dmxChannel {0};
    bool _brightnessIsSet {false};

    int _smartRemote {0};
    bool _smartRemoteCascadeOnPort {false};
    int _smartRemoteMaxCascade {1};
    int _smartRemoteTs {0};
    std::string _smartRemoteType {""};

    Model* _model;
};


