/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include <algorithm>
#include <filesystem>

#include "DmxModel.h"
#include "Mesh.h"
#include "../../UtilFunctions.h"
#include "../../ExternalHooks.h"
#include "../../ModelPreview.h"
#include "../../xLightsMain.h"
#include <log4cpp/Category.hh>

#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>

#include "../../graphics/tiny_obj_loader.h"
#include "../../graphics/xlMesh.h"
#include "../../graphics/xlGraphicsContext.h"

Mesh::Mesh(const std::string _name)
: base_name(_name)
{
}

Mesh::~Mesh()
{
}

void Mesh::SetRenderScaling(float s) {
    rscale = s;
}

void Mesh::Init(BaseObject* base, bool set_size) {

    if (brightness > 100) {
        brightness = 100;
    } else if (brightness < 0) {
        brightness = 0;
    }

    if (scalex < 0) {
        scalex = 1.0f;
    }
    if (scaley < 0) {
        scaley = 1.0f;
    }
    if (scalez < 0) {
        scalez = 1.0f;
    }

    if (rotatex < -180.0f || rotatex > 180.0f) {
        rotatex = 0.0f;
    }
    if (rotatey < -180.0f || rotatey > 180.0f) {
        rotatey = 0.0f;
    }
    if (rotatez < -180.0f || rotatez > 180.0f) {
        rotatez = 0.0f;
    }

    controls_size = set_size;
    if (controls_size) {
        width = render_width;
        height = render_height;
        depth = render_depth;
        base->GetBaseObjectScreenLocation().SetRenderSize(width, height, depth);
    }
}

void Mesh::loadObject(BaseObject* base, xlGraphicsContext *ctx) {
    if (FileExists(_objFile)) {
        static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));
        obj_exists = true;
                
        logger_base.debug("Loading mesh model file %s.",
                          (const char *)_objFile.c_str());
        mesh3d = ctx->loadMeshFromObjFile(_objFile);
        width = mesh3d->GetXMax() - mesh3d->GetXMin();
        height = mesh3d->GetYMax() - mesh3d->GetYMin();
        depth = mesh3d->GetZMax() - mesh3d->GetZMin();
        obj_loaded = true;
        
        std::string name = base->GetName() + ":" + base_name;
        mesh3d->SetName(name);
    }
}
bool Mesh::GetExists(BaseObject* base, xlGraphicsContext *ctx) {
    if (!obj_loaded) {
        loadObject(base, ctx);
        base->GetBaseObjectScreenLocation().SetSupportsZScaling(true);  // set here instead of constructor so model creation doesn't go up like a tree
    }
    return mesh3d != nullptr;
}

void Mesh::Draw(BaseObject* base, ModelPreview* preview, xlGraphicsProgram *sprogram, xlGraphicsProgram *tprogram, glm::mat4& base_matrix, glm::mat4& motion_matrix, bool show_empty, float pivot_offset_x, float pivot_offset_y, float pivot_offset_z, bool rotation, bool use_pivot)
{
    Draw(base, preview, sprogram, tprogram, base_matrix, motion_matrix, 0, 0, 0, show_empty, pivot_offset_x, pivot_offset_y, pivot_offset_z, rotation, use_pivot);
}

void Mesh::Draw(BaseObject* base, ModelPreview* preview, xlGraphicsProgram *sprogram, xlGraphicsProgram *tprogram, glm::mat4& base_matrix, glm::mat4& trans_matrix, float xrot, float yrot, float zrot, bool show_empty, float pivot_offset_x, float pivot_offset_y, float pivot_offset_z, bool rotation, bool use_pivot)
{
    if (!obj_loaded) {
        loadObject(base, preview->getCurrentGraphicsContext());
        base->GetBaseObjectScreenLocation().SetSupportsZScaling(true);  // set here instead of constructor so model creation doesn't go up like a tree
    }

    if (obj_loaded) {
        glm::mat4 Identity = glm::mat4(1.0f);
        glm::mat4 scalingMatrix = glm::scale(Identity, glm::vec3(scalex * rscale, scaley * rscale, scalez * rscale));
        glm::mat4 rx = glm::rotate(Identity, glm::radians(rotatex + xrot), glm::vec3(1.0f, 0.0f, 0.0f));
        glm::mat4 ry = glm::rotate(Identity, glm::radians(rotatey + yrot), glm::vec3(0.0f, 1.0f, 0.0f));
        glm::mat4 rz = glm::rotate(Identity, glm::radians(rotatez + zrot), glm::vec3(0.0f, 0.0f, 1.0f));
        glm::mat4 rotationMatrix = rz * ry * rx;
        glm::mat4 translationMatrix = glm::translate(Identity, glm::vec3(offset_x, offset_y, offset_z));
        glm::mat4 m;
        if (rotation) {
            glm::mat4 pivotToZero = glm::translate(Identity, glm::vec3(-pivot_offset_x, -pivot_offset_y, -pivot_offset_z));
            glm::mat4 pivotBack = glm::translate(Identity, glm::vec3(pivot_offset_x, pivot_offset_y, pivot_offset_z));
            m = base_matrix * translationMatrix * pivotBack * trans_matrix * pivotToZero * rotationMatrix * scalingMatrix;
        } else {
            m = base_matrix * translationMatrix * trans_matrix * rotationMatrix * scalingMatrix;
        }

        if (controls_size) {
            if (recalc_size) {
                base->GetBaseObjectScreenLocation().AdjustRenderSize(width * scalex, height * scaley * half_height, depth * scalez);
                render_width = width;
                render_height = height;
                render_depth = depth;
            }
            base->GetBaseObjectScreenLocation().UpdateBoundingBox(width, height, depth);  // FIXME: Modify to only call this when position changes
        }

        // skip updating color if it hasn't changed.
        if (update_color) {
            if (new_color != last_color) {
                last_color = new_color;
            } else {
                update_color = false;
            }
        }
        if (update_color) {
            mesh3d->SetMaterialColor(color_name, &new_color);
        }

        auto vac = sprogram->getAccumulator();
        int start = vac->getCount();
        if (use_pivot && rotation) {
            /*
            glm::vec3 scale = base->GetBaseObjectScreenLocation().GetScaleMatrix();
            float mw = std::min(std::min(4.0f / scale.x, 4.0f / scale.y);
            float mw2 = std::min(std::min(3.0f / scale.x, 3.0f / scale.y);

            
            xlColor pink = xlColor(255, 0, 255);
            vac->AddCircleAsTriangles(0, 0, 0, mw, xlBLACK);
            vac->AddCircleAsTriangles(0, 0, 0, mw2, pink);
            vac->AddVertex(0.0-mw/2, 0.0-mw/2, 0, xlBLACK);
            vac->AddVertex(0.0+mw/2, 0.0+mw/2, 0, xlBLACK);
            vac->AddVertex(0.0-mw/2, 0.0+mw/2, 0, xlBLACK);
            vac->AddVertex(0.0+mw/2, 0.0-mw/2, 0, xlBLACK);
             
             
             
             glm::vec3 scale = base->GetBaseObjectScreenLocation().GetScaleMatrix();
             float x1 = pivot_offset_x;
             float y1 = pivot_offset_y;
             float z1 = pivot_offset_z;
             float offx = width * scalex * scale.x * rscale;
             float offz = depth * scalez * scale.z * rscale;
             glm::vec4 v = base_matrix * translationMatrix * glm::vec4(glm::vec3(x1, y1, z1), 1.0f);
             x1 = v.x; y1 = v.y; z1 = v.z;
             va.AddVertex(x1 - offx, y1, z1, pink);
             va.AddVertex(x1 + offx, y1, z1, pink);
             va.AddVertex(x1, y1, z1 - offz, pink);
             va.AddVertex(x1, y1, z1 + offz, pink);
             va.Finish(GL_LINES);
            */
        }

        int end = vac->getCount();
        sprogram->addStep([=, this](xlGraphicsContext *ctx) {
            ctx->PushMatrix();
            ctx->ApplyMatrix(m);
            if (mesh_only) {
                ctx->drawMeshWireframe(mesh3d.get(), this->brightness);
            } else {
                ctx->drawMeshSolids(mesh3d.get(), this->brightness, preview->Is3D());
            }
            if (end != start) {
                if (rotation) {
                    ctx->Translate(pivot_offset_x, pivot_offset_y, 0.0f);
                }
                ctx->drawTriangles(vac, start, end - 4 - start);
                ctx->drawLines(vac, end - 4, 4);
            }
            ctx->PopMatrix();
        });
        if (!mesh_only) {
            tprogram->addStep([=, this](xlGraphicsContext *ctx) {
                ctx->PushMatrix();
                ctx->ApplyMatrix(m);
                ctx->drawMeshTransparents(mesh3d.get(), this->brightness);
                if (end != start) {
                    if (rotation) {
                        ctx->Translate(pivot_offset_x, pivot_offset_y, 0.0f);
                    }
                    ctx->drawTriangles(vac, start, end - 4 - start);
                    ctx->drawLines(vac, end - 4, 4);
                }
                ctx->PopMatrix();
            });
        }
    } else if( show_empty ) {
        DmxModel::DrawInvalid(sprogram, nullptr, false, false);
    }
}
