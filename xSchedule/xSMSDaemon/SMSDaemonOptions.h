#ifndef SMSDaemonOPTIONS_H
#define SMSDaemonOPTIONS_H
#include <list>
#include <string>
#include <vector>
#include <wx/wx.h>

class SMSDaemonOptions
{
    int _changeCount = 0;
    int _lastSavedChangeCount = 0;

	std::string _xScheduleIP = "127.0.0.1";
	std::string _textItem = "";
	std::string _user = "";
	std::string _sid = "";
	std::string _token = "";
	std::string _phone = "";
	std::string _defaultMessage = "";
	std::string _smsService = "";
	wxString _successMessage = "";
	wxString _rejectMessage = "";
	int _retrieveInterval = 10;
	int _displayDuration = 30;
	int _maximiumMessageLength = 100;
	int _maximiumMessageAge = 10;
	int _maximiumTimesToDisplay = 0;
	int _xSchedulePort = 80;
	bool _usePurgoMalum = false;
	bool _rejectProfanity = true;
	bool _usePhoneBlacklist = false;
	bool _useLocalBlacklist = false;
	bool _useLocalWhitelist = false;
	bool _acceptOneWordOnly = false;
	bool _ignoreOversizedMessages = false;
    bool _upperCase = false;

    public:

		std::string GetXScheduleIP() const { return _xScheduleIP; }
        void SetXScheduleIP(std::string xScheduleIP) { if (xScheduleIP != _xScheduleIP) { _xScheduleIP = xScheduleIP; _changeCount++; } }
		std::string GetTextItem() const { return _textItem; }
        void SetTextItem(std::string textItem) { if (textItem != _textItem) { _textItem = textItem; _changeCount++; } }
		std::string GetUser() const { return _user; }
        void SetUser(std::string user) { if (user != _user) { _user = user; _changeCount++; } }
		std::string GetSID() const { return _sid; }
        void SetSID(std::string sid) { if (sid != _sid) { _sid = sid; _changeCount++; } }
		std::string GetToken() const { return _token; }
        void SetToken(std::string token) { if (token != _token) { _token = token; _changeCount++; } }
		std::string GetPhone() const { return _phone; }
        void SetPhone(std::string phone) { if (phone != _phone) { _phone = phone; _changeCount++; } }
		std::string GetSMSService() const { return _smsService; }
        void SetSMSService(std::string smsService) { if (smsService != _smsService) { _smsService = smsService; _changeCount++; } }
		std::string GetDefaultMessage() const { return _defaultMessage; }
        void SetDefaultMessage(std::string defaultMessage) { if (defaultMessage != _defaultMessage) { _defaultMessage = defaultMessage; _changeCount++; } }
		wxString GetSuccessMessage() const { return _successMessage; }
        void SetSuccessMessage(wxString successMessage) { if (successMessage != _successMessage) { _successMessage = successMessage; _changeCount++; } }
		wxString GetRejectMessage() const { return _rejectMessage; }
        void SetRejectMessage(wxString rejectMessage) { if (rejectMessage != _rejectMessage) { _rejectMessage = rejectMessage; _changeCount++; } }

		int GetXSchedulePort() const { return _xSchedulePort; }
        void SetXSchedulePort(int xSchedulePort) { if (xSchedulePort != _xSchedulePort) { _xSchedulePort = xSchedulePort; _changeCount++; } }
		int GetRetrieveInterval() const { return _retrieveInterval; }
        void SetRetrieveInterval(int retrieveInterval) { if (retrieveInterval != _retrieveInterval) { _retrieveInterval = retrieveInterval; _changeCount++; } }
		int GetDisplayDuration() const { return _displayDuration; }
        void SetDisplayDuration(int displayDuration) { if (displayDuration != _displayDuration) { _displayDuration = displayDuration; _changeCount++; } }
		int GetMaxMessageLength() const { return _maximiumMessageLength; }
        void SetMaxMessageLength(int maximiumMessageLength) { if (maximiumMessageLength != _maximiumMessageLength) { _maximiumMessageLength = maximiumMessageLength; _changeCount++; } }
		int GetMaxMessageAge() const { return _maximiumMessageAge; }
        void SetMaxMessageAge(int maximiumMessageAge) { if (maximiumMessageAge != _maximiumMessageAge) { _maximiumMessageAge = maximiumMessageAge; _changeCount++; } }
		int GetMaxTimesToDisplay() const { return _maximiumTimesToDisplay; }
        void SetMaxTimesToDisplay(int maximiumTimesToDisplay) { if (maximiumTimesToDisplay != _maximiumTimesToDisplay) { _maximiumTimesToDisplay = maximiumTimesToDisplay; _changeCount++; } }

		bool GetUsePurgoMalum() const { return _usePurgoMalum; }
        void SetUsePurgoMalum(bool usePurgoMalum) { if (usePurgoMalum != _usePurgoMalum) { _usePurgoMalum = usePurgoMalum; _changeCount++; } }
		bool GetRejectProfanity() const { return _rejectProfanity; }
        void SetRejectProfanity(bool rejectProfanity) { if (rejectProfanity != _rejectProfanity) { _rejectProfanity = rejectProfanity; _changeCount++; } }
		bool GetUsePhoneBlacklist() const { return _usePhoneBlacklist; }
        void SetUsePhoneBlacklist(bool usePhoneBlacklist) { if (usePhoneBlacklist != _usePhoneBlacklist) { _usePhoneBlacklist = usePhoneBlacklist; _changeCount++; } }
		bool GetUseLocalBlacklist() const { return _useLocalBlacklist; }
        void SetUseLocalBlacklist(bool useLocalBlacklist) { if (useLocalBlacklist != _useLocalBlacklist) { _useLocalBlacklist = useLocalBlacklist; _changeCount++; } }
		bool GetUseLocalWhitelist() const { return _useLocalWhitelist; }
        void SetUseLocalWhitelist(bool useLocalWhitelist) { if (useLocalWhitelist != _useLocalWhitelist) { _useLocalWhitelist = useLocalWhitelist; _changeCount++; } }
		bool GetAcceptOneWordOnly() const { return _acceptOneWordOnly; }
        void SetAcceptOneWordOnly(bool acceptOneWordOnly) { if (acceptOneWordOnly != _acceptOneWordOnly) { _acceptOneWordOnly = acceptOneWordOnly; _changeCount++; } }
		bool GetIgnoreOversizedMessages() const { return _ignoreOversizedMessages; }
        void SetIgnoreOversizedMessages(bool ignoreOversizedMessages) { if (ignoreOversizedMessages != _ignoreOversizedMessages) { _ignoreOversizedMessages = ignoreOversizedMessages; _changeCount++; } }
		bool GetUpperCase() const { return _upperCase; }
        void SetUpperCase(bool upperCase) { if (upperCase != _upperCase) { _upperCase = upperCase; _changeCount++; } }
		
        bool IsDirty() const;
        void ClearDirty();
        SMSDaemonOptions();
        void Load(const std::string& showDir);
        //virtual ~SMSDaemonOptions();
        void Save(const std::string& showDir);
        bool IsValid() const;
};

#endif
