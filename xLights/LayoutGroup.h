#ifndef LAYOUTGROUP_H
#define LAYOUTGROUP_H

#include <string>
#include <wx/xml/xml.h>

class xLightsFrame;
class Model;
class ModelPreview;

class LayoutGroup : public wxObject
{
    public:
        LayoutGroup(const std::string & name, xLightsFrame* xl, wxXmlNode *node, wxString bkImage = "");
        virtual ~LayoutGroup();

        const std::string &GetName() const {return mName;}
        void SetName(const std::string & name) {mName = name;}

        void SetBackgroundImage(const wxString &filename);
        const wxString &GetBackgroundImage() const { return mBackgroundImage;}

        void SetBackgroundScaled(bool scaled);
        bool GetBackgroundScaled() {return mScaleBackgroundImage;}

        void SetBackgroundBrightness(int i);
        int GetBackgroundBrightness() {return mBackgroundBrightness;}

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
        wxMenuItem* GetMenuItem() {return mMenuItemPreview;}
        void ShowHidePreview();
        const long GetMenuId() {return id_menu_item;}

    protected:
        wxXmlNode* LayoutGroupXml;

    private:
        std::string mName;
        wxString mBackgroundImage;
        bool mScaleBackgroundImage;
        int mBackgroundBrightness;
        std::vector<Model*> previewModels;
        bool mPreviewHidden;
        bool mPreviewCreated;
        ModelPreview* mModelPreview;
        xLightsFrame* xlights;
        wxMenuItem* mMenuItemPreview;
        const long id_menu_item;

};

#endif // LAYOUTGROUP_H
