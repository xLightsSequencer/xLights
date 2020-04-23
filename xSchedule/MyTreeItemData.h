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

#include <wx/treebase.h>

class MyTreeItemData : public wxTreeItemData
{
    void* _data = nullptr;

public:
    MyTreeItemData(void* data) { _data = data; }
    void* GetData() const { return _data; }
    void SetData(void* data) { _data = data; }
};

