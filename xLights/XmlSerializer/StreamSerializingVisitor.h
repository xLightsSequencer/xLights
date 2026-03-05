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

// StreamSerializingVisitor is an intermediate base class for serializers that
// write to a std::ostream.  It owns all the stream-based XML writing logic
// (open/close tags, indentation, escaping).  Concrete subclasses provide the
// actual stream: StringSerializingVisitor uses an ostringstream,
// FileSerializingVisitor uses an ofstream or external ostream.

#include "BaseSerializingVisitor.h"

#include <ostream>
#include <string>
#include <vector>

class StreamSerializingVisitor : public BaseSerializingVisitor {
public:
    // Output primitives (write to the wrapped ostream)
    void WriteOpenTag(const std::string& name, const AttrCollector& attrs,
                      bool selfClose = false) override;
    void WriteCloseTag() override;
    void WriteBodyText(const std::string& txt) override;
    void WriteOpenTag(const std::string& name) override {
        AttrCollector attr;
        WriteOpenTag(name, attr);
    }

    // Append pre-formed XML text directly into the output stream.
    // The caller is responsible for correct indentation and newlines.
    void AppendRaw(const std::string& xml);

    int  GetIndentLevel() const { return indentLevel; }
    bool IsPrettyPrint()  const { return prettyPrint; }

    using AttrCollector = BaseSerializingVisitor::AttrCollector;

protected:
    StreamSerializingVisitor(std::ostream& os, bool exporting = false, bool prettyPrint = true);
    ~StreamSerializingVisitor() override = default;

    std::ostream& out;

private:
    bool prettyPrint;
    int  indentLevel = 0;
    std::vector<std::string> _tagStack;

    void WriteIndent();
    void WriteNewline();
    static std::string EscapeXml(const std::string& input);
};
