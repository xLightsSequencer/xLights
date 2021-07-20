#pragma once

#include <string>
#include <list>
#include <map>

class Controller;
class ControllerEthernet;
class BaseController;

class ScanObject
{
};

class IPObject : ScanObject
{
	void CheckPort80();

public:
	ControllerEthernet* _xLightsController = nullptr;
	ControllerEthernet* _discovered = nullptr;
	BaseController* _controller = nullptr;
	std::string _ip;
	bool _port80 = false;
	bool _pinged = false;
	bool _displayed = false;
	std::string _viaProxy;
	std::list<std::string> _otherIPs;
	std::map<std::string, std::string> _otherData;
	std::string _type;
	std::string _version;
	std::string _mode;
	std::string _name;
	std::string _mac;
	std::string _macVendor;
	std::string _banks;
	bool _deepScan = false;
	std::string _emittingData;
	int _xSchedulePort = 0;
	IPObject(const IPObject& old)
	{
		_xLightsController = old._xLightsController;
		_discovered = old._discovered;
		_controller = old._controller;
		_ip = old._ip;
		_port80 = old._port80;
		_pinged = old._pinged;
		_displayed = old._displayed;
		_viaProxy = old._viaProxy;
		_otherIPs = old._otherIPs;
		_otherData = old._otherData;
		_type = old._type;
		_version = old._version;
		_mode = old._mode;
		_name = old._name;
		_mac = old._mac;
		_macVendor = old._macVendor;
		_deepScan = old._deepScan;
		_banks = old._banks;
		_emittingData = old._emittingData;
		_xSchedulePort = old._xSchedulePort;
	}
	IPObject(const std::string& ip, const std::string& viaProxy, bool pinged);
	void SetXLightsController(ControllerEthernet* ce) { _xLightsController = ce; }
	void SetController(BaseController* bc) { _controller = bc; }
	void SetDiscovered(ControllerEthernet* ce) { _discovered = ce; }
	bool InSameSubnet(const std::string& ip) const;
	bool operator<(const IPObject& ip2);
	void Displayed() { _displayed = true; }
	std::string GetDisplayIP() const
	{
		if (_pinged) {
			return _ip + " (online)";
		}
		return _ip;
	}
};

class ComputerObject : ScanObject
{
public:
	std::list<std::string> _ips;
	std::string _name;
	std::list<std::string> _routes;
	std::string _xLightsFolder;
	std::string _xScheduleFolder;
};

class xLightsObject : ScanObject
{
public:
	std::string _showFolder;
	std::list<Controller*> _controllers;
	std::string _globalFPPProxy;
	std::string _forceIP;
};
