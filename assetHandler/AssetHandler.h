//
// Created by d on 2/4/26.
//

#ifndef ASSETHANDLER_H
#define ASSETHANDLER_H

#include <filesystem>
#include <unordered_map>

#include "Asset.h"
#include "AssetType.h"
#include "Texture.h"
#include "Mesh.h"
#include "ShaderModule.h"

namespace Plunksna {

const std::array g_workingPaths = {
    std::filesystem::current_path().parent_path(),
    std::filesystem::current_path()
};

const std::filesystem::path g_meshPath = "models";
const std::filesystem::path g_texturePath = "textures";
const std::filesystem::path g_shaderPath = "shaders";

const std::array g_assetFolders = {
    g_meshPath,
    g_texturePath,
    g_shaderPath
};

inline std::filesystem::path g_workingPath;

class AssetHandler {
public:
    AssetHandler();
    ~AssetHandler();

    void clean(const Context& context);

    //====TEXTURES====

    //load texture from disk to ram
    Asset loadTexture(const std::string& name);
    //get texture from handle
    Texture* getTexture(Asset texHnd);
    //destroy cpu resources
    void freeTextureHost(Asset texHnd);
    //destroy gpu resources
    void freeTextureDevice(const Context& context, Asset texHnd);
    //destroy the entire texture;
    void destroyTexture(const Context& context, Asset texHnd);
    //set texture device id
    void setTextureID(Asset texHnd, u32 id);
    //get texture device id
    u32 getTextureId(Asset texHnd) const;
    //get loaded textures
    std::vector<Asset> getLoadedTextures() const;

    //====MESHES======

    //load Mesh from disk to ram
    Asset loadMesh(std::string name);
    //get Mesh from handle
    Mesh* getMesh(Asset meshHnd);
    //destroy cpu resources
    void freeMeshHost(Asset meshHnd);
    //destroy gpu resources
    void freeMeshDevice(const Context& context, Asset meshHnd);
    //destroy the entire Mesh;
    void destroyMesh(const Context& context, Asset meshHnd);
    //get loaded meshes
    std::vector<Asset> getLoadedMeshes() const;

    //====SHADERS====

    //load shader from disk
    Asset loadShader(const Context& context, std::string name);
    //get shader from handle
    ShaderModule* getShader(Asset shader);
    //destroy cpu shader
    void freeShaderHost(Asset shader);
    //destroy device shader
    void freeShaderDevice(const Context& context, Asset shader);
    //destroy shader entirely
    void destroyShaderModule(const Context& context, Asset shader);

private:
    Asset makeAsset();
    std::filesystem::path initWorkingPath();

    //textures
    //destroy without checks
    static void destroyTextureHost(Texture* texture);
    static void destroyTextureDevice(const Context& context, Texture* texture);
    static void freeTexture(const Context& context, Texture* texture);

    //meshes
    //destroy without checks
    static void destroyMeshHost(Mesh* mesh);
    static void destroyMeshDevice(const Context& context, Mesh* mesh);
    static void freeMesh(const Context& context, Mesh* mesh);

    //shader
    static void destroyShaderHost(ShaderModule* shader);
    static void destroyShaderDevice(const Context& context, ShaderModule* shader);
    static void freeShader(const Context& context, ShaderModule* shader);

private:
    std::unordered_map<Asset, Texture>      m_textures;
    std::unordered_map<Asset, u32>          m_textureIDs;
    std::unordered_map<Asset, Mesh>         m_meshes;
    std::unordered_map<Asset, ShaderModule> m_shaders;

    //for later
    //std::unordered_map<Asset, AssetType>    m_assetTypes;

    std::vector<Asset> m_fragments;
    Asset m_maxAsset = 0;
};

} // Plunksna

#endif //ASSETHANDLER_H
