/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

// StreamSerializingVisitor — stream-based output primitives shared by
// StringSerializingVisitor and FileSerializingVisitor.

#include "StreamSerializingVisitor.h"

// ---------------------------------------------------------------------------
// Constructor — writes the XML declaration.
// ---------------------------------------------------------------------------

StreamSerializingVisitor::StreamSerializingVisitor(std::ostream& os, bool exporting, bool prettyPrint)
    : BaseSerializingVisitor(exporting), out(os), prettyPrint(prettyPrint) {
}

void StreamSerializingVisitor::WriteXmlDeclaration() {
    out << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>";
    WriteNewline();
}

// ---------------------------------------------------------------------------
// Internal stream primitives
// ---------------------------------------------------------------------------

void StreamSerializingVisitor::WriteIndent() {
    if (prettyPrint) {
        for (int i = 0; i < indentLevel; ++i) {
            out << "  ";
        }
    }
}

void StreamSerializingVisitor::WriteNewline() {
    if (prettyPrint) {
        out << '\n';
    }
}

// static
std::string StreamSerializingVisitor::EscapeXml(const std::string& input) {
    std::string result;
    result.reserve(input.size());
    for (char c : input) {
        switch (c) {
        case '&':  result += "&amp;";  break;
        case '<':  result += "&lt;";   break;
        case '>':  result += "&gt;";   break;
        case '"':  result += "&quot;"; break;
        case '\'': result += "&apos;"; break;
        case '\t': result += "&#x9;";   break;
        case '\n': result += "&#xA;";  break;
        case '\r': result += "&#xD;";  break;
        default:   result += c;        break;
        }
    }
    return result;
}

// ---------------------------------------------------------------------------
// WriteOpenTag / WriteCloseTag — the core output primitives
// ---------------------------------------------------------------------------

void StreamSerializingVisitor::WriteOpenTag(const std::string& name,
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
        _tagStack.push_back(name);
    }
}

void StreamSerializingVisitor::WriteCloseTag() {
    if (indentLevel > 0) --indentLevel;
    WriteIndent();
    std::string name = _tagStack.back();
    _tagStack.pop_back();
    out << "</" << name << '>';
    WriteNewline();
}

void StreamSerializingVisitor::WriteBodyText(const std::string& txt) {
    out << EscapeXml(txt);
}
