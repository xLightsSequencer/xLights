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

// StringSerializingVisitor serializes model/object data directly to an
// ostringstream (i.e., to a std::string) without building a wxXmlNode DOM.
// All Visit() method implementations live in BaseSerializingVisitor; this
// class only provides the stream-based WriteOpenTag / WriteCloseTag overrides.

#include "BaseSerializingVisitor.h"

#include <sstream>
#include <string>

struct StringSerializingVisitor : BaseSerializingVisitor {
    explicit StringSerializingVisitor(bool exporting = false, bool prettyPrint = true);

    // Output primitives (write to ostringstream)
    void WriteOpenTag(const std::string& name, const AttrCollector& attrs,
                      bool selfClose = false) override;
    void WriteCloseTag(const std::string& name) override;
    void WriteBodyText(const std::string& txt) override;
    void WriteOpenTag(const std::string& name) override {
        AttrCollector attr;
        WriteOpenTag(name, attr);
    }

    // Retrieve the accumulated XML string (includes the leading declaration).
    [[nodiscard]] std::string GetResult() const;

    // Append pre-formed XML text directly into the output stream (for use by
    // StringSerializer helpers that write model groups and dimensions).
    // The caller is responsible for correct indentation and newlines.
    void AppendRaw(const std::string& xml);

    int  GetIndentLevel() const { return indentLevel; }
    bool IsPrettyPrint()  const { return prettyPrint; }

    // AttrCollector is inherited from BaseSerializingVisitor.
    // Provide a using-declaration so that existing code that refers to
    // StringSerializingVisitor::AttrCollector continues to compile unchanged.
    using AttrCollector = BaseSerializingVisitor::AttrCollector;

private:
    std::ostringstream out;
    bool prettyPrint;
    int  indentLevel = 0;

    void WriteIndent();
    void WriteNewline();
    static std::string EscapeXml(const std::string& input);
};
