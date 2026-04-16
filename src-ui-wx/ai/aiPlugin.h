#pragma once

/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

// C ABI interface that AI plugin DLLs must export.
//
// A plugin DLL must export exactly two C functions:
//   aiBase* xlCreateAIService(ServiceManager* sm)   — allocate and return a new service
//   void    xlDestroyAIService(aiBase* service)     — destroy a service returned by the above
//
// Use the XL_AI_PLUGIN_EXPORT(ClassName) macro in your DLL's .cpp to generate these.
//
// Example plugin DLL:
//   #include "aiPlugin.h"
//   #include "MyAIService.h"
//   XL_AI_PLUGIN_EXPORT(MyAIService)

class aiBase;
class ServiceManager;

extern "C" {
    typedef aiBase* (*xlCreateAIServiceFn)(ServiceManager* sm);
    typedef void    (*xlDestroyAIServiceFn)(aiBase* service);
}

// __declspec(dllexport) is required on Windows so GetProcAddress can find the symbols.
#ifdef _WIN32
#  define XL_AI_PLUGIN_EXPORT_ATTR __declspec(dllexport)
#else
#  define XL_AI_PLUGIN_EXPORT_ATTR __attribute__((visibility("default")))
#endif

// Place this macro once in a plugin DLL .cpp file to export the required entry points.
#define XL_AI_PLUGIN_EXPORT(ClassName)                                             \
    extern "C" XL_AI_PLUGIN_EXPORT_ATTR aiBase* xlCreateAIService(ServiceManager* sm) { \
        return new ClassName(sm);                                                   \
    }                                                                               \
    extern "C" XL_AI_PLUGIN_EXPORT_ATTR void xlDestroyAIService(aiBase* service) { \
        delete service;                                                             \
    }
