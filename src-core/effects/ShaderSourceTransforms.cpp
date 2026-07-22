/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include "ShaderSourceTransforms.h"

#include <array>
#include <cctype>
#include <cstring>
#include <string>
#include <vector>

namespace {

bool isIdentChar(char c) {
    return std::isalnum((unsigned char)c) || c == '_';
}

// GL-compatible pow: negative base with an integral exponent returns
// pow(|x|, y) with the sign of an odd power (GL drivers strength-reduce
// constant integral exponents, so real shaders rely on this working).
// The scalar overload's internal pow always sees a non-negative base.
const char* kXlPowHelpers =
    "float xl_pow(float x, float y) {\n"
    "    if (x >= 0.0 || fract(y) != 0.0) return pow(x, y);\n"
    "    float m = pow(abs(x), y);\n"
    "    return (mod(y, 2.0) != 0.0) ? -m : m;\n"
    "}\n"
    "vec2 xl_pow(vec2 x, vec2 y) { return vec2(xl_pow(x.x, y.x), xl_pow(x.y, y.y)); }\n"
    "vec3 xl_pow(vec3 x, vec3 y) { return vec3(xl_pow(x.x, y.x), xl_pow(x.y, y.y), xl_pow(x.z, y.z)); }\n"
    "vec4 xl_pow(vec4 x, vec4 y) { return vec4(xl_pow(x.x, y.x), xl_pow(x.y, y.y), xl_pow(x.z, y.z), xl_pow(x.w, y.w)); }\n";

// Replace user pow( calls with xl_pow( (word-boundary aware), then inject the
// helpers early in the source — after the ShaderConfig prepend's XL_SHADER
// marker so they precede all user code. Runs before zero-init so the helpers'
// own pow() calls are never rewritten.
std::string injectSafePow(std::string s) {
    bool found = false;
    size_t p = 0;
    while ((p = s.find("pow", p)) != std::string::npos) {
        const bool bLeft = (p == 0) || !isIdentChar(s[p - 1]);
        size_t q = p + 3;
        while (q < s.size() && std::isspace((unsigned char)s[q])) q++;
        const bool bRight = !isIdentChar(s[p + 3]);
        if (bLeft && bRight && q < s.size() && s[q] == '(') {
            s.replace(p, 3, "xl_pow");
            found = true;
            p += 7;
        } else {
            p += 3;
        }
    }
    if (!found) {
        return s;
    }
    static const char* anchors[] = { "#define XL_SHADER\n", "out vec4 fragmentColor;\n" };
    for (const char* a : anchors) {
        size_t pos = s.find(a);
        if (pos != std::string::npos) {
            s.insert(pos + std::strlen(a), kXlPowHelpers);
            return s;
        }
    }
    // No stable anchor (unexpected): prepend after the #version line.
    size_t nl = s.find('\n');
    s.insert(nl == std::string::npos ? 0 : nl + 1, kXlPowHelpers);
    return s;
}

struct TypeZero {
    const char* type;
    const char* zero;
};
const TypeZero kZeroable[] = {
    { "float", "0.0" },
    { "vec2", "vec2(0.0)" },
    { "vec3", "vec3(0.0)" },
    { "vec4", "vec4(0.0)" },
    { "int", "0" },
    { "uint", "0u" },
    { "bool", "false" },
    { "ivec2", "ivec2(0)" },
    { "ivec3", "ivec3(0)" },
    { "ivec4", "ivec4(0)" },
    { "uvec2", "uvec2(0u)" },
    { "uvec3", "uvec3(0u)" },
    { "uvec4", "uvec4(0u)" },
    { "bvec2", "bvec2(false)" },
    { "bvec3", "bvec3(false)" },
    { "bvec4", "bvec4(false)" },
};

// Split a declarator list on top-level commas (commas inside parens/brackets
// belong to initializer expressions like vec2(1, 2)).
std::vector<std::string> splitDeclarators(const std::string& s) {
    std::vector<std::string> out;
    int depth = 0;
    size_t start = 0;
    for (size_t i = 0; i < s.size(); i++) {
        char c = s[i];
        if (c == '(' || c == '[') depth++;
        else if (c == ')' || c == ']') depth--;
        else if (c == ',' && depth == 0) {
            out.push_back(s.substr(start, i - start));
            start = i + 1;
        }
    }
    out.push_back(s.substr(start));
    return out;
}

std::string trim(const std::string& s) {
    size_t a = s.find_first_not_of(" \t\r");
    if (a == std::string::npos) return {};
    size_t b = s.find_last_not_of(" \t\r");
    return s.substr(a, b - a + 1);
}

// Add "= <zero>" to uninitialized local scalar/vector declarations inside
// function bodies. Brace-depth and comment aware; skips struct bodies, for(...)
// headers (not at statement start), arrays and consts.
std::string zeroInitLocals(const std::string& src) {
    std::string out;
    out.reserve(src.size() + 256);
    bool inBlockComment = false;
    int depth = 0;
    int structDepth = -1; // depth at which a struct body opened, -1 = none

    size_t lineStart = 0;
    while (lineStart <= src.size()) {
        size_t lineEnd = src.find('\n', lineStart);
        std::string line = src.substr(lineStart, (lineEnd == std::string::npos ? src.size() : lineEnd) - lineStart);
        const size_t nextStart = (lineEnd == std::string::npos) ? src.size() + 1 : lineEnd + 1;

        // Compute the comment-stripped view for analysis and brace counting.
        std::string code;
        code.reserve(line.size());
        for (size_t i = 0; i < line.size(); i++) {
            if (inBlockComment) {
                if (line[i] == '*' && i + 1 < line.size() && line[i + 1] == '/') {
                    inBlockComment = false;
                    i++;
                }
                continue;
            }
            if (line[i] == '/' && i + 1 < line.size() && line[i + 1] == '*') {
                inBlockComment = true;
                i++;
                continue;
            }
            if (line[i] == '/' && i + 1 < line.size() && line[i + 1] == '/') {
                break;
            }
            code += line[i];
        }

        const int depthAtLineStart = depth;
        for (char c : code) {
            if (c == '{') {
                depth++;
            } else if (c == '}') {
                depth--;
                if (structDepth >= 0 && depth < structDepth) structDepth = -1;
            }
        }
        if (code.find("struct") != std::string::npos && structDepth < 0 && code.find('{') != std::string::npos) {
            structDepth = depthAtLineStart + 1;
        }

        // Only rewrite simple single-line declaration statements inside a
        // function body (not struct members, not globals, not for-headers).
        std::string t = trim(code);
        bool rewritten = false;
        if (depthAtLineStart >= 1 && structDepth < 0 && !t.empty() && t.back() == ';' &&
            t.find('{') == std::string::npos && t.find("const ") == std::string::npos) {
            for (const auto& tz : kZeroable) {
                const size_t tl = std::strlen(tz.type);
                if (t.compare(0, tl, tz.type) == 0 && t.size() > tl && std::isspace((unsigned char)t[tl])) {
                    std::string decls = trim(t.substr(tl, t.size() - tl - 1)); // between type and ';'
                    if (decls.find('[') != std::string::npos || decls.empty()) break; // arrays: leave alone
                    auto parts = splitDeclarators(decls);
                    bool any = false;
                    std::string rebuilt;
                    bool valid = true;
                    for (size_t i = 0; i < parts.size(); i++) {
                        std::string d = trim(parts[i]);
                        // declarator must start with an identifier
                        if (d.empty() || !(std::isalpha((unsigned char)d[0]) || d[0] == '_')) { valid = false; break; }
                        if (d.find('=') == std::string::npos) {
                            // bare name only (defensive: skip anything odd)
                            for (char c : d) {
                                if (!isIdentChar(c)) { valid = false; break; }
                            }
                            if (!valid) break;
                            d += std::string(" = ") + tz.zero;
                            any = true;
                        }
                        if (i) rebuilt += ", ";
                        rebuilt += d;
                    }
                    if (valid && any) {
                        // preserve the original line's leading whitespace
                        size_t indent = line.find_first_not_of(" \t");
                        std::string lead = (indent == std::string::npos) ? "" : line.substr(0, indent);
                        line = lead + tz.type + " " + rebuilt + ";";
                        rewritten = true;
                    }
                    break;
                }
            }
        }
        (void)rewritten;

        out += line;
        if (lineEnd != std::string::npos) out += '\n';
        lineStart = nextStart;
    }
    return out;
}

} // namespace

namespace ShaderSourceTransforms {

std::string Apply(const std::string& assembledFragment) {
    return zeroInitLocals(injectSafePow(assembledFragment));
}

} // namespace ShaderSourceTransforms
