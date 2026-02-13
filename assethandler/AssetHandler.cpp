//
// Created by d on 2/4/26.
//

#include "AssetHandler.h"

#include "engine/Exception.h"
#include "vkrenderer/RendererUtils.h"

using namespace Plunksna::RenderUtils;

namespace Plunksna {
AssetHandler::AssetHandler()
{
    initWorkingPath();
}

AssetHandler::~AssetHandler()
{
    LOG("Asset handler destroyed")
}

Asset AssetHandler::loadTexture(std::string name)
{
    Asset asset = makeAsset();
    Texture& tex = m_textures[asset];
    const auto path = g_workingPath / g_texturePath / name;

    tex.pixels = stbi_load(path.c_str(), &tex.extents.x, &tex.extents.y, &tex.extents.z, STBI_rgb_alpha);

    ASSERT_SS(tex.pixels, "failed to load texture image! : " << path.string())

    return asset;
}

Texture* AssetHandler::getTexture(Asset tex)
{
    auto it = m_textures.find(tex);
    if (it != m_textures.end())
        return &it->second;

    LOG_S(eWARNING, "tried to retrieve an unaccounted texture! Asset:" << tex)
    return nullptr;
}

void AssetHandler::freeTextureHost(Asset tex)
{
    Texture* texture = getTexture(tex);

    if (!texture) {
        LOG_S(eWARNING, "free called on unaccounted texture")
        return;
    }

    if (!texture->isHostLoaded()) {
        LOG_S(eWARNING, "free called on host unloaded texture")
        return;
    }

    destroyTextureHost(texture);
}

void AssetHandler::freeTextureDevice(const Context& context, Asset tex)
{
    Texture* texture = getTexture(tex);

    if (!texture) {
        LOG_S(eWARNING, "free called on unaccounted texture")
        return;
    }

    if (!texture->isDeviceLoaded()) {
        LOG_S(eWARNING, "free called on device unloaded texture")
        return;
    }

    destroyTextureDevice(context, texture);
}

void AssetHandler::destroyTexture(const Context& context, Asset tex)
{
    Texture* texture = getTexture(tex);

    if (!texture) {
        LOG_S(eWARNING, "destroy called on unaccounted texture")
        return;
    }

    if (texture->isHostLoaded())
        destroyTextureHost(texture);

    if (texture->isDeviceLoaded())
        destroyTextureDevice(context, texture);

    removeTexture(tex);
}

Asset AssetHandler::makeAsset()
{
    if (m_fragments.empty()) {
        return m_maxAsset++;
    }

    auto asset = m_fragments.back();
    m_fragments.pop_back();
    return asset;
}

std::filesystem::path AssetHandler::initWorkingPath()
{
    if (!g_workingPath.empty()) {
        return g_workingPath;
    }

    for (const auto& path : g_workingPaths) {
        for (const auto& folder : g_assetFolders) {
            if (std::filesystem::exists(path / folder) &&
                std::filesystem::is_directory(path / folder))
            {
                g_workingPath = path;
                return g_workingPath;
            }
        }
    }

    THROW("Invalid working paths!")
}

void AssetHandler::removeTexture(Asset asset)
{
    m_textures.erase(asset);
    m_fragments.push_back(asset);
}

void AssetHandler::destroyTextureHost(Texture* texture)
{
    stbi_image_free(texture->pixels);
    texture->pixels = nullptr;
    texture->extents = {0,0,0};
}

void AssetHandler::destroyTextureDevice(const Context& context, Texture* texture)
{
    texture->image.destroy(context);
    VK_DESTROY(texture->fullView, context.device, vkDestroyImageView)
    texture->mipLevels = 1;
}
} // Plunksna