#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>

#include "import_export/MusicCatalog.h"
#include "import_export/VendorCatalog.h"
#include "support/TempDir.h"

#include <pugixml.hpp>

#include <climits>
#include <cstdlib>
#include <string>
#include <vector>

namespace {

pugi::xml_document Parse(const std::string& xml) {
    pugi::xml_document d;
    d.load_string(xml.c_str());
    return d;
}

std::string Inv(const std::string& body) {
    return "<modelinventory>" + body + "</modelinventory>";
}

std::string ModelXml(const std::string& inner) {
    return "<model>" + inner + "</model>";
}

std::string MusicInv(const std::string& body) {
    return "<musicinventory>" + body + "</musicinventory>";
}

std::string Song(const std::string& title, const std::string& extra = "") {
    return "<song><title>" + title + "</title>" + extra + "</song>";
}

using vendor_catalog::Model;

std::string Pad(std::string s, size_t w = 18) {
    while (s.size() < w) s += " ";
    return s;
}

} // namespace

// ==========================================================================
// vendor_catalog::Model::InterpretSize
// ==========================================================================

TEST_CASE("InterpretSize understands millimetres", "[impexp2][catalog]") {
    auto [in, expected] = GENERATE(table<std::string, int>({
        { "10mm", 10 },
        { "10.5mm", 10 },
        { "0mm", 0 },
        { "5.mm", 5 },
        { "10 mm", 10 },
        { " 10mm ", 10 },
        { "1 0 m m", 10 },
        { "(30mm)", 30 },
        { "Height (45.7mm)", 45 },
        { "12 inch (305mm)", 305 },
        { "1000mm", 1000 },
        { "007mm", 7 },
    }));
    CAPTURE(in);
    CHECK(Model::InterpretSize(in) == expected);
}

TEST_CASE("InterpretSize understands centimetres", "[impexp2][catalog]") {
    auto [in, expected] = GENERATE(table<std::string, int>({
        { "30cm", 300 },
        { "30 cm", 300 },
        { "30.5cm", 305 },
        { "(30cm)", 300 },
        { "Width (12.5cm)", 125 },
    }));
    CAPTURE(in);
    CHECK(Model::InterpretSize(in) == expected);
}

TEST_CASE("InterpretSize treats bare numbers as inches", "[impexp2][catalog]") {
    auto [in, expected] = GENERATE(table<std::string, int>({
        { "10", 254 },
        { "1", 25 },
        { "2.5", 63 },
        { "100", 2540 },
        { " 4 ", 101 },
    }));
    CAPTURE(in);
    CHECK(std::abs(Model::InterpretSize(in) - expected) <= 1);
}

TEST_CASE("InterpretSize understands feet and inches", "[impexp2][catalog]") {
    auto [in, expected] = GENERATE(table<std::string, int>({
        { "5'2\"", 1574 },
        { "5' 2\"", 1574 },
        { "0'10\"", 254 },
        { "6'0\"", 1828 },
        { "5Feet", 1524 },
        { "5 feet", 1524 },
        { "2.5 Feet", 762 },
        { "37/47\"", 1193 },
        { "47/37\"", 1193 },
        { "10/10.5\"", 266 },
        { "19\"", 482 },
        { "2.5\"", 63 },
        { "0\"", 0 },
    }));
    CAPTURE(in);
    CHECK(std::abs(Model::InterpretSize(in) - expected) <= 1);
}

TEST_CASE("InterpretSize returns -1 for things it cannot read", "[impexp2][catalog]") {
    auto in = GENERATE(as<std::string>{}, "", " ", "abc", "mm", "cm", "ft", "5 ft", "-5mm", "-5", "1e3mm", ".5mm", ".5", "1,5mm", "10 meters", "10mmm", "10 mm tall", "10㎜", "five", "5'", "'2\"", "5'2", "//\"", "1/2", "1/2/3\"", "0Feet", "5ft2in", "N/A", "TBD", "~10mm");
    CAPTURE(in);
    int r = Model::InterpretSize(in);
    // a few of these are accepted by looser branches; none may crash, and unreadable ones give -1
    if (in == "0Feet") {
        CHECK(r == -1);
    } else if (in == "//\"") {
        CHECK(r <= 0);
    } else {
        CHECK(r == -1);
    }
}

TEST_CASE("InterpretSize inch quote form accepts trailing junk", "[impexp2][catalog]") {
    CHECK(Model::InterpretSize("12\" tall") == 304);
    CHECK(Model::InterpretSize("12\"(30cm)") >= 300);
    CHECK(Model::InterpretSize("0\"(0cm)") == 0);
}

TEST_CASE("InterpretSize with a very long digit string", "[impexp2][catalog]") {
    std::string digits(5000, '1');
    // strtod gives inf and the double->int cast is undefined
    int r = Model::InterpretSize(digits + "mm");
    CHECK((r == -1 || r > 0));
}

TEST_CASE("InterpretSize with huge numbers does not overflow int", "[impexp2][catalog]") {
    // static_cast<int>(double) of an out-of-range value is undefined (INT_MIN on x86).
    auto in = GENERATE(as<std::string>{}, "99999999999mm", "99999999999cm", "99999999999", "99999999999\"", "99999999999Feet", "99999999999'2\"");
    CAPTURE(in);
    int r = Model::InterpretSize(in);
    CHECK((r == -1 || r > 0));
}

// ==========================================================================
// vendor_catalog::Model / ModelWiring parsing
// ==========================================================================

TEST_CASE("Vendor catalog Model reads every field", "[impexp2][catalog]") {
    auto doc = Parse(ModelXml("<id>7</id><categoryid>1</categoryid><categoryid>2</categoryid><name>Tree</name><type>Mega</type><material>Steel</material>"
                              "<thickness>3mm</thickness><width>30cm</width><height>2.5\"</height><depth>10mm</depth><pixelcount>100</pixelcount>"
                              "<pixelspacing>2</pixelspacing><pixeldescription>WS2811</pixeldescription><notes>n</notes><weblink>http://x</weblink>"
                              "<imagefile>http://i/1.png</imagefile><imagefile></imagefile><imagefile>http://i/2.png</imagefile>"));
    Model m(doc.document_element(), nullptr);
    CHECK(m._id == "7");
    CHECK(m._categoryIds == std::list<std::string>{ "1", "2" });
    CHECK(m._name == "Tree");
    CHECK(m._type == "Mega");
    CHECK(m._material == "Steel");
    CHECK(m._thickness == "3mm");
    CHECK(m._width == "30cm");
    CHECK(m._height == "2.5\"");
    CHECK(m._depth == "10mm");
    CHECK(m._pixelCount == "100");
    CHECK(m._pixelSpacing == "2");
    CHECK(m._pixelDescription == "WS2811");
    CHECK(m._notes == "n");
    CHECK(m._webpage == "http://x");
    CHECK(m._images == std::list<std::string>{ "http://i/1.png", "http://i/2.png" });
    CHECK(m._wiring.empty());
    CHECK(m._vendor == nullptr);
}

TEST_CASE("Vendor catalog Model element names are case insensitive and unknown ones ignored", "[impexp2][catalog]") {
    auto doc = Parse(ModelXml("<ID>3</ID><NAME>X</NAME><Weblink>w</Weblink><bogus>1</bogus><!-- comment --><type>T</type>"));
    Model m(doc.document_element(), nullptr);
    CHECK(m._id == "3");
    CHECK(m._name == "X");
    CHECK(m._webpage == "w");
    CHECK(m._type == "T");
}

TEST_CASE("Vendor catalog Model with an empty element", "[impexp2][catalog]") {
    auto doc = Parse("<model/>");
    Model m(doc.document_element(), nullptr);
    CHECK(m._name.empty());
    CHECK(m._wiring.empty());
    CHECK(m.GetDescription().empty());
    CHECK_FALSE(m.HasDownloadableXmodel());
    CHECK_FALSE(m.InCategory(""));
}

TEST_CASE("Vendor catalog Model repeated scalar elements keep the last", "[impexp2][catalog]") {
    auto doc = Parse(ModelXml("<name>First</name><name>Second</name><id>1</id><id>2</id>"));
    Model m(doc.document_element(), nullptr);
    CHECK(m._name == "Second");
    CHECK(m._id == "2");
}

TEST_CASE("Vendor catalog Model wiring inherits dimensions and model images", "[impexp2][catalog]") {
    auto doc = Parse(ModelXml("<width>30cm</width><height>10mm</height><depth>junk</depth><imagefile>http://m/1.png</imagefile>"
                              "<wiring><name>W1</name><description>D1</description><xmodellink>http://x/w1.xmodel</xmodellink><imagefile>http://w/1.png</imagefile></wiring>"
                              "<wiring><name>W2</name></wiring>"));
    Model m(doc.document_element(), nullptr);
    REQUIRE(m._wiring.size() == 2);
    auto* w1 = m._wiring.front();
    auto* w2 = m._wiring.back();
    CHECK(w1->_name == "W1");
    CHECK(w1->_wiringDescription == "D1");
    CHECK(w1->_xmodelLink == "http://x/w1.xmodel");
    CHECK(w1->HasDownloadableXmodel());
    CHECK_FALSE(w2->HasDownloadableXmodel());
    CHECK(w1->GetWidthMM() == 300);
    CHECK(w1->GetHeightMM() == 10);
    CHECK(w1->GetDepthMM() == -1);
    CHECK(w1->_model == &m);
    CHECK(w1->_images == std::list<std::string>{ "http://w/1.png", "http://m/1.png" });
    CHECK(w2->_images == std::list<std::string>{ "http://m/1.png" });
    CHECK(m.HasDownloadableXmodel());
}

TEST_CASE("Vendor catalog wiring parsing ignores comments and unknown elements", "[impexp2][catalog]") {
    auto doc = Parse("<wiring><!--c--><NAME>N</NAME><junk/><XmodelLink></XmodelLink><imagefile/></wiring>");
    vendor_catalog::ModelWiring w(doc.document_element(), nullptr, 1, 2, 3);
    CHECK(w._name == "N");
    CHECK(w._xmodelLink.empty());
    CHECK_FALSE(w.HasDownloadableXmodel());
    CHECK(w._images.empty());
    CHECK(w.GetWidthMM() == 1);
    CHECK(w.GetHeightMM() == 2);
    CHECK(w.GetDepthMM() == 3);
}

TEST_CASE("Vendor catalog wiring AddImages appends", "[impexp2][catalog]") {
    auto doc = Parse("<wiring><imagefile>a</imagefile></wiring>");
    vendor_catalog::ModelWiring w(doc.document_element(), nullptr, -1, -1, -1);
    w.AddImages({ "b", "c" });
    w.AddImages({});
    CHECK(w._images == std::list<std::string>{ "a", "b", "c" });
}

TEST_CASE("Vendor catalog wiring download helpers are no-ops without work to do", "[impexp2][catalog]") {
    auto doc = Parse("<wiring><name>N</name></wiring>");
    vendor_catalog::ModelWiring w(doc.document_element(), nullptr, -1, -1, -1);
    w.DownloadImages();
    CHECK(w._imageFiles.empty());
    w.DownloadXModel();
    CHECK(w._xmodelFile.empty());
}

TEST_CASE("Vendor catalog wiring DownloadXModel keeps an existing cached file", "[impexp2][catalog]") {
    TempDir d;
    auto doc = Parse("<wiring><xmodellink>http://invalid.invalid/a.xmodel</xmodellink></wiring>");
    vendor_catalog::ModelWiring w(doc.document_element(), nullptr, -1, -1, -1);
    w._xmodelFile = d.Write("a.xmodel", "<model/>");
    w.DownloadXModel();
    CHECK(w._xmodelFile == d.File("a.xmodel"));
}

TEST_CASE("Vendor catalog Model categories", "[impexp2][catalog]") {
    auto doc = Parse(ModelXml("<categoryid>a</categoryid><categoryid>b</categoryid><categoryid></categoryid>"));
    Model m(doc.document_element(), nullptr);
    CHECK(m.InCategory("a"));
    CHECK(m.InCategory("b"));
    CHECK(m.InCategory(""));
    CHECK_FALSE(m.InCategory("A"));
    CHECK_FALSE(m.InCategory("c"));
    CHECK_FALSE(m.InCategory("ab"));
}

TEST_CASE("Vendor catalog Model description layout", "[impexp2][catalog]") {
    auto doc = Parse(ModelXml("<name>Tree</name><type>Mega</type><width>30cm</width><pixelcount>100</pixelcount><notes>Note</notes>"));
    Model m(doc.document_element(), nullptr);
    CHECK(m.GetDescription() == Pad("Name:") + "Tree\n\n" + Pad("Type:") + "Mega\n" + Pad("Width:") + "30cm\n" + Pad("Pixel Count:") + "100\n\nNote\n");
}

TEST_CASE("Vendor catalog wiring description combines model and wiring", "[impexp2][catalog]") {
    auto doc = Parse(ModelXml("<name>Tree</name><wiring><name>Option A</name><description>Wire it up</description></wiring><wiring/>"));
    Model m(doc.document_element(), nullptr);
    REQUIRE(m._wiring.size() == 2);
    auto d1 = m._wiring.front()->GetDescription();
    CHECK(d1.find(Pad("Name:") + "Tree") == 0);
    CHECK(d1.find("Wiring Option: Option A\n\nWire it up") != std::string::npos);
    auto d2 = m._wiring.back()->GetDescription();
    CHECK(d2.find("Wiring Option") == std::string::npos);
    vendor_catalog::ModelWiring orphan(Parse("<w><name>Z</name></w>").document_element(), nullptr, -1, -1, -1);
    CHECK(orphan.GetDescription() == "Wiring Option: Z\n\n");
}

// ==========================================================================
// vendor_catalog::Category / Vendor
// ==========================================================================

TEST_CASE("Vendor catalog category tree and paths", "[impexp2][catalog]") {
    auto doc = Parse(Inv("<categories><category><id>1</id><name>Trees</name><categories><category><id>2</id><name>Mega</name><categories><category><id>3</id><name>Giant</name></category></categories></category></categories></category>"
                         "<category><id>4</id><name>Stars</name></category></categories>"));
    vendor_catalog::Vendor v(doc, -1);
    REQUIRE(v._categories.size() == 2);
    auto* trees = v._categories.front();
    CHECK(trees->_id == "1");
    CHECK(trees->GetPath() == "Trees");
    REQUIRE(trees->_categories.size() == 1);
    auto* mega = trees->_categories.front();
    CHECK(mega->GetPath() == "Trees/Mega");
    CHECK(mega->_parent == trees);
    CHECK(mega->_vendor == &v);
    REQUIRE(mega->_categories.size() == 1);
    CHECK(mega->_categories.front()->GetPath() == "Trees/Mega/Giant");
    CHECK(v._categories.back()->GetPath() == "Stars");
}

TEST_CASE("Vendor catalog category oddities", "[impexp2][catalog]") {
    SECTION("category without name has an empty path segment") {
        auto doc = Parse(Inv("<categories><category><id>1</id><categories><category><id>2</id><name>Kid</name></category></categories></category></categories>"));
        vendor_catalog::Vendor v(doc, -1);
        REQUIRE(v._categories.size() == 1);
        CHECK(v._categories.front()->_categories.front()->GetPath() == "/Kid");
    }
    SECTION("non category children of categories are ignored") {
        auto doc = Parse(Inv("<categories><junk/><category><id>1</id><name>A</name></category><!--c--></categories>"));
        vendor_catalog::Vendor v(doc, -1);
        CHECK(v._categories.size() == 1);
    }
    SECTION("empty categories element") {
        auto doc = Parse(Inv("<categories/>"));
        vendor_catalog::Vendor v(doc, -1);
        CHECK(v._categories.empty());
    }
    SECTION("name containing slashes") {
        auto doc = Parse(Inv("<categories><category><name>A/B</name><categories><category><name>C</name></category></categories></category></categories>"));
        vendor_catalog::Vendor v(doc, -1);
        CHECK(v._categories.front()->_categories.front()->GetPath() == "A/B/C");
    }
}

TEST_CASE("Vendor catalog very deep category nesting", "[impexp2][catalog]") {
    std::string xml = "<categories>";
    const int depth = 500;
    for (int i = 0; i < depth; ++i) xml += "<category><name>c" + std::to_string(i) + "</name><categories>";
    for (int i = 0; i < depth; ++i) xml += "</categories></category>";
    xml += "</categories>";
    auto doc = Parse(Inv(xml));
    vendor_catalog::Vendor v(doc, -1);
    REQUIRE(v._categories.size() == 1);
    vendor_catalog::Category* c = v._categories.front();
    int n = 1;
    while (!c->_categories.empty()) {
        c = c->_categories.front();
        ++n;
    }
    CHECK(n == depth);
    CHECK(c->GetPath().find("c0/c1/c2/") == 0);
}

TEST_CASE("Vendor catalog reads vendor details", "[impexp2][catalog]") {
    auto doc = Parse(Inv("<vendor><name>Acme</name><contact>Bob</contact><email>b@a.com</email><phone>555</phone><website>http://a</website>"
                         "<facebook>fb</facebook><twitter>tw</twitter><notes>Some notes</notes><junk>x</junk></vendor>"));
    vendor_catalog::Vendor v(doc, 5);
    CHECK(v._name == "Acme");
    CHECK(v._contact == "Bob");
    CHECK(v._email == "b@a.com");
    CHECK(v._phone == "555");
    CHECK(v._website == "http://a");
    CHECK(v._facebook == "fb");
    CHECK(v._twitter == "tw");
    CHECK(v._notes == "Some notes");
    CHECK(v._maxModels == 5);
    CHECK(v._logoFile.empty());
}

TEST_CASE("Vendor catalog ignores documents with the wrong root or no root", "[impexp2][catalog]") {
    auto root = GENERATE(as<std::string>{}, "<musicinventory><vendor><name>X</name></vendor></musicinventory>", "<xrgb/>", "<modelinventory2/>", "", "not xml at all");
    auto doc = Parse(root);
    vendor_catalog::Vendor v(doc, -1);
    CAPTURE(root);
    CHECK(v._name.empty());
    CHECK(v._models.empty());
    CHECK(v._categories.empty());
}

TEST_CASE("Vendor catalog root name is case insensitive", "[impexp2][catalog]") {
    auto doc = Parse("<ModelInventory><vendor><name>X</name></vendor></ModelInventory>");
    vendor_catalog::Vendor v(doc, -1);
    CHECK(v._name == "X");
}

TEST_CASE("Vendor catalog suppressed vendor only has a name", "[impexp2][catalog]") {
    vendor_catalog::Vendor v("Hidden");
    CHECK(v._name == "Hidden");
    CHECK(v._models.empty());
    CHECK(v._maxModels == -1);
    CHECK(v.GetDescription() == "Name:    Hidden\n\n");
}

TEST_CASE("Vendor catalog model count limit", "[impexp2][catalog]") {
    std::string models = "<models>";
    for (int i = 0; i < 5; ++i) models += ModelXml("<name>M" + std::to_string(i) + "</name>");
    models += "<notamodel/></models>";
    auto [limit, expected] = GENERATE(table<int, size_t>({
        { -1, 5 }, { 0, 5 }, { -100, 5 }, { 1, 1 }, { 2, 2 }, { 5, 5 }, { 6, 5 }, { INT_MAX, 5 },
    }));
    auto doc = Parse(Inv(models));
    vendor_catalog::Vendor v(doc, limit);
    CAPTURE(limit);
    CHECK(v._models.size() == expected);
    if (limit == 2) {
        CHECK(v._models.front()->_name == "M0");
        CHECK(v._models.back()->_name == "M1");
    }
}

TEST_CASE("Vendor catalog keeps model order and sets the back pointer", "[impexp2][catalog]") {
    auto doc = Parse(Inv("<models>" + ModelXml("<name>Z</name>") + ModelXml("<name>A</name>") + "</models>"));
    vendor_catalog::Vendor v(doc, -1);
    REQUIRE(v._models.size() == 2);
    CHECK(v._models.front()->_name == "Z");
    CHECK(v._models.front()->_vendor == &v);
}

TEST_CASE("Vendor catalog GetModels filters by category", "[impexp2][catalog]") {
    auto doc = Parse(Inv("<models>" + ModelXml("<name>A</name><categoryid>1</categoryid>") + ModelXml("<name>B</name><categoryid>2</categoryid>") +
                         ModelXml("<name>C</name><categoryid>1</categoryid><categoryid>2</categoryid>") + ModelXml("<name>D</name>") + "</models>"));
    vendor_catalog::Vendor v(doc, -1);
    auto one = v.GetModels("1");
    REQUIRE(one.size() == 2);
    CHECK(one.front()->_name == "A");
    CHECK(one.back()->_name == "C");
    CHECK(v.GetModels("2").size() == 2);
    CHECK(v.GetModels("3").empty());
    CHECK(v.GetModels("").empty());
}

TEST_CASE("Vendor catalog sections may appear in any order and repeat", "[impexp2][catalog]") {
    auto doc = Parse(Inv("<models>" + ModelXml("<name>A</name>") + "</models><categories><category><name>C1</name></category></categories>"
                         "<vendor><name>V1</name></vendor><models>" + ModelXml("<name>B</name>") + "</models><vendor><name>V2</name></vendor><categories><category><name>C2</name></category></categories>"));
    vendor_catalog::Vendor v(doc, -1);
    CHECK(v._name == "V2");
    CHECK(v._models.size() == 2);
    CHECK(v._categories.size() == 2);
}

TEST_CASE("Vendor catalog models inside the vendor element are ignored", "[impexp2][catalog]") {
    auto doc = Parse(Inv("<vendor><name>V</name><models>" + ModelXml("<name>Hidden</name>") + "</models><categories/></vendor>"));
    vendor_catalog::Vendor v(doc, -1);
    CHECK(v._models.empty());
}

TEST_CASE("Vendor catalog description", "[impexp2][catalog]") {
    auto doc = Parse(Inv("<vendor><name>Acme</name><contact>Bob</contact><phone>1</phone><email>e</email><twitter>t</twitter><notes>N</notes><website>w</website></vendor>"));
    vendor_catalog::Vendor v(doc, -1);
    CHECK(v.GetDescription() == "Name:    Acme\n\nContact: Bob\nPhone:   1\nEmail:   e\nTwitter: t\n\nN\n");
}

TEST_CASE("Vendor catalog large inventory", "[impexp2][catalog]") {
    std::string models = "<models>";
    for (int i = 0; i < 3000; ++i) models += ModelXml("<name>M" + std::to_string(i) + "</name><width>" + std::to_string(i) + "mm</width><wiring><name>w</name></wiring>");
    models += "</models>";
    auto doc = Parse(Inv(models));
    vendor_catalog::Vendor v(doc, -1);
    CHECK(v._models.size() == 3000);
    CHECK(v._models.back()->_wiring.front()->GetWidthMM() == 2999);
}

TEST_CASE("Vendor catalog empty catalog object", "[impexp2][catalog]") {
    vendor_catalog::Catalog c;
    CHECK(c.Vendors().empty());
}

// ==========================================================================
// music_catalog
// ==========================================================================

TEST_CASE("Music catalog item fields", "[impexp2][catalog]") {
    auto doc = Parse("<song><hash>h1</hash><hash>h2</hash><categoryid>1</categoryid><title>T</title><creator>C</creator><artist>A</artist><notes>N</notes>"
                     "<sequence/><weblink>w</weblink><video>v</video><music>m</music><download>http://h/x.zip</download><junk/></song>");
    music_catalog::Item it(doc.document_element(), nullptr);
    CHECK(it._hashes == std::list<std::string>{ "h1", "h2" });
    CHECK(it._categoryIds == std::list<std::string>{ "1" });
    CHECK(it._title == "T");
    CHECK(it._creator == "C");
    CHECK(it._artist == "A");
    CHECK(it._notes == "N");
    CHECK(it._type == music_catalog::ItemType::Sequence);
    CHECK(it._webpage == "w");
    CHECK(it._video == "v");
    CHECK(it._music == "m");
    CHECK(it._download == "http://h/x.zip");
    CHECK(it.GetTypeName() == "Sequence");
}

TEST_CASE("Music catalog item type defaults and last marker wins", "[impexp2][catalog]") {
    auto check = [](const std::string& xml, music_catalog::ItemType t) {
        auto doc = Parse(xml);
        music_catalog::Item it(doc.document_element(), nullptr);
        CHECK(it._type == t);
    };
    check("<song/>", music_catalog::ItemType::Unknown);
    check("<song><lyric/></song>", music_catalog::ItemType::Lyric);
    check("<song><sequence/><lyric/></song>", music_catalog::ItemType::Lyric);
    check("<song><lyric/><sequence/></song>", music_catalog::ItemType::Sequence);
    check("<song><SEQUENCE/></song>", music_catalog::ItemType::Sequence);
}

TEST_CASE("Music catalog GetExt", "[impexp2][catalog]") {
    auto [type, url, expected] = GENERATE(table<std::string, std::string, std::string>({
        { "<lyric/>", "http://h/x.zip", "xtiming" },
        { "<sequence/>", "http://h/x.mp3", "zip" },
        { "", "http://h/x.mp3", "mp3" },
        { "", "http://h/dir.d/x", "" },
        { "", "http://h/x", "" },
        { "", "", "" },
        { "", "x.tar.gz", "gz" },
        { "", "http://h/x.", "" },
    }));
    auto doc = Parse("<song>" + type + "<download>" + url + "</download></song>");
    music_catalog::Item it(doc.document_element(), nullptr);
    CAPTURE(type, url);
    CHECK(it.GetExt() == expected);
}

TEST_CASE("Music catalog GetExt ignores dots in the host name and query", "[impexp2][catalog]") {
    // MusicCatalog.cpp Item::GetExt takes everything after the last '.' even if it is the host's TLD or a query string.
    auto [url, expected] = GENERATE(table<std::string, std::string>({
        { "http://host.com", "" },
        { "http://h/x.zip?dl=1", "zip" },
        { "http://h/x.mp3#frag", "mp3" },
    }));
    auto doc = Parse("<song><download>" + url + "</download></song>");
    music_catalog::Item it(doc.document_element(), nullptr);
    CAPTURE(url);
    CHECK(it.GetExt() == expected);
}

TEST_CASE("Music catalog GetDownloadFileName", "[impexp2][catalog]") {
    auto [type, url, title, expected] = GENERATE(table<std::string, std::string, std::string, std::string>({
        { "<sequence/>", "http://h/dir/file.zip", "T", "file.zip" },
        { "<sequence/>", "http://h/dir/file.zip?x=1", "T", "file.zip" },
        { "<sequence/>", "http://h/file.zip?u=a/b", "T", "file.zip" },
        { "<sequence/>", "file.zip", "T", "file.zip" },
        { "<sequence/>", "http://h/", "Song", "Song.zip" },
        { "<sequence/>", "http://h/dir/", "Song", "Song.zip" },
        { "<lyric/>", "", "Song", "Song.xtiming" },
        { "", "", "Song", "Song" },
        { "", "", "", "" },
        { "<sequence/>", "?only=query", "Song", "Song.zip" },
        { "<sequence/>", "", "", ".zip" },
        { "<sequence/>", "http://h/a%20b.zip", "T", "a%20b.zip" },
    }));
    auto doc = Parse("<song>" + type + "<title>" + title + "</title><download>" + url + "</download></song>");
    music_catalog::Item it(doc.document_element(), nullptr);
    CAPTURE(type, url, title);
    CHECK(it.GetDownloadFileName() == expected);
}

TEST_CASE("Music catalog file name from a title never contains path separators", "[impexp2][catalog]") {
    // Item::GetDownloadFileName falls back to the raw title, so "AC/DC" or "../x" yields a path, not a file name.
    auto title = GENERATE(as<std::string>{}, "AC/DC", "../../evil", "back\\slash");
    auto doc = Parse("<song><sequence/><title>" + title + "</title></song>");
    music_catalog::Item it(doc.document_element(), nullptr);
    auto n = it.GetDownloadFileName();
    CAPTURE(title, n);
    CHECK(n.find('/') == std::string::npos);
    CHECK(n.find('\\') == std::string::npos);
}

TEST_CASE("Music catalog description", "[impexp2][catalog]") {
    auto doc = Parse("<song><lyric/><title>Jingle</title><artist>Santa</artist><creator>Elf</creator><notes>a\\nb\\n\\nc\\n</notes></song>");
    music_catalog::Item it(doc.document_element(), nullptr);
    CHECK(it.GetDescription() == Pad("Song:") + "Jingle\n\n" + Pad("Artist:") + "Santa\n\n" + Pad("Type:") + "Lyrics\n" + Pad("Creator:") + "Elf\n\na\nb\n\nc\n\n");
}

TEST_CASE("Music catalog description of an empty item", "[impexp2][catalog]") {
    auto doc = Parse("<song/>");
    music_catalog::Item it(doc.document_element(), nullptr);
    CHECK(it.GetDescription() == Pad("Type:") + "Unknown\n");
}

TEST_CASE("Music catalog description handles many literal newline markers", "[impexp2][catalog]") {
    std::string notes;
    for (int i = 0; i < 10000; ++i) notes += "x\\n";
    auto doc = Parse("<song><notes>" + notes + "</notes></song>");
    music_catalog::Item it(doc.document_element(), nullptr);
    auto d = it.GetDescription();
    CHECK(d.find("\\n") == std::string::npos);
    CHECK(std::count(d.begin(), d.end(), '\n') >= 10000);
}

TEST_CASE("Music catalog description keeps a lone backslash and an escaped backslash", "[impexp2][catalog]") {
    auto doc = Parse("<song><notes>a\\b c\\\\n</notes></song>");
    music_catalog::Item it(doc.document_element(), nullptr);
    auto d = it.GetDescription();
    CHECK(d.find("a\\b") != std::string::npos);
    CHECK(d.find("c\\\n") != std::string::npos);
}

TEST_CASE("Music catalog categories", "[impexp2][catalog]") {
    auto doc = Parse(MusicInv("<categories><category><id>1</id><name>Pop</name><categories><category><id>2</id><name>Rock</name></category></categories></category></categories>"));
    music_catalog::Vendor v(doc, -1);
    REQUIRE(v._categories.size() == 1);
    CHECK(v._categories.front()->_categories.front()->GetPath() == "Pop/Rock");
    CHECK(v._categories.front()->_categories.front()->_parent == v._categories.front());
    CHECK(v._categories.front()->_vendor == &v);
}

TEST_CASE("Music catalog sorts songs case-insensitively and stably", "[impexp2][catalog]") {
    auto doc = Parse(MusicInv("<music>" + Song("b", "<artist>1</artist>") + Song("B", "<artist>2</artist>") + Song("a") + Song("") + Song("Ä") + "</music>"));
    music_catalog::Vendor v(doc, -1);
    std::vector<std::string> titles;
    for (auto* i : v._items) titles.push_back(i->_title + ":" + i->_artist);
    REQUIRE(titles.size() == 5);
    CHECK(titles[0] == ":");
    CHECK(titles[1] == "a:");
    CHECK(titles[2] == "b:1");
    CHECK(titles[3] == "B:2");
    CHECK(titles[4] == "Ä:");
}

TEST_CASE("Music catalog item count limit applies before sorting", "[impexp2][catalog]") {
    auto [limit, expected] = GENERATE(table<int, size_t>({ { -1, 4 }, { 0, 4 }, { -9, 4 }, { 1, 1 }, { 3, 3 }, { 4, 4 }, { 100, 4 }, { INT_MAX, 4 } }));
    auto doc = Parse(MusicInv("<music>" + Song("d") + Song("c") + Song("b") + Song("a") + "</music>"));
    music_catalog::Vendor v(doc, limit);
    CAPTURE(limit);
    CHECK(v._items.size() == expected);
    if (limit == 1) CHECK(v._items.front()->_title == "d");
    if (limit == 3) {
        CHECK(v._items.front()->_title == "b");
        CHECK(v._items.back()->_title == "d");
    }
}

TEST_CASE("Music catalog vendor details and wrong roots", "[impexp2][catalog]") {
    SECTION("details") {
        auto doc = Parse(MusicInv("<vendor><name>Acme</name><contact>Bob</contact><email>e</email><phone>p</phone><website>w</website><facebook>f</facebook><twitter>t</twitter><notes>n</notes><x/></vendor>"));
        music_catalog::Vendor v(doc, 7);
        CHECK(v._name == "Acme");
        CHECK(v._facebook == "f");
        CHECK(v._notes == "n");
        CHECK(v._maxItems == 7);
    }
    SECTION("model inventory is not a music inventory") {
        auto doc = Parse(Inv("<vendor><name>X</name></vendor>"));
        music_catalog::Vendor v(doc, -1);
        CHECK(v._name.empty());
    }
    SECTION("non song children in music are skipped") {
        auto doc = Parse(MusicInv("<music><junk/><!--c-->" + Song("a") + "</music>"));
        music_catalog::Vendor v(doc, -1);
        CHECK(v._items.size() == 1);
    }
    SECTION("garbage") {
        auto doc = Parse("garbage");
        music_catalog::Vendor v(doc, -1);
        CHECK(v._items.empty());
    }
    SECTION("suppressed style vendor") {
        music_catalog::Vendor v("Name");
        CHECK(v._name == "Name");
        CHECK(v._maxItems == -1);
    }
}

TEST_CASE("Music catalog vendor description expands literal newlines", "[impexp2][catalog]") {
    auto doc = Parse(MusicInv("<vendor><name>Acme</name><notes>one\\ntwo</notes></vendor>"));
    music_catalog::Vendor v(doc, -1);
    CHECK(v.GetDescription() == "Name:    Acme\n\n\none\ntwo\n");
}

TEST_CASE("Music catalog many songs", "[impexp2][catalog]") {
    std::string songs = "<music>";
    for (int i = 0; i < 4000; ++i) songs += Song("song" + std::to_string(4000 - i));
    songs += "</music>";
    auto doc = Parse(MusicInv(songs));
    music_catalog::Vendor v(doc, -1);
    CHECK(v._items.size() == 4000);
    CHECK(v._items.front()->_title == "song1");
}

TEST_CASE("Music catalog item category membership", "[impexp2][catalog]") {
    auto doc = Parse("<song><categoryid>1</categoryid><categoryid></categoryid></song>");
    music_catalog::Item it(doc.document_element(), nullptr);
    CHECK(it.InCategory("1"));
    CHECK(it.InCategory(""));
    CHECK_FALSE(it.InCategory("2"));
}

TEST_CASE("Music catalog download target path joining", "[impexp2][catalog]") {
    TempDir d;
    d.Write("have.zip", "x");
    auto base = d.path.string();
    SECTION("no trailing separator") {
        auto got = music_catalog::Catalog::DownloadTo("http://invalid.invalid/x", base, "have.zip");
        CHECK(got == base + "/have.zip");
    }
    SECTION("trailing slash is not doubled") {
        auto got = music_catalog::Catalog::DownloadTo("http://invalid.invalid/x", base + "/", "have.zip");
        CHECK(got == base + "/have.zip");
    }
}

TEST_CASE("Music catalog empty catalog object", "[impexp2][catalog]") {
    music_catalog::Catalog c;
    CHECK(c.Vendors().empty());
}
