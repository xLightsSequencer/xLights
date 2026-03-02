/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

// StringSerializingVisitor — stream-based output overrides only.
// All Visit() and attribute/child-element logic lives in BaseSerializingVisitor.

#include "StringSerializingVisitor.h"

// ---------------------------------------------------------------------------
// Constructor — writes the XML declaration.
// ---------------------------------------------------------------------------

StringSerializingVisitor::StringSerializingVisitor(bool exporting, bool prettyPrint)
    : BaseSerializingVisitor(exporting), prettyPrint(prettyPrint) {
    out << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>";
    WriteNewline();
}

// ---------------------------------------------------------------------------
// Result access
// ---------------------------------------------------------------------------

std::string StringSerializingVisitor::GetResult() const {
    return out.str();
}

void StringSerializingVisitor::AppendRaw(const std::string& xml) {
    out << xml;
}

// ---------------------------------------------------------------------------
// Internal stream primitives
// ---------------------------------------------------------------------------

void StringSerializingVisitor::WriteIndent() {
    if (prettyPrint) {
        for (int i = 0; i < indentLevel; ++i) {
            out << "  ";
        }
    }
}

void StringSerializingVisitor::WriteNewline() {
    if (prettyPrint) {
        out << '\n';
    }
}

// static
std::string StringSerializingVisitor::EscapeXml(const std::string& input) {
    std::string result;
    result.reserve(input.size());
    for (char c : input) {
        switch (c) {
        case '&':  result += "&amp;";  break;
        case '<':  result += "&lt;";   break;
        case '>':  result += "&gt;";   break;
        case '"':  result += "&quot;"; break;
        case '\'': result += "&apos;"; break;
        default:   result += c;        break;
        }
    }
    return result;
}

// ---------------------------------------------------------------------------
// WriteOpenTag / WriteCloseTag — the core output primitives
// ---------------------------------------------------------------------------

void StringSerializingVisitor::WriteOpenTag(const std::string& name,
                                             const AttrCollector& attrs,
                                             bool selfClose) {
    WriteIndent();
    out << '<' << name;
    for (const auto& [key, val] : attrs.attrs) {
        out << ' ' << key << "=\"" << EscapeXml(val) << '"';
    }
    if (selfClose) {
        out << "/>";
        WriteNewline();
    } else {
        out << '>';
        WriteNewline();
        ++indentLevel;
    }
}

void StringSerializingVisitor::WriteCloseTag(const std::string& name) {
    if (indentLevel > 0) --indentLevel;
    WriteIndent();
    out << "</" << name << '>';
    WriteNewline();
}

void StringSerializingVisitor::WriteBodyText(const std::string& txt) {
    AppendRaw(EscapeXml(txt));
}
