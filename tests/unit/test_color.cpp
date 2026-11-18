#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

#include "utils/Color.h"

using Catch::Matchers::WithinAbs;

TEST_CASE("xlColor constructors", "[color]") {
    xlColor def;
    CHECK(def == xlColor(0, 0, 0, 255));

    xlColor rgb(0xFF8000);
    CHECK(rgb.Red() == 0xFF);
    CHECK(rgb.Green() == 0x80);
    CHECK(rgb.Blue() == 0x00);

    xlColor bgr(0xFF8000, true);
    CHECK(bgr.Red() == 0x00);
    CHECK(bgr.Green() == 0x80);
    CHECK(bgr.Blue() == 0xFF);

    CHECK(xlColor::NilColor().IsNilColor());
    CHECK_FALSE(xlBLACK.IsNilColor());
}

TEST_CASE("xlColor packs RGB in both orders", "[color]") {
    xlColor c(0x11, 0x22, 0x33);
    CHECK(c.GetRGB(true) == 0x332211u);
    CHECK(c.GetRGB(false) == 0x112233u);
    CHECK(c.GetRGBA(false) == 0xFF112233u);
}

TEST_CASE("xlColor parses string forms", "[color]") {
    SECTION("#RRGGBB") {
        CHECK(xlColor("#FF8000") == xlColor(255, 128, 0));
    }
    SECTION("#AARRGGBB") {
        CHECK(xlColor("#80FF0000") == xlColor(255, 0, 0, 0x80));
    }
    SECTION("0xRRGGBB") {
        CHECK(xlColor("0x0000FF") == xlBLUE);
    }
    SECTION("rgb() and rgba()") {
        CHECK(xlColor("rgb(10,20,30)") == xlColor(10, 20, 30));
        xlColor a("rgba(10,20,30,0.5)");
        CHECK(a.Red() == 10);
        CHECK(a.Green() == 20);
        CHECK(a.Blue() == 30);
        CHECK(a.Alpha() == 127);
    }
    SECTION("named colours are case-insensitive") {
        CHECK(xlColor("Red") == xlRED);
        CHECK(xlColor("rEd") == xlRED);
    }
    SECTION("empty and unknown strings give black") {
        CHECK(xlColor("") == xlBLACK);
        CHECK(xlColor("not a colour") == xlBLACK);
    }
}

TEST_CASE("xlColor string conversion round-trips", "[color]") {
    xlColor c(1, 2, 3);
    xlColor back(static_cast<std::string>(c));
    CHECK(back == c);
}

TEST_CASE("xlColor blends", "[color]") {
    CHECK(xlWHITE.Blend(xlBLACK) == xlColor(127, 127, 127));
    CHECK(xlColor(10, 200, 30).ChannelMax(xlColor(100, 20, 40)) == xlColor(100, 200, 40));

    CHECK(xlColor(255, 0, 0, 0).AlphaBlend(xlBLUE) == xlBLUE);
    CHECK(xlColor(255, 0, 0, 255).AlphaBlend(xlBLUE) == xlRED);

    xlColor bg(0, 0, 0);
    bg.AlphaBlendForgroundOnto(xlColor(200, 100, 50, 255));
    CHECK(bg.Red() == 200);
}

TEST_CASE("xlColor brightness", "[color]") {
    CHECK(xlColor(10, 200, 30).Brightness() == 200);
    CHECK(xlColor(10, 200, 30, 0).Brightness() == 0);
    CHECK(xlColor(255, 255, 255).ApplyBrightness(0.5f) == xlColor(127, 127, 127));
}

TEST_CASE("HSV conversion", "[color]") {
    HSVValue red(xlRED);
    CHECK_THAT(red.hue, WithinAbs(0.0, 0.001));
    CHECK_THAT(red.saturation, WithinAbs(1.0, 0.001));
    CHECK_THAT(red.value, WithinAbs(1.0, 0.001));

    HSVValue green(xlGREEN);
    CHECK_THAT(green.hue, WithinAbs(1.0 / 3.0, 0.001));

    HSVValue grey(xlColor(128, 128, 128));
    CHECK_THAT(grey.saturation, WithinAbs(0.0, 0.001));

    CHECK(xlColor(HSVValue(0.0f, 1.0f, 1.0f)) == xlRED);
    CHECK(xlColor(HSVValue(2.0f / 3.0f, 1.0f, 1.0f)) == xlBLUE);

    SECTION("round-trips through HSV within one level") {
        for (int r = 0; r < 256; r += 17)
            for (int g = 0; g < 256; g += 17)
                for (int b = 0; b < 256; b += 17) {
                    xlColor c(r, g, b);
                    HSVValue h(c);
                    xlColor back(h);
                    CHECK(std::abs((int)back.Red() - r) <= 1);
                    CHECK(std::abs((int)back.Green() - g) <= 1);
                    CHECK(std::abs((int)back.Blue() - b) <= 1);
                }
    }
}

TEST_CASE("xlColor contrast", "[color]") {
    CHECK(xlBLACK.HasSufficientContrast(xlWHITE));
    CHECK_FALSE(xlWHITE.HasSufficientContrast(xlWHITE));
}
