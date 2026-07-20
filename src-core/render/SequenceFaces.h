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

#include <list>
#include <map>
#include <string>

class SequenceMedia;
namespace pugi { class xml_node; }

/**
 * SequenceFaces - Sequence-level face definitions stored in the .xsq file.
 *
 * Holds Matrix (image) style face definitions only — node-based face styles
 * depend on a specific model's node layout and remain model-level. Definitions
 * use the same shape as Model::faceInfo: name -> (attribute -> value), with
 * image paths under Mouth-<PHONEME>-EyesOpen/-EyesClosed keys. The Faces
 * effect resolves a definition name against the model first, then here, so
 * any matrix/group/submodel in the sequence can use these without duplicating
 * them onto every model.
 */
class SequenceFaces
{
public:
    using FaceDefinition = std::map<std::string, std::string>;
    using FaceMap = std::map<std::string, FaceDefinition>;

    bool empty() const { return _faces.empty(); }
    const FaceMap& GetFaces() const { return _faces; }
    const FaceDefinition* GetFace(const std::string& name) const;
    void SetFace(const std::string& name, const FaceDefinition& def);
    bool RemoveFace(const std::string& name);
    bool RenameFace(const std::string& oldName, const std::string& newName);
    void Clear() { _faces.clear(); }

    // Image-path helpers for media walkers (rename/embed/remove/package)
    std::list<std::string> GetImagePaths() const;
    int CountImageReferences(const std::string& path) const;
    int RewriteImagePath(const std::string& from, const std::string& to);

    bool LoadFromXml(const pugi::xml_node& node, SequenceMedia& media);
    void SaveToXml(pugi::xml_node& parent) const;

    // True for keys whose value is an image file path (Mouth-*/Eyes-* but not
    // node-style color keys)
    static bool IsImageKey(const std::string& key);

private:
    FaceMap _faces;
};
