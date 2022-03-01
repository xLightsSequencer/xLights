#pragma once

#include <string>
#include "../Color.h"
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
    
    
    static std::vector<std::string> GetMaterialFilenamesFromOBJ(const std::string &obj, bool strict = true);
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

