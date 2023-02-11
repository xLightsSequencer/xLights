/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/smeighan/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/smeighan/xLights/blob/master/License.txt
 **************************************************************/

#include <map>

#include "LayoutUtils.h"
#include "UtilFunctions.h"

#include "BitmapCache.h"

#include <wx/artprov.h>
#include <wx/imaglist.h>

namespace LayoutUtils
{
    void CreateImageList(wxVector<wxBitmapBundle> & imageList)
    {
        imageList.push_back(wxArtProvider::GetBitmapBundle("wxART_NORMAL_FILE", wxART_LIST));
        imageList.push_back(wxArtProvider::GetBitmapBundle("xlART_GROUP_CLOSED", wxART_LIST));
        imageList.push_back(wxArtProvider::GetBitmapBundle("xlART_GROUP_OPEN", wxART_LIST));
        imageList.push_back(BitmapCache::GetModelGroupIcon());
        imageList.push_back(wxArtProvider::GetBitmapBundle("xlART_ARCH_ICON", wxART_LIST));
        imageList.push_back(wxArtProvider::GetBitmapBundle("xlART_CANE_ICON", wxART_LIST));
        imageList.push_back(wxArtProvider::GetBitmapBundle("xlART_CIRCLE_ICON", wxART_LIST));
        imageList.push_back(wxArtProvider::GetBitmapBundle("xlART_CHANNELBLOCK_ICON", wxART_LIST));
        imageList.push_back(wxArtProvider::GetBitmapBundle("xlART_CUBE_ICON", wxART_LIST));
        imageList.push_back(wxArtProvider::GetBitmapBundle("xlART_CUSTOM_ICON", wxART_LIST));
        imageList.push_back(wxArtProvider::GetBitmapBundle("xlART_DMX_ICON", wxART_LIST));
        imageList.push_back(wxArtProvider::GetBitmapBundle("xlART_ICICLE_ICON", wxART_LIST));
        imageList.push_back(wxArtProvider::GetBitmapBundle("xlART_IMAGE_ICON", wxART_LIST));
        imageList.push_back(wxArtProvider::GetBitmapBundle("xlART_LINE_ICON", wxART_LIST));
        imageList.push_back(wxArtProvider::GetBitmapBundle("xlART_MATRIX_ICON", wxART_LIST));
        imageList.push_back(wxArtProvider::GetBitmapBundle("xlART_POLY_ICON", wxART_LIST));
        imageList.push_back(wxArtProvider::GetBitmapBundle("xlART_SPHERE_ICON", wxART_LIST));
        imageList.push_back(wxArtProvider::GetBitmapBundle("xlART_SPINNER_ICON", wxART_LIST));
        imageList.push_back(wxArtProvider::GetBitmapBundle("xlART_STAR_ICON", wxART_LIST));
        imageList.push_back(wxArtProvider::GetBitmapBundle("xlART_SUBMODEL_ICON", wxART_LIST));
        imageList.push_back(wxArtProvider::GetBitmapBundle("xlART_TREE_ICON", wxART_LIST));
        imageList.push_back(wxArtProvider::GetBitmapBundle("xlART_WINDOW_ICON", wxART_LIST));
        imageList.push_back(wxArtProvider::GetBitmapBundle("xlART_WREATH_ICON", wxART_LIST));
    }

    void CreateImageList(wxImageList* imageList, std::map<int, int> &idxs)
    {
        idxs[Icon_File] = imageList->Add(wxArtProvider::GetIcon("wxART_NORMAL_FILE", wxART_LIST));
        idxs[Icon_FolderClosed] = imageList->Add(wxArtProvider::GetIcon("xlART_GROUP_CLOSED", wxART_LIST));
        idxs[Icon_FolderOpened] = imageList->Add(wxArtProvider::GetIcon("xlART_GROUP_OPEN", wxART_LIST));
        idxs[Icon_Group] = imageList->Add(BitmapCache::GetModelGroupIcon().GetIcon(wxDefaultSize));
        idxs[Icon_Arches] = imageList->Add(wxArtProvider::GetIcon("xlART_ARCH_ICON", wxART_LIST));
        idxs[Icon_CandyCane] = imageList->Add(wxArtProvider::GetIcon("xlART_CANE_ICON", wxART_LIST));
        idxs[Icon_Circle] = imageList->Add(wxArtProvider::GetIcon("xlART_CIRCLE_ICON", wxART_LIST));
        idxs[Icon_ChannelBlock] = imageList->Add(wxArtProvider::GetIcon("xlART_CHANNELBLOCK_ICON", wxART_LIST));
        idxs[Icon_Cube] = imageList->Add(wxArtProvider::GetIcon("xlART_CUBE_ICON", wxART_LIST));
        idxs[Icon_Custom] = imageList->Add(wxArtProvider::GetIcon("xlART_CUSTOM_ICON", wxART_LIST));
        idxs[Icon_Dmx] = imageList->Add(wxArtProvider::GetIcon("xlART_DMX_ICON", wxART_LIST));
        idxs[Icon_Icicle] = imageList->Add(wxArtProvider::GetIcon("xlART_ICICLE_ICON", wxART_LIST));
        idxs[Icon_Image] = imageList->Add(wxArtProvider::GetIcon("xlART_IMAGE_ICON", wxART_LIST));
        idxs[Icon_Line] = imageList->Add(wxArtProvider::GetIcon("xlART_LINE_ICON", wxART_LIST));
        idxs[Icon_Matrix] = imageList->Add(wxArtProvider::GetIcon("xlART_MATRIX_ICON", wxART_LIST));
        idxs[Icon_Poly] = imageList->Add(wxArtProvider::GetIcon("xlART_POLY_ICON", wxART_LIST));
        idxs[Icon_Sphere] = imageList->Add(wxArtProvider::GetIcon("xlART_SPHERE_ICON", wxART_LIST));
        idxs[Icon_Spinner] = imageList->Add(wxArtProvider::GetIcon("xlART_SPINNER_ICON", wxART_LIST));
        idxs[Icon_Star] = imageList->Add(wxArtProvider::GetIcon("xlART_STAR_ICON", wxART_LIST));
        idxs[Icon_SubModel] = imageList->Add(wxArtProvider::GetIcon("xlART_SUBMODEL_ICON", wxART_LIST));
        idxs[Icon_Tree] = imageList->Add(wxArtProvider::GetIcon("xlART_TREE_ICON", wxART_LIST));
        idxs[Icon_Window] = imageList->Add(wxArtProvider::GetIcon("xlART_WINDOW_ICON", wxART_LIST));
        idxs[Icon_Wreath] = imageList->Add(wxArtProvider::GetIcon("xlART_WREATH_ICON", wxART_LIST));
    }

    int GetModelTreeIcon(std::string const& type, GroupMode mode)
    {
        if (type == "ModelGroup") {
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