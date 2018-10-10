#include "Shapes.h"
#include "../DrawGLUtils.h"
#include <glm/glm.hpp>
#include <glm/gtx/matrix_transform_2d.hpp>
#include <glm/mat3x3.hpp>

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

        glm::mat3 scalingMatrix = glm::scale(glm::mat3(1.0f), glm::vec2(scale, scale));
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
