#ifndef OUTPUTPROCESS_H
#define OUTPUTPROCESS_H
#include <list>
#include <string>
#include <wx/wx.h>

class wxXmlNode;
class OutputManager;

class OutputProcess
{
    protected:

        std::string _description;
        std::string _startChannel;
        int _changeCount;
        int _lastSavedChangeCount;
        bool _enabled;
        OutputManager* _outputManager;
        long _sc;

    void Save(wxXmlNode* node);

    public:

        static OutputProcess* CreateFromXml(OutputManager* outputManager, wxXmlNode* node);

        bool IsDirty() const { return _changeCount != _lastSavedChangeCount; };
        void ClearDirty() { _lastSavedChangeCount = _changeCount; };
        OutputProcess(OutputManager* outputManager, wxXmlNode* node);
        OutputProcess(OutputManager* outputManager);
        OutputProcess(const OutputProcess& op);
        OutputProcess(OutputManager* outputManager, std::string startChannel, const std::string& description);
        std::string GetDescription() const { return _description; }
        virtual ~OutputProcess() {}
        virtual wxXmlNode* Save() = 0;
        std::string GetStartChannel() const { return _startChannel; }
        size_t GetStartChannelAsNumber();
        virtual size_t GetP1() const = 0;
        virtual size_t GetP2() const = 0;
        virtual std::string GetType() const = 0;
        bool IsEnabled() const
        {
            return _enabled;
        }
        void Enable(bool enable) { _enabled = enable; _changeCount++; }

        virtual void Frame(wxByte* buffer, size_t size) = 0;
};

#endif
