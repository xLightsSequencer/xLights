#include "Models/model.h"
#include "outputs/Output.h"
#include "xLightsMain.h"
#include "OutputModelManager.h"
#include <log4cpp/Category.hh>

void OutputModelManager::AddASAPWork(uint32_t work, Model* m, Output* o, const std::string& selectedModel)
{
    if (selectedModel != "")
    {
        wxASSERT(_selectedModel == "" || _selectedModel == selectedModel);
        _selectedModel = selectedModel;
    }
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

void OutputModelManager::AddSetupTabWork(uint32_t work, Model* m, Output* o, const std::string& selectedModel)
{
    if (selectedModel != "")
    {
        wxASSERT(_selectedModel == "" || _selectedModel == selectedModel);
        _selectedModel = selectedModel;
    }
    if (work & WORK_RELOAD_MODEL_FROM_XML)
    {
        // this is not allowed as the model pointer must be valid until this message is processed
        // and there is no way to guarantee that
        wxASSERT(false);
        work &= ~WORK_RELOAD_MODEL_FROM_XML;
    }
    _setupTabWork |= work;
}

void OutputModelManager::AddLayoutTabWork(uint32_t work, Model* m, Output* o, const std::string& selectedModel)
{
    if (selectedModel != "")
    {
        wxASSERT(_selectedModel == "" || _selectedModel == selectedModel);
        _selectedModel = selectedModel;
    }
    if (work & WORK_RELOAD_MODEL_FROM_XML)
    {
        // this is not allowed as the model pointer must be valid until this message is processed
        // and there is no way to guarantee that
        wxASSERT(false);
        work &= ~WORK_RELOAD_MODEL_FROM_XML;
    }
    _layoutTabWork |= work;
}

Model* OutputModelManager::GetModelToReload()
{
    wxASSERT(_modelToModelFromXml != nullptr);
    Model* res = _modelToModelFromXml;
    _modelToModelFromXml = nullptr;
    return res;
}

std::string OutputModelManager::GetSelectedModel()
{
    auto res = _selectedModel;
    _selectedModel = "";
    return res;
}

void OutputModelManager::AddImmediateWork(uint32_t work, Model* m, Output* o, const std::string& selectedModel)
{
    static log4cpp::Category& logger_work = log4cpp::Category::getInstance(std::string("log_work"));
    if (selectedModel != "")
    {
        wxASSERT(_selectedModel == "" || _selectedModel == selectedModel);
        _selectedModel = selectedModel;
    }
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
    logger_work.debug("Doing Immediate Work.");
    _frame->DoWork(work);
}