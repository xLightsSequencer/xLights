#include "ViewpointMgr.h"
#include <wx/wx.h>
#include <wx/xml/xml.h>

PreviewCamera::PreviewCamera(bool is_3d_)
: posX(-500.0f), posY(0.0f), angleX(20.0f), angleY(5.0f), distance(-2000.0f), zoom(1.0f),
  panx(0.0f), pany(0.0f), zoom_corrx(0.0f), zoom_corry(0.0f), is_3d(is_3d_), name("Name Unspecified"), menu_id(wxNewId())
{
}

PreviewCamera::~PreviewCamera()
{
}

// Copy constructor
PreviewCamera::PreviewCamera(const PreviewCamera &cam)
: posX(cam.posX), posY(cam.posY), angleX(cam.angleX), angleY(cam.angleY), distance(cam.distance), zoom(cam.zoom),
  panx(cam.panx), pany(cam.pany), zoom_corrx(cam.zoom_corrx), zoom_corry(cam.zoom_corry), is_3d(cam.is_3d), name(cam.name), menu_id(wxNewId())
{
}

PreviewCamera& PreviewCamera::operator= (const PreviewCamera& rhs)
{
    if (this == &rhs) return *this;   // Gracefully handle self assignment
    posX = rhs.posX;
    posY = rhs.posY;
    angleX = rhs.angleX;
    angleY = rhs.angleY;
    distance = rhs.distance;
    zoom = rhs.zoom;
    panx = rhs.panx;
    pany = rhs.pany;
    zoom_corrx = rhs.zoom_corrx;
    zoom_corry = rhs.zoom_corry;
    is_3d = rhs.is_3d;
    return *this;
}

ViewpointMgr::ViewpointMgr()
{
    //ctor
}

ViewpointMgr::~ViewpointMgr()
{
    previewCameras2d.clear();
    previewCameras3d.clear();
}

void ViewpointMgr::AddCamera( std::string name, PreviewCamera* current_camera, bool is_3d )
{
    PreviewCamera* new_camera;
    if (current_camera != nullptr) {
        new_camera = new PreviewCamera(*current_camera);
    }
    else {
        new_camera = new PreviewCamera(is_3d);
    }
    new_camera->name = name;
    if (new_camera->name == "") {
        new_camera->name = "...";  // avoid exception that occurs if menu name is blank
    }
    if (is_3d) {
        previewCameras3d.push_back(new_camera);
    }
    else {
        previewCameras2d.push_back(new_camera);
    }
}

void ViewpointMgr::Save(wxXmlDocument* doc)
{
	wxXmlNode* vp_node = nullptr;

	// find an existing view node in the document and delete it
	for (wxXmlNode* e = doc->GetRoot()->GetChildren(); e != nullptr; e = e->GetNext())
	{
		if (e->GetName() == "Viewpoints") vp_node = e;
	}
	if (vp_node != nullptr)
	{
		doc->GetRoot()->RemoveChild(vp_node);
	}

	wxXmlNode* newnode = Save();
    doc->GetRoot()->AddChild(newnode);
}

wxXmlNode* ViewpointMgr::Save() const
{
	wxXmlNode* node = new wxXmlNode(wxXML_ELEMENT_NODE, "Viewpoints");

	for( size_t i = 0; i < previewCameras2d.size(); ++i )
	{
	    wxXmlNode* cnode = new wxXmlNode(wxXML_ELEMENT_NODE, previewCameras2d[i]->name);
	    cnode->AddAttribute("posX", wxString::Format("%f", previewCameras2d[i]->posX));
	    cnode->AddAttribute("posY", wxString::Format("%f", previewCameras2d[i]->posY));
	    cnode->AddAttribute("angleX", wxString::Format("%f", previewCameras2d[i]->angleX));
	    cnode->AddAttribute("angleY", wxString::Format("%f", previewCameras2d[i]->angleY));
	    cnode->AddAttribute("distance", wxString::Format("%f", previewCameras2d[i]->distance));
	    cnode->AddAttribute("zoom", wxString::Format("%f", previewCameras2d[i]->zoom));
	    cnode->AddAttribute("panx", wxString::Format("%f", previewCameras2d[i]->panx));
	    cnode->AddAttribute("pany", wxString::Format("%f", previewCameras2d[i]->pany));
	    cnode->AddAttribute("zoom_corrx", wxString::Format("%f", previewCameras2d[i]->zoom_corrx));
	    cnode->AddAttribute("zoom_corry", wxString::Format("%f", previewCameras2d[i]->zoom_corry));
	    cnode->AddAttribute("is_3d", wxString::Format("%d", previewCameras2d[i]->is_3d));
        node->AddChild(cnode);
	}

	for( size_t i = 0; i < previewCameras3d.size(); ++i )
	{
	    wxXmlNode* cnode = new wxXmlNode(wxXML_ELEMENT_NODE, previewCameras3d[i]->name);
	    cnode->AddAttribute("posX", wxString::Format("%f", previewCameras3d[i]->posX));
	    cnode->AddAttribute("posY", wxString::Format("%f", previewCameras3d[i]->posY));
	    cnode->AddAttribute("angleX", wxString::Format("%f", previewCameras3d[i]->angleX));
	    cnode->AddAttribute("angleY", wxString::Format("%f", previewCameras3d[i]->angleY));
	    cnode->AddAttribute("distance", wxString::Format("%f", previewCameras3d[i]->distance));
	    cnode->AddAttribute("zoom", wxString::Format("%f", previewCameras3d[i]->zoom));
	    cnode->AddAttribute("panx", wxString::Format("%f", previewCameras3d[i]->panx));
	    cnode->AddAttribute("pany", wxString::Format("%f", previewCameras3d[i]->pany));
	    cnode->AddAttribute("zoom_corrx", wxString::Format("%f", previewCameras3d[i]->zoom_corrx));
	    cnode->AddAttribute("zoom_corry", wxString::Format("%f", previewCameras3d[i]->zoom_corry));
	    cnode->AddAttribute("is_3d", wxString::Format("%d", previewCameras3d[i]->is_3d));
        node->AddChild(cnode);
	}

	return node;
}

void ViewpointMgr::Load(wxXmlNode* vp_node)
{
	if (vp_node != nullptr)
	{
        previewCameras2d.clear();
        previewCameras3d.clear();
        for (wxXmlNode* c = vp_node->GetChildren(); c != nullptr; c = c->GetNext())
        {
            std::string name = c->GetName().ToStdString();
            wxString is_3d_;
            c->GetAttribute("is_3d", &is_3d_);
            bool is_3d = wxAtoi(is_3d_);
            PreviewCamera* new_camera = new PreviewCamera(is_3d);
            new_camera->name = name;
            wxString attr;
            c->GetAttribute("posX", &attr);
            new_camera->posX = wxAtof(attr);
            c->GetAttribute("posY", &attr);
            new_camera->posY = wxAtof(attr);
            c->GetAttribute("angleX", &attr);
            new_camera->angleX = wxAtof(attr);
            c->GetAttribute("angleY", &attr);
            new_camera->angleY = wxAtof(attr);
            c->GetAttribute("distance", &attr);
            new_camera->distance = wxAtof(attr);
            c->GetAttribute("zoom", &attr);
            new_camera->zoom = wxAtof(attr);
            c->GetAttribute("panx", &attr);
            new_camera->panx = wxAtof(attr);
            c->GetAttribute("pany", &attr);
            new_camera->pany = wxAtof(attr);
            c->GetAttribute("zoom_corrx", &attr);
            new_camera->zoom_corrx = wxAtof(attr);
            c->GetAttribute("zoom_corry", &attr);
            new_camera->zoom_corry = wxAtof(attr);
            if( is_3d ) {
                previewCameras3d.push_back(new_camera);
            } else {
                previewCameras2d.push_back(new_camera);
            }
        }
	}
}


