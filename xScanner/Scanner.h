#pragma once

#include <string>
#include <mutex>

#include "ScanObject.h"
#include "../xLights/outputs/OutputManager.h"
#include "../xLights/Discovery.h"

class wxProgressDialog;
class xScannerFrame;

class Scanner
{
public:

	std::mutex _mutex;
	OutputManager _om;
	Discovery* _discovery = nullptr;
	ComputerObject _computer;
	std::list<IPObject> _ips;
	xLightsObject _xLights;
	int _progressv = 1;
	wxWindow* _frame = nullptr;
	std::string _showDir;
	std::list<std::string> _ipscanned; //subnets + proxies scanned for ips
	std::list<IPObject> GetIPsInSameSubnet(const std::string& ip) const;
	std::list<IPObject> GetUndisplayedIPs() const;
	std::list<IPObject> GetProxiedBy(const std::string& ip) const;

	void SetDisplayed(const IPObject& ip);
	void IPScan(const std::string& ip, const std::string& proxy);
	void ComputerScan();
	void xLightsScan();
	bool Scanned(const std::string& ip, const std::string& proxy);
	IPObject* GetIP(const std::string& ip);
	void CheckXSchedule(IPObject& ip, int port);
	void IPScan(IPObject& it);
	void SendProgress(int progress, const std::string& msg);

	void PreScan(xScannerFrame* frame);
	void Scan(xScannerFrame* frame);
};
