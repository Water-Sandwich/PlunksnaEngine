//
// Created by d on 2/4/26.
//
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/norm.hpp>

#include "AssetHandler.h"

#include <tiny_obj_loader.h>

#include "engine/Exception.h"
#include "vkRenderer/RendererUtils.h"
#include "vkRenderer/Vertex.h"

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
std::vector<Asset> getLoadedAssets(std::unordered_map<Asset, T> map)
{
    std::vector<Asset> vec;
    vec.reserve(map.size());
    for (const auto& [asset, other] : map) {
        vec.push_back(asset);
    }
    return vec;
}

AssetHandler::AssetHandler()
{
    initWorkingPath();
}

AssetHandler::~AssetHandler()
{
    LOG("Asset handler destroyed")
}

void AssetHandler::clean(const Context& context)
{
    for (auto& [ass, obj] : m_textures) {
        freeTexture(context, &obj);
    }

    for (auto& [ass, obj] : m_meshes) {
        freeMesh(context, &obj);
    }

    for (auto& [ass, obj] : m_shaders) {
        freeShader(context, &obj);
    }

    m_textures.clear();
    m_meshes.clear();
    m_shaders.clear();
    m_textureIDs.clear();
}

//TEXTURES

Asset AssetHandler::loadTexture(const std::string& name)
{
    Asset asset = makeAsset();
    Texture& tex = m_textures[asset];
    const auto path = g_workingPath / g_texturePath / name;

    tex.pixels = stbi_load(path.c_str(), &tex.extents.x, &tex.extents.y, &tex.extents.z, STBI_rgb_alpha);

    ASSERT(tex.pixels, "failed to load texture image! : " << path.string())

    return asset;
}

Texture* AssetHandler::getTexture(Asset texHnd)
{
    return getFromUMap(texHnd, m_textures);
}

void AssetHandler::freeTextureHost(Asset texHnd)
{
    Texture* texture = getTexture(texHnd);

    CHECK_R(texture, "free called on unaccounted texture")

    CHECK_R(texture->isHostLoaded(), "free called on host unloaded texture")

    destroyTextureHost(texture);
}

void AssetHandler::freeTextureDevice(const Context& context, Asset texHnd)
{
    Texture* texture = getTexture(texHnd);

    CHECK_R(texture, "free called on unaccounted texture");

    CHECK_R(texture->isDeviceLoaded(), "free called on device unloaded texture")

    destroyTextureDevice(context, texture);

    m_textureIDs.erase(texHnd);
}

void AssetHandler::destroyTexture(const Context& context, Asset texHnd)
{
    Texture* texture = getTexture(texHnd);

    CHECK_R(texture, "destroy called on unaccounted texture")

    freeTexture(context, texture);

    m_textures.erase(texHnd);
    m_fragments.push_back(texHnd);
}

void AssetHandler::setTextureID(Asset texHnd, u32 id)
{
    m_textureIDs[texHnd] = id;
}

u32 AssetHandler::getTextureId(Asset texHnd) const
{
    if (m_textureIDs.contains(texHnd))
        return m_textureIDs.at(texHnd);

    return 0;
}

std::vector<Asset> AssetHandler::getLoadedTextures() const
{
    return getLoadedAssets(m_textures);
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

void AssetHandler::freeTexture(const Context& context, Texture* texture)
{
    if (texture->isHostLoaded())
        destroyTextureHost(texture);

    if (texture->isDeviceLoaded())
        destroyTextureDevice(context, texture);
}

//MESHES

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

    glm::vec3 furthest{0};

    ASSERT(tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, path.c_str()), err);

    std::unordered_map<Vertex, i32> uniqueVertices{};

    for (const auto& shape : shapes) {
        for (const auto& index : shape.mesh.indices) {
            Vertex vertex{};

            vertex.pos = {
                attrib.vertices[3 * index.vertex_index + 0],
                attrib.vertices[3 * index.vertex_index + 1],
                attrib.vertices[3 * index.vertex_index + 2]
            };

            vertex.normal = {
                attrib.normals[3 * index.normal_index + 0],
                attrib.normals[3 * index.normal_index + 1],
                attrib.normals[3 * index.normal_index + 2]
            };

            vertex.texCoord = {
                attrib.texcoords[2 * index.texcoord_index + 0],
                attrib.texcoords[2 * index.texcoord_index + 1]
            };

            if (!uniqueVertices.contains(vertex)) {
                uniqueVertices[vertex] = static_cast<i32>(mesh.vertices.size());
                mesh.vertices.push_back(vertex);
            }

            if (glm::length2(vertex.pos) > glm::length2(furthest))
                furthest = vertex.pos;

            mesh.indices.push_back(uniqueVertices[vertex]);
        }
    }

    mesh.cullSphere.offset = furthest / 2.f;
    mesh.cullSphere.radius = glm::length(furthest) / 2.f;

    mesh.verticesCount = mesh.vertices.size();
    mesh.verticesSize = mesh.vertices.size() * sizeof(mesh.vertices[0]);
    mesh.indicesCount = mesh.indices.size();
    mesh.indicesSize = mesh.indices.size() * sizeof(mesh.indices[0]);

    return asset;
}

Mesh* AssetHandler::getMesh(Asset meshHnd)
{
    return getFromUMap(meshHnd, m_meshes);
}

void AssetHandler::freeMeshHost(Asset meshHnd)
{
    Mesh* mesh = getMesh(meshHnd);

    CHECK_R(mesh, "free called on unaccounted mesh")

    CHECK_R(mesh->isHostLoaded(), "free called on host unloaded mesh")

    destroyMeshHost(mesh);
}

void AssetHandler::freeMeshDevice(const Context& context, Asset meshHnd)
{
    Mesh* mesh = getMesh(meshHnd);

    CHECK_R(mesh, "free called on unaccounted mesh")

    CHECK_R(mesh->isHostLoaded(), "free called on device unloaded mesh")

    destroyMeshDevice(context, mesh);
}

void AssetHandler::destroyMesh(const Context& context, Asset meshHnd)
{
    Mesh* mesh = getMesh(meshHnd);

    CHECK_R(mesh, "destroy called on unaccounted texture")

    freeMesh(context, mesh);

    m_meshes.erase(meshHnd);
    m_fragments.push_back(meshHnd);
}

std::vector<Asset> AssetHandler::getLoadedMeshes() const
{
    return getLoadedAssets(m_meshes);
}

void AssetHandler::destroyMeshHost(Mesh* mesh)
{
    //clear doesnt free memory
    std::vector<Vertex>().swap(mesh->vertices);
    std::vector<u32>().swap(mesh->indices);
}

void AssetHandler::destroyMeshDevice(const Context& context, Mesh* mesh)
{
    mesh->combinedBuffer.destroy(context);
}

void AssetHandler::freeMesh(const Context& context, Mesh* mesh)
{
    if (mesh->isHostLoaded())
        destroyMeshHost(mesh);

    if (mesh->isDeviceLoaded())
        destroyMeshDevice(context, mesh);
}

//SHADERS

Asset AssetHandler::loadShader(const Context& context, std::string name)
{
    Asset asset = makeAsset();
    ShaderModule& shaderModule = m_shaders[asset];
    auto path = g_workingPath / g_shaderPath / name;

    shaderModule.byteCode = readFile(path.string());
    shaderModule.shaderModule = createShaderModule(context, shaderModule.byteCode);

    return asset;
}

ShaderModule* AssetHandler::getShader(Asset shader)
{
    return getFromUMap(shader, m_shaders);
}

void AssetHandler::freeShaderHost(Asset shader)
{
    ShaderModule* shaderMod = getShader(shader);

    CHECK_R(shaderMod, "Called host free on unloaded shader module");

    CHECK_R(shaderMod->isHostLoaded(), "Called host free on unloaded shader bytecode")

    destroyShaderHost(shaderMod);
}

void AssetHandler::freeShaderDevice(const Context& context, Asset shader)
{
    ShaderModule* shaderMod = getShader(shader);

    CHECK_R(shaderMod, "Called host free on unloaded shader module");

    CHECK_R(shaderMod->isDeviceLoaded(), "Called host free on unloaded VkShaderModule")

    destroyShaderDevice(context, shaderMod);
}

void AssetHandler::destroyShaderModule(const Context& context, Asset shader)
{
    ShaderModule* shaderMod = getShader(shader);

    CHECK_R(shaderMod, "destroy called on unaccounted texture")

    if (shaderMod->isHostLoaded())
        destroyShaderHost(shaderMod);

    if (shaderMod->isDeviceLoaded())
        destroyShaderDevice(context, shaderMod);

    m_shaders.erase(shader);
    m_fragments.push_back(shader);
}

void AssetHandler::destroyShaderHost(ShaderModule* shader)
{
    std::vector<char>().swap(shader->byteCode);
}

void AssetHandler::destroyShaderDevice(const Context& context, ShaderModule* shader)
{
    vkDestroyShaderModule(context.device, shader->shaderModule, nullptr);
}

void AssetHandler::freeShader(const Context& context, ShaderModule* shaderMod)
{
    if (shaderMod->isHostLoaded())
        destroyShaderHost(shaderMod);

    if (shaderMod->isDeviceLoaded())
        destroyShaderDevice(context, shaderMod);
}

//OTHER

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

} // Plunksna