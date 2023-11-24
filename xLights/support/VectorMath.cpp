/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/smeighan/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/smeighan/xLights/blob/master/License.txt
 **************************************************************/

#include "VectorMath.h"
#ifdef _MSC_VER
// required so M_PI will be defined by MSC
#define _USE_MATH_DEFINES
#include <math.h>
#endif

void VectorMath::ScreenPosToWorldRay(
    int mouseX, int mouseY,             // Mouse position, in pixels, from bottom-left corner of the window
    int screenWidth, int screenHeight,  // Window size, in pixels
    const glm::mat4 &ProjViewMatrix,           // Projection / View matrix
    glm::vec3& out_origin,              // Ouput : Origin of the ray. /!\ Starts at the near plane, so if you want the ray to start at the camera's position instead, ignore this.
    glm::vec3& out_direction            // Ouput : Direction, in world space, of the ray that goes "through" the mouse.
) {

    // The ray Start and End positions, in Normalized Device Coordinates (Have you read Tutorial 4 ?)
    glm::vec4 lRayStart_NDC(
        ((float)mouseX / (float)screenWidth - 0.5f) * 2.0f, // [0,1024] -> [-1,1]
        ((float)mouseY / (float)screenHeight - 0.5f) * 2.0f, // [0, 768] -> [-1,1]
        -1.0, // The near plane maps to Z=-1 in Normalized Device Coordinates
        1.0f
    );
    glm::vec4 lRayEnd_NDC(
        ((float)mouseX / (float)screenWidth - 0.5f) * 2.0f,
        ((float)mouseY / (float)screenHeight - 0.5f) * 2.0f,
        0.0,
        1.0f
    );

    glm::mat4 M = glm::inverse(ProjViewMatrix);
    glm::vec4 lRayStart_world = M * lRayStart_NDC; lRayStart_world /= lRayStart_world.w;
    glm::vec4 lRayEnd_world = M * lRayEnd_NDC; lRayEnd_world /= lRayEnd_world.w;

    glm::vec3 lRayDir_world(lRayEnd_world - lRayStart_world);
    lRayDir_world = glm::normalize(lRayDir_world);

    out_origin = glm::vec3(lRayStart_world);
    out_direction = glm::normalize(lRayDir_world);
}

bool VectorMath::TestRayOBBIntersection(
    glm::vec3 ray_origin,        // Ray origin, in world space
    glm::vec3 ray_direction,     // Ray direction (NOT target position!), in world space. Must be normalize()'d.
    glm::vec3 aabb_min,          // Minimum X,Y,Z coords of the mesh when not transformed at all.
    glm::vec3 aabb_max,          // Maximum X,Y,Z coords. Often aabb_min*-1 if your mesh is centered, but it's not always the case.
    glm::mat4 ModelMatrix,       // Transformation applied to the mesh (which will thus be also applied to its bounding box)
    float& intersection_distance // Output : distance between ray_origin and the intersection with the OBB
) {
    // Intersection method from Real-Time Rendering and Essential Mathematics for Games

    float tMin = 0.0f;
    float tMax = 100000.0f;

    glm::vec3 OBBposition_worldspace(ModelMatrix[3].x, ModelMatrix[3].y, ModelMatrix[3].z);

    glm::vec3 delta = OBBposition_worldspace - ray_origin;

    // Test intersection with the 2 planes perpendicular to the OBB's X axis
    {
        glm::vec3 xaxis(ModelMatrix[0].x, ModelMatrix[0].y, ModelMatrix[0].z);
        float e = glm::dot(xaxis, delta);
        float f = glm::dot(ray_direction, xaxis);

        if (fabs(f) > 0.001f) { // Standard case

            float t1 = (e + aabb_min.x) / f; // Intersection with the "left" plane
            float t2 = (e + aabb_max.x) / f; // Intersection with the "right" plane
                                             // t1 and t2 now contain distances betwen ray origin and ray-plane intersections

                                             // We want t1 to represent the nearest intersection, 
                                             // so if it's not the case, invert t1 and t2
            if (t1 > t2) {
                float w = t1; t1 = t2; t2 = w; // swap t1 and t2
            }

            // tMax is the nearest "far" intersection (amongst the X,Y and Z planes pairs)
            if (t2 < tMax)
                tMax = t2;
            // tMin is the farthest "near" intersection (amongst the X,Y and Z planes pairs)
            if (t1 > tMin)
                tMin = t1;

            // And here's the trick :
            // If "far" is closer than "near", then there is NO intersection.
            // See the images in the tutorials for the visual explanation.
            if (tMax < tMin)
                return false;

        }
        else { // Rare case : the ray is almost parallel to the planes, so they don't have any "intersection"
            if (e + aabb_min.x > 0.0f || e + aabb_max.x < 0.0f)
                return false;
        }
    }


    // Test intersection with the 2 planes perpendicular to the OBB's Y axis
    // Exactly the same thing than above.
    {
        glm::vec3 yaxis(ModelMatrix[1].x, ModelMatrix[1].y, ModelMatrix[1].z);
        float e = glm::dot(yaxis, delta);
        float f = glm::dot(ray_direction, yaxis);

        if (fabs(f) > 0.001f) {

            float t1 = (e + aabb_min.y) / f;
            float t2 = (e + aabb_max.y) / f;

            if (t1 > t2) { float w = t1; t1 = t2; t2 = w; }

            if (t2 < tMax)
                tMax = t2;
            if (t1 > tMin)
                tMin = t1;
            if (tMin > tMax)
                return false;

        }
        else {
            if (e + aabb_min.y > 0.0f || e + aabb_max.y < 0.0f)
                return false;
        }
    }


    // Test intersection with the 2 planes perpendicular to the OBB's Z axis
    // Exactly the same thing than above.
    {
        glm::vec3 zaxis(ModelMatrix[2].x, ModelMatrix[2].y, ModelMatrix[2].z);
        float e = glm::dot(zaxis, delta);
        float f = glm::dot(ray_direction, zaxis);

        if (fabs(f) > 0.001f) {

            float t1 = (e + aabb_min.z) / f;
            float t2 = (e + aabb_max.z) / f;

            if (t1 > t2) { float w = t1; t1 = t2; t2 = w; }

            if (t2 < tMax)
                tMax = t2;
            if (t1 > tMin)
                tMin = t1;
            if (tMin > tMax)
                return false;

        }
        else {
            if (e + aabb_min.z > 0.0f || e + aabb_max.z < 0.0f)
                return false;
        }
    }

    intersection_distance = tMin;
    return true;
}

bool VectorMath::TestRayOBBIntersection2D(
    glm::vec3 ray_origin,        // Ray origin, in world space
    glm::vec3 aabb_min,          // Minimum X,Y,Z coords of the mesh when not transformed at all.
    glm::vec3 aabb_max,          // Maximum X,Y,Z coords. Often aabb_min*-1 if your mesh is centered, but it's not always the case.
    glm::mat4 ModelMatrix        // Transformation applied to the mesh (which will thus be also applied to its bounding box)
) {
    glm::vec3 OBBposition_worldspace(ModelMatrix[3].x, ModelMatrix[3].y, 0.0f);
    glm::vec3 delta = OBBposition_worldspace - ray_origin;

    // Test intersection with the OBB's X axis
    glm::vec3 xaxis(ModelMatrix[0].x, ModelMatrix[0].y, ModelMatrix[0].z);
    float ex = glm::dot(xaxis, delta);

    if (ex + aabb_min.x > 0.0f || ex + aabb_max.x < 0.0f)
        return false;

    // Test intersection with the OBB's Y axis
    glm::vec3 yaxis(ModelMatrix[1].x, ModelMatrix[1].y, ModelMatrix[1].z);
    float ey = glm::dot(yaxis, delta);

    if (ey + aabb_min.y > 0.0f || ey + aabb_max.y < 0.0f)
        return false;

    return true;
}

bool VectorMath::TestVolumeOBBIntersection(
    int mouseX1, int mouseY1,    // Mouse position, in pixels, from bottom-left corner of the window
    int mouseX2, int mouseY2,    // Mouse position, in pixels, from bottom-left corner of the window
    int screenWidth, int screenHeight,  // Window size, in pixels
    glm::vec3 aabb_min,          // Minimum X,Y,Z coords of the mesh when not transformed at all.
    glm::vec3 aabb_max,          // Maximum X,Y,Z coords. Often aabb_min*-1 if your mesh is centered, but it's not always the case.
    glm::mat4 ProjViewMatrix,    // Projection / View matrix
    glm::mat4 ModelMatrix        // Transformation applied to the mesh (which will thus be also applied to its bounding box)
) {
    glm::mat4 MVP = ProjViewMatrix * ModelMatrix;
    glm::vec4 clipSpacePos = MVP * glm::vec4(aabb_min, 1.0);
    if (clipSpacePos.w == 0.0f) return false;
    glm::vec3 ndcSpacePos = glm::vec3(clipSpacePos.x / clipSpacePos.w, clipSpacePos.y / clipSpacePos.w, clipSpacePos.z / clipSpacePos.w);
    glm::vec2 min_pos(((ndcSpacePos.x + 1.0) / 2.0) * screenWidth, ((1.0 - ndcSpacePos.y) / 2.0) * screenHeight);
    clipSpacePos = MVP * glm::vec4(aabb_max, 1.0);
    if (clipSpacePos.w == 0.0f) return false;
    ndcSpacePos = glm::vec3(clipSpacePos.x / clipSpacePos.w, clipSpacePos.y / clipSpacePos.w, clipSpacePos.z / clipSpacePos.w);
    glm::vec2 max_pos(((ndcSpacePos.x + 1.0) / 2.0) * screenWidth, ((1.0 - ndcSpacePos.y) / 2.0) * screenHeight);

    return (min_pos.x >= mouseX1 && max_pos.x <= mouseX2 && min_pos.y >= mouseY1 && max_pos.y <= mouseY2);
}

glm::vec2 VectorMath::GetScreenCoord(
    int screenWidth, int screenHeight,  // Window size, in pixels
    glm::vec3 position,          // X,Y,Z coords of the position when not transformed at all.
    glm::mat4 ProjViewMatrix,    // Projection / View matrix
    glm::mat4 ModelMatrix        // Transformation applied to the position
) {
    glm::mat4 MVP = ProjViewMatrix * ModelMatrix;
    glm::vec4 clipSpacePos = MVP * glm::vec4(position, 1.0);
    if (clipSpacePos.w == 0.0f) return glm::vec2(0.0f);
    glm::vec3 ndcSpacePos = glm::vec3(clipSpacePos.x / clipSpacePos.w, clipSpacePos.y / clipSpacePos.w, clipSpacePos.z / clipSpacePos.w);
    glm::vec2 screen_position(((ndcSpacePos.x + 1.0) / 2.0) * screenWidth, ((1.0 - ndcSpacePos.y) / 2.0) * screenHeight);
    return screen_position;
}


///////////////////////////////////////////////////////////////////////////////
// find the intersect point
// substitute a point on the line to the plane equation, then solve for alpha
// a point on a line: (x0 + x*t, y0 + y*t, z0 + z*t)
// plane: a*X + b*Y + c*Z + d = 0
//
// a*(x0 + x*t) + b*(y0 + y*t) + c*(z0 + z*t) + d = 0
// a*x0 + a*x*t + b*y0 + b*y*t + c*z0 + c*z*t + d = 0
// (a*x + b*x + c*x)*t = -(a*x0 + b*y0 + c*z0 + d)
//
// t = -(a*x0 + b*y0 + c*z0 + d) / (a*x + b*x + c*x)
///////////////////////////////////////////////////////////////////////////////
bool VectorMath::GetPlaneIntersect(
    glm::vec3 p,                 // Point origin  (x0, y0, z0)
    glm::vec3 v,                 // Ray direction (x,  y,  z)
    glm::vec3 point,             // Point on the plane
    glm::vec3 normal,            // Normal to the plane
    glm::vec3& intersect         // Output: intersect point
) {
                                            // dot products
        float dot1 = glm::dot(normal,p);    // a*x0 + b*y0 + c*z0
        float dot2 = glm::dot(normal,v);    // a*x + b*y + c*z

                                            // if denominator=0, no intersect
        if (dot2 == 0)
            return false;

        float d = -glm::dot(normal,point);  // -(a*x0 + b*y0 + c*z0)

        // find t = -(a*x0 + b*y0 + c*z0 + d) / (a*x + b*y + c*z)
        float t = -(dot1 + d) / dot2;

        // find intersection point
        intersect = p + (v * t);

        return true;
    }

#include <glm/gtx/quaternion.hpp>

glm::quat VectorMath::rotationBetweenVectors(const glm::vec3 &start, const glm::vec3 &dest)
{
    // normalize the vectors
    glm::vec3 startn = glm::normalize(start);
    glm::vec3 destn = glm::normalize(dest);

    // the rotation axis would be the cross product between the two vectors
    glm::vec3 rotAxis;

    // dot product gives us the angle between the two vectors
    float cosTheta = glm::dot(startn, destn);

    if (cosTheta >= 1.0f)
    {
        // if this is the case, the two vectors were parallel
        return glm::quat(1.0f, 0.0f, 0.0f, 0.0f);
    }
    // if the angle is -1 or less (or really close to it), then
    // we can pick many directions to rotate since we are rotating
    // close to 180 degrees (pi)
    if (cosTheta < (1e-4f - 1.0f))
    {
        // so use the z-axis to find an axis of rotate instead
        rotAxis = glm::cross(glm::vec3(1.0f, 0.0f, 0.0f), startn);

        // if the length was close to zero, then
        if (glm::length2(rotAxis) < 1e-6f)
            rotAxis = glm::cross(glm::vec3(0.0f, 1.0f, 0.0f), startn);

        // now normalize it and ask for a 
        rotAxis = glm::normalize(rotAxis);
        return glm::quat(180.0f, rotAxis);
    }

    // otherwise, the angle was fine (not close to pi rotation), so
    // find the rotation axis which is the cross product
    rotAxis = glm::normalize(glm::cross(startn, destn));

    float s = sqrt((1 + cosTheta) * 2);
    float invs = 1 / s;

    // otherwise return the rotation
    return glm::quat(s *.5f, rotAxis.x * invs, rotAxis.y * invs, rotAxis.z*invs);
}

glm::mat4 VectorMath::rotMatrixFromXAxisToVector(const glm::vec3 &vector)
{
    glm::vec3 xaxis(1.0f, 0.0f, 0.0f);
    glm::quat rotQuat = rotationBetweenVectors(xaxis, vector);
    return glm::mat4_cast(rotQuat/glm::length(rotQuat));
}

#include <glm/gtx/rotate_vector.hpp>

glm::mat4 VectorMath::rotationMatrixFromXAxisToVector(const glm::vec3 &a)
{
    glm::vec3 v = glm::vec3(0, -a.z, a.y);
    float len = glm::length(a);
    float ax = a.x / len;
    float angle = acos(ax);
    if (ax > 0.9999f || angle == 0.0f) {
        // aligned with x-axis so return identity matrix
        return glm::mat4(1.0f);
    } else if (ax < -0.9999f || std::abs(angle - M_PI) < 0.0001f) {
        // aligned with x-axis in opposite direction
        return glm::rotate(glm::mat4(1.0f), glm::radians(180.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    }
    return glm::rotate(angle, v);
}

glm::mat4 VectorMath::rotationMatrixFromXAxisToVector2(const glm::vec3 &o, const glm::vec3 &p)
{
    glm::vec3 a = p - o;
    if (o.y != p.y || o.z != p.z) {
        glm::vec3 v = glm::vec3(0, -a.z, a.y);
        float len = glm::length(a);
        float angle = len == 0.0f ? 0.0f : acos(a.x / len);
        return glm::rotate(angle, v);
    } else {
        if (p.x < o.x) {
            return glm::rotate(glm::mat4(1.0f), (float)glm::radians(180.0f), glm::vec3(0.0f, 0.0f, 1.0f));
        } else {
            return glm::mat4(1.0f);
        }
    }
}

glm::mat4 VectorMath::rotationMatrixBetweenVectors(const glm::vec3 &a, const glm::vec3 &b)
{
    glm::vec3 v = glm::cross(b, a);
    float angle = acos(glm::dot(b, a) / (glm::length(b) * glm::length(a)));
    return glm::rotate(angle, v);
}
