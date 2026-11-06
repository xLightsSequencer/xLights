// The GPU rotate/zoom (Metal and Vulkan) against PixelBufferClass::RotateX/Y/
// RotateZAndZoom. The kernels scatter like the CPU loops, truncate like
// SetPixel and resolve collisions in the CPU's write order, so the renders
// are byte-identical. Skips without a GPU compute backend; see test_gpu_common.h.

#include <catch2/catch_test_macros.hpp>

#include <string>
#include <vector>

#include "test_gpu_common.h"

using namespace gpuhelp;

namespace {

struct Case {
    const char* name;
    std::string settings;
};

// Left-moving bars change along x, up-moving along y: each rotation gets
// content it actually moves.
const std::string kLeft = "E_CHOICE_Bars_Direction=Left";
const std::string kUp = "E_CHOICE_Bars_Direction=up";

const std::vector<Case>& Cases() {
    static const std::vector<Case> v = {
        { "z rotation", kLeft + ",B_SLIDER_Rotation=30" },
        { "zoom in", kLeft + ",B_SLIDER_Zoom=15" },
        { "zoom out", kLeft + ",B_SLIDER_Zoom=6" },
        { "rotation and zoom", kLeft + ",B_SLIDER_Rotation=30,B_SLIDER_Zoom=15" },
        { "zoom quality 3 around an off-centre pivot",
          kLeft + ",B_SLIDER_Rotation=45,B_SLIDER_Zoom=22,B_SLIDER_ZoomQuality=3,B_SLIDER_PivotPointX=30,B_SLIDER_PivotPointY=70" },
        { "x rotation", kLeft + ",B_SLIDER_XRotation=60,B_SLIDER_XPivot=35" },
        { "x rotation past 90 degrees", kLeft + ",B_SLIDER_XRotation=135" },
        { "y rotation", kUp + ",B_SLIDER_YRotation=60,B_SLIDER_YPivot=65" },
        { "x, y and z in Z-Y-X order",
          kLeft + ",B_SLIDER_XRotation=40,B_SLIDER_YRotation=50,B_SLIDER_Rotation=20,B_SLIDER_Zoom=12,B_CHOICE_RZ_RotationOrder=Z-Y-X" },
    };
    return v;
}

} // namespace

TEST_CASE("GPU rotation and zoom match the CPU render exactly", "[gpu][rotozoom]") {
    XL_REQUIRE_GPU();
    for (const auto& c : Cases()) {
        INFO(c.name);
        Pair p = RenderBoth(MatrixShow({ Eff("M", 0, "Bars", 0, 500, Pal("#FF0000", "#0000FF"), c.settings) }),
                            [] { return GPURenderUtils::stats.rotoZoom.load(); });
        CheckWithin(p, { 0.0, 0 });
    }
}
