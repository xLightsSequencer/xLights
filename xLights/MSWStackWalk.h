#pragma once

/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/smeighan/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/smeighan/xLights/blob/master/License.txt
 **************************************************************/

#ifdef __WXMSW__
    #include <wx/textfile.h>
    #include <wx/stdpaths.h>
    #include <wx/string.h>

    #include <windows.h>
    #include <imagehlp.h>

#ifdef _MSC_VER
#include <wx/stackwalk.h>

class MyStackWalk : public wxStackWalker
{
    wxArrayString& _mapLines;
public:

    wxString _result = "";

    MyStackWalk(wxArrayString& mapLines) : wxStackWalker(), _mapLines(mapLines) {}

    virtual void OnStackFrame(const wxStackFrame& frame) override
    {
        long long baseAddress = (long long)::GetModuleHandle(nullptr);

        if (frame.GetName() != "")
        {
            _result += wxString::Format("0x%016llx\t%s\t%s\t%s\t%ld\n", (long long)frame.GetAddress() - baseAddress, frame.GetFileName(), frame.GetModule(), frame.GetName(), (long)frame.GetLine());
        }
        else
        {
            wxString buffer(wxString::Format("%016llx", (long long)frame.GetAddress() - baseAddress));
            for (size_t x = 1; x < _mapLines.GetCount(); x++) {
                if (wxString(buffer) < _mapLines[x]) {
                    buffer = "0x" + buffer + "\t" + _mapLines[x - 1].AfterFirst('\t');
                    break;
                }
            }

            _result += buffer + "\n";
        }
    }
};

#include <iostream>
#include <fstream>

wxString windows_get_stacktrace(void* data)
{
    static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    wxString trace;

    wxArrayString mapLines;
    wxFileName name = wxStandardPaths::Get().GetExecutablePath();
    name.SetExt("map");

    logger_base.debug("Loading map file " + name.GetFullPath());

    HMODULE hModule = ::GetModuleHandle(nullptr);
    logger_base.debug("Base module handle: 0x%016llx", (long long)hModule);

    std::ifstream infile;
    infile.open(name.GetFullPath().ToStdString());
    long long preferedLoadAddress = 0;
    if (infile.is_open())
    {
        logger_base.debug("    File open.");

        while (!infile.eof())
        {
            std::string inl;
            std::getline(infile, inl);
            wxString line(inl);
            line.Trim(true).Trim(false);

            if (line.Contains("Preferred load address is"))
            {
                sscanf(line.substr(28).c_str(), "%llx", &preferedLoadAddress);
            }

            if (line.StartsWith("0001:") && line.EndsWith(".obj")) {
                while (line.Replace("  ", " ") > 0);
                auto comp = wxSplit(line, ' ');
                if (comp.size() > 3)
                {
                    int ln = 0;
                    for (int i = 3; i < comp.size(); i++)
                    {
                        if (comp[i].size() > 2)
                        {
                            ln = i;
                            break;
                        }
                    }

                    long long addr = 0;
                    sscanf(comp[2].c_str(), "%llx", &addr);

                    auto l = wxString::Format("%016llx\t\t%s\t%s", addr - preferedLoadAddress, comp[ln], comp[1]);
                    mapLines.Add(l);
                    //logger_base.debug("Map file line: %s", (const char *)line.c_str());
                }
            }
        }
        mapLines.Sort();
        logger_base.debug("    Map file loaded.");
    }
    else
    {
        trace += name.GetFullPath() + " does not exist\n";
        logger_base.debug("    File not found.");
    }

    logger_base.debug("Preferred load address: 0x%016llx", preferedLoadAddress);

    MyStackWalk sw(mapLines);
    sw.WalkFromException();

    return sw._result;
}
#else
    #ifdef _WIN64
        wxString windows_get_stacktrace(void *data)
        {
            if (data == nullptr) return "No trace.";

            //static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));

            wxString trace;
            CONTEXT *context = (CONTEXT*)data;
            SymInitialize(GetCurrentProcess(), 0, true);

            STACKFRAME frame = { 0 };

            wxArrayString mapLines;
            wxFileName name = wxStandardPaths::Get().GetExecutablePath();
            name.SetExt("map");
            wxTextFile mapFile(name.GetFullPath());
            if (mapFile.Exists()) {
                mapFile.Open();
                wxString line = mapFile.GetFirstLine();
                while (!mapFile.Eof()) {
                    line = mapFile.GetNextLine();
                    line.Trim(true).Trim(false);
                    if (line.StartsWith("0x") && !line.EndsWith(".o") && !line.EndsWith(".o)")) {
                        mapLines.Add(line);
                        //logger_base.debug("Map file line: %s", (const char *)line.c_str());
                    }
                }
                mapLines.Sort();
            }
            else {
                trace += name.GetFullPath() + " does not exist\n";
            }

            // setup initial stack frame
            frame.AddrPC.Offset = context->Rip;
            frame.AddrPC.Mode = AddrModeFlat;
            frame.AddrStack.Offset = context->Rsp;
            frame.AddrStack.Mode = AddrModeFlat;
            frame.AddrFrame.Offset = context->Rbp;
            frame.AddrFrame.Mode = AddrModeFlat;

            while (StackWalk64(IMAGE_FILE_MACHINE_AMD64,
                GetCurrentProcess(),
                GetCurrentThread(),
                &frame,
                context,
                0,
                SymFunctionTableAccess64,
                SymGetModuleBase64,
                0))
            {
                //logger_base.debug("Rip: %I64x Rsp: %I64x Rbp: %I64x", frame.AddrPC.Offset, frame.AddrStack.Offset, frame.AddrFrame.Offset);

                static char symbolBuffer[sizeof(IMAGEHLP_SYMBOL) + 255];
                memset(symbolBuffer, 0, sizeof(IMAGEHLP_SYMBOL) + 255);
                IMAGEHLP_SYMBOL * symbol = (IMAGEHLP_SYMBOL*)symbolBuffer;

                // Need to set the first two fields of this symbol before obtaining name info:
                symbol->SizeOfStruct = sizeof(IMAGEHLP_SYMBOL) + 255;
                symbol->MaxNameLength = 254;

                // The displacement from the beginning of the symbol is stored here: pretty useless
                DWORD64 displacement = 0;

                // Get the symbol information from the address of the instruction pointer register:
                if (SymGetSymFromAddr64(
                    GetCurrentProcess(),   // Process to get symbol information for
                    frame.AddrPC.Offset,   // Address to get symbol for: instruction pointer register
                    (DWORD64*)& displacement,   // Displacement from the beginning of the symbol: whats this for ?
                    symbol                      // Where to save the symbol
                )) {
                    // Add the name of the function to the function list:
                    char buffer[2048]; sprintf(buffer, "0x%08llx %s offset: 0x%08llx\n", frame.AddrPC.Offset, symbol->Name, displacement);
                    trace += buffer;
                    //logger_base.debug("SymGetSymFromAddr64: %s", (const char *)buffer);
                }
                else {
                    // Print an unknown location:
                    // functionNames.push_back("unknown location");
                    wxString buffer(wxString::Format("0x%016llx", frame.AddrPC.Offset));
                    for (size_t x = 1; x < mapLines.GetCount(); x++) {
                        if (wxString(buffer) < mapLines[x]) {
                            buffer += mapLines[x - 1].substr(12).Trim();
                            x = mapLines.GetCount();
                        }
                    }
                    trace += buffer + "\n";
                    //logger_base.debug("Unknown: %s", (const char *)buffer);
                }
            }

            SymCleanup(GetCurrentProcess());
            return trace;
        }
    #else
        wxString windows_get_stacktrace(void *data)
        {
            if (data == nullptr) return "No trace.";

            wxString trace;
            CONTEXT *context = (CONTEXT*)data;
            SymInitialize(GetCurrentProcess(), 0, true);

            STACKFRAME frame = { 0 };

            wxArrayString mapLines;
            wxFileName name = wxStandardPaths::Get().GetExecutablePath();
            name.SetExt("map");
            wxTextFile mapFile(name.GetFullPath());
            if (mapFile.Exists()) {
                mapFile.Open();
                wxString line = mapFile.GetFirstLine();
                while (!mapFile.Eof()) {
                    line = mapFile.GetNextLine();
                    line.Trim(true).Trim(false);
                    if (line.StartsWith("0x")) {
                        mapLines.Add(line);
                    }
                }
                mapLines.Sort();
            }
            else {
                trace += name.GetFullPath() + " does not exist\n";
            }

            // setup initial stack frame
            frame.AddrPC.Offset = context->Eip;
            frame.AddrPC.Mode = AddrModeFlat;
            frame.AddrStack.Offset = context->Esp;
            frame.AddrStack.Mode = AddrModeFlat;
            frame.AddrFrame.Offset = context->Ebp;
            frame.AddrFrame.Mode = AddrModeFlat;

            while (StackWalk(IMAGE_FILE_MACHINE_I386,
                GetCurrentProcess(),
                GetCurrentThread(),
                &frame,
                context,
                0,
                SymFunctionTableAccess,
                SymGetModuleBase,
                0))
            {
                static char symbolBuffer[sizeof(IMAGEHLP_SYMBOL) + 255];
                memset(symbolBuffer, 0, sizeof(IMAGEHLP_SYMBOL) + 255);
                IMAGEHLP_SYMBOL * symbol = (IMAGEHLP_SYMBOL*)symbolBuffer;

                // Need to set the first two fields of this symbol before obtaining name info:
                symbol->SizeOfStruct = sizeof(IMAGEHLP_SYMBOL) + 255;
                symbol->MaxNameLength = 254;

                // The displacement from the beginning of the symbol is stored here: pretty useless
                unsigned displacement = 0;

                // Get the symbol information from the address of the instruction pointer register:
                if (SymGetSymFromAddr(
                    GetCurrentProcess(),   // Process to get symbol information for
                    frame.AddrPC.Offset,   // Address to get symbol for: instruction pointer register
                    (DWORD*)& displacement,   // Displacement from the beginning of the symbol: whats this for ?
                    symbol                      // Where to save the symbol
                )) {
                    // Add the name of the function to the function list:
                    char buffer[2048]; sprintf(buffer, "0x%08x %s\n", frame.AddrPC.Offset, symbol->Name);
                    trace += buffer;
                }
                else {
                    // Print an unknown location:
                    // functionNames.push_back("unknown location");
                    wxString buffer(wxString::Format("0x%08x", frame.AddrPC.Offset));
                    for (size_t x = 1; x < mapLines.GetCount(); x++) {
                        if (wxString(buffer) < mapLines[x]) {
                            buffer += mapLines[x - 1].substr(12).Trim();
                            x = mapLines.GetCount();
                        }
                    }
                    trace += buffer + "\n";
                }
            }

            SymCleanup(GetCurrentProcess());
            return trace;
        }
    #endif // _WIN64
#endif
#endif
