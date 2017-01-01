#ifndef MYTREEITEMDATA_H
#define MYTREEITEMDATA_H

#include <wx/treebase.h>

class MyTreeItemData : public wxTreeItemData
{
    void* _data;

public:
    MyTreeItemData(void* data) { _data = data; }
    void* GetData() const { return _data; }
    void SetData(void* data) { _data = data; }
};

#endif