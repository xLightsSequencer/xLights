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

#include "../Model.h"

class wxPropertyGridInterface;
class wxPropertyGridEvent;
class BaseObject;
class wxXmlNode;

class DmxBeamAbility
{
    public:
        DmxBeamAbility();
        virtual ~DmxBeamAbility();

        void AddBeamTypeProperties(wxPropertyGridInterface *grid);
        int OnBeamPropertyGridChange(wxPropertyGridInterface *grid, wxPropertyGridEvent& event, BaseObject* base);

        [[nodiscard]] float GetBeamLength() const { return beam_length; }
        [[nodiscard]] float GetDefaultBeamLength() const { return default_beam_length; }
        [[nodiscard]] float GetBeamWidth() const { return beam_width; }
        [[nodiscard]] float GetDefaultBeamWidth() const { return default_beam_width; }
        [[nodiscard]] float GetBeamYOffset() const { return beam_y_offset; }
        [[nodiscard]] float GetDefaultBeamYOffset() const { return default_beam_y_offset; }
        [[nodiscard]] int GetBeamOrient() const { return beam_orient; }
        [[nodiscard]] bool SupportsOrient() const { return supports_orient; }
        [[nodiscard]] bool SupportsYOffset() const { return supports_y_offset; }

        void SetBeamLength( float length ) { beam_length = length; }
        void SetDefaultBeamLength( float length ) { default_beam_length = length; }
        void SetBeamWidth( float width ) {
            beam_width = width; }
        void SetDefaultBeamWidth( float width ) { default_beam_width = width; }
        void SetBeamYOffset(float offset) { default_beam_y_offset = offset; }
        void SetDefaultBeamYOffset(float offset) { beam_y_offset = offset; }
        void SetBeamOrient(int orient) { beam_orient = orient; }

        void SetSupportsOrient(bool val) { supports_orient = val; }
        void SetSupportsYOffset(bool val) { supports_y_offset = val; }

    protected:
        bool supports_orient = false;
        bool supports_y_offset = false;
        float beam_length = 1.0;
        float default_beam_length = 1.0;
        float beam_width = 1.0;
        float default_beam_width = 1.0;
        int beam_orient = 0;
        float beam_y_offset = 0.0;
        float default_beam_y_offset = 0.0;
};


