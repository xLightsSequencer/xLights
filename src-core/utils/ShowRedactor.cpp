/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include "ShowRedactor.h"

#include <algorithm>
#include <cctype>
#include <map>
#include <string>

#include <pugixml.hpp>

namespace ShowRedactor {

namespace {

// A serial controller's port is not an address and must survive: rewriting
// /dev/tty* or COM3 would stop the controller loading as the kind it is.
bool IsSerialPort(std::string const& v)
{
    if (v.rfind("/dev/", 0) == 0) {
        return true;
    }
    if (v.size() >= 4 && (v.rfind("COM", 0) == 0 || v.rfind("com", 0) == 0)) {
        return std::isdigit(static_cast<unsigned char>(v[3])) != 0;
    }
    return v.find("Serial") != std::string::npos || v.find("usbserial") != std::string::npos;
}

bool IsAbsolutePath(std::string const& v)
{
    if (v.size() < 2) {
        return false;
    }
    if (v[0] == '/') {
        return true; // posix
    }
    if (v[0] == '\\' && v[1] == '\\') {
        return true; // UNC
    }
    // A drive letter, either slash. Checked before any generic separator test so
    // "C:/foo" is caught on a posix host too - reports come from every platform.
    return v.size() >= 3 && std::isalpha(static_cast<unsigned char>(v[0])) && v[1] == ':' && (v[2] == '/' || v[2] == '\\');
}

std::string BaseName(std::string const& path)
{
    auto const cut = path.find_last_of("/\\");
    return cut == std::string::npos ? path : path.substr(cut + 1);
}

// Keeps the leaf so the report still says what the show was reaching for - a
// .obj that failed to load is a different bug from a .gif that did. The
// directories above it are what name the machine and the person.
std::string RedactPath(std::string const& path)
{
    std::string const leaf = BaseName(path);
    if (leaf.empty()) {
        return std::string(PATH_TOKEN) + "/";
    }
    return std::string(PATH_TOKEN) + "/" + leaf;
}

// Effect settings arrive packed as "Key=Value,Key=Value,..." in a single
// attribute (presets carry them), so a path can hide one level below the
// attribute value itself.
bool RedactSettingsBlob(std::string const& value, std::string& out, int& pathCount)
{
    if (value.find('=') == std::string::npos || value.find(',') == std::string::npos) {
        return false;
    }
    std::string result;
    result.reserve(value.size());
    bool changed = false;
    size_t pos = 0;
    while (pos <= value.size()) {
        auto const comma = value.find(',', pos);
        std::string const token = value.substr(pos, comma == std::string::npos ? std::string::npos : comma - pos);
        auto const eq = token.find('=');
        if (eq != std::string::npos && IsAbsolutePath(token.substr(eq + 1))) {
            result += token.substr(0, eq + 1);
            result += RedactPath(token.substr(eq + 1));
            ++pathCount;
            changed = true;
        } else {
            result += token;
        }
        if (comma == std::string::npos) {
            break;
        }
        result += ',';
        pos = comma + 1;
    }
    if (changed) {
        out = result;
    }
    return changed;
}

// Every attribute in the document, so a path is caught wherever it hides -
// including nodes added after this was written. Enumerating known attributes
// instead would silently leak the first new one.
void WalkAttributes(pugi::xml_node node, Stats& stats)
{
    for (pugi::xml_attribute attr : node.attributes()) {
        std::string const value = attr.value();
        if (value.empty() || IsSerialPort(value)) {
            // A device node is an absolute path that names no one, and a
            // controller whose port was rewritten stops loading as a serial
            // controller at all.
            continue;
        }
        if (IsAbsolutePath(value)) {
            attr.set_value(RedactPath(value).c_str());
            ++stats.paths;
            continue;
        }
        std::string blob;
        if (RedactSettingsBlob(value, blob, stats.paths)) {
            attr.set_value(blob.c_str());
        }
    }
    for (pugi::xml_node child : node.children()) {
        WalkAttributes(child, stats);
    }
}

// Distinct real addresses map to distinct synthetic ones, first-seen order, so
// the show's topology - which controllers share a proxy, how many there are -
// still reads correctly. Collapsing them all to one string would lose that.
class AddressMap
{
public:
    std::string For(std::string const& real)
    {
        auto const it = _map.find(real);
        if (it != _map.end()) {
            return it->second;
        }
        // Two octets so a large show's controllers stay distinct; collapsing
        // two real addresses onto one would invent a shared proxy.
        std::string const synthetic = "10.0." + std::to_string(_next / 254) + "." + std::to_string(_next % 254 + 1);
        ++_next;
        _map[real] = synthetic;
        return synthetic;
    }

private:
    std::map<std::string, std::string> _map;
    int _next = 1;
};

void RedactAddressAttr(pugi::xml_node node, char const* name, AddressMap& addresses, Stats& stats)
{
    pugi::xml_attribute attr = node.attribute(name);
    if (!attr) {
        return;
    }
    std::string const value = attr.value();
    if (value.empty() || IsSerialPort(value)) {
        return;
    }
    // Hostnames leak as readily as addresses ("<name>-pi.local"), so anything
    // that is not a serial port is replaced, not just dotted quads.
    attr.set_value(addresses.For(value).c_str());
    ++stats.addresses;
}

void WalkAddresses(pugi::xml_node node, AddressMap& addresses, Stats& stats)
{
    for (char const* name : { "IP", "ComPort", "FPPProxy", "GlobalFPPProxy" }) {
        RedactAddressAttr(node, name, addresses, stats);
    }
    for (pugi::xml_node child : node.children()) {
        WalkAddresses(child, addresses, stats);
    }
}

} // namespace

Stats RedactRgbEffects(pugi::xml_document& doc)
{
    Stats stats;
    WalkAttributes(doc, stats);
    return stats;
}

Stats RedactNetworks(pugi::xml_document& doc)
{
    Stats stats;
    AddressMap addresses;
    // Addresses first: an address pass that ran after the path pass would find
    // ComPort already rewritten for the serial controllers whose port is a path.
    WalkAddresses(doc, addresses, stats);
    WalkAttributes(doc, stats);
    return stats;
}

bool RedactFileToFile(std::string const& srcPath, std::string const& destPath, bool isNetworksFile, Stats* stats)
{
    pugi::xml_document doc;
    if (!doc.load_file(srcPath.c_str())) {
        return false;
    }
    Stats const s = isNetworksFile ? RedactNetworks(doc) : RedactRgbEffects(doc);
    if (stats != nullptr) {
        *stats = s;
    }
    return doc.save_file(destPath.c_str());
}

} // namespace ShowRedactor
