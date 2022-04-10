

#define TINYOBJLOADER_IMPLEMENTATION
#define TINYOBJLOADER_USE_MAPBOX_EARCUT
#include "xlMesh.h"

#include <filesystem>
#include <algorithm>

#include <wx/filename.h>

#include "../ExternalHooks.h"
#include "../UtilFunctions.h"

#include <log4cpp/Category.hh>

xlMesh::xlMesh(xlGraphicsContext *ctx, const std::string &f) : graphicsContext(ctx), filename(f) {
    static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    objectsLoaded = false;
    wxFileName fn(filename);
    
    tinyobj::ObjReaderConfig reader_config;
    reader_config.triangulate = true;
    reader_config.mtl_search_path = fn.GetPath();  // Path to material files

    if (!objects.ParseFromFile(filename, reader_config)) {
        if (!objects.Error().empty()) {
            logger_base.error("TinyObjReader: %s", objects.Error().c_str());
            return;
        }
    }
    if (!objects.Warning().empty()) {
        logger_base.warn("TinyObjReader: %s", objects.Warning().c_str());
    }
    objectsLoaded = true;
    
    
    materials.resize(objects.GetMaterials().size());
    int idx = 0;
    for (auto &m : objects.GetMaterials()) {
        float dissolve = m.dissolve * 255.0f;
        float red = m.diffuse[0] * 255.0f;
        float green = m.diffuse[1] * 255.0f;
        float blue = m.diffuse[2] * 255.0f;
        materials[idx].name = m.name;
        materials[idx].color.Set((uint8_t)red, (uint8_t)green, (uint8_t)blue, (uint8_t)dissolve);
        if (m.diffuse_texname != "") {
            wxString texName = m.diffuse_texname;
            if (!FileExists(texName)) {
                texName = fn.GetPath() + fn.GetPathSeparator() + m.diffuse_texname;
            }
            if (!FileExists(texName)) {
                texName = m.diffuse_texname;
                if (texName.Contains(("/"))) {
                    texName = texName.substr(texName.Last('/') + 1);
                }
                texName = fn.GetPath() + fn.GetPathSeparator() + texName;
            }
            if (FileExists(texName)) {
                ObtainAccessToURL(texName);
                wxImage image(texName);
                if (image.IsOk()) {
                    image = image.Mirror(false);
                    materials[idx].texture = ctx->createTexture(image);
                    materials[idx].texture->SetName(m.diffuse_texname);
                    materials[idx].texture->Finalize();
                }
            }
        }
        materials[idx].origColor = materials[idx].color;
        idx++;
    }
    
    xMin = 9999999;
    yMin = 9999999;
    zMin = 9999999;
    xMax = -9999999;
    yMax = -9999999;
    zMax = -9999999;
    auto &vertices = objects.GetAttrib().vertices;
    for (int x = 0; x < vertices.size(); x += 3) {
        xMin = std::min(vertices[x], xMin);
        xMax = std::max(vertices[x], xMax);
        yMin = std::min(vertices[x + 1], yMin);
        yMax = std::max(vertices[x + 1], yMax);
        zMin = std::min(vertices[x + 2], zMin);
        zMax = std::max(vertices[x + 2], zMax);
    }
}

xlMesh::~xlMesh() {
}


void xlMesh::SetMaterialColor(const std::string materialName, const xlColor *c) {
    for (auto &m : materials) {
        if (m.name == materialName) {
            if (c) {
                m.color = *c;
                m.forceColor = true;
                materialsNeedResyncing = true;
            } else {
                m.forceColor = false;
                m.color = m.origColor;
                materialsNeedResyncing = true;
            }
        }
    }
}

// this list should contain 0 or more .mtl files ... if it just contains 2 and one does not end in mtl then it is likely a space in the mtl file name
bool xlMesh::InvalidMaterialsList(const std::vector<std::string>& materialFiles)
{
    if (materialFiles.size() == 2) {
        for (const auto& it : materialFiles) {
            if (!EndsWith(Lower(it), ".mtl")) {
                return true;
            }
        }
    }
    return false;
}

std::vector<std::string> xlMesh::GetMaterialFilenamesFromOBJ(const std::string &obj, bool strict) {
    std::vector<std::string> ret;

    std::ifstream input(obj);
    for (std::string line; std::getline(input, line);) {
        if (line.rfind("mtllib ", 0) == 0) {
            line = line.substr(7);
            auto idx = line.find(' ');
            while (strict && idx != std::string::npos) {
                std::string f = line.substr(0, idx);
                ret.push_back(f);
                line = line.substr(idx + 1);
                idx = line.find(' ');
            }
            if (line != "") {
                ret.push_back(line);
            }
        }
    }
    return ret;
}

void xlMesh::FixMaterialFilenamesInOBJ(const std::string &obj) {
    std::filesystem::copy(obj, obj + ".bak");
    
    std::filesystem::path path(obj);

    std::ifstream input(obj + ".bak");
    std::ofstream output(obj, std::ofstream::out | std::ofstream::trunc);
    for (std::string line; std::getline(input, line); ) {
        if (line.rfind("mtllib ", 0) == 0) {
            output << "mtllib ";
            line = line.substr(7);
            int idx = line.find(' ');
            if (idx != std::string::npos) {
                std::filesystem::path mtlpath(path);
                mtlpath.replace_filename(line);
                if (std::filesystem::exists(mtlpath)) {
                    std::replace(line.begin(), line.end(), ' ', '_');
                    std::filesystem::path nmtlpath(path);
                    nmtlpath.replace_filename(line);
                    std::filesystem::copy(mtlpath, nmtlpath);
                    output << line << "\n";
                } else {
                    output << line << "\n";
                }
            } else {
                output << line << "\n";
            }
        } else {
            output << line << "\n";
        }
    }
}


