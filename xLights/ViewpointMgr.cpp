#include "ViewpointMgr.h"
#include <wx/wx.h>
#include <wx/xml/xml.h>
#include <glm/mat4x4.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "UtilFunctions.h"

void PreviewCamera::Reset()
{
    posX = 0.0f;
    posY = 0.0f;
    posZ = 0.0f;
    angleX = 20.0f;
    angleY = 5.0f;
    distance = -2000.0f;
    zoom = 1.0f;
    panx = 0.0f; 
    pany = 0.0f;
    panz = 0.0f;
    zoom_corrx = 0.0f;
    zoom_corry = 0.0f;
    mat_valid = false;
}

PreviewCamera::PreviewCamera(bool is_3d_)
: posX(0.0f), posY(0.0f), posZ(0.0f), angleX(20.0f), angleY(5.0f), distance(-2000.0f), zoom(1.0f),
  panx(0.0f), pany(0.0f), panz(0.0f), zoom_corrx(0.0f), zoom_corry(0.0f), is_3d(is_3d_),
  name("Name Unspecified"), menu_id(wxNewId()), deletemenu_id(wxNewId()), mat_valid(false)
{
}

PreviewCamera::~PreviewCamera()
{
}

// Copy constructor
PreviewCamera::PreviewCamera(const PreviewCamera &cam)
: posX(cam.posX), posY(cam.posY), posZ(cam.posZ), angleX(cam.angleX), angleY(cam.angleY), distance(cam.distance), zoom(cam.zoom),
  panx(cam.panx), pany(cam.pany), panz(cam.panz), zoom_corrx(cam.zoom_corrx), zoom_corry(cam.zoom_corry), is_3d(cam.is_3d),
  name(cam.name), menu_id(wxNewId()), deletemenu_id(wxNewId()), mat_valid(false)
{
}

PreviewCamera& PreviewCamera::operator= (const PreviewCamera& rhs)
{
    if (this == &rhs) return *this;   // Gracefully handle self assignment
    posX = rhs.posX;
    posY = rhs.posY;
    posZ = rhs.posZ;
    angleX = rhs.angleX;
    angleY = rhs.angleY;
    distance = rhs.distance;
    zoom = rhs.zoom;
    panx = rhs.panx;
    pany = rhs.pany;
    panz = rhs.panz;
    zoom_corrx = rhs.zoom_corrx;
    zoom_corry = rhs.zoom_corry;
    is_3d = rhs.is_3d;
    mat_valid = false;
    return *this;
}

glm::mat4& PreviewCamera::GetViewMatrix()
{
    if (!mat_valid) {
        glm::mat4 ViewTranslatePan = glm::translate(glm::mat4(1.0f), glm::vec3(posX + panx, posY + pany, posZ + panz));
        glm::mat4 ViewTranslateDistance = glm::translate(glm::mat4(1.0f), glm::vec3(1.0f, 1.0f, distance * zoom));
        glm::mat4 ViewRotateX = glm::rotate(glm::mat4(1.0f), glm::radians(angleX), glm::vec3(1.0f, 0.0f, 0.0f));
        glm::mat4 ViewRotateY = glm::rotate(glm::mat4(1.0f), glm::radians(angleY), glm::vec3(0.0f, 1.0f, 0.0f));
        view_matrix = ViewTranslateDistance * ViewRotateX * ViewRotateY * ViewTranslatePan;
        mat_valid = true;
    }
    return view_matrix;
}

void PreviewCamera::SetAngleX(float value)
{
    angleX = value;
    if (angleX < 0.0f) {
        angleX += 360.0f;
    }
    if (angleX >= 360.0f) {
            angleX -= 360.0f;
    }
    mat_valid = false;
}

void PreviewCamera::SetAngleY(float value)
{
    angleY = value;
    if (angleY < 0.0f) {
        angleY += 360.0f;
    }
    if (angleY >= 360.0f) {
            angleY -= 360.0f;
    }
    mat_valid = false;
}

ViewpointMgr::ViewpointMgr()
{
    //ctor
}

ViewpointMgr::~ViewpointMgr()
{
    Clear();
}

void ViewpointMgr::DeleteCamera3D(int i)
{
    if (previewCameras3d.size() <= i) return;
    auto todelete = GetCamera3D(i);
    auto it = previewCameras3d.begin();
    std::advance(it, i);
    previewCameras3d.erase(it);
    delete todelete;
}

void ViewpointMgr::DeleteCamera2D(int i)
{
    if (previewCameras2d.size() <= i) return;
    auto todelete = GetCamera2D(i);
    auto it = previewCameras2d.begin();
    std::advance(it, i);
    previewCameras2d.erase(it);
    delete todelete;
}

PreviewCamera* ViewpointMgr::GetNamedCamera3D(const std::string& name)
{
    PreviewCamera* camera = nullptr;
    for (size_t i = 0; i < previewCameras3d.size(); ++i)
    {
        if (previewCameras3d[i]->name == name) {
            camera = previewCameras3d[i];
            break;
        }
    }
    return camera;
}

bool ViewpointMgr::IsNameUnique(const std::string& name, bool is_3d)
{
    if (is_3d) {
        for (size_t i = 0; i < previewCameras3d.size(); ++i)
        {
            if (previewCameras3d[i]->name == name) {
                return false;
            }
        }
    }
    else {
        for (size_t i = 0; i < previewCameras2d.size(); ++i)
        {
            if (previewCameras2d[i]->name == name) {
                return false;
            }
        }
    }
    return true;
}

void ViewpointMgr::Clear()
{
    for (auto x : previewCameras2d)
    {
        delete x;
    }
    for (auto x : previewCameras3d)
    {
        delete x;
    }
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
	    wxXmlNode* cnode = new wxXmlNode(wxXML_ELEMENT_NODE, "Camera");
	    cnode->AddAttribute("name", XmlSafe(previewCameras2d[i]->name));
	    cnode->AddAttribute("posX", wxString::Format("%f", previewCameras2d[i]->posX));
	    cnode->AddAttribute("posY", wxString::Format("%f", previewCameras2d[i]->posY));
	    cnode->AddAttribute("posZ", wxString::Format("%f", previewCameras2d[i]->posZ));
	    cnode->AddAttribute("angleX", wxString::Format("%f", previewCameras2d[i]->angleX));
	    cnode->AddAttribute("angleY", wxString::Format("%f", previewCameras2d[i]->angleY));
	    cnode->AddAttribute("distance", wxString::Format("%f", previewCameras2d[i]->distance));
	    cnode->AddAttribute("zoom", wxString::Format("%f", previewCameras2d[i]->zoom));
	    cnode->AddAttribute("panx", wxString::Format("%f", previewCameras2d[i]->panx));
	    cnode->AddAttribute("pany", wxString::Format("%f", previewCameras2d[i]->pany));
	    cnode->AddAttribute("panz", wxString::Format("%f", previewCameras2d[i]->panz));
	    cnode->AddAttribute("zoom_corrx", wxString::Format("%f", previewCameras2d[i]->zoom_corrx));
	    cnode->AddAttribute("zoom_corry", wxString::Format("%f", previewCameras2d[i]->zoom_corry));
	    cnode->AddAttribute("is_3d", wxString::Format("%d", previewCameras2d[i]->is_3d));
        node->AddChild(cnode);
	}

	for( size_t i = 0; i < previewCameras3d.size(); ++i )
	{
	    wxXmlNode* cnode = new wxXmlNode(wxXML_ELEMENT_NODE, "Camera");
	    cnode->AddAttribute("name", XmlSafe(previewCameras3d[i]->name));
	    cnode->AddAttribute("posX", wxString::Format("%f", previewCameras3d[i]->posX));
	    cnode->AddAttribute("posY", wxString::Format("%f", previewCameras3d[i]->posY));
	    cnode->AddAttribute("posZ", wxString::Format("%f", previewCameras3d[i]->posZ));
	    cnode->AddAttribute("angleX", wxString::Format("%f", previewCameras3d[i]->angleX));
	    cnode->AddAttribute("angleY", wxString::Format("%f", previewCameras3d[i]->angleY));
	    cnode->AddAttribute("distance", wxString::Format("%f", previewCameras3d[i]->distance));
	    cnode->AddAttribute("zoom", wxString::Format("%f", previewCameras3d[i]->zoom));
	    cnode->AddAttribute("panx", wxString::Format("%f", previewCameras3d[i]->panx));
	    cnode->AddAttribute("pany", wxString::Format("%f", previewCameras3d[i]->pany));
	    cnode->AddAttribute("panz", wxString::Format("%f", previewCameras3d[i]->panz));
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
            std::string name = UnXmlSafe(c->GetAttribute("name", ""));
            if (name == "")
            {
                // This is so Gils early file still loads correctly
                name = c->GetName().ToStdString();
            }
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
            c->GetAttribute("posZ", &attr);
            new_camera->posZ = wxAtof(attr);
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
            c->GetAttribute("panz", &attr);
            new_camera->panz = wxAtof(attr);
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
