#include "camera.hpp"

#include <algorithm>
#include <glm/gtc/matrix_transform.hpp>
#include <limits>

void Camera::mouseMove(glm::ivec2 const &position) {
  // Rotates camera by dragging mouse on the window
  if (!m_mouseTracking)
    return;

  auto const currentPosition{project(position)};

  if (m_lastPosition == currentPosition)
    return;

  // Rotation axis
  m_axis = glm::cross(m_lastPosition, currentPosition);
  m_axis = glm::normalize(m_axis);

  // Rotation angle
  auto const angle{glm::length(m_axis)};

  // Concatenate rotation: R_old = R_new * R_old
  m_rotation = glm::rotate(glm::mat4(1.0f), angle, m_axis) * m_rotation;

  m_lastPosition = currentPosition;
}

void Camera::mousePress(glm::ivec2 const &position) {
  // start tracking the mouse movement when left button is pressed
  m_lastPosition = project(position);
  m_mouseTracking = true;
}

void Camera::mouseRelease(glm::ivec2 const &position) {
  // stop tracking the mouse moovement when left buttom is released
  mouseMove(position);
  m_mouseTracking = false;
}

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
  m_projMatrix = glm::perspective(glm::radians(70.0f), aspect, 0.1f, 5.0f);
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