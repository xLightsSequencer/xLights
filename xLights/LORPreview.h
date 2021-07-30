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

/* Scott Adventure's in Reverse Engineering the LOR S5 Preview File
 * 
 * Screen Location: Bottom Left is  OffsetX = -1, OffsetY = -1
 *                  Bottom Right is OffsetX =  1, OffsetY = -1
 *                  Top Left is     OffsetX = -1, OffsetY =  1
 *                  Top Right is    OffsetX =  1, OffsetY =  1
 *                  Middle is       OffsetX =  0, OffsetY =  0
 *                  Values are floats
 * 
 * Scaling:         ScaleX = 2 is full width, ScaleY = 2 is full height
 *                  Values are floats
 * 
 * Rotation:        Radians = -1.53  (pi/2) is 90 degrees CW Rotation
 *                  Radians = -4.725 (2pi - pi/2) is 90 degrees CCW Rotation
 *                  Values are floats
 *                  Values are always negative
 * 
 * Models With Points are absolute to the preview X,Y, not relative to OffsetX, OffsetY. 
 * 
 */

#include <vector>
#include <map>
#include "Color.h"
#include <wx/xml/xml.h>

class Model;
class xLightsFrame;

struct S5Point {

    S5Point()
    {}

    S5Point(float x_, float y_) :
        x(x_),
        y(y_)
    {}

    S5Point(S5Point const& pt) :
        x(pt.x),
        y(pt.y)
    {}

    float x{ 0.0f };
    float y{ 0.0f };
};

struct S5Model {
    wxString id;
    wxString name;
    wxString shapeName;
    wxString deviceType;
    wxString bulbShape;
    wxString rgbOrder;
    wxString DeviceType;
    wxString startLocation;
    wxString stringType;
    wxString traditionalColors;
    wxString traditionalType;
    wxString channelGrid;
    int previewBulbSize{2};
    bool separateIds{ false };
    bool individualChannels{ false };
    int opacity { 0 };

    std::vector<int> parms;

    S5Point offset{ 0.0f , 0.0f };
    S5Point scale{ 0.0f , 0.0f };
    float radians{ 0.0f };
    wxString customWidth;
    wxString customHeight;
    wxString customGrid;
    std::vector<S5Point> points;

    bool ParseXML(wxXmlNode* m);
private:
    void ParseParms(wxXmlNode* p);
    void ParsePoints(wxXmlNode* p);
};

struct S5Group {
    wxString id;
    wxString name;
    wxString arrangement;
    std::vector<wxString> modelIds;

    bool ParseXML(wxXmlNode* g);

};

class LORPreview {

public:
    LORPreview(xLightsFrame* frame, wxString xLigthsPreview);
    virtual ~LORPreview() {};

    bool LoadPreviewFile();
    Model* LoadModelFile(wxString const& modelFile, wxString const& startChan, int previewW, int previewH, bool & error);

private:
    //member varables
    xLightsFrame* xlights;
    const wxString xLigths_preview;

    //private functions
    wxString FindLORPreviewFile();
    wxArrayString GetPreviews(wxXmlNode* root) const;
    Model* CreateModel(S5Model const& model, wxString const& startChan, int previewW, int previewH, bool& error);

    bool LoadPreview(wxXmlNode* root, wxString const& name);
    bool ReadPreview(wxXmlNode* preview);

    wxString DecodeTreeType(wxString const& value) const;
    void SetStartChannel(S5Model const& model, Model* xModel, bool doMultiString);
    bool GetStartUniverseChan( wxString const& value, int& unv, int& chan ) const;
    void SetDirection(S5Model const& model, Model* xModel);
    void SetStringType(S5Model const& model, Model* xModel);
    void SetBulbTypeSize( S5Model const& model, Model* xModel );

    void ScaleToPreview(S5Model const& model, Model* m, int pvwW, int pvwH) const;
    void ScalePointsToSingleLine(S5Model const& model, Model* m, int pvwW, int pvwH) const;
    void ScaleModelToSingleLine(S5Model const& model, Model* m, int pvwW, int pvwH) const;
    void ScaleIcicleToSingleLine( S5Model const& model, int maxdrop, Model* m, int pvwW, int pvwH ) const;

    S5Point ScalePointToXLights(S5Point const& pt, int pvwW, int pvwH) const;
    S5Point GetXLightsSizeFromScale( S5Point const& scale, int pvwW, int pvwH ) const;

    void CreateGroup(S5Group const& grp, std::vector < S5Model > const& models);

#ifdef _DEBUG
    void RunTests();
#endif

};
