#include "Models/model.h"
#include "outputs/Output.h"
#include "xLightsMain.h"
#include "OutputModelManager.h"

void OutputModelManager::AddASAPWork(uint32_t work, Model* m, Output* o)
{
	_workASAP |= work;
    _frame->CallAfter(&xLightsFrame::DoASAPWork);
    _workRequested = true;
}
void OutputModelManager::AddSetupTabWork(uint32_t work, Model* m, Output* o)
{
	_setupTabWork |= work;
}
void OutputModelManager::AddLayoutTabWork(uint32_t work, Model* m, Output* o)
{
	_layoutTabWork |= work;
}

void OutputModelManager::AddImmediateWork(uint32_t work, Model* m, Output* o)
{
    _frame->DoWork(work);
}