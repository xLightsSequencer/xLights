#pragma once
#include "BaseController.h"
class ILightThat : public BaseController
{
private:
    std::vector<int> _model_test_default_colours {
        0x00C7313A,
        0x00C66826,
        0x000A8E0C,
        0x00E0EF53,
        0x00954FD0,
        0x002860D0,
        0x00AB579B,
        0x005DB5D0,
        0x0053EF57,
        0x006C6A6A,
        0x0073452A,
        0x009C8226
    };

    int _model_test_default_col_idx = 0;

public:
#pragma region Constructors and Destructors
    ILightThat(const std::string& ip, const std::string& proxy);
    virtual ~ILightThat();
#pragma endregion
#pragma region Getters and Setters
#ifndef DISCOVERYONLY
    virtual bool SetOutputs(ModelManager* allmodels, OutputManager* outputManager, Controller* controller, wxWindow* parent) override;
#endif
    virtual bool UsesHTTP() const override
    {
        return true;
    }
#pragma endregion
};
