#include "aiBase.h"


aiBase::aiBase(ServiceManager* sm) :
    _sm(sm) {
     //LoadSettings();
}

std::pair<std::string, bool> aiBase::TestLLM() const {
    return CallLLM("Hello");
}