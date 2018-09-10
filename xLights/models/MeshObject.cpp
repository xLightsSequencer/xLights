#include <wx/xml/xml.h>
#include <wx/propgrid/propgrid.h>
#include <wx/propgrid/advprops.h>
#include <algorithm>

#define TINYOBJLOADER_IMPLEMENTATION
#include "MeshObject.h"
#include "DrawGLUtils.h"
#include "UtilFunctions.h"
#include "ModelPreview.h"
#include <log4cpp/Category.hh>

MeshObject::MeshObject(wxXmlNode *node, const ViewObjectManager &manager)
 : ObjectWithScreenLocation(manager), _objFile(""),
    width(100), height(100), depth(100), transparency(0), obj_loaded(false)
{
    SetFromXml(node);
    screenLocation.SetSupportsZScaling(true);
}

MeshObject::~MeshObject()
{
}

void MeshObject::InitModel() {
	_objFile = FixFile("", ModelXml->GetAttribute("ObjFile", ""));

    if (ModelXml->HasAttribute("Transparency")) {
        transparency = wxAtoi(ModelXml->GetAttribute("Transparency"));
    }

    screenLocation.SetRenderSize(width, height, depth);
}

void MeshObject::AddTypeProperties(wxPropertyGridInterface *grid) {
	wxPGProperty *p = grid->Append(new wxFileProperty("ObjFile",
                                             "ObjFile",
                                             _objFile));
    p->SetAttribute(wxPG_FILE_WILDCARD, "Wavefront files|*.obj|All files (*.*)|*.*");

    p = grid->Append(new wxUIntProperty("Transparency", "Transparency", transparency));
    p->SetAttribute("Min", 0);
    p->SetAttribute("Max", 100);
    p->SetEditor("SpinCtrl");
}

int MeshObject::OnPropertyGridChange(wxPropertyGridInterface *grid, wxPropertyGridEvent& event) {
    if ("ObjFile" == event.GetPropertyName()) {
        obj_loaded = false;
        _objFile = event.GetValue().GetString();
        ModelXml->DeleteAttribute("ObjFile");
        ModelXml->AddAttribute("ObjFile", _objFile);
        SetFromXml(ModelXml);
        return 3;
    }
    else if ("Transparency" == event.GetPropertyName()) {
        transparency = (int)event.GetPropertyValue().GetLong();
        ModelXml->DeleteAttribute("Transparency");
        ModelXml->AddAttribute("Transparency", wxString::Format("%d", transparency));
        return 3 | 0x0008;
    }

    return ViewObject::OnPropertyGridChange(grid, event);
}

void MeshObject::Draw(ModelPreview* preview, DrawGLUtils::xl3Accumulator &va3, bool allowSelected)
{
    if( !active ) { return; }

    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));

    GetObjectScreenLocation().PrepareToDraw(true, allowSelected);

    if (!obj_loaded) {
        if (wxFileExists(_objFile)) {
            logger_base.debug("Loading mesh model %s file %s for preview %s.",
                (const char *)GetName().c_str(),
                (const char *)_objFile.c_str(),
                (const char *)preview->GetName().c_str());
            std::string err;
            bool ret = tinyobj::LoadObj(&attrib, &shapes, &materials, &err, (char *)_objFile.c_str());

            bmin[0] = bmin[1] = bmin[2] = std::numeric_limits<float>::max();
            bmax[0] = bmax[1] = bmax[2] = -std::numeric_limits<float>::max();

            for (size_t s = 0; s < shapes.size(); s++) {
                // Loop over faces(polygon)
                size_t index_offset = 0;

                for (size_t f = 0; f < shapes[s].mesh.indices.size() / 3; f++) {
                    tinyobj::index_t idx0 = shapes[s].mesh.indices[3 * f + 0];
                    tinyobj::index_t idx1 = shapes[s].mesh.indices[3 * f + 1];
                    tinyobj::index_t idx2 = shapes[s].mesh.indices[3 * f + 2];

                    float v[3][3];
                    for (int k = 0; k < 3; k++) {
                        int f0 = idx0.vertex_index;
                        int f1 = idx1.vertex_index;
                        int f2 = idx2.vertex_index;
                        assert(f0 >= 0);
                        assert(f1 >= 0);
                        assert(f2 >= 0);

                        v[0][k] = attrib.vertices[3 * f0 + k];
                        v[1][k] = attrib.vertices[3 * f1 + k];
                        v[2][k] = attrib.vertices[3 * f2 + k];
                        bmin[k] = std::min(v[0][k], bmin[k]);
                        bmin[k] = std::min(v[1][k], bmin[k]);
                        bmin[k] = std::min(v[2][k], bmin[k]);
                        bmax[k] = std::max(v[0][k], bmax[k]);
                        bmax[k] = std::max(v[1][k], bmax[k]);
                        bmax[k] = std::max(v[2][k], bmax[k]);
                    }
                }
            }
            width = std::max(std::abs(bmin[0]), bmax[0]) * 2.0f;
            height = std::max(std::abs(bmin[1]), bmax[1]) * 2.0f;
            depth = std::max(std::abs(bmin[2]), bmax[2]) * 2.0f;
            screenLocation.SetRenderSize(width, height, depth);
            obj_loaded = true;
        }
    }

    GetObjectScreenLocation().UpdateBoundingBox(width, height);  // FIXME: Modify to only call this when position changes

    if (obj_loaded) {
        // Loop over shapes
        for (size_t s = 0; s < shapes.size(); s++) {
            // Loop over faces(polygon)
            size_t index_offset = 0;

            for (size_t f = 0; f < shapes[s].mesh.indices.size() / 3; f++) {
                tinyobj::index_t idx0 = shapes[s].mesh.indices[3 * f + 0];
                tinyobj::index_t idx1 = shapes[s].mesh.indices[3 * f + 1];
                tinyobj::index_t idx2 = shapes[s].mesh.indices[3 * f + 2];

                float v[3][3];
                for (int k = 0; k < 3; k++) {
                    int f0 = idx0.vertex_index;
                    int f1 = idx1.vertex_index;
                    int f2 = idx2.vertex_index;

                    v[0][k] = attrib.vertices[3 * f0 + k];
                    v[1][k] = attrib.vertices[3 * f1 + k];
                    v[2][k] = attrib.vertices[3 * f2 + k];
                    bmin[k] = std::min(v[0][k], bmin[k]);
                    bmin[k] = std::min(v[1][k], bmin[k]);
                    bmin[k] = std::min(v[2][k], bmin[k]);
                    bmax[k] = std::max(v[0][k], bmax[k]);
                    bmax[k] = std::max(v[1][k], bmax[k]);
                    bmax[k] = std::max(v[2][k], bmax[k]);
                }

                // Mesh Lines
                float x0 = v[0][0];
                float y0 = v[0][1];
                float z0 = v[0][2];
                GetObjectScreenLocation().TranslatePoint(x0, y0, z0);
                float x1 = v[1][0];
                float y1 = v[1][1];
                float z1 = v[1][2];
                GetObjectScreenLocation().TranslatePoint(x1, y1, z1);
                float x2 = v[2][0];
                float y2 = v[2][1];
                float z2 = v[2][2];
                GetObjectScreenLocation().TranslatePoint(x2, y2, z2);

                va3.AddVertex(x0, y0, z0, *wxGREEN);
                va3.AddVertex(x1, y1, z1, *wxGREEN);
                va3.AddVertex(x1, y1, z1, *wxGREEN);
                va3.AddVertex(x2, y2, z2, *wxGREEN);
                va3.AddVertex(x2, y2, z2, *wxGREEN);
                va3.AddVertex(x0, y0, z0, *wxGREEN);

                /*
                for (int k = 0; k < 3; k++) {
                    GetObjectScreenLocation().TranslatePoint(v[k][0], v[k][1], v[k][2]);
                    va3.AddVertex(v[k][0], v[k][1], v[k][2], *wxGREEN);

                    //buffer.push_back(v[k][0]);
                    //buffer.push_back(v[k][1]);
                    //buffer.push_back(v[k][2]);
                    //buffer.push_back(n[k][0]);
                    //buffer.push_back(n[k][1]);
                    //buffer.push_back(n[k][2]);
                    // Combine normal and diffuse to get color.
                    float normal_factor = 0.2;
                    float diffuse_factor = 1 - normal_factor;
                    float c[3] = { n[k][0] * normal_factor + diffuse[0] * diffuse_factor,
                        n[k][1] * normal_factor + diffuse[1] * diffuse_factor,
                        n[k][2] * normal_factor + diffuse[2] * diffuse_factor };
                    float len2 = c[0] * c[0] + c[1] * c[1] + c[2] * c[2];
                    if (len2 > 0.0f) {
                        float len = sqrtf(len2);

                        c[0] /= len;
                        c[1] /= len;
                        c[2] /= len;
                    }
                    buffer.push_back(c[0] * 0.5 + 0.5);
                    buffer.push_back(c[1] * 0.5 + 0.5);
                    buffer.push_back(c[2] * 0.5 + 0.5);

                    buffer.push_back(tc[k][0]);
                    buffer.push_back(tc[k][1]);
                }*/
            }
        }
        //va3.Finish(GL_TRIANGLES, GL_LINE_SMOOTH, 1.0f);
        va3.Finish(GL_LINES, GL_LINE_SMOOTH, 1.0f);
    }

    if ((Selected || Highlighted) && allowSelected) {
        GetObjectScreenLocation().DrawHandles(va3);
    }
}
