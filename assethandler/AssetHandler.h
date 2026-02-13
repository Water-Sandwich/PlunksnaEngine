//
// Created by d on 2/4/26.
//

#ifndef ASSETHANDLER_H
#define ASSETHANDLER_H
#include <cstdint>
#include <filesystem>
#include <unordered_map>

#include "Asset.h"
#include "Texture.h"
#include "Mesh.h"

namespace Plunksna {

const std::array g_workingPaths = {
    std::filesystem::current_path().parent_path(),
    std::filesystem::current_path()
};

const std::filesystem::path g_meshPath = "models";
const std::filesystem::path g_texturePath = "textures";

const std::array g_assetFolders = {
    g_meshPath,
    g_texturePath
};

inline std::filesystem::path g_workingPath;

class AssetHandler {
public:
    AssetHandler();
    ~AssetHandler();

    //====TEXTURES====

    //load texture from disk to ram
    Asset loadTexture(std::string name);
    //get texture from handle
    Texture* getTexture(Asset tex);
    //destroy cpu resources
    void freeTextureHost(Asset tex);
    //destroy gpu resources
    void freeTextureDevice(const Context& context, Asset tex);
    //destroy the entire texture;
    void destroyTexture(const Context& context, Asset tex);

    //====MESHES======

    //load Mesh from disk to ram
    Asset loadMesh(std::string name);
    //get Mesh from handle
    Mesh* getMesh(Asset tex);
    //destroy cpu resources
    void freeMeshHost(Asset tex);
    //destroy gpu resources
    void freeMeshDevice(const Context& context, Asset tex);
    //destroy the entire Mesh;
    void destroyMesh(const Context& context, Asset meshHnd);

private:
    Asset makeAsset();
    std::filesystem::path initWorkingPath();

    //textures
    //destroy without checks
    void destroyTextureHost(Texture* texture);
    void destroyTextureDevice(const Context& context, Texture* texture);

    //meshes
    //destroy without checks
    void destroyMeshHost(Mesh* mesh);
    void destroyMeshDevice(const Context& context, Mesh* mesh);

private:
    std::unordered_map<Asset, Texture>  m_textures;
    std::unordered_map<Asset, Mesh>     m_meshes;

    std::vector<Asset> m_fragments;
    Asset m_maxAsset = 0;
};

} // Plunksna

#endif //ASSETHANDLER_H
