#include <catch2/catch_test_macros.hpp>

#include <algorithm>
#include <filesystem>
#include <map>
#include <memory>
#include <string>
#include <vector>

#include "ai/IServiceSettingsStore.h"
#include "ai/GenericClient.h"
#include "ai/ServiceManager.h"
#include "ai/aiBase.h"
#include "ai/aiType.h"
#include "ai/chatGPT.h"
#include "ai/claude.h"
#include "ai/gemini.h"
#include "ai/ollama.h"
#include "support/TempDir.h"

namespace {

class MemStore : public IServiceSettingsStore {
public:
    std::map<std::string, std::string> strings;
    std::map<std::string, int> ints;
    std::map<std::string, bool> bools;
    std::map<std::string, std::string> secrets;

    std::string getString(std::string_view k, const std::string& d) const override {
        auto it = strings.find(std::string(k));
        return it == strings.end() ? d : it->second;
    }
    int getInt(std::string_view k, int d) const override {
        auto it = ints.find(std::string(k));
        return it == ints.end() ? d : it->second;
    }
    bool getBool(std::string_view k, bool d) const override {
        auto it = bools.find(std::string(k));
        return it == bools.end() ? d : it->second;
    }
    void setString(std::string_view k, const std::string& v) override { strings[std::string(k)] = v; }
    void setInt(std::string_view k, int v) override { ints[std::string(k)] = v; }
    void setBool(std::string_view k, bool v) override { bools[std::string(k)] = v; }
    std::string getSecret(std::string_view k) const override {
        auto it = secrets.find(std::string(k));
        return it == secrets.end() ? std::string() : it->second;
    }
    void setSecret(std::string_view k, const std::string& v) override { secrets[std::string(k)] = v; }
};

class FakeService : public aiBase {
public:
    std::string name;
    std::list<aiType::TYPE> types;
    int loads = 0;
    FakeService(ServiceManager* sm, std::string n, std::list<aiType::TYPE> t) : aiBase(sm), name(std::move(n)), types(std::move(t)) {}
    void SaveSettings() const override {}
    void LoadSettings() override { ++loads; }
    std::string GetLLMName() const override { return name; }
    bool IsAvailable() const override { return true; }
    std::list<aiType::TYPE> GetTypes() const override { return types; }
    std::pair<std::string, bool> CallLLM(const std::string& p) const override { return { "echo:" + p, true }; }
    std::vector<ServiceProperty> GetProperties() const override { return {}; }
};

const ServiceProperty* Find(const std::vector<ServiceProperty>& v, const std::string& id) {
    for (auto const& p : v) {
        if (p.id == id) {
            return &p;
        }
    }
    return nullptr;
}

} // namespace

TEST_CASE("aiType names and settings suffixes", "[misc][ai]") {
    CHECK(std::string(aiType::TypeName(aiType::PROMPT)) == "Prompt");
    CHECK(std::string(aiType::TypeName(aiType::COLORPALETTES)) == "Color Palette");
    CHECK(std::string(aiType::TypeName(aiType::IMAGES)) == "Images");
    CHECK(std::string(aiType::TypeName(aiType::MAPPING)) == "Mapping");
    CHECK(std::string(aiType::TypeName(aiType::SPEECH2TEXT)) == "Speech to Text");
    CHECK(std::string(aiType::TypeName((aiType::TYPE)99)) == "Unknown");
    CHECK(std::string(aiType::TypeSettingsSuffix(aiType::COLORPALETTES)) == "ColorPalette");
    CHECK(std::string(aiType::TypeSettingsSuffix(aiType::SPEECH2TEXT)) == "Speech2Text");
    CHECK(std::string(aiType::TypeSettingsSuffix((aiType::TYPE)-1)) == "Unknown");
}

TEST_CASE("aiBase enabled types are tracked per capability", "[misc][ai]") {
    FakeService s(nullptr, "Fake", { aiType::PROMPT, aiType::IMAGES });
    CHECK_FALSE(s.IsEnabled());
    CHECK_FALSE(s.IsEnabledForType(aiType::PROMPT));
    s.SetEnabledForType(aiType::PROMPT, true);
    CHECK(s.IsEnabled());
    CHECK(s.IsEnabledForType(aiType::PROMPT));
    CHECK_FALSE(s.IsEnabledForType(aiType::IMAGES));
    s.SetEnabledForType(aiType::PROMPT, true);
    s.SetEnabledForType(aiType::PROMPT, false);
    CHECK_FALSE(s.IsEnabled());
    s.SetEnabledForType(aiType::MAPPING, false);
    CHECK_FALSE(s.IsEnabled());
}

TEST_CASE("aiBase defaults are inert", "[misc][ai]") {
    FakeService s(nullptr, "Fake", { aiType::PROMPT });
    CHECK_FALSE(s.SupportsModelListing());
    CHECK(s.GetAvailableModels().empty());
    CHECK(s.createAIImageGenerator() == nullptr);
    CHECK(s.GenerateColorPalette("x").colors.empty());
    CHECK(s.GenerateModelMapping({}, {}, {}).mappings.empty());
    CHECK(s.GenerateLyricTrack("nofile.wav").lyrics.empty());
    s.SetProperty("a", true);
    s.SetProperty("a", 1);
    s.SetProperty("a", std::string("b"));
    CHECK(s.TestLLM().first == "echo:Hello");
}

TEST_CASE("ServiceManager registers the built in services", "[misc][ai]") {
    MemStore store;
    ServiceManager sm(&store);
    for (auto n : { "ChatGPT", "Claude", "ollama", "Gemini", "genericClient" }) {
        INFO(n);
        CHECK(sm.getService(n) != nullptr);
    }
    CHECK(sm.getService("nothing") == nullptr);
    CHECK(sm.getService("") == nullptr);
    CHECK(sm.getServices().size() >= 5);
}

TEST_CASE("ServiceManager service lookup ignores case", "[misc][ai]") {
    MemStore store;
    ServiceManager sm(&store);
    auto* a = sm.getService("claude");
    auto* b = sm.getService("CLAUDE");
    REQUIRE(a != nullptr);
    CHECK(a == b);
    CHECK(a->GetLLMName() == "Claude");
}

TEST_CASE("ServiceManager settings are prefixed in the store", "[misc][ai]") {
    MemStore store;
    ServiceManager sm(&store);
    sm.setServiceSetting("Key", std::string("v"));
    sm.setServiceSetting("N", 5);
    sm.setServiceSetting("B", true);
    sm.setSecretServiceToken("Tok", "secret");
    CHECK(store.strings.at("xLightsServiceSettingsKey") == "v");
    CHECK(store.ints.at("xLightsServiceSettingsN") == 5);
    CHECK(store.bools.at("xLightsServiceSettingsB") == true);
    CHECK(store.secrets.at("xLightsServiceSettingsTok") == "secret");
    CHECK(sm.getServiceSetting("Key", std::string("d")) == "v");
    CHECK(sm.getServiceSetting("N", 0) == 5);
    CHECK(sm.getServiceSetting("B", false));
    CHECK(sm.getSecretServiceToken("Tok") == "secret");
    CHECK(sm.getServiceSetting("Missing", std::string("d")) == "d");
    CHECK(sm.getServiceSetting("Missing", 3) == 3);
    CHECK(sm.getSecretServiceToken("Missing").empty());
}

TEST_CASE("ServiceManager without a store returns defaults and drops writes", "[misc][ai]") {
    ServiceManager sm(nullptr);
    sm.setServiceSetting("k", std::string("v"));
    sm.setServiceSetting("k", 1);
    sm.setServiceSetting("k", true);
    sm.setSecretServiceToken("k", "t");
    CHECK(sm.getServiceSetting("k", std::string("d")) == "d");
    CHECK(sm.getServiceSetting("k", 4) == 4);
    CHECK(sm.getServiceSetting("k", true));
    CHECK(sm.getSecretServiceToken("k").empty());
    CHECK(sm.findService(aiType::PROMPT) == nullptr);
}

TEST_CASE("ServiceManager addService loads settings and finds enabled services", "[misc][ai]") {
    MemStore store;
    ServiceManager sm(&store);
    auto fake = std::make_unique<FakeService>(&sm, "FakeSvc", std::list<aiType::TYPE>{ aiType::SPEECH2TEXT });
    auto* raw = fake.get();
    sm.addService(std::move(fake));
    CHECK(raw->loads == 1);
    CHECK(sm.getService("fakesvc") == raw);
    CHECK(sm.findService(aiType::SPEECH2TEXT) != raw);
    raw->SetEnabledForType(aiType::SPEECH2TEXT, true);
    auto all = sm.findServices(aiType::SPEECH2TEXT);
    CHECK(std::find(all.begin(), all.end(), (aiBase*)raw) != all.end());
    CHECK(sm.findService(aiType::SPEECH2TEXT) != nullptr);
}

TEST_CASE("ServiceManager findServices only returns services with the type enabled", "[misc][ai]") {
    MemStore store;
    ServiceManager sm(&store);
    CHECK(sm.findServices(aiType::MAPPING).empty());
    auto* claude = sm.getService("Claude");
    REQUIRE(claude != nullptr);
    claude->SetEnabledForType(aiType::MAPPING, true);
    auto m = sm.findServices(aiType::MAPPING);
    REQUIRE(m.size() == 1);
    CHECK(m[0] == claude);
    CHECK(sm.findServices(aiType::PROMPT).empty());
    claude->SetEnabledForType(aiType::PROMPT, true);
    CHECK(sm.findService(aiType::PROMPT) == claude);
}

TEST_CASE("ServiceManager loads saved settings into the services at start", "[misc][ai]") {
    MemStore store;
    store.secrets["xLightsServiceSettingsClaudeApiKey"] = "sk-test";
    store.strings["xLightsServiceSettingsClaudeModel"] = "claude-test";
    store.bools["xLightsServiceSettingsClaudeEnable_Prompt"] = true;
    ServiceManager sm(&store);
    auto* c = sm.getService("Claude");
    REQUIRE(c != nullptr);
    CHECK(c->IsEnabledForType(aiType::PROMPT));
    CHECK_FALSE(c->IsEnabledForType(aiType::MAPPING));
    CHECK(c->IsAvailable());
    auto props = c->GetProperties();
    auto* key = Find(props, "Claude.Key");
    auto* model = Find(props, "Claude.Model");
    REQUIRE(key != nullptr);
    REQUIRE(model != nullptr);
    CHECK(std::get<std::string>(key->value) == "sk-test");
    CHECK(std::get<std::string>(model->value) == "claude-test");
}

TEST_CASE("ServiceManager migrates the old single enable flag to every type", "[misc][ai]") {
    MemStore store;
    store.bools["xLightsServiceSettingsClaudeEnable"] = true;
    store.bools["xLightsServiceSettingsClaudeEnable_Mapping"] = false;
    ServiceManager sm(&store);
    auto* c = sm.getService("Claude");
    REQUIRE(c != nullptr);
    CHECK(c->IsEnabledForType(aiType::PROMPT));
    CHECK(c->IsEnabledForType(aiType::COLORPALETTES));
    CHECK_FALSE(c->IsEnabledForType(aiType::MAPPING));
}

TEST_CASE("ServiceManager survives a missing or unreadable plugin directory", "[misc][ai]") {
    TempDir t;
    MemStore s1;
    ServiceManager a(&s1, t.File("no_such_dir"));
    CHECK(a.getServices().size() >= 5);
    MemStore s2;
    ServiceManager b(&s2, t.path.string());
    CHECK(b.getServices().size() == a.getServices().size());
}

TEST_CASE("ServiceManager ignores files in the plugin folder that are not plugins", "[misc][ai]") {
    TempDir t;
#if defined(_WIN32)
    const char* ext = ".dll";
#elif defined(__APPLE__)
    const char* ext = ".dylib";
#else
    const char* ext = ".so";
#endif
    t.Write(std::string("AIPlugin_broken") + ext, "this is not a shared library");
    t.Write(std::string("other") + ext, "ignored by name");
    t.Write("AIPlugin_readme.txt", "ignored by extension");
    std::filesystem::create_directories(t.path / (std::string("AIPlugin_dir") + ext));
    MemStore store;
    ServiceManager plain(&store);
    MemStore store2;
    ServiceManager sm(&store2, t.path.string());
    CHECK(sm.getServices().size() == plain.getServices().size());
    sm.loadPlugins(t.File("again_missing"));
    CHECK(sm.getServices().size() == plain.getServices().size());
}

TEST_CASE("Claude properties expose enable flags key and model", "[misc][ai]") {
    MemStore store;
    ServiceManager sm(&store);
    auto* c = sm.getService("Claude");
    REQUIRE(c != nullptr);
    auto props = c->GetProperties();
    REQUIRE(props.size() >= 4);
    CHECK(props.front().kind == ServiceProperty::Kind::Category);
    auto* k = Find(props, "Claude.Key");
    REQUIRE(k != nullptr);
    CHECK(k->kind == ServiceProperty::Kind::Secret);
    auto* e = Find(props, "Claude.Enable_Prompt");
    REQUIRE(e != nullptr);
    CHECK(e->kind == ServiceProperty::Kind::Bool);
    CHECK(e->label == "Enable Prompt");
    CHECK(std::get<bool>(e->value) == false);
    CHECK(Find(props, "Claude.Enable_ColorPalette") != nullptr);
    CHECK(Find(props, "Claude.Enable_Mapping") != nullptr);
    CHECK(Find(props, "Claude.Enable_Images") == nullptr);
}

TEST_CASE("Claude SetProperty updates state and ignores unknown ids", "[misc][ai]") {
    MemStore store;
    ServiceManager sm(&store);
    auto* c = sm.getService("Claude");
    REQUIRE(c != nullptr);
    CHECK_FALSE(c->IsAvailable());
    c->SetProperty("Claude.Key", std::string("abc"));
    c->SetProperty("Claude.Enable_Prompt", true);
    CHECK(c->IsAvailable());
    c->SetProperty("Claude.Model", std::string("m1"));
    c->SetProperty("Claude.Bogus", std::string("x"));
    c->SetProperty("Claude.Enable_Bogus", true);
    c->SetProperty("Other.Key", std::string("zzz"));
    auto props = c->GetProperties();
    CHECK(std::get<std::string>(Find(props, "Claude.Key")->value) == "abc");
    CHECK(std::get<std::string>(Find(props, "Claude.Model")->value) == "m1");
    CHECK(std::get<bool>(Find(props, "Claude.Enable_Prompt")->value));
}

TEST_CASE("Claude settings round trip through the store", "[misc][ai]") {
    MemStore store;
    {
        ServiceManager sm(&store);
        auto* c = sm.getService("Claude");
        c->SetProperty("Claude.Key", std::string("key1"));
        c->SetProperty("Claude.Model", std::string("model1"));
        c->SetProperty("Claude.Enable_Mapping", true);
        c->SaveSettings();
    }
    CHECK(store.secrets.at("xLightsServiceSettingsClaudeApiKey") == "key1");
    CHECK(store.strings.at("xLightsServiceSettingsClaudeModel") == "model1");
    CHECK(store.bools.at("xLightsServiceSettingsClaudeEnable_Mapping"));
    CHECK_FALSE(store.bools.at("xLightsServiceSettingsClaudeEnable_Prompt"));
    ServiceManager again(&store);
    auto* c2 = again.getService("Claude");
    CHECK(c2->IsEnabledForType(aiType::MAPPING));
    CHECK_FALSE(c2->IsEnabledForType(aiType::PROMPT));
    CHECK(c2->IsAvailable());
}

TEST_CASE("Claude without a key reports errors without any network use", "[misc][ai]") {
    MemStore store;
    ServiceManager sm(&store);
    auto* c = sm.getService("Claude");
    REQUIRE(c != nullptr);
    auto r = c->CallLLM("hello");
    CHECK_FALSE(r.second);
    CHECK(r.first == "Claude: API Key is empty");
    auto t = c->TestLLM();
    CHECK_FALSE(t.second);
    auto pal = c->GenerateColorPalette("sunset");
    CHECK(pal.colors.empty());
    CHECK_FALSE(pal.error.empty());
    aiBase::MappingModelInfo m;
    m.name = "Tree";
    auto map = c->GenerateModelMapping({ m }, { m }, {});
    CHECK(map.mappings.empty());
    CHECK_FALSE(map.error.empty());
}

TEST_CASE("ChatGPT properties and key handling", "[misc][ai]") {
    MemStore store;
    ServiceManager sm(&store);
    auto* c = sm.getService("ChatGPT");
    REQUIRE(c != nullptr);
    auto props = c->GetProperties();
    auto* tok = Find(props, "ChatGPT.Token");
    REQUIRE(tok != nullptr);
    CHECK(tok->kind == ServiceProperty::Kind::Secret);
    auto* model = Find(props, "ChatGPT.Model");
    REQUIRE(model != nullptr);
    CHECK(model->kind == ServiceProperty::Kind::String);
    CHECK(std::get<std::string>(model->value) == "gpt-4o-mini");
    CHECK_FALSE(c->IsAvailable());
    c->SetProperty("ChatGPT.Token", std::string("sk"));
    c->SetProperty("ChatGPT.Enable_Prompt", true);
    CHECK(c->IsAvailable());
    CHECK(c->SupportsModelListing());
    c->SetProperty("ChatGPT.Model", std::string("gpt-x"));
    c->SetProperty("ChatGPT.ImgModel", std::string("img-x"));
    props = c->GetProperties();
    CHECK(std::get<std::string>(Find(props, "ChatGPT.Model")->value) == "gpt-x");
    CHECK(std::get<std::string>(Find(props, "ChatGPT.ImgModel")->value) == "img-x");
}

TEST_CASE("ChatGPT without a token fails before any request", "[misc][ai]") {
    MemStore store;
    ServiceManager sm(&store);
    auto* c = sm.getService("ChatGPT");
    REQUIRE(c != nullptr);
    auto r = c->CallLLM("hi");
    CHECK_FALSE(r.second);
    CHECK(r.first == "ChatGPT: Bearer Token is empty");
}

TEST_CASE("Gemini offers image generation only", "[misc][ai]") {
    MemStore store;
    ServiceManager sm(&store);
    auto* g = sm.getService("Gemini");
    REQUIRE(g != nullptr);
    auto types = g->GetTypes();
    REQUIRE(types.size() == 1);
    CHECK(types.front() == aiType::IMAGES);
    auto props = g->GetProperties();
    CHECK(Find(props, "Gemini.Key") != nullptr);
    CHECK(Find(props, "Gemini.Model") != nullptr);
    CHECK(Find(props, "Gemini.Enable_Images") != nullptr);
    CHECK_FALSE(g->IsAvailable());
    g->SetProperty("Gemini.Key", std::string("k"));
    CHECK_FALSE(g->IsAvailable());
    g->SetProperty("Gemini.Enable_Images", true);
    CHECK(g->IsAvailable());
}

TEST_CASE("ollama defaults to a local server and needs only a host", "[misc][ai]") {
    MemStore store;
    ServiceManager sm(&store);
    auto* o = sm.getService("ollama");
    REQUIRE(o != nullptr);
    auto props = o->GetProperties();
    auto* host = Find(props, "ollama.Host");
    auto* port = Find(props, "ollama.Port");
    auto* https = Find(props, "ollama.Https");
    auto* model = Find(props, "ollama.Model");
    REQUIRE(host != nullptr);
    REQUIRE(port != nullptr);
    REQUIRE(https != nullptr);
    REQUIRE(model != nullptr);
    CHECK(std::get<std::string>(host->value) == "localhost");
    CHECK(std::get<int>(port->value) == 11434);
    CHECK(std::get<bool>(https->value) == false);
    CHECK(model->kind == ServiceProperty::Kind::String);
    CHECK(port->kind == ServiceProperty::Kind::Int);
    CHECK_FALSE(o->IsAvailable());
    o->SetProperty("ollama.Enable_Prompt", true);
    CHECK(o->IsAvailable());
    o->SetProperty("ollama.Host", std::string(""));
    CHECK_FALSE(o->IsAvailable());
}

TEST_CASE("ollama settings persist host port and model", "[misc][ai]") {
    MemStore store;
    {
        ServiceManager sm(&store);
        auto* o = sm.getService("ollama");
        o->SetProperty("ollama.Host", std::string("10.0.0.5"));
        o->SetProperty("ollama.Port", 8080);
        o->SetProperty("ollama.Model", std::string("llama3"));
        o->SaveSettings();
    }
    ServiceManager sm(&store);
    auto props = sm.getService("ollama")->GetProperties();
    CHECK(std::get<std::string>(Find(props, "ollama.Host")->value) == "10.0.0.5");
    CHECK(std::get<int>(Find(props, "ollama.Port")->value) == 8080);
    CHECK(std::get<std::string>(Find(props, "ollama.Model")->value) == "llama3");
}

TEST_CASE("GenericClient exposes four capabilities and a local default endpoint", "[misc][ai]") {
    MemStore store;
    ServiceManager sm(&store);
    auto* g = sm.getService("genericClient");
    REQUIRE(g != nullptr);
    auto types = g->GetTypes();
    CHECK(types.size() == 4);
    auto props = g->GetProperties();
    auto* url = Find(props, "GenericClient.BaseURL");
    REQUIRE(url != nullptr);
    CHECK(std::get<std::string>(url->value) == "http://localhost:8000/api/v1");
    CHECK(Find(props, "GenericClient.Model") != nullptr);
    CHECK(Find(props, "GenericClient.ImageModel") != nullptr);
    CHECK(Find(props, "GenericClient.TranscribeModel") != nullptr);
    CHECK_FALSE(g->IsAvailable());
    g->SetProperty("GenericClient.Enable_Prompt", true);
    CHECK(g->IsAvailable());
    g->SetProperty("GenericClient.BaseURL", std::string(""));
    CHECK_FALSE(g->IsAvailable());
}

TEST_CASE("Every built in service names its properties with its own prefix", "[misc][ai]") {
    MemStore store;
    ServiceManager sm(&store);
    for (auto* s : sm.getServices()) {
        INFO(s->GetLLMName());
        auto props = s->GetProperties();
        REQUIRE_FALSE(props.empty());
        CHECK(props.front().kind == ServiceProperty::Kind::Category);
        for (auto const& p : props) {
            if (p.kind == ServiceProperty::Kind::Category) {
                continue;
            }
            CHECK(p.id.find('.') != std::string::npos);
            CHECK_FALSE(p.label.empty());
        }
    }
}

TEST_CASE("Choice properties are only offered with a non empty list", "[misc][ai]") {
    MemStore store;
    ServiceManager sm(&store);
    for (auto* s : sm.getServices()) {
        for (auto const& p : s->GetProperties()) {
            if (p.kind == ServiceProperty::Kind::Choice) {
                CHECK_FALSE(p.choices.empty());
            }
        }
    }
}
