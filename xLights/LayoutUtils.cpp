/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include <map>

#include "LayoutUtils.h"
#include "UtilFunctions.h"

#include "BitmapCache.h"

#include <wx/artprov.h>
#include <wx/imaglist.h>

namespace LayoutUtils
{
    inline int pushBack(wxVector<wxBitmapBundle> & imageList, const wxBitmapBundle &bb) {
        int i = imageList.size();
        imageList.push_back(bb);
        return i;
    }
    void CreateImageList(wxVector<wxBitmapBundle> & imageList, std::map<int, int> &idxs) {
        wxSize sz(16, 16);
        idxs[Icon_File] = pushBack(imageList, wxArtProvider::GetBitmapBundle("wxART_NORMAL_FILE", wxART_LIST, sz));
        idxs[Icon_FolderClosed] = pushBack(imageList, wxArtProvider::GetBitmapBundle("xlART_GROUP_CLOSED", wxART_LIST, sz));
        idxs[Icon_FolderOpened] = pushBack(imageList, wxArtProvider::GetBitmapBundle("xlART_GROUP_OPEN", wxART_LIST, sz));
        idxs[Icon_OrgGroup] = pushBack(imageList, wxArtProvider::GetBitmapBundle("wxART_HELP_FOLDER", wxART_LIST, sz));
        idxs[Icon_Group] = pushBack(imageList, BitmapCache::GetModelGroupIcon());
        idxs[Icon_Arches] = pushBack(imageList, wxArtProvider::GetBitmapBundle("xlART_ARCH_ICON", wxART_LIST));
        idxs[Icon_CandyCane] = pushBack(imageList, wxArtProvider::GetBitmapBundle("xlART_CANE_ICON", wxART_LIST));
        idxs[Icon_Circle] = pushBack(imageList, wxArtProvider::GetBitmapBundle("xlART_CIRCLE_ICON", wxART_LIST));
        idxs[Icon_ChannelBlock] = pushBack(imageList, wxArtProvider::GetBitmapBundle("xlART_CHANNELBLOCK_ICON", wxART_LIST));
        idxs[Icon_Cube] = pushBack(imageList, wxArtProvider::GetBitmapBundle("xlART_CUBE_ICON", wxART_LIST));
        idxs[Icon_Custom] = pushBack(imageList, wxArtProvider::GetBitmapBundle("xlART_CUSTOM_ICON", wxART_LIST));
        idxs[Icon_Dmx] = pushBack(imageList, wxArtProvider::GetBitmapBundle("xlART_DMX_ICON", wxART_LIST));
        idxs[Icon_Icicle] = pushBack(imageList, wxArtProvider::GetBitmapBundle("xlART_ICICLE_ICON", wxART_LIST));
        idxs[Icon_Image] = pushBack(imageList, wxArtProvider::GetBitmapBundle("xlART_IMAGE_ICON", wxART_LIST));
        idxs[Icon_Line] = pushBack(imageList, wxArtProvider::GetBitmapBundle("xlART_LINE_ICON", wxART_LIST));
        idxs[Icon_Matrix] = pushBack(imageList, wxArtProvider::GetBitmapBundle("xlART_MATRIX_ICON", wxART_LIST));
        idxs[Icon_Poly] = pushBack(imageList, wxArtProvider::GetBitmapBundle("xlART_POLY_ICON", wxART_LIST));
        idxs[Icon_Sphere] = pushBack(imageList, wxArtProvider::GetBitmapBundle("xlART_SPHERE_ICON", wxART_LIST));
        idxs[Icon_Spinner] = pushBack(imageList, wxArtProvider::GetBitmapBundle("xlART_SPINNER_ICON", wxART_LIST));
        idxs[Icon_Star] = pushBack(imageList, wxArtProvider::GetBitmapBundle("xlART_STAR_ICON", wxART_LIST));
        idxs[Icon_SubModel] = pushBack(imageList, wxArtProvider::GetBitmapBundle("xlART_SUBMODEL_ICON", wxART_LIST));
        idxs[Icon_Tree] = pushBack(imageList, wxArtProvider::GetBitmapBundle("xlART_TREE_ICON", wxART_LIST));
        idxs[Icon_Window] = pushBack(imageList, wxArtProvider::GetBitmapBundle("xlART_WINDOW_ICON", wxART_LIST));
        idxs[Icon_Wreath] = pushBack(imageList, wxArtProvider::GetBitmapBundle("xlART_WREATH_ICON", wxART_LIST));
    }
    void CreateImageList(wxVector<wxBitmapBundle> & imageList) {
        std::map<int, int> idxs;
        CreateImageList(imageList, idxs);
    }

    int GetModelTreeIcon(std::string const& type, GroupMode mode, bool orgGroup)
    {
        if (type == "ModelGroup") {
            if (orgGroup == true) {
                return Icon_OrgGroup;
            }
            if (mode == GroupMode::Opened) {
                return Icon_FolderOpened;
            }
            if (mode == GroupMode::Closed) {
                return Icon_FolderClosed;
            }
            if (mode == GroupMode::Regular) {
                return Icon_Group;
            }
        }
        if( type == "Arches" ) {
            return Icon_Arches;
        } else if( type == "Candy Canes" ) {
            return Icon_CandyCane;
        } else if( type == "Circle" ) {
            return Icon_Circle;
        } else if (type == "Channel Block") {
            return Icon_ChannelBlock;
        } else if( type == "Cube" ) {
            return Icon_Cube;
        } else if( type == "Custom" ) {
            return Icon_Custom;
        } else if (Contains(type, "Dmx") ||
                   Contains(type, "DMX")) {
            return Icon_Dmx;
        } else if( type == "Image" ) {
            return Icon_Image;
        } else if( type == "Icicles" ) {
            return Icon_Icicle;
        } else if( type == "Single Line" ) {
            return Icon_Line;
        } else if (Contains(type,"Matrix")) {
            return Icon_Matrix;
        } else if( type == "Poly Line" ) {
            return Icon_Poly;
        } else if( type == "Sphere" ) {
            return Icon_Sphere;
        } else if( type == "Spinner" ) {
            return Icon_Spinner;
        } else if( type == "Star" ) {
            return Icon_Star;
        } else if( type == "SubModel" ) {
            return Icon_SubModel;
        } else if (Contains(type,"Tree")) {
            return Icon_Tree;
        } else if( type == "Wreath" ) {
            return Icon_Wreath;
        } else if( type == "Window Frame" ) {
            return Icon_Window;
        }
        return Icon_File;
    }
}
