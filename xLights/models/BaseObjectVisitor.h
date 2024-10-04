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

class ArchesModel;
class DmxMovingHead;
class DmxMovingHeadAdv;

struct BaseObjectVisitor
{
    virtual void Visit(const ArchesModel &arch) = 0;
    virtual void Visit(const DmxMovingHeadAdv &moving_head) = 0;
    virtual void Visit(const DmxMovingHead& moving_head) = 0;

    virtual ~BaseObjectVisitor() {}
};
