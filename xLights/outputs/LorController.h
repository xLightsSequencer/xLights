#ifndef LORCONTROLLER_H
#define LORCONTROLLER_H

#include <string>
#include <wx/xml/xml.h>

class LorController
{
    public:
        typedef enum AddressModes {
            LOR_ADDR_MODE_NORMAL,
            LOR_ADDR_MODE_LEGACY,
            LOR_ADDR_MODE_SPLIT
        } AddressMode;
        LorController();
        LorController(wxXmlNode* node);

        virtual ~LorController();

        void Save(wxXmlNode* node);
        int GetUnitId() const { return _unit_id; }
        int GetNumChannels() const { return _num_channels; }
        int GetTotalNumChannels() const;
        AddressMode GetAddressMode() const { return _mode; }
        std::string GetModeString() const;
        std::string GetType() const { return _type; }
        std::string GetDescription() const { return _description; }
        bool IsDirty() const { return _lastSavedChangeCount != _changeCount; }
        void ClearDirty() { _lastSavedChangeCount = _changeCount; }
        void SetType(std::string type) { _type = type; }
        void SetDescription(std::string description) { _description = description; }
        void SetUnitID(int id) { _unit_id = id; }
        void SetNumChannels(int channels) { _num_channels = channels; }
        void SetMode(AddressMode mode) { _mode = mode; }

    protected:
        int _unit_id;
        int _num_channels;
        std::string _type;
        std::string _description;
        AddressMode _mode;

    private:
        int _changeCount;
        int _lastSavedChangeCount;
};

#endif // LORCONTROLLER_H
