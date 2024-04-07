#pragma once
#include "BaseController.h"
class ILightThat : public BaseController
{
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
