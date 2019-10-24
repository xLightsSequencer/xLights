#ifndef __XLIGHTS_TRACELOG_H
#define __XLIGHTS_TRACELOG_H

#include <string>
#include <list>

namespace TraceLog {
    
    void AddTraceMessage(const std::string &msg);
    void PushTraceContext();
    void PopTraceContext();
    void ClearTraceMessages();

    void GetTraceMessages(std::list<std::string> &msgs);

}

#endif // __XLIGHTS_TRACELOG_H
