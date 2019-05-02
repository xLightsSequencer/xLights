
#ifndef OUTPUT_MODEL_MANAGER_H
#define OUTPUT_MODEL_MANAGER_H

class xLightsFrame;
class Model;
class Output;

class OutputModelManager {
	
	uint32_t _workASAP = 0;      // work to do asap
	uint32_t _setupTabWork = 0;  // work to do when we next switch to the setup tab
	uint32_t _layoutTabWork = 0; // work to do when we next switch to the layout tab
	xLightsFrame* _frame = nullptr;
    bool _workRequested = false;
	
public:

    static const uint32_t WORK_NETWORK_CHANGE = 0x0100;
    static const uint32_t WORK_UPDATE_NETWORK_LIST = 0x0200;
    static const uint32_t WORK_CALCULATE_START_CHANNELS = 0x0400;
    static const uint32_t WORK_NETWORK_CHANNELSCHANGE = 0x0800;
    static const uint32_t WORK_RESEND_CONTROLLER_CONFIG = 0x1000;
    static const uint32_t WORK_SAVE_NETWORKS = 0x2000;

	OutputModelManager() {}
	void SetFrame(xLightsFrame* frame)
	{
		_frame = frame;
	}
	uint32_t GetASAPWork() {
		auto res = _workASAP;
		_workASAP = 0;
        _workRequested = false;
		return res;
	}
	uint32_t GetSetupWork() {
		auto res = _setupTabWork;
		_setupTabWork = 0;
		return res;
	}
	uint32_t GetLayoutWork() {
		auto res = _layoutTabWork;
		_layoutTabWork = 0;
		return res;
	}	
    void AddImmediateWork(uint32_t work, Model* m, Output* o);
    void AddASAPWork(uint32_t work, Model* m, Output* o);
    void AddSetupTabWork(uint32_t work, Model* m, Output* o);
    void AddLayoutTabWork(uint32_t work, Model* m, Output* o);
};

#endif 
