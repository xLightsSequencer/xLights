#ifndef SCHEDULEOPTIONS_H
#define SCHEDULEOPTIONS_H
#include <list>

class wxXmlNode;

class ScheduleOptions
{
	bool _sync;
    bool _dirty;
    bool _sendOffWhenNotRunning;
	
    public:

        bool IsDirty() const { return _dirty; };
        void ClearDirty() { _dirty = false; };
        ScheduleOptions(wxXmlNode* node);
        ScheduleOptions();
        virtual ~ScheduleOptions();
        wxXmlNode* Save() const;
		bool IsSync() const { return _sync; }
        bool IsSendOffWhenNotRunning() const { return _sendOffWhenNotRunning; }
        std::list<std::string> GetProjectors() const;
        std::string GetProjectorIpAddress(const std::string& projector);
        std::string GetProjectorPassword(const std::string& projector);
};

#endif 