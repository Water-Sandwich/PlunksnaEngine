//
// Created by d on 2/4/26.
//

#include "AssetHandler.h"

#include <tiny_obj_loader.h>

#include "engine/Exception.h"
#include "vkrenderer/RendererUtils.h"
#include "vkrenderer/Vertex.h"

using namespace Plunksna::RenderUtils;

namespace Plunksna {

template<typename T>
T* getFromUMap(Asset asset, std::unordered_map<Asset, T>& umap)
{
    auto it = umap.find(asset);
    if (it != umap.end())
        return &it->second;

    LOG_S(eWARNING, "tried to retrieve an unaccounted asset! Asset:" << asset << " Type: " <<typeid(T).name())
    return nullptr;
}

template<typename T>
void removeAsset(Asset asset, std::unordered_map<Asset, T>& umap)
{
    umap.erase(asset);
    umap.push_back(asset);
}

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
    return getFromUMap(tex, m_textures);
}

void AssetHandler::freeTextureHost(Asset tex)
{
    Texture* texture = getTexture(tex);

    CHECK_R(texture, "free called on unaccounted texture")

    CHECK_R(texture->isHostLoaded(), "free called on host unloaded texture")

    destroyTextureHost(texture);
}

void AssetHandler::freeTextureDevice(const Context& context, Asset tex)
{
    Texture* texture = getTexture(tex);

    CHECK_R(texture, "free called on unaccounted texture");

    CHECK_R(texture->isDeviceLoaded(), "free called on device unloaded texture")

    destroyTextureDevice(context, texture);
}

void AssetHandler::destroyTexture(const Context& context, Asset tex)
{
    Texture* texture = getTexture(tex);

    CHECK_R(texture, "destroy called on unaccounted texture")

    if (texture->isHostLoaded())
        destroyTextureHost(texture);

    if (texture->isDeviceLoaded())
        destroyTextureDevice(context, texture);

    removeAsset(tex, m_textures);
}

Asset AssetHandler::loadMesh(std::string name)
{
    Asset asset = makeAsset();
    Mesh& mesh = m_meshes[asset];
    auto path = g_workingPath / g_meshPath / name;

    tinyobj::attrib_t attrib;
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;
    std::string err;
    std::string warn;

    ASSERT(tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, path.c_str()), err);

    std::unordered_map<Vertex, uint32_t> uniqueVertices{};

    for (const auto& shape : shapes) {
        for (const auto& index : shape.mesh.indices) {
            Vertex vertex{};

            vertex.pos = {
                attrib.vertices[3 * index.vertex_index + 0],
                attrib.vertices[3 * index.vertex_index + 1],
                attrib.vertices[3 * index.vertex_index + 2]
            };

            vertex.texCoord = {
                attrib.texcoords[2 * index.texcoord_index + 0],
                attrib.texcoords[2 * index.texcoord_index + 1]
            };

            vertex.color = {1.0f, 1.0f, 1.0f};

            if (!uniqueVertices.contains(vertex)) {
                uniqueVertices[vertex] = static_cast<uint32_t>(mesh.vertices.size());
                mesh.vertices.push_back(vertex);
            }

            mesh.indices.push_back(uniqueVertices[vertex]);
        }
    }

    return asset;
}

Mesh* AssetHandler::getMesh(Asset tex)
{
    return getFromUMap(tex, m_meshes);
}

void AssetHandler::freeMeshHost(Asset tex)
{
    Mesh* mesh = getMesh(tex);

    CHECK_R(mesh, "free called on unaccounted mesh")

    CHECK_R(mesh->isHostLoaded(), "free called on host unloaded mesh")

    destroyMeshHost(mesh);
}

void AssetHandler::freeMeshDevice(const Context& context, Asset tex)
{
    Mesh* mesh = getMesh(tex);

    CHECK_R(mesh, "free called on unaccounted mesh")

    CHECK_R(mesh->isHostLoaded(), "free called on device unloaded mesh")

    destroyMeshDevice(context, mesh);
}

void AssetHandler::destroyMesh(const Context& context, Asset meshHnd)
{
    Mesh* mesh = getMesh(meshHnd);

    CHECK_R(mesh, "destroy called on unaccounted texture")

    if (mesh->isHostLoaded())
        destroyMeshHost(mesh);

    if (mesh->isDeviceLoaded())
        destroyMeshDevice(context, mesh);

    removeAsset(meshHnd, m_meshes);
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

void AssetHandler::destroyMeshHost(Mesh* mesh)
{
    //clear doesnt free memory
    std::vector<Vertex>().swap(mesh->vertices);
    std::vector<uint32_t>().swap(mesh->indices);
}

void AssetHandler::destroyMeshDevice(const Context& context, Mesh* mesh)
{
    mesh->indexBuffer.destroy(context);
    mesh->vertexBuffer.destroy(context);
}

} // Plunksna