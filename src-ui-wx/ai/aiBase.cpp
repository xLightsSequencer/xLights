#include "aiBase.h"

// aiBase constructor and TestLLM are inlined in the header so that plugin DLLs
// can implement the aiBase interface without needing an import linkage back to
// xLights.exe.  This translation unit is intentionally empty.
