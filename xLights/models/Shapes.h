#ifndef SHAPES_H
#define SHAPES_H

#include <vector>
#include <glm/mat3x3.hpp>
#include "../DrawGLUtils.h"

class BezierCurve
{
    public:
        BezierCurve();
        virtual ~BezierCurve();

        struct xlPointf {
            float x;
            float y;
            float length;
            glm::mat3 *matrix;
            xlPointf( ) : x(0.0f), y(0.0f), length(0.0f), matrix(nullptr) {}
            xlPointf( float x_, float y_ ) : x(x_), y(y_), length(0.0f), matrix(nullptr) {}
        };

        virtual void UpdatePoints() = 0;
		void UpdateMatrices();
        void CreateNormalizedMatrix(float &minX, float &maxX, float &minY, float &maxY);
        float GetLength();
        float GetSegLength(int segment);
        int GetNumPoints() const {return num_points;}
        int GetNumSegments() const {return num_points-1;}
        glm::mat3* GetMatrix(int segment) {return points[segment].matrix;}
        float get_px(int num);
        float get_py(int num);

        void check_min_max( float &minX, float &maxX, float &minY, float &maxY );

        bool HitTest(int sx, int sy);

        virtual void OffsetX(float diff);
        virtual void OffsetY(float diff);
        void SetScale( int width_, int height_, float ren_width_ )
            { width = width_; height = height_; ren_width = ren_width_; }

        void set_p0( float x_, float y_ ) { p0.x = x_; p0.y = y_; }
        void set_p1( float x_, float y_ ) { p1.x = x_; p1.y = y_; }
        void set_cp0( float x_, float y_ ) { cp0.x = x_; cp0.y = y_; }

        float get_p0x() const {return p0.x;}
        float get_p0y() const {return p0.y;}
        float get_p1x() const {return p1.x;}
        float get_p1y() const {return p1.y;}
        float get_cp0x() const {return cp0.x;}
        float get_cp0y() const {return cp0.y;}

	protected:
		xlPointf p0;
		xlPointf p1;
		xlPointf cp0;
		int steps;
		int width;
		int height;
		int num_points;
		float ren_width;
		bool matrix_valid;

		std::vector<xlPointf> points;

		xlPointf old_p0;
		xlPointf old_p1;
		xlPointf old_cp0;
		int old_steps;

		float interpPt(float n1 , float n2 , float perc );
		void clear_points();
};

class BezierCurveCubic : public BezierCurve
{
    public:
        BezierCurveCubic();
        virtual ~BezierCurveCubic();

        void set_cp1( float x_, float y_ ) { cp1.x = x_; cp1.y = y_; }
        virtual void OffsetX(float diff) override;
        virtual void OffsetY(float diff) override;

        float get_cp1x() const {return cp1.x;}
        float get_cp1y() const {return cp1.y;}

    	virtual void UpdatePoints() override;

	private:
		xlPointf cp1;
		xlPointf old_cp1;
};

class BezierCurve3D
{
public:
    BezierCurve3D();
    virtual ~BezierCurve3D();

    struct xlPointf {
        float x;
        float y;
        float z;
        float length;
        float length2d;
        glm::mat4 *matrix;
        glm::mat4 *mod_matrix;
        glm::mat4 *mod_matrix2d;
        glm::vec3 aabb_min;
        glm::vec3 aabb_max;
        xlPointf() : x(0.0f), y(0.0f), z(0.0f), length(0.0f), length2d(0.0f),
            matrix(nullptr), mod_matrix(nullptr),
            aabb_min(glm::vec3(0.0f)), aabb_max(glm::vec3(1.0f)) {}
        xlPointf(float x_, float y_, float z_) : x(x_), y(y_), z(z_), length(0.0f), length2d(0.0f),
            matrix(nullptr), mod_matrix(nullptr),
            aabb_min(glm::vec3(0.0f)), aabb_max(glm::vec3(1.0f)) {}
    };

    virtual void UpdatePoints() = 0;
    void UpdateMatrices();
    void UpdateBoundingBox(bool is_3d);
    void DrawBoundingBoxes(DrawGLUtils::xl3Accumulator &va);
    void DrawBoundingBox(DrawGLUtils::xlAccumulator &va); // used for debugging hit testing
    void CreateNormalizedMatrix(float &minX, float &maxX, float &minY, float &maxY, float &minZ, float &maxZ);
    float GetLength();
    float GetSegLength(int segment);
    int GetNumPoints() const { return num_points; }
    int GetNumSegments() const { return num_points - 1; }
    glm::mat4* GetMatrix(int segment) { return points[segment].matrix; }
    float get_px(int num);
    float get_py(int num);
    float get_pz(int num);

    void check_min_max(float &minX, float &maxX, float &minY, float &maxY, float &minZ, float &maxZ);

    bool HitTest(glm::vec3& ray_origin);
    bool HitTest3D(glm::vec3& ray_origin, glm::vec3& ray_direction, float& intersection_distance);

    virtual void OffsetX(float diff);
    virtual void OffsetY(float diff);
    virtual void OffsetZ(float diff);
    void SetPositioning(glm::vec3& scale_, glm::vec3& position)
    {
        world_pos = position; scale = scale_;
    }

    void set_p0(float x_, float y_, float z_) { p0.x = x_; p0.y = y_; p0.z = z_; }
    void set_p1(float x_, float y_, float z_) { p1.x = x_; p1.y = y_; p1.z = z_; }
    void set_cp0(float x_, float y_, float z_) { cp0.x = x_; cp0.y = y_; cp0.z = z_; }

    float get_p0x() const { return p0.x; }
    float get_p0y() const { return p0.y; }
    float get_p0z() const { return p0.z; }
    float get_p1x() const { return p1.x; }
    float get_p1y() const { return p1.y; }
    float get_p1z() const { return p1.z; }
    float get_cp0x() const { return cp0.x; }
    float get_cp0y() const { return cp0.y; }
    float get_cp0z() const { return cp0.z; }

protected:
    xlPointf p0;
    xlPointf p1;
    xlPointf cp0;
    int steps;
    int num_points;
    bool matrix_valid;
    float total_length;
    glm::vec3 scale;
    glm::vec3 world_pos;
    int sub_segment;

    std::vector<xlPointf> points;

    xlPointf old_p0;
    xlPointf old_p1;
    xlPointf old_cp0;
    glm::vec3 old_world;
    glm::vec3 old_scale;
    int old_steps;

    float interpPt(float n1, float n2, float perc);
    void clear_points();
};

class BezierCurveCubic3D : public BezierCurve3D
{
public:
    BezierCurveCubic3D();
    virtual ~BezierCurveCubic3D();

    void set_cp1(float x_, float y_, float z_) { cp1.x = x_; cp1.y = y_; cp1.z = z_; }
    virtual void OffsetX(float diff) override;
    virtual void OffsetY(float diff) override;
    virtual void OffsetZ(float diff) override;

    float get_cp1x() const { return cp1.x; }
    float get_cp1y() const { return cp1.y; }
    float get_cp1z() const { return cp1.z; }

    virtual void UpdatePoints() override;

private:
    xlPointf cp1;
    xlPointf old_cp1;
};
#endif // SHAPES_H
