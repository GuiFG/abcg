#include "camera.hpp"

#include <algorithm>
#include <glm/gtc/matrix_transform.hpp>
#include <limits>

void Camera::resizeViewport(glm::ivec2 const &size) { m_viewportSize = size; }

void Camera::mouseScroll(float scroll) {
  // Compute forward vector (view direction)
  auto const forward{glm::normalize(m_at - m_eye)};

  m_eye += forward * scroll / 10.0f;
  m_at += forward * scroll / 10.0f;
}

void Camera::computeProjectionMatrix(glm::vec2 const &size) {
  m_projMatrix = glm::mat4(1.0f);
  auto const aspect{size.x / size.y};
  m_projMatrix = glm::perspective(glm::radians(45.0f), aspect, 0.1f, 100.0f);
}

glm::vec3 Camera::project(glm::vec2 const &position) const {
  // Convert from window coordinates to NDC
  auto projected{glm::vec3(
      2.0f * position.x / gsl::narrow<float>(m_viewportSize.x) - 1.0f,
      1.0f - 2.0f * position.y / gsl::narrow<float>(m_viewportSize.y), 0.0f)};

  // Project to centered unit hemisphere
  if (auto const squaredLength{glm::length2(projected)};
      squaredLength >= 1.0f) {
    // Outside the sphere
    projected = glm::normalize(projected);
  } else {
    // Inside the sphere
    projected.z = std::sqrt(1.0f - squaredLength);
  }

  return projected;
}

void Camera::computeViewMatrix() {
  m_viewMatrix = glm::lookAt(m_eye, m_at, m_up);
}

void Camera::dolly(float speed) {
  // Compute forward vector (view direction)
  auto const forward{glm::normalize(m_at - m_eye)};

  // Move eye and center forward (speed > 0) or backward (speed < 0)
  m_eye += forward * speed;
  m_at += forward * speed;

  computeViewMatrix();
}

void Camera::truck(float speed) {
  // Compute forward vector (view direction)
  auto const forward{glm::normalize(m_at - m_eye)};
  // Compute vector to the left
  auto const left{glm::cross(m_up, forward)};

  // Move eye and center to the left (speed < 0) or to the right (speed > 0)
  m_at -= left * speed;
  m_eye -= left * speed;

  computeViewMatrix();
}

void Camera::pan(float speed) {
  glm::mat4 transform{1.0f};

  // Rotate camera around its local y axis
  transform = glm::translate(transform, m_eye);
  transform = glm::rotate(transform, -speed, m_up);
  transform = glm::translate(transform, -m_eye);

  m_at = transform * glm::vec4(m_at, 1.0f);

  computeViewMatrix();
}

void Camera::vertical(float speed) {
  // Moves the camera vertically
  // Compute up vector
  auto const up{glm::normalize(m_up)};

  // Move eye and center upward (speed > 0) or downward (speed < 0)
  m_eye += up * speed;
  m_at += up * speed;

  computeViewMatrix();
}