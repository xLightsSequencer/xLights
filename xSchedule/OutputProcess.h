#ifndef OUTPUTPROCESS_H
#define OUTPUTPROCESS_H
#include <list>
#include <string>
#include <wx/wx.h>

class wxXmlNode;

class OutputProcess
{
    protected:

        std::string _description;
        size_t _startChannel;
        int _changeCount;
        int _lastSavedChangeCount;
        bool _enabled;

    void Save(wxXmlNode* node);

    public:

        static OutputProcess* CreateFromXml(wxXmlNode* node);

        bool IsDirty() const { return _changeCount != _lastSavedChangeCount; };
        void ClearDirty() { _lastSavedChangeCount = _changeCount; };
        OutputProcess(wxXmlNode* node);
        OutputProcess();
        OutputProcess(const OutputProcess& op);
        OutputProcess(size_t startChannel, const std::string& description);
        std::string GetDescription() const { return _description; }
        virtual ~OutputProcess() {}
        virtual wxXmlNode* Save() = 0;
        size_t GetStartChannel() const { return _startChannel; }
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
