#ifndef XLCUSTOMCONTROL_H
#define XLCUSTOMCONTROL_H

#include <string>

class xlCustomControl
{
    public:
        xlCustomControl() {}
        virtual ~xlCustomControl() {}

    
        virtual void SetValue(const std::string &val) = 0;
};

#endif // XLCUSTOMCONTROL_H
