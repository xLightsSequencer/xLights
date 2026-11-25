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

// An exported .xmodel can carry the files its models refer to (DMX images,
// meshes with their materials and textures, matrix face images, custom model
// backgrounds...) so it works when handed to someone else. Each file is
// stored inside the first <model> that refers to it as
//     <xmodelFile path="modeldownload/<prop>/<name>">base64</xmodelFile>
// and the references are written as that path. Importing writes the files
// under the show's modeldownload folder, as a downloaded vendor model's files
// are, and points the references at them. A version that doesn't know about
// embedded files ignores the elements and reports the images missing.

#include <map>
#include <set>
#include <string>
#include <vector>

namespace pugi {
class xml_node;
}

namespace XModelFiles {

constexpr const char* NodeName = "xmodelFile";
constexpr const char* PathAttribute = "path";
constexpr const char* Folder = "modeldownload";

class Packer {
public:
    // prop names the folder the files are imported into.
    explicit Packer(const std::string& prop);

    // The path to write for a file reference, with the file queued to be
    // embedded. References that can't be read, and meshes bundled with
    // xLights, are written unchanged.
    std::string Add(const std::string& file);

    struct Entry {
        std::string path;
        std::string base64;
    };
    // The files added since the last call.
    std::vector<Entry> TakePending();

private:
    bool Queue(const std::string& source, const std::string& path);
    std::string UniquePath(const std::string& dir, const std::string& name);
    void AddMeshDependencies(const std::string& obj, const std::string& pkgDir);

    std::string _dir;                          // "modeldownload/<prop>"
    std::map<std::string, std::string> _added; // source file -> package path
    std::set<std::string> _usedPaths;          // lower-cased package paths
    std::vector<Entry> _pending;
};

// Writes the files embedded anywhere under root into
// <showDir>/modeldownload, points every attribute naming one at the written
// file and removes the elements. A file already there with the same bytes is
// reused; when a prop's files would clash with different ones, the whole
// prop goes in a new folder (<prop>-2, ...) so a mesh still finds its
// materials. Returns the number of files placed.
int Unpack(pugi::xml_node root, const std::string& showDir);

} // namespace XModelFiles
