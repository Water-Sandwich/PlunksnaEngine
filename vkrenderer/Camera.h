//
// Created by d on 2/14/26.
//

#ifndef CAMERA_H
#define CAMERA_H
#include <glm/fwd.hpp>
#include <glm/vec3.hpp>
#include "utils/Types.h"

namespace Plunksna {

class Camera {
public:
    Camera(glm::vec3 position, glm::vec3 target);
    ~Camera() = default;

    void resize(f32 aspect);
    glm::mat4 getView() const;
    glm::mat4 getPerspective() const;

public:
    //position of camera
    glm::vec3 m_position;
    //normalized vector for pointing direction
    glm::vec3 m_direction;

    //vertical FoV
    f32 m_fovY = 70;
    f32 m_nearClip = 0.1f;
    f32 m_farClip = 1000.0f;

private:
    f32 m_aspect = 4.0/3.0f;
};

} // Plunksna

#endif //CAMERA_H
