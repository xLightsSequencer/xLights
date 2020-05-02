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

#include <string>
#include <wx/xml/xml.h>

class xLightsFrame;
class Model;
class ModelPreview;
class PreviewPane;

class LayoutGroup : public wxObject
{
    public:
        LayoutGroup(const std::string & name, xLightsFrame* xl, wxXmlNode *node);
        virtual ~LayoutGroup();

        const std::string &GetName() const {return mName;}
        void SetName(const std::string & name) {mName = name;}

        void SetBackgroundImage(const wxString &filename);
        const wxString &GetBackgroundImage() const { return mBackgroundImage;}

        void SetBackgroundScaled(bool scaled);
        bool GetBackgroundScaled() {return mScaleBackgroundImage;}

        void SetBackgroundBrightness(int i, int a);
        int GetBackgroundBrightness() {return mBackgroundBrightness;}
        int GetBackgroundAlpha() {return mBackgroundAlpha;}

        void SetPreviewPosition(wxPoint point);
        void SetPreviewSize(wxSize size_);
        void ResetPositions();

        void SetFromXml(wxXmlNode* LayoutGroupNode);
        wxXmlNode* GetLayoutGroupXml() const;

        void SetModels(std::vector<Model*> &models);
        std::vector<Model*> &GetModels() {
            return previewModels;
        }

        ModelPreview* GetModelPreview() {return mModelPreview;}

        bool GetPreviewHidden() {return mPreviewHidden;}
        bool GetPreviewCreated() {return mPreviewCreated;}
        void SetPreviewActive(bool show);
        const long AddToPreviewMenu(wxMenu* preview_menu);
        void RemoveFromPreviewMenu(wxMenu* preview_menu);
        wxMenuItem* GetMenuItem() {return mMenuItemPreview;}
        void ShowPreview(bool show);
        const long GetMenuId() {return id_menu_item;}

    protected:
        wxXmlNode* LayoutGroupXml;

    private:
        std::string mName;
        wxString mBackgroundImage;
        bool mScaleBackgroundImage;
        int mBackgroundBrightness;
        int mBackgroundAlpha;
        std::vector<Model*> previewModels;
        bool mPreviewHidden;
        bool mPreviewCreated;
        ModelPreview* mModelPreview;
        xLightsFrame* xlights;
        wxMenuItem* mMenuItemPreview;
        PreviewPane* mPreviewPane;
        const long id_menu_item;
        int mPosX;
        int mPosY;
        int mPaneWidth;
        int mPaneHeight;
        bool ignore_size_and_pos;
};
