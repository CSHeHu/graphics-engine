#ifndef CAMERA_H
#define CAMERA_H

#include <glad/glad.h>
#include <glm.hpp>
#include <gtc/matrix_transform.hpp>

// Defines several possible options for camera movement. Used as abstraction to
// stay away from window-system specific input methods
enum Camera_Movement { FORWARD, BACKWARD, LEFT, RIGHT, UP, DOWN };

// An abstract camera class that processes input and calculates the
// corresponding Euler Angles, Vectors and Matrices for use in OpenGL
class Camera {
public:

  // constructor with vectors
  Camera(glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f),
         glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f), float yaw = -90.0f,
         float pitch = 0.0f)
      : Front(glm::vec3(0.0f, 0.0f, -1.0f)), MovementSpeed(2.5f),
        MouseSensitivity(0.1f), Zoom(45.0f) {
    Position = position;
    WorldUp = up;
    Yaw = yaw;
    Pitch = pitch;
    updateCameraVectors();
  }
  // constructor with scalar values
  Camera(float posX, float posY, float posZ, float upX, float upY, float upZ,
         float yaw, float pitch)
      : Front(glm::vec3(0.0f, 0.0f, -1.0f)), MovementSpeed(2.5f),
        MouseSensitivity(0.1f), Zoom(45.0f) {
    Position = glm::vec3(posX, posY, posZ);
    WorldUp = glm::vec3(upX, upY, upZ);
    Yaw = yaw;
    Pitch = pitch;
    updateCameraVectors();
  }

  /** @brief Build the current view matrix. */
  glm::mat4 GetViewMatrix();

  /** @brief Move camera using keyboard direction and frame delta time. */
  void ProcessKeyboard(Camera_Movement direction, float deltaTime);

  /** @brief Rotate camera from mouse movement deltas. */
  void ProcessMouseMovement(float xoffset, float yoffset,
                            GLboolean constrainPitch = true);

  /** @brief Update camera zoom from mouse wheel delta. */
  void ProcessMouseScroll(float yoffset);

  /** @brief Set camera position and orient it to look at a target point. */
  void SetPoseLookAt(const glm::vec3 &position, const glm::vec3 &lookAtTarget);

  glm::vec3 getPosition() const;
  float getZoom() const;
  void setMovementSpeed(float movementSpeed);
  void setMouseSensitivity(float mouseSensitivity);
  void setZoom(float zoom);

private:
  /** @brief Recompute direction vectors from Euler angles. */
  void updateCameraVectors();

  // camera Attributes
  glm::vec3 Position;
  glm::vec3 Front;
  glm::vec3 Up;
  glm::vec3 Right;
  glm::vec3 WorldUp;
  
  // euler Angles
  float Yaw;
  float Pitch;
  // camera options
  float MovementSpeed;
  float MouseSensitivity;
  float Zoom;
};
#endif
