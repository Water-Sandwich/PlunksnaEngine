//
// Created by d on 2/14/26.
//

#include "Camera.h"

#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/gtc/matrix_access.hpp>

namespace Plunksna {
Camera::Camera(glm::vec3 position, glm::vec3 target) :
    m_position(position), m_direction(target)
{

}

void Camera::resize(f32 aspect)
{
    m_aspect = aspect;
}

glm::mat4 Camera::getModel() const
{
    glm::mat4 model = glm::mat4(1);
    model = glm::translate(model, m_position);

    float angle = atan2(m_direction.y, m_direction.x);
    model = glm::rotate(model, angle, glm::vec3(0,0,1));

    return model;
}

glm::mat4 Camera::getView() const
{
    return glm::lookAt(m_position, m_position + m_direction, glm::vec3(0,0,1));
}

glm::mat4 Camera::getPerspective() const
{
    auto proj = glm::perspective(glm::radians(m_fovY), m_aspect, m_nearClip, m_farClip);
    proj[1][1] *= -1;
    return proj;
}

std::array<glm::vec4, 6> Camera::getFrustumPlanes() const
{
    glm::mat4 VP = getPerspective() * getView();

    std::array<glm::vec4, 6> planes{};

    glm::vec4 row0 = glm::row(VP, 0);
    glm::vec4 row1 = glm::row(VP, 1);
    glm::vec4 row2 = glm::row(VP, 2);
    glm::vec4 row3 = glm::row(VP, 3);

    planes[0] = row3 + row0; // left
    planes[1] = row3 - row0; // right
    planes[2] = row3 + row1; // bottom
    planes[3] = row3 - row1; // top
    planes[4] = row3 + row2; // near
    planes[5] = row3 - row2; // far

    for (i32 i = 0; i < 6; i++) {
        float len = glm::length(glm::vec3(planes[i]));
        planes[i] /= len;
    }

    return planes;
}
} // Plunksna