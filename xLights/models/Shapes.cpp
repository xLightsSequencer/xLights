#include "Shapes.h"
#include "../DrawGLUtils.h"
#include "../support/VectorMath.h"
#include <glm/glm.hpp>
#include <glm/gtx/matrix_transform_2d.hpp>
#include <glm/mat3x3.hpp>
#include <glm/gtx/rotate_vector.hpp>

BezierCurve::BezierCurve()
: steps(25), old_steps(-1), width(1), height(1), num_points(0), ren_width(1.0f), matrix_valid(false)
{
}

BezierCurve::~BezierCurve()
{
}

BezierCurveCubic::BezierCurveCubic()
: BezierCurve()
{
}

BezierCurveCubic::~BezierCurveCubic()
{
    clear_points();
}

void BezierCurve::clear_points()
{
	for( int j = 0; j < points.size(); ++j ) {
        if( points[j].matrix != nullptr ) {
            delete points[j].matrix;
            points[j].matrix = nullptr;
        }
	}
    points.clear();
    num_points = 0;
    matrix_valid = false;
}

float BezierCurve::interpPt(float n1 , float n2 , float perc )
{
    float diff = n2 - n1;
    return n1 + ( diff * perc );
}

float BezierCurve::get_px(int num)
{
    if( num < num_points ) {
        return points[num].x;
    }
    return 0.0f;
}

float BezierCurve::get_py(int num)
{
    if( num < num_points ) {
        return points[num].y;
    }
    return 0.0f;
}

void BezierCurve::OffsetX(float diff)
{
    cp0.x += diff;
}

void BezierCurve::OffsetY(float diff)
{
    cp0.y += diff;
}

void BezierCurveCubic::OffsetX(float diff)
{
    cp0.x += diff;
    cp1.x += diff;
}

void BezierCurveCubic::OffsetY(float diff)
{
    cp0.y += diff;
    cp1.y += diff;
}

void BezierCurveCubic::UpdatePoints() {
	float x, y, xa, xb, xc, xd, xe, ya, yb, yc, yd, ye;

	if( p0.x  == old_p0.x  && p0.y  == old_p0.y  &&
        p1.x  == old_p1.x  && p1.y  == old_p1.y  &&
        cp0.x == old_cp0.x && cp0.y == old_cp0.y &&
        cp1.x == old_cp1.x && cp1.y == old_cp1.y &&
        steps == old_steps ) {
		return;
	}

	clear_points();
	num_points = 0;

	for( float i = 0.0f ; i < 1.0f ; i += 1.0f / (float)steps )
	{
	    // Control Line Segments
	    xa = interpPt( p0.x , cp0.x , i );
	    ya = interpPt( p0.y , cp0.y , i );
	    xb = interpPt( cp0.x , cp1.x , i );
	    yb = interpPt( cp0.y , cp1.y , i );
	    xc = interpPt( cp1.x , p1.x , i );
	    yc = interpPt( cp1.y , p1.y , i );

	    // Between Control Lines
	    xd = interpPt( xa , xb , i );
	    yd = interpPt( ya , yb , i );
	    xe = interpPt( xb , xc , i );
	    ye = interpPt( yb , yc , i );

	    // the curve points
	    x = interpPt( xd , xe , i );
	    y = interpPt( yd , ye , i );

        xlPointf new_point(x,y);
        new_point.matrix = nullptr;
	    points.push_back(new_point);
	    num_points++;
	}
	// add final point
    xlPointf new_point(p1.x,p1.y);
    new_point.matrix = nullptr;
    points.push_back(new_point);
    num_points++;

	old_p0.x = p0.x;
	old_p0.y = p0.y;
	old_p1.x = p1.x;
	old_p1.y = p1.y;
	old_cp0.x = cp0.x;
	old_cp0.y = cp0.y;
	old_cp1.x = cp1.x;
	old_cp1.y = cp1.y;
	old_steps = steps;
}

float BezierCurve::GetLength() {
    float total_length = 0.0f;
    if( !matrix_valid ) {
        UpdateMatrices();
    }
	for(int i = 0; i < num_points-1; ++i ) {
        total_length += points[i].length;
	}
	return total_length;
}

float BezierCurve::GetSegLength(int segment) {
    if( !matrix_valid ) {
        UpdateMatrices();
    }
    if( segment < num_points-1 ) {
        return points[segment].length;
    } else {
        return 0.0f;
    }
}

void BezierCurve::UpdateMatrices()
{
    for( int i = 0; i < num_points-1; ++i ) {
        float x1p = points[i].x * (float)width;
        float x2p = points[i+1].x * (float)width;
        float y1p = points[i].y * (float)height;
        float y2p = points[i+1].y * (float)height;

        float angle = (float)M_PI/2.0f;
        if (points[i+1].x != points[i].x) {
            float slope = (y2p - y1p)/(x2p - x1p);
            angle = std::atan(slope);
            if (points[i].x > points[i+1].x) {
                angle += (float)M_PI;
            }
        } else if (points[i+1].y < points[i].y) {
            angle += (float)M_PI;
        }
        float scale = std::sqrt((y2p - y1p)*(y2p - y1p) + (x2p - x1p)*(x2p - x1p));
        scale /= ren_width;
        points[i].length = scale;

        glm::mat3 scalingMatrix = glm::scale(glm::mat3(1.0f), glm::vec2(scale, 1.0f));
        glm::mat3 rotationMatrix = glm::rotate(glm::mat3(1.0f), (float)angle);
        glm::mat3 translateMatrix = glm::translate(glm::mat3(1.0f), glm::vec2(x1p, y1p));
        glm::mat3 mat3 = translateMatrix * rotationMatrix * scalingMatrix;

        if (points[i].matrix != nullptr) {
            delete points[i].matrix;
        }
        points[i].matrix = new glm::mat3(mat3);
    }
    matrix_valid = true;
}

void BezierCurve::CreateNormalizedMatrix(float &minX, float &maxX, float &minY, float &maxY)
{
    float deltax = maxX-minX;
    float deltay = maxY-minY;
    for( int i = 0; i < num_points-1; ++i ) {
        float x1p, y1p, x2p, y2p;
        if (deltax == 0.0f) {
            x1p = 0.0f;
            x2p = 0.0f;
        }
        else {
            x1p = (points[i].x - minX) / deltax;
            x2p = (points[i+1].x - minX) / deltax;
        }
        if (deltay == 0.0f) {
            y1p = 0.0f;
            y2p = 0.0f;
        }
        else {
            y1p = (points[i].y - minY) / deltay;
            y2p = (points[i+1].y - minY) / deltay;
        }

        float angle = (float)M_PI/2.0f;
        if (points[i+1].x != points[i].x) {
            float slope = (y2p - y1p)/(x2p - x1p);
            angle = std::atan(slope);
            if (points[i].x > points[i+1].x) {
                angle += (float)M_PI;
            }
        } else if (points[i+1].y < points[i].y) {
            angle += (float)M_PI;
        }
        float scale = std::sqrt((y2p - y1p)*(y2p - y1p) + (x2p - x1p)*(x2p - x1p));

        glm::mat3 scalingMatrix = glm::scale(glm::mat3(1.0f), glm::vec2(scale, 1.0));
        glm::mat3 rotationMatrix = glm::rotate(glm::mat3(1.0f), (float)angle);
        glm::mat3 translateMatrix = glm::translate(glm::mat3(1.0f), glm::vec2(x1p, y1p));
        glm::mat3 mat3 = translateMatrix * rotationMatrix * scalingMatrix;

        if (points[i].matrix != nullptr) {
            delete points[i].matrix;
        }
        points[i].matrix = new glm::mat3(mat3);
    }
}

bool BezierCurve::HitTest(int sx,int sy)
{
    if( !matrix_valid ) {
        UpdateMatrices();
    }

    float min_y, max_y;

    // iterate through all line segments of spline for hit detection
    for( int j = 0; j < num_points-1; ++j ) {
        //invert the matrix, get into render space
        glm::mat3 m = glm::inverse(*points[j].matrix);
        glm::vec3 v = m * glm::vec3(sx, sy, 1);

        float sx1 = (points[j].x + points[j+1].x) * width / 2.0;
        float sy1 = (points[j].y + points[j+1].y) * height / 2.0;

        glm::vec3 v2 = m * glm::vec3(sx1 + 3, sy1 + 3, 1);
        glm::vec3 v3 = m * glm::vec3(sx1 + 3, sy1 - 3, 1);
        glm::vec3 v4 = m * glm::vec3(sx1 - 3, sy1 + 3, 1);
        glm::vec3 v5 = m * glm::vec3(sx1 - 3, sy1 - 3, 1);
        max_y = std::max(std::max(v2.y, v3.y), std::max(v4.y, v5.y));
        min_y = std::min(std::min(v2.y, v3.y), std::min(v4.y, v5.y));

        if (v.x >= 0.0 && v.x <= 1.0 && v.y >= min_y && v.y <= max_y) {
            return true;
        }
    }
    return false;
}

void BezierCurve::check_min_max( float &minX, float &maxX, float &minY, float &maxY )
{
    for( int i = 0; i < num_points; ++i ) {
        if( points[i].x < minX ) minX = points[i].x;
        if( points[i].y < minY ) minY = points[i].y;
        if( points[i].x > maxX ) maxX = points[i].x;
        if( points[i].y > maxY ) maxY = points[i].y;
    }
}


///////////////////////////////////////////////
//////////////////   3D   /////////////////////
///////////////////////////////////////////////

BezierCurve3D::BezierCurve3D()
    : steps(25), old_steps(-1), scale(glm::vec3(100.0)), world_pos(glm::vec3(0.0f)), num_points(0), matrix_valid(false), total_length(0.0f)
{
}

BezierCurve3D::~BezierCurve3D()
{
}

BezierCurveCubic3D::BezierCurveCubic3D()
    : BezierCurve3D()
{
}

BezierCurveCubic3D::~BezierCurveCubic3D()
{
    clear_points();
}

void BezierCurve3D::clear_points()
{
    for (int j = 0; j < points.size(); ++j) {
        if (points[j].matrix != nullptr) {
            delete points[j].matrix;
            points[j].matrix = nullptr;
        }
        if (points[j].mod_matrix != nullptr) {
            delete points[j].mod_matrix;
            points[j].mod_matrix = nullptr;
        }
        if (points[j].mod_matrix2d != nullptr) {
            delete points[j].mod_matrix2d;
            points[j].mod_matrix2d = nullptr;
        }
    }
    points.clear();
    num_points = 0;
    matrix_valid = false;
}

float BezierCurve3D::interpPt(float n1, float n2, float perc)
{
    float diff = n2 - n1;
    return n1 + (diff * perc);
}

float BezierCurve3D::get_px(int num)
{
    if (num < num_points) {
        return points[num].x;
    }
    return 0.0f;
}

float BezierCurve3D::get_py(int num)
{
    if (num < num_points) {
        return points[num].y;
    }
    return 0.0f;
}

float BezierCurve3D::get_pz(int num)
{
    if (num < num_points) {
        return points[num].z;
    }
    return 0.0f;
}

void BezierCurve3D::OffsetX(float diff)
{
    cp0.x += diff;
}

void BezierCurve3D::OffsetY(float diff)
{
    cp0.y += diff;
}

void BezierCurve3D::OffsetZ(float diff)
{
    cp0.z += diff;
}

void BezierCurveCubic3D::OffsetX(float diff)
{
    cp0.x += diff;
    cp1.x += diff;
}

void BezierCurveCubic3D::OffsetY(float diff)
{
    cp0.y += diff;
    cp1.y += diff;
}

void BezierCurveCubic3D::OffsetZ(float diff)
{
    cp0.z += diff;
    cp1.z += diff;
}

void BezierCurveCubic3D::UpdatePoints() {
    float x, y, z, xa, xb, xc, xd, xe, ya, yb, yc, yd, ye, za, zb, zc, zd, ze;

    if (p0.x == old_p0.x  && p0.y == old_p0.y  && p0.z == old_p0.z &&
        p1.x == old_p1.x  && p1.y == old_p1.y  && p1.z == old_p1.z &&
        cp0.x == old_cp0.x && cp0.y == old_cp0.y && cp0.z == old_cp0.z &&
        cp1.x == old_cp1.x && cp1.y == old_cp1.y && cp1.z == old_cp1.z &&
        steps == old_steps && old_world == world_pos && old_scale == scale ) {
        return;
    }

    clear_points();
    num_points = 0;

    for (float i = 0.0f; i < 1.0f; i += 1.0f / (float)steps)
    {
        // Control Line Segments
        xa = interpPt(p0.x, cp0.x, i);
        ya = interpPt(p0.y, cp0.y, i);
        za = interpPt(p0.z, cp0.z, i);
        xb = interpPt(cp0.x, cp1.x, i);
        yb = interpPt(cp0.y, cp1.y, i);
        zb = interpPt(cp0.z, cp1.z, i);
        xc = interpPt(cp1.x, p1.x, i);
        yc = interpPt(cp1.y, p1.y, i);
        zc = interpPt(cp1.z, p1.z, i);

        // Between Control Lines
        xd = interpPt(xa, xb, i);
        yd = interpPt(ya, yb, i);
        zd = interpPt(za, zb, i);
        xe = interpPt(xb, xc, i);
        ye = interpPt(yb, yc, i);
        ze = interpPt(zb, zc, i);

        // the curve points
        x = interpPt(xd, xe, i);
        y = interpPt(yd, ye, i);
        z = interpPt(zd, ze, i);

        xlPointf new_point(x, y, z);
        new_point.matrix = nullptr;
        new_point.mod_matrix = nullptr;
        new_point.mod_matrix2d = nullptr;
        points.push_back(new_point);
        num_points++;
    }
    // add final point
    xlPointf new_point(p1.x, p1.y, p1.z);
    new_point.matrix = nullptr;
    new_point.mod_matrix = nullptr;
    new_point.mod_matrix2d = nullptr;
    points.push_back(new_point);
    num_points++;

    old_p0.x = p0.x;
    old_p0.y = p0.y;
    old_p0.z = p0.z;
    old_p1.x = p1.x;
    old_p1.y = p1.y;
    old_p1.z = p1.z;
    old_cp0.x = cp0.x;
    old_cp0.y = cp0.y;
    old_cp0.z = cp0.z;
    old_cp1.x = cp1.x;
    old_cp1.y = cp1.y;
    old_cp1.z = cp1.z;
    old_steps = steps;
    old_world = world_pos;
    old_scale = scale;
}

float BezierCurve3D::GetLength() {
    total_length = 0.0f;
    if (!matrix_valid) {
        UpdateMatrices();
    }
    for (int i = 0; i < num_points - 1; ++i) {
        total_length += points[i].length;
    }
    return total_length;
}

float BezierCurve3D::GetSegLength(int segment) {
    if (!matrix_valid) {
        UpdateMatrices();
    }
    if (segment < num_points - 1) {
        return points[segment].length;
    }
    else {
        return 0.0f;
    }
}

void BezierCurve3D::UpdateMatrices()
{
    for (int i = 0; i < num_points - 1; ++i) {
        float x1p = points[i].x * scale.x + world_pos.x;
        float x2p = points[i + 1].x * scale.x + world_pos.x;
        float y1p = points[i].y* scale.y + world_pos.y;
        float y2p = points[i + 1].y* scale.y + world_pos.y;
        float z1p = points[i].z * scale.z + world_pos.z;
        float z2p = points[i + 1].z * scale.z + world_pos.z;

        glm::vec3 pt1(x1p, y1p, z1p);
        glm::vec3 pt2(x2p, y2p, z2p);

        glm::vec3 a = pt2 - pt1;
        points[i].length = glm::length(a) / scale.x;

        glm::mat4 rotationMatrix = VectorMath::rotationMatrixFromXAxisToVector(a);
        glm::mat4 scalingMatrix = glm::scale(glm::mat4(1.0f), glm::vec3(points[i].length * scale.x, 1.0f, 1.0f));
        glm::mat4 translateMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(x1p, y1p, z1p));
        glm::mat4 mat = translateMatrix * rotationMatrix * scalingMatrix;
        glm::mat4 mod_mat = translateMatrix * rotationMatrix;

        if (points[i].matrix != nullptr) {
            delete points[i].matrix;
        }
        if (points[i].mod_matrix != nullptr) {
            delete points[i].mod_matrix;
        }
        points[i].matrix = new glm::mat4(mat);
        points[i].mod_matrix = new glm::mat4(mod_mat);

        {  // need to calculate matrix without Z axis to allow hit testing to work in 2D
            pt1.z = 0.0f;
            pt2.z = 0.0f;
            glm::vec3 a = pt2 - pt1;
            points[i].length2d = glm::length(a) / scale.x;
            glm::mat4 rotationMatrix = VectorMath::rotationMatrixFromXAxisToVector(a);
            glm::mat4 mod_mat = translateMatrix * rotationMatrix;
            if (points[i].mod_matrix2d != nullptr) {
                delete points[i].mod_matrix2d;
            }
            points[i].mod_matrix2d = new glm::mat4(mod_mat);
        }
    }
    matrix_valid = true;
}

void BezierCurve3D::CreateNormalizedMatrix(float &minX, float &maxX, float &minY, float &maxY, float &minZ, float &maxZ)
{
    float deltax = maxX - minX;
    float deltay = maxY - minY;
    float deltaz = maxZ - minZ;
    for (int i = 0; i < num_points - 1; ++i) {
        float x1p, y1p, z1p, x2p, y2p, z2p;
        if (deltax == 0.0f) {
            x1p = 0.0f;
            x2p = 0.0f;
        }
        else {
            x1p = (points[i].x - minX) / deltax;
            x2p = (points[i + 1].x - minX) / deltax;
        }
        if (deltay == 0.0f) {
            y1p = 0.0f;
            y2p = 0.0f;
        }
        else {
            y1p = (points[i].y - minY) / deltay;
            y2p = (points[i + 1].y - minY) / deltay;
        }
        if (deltaz == 0.0f) {
            z1p = 0.0f;
            z2p = 0.0f;
        }
        else {
            z1p = (points[i].z - minZ) / deltaz;
            z2p = (points[i + 1].z - minZ) / deltaz;
        }

        glm::vec3 pt1(x1p, y1p, z1p);
        glm::vec3 pt2(x2p, y2p, z2p);
        glm::vec3 a = pt2 - pt1;
        float scalex = glm::length(a);
        glm::mat4 rotationMatrix = VectorMath::rotationMatrixFromXAxisToVector(a);
        glm::mat4 scalingMatrix = glm::scale(glm::mat4(1.0f), glm::vec3(scalex, 1.0f, 1.0f));
        glm::mat4 translateMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(x1p, y1p, z1p));
        glm::mat4 mat = translateMatrix * rotationMatrix * scalingMatrix;

        if (points[i].matrix != nullptr) {
            delete points[i].matrix;
        }
        points[i].matrix = new glm::mat4(mat);
    }
}

bool BezierCurve3D::HitTest(glm::vec3& ray_origin)
{
    if (!matrix_valid) {
        UpdateMatrices();
    }

    // iterate through all line segments of spline for hit detection
    for (int j = 0; j < num_points - 1; ++j) {
        if (VectorMath::TestRayOBBIntersection2D(
            ray_origin,
            points[j].aabb_min,
            points[j].aabb_max,
            *points[j].mod_matrix2d)
            ) {
            sub_segment = j;
            return true;
        }
    }
    return false;
}

bool BezierCurve3D::HitTest3D(glm::vec3& ray_origin, glm::vec3& ray_direction, float& intersection_distance)
{
    if (!matrix_valid) {
        UpdateMatrices();
    }

    float distance = 1000000000.0f;

    // iterate through all line segments of spline for hit detection
    for (int j = 0; j < num_points - 1; ++j) {
        // perform normal line segment hit detection
        if (VectorMath::TestRayOBBIntersection(
            ray_origin,
            ray_direction,
            points[j].aabb_min,
            points[j].aabb_max,
            *points[j].mod_matrix,
            distance)
            ) {
            if (distance < intersection_distance) {
                intersection_distance = distance;
                return true;
            }
        }
    }
    return false;
}

static float BB_OFF = 5.0f;

void BezierCurve3D::UpdateBoundingBox(bool is_3d)
{
    if (!matrix_valid) {
        UpdateMatrices();
    }
    for (int i = 0; i < num_points - 1; ++i) {
        // create normal line segment bounding boxes
        points[i].aabb_min = glm::vec3(0.0f, -BB_OFF, -BB_OFF);
        if (is_3d) {
            points[i].aabb_max = glm::vec3(points[i].length * scale.x, BB_OFF, BB_OFF);
        }
        else {
            points[i].aabb_max = glm::vec3(points[i].length2d * scale.x, BB_OFF, BB_OFF);
        }
    }
}

void BezierCurve3D::check_min_max(float &minX, float &maxX, float &minY, float &maxY, float &minZ, float &maxZ)
{
    for (int i = 0; i < num_points; ++i) {
        if (points[i].x < minX) minX = points[i].x;
        if (points[i].y < minY) minY = points[i].y;
        if (points[i].z < minZ) minZ = points[i].z;
        if (points[i].x > maxX) maxX = points[i].x;
        if (points[i].y > maxY) maxY = points[i].y;
        if (points[i].z > maxZ) maxZ = points[i].z;
    }
}

void BezierCurve3D::DrawBoundingBox(xlColor c, DrawGLUtils::xlAccumulator &va)
{
    if (!matrix_valid) {
        UpdateMatrices();
    }
    DrawGLUtils::DrawBoundingBox(c, points[sub_segment].aabb_min, points[sub_segment].aabb_max, *points[sub_segment].mod_matrix2d, va);
}

void BezierCurve3D::DrawBoundingBoxes(xlColor c, DrawGLUtils::xl3Accumulator &va)
{
    if (!matrix_valid) {
        UpdateMatrices();
    }
    for (int i = 0; i < num_points - 1; ++i) {
        DrawGLUtils::DrawBoundingBox(c, points[i].aabb_min, points[i].aabb_max, *points[i].mod_matrix, va);
    }
}
