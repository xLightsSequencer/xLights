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

#include "DmxModel.h"
#include "DmxImage.h"
#include "../../utils/xlImage.h"
#include "UtilFunctions.h"
#include "../../ExternalHooks.h"
#include "../../ModelPreview.h"

#include <log.h>

#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>

DmxImage::DmxImage(std::string _name)
 : base_name(_name)
{
}

DmxImage::~DmxImage()
{
    for (auto it = _images.begin(); it != _images.end(); ++it) {
        delete it->second;
    }
}

void DmxImage::Init(BaseObject* base)
{
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
}

void DmxImage::Draw(BaseObject* base, ModelPreview* preview, xlGraphicsProgram *pg,
                    glm::mat4 &motion_matrix,
                    int transparency, float brightness, bool only_image,
                    float pivot_offset_x, float pivot_offset_y, bool rotation, bool use_pivot)
{
    bool exists = false;
    

    if (_images.find(preview->GetName().ToStdString()) == _images.end()) {
        if (FileExists(_imageFile)) {
            spdlog::debug("Loading image model {} file {} for preview {}.",
                (const char*)base->GetName().c_str(),
                (const char*)_imageFile.c_str(),
                (const char*)preview->GetName().c_str());
            xlImage img;
            if (img.LoadFromFile(_imageFile)) {
                xlTexture *t = preview->getCurrentGraphicsContext()->createTexture(img, _imageFile, true);
                _images[preview->GetName().ToStdString()] = t;
                width = img.GetWidth();
                height = img.GetHeight();
                exists = true;
                obj_exists = true;
            }
        }
    } else {
        exists = true;
    }


    if (exists) {
        xlTexture* image = _images[preview->GetName().ToStdString()];

        glm::mat4 Identity = glm::mat4(1.0f);
        glm::mat4 scalingMatrix = glm::scale(Identity, glm::vec3(scalex, scaley, scalez));
        glm::mat4 rx = glm::rotate(Identity, glm::radians(rotatex), glm::vec3(1.0f, 0.0f, 0.0f));
        glm::mat4 ry = glm::rotate(Identity, glm::radians(rotatey), glm::vec3(0.0f, 1.0f, 0.0f));
        glm::mat4 rz = glm::rotate(Identity, glm::radians(rotatez), glm::vec3(0.0f, 0.0f, 1.0f));
        glm::quat rotate_quat = glm::quat_cast(rx * ry * rz);
        glm::mat4 translationMatrix = glm::translate(Identity, glm::vec3(offset_x, offset_y, offset_z));
        glm::mat4 m = translationMatrix * glm::toMat4(rotate_quat) * scalingMatrix;
        if (rotation) {
            glm::mat4 pivotToZero = glm::translate(Identity, glm::vec3(-pivot_offset_x, -pivot_offset_y, 0.0f));
            glm::mat4 pivotBack = glm::translate(Identity, glm::vec3(pivot_offset_x, pivot_offset_y, 0.0f));
            m = m * pivotBack * motion_matrix * pivotToZero;
        } else {
            m = m * motion_matrix;
        }
        
        auto vac = pg->getAccumulator();
        int start = vac->getCount();
        if (use_pivot && rotation) {
            glm::vec3 scale = base->GetBaseObjectScreenLocation().GetScaleMatrix();
            float mw = std::min(4.0f / scale.x, 4.0f / scale.y);
            float mw2 = std::min(3.0f / scale.x, 3.0f / scale.y);

            
            xlColor pink = xlColor(255, 0, 255);
            //stack z coordinates
            vac->AddCircleAsTriangles(0, 0, 0.001f, mw, xlBLACK);
            vac->AddCircleAsTriangles(0, 0, 0.002f, mw2, pink);
            vac->AddVertex(0.0-mw/2, 0.0-mw/2, 0.003f, xlBLACK);
            vac->AddVertex(0.0+mw/2, 0.0+mw/2, 0.003f, xlBLACK);
            vac->AddVertex(0.0-mw/2, 0.0+mw/2, 0.003f, xlBLACK);
            vac->AddVertex(0.0+mw/2, 0.0-mw/2, 0.003f, xlBLACK);
        }

        int end = vac->getCount();
        int alpha = (100.0 - transparency) * 255.0 / 100.0;
        pg->addStep([=](xlGraphicsContext *ctx) {
            ctx->PushMatrix();
            ctx->ApplyMatrix(m);
            ctx->drawTexture(image,
                             -0.5, 0.5, 0.5, -0.5,
                             0.0, 0.0, 1.0, 1.0,
                             true, 100, alpha);
            if (end != start) {
                if (rotation) {
                    ctx->Translate(pivot_offset_x, pivot_offset_y, 0.0f);
                }
                ctx->drawTriangles(vac, start, end - 4 - start);
                ctx->drawLines(vac, end - 4, 4);
            }
            ctx->PopMatrix();
        });

    } else if (only_image) {
        DmxModel::DrawInvalid(pg, nullptr, false, false);
    }
}
