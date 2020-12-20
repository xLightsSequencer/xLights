#include <wx/msgdlg.h>
#include <wx/config.h>
#include <wx/regex.h>
#include <wx/sstream.h>

#include <mutex>

#ifdef __WXMSW__
#include <iphlpapi.h>
#endif

//#define SINGLE_THREAD

#include "Scanner.h"
#include "xScannerMain.h"
#include "../xLights/Parallel.h"
#include "../xLights/outputs/OutputManager.h"
#include "../xLights/UtilFunctions.h"
#include "../xLights/outputs/IPOutput.h"
#include "../xLights/outputs/ControllerEthernet.h"
#include "../xLights/Discovery.h"
#include "../xLights/outputs/ZCPPOutput.h"
#include "../xLights/outputs/DDPOutput.h"
#include "../xLights/outputs/ArtNetOutput.h"
#include "../xLights/controllers/FPP.h"
#include "../xLights/controllers/Pixlite16.h"
#include "../xLights/controllers/Falcon.h"
#include "../xSchedule/xSMSDaemon/Curl.h"
#include "../xSchedule/wxJSON/jsonreader.h"

#include <log4cpp/Category.hh>

#define FAST_TIMEOUT 2
#define SLOW_TIMEOUT 5

std::string  GetXLightsShowFolder()
{
	wxString showDir = "";

	wxConfig* xlconfig = new wxConfig(_("xLights"));
	if (xlconfig != nullptr) {
		xlconfig->Read(_("LastDir"), &showDir);
		delete xlconfig;
	}

	return showDir.ToStdString();
}

std::string GetXScheduleShowFolder()
{
	wxString showDir = "";

	wxConfig* xlconfig = new wxConfig(_("xSchedule"));
	if (xlconfig != nullptr) {
		xlconfig->Read(_("SchedulerLastDir"), &showDir);
		delete xlconfig;
	}

	return showDir.ToStdString();
}

std::string GetForceIP()
{
	wxString localIP = "";

	wxConfig* xlconfig = new wxConfig(_("xLights"));
	if (xlconfig != nullptr) {
		xlconfig->Read(_("xLightsLocalIP"), &localIP, "");
		delete xlconfig;
	}

	return localIP.ToStdString();
}

void Scanner::xLightsScan()
{
	auto xLightsShowFolder = GetXLightsShowFolder();
	auto xScheduleShowFolder = GetXScheduleShowFolder();
	_showDir = xLightsShowFolder;

	if (xScheduleShowFolder != "" && xScheduleShowFolder != xLightsShowFolder) {
		if (wxMessageBox("xSchedule and xLights are using different show folders. Should i use the xSchedule folder?", "Choose folder.", wxYES_NO | wxCENTRE, _frame) == wxYES) {
			_showDir = xScheduleShowFolder;
		}
	}

	_om.Load(_showDir, false);

	_xLights._globalFPPProxy = _om.GetGlobalFPPProxy();
	_xLights._forceIP = GetForceIP();

	for (const auto& it : _om.GetControllers()) {
		_xLights._controllers.push_back(it);
	}
}

void Scanner::ComputerScan()
{
	// read the local ips
	auto localIPs = GetLocalIPs();
	for (const auto& it : localIPs) {
		_computer._ips.push_back(it);
	}

	_computer._name = wxGetHostName();
	_computer._xLightsFolder = GetXLightsShowFolder();
	_computer._xScheduleFolder = GetXScheduleShowFolder();

	// read any static routes
#ifdef __WXMSW__
	DWORD dwSize = 0;
	if (::GetIpForwardTable(nullptr, &dwSize, true) == ERROR_INSUFFICIENT_BUFFER) {
		MIB_IPFORWARDTABLE* p = (MIB_IPFORWARDTABLE * )malloc(dwSize);
		if (p != nullptr) {
			if (::GetIpForwardTable(p, &dwSize, true) == NO_ERROR) {
				for (int i = 0; i < (int)p->dwNumEntries; i++) {
					if (p->table[i].dwForwardProto == MIB_IPPROTO_NETMGMT && (u_long)p->table[i].dwForwardDest != 0) {
						char szDestIp[128];
						//char szMaskIp[128];
						struct in_addr IpAddr;
						IpAddr.S_un.S_addr = (u_long)p->table[i].dwForwardDest;
						strcpy_s(szDestIp, sizeof(szDestIp), inet_ntoa(IpAddr));
						//IpAddr.S_un.S_addr = (u_long)p->table[i].dwForwardMask;
						//strcpy_s(szMaskIp, sizeof(szMaskIp), inet_ntoa(IpAddr));
						auto ip = std::string(szDestIp);
						_computer._routes.push_back(ip);

						if (!Scanned(ip, ""))
						{
							IPScan(ip, "");
						}
					}
				}
			}
			free(p);
		}
	}
#endif
}

std::list<IPObject> Scanner::GetIPsInSameSubnet(const std::string& ip) const
{
	std::list<IPObject> res;

	for (const auto& it : _ips) {
		if (it.InSameSubnet(ip)) {
			res.push_back(it);
		}
	}

	return res;
}

std::list<IPObject> Scanner::GetUndisplayedIPs() const
{
	std::list<IPObject> res;

	for (const auto& it : _ips) {
		if (!it._displayed) {
			res.push_back(it);
		}
	}

	return res;
}

std::list<IPObject> Scanner::GetProxiedBy(const std::string& ip) const
{
	std::list<IPObject> res;

	for (const auto& it : _ips) 		{
		if (it._viaProxy == ip) 			{
			res.push_back(it);
		}
	}

	return res;
}

void Scanner::SetDisplayed(const IPObject& ip)
{
	for (auto& it : _ips) 		{
		if (it._ip == ip._ip) 			{
			it.Displayed();
			return;
		}
	}
}

void Scanner::IPScan(const std::string& ip, const std::string& proxy)
{
	static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));

	std::list<int> ips;
	for (int i = 1; i < 255; i++) ips.push_back(i);

	std::function<void(int)> f = [this, ip, proxy](int n) {
		struct in_addr IpAddr;
		IpAddr.S_un.S_addr = inet_addr(ip.c_str());
		IpAddr.S_un.S_un_b.s_b4 = n;

		char szDestIp[128];
		strcpy_s(szDestIp, sizeof(szDestIp), inet_ntoa(IpAddr));

		logger_base.debug("Pinging %s", (const char*)szDestIp);
		SendProgress(-1, "Pinging " + std::string(szDestIp));
		if (IPOutput::Ping(std::string(szDestIp), proxy) == IPOutput::PINGSTATE::PING_OK) {
			std::unique_lock<std::mutex> lock(_mutex);
			this->_ips.push_back(IPObject(std::string(szDestIp), proxy, true));
		}
	};

#ifdef SINGLE_THREAD
	for (int i = 1; i < 255; i++) {
		f(i);
	}
#else
	// ping the direct networks on the computer
	parallel_for(1, 255, std::move(f));
#endif
}

bool Scanner::Scanned(const std::string& ip, const std::string& proxy)
{
	struct in_addr IpAddr;
	IpAddr.S_un.S_addr = inet_addr(ip.c_str());
	IpAddr.S_un.S_un_b.s_b4 = 0;
	char szDestIp[128];
	strcpy_s(szDestIp, sizeof(szDestIp), inet_ntoa(IpAddr));
	std::string ipp = std::string(szDestIp) + "|" + proxy;
	if (std::find(begin(_ipscanned), end(_ipscanned), ipp) == end(_ipscanned)) 		{
		_ipscanned.push_back(ipp);
		return false;
	}
	return true;
}

IPObject* Scanner::GetIP(std::list<IPObject>& ips, const std::string& ip)
{
	for (auto& it : ips) 		{
		if (it._ip == ip) 			{
			return &it;
		}
	}
	return nullptr;
}

void Scanner::CheckXSchedule(IPObject& ip, int port)
{
	auto xs = Curl::HTTPSGet(ip._ip + ":" + wxString::Format("%d", port) + "/xScheduleQuery?Query=getplayingstatus", "", "", FAST_TIMEOUT);
	if (xs != "" && xs[0] == '{') {
		ip._type = "xSchedule";
		wxJSONValue defaultValue = wxString("");
		wxJSONReader reader;
		wxJSONValue root;
		if (reader.Parse(xs, &root) == 0) {
			ip._version = root.Get("version", defaultValue).AsString();
		}
	}
}

void Scanner::SendProgress(int progress, const std::string& msg)
{
	wxCommandEvent event(EVT_SCANPROGRESS);
	if (progress == -1) 		{
		event.SetInt(_progressv);
	}
	else {
		event.SetInt(progress);
	}
	event.SetString(msg);
	wxPostEvent(_frame, event);
}

void Scanner::PreScan(xScannerFrame* frame)
{
	static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));

	_frame = frame;

	logger_base.debug("Scanning computer");
	SendProgress(_progressv += 4, "Scanning computer");
	ComputerScan();

	logger_base.debug("Scanning xLights show folder");
	SendProgress(_progressv += 4, "Scanning xLights Show");
	xLightsScan();

	logger_base.debug("Running controller discovery");
	SendProgress(_progressv += 4, "Running controller discovery");
	if (_discovery != nullptr) delete _discovery;

	_discovery = new Discovery(_frame, &_om);

	Pixlite16::PrepareDiscovery(*_discovery);
	ZCPPOutput::PrepareDiscovery(*_discovery);
	ArtNetOutput::PrepareDiscovery(*_discovery);
	DDPOutput::PrepareDiscovery(*_discovery);
	FPP::PrepareDiscovery(*_discovery);

	_discovery->Discover();
}

void Scanner::Scan(xScannerFrame* frame)
{
	static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));

	_frame = frame;

	logger_base.debug("Processing local IPs");
	for (const auto& ip : _computer._ips) {
		if (!Scanned(ip, "")) {
			SendProgress(_progressv += 4, "Scanning ip subnet " + ip);
			IPScan(ip, "");
		}
	}

	logger_base.debug("Processing xLights Controllers");
	for (const auto& c : _xLights._controllers) {
		if (c->GetType() == "Ethernet") {
			auto proxy = _xLights._globalFPPProxy;
			auto eth = dynamic_cast<ControllerEthernet*>(c);
			if (eth->GetFPPProxy() != "") {
				proxy = eth->GetFPPProxy();
			}
			if (!Scanned(eth->GetResolvedIP(), proxy)) {
				SendProgress(_progressv += 4, "Scanning ip subnet " + eth->GetResolvedIP());
				IPScan(eth->GetResolvedIP(), proxy);
			}
		}
	}

	// now join everything together
	logger_base.debug("Unifying devices");
	SendProgress(55, "Unifying devices");

	UnifyxLightsController(_ips, true);

	UnifyDiscovery(_ips, true);

	logger_base.debug("Sorting IPs");
	_ips.sort();

	std::map<std::string, std::string> arps;

#ifdef __WXMSW__
	logger_base.debug("Reading ARP table");
	SendProgress(60, "Reading ARP table");
	DWORD bytesNeeded = 0;
	PMIB_IPNETTABLE arp = nullptr;

	// The result from the API call.
	int result = ::GetIpNetTable(nullptr, &bytesNeeded, false);


	// Call the function, expecting an insufficient buffer.
	if (result == ERROR_INSUFFICIENT_BUFFER) {

		arp = (PMIB_IPNETTABLE)malloc(bytesNeeded);
		if (arp != nullptr) {
			result = GetIpNetTable(arp, &bytesNeeded, false);

			if (result == 0) {
				for (size_t i = 0; i < arp->dwNumEntries; i++) {
					auto a = arp->table[i];

					std::string iip;
					std::string mac;

					if (memcmp(a.bPhysAddr, "\0\0\0\0\0\0\0\0", 8) != 0) {
						mac = wxString::Format("%02X-%02X-%02X-%02X-%02X-%02X-%02X-%02X", a.bPhysAddr[0], a.bPhysAddr[1], a.bPhysAddr[2], a.bPhysAddr[3], a.bPhysAddr[4], a.bPhysAddr[5], a.bPhysAddr[6], a.bPhysAddr[7]);
						iip = wxString::Format("%d.%d.%d.%d", a.dwAddr & 0xFF, (a.dwAddr >> 8) & 0xFF, (a.dwAddr >> 16) & 0xFF, (a.dwAddr >> 24) & 0xFF);
						arps[iip] = mac;
					}
				}
			}
			free(arp);
		}
	}
#endif

	logger_base.debug("Getting MAC Vendors");
	SendProgress(65, "Getting MAC Vendors");
	LookupMac(arps, _ips);

	logger_base.debug("Querying devices");
	SendProgress(70, "Querying devices");
	std::function<void(IPObject&, int)> f = [this](IPObject& o, int n) {
		SendProgress(-1, "Querying " + o._name + ":" + o._ip);
		IPScan(o);
	};
#ifdef SINGLE_THREAD
	{
		int i = 0;
		for (auto& it : _ips) {
			f(it, i++);
		}
	}
#else
	parallel_for(_ips, f, 1);
#endif

	// if we found any extra IPs redo the scan
	if (_extraips.size() > 0) 		{
		LookupMac(arps, _extraips);
		UnifyxLightsController(_extraips, false);
		UnifyDiscovery(_extraips, false);
		for (const auto& it : _extraips) 			{
			_ips.push_back(IPObject(it));
		}
#ifdef SINGLE_THREAD
		{
			int i = 0;
			for (auto& it : _ips) {
				f(it, i++);
			}
		}
#else
		parallel_for(_ips, f, 1);
#endif
	}

	SendProgress(100, "Done");
}

void Scanner::UnifyDiscovery(std::list<IPObject>& ips, bool addMissing)
{
	for (size_t x = 0; x < _discovery->GetResults().size(); x++) {
		auto discovered = _discovery->GetResults()[x];
		if (!discovered->controller) {
			continue;
		}
		ControllerEthernet* it = discovered->controller;
		auto ip = GetIP(ips, it->GetResolvedIP());
		if (addMissing && ip == nullptr) {
			ips.push_back(IPObject(it->GetResolvedIP(), "", false));
			ip = &_ips.back();
		}
		if (ip != nullptr) {
			ip->SetDiscovered(it);
		}
	}
}

void Scanner::UnifyxLightsController(std::list<IPObject>& ips, bool addMissing)
{
	for (const auto& c : _xLights._controllers) {
		auto eth = dynamic_cast<ControllerEthernet*>(c);
		if (eth != nullptr) {
			auto ip = GetIP(ips, eth->GetResolvedIP());
			if (addMissing && ip == nullptr) {
				_ips.push_back(IPObject(eth->GetResolvedIP(), "", false));
				ip = &_ips.back();
			}
			if (ip != nullptr) {
				ip->SetXLightsController(eth);
			}
		}
	}
}

void Scanner::LookupMac(std::map<std::string, std::string>& arps, std::list<IPObject>& ips)
{
	// we cant do these in parallel as we will hit concurrent request limits on this service
	for (auto& it : ips) {
		// try to get the mac address
		if (it._mac == "") {
			if (arps.find(it._ip) != arps.end()) {
				it._mac = arps.at(it._ip);
			}
		}

		if (it._mac != "") {
			it._macVendor = Curl::HTTPSGet("https://api.macvendors.com/" + it._mac, "", "", SLOW_TIMEOUT);
			if (Contains(it._macVendor, "\"Not Found\"")) it._macVendor = "";
			if (Contains(it._macVendor, "\"Too Many Requests\"")) it._macVendor = "MAC Lookup Unavailable";
		}
	}
}

void Scanner::IPScan(IPObject& it)
{
	static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));

	logger_base.debug("IPScan: %s", (const char*)it._ip.c_str());

	if (it._deepScan) return;

	it._deepScan = true;

	if (it._port80) {

		logger_base.debug("    Getting FPP network interface config");
		auto netconfig = Curl::HTTPSGet(it._ip + "/api/network/interface", "", "", FAST_TIMEOUT);
		if (netconfig != "" && Contains(netconfig, "operstate")) {

			it._type = "FPP";

			wxJSONValue defaultValue = wxString("");
			wxJSONReader reader;
			wxJSONValue root;
			if (reader.Parse(netconfig, &root) == 0) {
				// extract the type of request
				auto net = root.AsArray();

				for (size_t i = 0; i < net->Count(); i++) {
					auto n = (*net)[i];
					wxString operstate = n.Get("operstate", defaultValue).AsString();
					wxString iip = n.Get("addr_info", defaultValue)[0].Get("local", defaultValue).AsString();
					wxString label = n.Get("addr_info", defaultValue)[0].Get("label", defaultValue).AsString();
					if (operstate == "UP" && iip != "") it._otherIPs.push_back(label + " : " + iip);
				}
			}

			logger_base.debug("    Getting FPP version");
			auto version = Curl::HTTPSGet(it._ip + "/api/fppd/version", "", "", FAST_TIMEOUT);
			if (version != "" && version[0] == '{') {
				if (reader.Parse(version, &root) == 0) {
					it._version = root.Get("version", defaultValue).AsString();
				}
			}

			logger_base.debug("    Getting FPP Channel Outputs");
			auto co = Curl::HTTPSGet(it._ip + "/api/configfile/co-universes.json", "", "", FAST_TIMEOUT);
			if (co != "" && co[0] == '{') {
				if (reader.Parse(co, &root) == 0) {
					auto cc = root.Get("channelOutputs", defaultValue);
					if (cc.IsArray() && cc[0].Get("enabled", defaultValue).AsInt() == 1) {
						it._emittingData = cc[0].Get("interface", defaultValue).AsString();
					}
				}
			}

			logger_base.debug("    Getting FPP status");
			auto status = Curl::HTTPSGet(it._ip + "/api/fppd/status", "", "", FAST_TIMEOUT);
			if (status != "" && status[0] == '{') {
				if (reader.Parse(status, &root) == 0) {
					it._mode = root.Get("mode_name", defaultValue).AsString();
				}
			}

			logger_base.debug("    Getting FPP multisync");
			auto multisync = Curl::HTTPSGet(it._ip + "/api/fppd/multiSyncSystems", "", "", FAST_TIMEOUT);
			if (multisync != "" && multisync[0] == '{') {
				if (reader.Parse(multisync, &root) == 0) {
					auto sys = root.Get("systems", defaultValue).AsArray();

					if (sys != nullptr) {
						for (size_t i = 0; i < sys->Count(); i++) {
							auto s = (*sys)[i];
							std::string address = s.Get("address", defaultValue).AsString().ToStdString();
							std::string mode = s.Get("fppModeString", defaultValue).AsString().ToStdString();
							std::string name = s.Get("hostname", defaultValue).AsString().ToStdString();
							std::string version = s.Get("version", defaultValue).AsString().ToStdString();
							auto iip = GetIP(_ips, address);
							if (iip != nullptr) {
								if (iip->_name == "") iip->_name = name;
								if (iip->_mode == "") iip->_mode = mode;
								if (iip->_version == "") iip->_version = version;
							}
							else {
								logger_base.debug("FPP multicast identified new address " + address);
								std::unique_lock<std::mutex> lock(_mutex);
								if (GetIP(_extraips, address) == nullptr) {
									_extraips.push_back(IPObject(address, "", false));
									_extraips.back()._name = name;
									_extraips.back()._mode = mode;
									_extraips.back()._version = version;
								}
							}
						}
					}
				}
			}
		}

		if (it._type == "" || it._type == "Falcon") {
			logger_base.debug("    Getting Falcon status");
			auto status = Curl::HTTPSGet("http://" + it._ip + "/status.xml", "", "", SLOW_TIMEOUT);

			if (status != "" && Contains(status, "<response>\r\n")) {
				it._type = "Falcon";
				if (it._mode == "" || it._banks == "") {
					wxXmlDocument doc;
					doc.Load(wxStringInputStream(status));
					if (doc.IsOk() && doc.GetRoot() != nullptr) {
						int k0 = 0;
						int k1 = 0;
						int k2 = 0;
						for (auto n = doc.GetRoot()->GetChildren(); n != nullptr; n = n->GetNext()) {
							if (n->GetChildren() != nullptr) {
								if (n->GetName() == "m") {
									int m = wxAtoi(n->GetChildren()->GetContent());
									it._mode = Falcon::DecodeMode(m);
								}
								else if (n->GetName() == "k0") {
									k0 = wxAtoi(n->GetChildren()->GetContent());
								}
								else if (n->GetName() == "k1") {
									k1 = wxAtoi(n->GetChildren()->GetContent());
								}
								else if (n->GetName() == "k2") {
									k2 = wxAtoi(n->GetChildren()->GetContent());
								}
							}
						}
						if (k0 != 0 || k1 != 0 || k2 != 0) 							{
							it._banks = wxString::Format("%d:%d:%d", k0, k1, k2);
						}
					}
				}
			}
		}
	}

	// check for xSchedule on some common ports
	if (it._type == "" || it._type == "xSchedule") {
		logger_base.debug("    Checking for xSchedule");
		CheckXSchedule(it, 80);
		if (it._type == "") {
			CheckXSchedule(it, 81);
			if (it._type == "") {
				CheckXSchedule(it, 8080);
				if (it._type == "") {
					CheckXSchedule(it, 8081);
				}
			}
		}
	}

	if (it._type == "") {

		logger_base.debug("    Getting the default page");

		// just scrape the homepage and make a guess
		auto home = Curl::HTTPSGet("http://" + it._ip, "", "", SLOW_TIMEOUT);

		if (Contains(home, "Joshua 1 Systems")) it._type = "J1Sys";
		else if (Contains(home, "SanDevices")) it._type = "SanDevices";
		else if (Contains(home, "Minleon") && Contains(home, "NDB")) it._type = "Minleon";
		else if (Contains(home, "WLED")) it._type = "WLED";
		else if (Contains(home, "ESPixel")) it._type = "ESPixelStick";
		else if (Contains(home, "AlphaPix") || Contains(home, "Port Ethernet to SPI Controller")) it._type = "AlphaPix";
		else if (Contains(home, "HinksPix")) it._type = "HinksPix";
		else if (Contains(home, "DIYLEDExpress")) it._type = "DIYLEDExpress";
	}

	if (it._type == "") {
		if (it._discovered != nullptr) {
			it._type = it._discovered->GetVendor();
		}
		else if (it._xLightsController != nullptr) {
			it._type = it._xLightsController->GetVendor();
		}

		if (it._type == "") {
			it._type = it._macVendor;
		}
	}
}