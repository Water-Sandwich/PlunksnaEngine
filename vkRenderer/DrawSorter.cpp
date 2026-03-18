//
// Created by d on 3/8/26.
//

#include "DrawSorter.h"

#include "assetHandler/Mesh.h"
#include "engine/Log.h"
#include "tracy/Tracy.hpp"

namespace Plunksna {
void DrawSorter::setAssets(AssetHandler* assets)
{
    m_assets = assets;
}

void DrawSorter::drawMesh(DrawMeshCommand command)
{
    m_meshInstancedDraws[command.mesh].emplace_back(command.model, command.textureID);
}

void DrawSorter::cullFrustum(Camera* camera)
{
    auto planes = camera->getFrustumPlanes();

    for (auto& [meshHnd, list] : m_meshInstancedDraws) {
        const Mesh& mesh = *m_assets->getMesh(meshHnd);

        for (u32 i = 0; i < list.size(); i++) {
            glm::vec3 center = glm::vec3(list[i].model * glm::vec4(mesh.cullSphere.offset, 1.0f));
            f32 distance = distanceToPlanes(planes, center);

            //object is visible
            if (distance > -mesh.cullSphere.radius * 2)
                continue;

            //list.erase(list.begin() + i);
            list[i] = list.back();
            list.pop_back();
        }
    }
}

void DrawSorter::reserve(u64 size)
{
    for (auto& [mesh, list] : m_meshInstancedDraws) {
        list.reserve(size);
    }
}

std::unordered_map<Asset, std::vector<PerObjectSO>>& DrawSorter::getDrawCommands()
{
    return m_meshInstancedDraws;
}

std::vector<PerObjectSO>& DrawSorter::getFinalObjects(Camera* camera)
{
    u64 size = 0;
    for (auto& [mesh, list] : m_meshInstancedDraws) {
        size += list.size();
    }

    cullFrustum(camera);

    m_finalObjects.reserve(size);
    for (auto& [mesh, list] : m_meshInstancedDraws) {
        m_finalObjects.append_range(list);
    }

    return m_finalObjects;
}

void DrawSorter::clearAll()
{
    for (auto& [mesh, list] : m_meshInstancedDraws) {
        list.clear();
    }

    clearFinalObjects();
}

void DrawSorter::clearFinalObjects()
{
    m_finalObjects.clear();
}

f32 DrawSorter::distanceToPlane(glm::vec4 plane, glm::vec3 point)
{
    return glm::dot(glm::vec4(point, 1), plane);
}

f32 DrawSorter::distanceToPlanes(const std::array<glm::vec4, 6>& planes, glm::vec3 point)
{
    f32 dist = INFINITY;

    for (int i = 0; i < 6; i++)
    {
        f32 d = glm::dot(glm::vec3(planes[i]), point) + planes[i].w;
        if (d < dist)
            dist = d;
    }

    return dist;
}

} // Plunksna