#pragma once

/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/smeighan/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/smeighan/xLights/blob/master/License.txt
 **************************************************************/

#include <string>

#include <wx/xml/xml.h>

class LorController
{
    public:
        enum class AddressMode {
            LOR_ADDR_MODE_NORMAL,
            LOR_ADDR_MODE_LEGACY,
            LOR_ADDR_MODE_SPLIT
        };

    protected:
    #pragma region Member Variables
        int _unit_id = 1;
        int _num_channels = 16;
        std::string _type;
        std::string _description;
        bool _expanded;
        LorController::AddressMode _mode = LorController::AddressMode::LOR_ADDR_MODE_NORMAL;
        bool _dirty = false;
    #pragma endregion
    
    public:

    #pragma region Constructors and Destructors
        LorController();
        LorController(wxXmlNode* node);
        LorController(const LorController& from);
        virtual ~LorController() {}
        void Save(wxXmlNode* node);
        LorController* Copy() const
        {
            return new LorController(*this);
        }
    #pragma endregion

    #pragma region Getters and Setters
        int GetUnitId() const { return _unit_id; }
        void SetUnitID(int id) { if (_unit_id != id) { _unit_id = id; _dirty = true; } }

        int GetNumChannels() const { return _num_channels; }
        void SetNumChannels(int channels) { if (_num_channels != channels) { _num_channels = channels; _dirty = true; } }

        bool IsExpanded() const { return _expanded; }
        void SetExpanded(bool expanded) { if (_expanded != expanded) { _expanded = expanded; _dirty = true; } }

        int GetTotalNumChannels() const;

        LorController::AddressMode GetAddressMode() const { return _mode; }
        void SetMode(LorController::AddressMode mode) { if (_mode != mode) { _mode = mode; _dirty = true; } }

        std::string GetModeString() const;

        std::string GetType() const { return _type; }
        void SetType(std::string type) { if (_type != type) { _type = type; _dirty = true; } }

        std::string GetDescription() const { return _description; }
        void SetDescription(std::string description) { if (_description != description) { _description = description; _dirty = true; } }

        bool IsDirty() const { return _dirty; }
        void ClearDirty() { _dirty = false; }

        int GetMaxChannels() { return 16383; }
        int GetMaxUnitId() { return 255; }
    #pragma endregion
};

