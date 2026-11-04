#include <list>
#include <functional>
#include <optional>
#include <sstream>
#include <catch2/catch_test_macros.hpp>
#include <pugixml.hpp>
#include <iostream>
#include "models/ModelManager.h"
#include "models/Model.h"
#include "models/ModelGroup.h"
#include "models/SubModel.h"
#include "models/CustomModel.h"
#include "outputs/OutputManager.h"
#include "XmlSerializer/XmlSerializer.h"
struct Fx {
    OutputManager om;
    ModelManager mm{&om};
    Model* Add(const std::string& xml) {
        pugi::xml_document doc;
        REQUIRE(doc.load_string(xml.c_str()));
        return mm.createAndAddModel(doc.first_child(), 100, 100);
    }
};
void P(Model* m) {
    std::cout << m->GetName() << ": nodes=" << m->GetNodeCount() << " chans=" << m->GetChanCount() << " cpn=" << m->GetChanCountPerNode() << " strings=" << m->GetNumStrings() << " strands=" << m->GetNumStrands() << " W=" << m->GetDefaultBufferWi() << " H=" << m->GetDefaultBufferHt() << " nps=" << m->NodesPerString() << " multi=" << m->IsMultiCoordsPerNode()<< " first=" << m->GetFirstChannel() << " last=" << m->GetLastChannel() << "\n";
}

void D(Model* m){
    std::cout << m->GetName() << " nodes=" << m->GetNodeCount() << " W=" << m->GetDefaultBufferWi() << " H=" << m->GetDefaultBufferHt() << " strands=" << m->GetNumStrands() << "\n";
    for (uint32_t i=0;i<m->GetNodeCount() && i<40;++i){ auto* n=m->GetNode(i); std::cout << " n"<<i<<" act="<<n->ActChan<<" ("; for(uint32_t c=0;c<n->Coords.size();++c) std::cout<<n->Coords[c].bufX<<","<<n->Coords[c].bufY<<" "; std::cout << ")";}
    std::cout << "\n";
}


TEST_CASE("scratch", "[scratch]") {
    Fx f;
    std::string type = getenv("XT"), attr = getenv("XA"), v = getenv("XV");
    std::string x = "<model name=\"H\" DisplayAs=\"" + type + "\" NumStrings=\"2\" NodesPerString=\"4\" StrandsPerString=\"1\" NumArches=\"2\" NodesPerArch=\"3\" NumCanes=\"2\" NodesPerCane=\"3\" StarPoints=\"5\" NodesPerArm=\"3\" ArmsPerString=\"3\" TopNodes=\"3\" SideNodes=\"3\" BottomNodes=\"3\" CubeWidth=\"2\" CubeHeight=\"2\" CubeDepth=\"2\" CustomWidth=\"2\" CustomHeight=\"2\" NumChannels=\"3\" StartChannel=\"1\" " + (attr == "ALL" ? std::string("") : attr + "=\"" + v + "\"") + "/>";
    if (attr != "ALL") { pugi::xml_document d; d.load_string(x.c_str()); std::istringstream is(attr); std::string tok; while (std::getline(is, tok, ',')) d.first_child().attribute(tok.c_str()).set_value(v.c_str()); std::ostringstream os; d.save(os); x = os.str(); }
    if (attr == "ALL") { pugi::xml_document d; d.load_string(x.c_str()); for (auto a : d.first_child().attributes()) { std::string n = a.name(); if (n != "name" && n != "DisplayAs" && n != "StartChannel") a.set_value(v.c_str()); } std::ostringstream os; d.save(os); x = os.str(); }
    Model* m = f.Add(x);
    if (!m) { std::cout << "null\n"; return; }
    std::cout << m->GetNodeCount() << " " << m->GetChanCount() << " " << m->GetDefaultBufferWi() << "x" << m->GetDefaultBufferHt() << "\n";
}
