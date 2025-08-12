/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include "models/BaseObject.h"
#include "outputs/Controller.h"
#include "xLightsMain.h"
#include "OutputModelManager.h"
#include "./utils/spdlog_macros.h"

#ifdef _DEBUG
void OutputModelManager::Dump(const std::string& type, const std::list<std::pair<uint32_t, std::string>>& source)
{
    

    LOG_DEBUG("    Dump explaining how we got here : %s", (const char*)type.c_str());
    for (const auto& it : source)
    {
        LOG_DEBUG("          0x%04x  %s  %s", it.first, (const char*)DecodeWork(it.first).c_str(), (const char*)it.second.c_str());
    }
    if (_selectedModel != "") LOG_DEBUG("    Select model '%s'", (const char*)_selectedModel.c_str());
    if (_selectedController != "") LOG_DEBUG("    Select controller '%s'", (const char*)_selectedController.c_str());
    if (_modelToModelFromXml != nullptr) LOG_DEBUG("    Model to reload xml '%s'", (const char*)_modelToModelFromXml->GetName().c_str());
}

std::string OutputModelManager::DecodeWork(uint32_t work)
{
    std::string res;
    if (work & WORK_NOTHING) res += "WORK_NOTHING";
    if (work & WORK_UPDATE_PROPERTYGRID) res += "WORK_UPDATE_PROPERTYGRID";
    if (work & WORK_MODELS_REWORK_STARTCHANNELS) res += " WORK_MODELS_REWORK_STARTCHANNELS";
    if (work & WORK_RELOAD_MODEL_FROM_XML) res += " WORK_RELOAD_MODEL_FROM_XML";
    if (work & WORK_RELOAD_ALLMODELS) res += " WORK_RELOAD_ALLMODELS";
    if (work & WORK_RELOAD_OBJECTLIST) res += " WORK_RELOAD_OBJECTLIST";
    if (work & WORK_MODELS_CHANGE_REQUIRING_RERENDER) res += " WORK_MODELS_CHANGE_REQUIRING_RERENDER";
    if (work & WORK_RGBEFFECTS_CHANGE) res += " WORK_RGBEFFECTS_CHANGE";
    if (work & WORK_RELOAD_PROPERTYGRID) res += " WORK_RELOAD_PROPERTYGRID";
    if (work & WORK_NETWORK_CHANGE) res += " WORK_NETWORK_CHANGE";
    if (work & WORK_UPDATE_NETWORK_LIST) res += " WORK_UPDATE_NETWORK_LIST";
    if (work & WORK_CALCULATE_START_CHANNELS) res += " WORK_CALCULATE_START_CHANNELS";
    if (work & WORK_NETWORK_CHANNELSCHANGE) res += " WORK_NETWORK_CHANNELSCHANGE";
    if (work & WORK_RESEND_CONTROLLER_CONFIG) res += " WORK_RESEND_CONTROLLER_CONFIG";
    if (work & WORK_SAVE_NETWORKS)
    {
        res += " WORK_SAVE_NETWORKS";
    }
    if (work & WORK_RELOAD_MODELLIST) res += " WORK_RELOAD_MODELLIST";
    if (work & WORK_REDRAW_LAYOUTPREVIEW) res += " WORK_REDRAW_LAYOUTPREVIEW";
    return res;
}
#endif

void OutputModelManager::SetSelectedModelIfASAPWorkExists(const std::string& selectedModel)
{
    if (_workASAP != 0)
    {
        if (selectedModel != "") _selectedModel = selectedModel;
    }
}

void OutputModelManager::SetSelectedControllerIfASAPWorkExists(const std::string& selectedController)
{
    if (_workASAP != 0)
    {
        if (selectedController != "") _selectedController = selectedController;
    }
}

void OutputModelManager::AddASAPWork(uint32_t work, const std::string& from, BaseObject* m, Controller* o, const std::string& selectedModel)
{
#ifdef _DEBUG
    _sourceASAP.push_back({ work, from });
#endif
    if (selectedModel != "") _selectedModel = selectedModel;
    if (o != nullptr) _selectedController = o->GetName();

    if (work & WORK_RELOAD_MODEL_FROM_XML)
    {
        if (m != nullptr)
        {
            // either must not have previously had a model or must be the same model
            wxASSERT(_modelToModelFromXml == nullptr || _modelToModelFromXml == m);
            _modelToModelFromXml = m;
        }
        else
        {
            // model must be specified
            wxASSERT(false);
            work &= ~WORK_RELOAD_MODEL_FROM_XML;
        }
    }
	_workASAP |= work;
    if (!_workRequested)
    {
        _frame->CallAfter(&xLightsFrame::DoASAPWork);
        _workRequested = true;
    }
}

void OutputModelManager::AddSetupTabWork(uint32_t work, const std::string& from, BaseObject* m, Controller* o, const std::string& selectedModel)
{
#ifdef _DEBUG
    _sourceSetup.push_back({ work, from });
#endif
    if (selectedModel != "") _selectedModel = selectedModel;
    if (o != nullptr) _selectedController = o->GetName();

    if (work & WORK_RELOAD_MODEL_FROM_XML)
    {
        // this is not allowed as the model pointer must be valid until this message is processed
        // and there is no way to guarantee that
        wxASSERT(false);
        work &= ~WORK_RELOAD_MODEL_FROM_XML;
    }
    _setupTabWork |= work;
}

void OutputModelManager::AddLayoutTabWork(uint32_t work, const std::string& from, BaseObject* m, Controller* o, const std::string& selectedModel)
{
#ifdef _DEBUG
    _sourceLayout.push_back({ work, from });
#endif
    if (selectedModel != "") _selectedModel = selectedModel;
    if (o != nullptr) _selectedController = o->GetName();

    if (work & WORK_RELOAD_MODEL_FROM_XML)
    {
        // this is not allowed as the model pointer must be valid until this message is processed
        // and there is no way to guarantee that
        wxASSERT(false);
        work &= ~WORK_RELOAD_MODEL_FROM_XML;
    }
    _layoutTabWork |= work;
}

BaseObject* OutputModelManager::GetModelToReload()
{
    wxASSERT(_modelToModelFromXml != nullptr);
    BaseObject* res = _modelToModelFromXml;
    _modelToModelFromXml = nullptr;
    return res;
}

std::string OutputModelManager::GetSelectedModel()
{
    auto res = _selectedModel;
    _selectedModel = "";
    return res;
}

std::string OutputModelManager::GetSelectedController()
{
    auto res = _selectedController;
    _selectedController = "";
    return res;
}

void OutputModelManager::AddImmediateWork(uint32_t work, const std::string& from, BaseObject* m, Controller* o, const std::string& selectedModel)
{
    
    if (work & WORK_RELOAD_MODEL_FROM_XML)
    {
        if (m == nullptr)
        {
            // model must be specified
            wxASSERT(false);
            work &= ~WORK_RELOAD_MODEL_FROM_XML;
        }
    }
#ifdef _DEBUG
    LOG_DEBUG("Doing Immediate Work. Called from %s", (const char*)from.c_str());
#else
    LOG_DEBUG("Doing Immediate Work.");
#endif
    _frame->DoWork(work, "Immediate", m, selectedModel);
}

void OutputModelManager::RemoveWork(const std::string& type, uint32_t toremove)
{
    if (type == "ASAP") {
        _workASAP &= ~toremove;
    }
    else if (type == "Setup") {
        _setupTabWork &= ~toremove;
    }
    else if (type == "Layout") {
        _layoutTabWork &= ~toremove;
    }
}

uint32_t OutputModelManager::ClearWork(const std::string& type, uint32_t currentwork, uint32_t work)
{
    // current work is the work we are doing
    // work is the work not yet done that we will do
    // this function adds in any pending work currently saved up into the current process ... effectively bringing it forward

    // there is a risk this will cause work to happen out of order

    uint32_t newWork = currentwork;
    if (type == "ASAP")
    {
        newWork |= _workASAP;
        _workASAP &= ~work;
    }
    else if (type == "Setup")
    {
        newWork |= _setupTabWork;
        _setupTabWork &= ~work;
    }
    else if (type == "Layout")
    {
        newWork |= _layoutTabWork;
        _layoutTabWork &= ~work;
    }

    return newWork;
}
