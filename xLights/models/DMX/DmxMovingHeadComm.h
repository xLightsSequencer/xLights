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

#include "DmxModel.h"
#include "../ModelManager.h"

class DmxMotor;

enum DMX_FIXTURE {
    DMX_MOVING_HEAD_1,
    DMX_MOVING_HEAD_2,
    DMX_MOVING_HEAD_3,
    DMX_MOVING_HEAD_4,
    DMX_MOVING_HEAD_5,
    DMX_MOVING_HEAD_6,
    DMX_MOVING_HEAD_7,
    DMX_MOVING_HEAD_8
};

class PanTiltState
{
public:
    uint32_t ms = 0;
    float pan_angle = 0.0f;
    float tilt_angle = 0.0f;
};

class DmxMovingHeadComm : public DmxModel {
    public:
        DmxMovingHeadComm(const ModelManager& manager) :
            DmxModel(manager)
        {
        }
        virtual ~DmxMovingHeadComm(){};

        virtual DmxMotor* GetPanMotor() const = 0;
        virtual DmxMotor* GetTiltMotor() const = 0;
        virtual int GetFixtureVal() const {
            return fixture_val + 1;
        };
        std::string GetFixture() const {
            return FixtureIDtoString(fixture_val);
        }
    
        void SetDmxFixture(const std::string val) { dmx_fixture = val; }

        static std::string FixtureIDtoString(int fixture_val) {
            if (fixture_val == DMX_MOVING_HEAD_1) {
                return "MH1";
            }
            if (fixture_val == DMX_MOVING_HEAD_2) {
                return "MH2";
            }
            if (fixture_val == DMX_MOVING_HEAD_3) {
                return "MH3";
            }
            if (fixture_val == DMX_MOVING_HEAD_4) {
                return "MH4";
            }
            if (fixture_val == DMX_MOVING_HEAD_5) {
                return "MH5";
            }
            if (fixture_val == DMX_MOVING_HEAD_6) {
                return "MH6";
            }
            if (fixture_val == DMX_MOVING_HEAD_7) {
                return "MH7";
            }
            if (fixture_val == DMX_MOVING_HEAD_8) {
                return "MH8";
            }
            return "MH1";
        }

        static int FixtureStringtoID(std::string const& dmx_fixture) {
            if (dmx_fixture == "MH2") {
                return DMX_MOVING_HEAD_2;
            }
            if (dmx_fixture == "MH3") {
                return DMX_MOVING_HEAD_3;
            }
            if (dmx_fixture == "MH4") {
                return DMX_MOVING_HEAD_4;
            }
            if (dmx_fixture == "MH5") {
                return DMX_MOVING_HEAD_5;
            }
            if (dmx_fixture == "MH6") {
                return DMX_MOVING_HEAD_6;
            }
            if (dmx_fixture == "MH7") {
                return DMX_MOVING_HEAD_7;
            }
            if (dmx_fixture == "MH8") {
                return DMX_MOVING_HEAD_8;
            }
            return DMX_MOVING_HEAD_1;
        }

    protected:
        int fixture_val{ 0 };
        std::string dmx_fixture = "MH1";
};
