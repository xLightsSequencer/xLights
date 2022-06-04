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


class xlStackWalker : public wxStackWalker
{
public:
    xlStackWalker(bool const isAssert, bool const isFatalException) :
        m_isAssert(isAssert),
        m_stackTrace(),
        m_numFrames(0)
    {
        if (isFatalException)
        {
            WalkFromException();
        }
        else
        {
            Walk();
        }
    }

    wxString const& GetStackTrace() const { return m_stackTrace; }

    virtual void OnStackFrame(wxStackFrame const& frame) override
    {
        m_numFrames += 1;
        m_stackTrace << wxString::Format(wxT("[%02u] "), m_numFrames);

        wxString const& name = frame.GetName();

        if (m_isAssert && name.StartsWith("wxOnAssert"))
        {
            // Ignore all frames until the wxOnAssert() one, they are internal to wxWidgets and not interesting for the user
            // (but notice that if we never find the wxOnAssert() frame, e.g. because we don't have symbol info at all, we would show
            // everything which is better than not showing anything).
            m_stackTrace.clear();
            m_numFrames = 0;
        }
        else
        {
            if (name.empty())
            {
                m_stackTrace << wxString::Format(wxT("%p"), frame.GetAddress());
            }
            else
            {
                m_stackTrace << wxString::Format(wxT("%-40s"), name.c_str());
            }

            m_stackTrace << wxString::Format(wxT("@%llX"), frame.GetOffset());

            m_stackTrace << wxT('\t') << frame.GetModule() << wxT('!') << frame.GetFileName() << wxT(':') << frame.GetLine();

            for (size_t paramNum = 0; paramNum < frame.GetParamCount(); paramNum += 1)
            {
                wxString type, name, value;
                if (frame.GetParam(paramNum, &type, &name, &value))
                {
                    m_stackTrace << wxString::Format(wxT(" (%zu: %s %s = %s)"), paramNum, type, name, value);
                }
                else
                {
                    m_stackTrace << wxString::Format(wxT(" (%zu: ? ? = ?)"), paramNum);
                }
            }

            m_stackTrace << wxT('\n');
        }
    }

private:
    bool const m_isAssert;
    wxString m_stackTrace;
    unsigned m_numFrames;
};

#endif
