#include <list>
#include <functional>
#include <optional>
#include <catch2/catch_test_macros.hpp>
#include <pugixml.hpp>
#include <iostream>
#include "models/ModelManager.h"
#include "models/Model.h"
#include "outputs/OutputManager.h"
#include "XmlSerializer/XmlSerializer.h"
TEST_CASE("scratch", "[scratch]") {
    OutputManager om; ModelManager mm(&om);
    pugi::xml_document doc;
    doc.load_string(R"(<model name="S" DisplayAs="Single Line" NumStrings="1" NodesPerString="10" StartChannel="@S:5"/>)");
    Model* m = mm.createAndAddModel(doc.first_child(), 1,1);
    for (int i=0;i<4;i++){ mm.RecalcStartChannels(); std::cout << "first=" << m->GetFirstChannel() << " could=" << m->CouldComputeStartChannel << "\n"; }
    for (const char* s : {":", "::", "#:", "@:", ">", "!", "!:", "#::", "@:1", " : "}) { m->ModelStartChannel = s; std::cout << "'" << s << "' valid=" << m->IsValidStartChannelString() << " num=" << m->GetNumberFromChannelString(s) << "\n"; }
}
