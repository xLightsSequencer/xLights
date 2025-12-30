
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

namespace CONTROLLER_CONNECTION_DEFAULTS {

constexpr int DEFAULT_PROTOCOL_SPEED = 25000;
constexpr int DEFAULT_PORT = 0;
constexpr int DEFAULT_NULLS = 0;
constexpr int DEFAULT_BRIGHTNESS = 100;
constexpr int DEFAULT_GROUP_COUNT = 0;
constexpr int DEFAULT_REVERSE = 0;
constexpr int DEFAULT_ZIGZAG = 0;
constexpr float DEFAULT_GAMMA = 1.0f;
constexpr std::string DEFAULT_COLOR_ORDER = "RGB";

} // end namespace CTRL_CONN

#define CtrlProps ControllerConnection::CTRL_PROPS
#define CtrlDefs CONTROLLER_CONNECTION_DEFAULTS

class ControllerConnection
{
public:
    ControllerConnection(Model* model);
    virtual ~ControllerConnection();

    enum CTRL_PROPS {
        USE_SMART_REMOTE,
        START_NULLS_ACTIVE,
        END_NULLS_ACTIVE,
        BRIGHTNESS_ACTIVE,
        GAMMA_ACTIVE,
        COLOR_ORDER_ACTIVE,
        REVERSE_ACTIVE,
        GROUP_COUNT_ACTIVE,
        ZIG_ZAG_ACTIVE,
        TS_ACTIVE
    };

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
    [[nodiscard]] bool IsPixelProtocol() const;
    [[nodiscard]] bool IsSerialProtocol() const;
    [[nodiscard]] bool IsMatrixProtocol() const;
    [[nodiscard]] bool IsLEDPanelMatrixProtocol() const;
    [[nodiscard]] bool IsVirtualMatrixProtocol() const;
    [[nodiscard]] bool IsPWMProtocol() const;

    void SetProperty(enum CTRL_PROPS prop) { active_props[prop] = true; }
    void ClearProperty(enum CTRL_PROPS prop) { active_props[prop] = false; }
    void UpdateProperty(enum CTRL_PROPS prop, bool value) { value ? active_props[prop] = true : active_props[prop] = false; }
    [[nodiscard]] bool IsPropertySet(enum CTRL_PROPS prop) const { return active_props.at(prop); }

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
    int _protocolSpeed {CtrlDefs::DEFAULT_PROTOCOL_SPEED};
    int _port {CtrlDefs::DEFAULT_PORT};
    int _brightness {CtrlDefs::DEFAULT_BRIGHTNESS};
    int _startNulls {CtrlDefs::DEFAULT_NULLS};
    int _endNulls {CtrlDefs::DEFAULT_NULLS};
    std::string _colorOrder {CtrlDefs::DEFAULT_COLOR_ORDER};
    int _groupCount {CtrlDefs::DEFAULT_GROUP_COUNT};
    float _gamma {CtrlDefs::DEFAULT_GAMMA};
    int _reverse {CtrlDefs::DEFAULT_REVERSE};
    int _zigzag {CtrlDefs::DEFAULT_ZIGZAG};
    int _dmxChannel {0};
    bool _brightnessIsSet {false};

    int _smartRemote {0};
    bool _smartRemoteCascadeOnPort {false};
    int _smartRemoteMaxCascade {1};
    int _smartRemoteTs {0};
    std::string _smartRemoteType {""};
    
    std::map<int, bool> active_props = {
        { USE_SMART_REMOTE,   false },
        { START_NULLS_ACTIVE, false },
        { END_NULLS_ACTIVE,   false },
        { BRIGHTNESS_ACTIVE,  false },
        { GAMMA_ACTIVE,       false },
        { COLOR_ORDER_ACTIVE, false },
        { REVERSE_ACTIVE,     false },
        { GROUP_COUNT_ACTIVE, false },
        { ZIG_ZAG_ACTIVE,     false },
        { TS_ACTIVE,          false }
    };  // tracks the active status of the controller connection properties

    Model* _model;
};


