#pragma once

#include <cmath>
#include <glm/glm.hpp>

/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

struct xlPoint {
    int x = 0;
    int y = 0;
    xlPoint() = default;
    xlPoint(int x_, int y_) : x(x_), y(y_) {}
    bool operator<(const xlPoint& r) const {
        if (x < r.x) return true;
        if (x > r.x) return false;
        return y < r.y;
    }
    xlPoint operator+(const xlPoint& r) const {
        return xlPoint(x + r.x, y + r.y);
    }
    bool operator==(const xlPoint& r) const {
        return x == r.x && y == r.y;
    }
    bool operator!=(const xlPoint& r) const {
        return !(*this == r);
    }
};

struct xlPointD {
    double x = 0.0;
    double y = 0.0;
    xlPointD() = default;
    xlPointD(double x_, double y_) : x(x_), y(y_) {}
    xlPointD(const glm::dvec2& v) : x(v.x), y(v.y) {}
    operator glm::dvec2() const { return glm::dvec2(x, y); }
    xlPointD operator+(const xlPointD& r) const {
        return xlPointD(x + r.x, y + r.y);
    }
    xlPointD& operator+=(const xlPointD& r) {
        x += r.x; y += r.y; return *this;
    }
    xlPointD operator-(const xlPointD& r) const {
        return xlPointD(x - r.x, y - r.y);
    }
    xlPointD& operator-=(const xlPointD& r) {
        x -= r.x; y -= r.y; return *this;
    }
    xlPointD operator*(double s) const {
        return xlPointD(x * s, y * s);
    }
    bool operator==(const xlPointD& r) const {
        return x == r.x && y == r.y;
    }
    bool operator!=(const xlPointD& r) const {
        return !(*this == r);
    }
    friend xlPointD operator*(double s, const xlPointD& p) {
        return p * s;
    }
    double dot(const xlPointD& r) const {
        return x * r.x + y * r.y;
    }
    double lengthSq() const {
        return x * x + y * y;
    }
    double length() const {
        return std::sqrt(lengthSq());
    }
    double distance(const xlPointD& r) const {
        return (*this - r).length();
    }
};
