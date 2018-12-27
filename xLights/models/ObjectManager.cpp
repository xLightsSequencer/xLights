#include <wx/regex.h>
#include <cctype>

#include "ObjectManager.h"

ObjectManager::ObjectManager()
{
    //ctor
}

ObjectManager::~ObjectManager()
{
    //dtor
}

// generate the next similar object name to the candidateName we are given
std::string ObjectManager::GenerateObjectName(const std::string& candidateName) const
{
    // if it is already unique return it
    if (GetObject(candidateName) == nullptr) return candidateName;

    std::string base = candidateName;
    char sep = '-';

    static wxRegEx dashRegex("-[0-9]+$", wxRE_ADVANCED);
    static wxRegEx underscoreRegex("_[0-9]+$", wxRE_ADVANCED);
    static wxRegEx spaceRegex(" [0-9]+$", wxRE_ADVANCED);
    static wxRegEx nilRegex("[A-Za-z][0-9]+$", wxRE_ADVANCED);
    if (dashRegex.Matches(candidateName))
    {
        base = wxString(candidateName).BeforeLast('-');
    }
    else if (underscoreRegex.Matches(candidateName))
    {
        base = wxString(candidateName).BeforeLast('_');
        sep = '_';
    }
    else if (spaceRegex.Matches(candidateName))
    {
        base = wxString(candidateName).BeforeLast(' ');
        sep = ' ';
    }
    else if (nilRegex.Matches(candidateName))
    {
        while (base != "" && std::isdigit(base[base.size() - 1]))
        {
            base = base.substr(0, base.size() - 1);
        }
        sep = 'x';
    }

    // We start at 2 assuming if we are adding multiple then the user will typically rename the first one number one.
    int seq = 2;

    for (;;)
    {
        std::string tryName = base;

        if (sep == 'x')
        {
            tryName += std::to_string(seq++);
        }
        else
        {
            tryName += sep + std::to_string(seq++);
        }

        if (GetObject(tryName) == nullptr) return tryName;
    }
}
