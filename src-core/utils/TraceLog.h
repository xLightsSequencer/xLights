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

#include <string>
#include <list>

namespace TraceLog {
    
    void AddTraceMessage(const std::string &msg);
    void PushTraceContext();
    void PopTraceContext();
    void ClearTraceMessages();

    // Only the calling thread's messages. Right for a crash handler, which runs
    // on the thread that faulted; wrong for anything collecting a report from
    // elsewhere, which gets an empty list.
    void GetTraceMessages(std::list<std::string> &msgs);

    // Every thread's messages, each preceded by a "thread <id>:" line.
    void GetAllTraceMessages(std::list<std::string> &msgs);

}
