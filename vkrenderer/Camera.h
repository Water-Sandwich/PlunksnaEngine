//
// Created by d on 2/14/26.
//

#ifndef CAMERA_H
#define CAMERA_H
#include <glm/fwd.hpp>
#include <glm/vec3.hpp>

namespace Plunksna {

class Camera {
public:
    Camera(glm::vec3 position, glm::vec3 target);
    ~Camera() = default;

    void resize(float aspect);
    glm::mat4 getView() const;
    glm::mat4 getPerspective() const;

public:
    //position of camera
    glm::vec3 m_position;
    //normalized vector for pointing direction
    glm::vec3 m_direction;

    //vertical FoV
    float m_fovY = 70;
    float m_nearClip = 0.1f;
    float m_farClip = 10.0f;

private:
    float m_aspect = 4.0/3.0f;
};

} // Plunksna

#endif //CAMERA_H
