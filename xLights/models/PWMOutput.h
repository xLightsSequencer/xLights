#pragma once

/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/


class PWMOutput {
public:
    enum class Type {
        SERVO,
        LED
    };

    PWMOutput() {};
    PWMOutput(PWMOutput&&) = default;
    PWMOutput(const PWMOutput&) = default;
    
    PWMOutput(uint32_t sc, Type t, int c) : startChannel(sc), type(t), channels(c) {}
    PWMOutput(uint32_t sc, Type t, int c, const std::string &l) : startChannel(sc), type(t), channels(c), label(l) {}
    
    
    PWMOutput(uint32_t sc, Type t, int c, const std::string &l, int b, float g) : startChannel(sc), type(t), channels(c), label(l), brightness(b), gamma(g) {}
    PWMOutput(uint32_t sc, Type t, int c, const std::string &l, int minl, int maxl, bool rev, const std::string &z, const std::string &d) : startChannel(sc), type(t), channels(c), label(l), min_limit(minl), max_limit(maxl), reverse(rev), zeroStyle(z), dataType(d) {}

    
    PWMOutput& operator=(const PWMOutput& other) = default;
    PWMOutput& operator=(PWMOutput&& other) = default;

    uint32_t startChannel;
    
    int channels = 2;
    Type type = Type::SERVO;
    std::string label;

    //LED properties
    int brightness = 100;
    float gamma = 1.0;
    
    //SERVO properties
    int min_limit = 1000;
    int max_limit = 2000;
    bool reverse = false;
    std::string zeroStyle = "Hold";
    std::string dataType = "Scaled";    
};
