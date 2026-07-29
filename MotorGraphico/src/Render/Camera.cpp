#include "Render/Camera.h"

#include <algorithm>

#include <glm/gtc/matrix_transform.hpp>

#include "Core/Math/IsoMath.h"

Camera::Camera(int viewportWidth, int viewportHeight)
    : m_viewportWidth(viewportWidth), m_viewportHeight(viewportHeight) {}

void Camera::update(float deltaTime) {
    float t = std::clamp(m_lerpSpeed * deltaTime, 0.0f, 1.0f);
    m_position = m_position + (m_targetPosition - m_position) * t;
}

void Camera::move(const Vector2& delta) { m_targetPosition = m_targetPosition + delta; }

void Camera::setZoom(float zoom) { m_zoom = std::max(zoom, 0.01f); }

void Camera::setViewportSize(int width, int height) {
    m_viewportWidth = std::max(width, 1);
    m_viewportHeight = std::max(height, 1);
}

glm::mat4 Camera::getViewProjectionMatrix() const {
    float halfW = static_cast<float>(m_viewportWidth) / (2.0f * m_zoom);
    float halfH = static_cast<float>(m_viewportHeight) / (2.0f * m_zoom);

    // bottom=halfH, top=-halfH: ver comentario de Y invertida en el .h.
    glm::mat4 projection = glm::ortho(-halfW, halfW, halfH, -halfH, -1.0f, 1.0f);
    glm::mat4 view = glm::translate(glm::mat4(1.0f), glm::vec3(-m_position.x, -m_position.y, 0.0f));
    return projection * view;
}

Vector2 Camera::screenToWorld(const Vector2& screenPos) const {
    Vector2 centered{
        screenPos.x - static_cast<float>(m_viewportWidth) * 0.5f,
        screenPos.y - static_cast<float>(m_viewportHeight) * 0.5f,
    };
    return m_position + centered * (1.0f / m_zoom);
}

GridCoord Camera::worldToGrid(const Vector2& worldPos, float tileWidth, float tileHeight) const {
    return IsoMath::screenToGrid(worldPos, tileWidth, tileHeight);
}
