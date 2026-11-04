#include <map>
#include <string>
#include <utility>
#include <vector>

#include <catch2/catch_test_macros.hpp>

#include "models/SubModelOps.h"

using namespace submodel_ops;
using Strands = std::vector<std::string>;

TEST_CASE("ReverseRow reverses node order and flips ranges", "[models2][submodel-ops]") {
    CHECK(ReverseRow("1-5") == "5-1");
    CHECK(ReverseRow("1-5,10-12") == "12-10,5-1");
    CHECK(ReverseRow("1,2,3") == "3,2,1");
    CHECK(ReverseRow("7") == "7");
    CHECK(ReverseRow("") == "");
    CHECK(ReverseRow("1-5,9") == "9,5-1");
}

TEST_CASE("ReverseRow leaves odd entries untouched", "[models2][submodel-ops]") {
    CHECK(ReverseRow("1-2-3,4") == "4,1-2-3");
    CHECK(ReverseRow("a-b") == "b-a");
    CHECK(ReverseRow(",5") == "5,");
}

TEST_CASE("ReverseRow twice restores the row", "[models2][submodel-ops]") {
    for (const char* row : { "1-5,10-12", "3,2,1", "1-3,7,9-11", "4" }) {
        CHECK(ReverseRow(ReverseRow(row)) == row);
    }
}

TEST_CASE("Strand text conversions between blanks and zeros", "[models2][submodel-ops]") {
    CHECK(BlanksToZeros("1,,3") == "1,0,3");
    CHECK(BlanksToZeros(",,") == "0,0,0");
    CHECK(BlanksToZeros("1,2") == "1,2");
    CHECK(ZerosToBlanks("1,0,3") == "1,,3");
    CHECK(RemoveBlanksAndZeros("1,,0,3,0") == "1,3");
    CHECK(RemoveBlanksAndZeros(",0,") == "");
    CHECK(RemoveBlanksAndZeros("5") == "5");
}

TEST_CASE("Blanks and zeros conversions are inverse for clean rows", "[models2][submodel-ops]") {
    std::string row = "1,,3,,,6";
    CHECK(ZerosToBlanks(BlanksToZeros(row)) == row);
    CHECK(BlanksToZeros(ZerosToBlanks("1,0,3,0,0,6")) == "1,0,3,0,0,6");
}

TEST_CASE("Expand and compress strand text", "[models2][submodel-ops]") {
    CHECK(ExpandStrand("1-4") == "1,2,3,4");
    CHECK(ExpandStrand("5,1-3") == "5,1,2,3");
    CHECK(CompressStrand("1,2,3,5") == "1-3,5");
    CHECK(CompressStrand(ExpandStrand("2-6,9")) == "2-6,9");
    CHECK(ExpandStrand("") == "");
}

TEST_CASE("TransformAllStrands visits every strand once", "[models2][submodel-ops]") {
    Strands s = { "1", "2", "3" };
    std::vector<std::string> order;
    TransformAllStrands(s, [&](const std::string& v) {
        order.push_back(v);
        return v + "x";
    });
    CHECK(s == Strands{ "1x", "2x", "3x" });
    CHECK(order == std::vector<std::string>{ "3", "2", "1" });
}

TEST_CASE("TransformAllStrands on an empty list does nothing", "[models2][submodel-ops]") {
    Strands s;
    int calls = 0;
    TransformAllStrands(s, [&](const std::string& v) {
        ++calls;
        return v;
    });
    CHECK(calls == 0);
    CHECK(s.empty());
}

TEST_CASE("FlipHorizontal reverses every row", "[models2][submodel-ops]") {
    Strands s = { "1-3", "4-6,9" };
    FlipHorizontal(s);
    CHECK(s == Strands{ "3-1", "9,6-4" });
}

TEST_CASE("FlipVertical reverses row order", "[models2][submodel-ops]") {
    Strands s = { "a", "b", "c" };
    FlipVertical(s);
    CHECK(s == Strands{ "c", "b", "a" });
    Strands one = { "1-3" };
    FlipVertical(one);
    CHECK(one == Strands{ "1-3" });
    Strands none;
    FlipVertical(none);
    CHECK(none.empty());
}

TEST_CASE("ReverseNodes renumbers against the node count", "[models2][submodel-ops]") {
    Strands s = { "1-3", "10" };
    ReverseNodes(s, 10);
    CHECK(s == Strands{ "10-8", "1" });
}

TEST_CASE("ReverseNodes drops blanks and text", "[models2][submodel-ops]") {
    Strands s = { "1,,3,x" };
    ReverseNodes(s, 5);
    CHECK(s == Strands{ "5,3" });
}

TEST_CASE("ReverseNodes twice restores the numbers", "[models2][submodel-ops]") {
    Strands s = { "1-3,7", "9,10" };
    Strands original = s;
    ReverseNodes(s, 12);
    ReverseNodes(s, 12);
    CHECK(s == Strands{ "1-3,7", "9-10" });
    (void)original;
}

TEST_CASE("ShiftNodes wraps around the model", "[models2][submodel-ops]") {
    Strands s = { "1-3" };
    ShiftNodes(s, 10, 2);
    CHECK(s == Strands{ "3-5" });
    Strands wrap = { "9-10" };
    ShiftNodes(wrap, 10, 2);
    CHECK(wrap == Strands{ "1-2" });
    Strands back = { "1-2" };
    ShiftNodes(back, 10, -2);
    CHECK(back == Strands{ "9-10" });
}

TEST_CASE("ShiftNodes with zero shift or no nodes is a no-op", "[models2][submodel-ops]") {
    Strands s = { "1-3" };
    ShiftNodes(s, 10, 0);
    CHECK(s == Strands{ "1-3" });
    ShiftNodes(s, 0, 4);
    CHECK(s == Strands{ "1-3" });
    ShiftNodes(s, -5, 4);
    CHECK(s == Strands{ "1-3" });
}

TEST_CASE("ShiftNodes by the node count lands on the same nodes", "[models2][submodel-ops]") {
    Strands s = { "2-4" };
    ShiftNodes(s, 10, 10);
    CHECK(s == Strands{ "2-4" });
}

TEST_CASE("PivotRowsColumns transposes the grid", "[models2][submodel-ops]") {
    Strands s = { "1,2,3", "4,5,6" };
    PivotRowsColumns(s);
    REQUIRE(s.size() == 3);
    CHECK(s[2] == "4,1");
    CHECK(s[1] == "5,2");
    CHECK(s[0] == "6,3");
}

TEST_CASE("PivotRowsColumns pads short rows with blanks", "[models2][submodel-ops]") {
    Strands s = { "1,2,3", "4" };
    PivotRowsColumns(s);
    REQUIRE(s.size() == 3);
    CHECK(s[2] == "4,1");
    CHECK(s[1] == ",2");
    CHECK(s[0] == ",3");
}

TEST_CASE("PivotRowsColumns twice restores a rectangular grid", "[models2][submodel-ops]") {
    Strands s = { "1,2,3", "4,5,6" };
    PivotRowsColumns(s);
    PivotRowsColumns(s);
    CHECK(s == Strands{ "1-3", "4-6" });
    Strands empty;
    PivotRowsColumns(empty);
    CHECK(empty.empty());
}

TEST_CASE("CombineStrands joins rows in display order", "[models2][submodel-ops]") {
    Strands s = { "1-2", "3-4" };
    CombineStrands(s);
    REQUIRE(s.size() == 1);
    CHECK(s[0] == "3-4,1-2");
    Strands none;
    CombineStrands(none);
    CHECK(none.empty());
}

TEST_CASE("CombineStrands compresses adjacent ranges", "[models2][submodel-ops]") {
    Strands s = { "4-6", "1-3" };
    CombineStrands(s);
    CHECK(s == Strands{ "1-6" });
}

TEST_CASE("MakeRowsUniform pads at the front", "[models2][submodel-ops]") {
    Strands s = { "1-4", "5-6" };
    MakeRowsUniform(s, PadMode::Front);
    REQUIRE(s.size() == 2);
    CHECK(s[0] == "1-4");
    CHECK(s[1] == ",,5-6");
}

TEST_CASE("MakeRowsUniform pads at the rear", "[models2][submodel-ops]") {
    Strands s = { "1-4", "5-6" };
    MakeRowsUniform(s, PadMode::Rear);
    CHECK(s[0] == "1-4");
    CHECK(s[1] == "5-6,,");
}

TEST_CASE("MakeRowsUniform distribute spreads nodes evenly", "[models2][submodel-ops]") {
    Strands s = { "1-4", "5,6" };
    MakeRowsUniform(s, PadMode::Distribute);
    CHECK(s[0] == "1-4");
    auto cells = ExpandStrand(s[1]);
    int blanks = 0;
    int nodes = 0;
    std::string cur;
    for (char c : cells + ",") {
        if (c == ',') {
            if (cur.empty()) {
                ++blanks;
            } else {
                ++nodes;
            }
            cur.clear();
        } else {
            cur += c;
        }
    }
    CHECK(nodes == 2);
    CHECK(blanks == 2);
}

TEST_CASE("MakeRowsUniform leaves equal rows alone and tolerates empty input", "[models2][submodel-ops]") {
    for (PadMode mode : { PadMode::Front, PadMode::Rear, PadMode::Distribute }) {
        Strands s = { "1-3", "4-6" };
        MakeRowsUniform(s, mode);
        CHECK(s == Strands{ "1-3", "4-6" });
        Strands none;
        MakeRowsUniform(none, mode);
        CHECK(none.empty());
    }
}

TEST_CASE("RemoveDuplicatesInRow removes later repeats", "[models2][submodel-ops]") {
    Strands s = { "9", "1,2,1,3,2" };
    RemoveDuplicatesInRow(s, 0, false);
    CHECK(s[1] == "1-3");
    CHECK(s[0] == "9");
}

TEST_CASE("RemoveDuplicatesInRow can blank repeats in place", "[models2][submodel-ops]") {
    Strands s = { "1,2,1,3,2" };
    RemoveDuplicatesInRow(s, 0, true);
    CHECK(s[0] == "1-2,,3,");
}

TEST_CASE("RemoveDuplicatesInRow ignores blanks and zeros when comparing", "[models2][submodel-ops]") {
    Strands s = { "1,0,0,2,,," };
    RemoveDuplicatesInRow(s, 0, true);
    CHECK(s[0] == "1,0,0,2,,,");
}

TEST_CASE("RemoveDuplicatesInRow ignores an out of range row", "[models2][submodel-ops]") {
    Strands s = { "1,1" };
    RemoveDuplicatesInRow(s, 5, false);
    RemoveDuplicatesInRow(s, -1, false);
    CHECK(s == Strands{ "1,1" });
}

TEST_CASE("RemoveDuplicatesInRow addresses rows from the top", "[models2][submodel-ops]") {
    Strands s = { "1,1", "2,2" };
    RemoveDuplicatesInRow(s, 0, false);
    CHECK(s[1] == "2");
    CHECK(s[0] == "1,1");
}

TEST_CASE("RemoveAllDuplicates left to right keeps the first column occurrence", "[models2][submodel-ops]") {
    Strands s = { "2,1", "1,2" };
    RemoveAllDuplicates(s, true, true);
    CHECK(s[1] == "1,");
    CHECK(s[0] == "2,");
}

TEST_CASE("RemoveAllDuplicates top to bottom keeps the first row occurrence", "[models2][submodel-ops]") {
    Strands s = { "2,1", "1,2" };
    RemoveAllDuplicates(s, false, true);
    CHECK(s[1] == "1-2");
    CHECK(s[0] == ",");
}

TEST_CASE("RemoveAllDuplicates scan direction changes who wins", "[models2][submodel-ops]") {
    Strands lr = { "1,3", "3,1" };
    RemoveAllDuplicates(lr, true, true);
    Strands tb = { "1,3", "3,1" };
    RemoveAllDuplicates(tb, false, true);
    CHECK(lr[1] == "3,");
    CHECK(lr[0] == "1,");
    CHECK(tb[1] == "3,1");
    CHECK(tb[0] == ",");
}

TEST_CASE("RemoveAllDuplicates without suppress shortens the rows", "[models2][submodel-ops]") {
    Strands s = { "1,2,3", "1,5" };
    RemoveAllDuplicates(s, false, false);
    CHECK(s[1] == "1,5");
    CHECK(s[0] == "2-3");
}

TEST_CASE("RemoveAllDuplicates handles empty and unique grids", "[models2][submodel-ops]") {
    Strands none;
    RemoveAllDuplicates(none, true, false);
    CHECK(none.empty());
    Strands unique = { "1-3", "4-6" };
    RemoveAllDuplicates(unique, true, true);
    CHECK(unique == Strands{ "1-3", "4-6" });
}

TEST_CASE("Export CSV starts with the header", "[models2][submodel-ops]") {
    CHECK(ExportSubModelsCSV({}) == "Name,Type,Vertical Buffer,Buffer Style,Rows Name,Node Ranges\n");
}

TEST_CASE("Export CSV writes node range rows top first", "[models2][submodel-ops]") {
    SubModelSpec sm;
    sm.name = "Spiral";
    sm.isRanges = true;
    sm.vertical = true;
    sm.bufferStyle = "Keep XY";
    sm.strands = { "1-3", "4-6", "7-9" };
    std::string csv = ExportSubModelsCSV({ sm });
    CHECK(csv == "Name,Type,Vertical Buffer,Buffer Style,Rows Name,Node Ranges\n"
                 "Spiral,Node Ranges,true,Keep XY,,\n"
                 ",,,Top,\"7-9\"\n"
                 ",,,Line 2,\"4-6\"\n"
                 ",,,Bottom,\"1-3\"\n");
}

TEST_CASE("Export CSV writes a sub buffer submodel on one row", "[models2][submodel-ops]") {
    SubModelSpec sm;
    sm.name = "Half";
    sm.isRanges = false;
    sm.subBuffer = "0x0x50x100";
    std::string csv = ExportSubModelsCSV({ sm });
    CHECK(csv.find("Half,SubBuffer,false,Default,,\n") != std::string::npos);
    CHECK(csv.find(",,,SubBuffer,\"0x0x50x100\"\n") != std::string::npos);
}

TEST_CASE("Export CSV with a single range row labels it Bottom", "[models2][submodel-ops]") {
    SubModelSpec sm;
    sm.name = "One";
    sm.strands = { "1-9" };
    std::string csv = ExportSubModelsCSV({ sm });
    CHECK(csv.find(",,,Bottom,\"1-9\"\n") != std::string::npos);
    CHECK(csv.find("Top") == std::string::npos);
}

TEST_CASE("Export CSV with several submodels keeps their order", "[models2][submodel-ops]") {
    SubModelSpec a;
    a.name = "A";
    a.strands = { "1" };
    SubModelSpec b;
    b.name = "B";
    b.strands = { "2" };
    std::string csv = ExportSubModelsCSV({ a, b });
    CHECK(csv.find("\nA,") < csv.find("\nB,"));
}

TEST_CASE("Slice type names match the parsed types", "[models2][submodel-ops]") {
    auto names = SliceTypeNames();
    REQUIRE(names.size() == 7);
    for (const auto& n : names) {
        SliceType t;
        CHECK(ParseSliceType(n, t));
    }
    SliceType t;
    CHECK(ParseSliceType("Nodes", t));
    CHECK(t == SliceType::Nodes);
    CHECK(ParseSliceType("Vertical Slices", t));
    CHECK(t == SliceType::VerticalSlices);
    CHECK_FALSE(ParseSliceType("nodes", t));
    CHECK_FALSE(ParseSliceType("", t));
}

TEST_CASE("Vertical slices divide the buffer horizontally into bands", "[models2][submodel-ops]") {
    auto s0 = GenerateSlice(SliceType::VerticalSlices, 0, 4, 100);
    auto s3 = GenerateSlice(SliceType::VerticalSlices, 3, 4, 100);
    CHECK_FALSE(s0.isRanges);
    CHECK(s0.subBuffer == "0.00x0.00x24.75x100.00");
    CHECK(s3.subBuffer == "75.00x0.00x100.00x100.00");
    auto s1 = GenerateSlice(SliceType::VerticalSlices, 1, 4, 100);
    CHECK(s1.subBuffer == "25.00x0.00x49.75x100.00");
}

TEST_CASE("Horizontal slices swap the axes", "[models2][submodel-ops]") {
    auto s0 = GenerateSlice(SliceType::HorizontalSlices, 0, 2, 100);
    auto s1 = GenerateSlice(SliceType::HorizontalSlices, 1, 2, 100);
    CHECK(s0.subBuffer == "0.00x0.00x100.00x49.75");
    CHECK(s1.subBuffer == "0.00x50.00x100.00x100.00");
}

TEST_CASE("Single slice covers the whole buffer", "[models2][submodel-ops]") {
    CHECK(GenerateSlice(SliceType::VerticalSlices, 0, 1, 50).subBuffer == "0.00x0.00x100.00x100.00");
    CHECK(GenerateSlice(SliceType::HorizontalSlices, 0, 1, 50).subBuffer == "0.00x0.00x100.00x100.00");
}

TEST_CASE("Segments split the buffer into a grid", "[models2][submodel-ops]") {
    auto a = GenerateSlice(SliceType::Segments2Wide, 0, 4, 0);
    auto b = GenerateSlice(SliceType::Segments2Wide, 1, 4, 0);
    auto c = GenerateSlice(SliceType::Segments2Wide, 2, 4, 0);
    auto d = GenerateSlice(SliceType::Segments2Wide, 3, 4, 0);
    CHECK(a.subBuffer == "0.00x0.00x49.75x49.75");
    CHECK(b.subBuffer == "50.00x0.00x100.00x49.75");
    CHECK(c.subBuffer == "0.00x50.00x49.75x99.75");
    CHECK(d.subBuffer == "50.00x50.00x100.00x100.00");
}

TEST_CASE("Segments high mirror segments wide", "[models2][submodel-ops]") {
    auto wide = GenerateSlice(SliceType::Segments2Wide, 1, 4, 0);
    auto high = GenerateSlice(SliceType::Segments2High, 1, 4, 0);
    CHECK(wide.subBuffer == "50.00x0.00x100.00x49.75");
    CHECK(high.subBuffer == "0.00x50.00x49.75x100.00");
}

TEST_CASE("Three segment layouts cover the buffer edges", "[models2][submodel-ops]") {
    auto first = GenerateSlice(SliceType::Segments3Wide, 0, 6, 0);
    auto last = GenerateSlice(SliceType::Segments3Wide, 5, 6, 0);
    CHECK(first.subBuffer.rfind("0.00x0.00x", 0) == 0);
    CHECK(last.subBuffer == "66.67x50.00x100.00x100.00");
    auto highLast = GenerateSlice(SliceType::Segments3High, 5, 6, 0);
    CHECK(highLast.subBuffer == "50.00x66.67x100.00x100.00");
}

TEST_CASE("Node slices split the node count into runs", "[models2][submodel-ops]") {
    auto a = GenerateSlice(SliceType::Nodes, 0, 3, 10);
    auto b = GenerateSlice(SliceType::Nodes, 1, 3, 10);
    auto c = GenerateSlice(SliceType::Nodes, 2, 3, 10);
    CHECK(a.isRanges);
    CHECK(a.range == "1-3");
    CHECK(b.range == "4-6");
    CHECK(c.range == "7-10");
}

TEST_CASE("Node slices cover every node exactly once", "[models2][submodel-ops]") {
    for (int nodes : { 7, 10, 25, 100 }) {
        for (int count : { 1, 2, 3, 7 }) {
            if (count > nodes) {
                continue;
            }
            std::vector<int> seen(nodes + 1, 0);
            for (int i = 0; i < count; ++i) {
                auto s = GenerateSlice(SliceType::Nodes, i, count, nodes);
                REQUIRE(s.isRanges);
                int lo = 0, hi = 0;
                auto dash = s.range.find('-');
                if (dash == std::string::npos) {
                    lo = hi = (int)std::strtol(s.range.c_str(), nullptr, 10);
                } else {
                    lo = (int)std::strtol(s.range.substr(0, dash).c_str(), nullptr, 10);
                    hi = (int)std::strtol(s.range.substr(dash + 1).c_str(), nullptr, 10);
                }
                for (int n = lo; n <= hi; ++n) {
                    REQUIRE(n >= 1);
                    REQUIRE(n <= nodes);
                    seen[n]++;
                }
            }
            for (int n = 1; n <= nodes; ++n) {
                INFO(nodes << " nodes " << count << " slices node " << n);
                CHECK(seen[n] == 1);
            }
        }
    }
}

TEST_CASE("Node slice of a single node is written without a dash", "[models2][submodel-ops]") {
    auto s = GenerateSlice(SliceType::Nodes, 1, 2, 2);
    CHECK(s.range == "2");
    auto first = GenerateSlice(SliceType::Nodes, 0, 2, 2);
    CHECK(first.range == "1");
}

TEST_CASE("GenerateSlice rejects impossible requests", "[models2][submodel-ops]") {
    auto none = GenerateSlice(SliceType::Nodes, 0, 0, 10);
    CHECK(none.range.empty());
    CHECK(none.subBuffer.empty());
    CHECK(GenerateSlice(SliceType::VerticalSlices, -1, 3, 10).subBuffer.empty());
    CHECK(GenerateSlice(SliceType::VerticalSlices, 3, 3, 10).subBuffer.empty());
    CHECK(GenerateSlice(SliceType::Nodes, 5, 3, 10).range.empty());
}

TEST_CASE("OrderPoints choices all parse", "[models2][submodel-ops]") {
    auto choices = OrderPointsChoices();
    CHECK(choices.size() == 8 * 6 + 2 * 3);
    for (const auto& c : choices) {
        INFO(c);
        OrderPointsOptions o;
        CHECK(ParseOrderPointsChoice(c, o));
    }
}

TEST_CASE("OrderPoints choice parsing decodes options", "[models2][submodel-ops]") {
    OrderPointsOptions o;
    REQUIRE(ParseOrderPointsChoice("Radial|From Far To Near|Strand Center", o));
    CHECK(o.radial);
    CHECK(o.reverse);
    CHECK(o.center == OrderCenter::Strand);
    REQUIRE(ParseOrderPointsChoice("Radial|From Near To Far|Submodel Center", o));
    CHECK(o.radial);
    CHECK_FALSE(o.reverse);
    CHECK(o.center == OrderCenter::Submodel);
    REQUIRE(ParseOrderPointsChoice("Circumferential|Start From Up|CCW Around Model Center", o));
    CHECK_FALSE(o.radial);
    CHECK(o.reverse);
    CHECK(o.center == OrderCenter::Model);
    REQUIRE(ParseOrderPointsChoice("Circumferential|Start From Up|CW Around Submodel Center", o));
    CHECK_FALSE(o.reverse);
    CHECK(o.center == OrderCenter::Submodel);
}

TEST_CASE("OrderPoints choice parsing rejects garbage", "[models2][submodel-ops]") {
    OrderPointsOptions o;
    CHECK_FALSE(ParseOrderPointsChoice("", o));
    CHECK_FALSE(ParseOrderPointsChoice("Radial", o));
    CHECK_FALSE(ParseOrderPointsChoice("Radial|x|Model Center", o));
    CHECK_FALSE(ParseOrderPointsChoice("Radial|From Near To Far|Nowhere", o));
    CHECK_FALSE(ParseOrderPointsChoice("Spiral|Start From Up|CW Around Model Center", o));
    CHECK_FALSE(ParseOrderPointsChoice("Circumferential|Start From Up|Sideways", o));
    CHECK_FALSE(ParseOrderPointsChoice("a|b|c|d", o));
}

TEST_CASE("OrderPoints radial sorts near to far", "[models2][submodel-ops]") {
    std::map<int, std::pair<float, float>> coords = { { 1, { 5.0f, 0.0f } }, { 2, { 1.0f, 0.0f } }, { 3, { 3.0f, 0.0f } }, { 4, { 0.0f, 0.0f } } };
    Strands s = { "1,2,3" };
    OrderPointsOptions o;
    o.radial = true;
    o.center = OrderCenter::Model;
    OrderPoints(s, coords, o, 0, 0);
    CHECK(ExpandStrand(s[0]) == "3,2,1");
}

TEST_CASE("OrderPoints radial far to near reverses the order", "[models2][submodel-ops]") {
    std::map<int, std::pair<float, float>> coords = { { 1, { 5.0f, 0.0f } }, { 2, { 1.0f, 0.0f } }, { 3, { 3.0f, 0.0f } }, { 4, { 0.0f, 0.0f } } };
    Strands s = { "3,2,1" };
    OrderPointsOptions o;
    o.radial = true;
    o.reverse = true;
    o.center = OrderCenter::Model;
    OrderPoints(s, coords, o, 0, 0);
    CHECK(ExpandStrand(s[0]) == "1,2,3");
}

TEST_CASE("OrderPoints leaves rows outside the range alone", "[models2][submodel-ops]") {
    std::map<int, std::pair<float, float>> coords = { { 1, { 5.0f, 0.0f } }, { 2, { 1.0f, 0.0f } }, { 3, { 3.0f, 0.0f } }, { 4, { 0.0f, 0.0f } } };
    Strands s = { "4,3", "1,2,3" };
    OrderPointsOptions o;
    o.radial = true;
    o.center = OrderCenter::Model;
    OrderPoints(s, coords, o, 0, 0);
    CHECK(s[0] == "4,3");
    CHECK(s[1] != "1,2,3");
}

TEST_CASE("OrderPoints keeps blank cells attached to the node that follows", "[models2][submodel-ops]") {
    std::map<int, std::pair<float, float>> coords = { { 1, { 5.0f, 0.0f } }, { 2, { 1.0f, 0.0f } }, { 3, { 0.0f, 0.0f } } };
    Strands s = { "1,0,2" };
    OrderPointsOptions o;
    o.radial = true;
    o.center = OrderCenter::Model;
    OrderPoints(s, coords, o, 0, 0);
    CHECK(ExpandStrand(s[0]) == ",2,1");
}

TEST_CASE("OrderPoints circumferential orders nodes by angle", "[models2][submodel-ops]") {
    std::map<int, std::pair<float, float>> coords = {
        { 1, { 1.0f, 0.0f } }, { 2, { 0.0f, 1.0f } }, { 3, { -1.0f, 0.0f } }, { 4, { 0.0f, -1.0f } }, { 5, { 0.0f, 0.0f } }
    };
    Strands s = { "1,2,3,4" };
    OrderPointsOptions o;
    o.radial = false;
    o.center = OrderCenter::Strand;
    o.startAngle = 0.0f;
    OrderPoints(s, coords, o, 0, 0);
    Strands rev = { "1,2,3,4" };
    o.reverse = true;
    OrderPoints(rev, coords, o, 0, 0);
    auto a = ExpandStrand(s[0]);
    auto b = ExpandStrand(rev[0]);
    CHECK(a.size() == 7);
    CHECK(b.size() == 7);
    std::string ra(a.rbegin(), a.rend());
    CHECK(ra != a);
    CHECK(a != b);
}

TEST_CASE("OrderPoints tolerates missing coordinates and empty input", "[models2][submodel-ops]") {
    std::map<int, std::pair<float, float>> none;
    Strands s = { "1,2,3" };
    OrderPointsOptions o;
    OrderPoints(s, none, o, 0, 0);
    CHECK_FALSE(s[0].empty());
    Strands empty;
    OrderPoints(empty, none, o, 0, 0);
    CHECK(empty.empty());
    Strands blanks = { ",,," };
    OrderPoints(blanks, none, o, 0, 0);
    CHECK(blanks.size() == 1);
}

TEST_CASE("OrderPoints clamps a row range that runs past the grid", "[models2][submodel-ops]") {
    std::map<int, std::pair<float, float>> coords = { { 1, { 5.0f, 0.0f } }, { 2, { 1.0f, 0.0f } } };
    Strands s = { "1,2" };
    OrderPointsOptions o;
    o.radial = true;
    OrderPoints(s, coords, o, 0, 10);
    OrderPoints(s, coords, o, -3, 0);
    OrderPoints(s, coords, o, 5, 2);
    CHECK_FALSE(s[0].empty());
}

TEST_CASE("ZerosToBlanks keeps leading zeros as blank cells", "[models2][submodel-ops]") {
    CHECK(ZerosToBlanks("0,0,5") == ",,5");
    CHECK(ZerosToBlanks("0,5") == ",5");
    CHECK(ZerosToBlanks("0,0") == ",");
}

TEST_CASE("ZerosToBlanks leaves non-zero cells and single zeros correct", "[models2][submodel-ops]") {
    CHECK(ZerosToBlanks("0") == "");
    CHECK(ZerosToBlanks("1,0,0") == "1,,");
    CHECK(ZerosToBlanks("0,0,0,7") == ",,,7");
}
