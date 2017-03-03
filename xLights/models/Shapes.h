#ifndef SHAPES_H
#define SHAPES_H

#include <vector>
#include <glm/mat3x3.hpp>

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

        bool HitTest(int sx,int sy);

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

#endif // SHAPES_H
