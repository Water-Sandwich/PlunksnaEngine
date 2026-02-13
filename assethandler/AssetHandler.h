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

namespace Plunksna {

const std::array g_workingPaths = {
    std::filesystem::current_path().parent_path(),
    std::filesystem::current_path()
};

const std::filesystem::path g_modelPath = "models";
const std::filesystem::path g_texturePath = "textures";

const std::array g_assetFolders = {
    g_modelPath,
    g_texturePath
};

inline std::filesystem::path g_workingPath;

class AssetHandler {
public:
    AssetHandler();
    ~AssetHandler();

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

private:
    Asset makeAsset();
    std::filesystem::path initWorkingPath();

    //textures
    //destroy without checks
    void removeTexture(Asset asset);
    void destroyTextureHost(Texture* texture);
    void destroyTextureDevice(const Context& context, Texture* texture);

private:
    std::unordered_map<Asset, Texture> m_textures;
    std::vector<Asset> m_fragments;
    Asset m_maxAsset = 0;
};

} // Plunksna

#endif //ASSETHANDLER_H
