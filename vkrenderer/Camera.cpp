//
// Created by d on 2/14/26.
//

#include "Camera.h"

#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_transform.hpp>

namespace Plunksna {
Camera::Camera(glm::vec3 position, glm::vec3 target) :
    m_position(position), m_direction(target)
{

}

void Camera::resize(float aspect)
{
    m_aspect = aspect;
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
} // Plunksna