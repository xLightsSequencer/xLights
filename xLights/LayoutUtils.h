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

#include <wx/bmpbuttn.h>
#include <wx/vector.h>

class wxImageList;

namespace LayoutUtils
{
    enum
    {
        Icon_File,
        Icon_FolderClosed,
        Icon_FolderOpened,
        Icon_Group,
        Icon_Arches,
        Icon_CandyCane,
        Icon_Circle,
        Icon_ChannelBlock,
        Icon_Cube,
        Icon_Custom,
        Icon_Dmx,
        Icon_Icicle,
        Icon_Image,
        Icon_Line,
        Icon_Matrix,
        Icon_Poly,
        Icon_Sphere,
        Icon_Spinner,
        Icon_Star,
        Icon_SubModel,
        Icon_Tree,
        Icon_Window,
        Icon_Wreath
    };

    enum class GroupMode{
        Closed,
        Opened,
        Regular
        
    };

    void CreateImageList(wxVector<wxBitmapBundle> & imageList);
    void CreateImageList(wxImageList* imageList, std::map<int, int> &remap);
    int GetModelTreeIcon(std::string const& type, GroupMode mode);

};