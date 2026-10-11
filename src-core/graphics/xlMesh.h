#pragma once

#include <atomic>
#include <chrono>
#include <memory>
#include <string>
#include <vector>
#include "Color.h"
#include "tiny_obj_loader.h"
#include "xlGraphicsContext.h"


class xlMesh {
public:
    xlMesh(xlGraphicsContext *ctx, const std::string &file);
    virtual ~xlMesh();
 
    void SetName(const std::string &n) { name = n; }
    const std::string &GetName() const { return name; }
    
    class Material {
    public:
        Material() {}
        ~Material() {
            if (texture) {
                delete texture;
            }
        }
        
        std::string name;
        xlTexture *texture = nullptr;
        xlColor color = xlBLACK;
        bool forceColor = false;
        xlColor origColor = xlBLACK;
    };
    
    const Material& GetMaterial(int idx) const { return materials[idx]; }
    
    float GetXMin() const { return xMin; }
    float GetYMin() const { return yMin; }
    float GetZMin() const { return zMin; }

    float GetXMax() const { return xMax; }
    float GetYMax() const { return yMax; }
    float GetZMax() const { return zMax; }
    
    void SetMaterialColor(const std::string materialName, const xlColor *c);

    // True only when ParseFromFile succeeded in the constructor. Subclasses
    // (xlGLMesh / xlMetalMesh) consult this before walking objects' shapes /
    // attrib vectors, which on a half-init tinyobj state can crash on the
    // very first vertices.size() read (Windows bucket 7f18c56ac4).
    bool IsLoaded() const { return objectsLoaded; }
    bool HasGeometry() const { return objectsLoaded && (!objects.GetShapes().empty() || !objects.GetAttrib().vertices.empty()); }
    
    static bool InvalidMaterialsList(const std::vector<std::string>& mtls);
    static std::vector<std::string> GetMaterialFilenamesFromOBJ(const std::string& obj, bool strict = true);
    static std::vector<std::string> GetTextureFilenamesFromMTL(const std::string& mtl, bool strict = true);
    static void FixMaterialFilenamesInOBJ(const std::string &obj);

protected:
    std::string name;
    
    std::string filename;
    xlGraphicsContext *graphicsContext;
    
    tinyobj::ObjReader objects;
    bool objectsLoaded;
    
    std::vector<Material> materials;
    bool materialsNeedResyncing = false;
    
    float xMin = 9999999;
    float yMin = 9999999;
    float zMin = 9999999;
    float xMax = -9999999;
    float yMax = -9999999;
    float zMax = -9999999;
};

// Meshes are loaded from the preview draw, on the main thread.  On Apple an
// OBJ, MTL or texture evicted to iCloud made FileExists wait there for up to a
// minute while it downloaded, and iOS killed the app for it (0x8BADF00D).
// Ready() never blocks: it waits for the downloads on a background thread and
// reports true once the OBJ is local, so the draw loads it on a later frame.
class xlMeshFilePrefetch {
public:
    bool Ready(const std::string& objFile);

private:
    std::string _file;
    std::shared_ptr<std::atomic<int>> _state;
    std::chrono::steady_clock::time_point _started;
};

