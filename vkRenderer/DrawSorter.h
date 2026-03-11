//
// Created by d on 3/8/26.
//

#ifndef DRAWSORTER_H
#define DRAWSORTER_H
#include "assetHandler/Asset.h"
#include <glm/mat4x4.hpp>

#include "Camera.h"
#include "ShaderObjects.h"

namespace Plunksna {

struct DrawMeshCommand
{
    Asset mesh;
    glm::mat4 model;
    u32 textureID;
};

//immediate mode draw sorter, clear all commands after each frame
class DrawSorter {
public:
    //add a draw command
    void drawMesh(DrawMeshCommand command);

    //cull any meshes not within camera frustum
    void cullFrustum(Camera* camera);

    //reserve size amount in the final draw command buffer if needed (per mesh instance)
    void reserve(u64 size);

    //get final draw commands to send to gpu
    std::unordered_map<Asset, std::vector<PerObjectSO>>& getDrawCommands();

    //get object SOs from sorted draw calls
    std::vector<PerObjectSO>& getFinalObjects();

    //clear draw command buffers
    void clearAll();

    //clear only the final draw object buffer
    void clearFinalObjects();
private:
    std::unordered_map<Asset, std::vector<PerObjectSO>> m_meshInstancedDraws;
    std::vector<PerObjectSO> m_finalObjects;
};

} // Plunksna

#endif //DRAWSORTER_H
