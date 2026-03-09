//
// Created by d on 3/8/26.
//

#include "DrawSorter.h"

namespace Plunksna {
void DrawSorter::drawMesh(DrawMeshCommand command)
{
    m_meshInstancedDraws[command.mesh].emplace_back(command.model, command.textureID);
}

void DrawSorter::cullFrustum(Camera* camera)
{

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

std::vector<PerObjectSO>& DrawSorter::getFinalObjects()
{
    u64 size = 0;
    for (auto& [mesh, list] : m_meshInstancedDraws) {
        size += list.size();
    }

    m_finalObjects.reserve(size);
    for (auto& [mesh, list] : m_meshInstancedDraws) {
        m_finalObjects.append_range(list);
    }

    return m_finalObjects;
}

void DrawSorter::clear()
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
} // Plunksna