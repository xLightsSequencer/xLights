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
#include <wx/stackwalk.h>
#include <wx/stdpaths.h>
#include <wx/string.h>
#include <inttypes.h>
#include <iostream>
#include <fstream>

#include <libloaderapi.h>

#include <log4cpp/Category.hh>

#define USE_MAP_TO_STACKWALK
#define FORCE_MAP_TO_STACKWALK

class xlStackWalker : public wxStackWalker
{
    bool LoadMapFile()
    {
        // dont reload it
        if (_mapFileOk)
            return true;

        static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));

        wxFileName name = wxStandardPaths::Get().GetExecutablePath();
        name.SetExt("map");

        logger_base.debug("Loading map file " + name.GetFullPath());

        HMODULE hModule = ::GetModuleHandle(nullptr);
        logger_base.debug("Base module handle: 0x%016llx", (long long)hModule);

        std::ifstream infile;
        infile.open(name.GetFullPath().ToStdString());
        long long preferedLoadAddress = 0;
        if (infile.is_open()) {
            logger_base.debug("    File open.");

            while (!infile.eof()) {
                std::string inl;
                std::getline(infile, inl);
                _mapLines.push_back(inl);
            }
            logger_base.debug("    Map file loaded.");
            return true;
        } else {
            logger_base.debug("    File not found.");
            return false;
        }
    }

    std::string FindFunction(HMODULE hmod, void* fn)
    {
        wxString addr = wxString::Format("%016llx", (uint64_t)fn - (uint64_t)hmod);
        wxString res;
        
        for (const auto& it : _mapLines) {
            if (it.substr(0, 16) > addr) {
                return res;
            }
            res = it;
        }
        return "";
    }

public:
    xlStackWalker(bool const isAssert, bool const isFatalException) :
        m_isAssert(isAssert)
    {
        if (isFatalException)
        {
            m_stackTrace << "Walking from an exception.\n";
            WalkFromException();
        }
        else
        {
            m_stackTrace << "Regular walk.\n";
            Walk();
        }
    }

    wxString const& GetStackTrace() const { return m_stackTrace; }

    virtual void OnStackFrame(wxStackFrame const& frame) override
    {
        m_numFrames += 1;
        m_stackTrace << wxString::Format(wxT("[%02u]\t"), m_numFrames);

        HMODULE hmod = 0;
        ::GetModuleHandleExA(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
                             (char*)frame.GetAddress(), &hmod);

        wxString const& name = frame.GetName();

#ifdef USE_MAP_TO_STACKWALK
#ifndef FORCE_MAP_TO_STACKWALK
        if (name == "") 
        {
#endif
            if (!_mapFileOk)
                _mapFileOk = LoadMapFile();

            m_stackTrace << wxString::Format(wxT("ADDR:0x%016" PRIx64 "\t"), (uint64_t)frame.GetAddress());
            m_stackTrace << wxString::Format(wxT("HMODULE:0x%016" PRIx64 "\t"), (uint64_t)hmod);
            m_stackTrace << FindFunction(hmod, frame.GetAddress());
            m_stackTrace << wxString::Format(wxT("@%llX"), frame.GetOffset());
            m_stackTrace << " " << frame.GetModule() << "\n";
#ifndef FORCE_MAP_TO_STACKWALK
        } else {
#endif
#endif
#ifndef FORCE_MAP_TO_STACKWALK
            if (m_isAssert && name.StartsWith("wxOnAssert")) {
                // Ignore all frames until the wxOnAssert() one, they are internal to wxWidgets and not interesting for the user
                // (but notice that if we never find the wxOnAssert() frame, e.g. because we don't have symbol info at all, we would show
                // everything which is better than not showing anything).
                m_stackTrace.clear();
                m_numFrames = 0;
            } else {
                if (name.empty()) {
                    m_stackTrace << wxString::Format(wxT("%p"), frame.GetAddress());
                } else {
                    m_stackTrace << wxString::Format(wxT("%-40s"), name.c_str());
                }

                m_stackTrace << wxString::Format(wxT("@%llX"), frame.GetOffset());

                m_stackTrace << wxT('\t') << frame.GetModule() << wxT('!') << frame.GetFileName() << wxT(':') << frame.GetLine();

                for (size_t paramNum = 0; paramNum < frame.GetParamCount(); paramNum += 1) {
                    wxString type, name, value;
                    if (frame.GetParam(paramNum, &type, &name, &value)) {
                        m_stackTrace << wxString::Format(wxT(" (%zu: %s %s = %s)"), paramNum, type, name, value);
                    } else {
                        m_stackTrace << wxString::Format(wxT(" (%zu: ? ? = ?)"), paramNum);
                    }
                }

                m_stackTrace << wxT('\n');
            }
#ifdef USE_MAP_TO_STACKWALK
        }
#endif
#endif
    }

private:
    bool const m_isAssert = false;
    wxString m_stackTrace;
    unsigned m_numFrames = 0;
    bool _mapFileOk = false;
    std::list<std::string> _mapLines;
};

#endif
