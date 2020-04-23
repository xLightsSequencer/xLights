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
#include <list>

namespace TraceLog {
    
    void AddTraceMessage(const std::string &msg);
    void PushTraceContext();
    void PopTraceContext();
    void ClearTraceMessages();

    void GetTraceMessages(std::list<std::string> &msgs);

}
