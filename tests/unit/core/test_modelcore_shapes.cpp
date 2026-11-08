#include <cmath>
#include <vector>

#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>

#include <glm/glm.hpp>

#include "graphics/xlGraphicsAccumulators.h"
#include "models/Shapes.h"

using Catch::Approx;

namespace {

struct CountingCVA : xlVertexColorAccumulator {
    std::vector<glm::vec3> v;
    void PreAlloc(unsigned int) override {}
    void AddVertex(float x, float y, float z, const xlColor&) override { v.push_back({ x, y, z }); }
    uint32_t getCount() override { return (uint32_t)v.size(); }
    void SetVertex(uint32_t, float, float, float, const xlColor&) override {}
    void SetVertex(uint32_t, float, float, float) override {}
    void SetVertex(uint32_t, const xlColor&) override {}
};

// A straight cubic: control points on the line between the ends.
BezierCurveCubic StraightCurve2D() {
    BezierCurveCubic c;
    c.set_p0(0.0f, 0.0f);
    c.set_cp0(1.0f / 3.0f, 0.0f);
    c.set_cp1(2.0f / 3.0f, 0.0f);
    c.set_p1(1.0f, 0.0f);
    return c;
}

} // namespace

TEST_CASE("2D cubic bezier samples from p0 to p1", "[modelcore][shapes]") {
    BezierCurveCubic c;
    c.set_p0(0.0f, 0.0f);
    c.set_cp0(0.0f, 1.0f);
    c.set_cp1(1.0f, 1.0f);
    c.set_p1(1.0f, 0.0f);
    c.UpdatePoints();

    int n = c.GetNumPoints();
    REQUIRE(n >= 26);
    CHECK(c.GetNumSegments() == n - 1);
    CHECK(c.get_px(0) == 0.0f);
    CHECK(c.get_py(0) == 0.0f);
    CHECK(c.get_px(n - 1) == 1.0f);
    CHECK(c.get_py(n - 1) == 0.0f);
    // out of range reads are 0
    CHECK(c.get_px(n) == 0.0f);
    CHECK(c.get_py(n + 5) == 0.0f);

    // symmetric control polygon: the apex is at x=0.5, y=0.75 (t=0.5)
    float maxY = 0.0f;
    for (int i = 0; i < n; ++i) {
        maxY = std::max(maxY, c.get_py(i));
        CHECK(c.get_py(i) >= 0.0f);
        if (i > 0) {
            CHECK(c.get_px(i) >= c.get_px(i - 1));
        }
    }
    CHECK(maxY == Approx(0.75f).margin(0.01f));

    float minX = 10, maxX = -10, minY = 10, maxYY = -10;
    c.check_min_max(minX, maxX, minY, maxYY);
    CHECK(minX == 0.0f);
    CHECK(maxX == 1.0f);
    CHECK(minY == 0.0f);
    CHECK(maxYY == Approx(0.75f).margin(0.01f));
}

TEST_CASE("2D cubic bezier only recomputes when its inputs change", "[modelcore][shapes]") {
    BezierCurveCubic c = StraightCurve2D();
    c.UpdatePoints();
    int n = c.GetNumPoints();
    float x5 = c.get_px(5);
    c.UpdatePoints();
    CHECK(c.GetNumPoints() == n);
    CHECK(c.get_px(5) == x5);

    // moving the end point re-samples
    c.set_p1(2.0f, 0.0f);
    c.UpdatePoints();
    CHECK(c.get_px(c.GetNumPoints() - 1) == 2.0f);
}

TEST_CASE("2D bezier length scales with the render size", "[modelcore][shapes]") {
    BezierCurveCubic c = StraightCurve2D();
    c.SetScale(100, 50, 1.0f);
    c.UpdatePoints();
    CHECK(c.GetLength() == Approx(100.0f).epsilon(0.001));
    float sum = 0.0f;
    for (int i = 0; i < c.GetNumSegments(); ++i) {
        sum += c.GetSegLength(i);
        CHECK(c.GetMatrix(i) != nullptr);
    }
    CHECK(sum == Approx(100.0f).epsilon(0.001));
    CHECK(c.GetSegLength(c.GetNumSegments()) == 0.0f);

    // ren_width divides the length
    BezierCurveCubic c2 = StraightCurve2D();
    c2.SetScale(100, 50, 4.0f);
    c2.UpdatePoints();
    CHECK(c2.GetLength() == Approx(25.0f).epsilon(0.001));
}

TEST_CASE("2D bezier segment matrix maps the unit x axis onto the segment", "[modelcore][shapes]") {
    BezierCurveCubic c;
    // a vertical straight line, travelling down: the else-if branch for equal x
    c.set_p0(0.5f, 1.0f);
    c.set_cp0(0.5f, 2.0f / 3.0f);
    c.set_cp1(0.5f, 1.0f / 3.0f);
    c.set_p1(0.5f, 0.0f);
    c.SetScale(10, 10, 1.0f);
    c.UpdatePoints();
    c.GetLength();
    glm::mat3* m = c.GetMatrix(0);
    REQUIRE(m != nullptr);
    glm::vec3 start = (*m) * glm::vec3(0, 0, 1);
    glm::vec3 end = (*m) * glm::vec3(1, 0, 1);
    CHECK(start.x == Approx(5.0f));
    CHECK(start.y == Approx(10.0f));
    CHECK(end.x == Approx(5.0f).margin(1e-4));
    CHECK(end.y == Approx(10.0f - c.GetSegLength(0)).margin(1e-3));

    // a line going right-to-left adds pi to the angle
    BezierCurveCubic r;
    r.set_p0(1.0f, 0.0f);
    r.set_cp0(2.0f / 3.0f, 0.0f);
    r.set_cp1(1.0f / 3.0f, 0.0f);
    r.set_p1(0.0f, 0.0f);
    r.SetScale(10, 10, 1.0f);
    r.UpdatePoints();
    r.GetLength();
    glm::vec3 e = (*r.GetMatrix(0)) * glm::vec3(1, 0, 1);
    CHECK(e.x < 10.0f);
    CHECK(e.y == Approx(0.0f).margin(1e-4));
}

TEST_CASE("2D bezier hit test finds points on the curve only", "[modelcore][shapes]") {
    BezierCurveCubic c = StraightCurve2D();
    c.SetScale(100, 100, 1.0f);
    c.UpdatePoints();
    CHECK(c.HitTest(50, 0));
    CHECK(c.HitTest(10, 1));
    CHECK_FALSE(c.HitTest(50, 40));
    CHECK_FALSE(c.HitTest(150, 0));
}

TEST_CASE("2D bezier offsets move only the control points", "[modelcore][shapes]") {
    BezierCurveCubic c = StraightCurve2D();
    c.OffsetX(2.0f);
    c.OffsetY(3.0f);
    CHECK(c.get_cp0x() == Approx(1.0f / 3.0f + 2.0f));
    CHECK(c.get_cp1x() == Approx(2.0f / 3.0f + 2.0f));
    CHECK(c.get_cp0y() == 3.0f);
    CHECK(c.get_cp1y() == 3.0f);
    CHECK(c.get_p0x() == 0.0f);
    CHECK(c.get_p1x() == 1.0f);
    CHECK(c.get_p0y() == 0.0f);
    CHECK(c.get_p1y() == 0.0f);
}

TEST_CASE("2D bezier normalized matrix spans the unit square", "[modelcore][shapes]") {
    BezierCurveCubic c;
    c.set_p0(2.0f, 2.0f);
    c.set_cp0(2.0f, 4.0f);
    c.set_cp1(4.0f, 4.0f);
    c.set_p1(4.0f, 2.0f);
    c.UpdatePoints();
    float minX = 2, maxX = 4, minY = 2, maxY = 3.5f;
    c.CreateNormalizedMatrix(minX, maxX, minY, maxY);
    glm::vec3 s = (*c.GetMatrix(0)) * glm::vec3(0, 0, 1);
    CHECK(s.x == Approx(0.0f).margin(1e-5));
    CHECK(s.y == Approx(0.0f).margin(1e-5));
    int last = c.GetNumSegments() - 1;
    glm::vec3 e = (*c.GetMatrix(last)) * glm::vec3(1, 0, 1);
    CHECK(e.x == Approx(1.0f).margin(1e-4));
    CHECK(e.y == Approx(0.0f).margin(1e-4));
}

TEST_CASE("3D cubic bezier samples a straight line in space", "[modelcore][shapes]") {
    BezierCurveCubic3D c;
    c.set_p0(0, 0, 0);
    c.set_cp0(1.0f / 3.0f, 0, 0);
    c.set_cp1(2.0f / 3.0f, 0, 0);
    c.set_p1(1, 0, 0);
    glm::vec3 scale(100.0f, 100.0f, 100.0f);
    glm::vec3 pos(10.0f, 20.0f, 30.0f);
    c.SetPositioning(scale, pos);
    c.UpdatePoints();
    int n = c.GetNumPoints();
    REQUIRE(n >= 26);
    CHECK(c.get_px(n - 1) == 1.0f);
    CHECK(c.get_py(n - 1) == 0.0f);
    CHECK(c.get_pz(n - 1) == 0.0f);
    CHECK(c.get_pz(n) == 0.0f);
    CHECK(c.get_px(n) == 0.0f);
    CHECK(c.get_py(n) == 0.0f);

    // length is reported in model units (divided by scale.x)
    CHECK(c.GetLength() == Approx(1.0f).epsilon(0.001));
    CHECK(c.GetSegLength(-1) == 0.0f);
    CHECK(c.GetSegLength(n - 1) == 0.0f);
    glm::vec4 start = (*c.GetMatrix(0)) * glm::vec4(0, 0, 0, 1);
    CHECK(start.x == Approx(10.0f));
    CHECK(start.y == Approx(20.0f));
    CHECK(start.z == Approx(30.0f));
    glm::vec4 end = (*c.GetMatrix(n - 2)) * glm::vec4(1, 0, 0, 1);
    CHECK(end.x == Approx(110.0f).epsilon(0.001));
    CHECK(end.y == Approx(20.0f).margin(1e-3));

    float minX = 5, maxX = -5, minY = 5, maxY = -5, minZ = 5, maxZ = -5;
    c.check_min_max(minX, maxX, minY, maxY, minZ, maxZ);
    CHECK(minX == 0.0f);
    CHECK(maxX == 1.0f);
    CHECK(minZ == 0.0f);
    CHECK(maxZ == 0.0f);
}

TEST_CASE("3D bezier resamples when its position changes", "[modelcore][shapes]") {
    BezierCurveCubic3D c;
    c.set_p0(0, 0, 0);
    c.set_cp0(0, 1, 0);
    c.set_cp1(1, 1, 1);
    c.set_p1(1, 0, 1);
    glm::vec3 scale(10.0f, 10.0f, 10.0f);
    glm::vec3 pos(0.0f, 0.0f, 0.0f);
    c.SetPositioning(scale, pos);
    c.UpdatePoints();
    float len1 = c.GetLength();
    CHECK(len1 > std::sqrt(2.0f));
    c.UpdatePoints(); // no change, no resample
    CHECK(c.GetLength() == Approx(len1));

    glm::vec3 scale2(20.0f, 20.0f, 20.0f);
    c.SetPositioning(scale2, pos);
    c.UpdatePoints();
    // uniform scaling leaves the length in model units unchanged
    CHECK(c.GetLength() == Approx(len1).epsilon(0.001));
}

TEST_CASE("3D bezier offsets move only the control points", "[modelcore][shapes]") {
    BezierCurveCubic3D c;
    c.set_p0(0, 0, 0);
    c.set_cp0(1, 2, 3);
    c.set_cp1(4, 5, 6);
    c.set_p1(7, 8, 9);
    c.OffsetX(1);
    c.OffsetY(2);
    c.OffsetZ(3);
    CHECK(c.get_cp0x() == 2);
    CHECK(c.get_cp0y() == 4);
    CHECK(c.get_cp0z() == 6);
    CHECK(c.get_cp1x() == 5);
    CHECK(c.get_cp1y() == 7);
    CHECK(c.get_cp1z() == 9);
    CHECK(c.get_p0x() == 0);
    CHECK(c.get_p0y() == 0);
    CHECK(c.get_p0z() == 0);
    CHECK(c.get_p1x() == 7);
    CHECK(c.get_p1y() == 8);
    CHECK(c.get_p1z() == 9);
}

TEST_CASE("3D bezier hit tests use the segment bounding boxes", "[modelcore][shapes]") {
    BezierCurveCubic3D c;
    c.set_p0(0, 0, 0);
    c.set_cp0(1.0f / 3.0f, 0, 0);
    c.set_cp1(2.0f / 3.0f, 0, 0);
    c.set_p1(1, 0, 0);
    glm::vec3 scale(100.0f, 100.0f, 100.0f);
    glm::vec3 pos(0.0f, 0.0f, 0.0f);
    c.SetPositioning(scale, pos);
    c.UpdatePoints();
    c.UpdateBoundingBox(false);

    glm::vec3 on(50.0f, 1.0f, 0.0f);
    CHECK(c.HitTest(on));
    glm::vec3 off(50.0f, 40.0f, 0.0f);
    CHECK_FALSE(c.HitTest(off));

    c.UpdateBoundingBox(true);
    glm::vec3 origin(50.0f, 0.0f, 100.0f);
    glm::vec3 dir(0.0f, 0.0f, -1.0f);
    float dist = 1e9f;
    CHECK(c.HitTest3D(origin, dir, dist));
    CHECK(dist == Approx(95.0f).margin(0.5f)); // box is +/-5 deep
    // a second test with a closer recorded distance does not replace it
    float closer = 1.0f;
    CHECK_FALSE(c.HitTest3D(origin, dir, closer));
    glm::vec3 missOrigin(50.0f, 50.0f, 100.0f);
    float d2 = 1e9f;
    CHECK_FALSE(c.HitTest3D(missOrigin, dir, d2));

    CountingCVA va;
    c.DrawBoundingBoxes(xlWHITE, &va);
    CHECK(va.v.size() == (size_t)c.GetNumSegments() * 24); // 12 box edges, 2 vertices each
    CountingCVA one;
    c.HitTest(on); // selects the sub segment drawn by DrawBoundingBox
    c.DrawBoundingBox(xlWHITE, &one);
    CHECK(one.v.size() == 24);
}

TEST_CASE("3D bezier normalized matrix spans the unit cube", "[modelcore][shapes]") {
    BezierCurveCubic3D c;
    c.set_p0(0, 0, 0);
    c.set_cp0(1.0f / 3.0f, 1.0f / 3.0f, 1.0f / 3.0f);
    c.set_cp1(2.0f / 3.0f, 2.0f / 3.0f, 2.0f / 3.0f);
    c.set_p1(2, 2, 2);
    c.UpdatePoints();
    float minX = 0, maxX = 2, minY = 0, maxY = 2, minZ = 0, maxZ = 2;
    c.CreateNormalizedMatrix(minX, maxX, minY, maxY, minZ, maxZ);
    glm::vec4 e = (*c.GetMatrix(c.GetNumSegments() - 1)) * glm::vec4(1, 0, 0, 1);
    CHECK(e.x == Approx(1.0f).margin(1e-3));
    CHECK(e.y == Approx(1.0f).margin(1e-3));
    CHECK(e.z == Approx(1.0f).margin(1e-3));
}

