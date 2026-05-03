#ifndef CAMERA_H
#define CAMERA_H

#include <glad/glad.h>
#include <glm.hpp>
#include <gtc/matrix_transform.hpp>

// Defines several possible options for camera movement. Used as abstraction to
// stay away from window-system specific input methods
enum Camera_Movement
{
    FORWARD,
    BACKWARD,
    LEFT,
    RIGHT,
    UP,
    DOWN
};

// An abstract camera class that processes input and calculates the
// corresponding Euler Angles, Vectors and Matrices for use in OpenGL
class Camera
{
    public:
        /**
         * @brief Construct a camera with position and up vectors, yaw, and
         * pitch.
         * @param position Initial position of the camera.
         * @param up Up direction vector.
         * @param yaw Initial yaw angle.
         * @param pitch Initial pitch angle.
         */
        Camera(glm::vec3 position, glm::vec3 up, float yaw, float pitch);

        /**
         * @brief Construct a camera with scalar values for position and up,
         * yaw, and pitch.
         * @param posX X position.
         * @param posY Y position.
         * @param posZ Z position.
         * @param upX X component of up vector.
         * @param upY Y component of up vector.
         * @param upZ Z component of up vector.
         * @param yaw Initial yaw angle.
         * @param pitch Initial pitch angle.
         */
        Camera(float posX, float posY, float posZ, float upX, float upY,
               float upZ, float yaw, float pitch);

        /**
         * @brief Build the current view matrix.
         * @return View matrix for the camera.
         */
        glm::mat4 GetViewMatrix();

        /**
         * @brief Move camera using keyboard direction and frame delta time.
         * @param direction Movement direction (FORWARD, BACKWARD, etc).
         * @param deltaTime Frame delta time.
         */
        void ProcessKeyboard(Camera_Movement direction, float deltaTime);

        /**
         * @brief Rotate camera from mouse movement deltas.
         * @param xoffset Mouse X offset.
         * @param yoffset Mouse Y offset.
         * @param constrainPitch Whether to constrain pitch angle.
         */
        void ProcessMouseMovement(float xoffset, float yoffset,
                                  GLboolean constrainPitch = true);

        /**
         * @brief Update camera zoom from mouse wheel delta.
         * @param yoffset Mouse wheel Y offset.
         */
        void ProcessMouseScroll(float yoffset);

        /**
         * @brief Set camera position and orient it to look at a target point.
         * @param position New camera position.
         * @param lookAtTarget Target point to look at.
         */
        void SetPoseLookAt(const glm::vec3& position,
                           const glm::vec3& lookAtTarget);

        /**
         * @brief Get the camera position.
         * @return The position vector of the camera.
         */
        glm::vec3 getPosition() const;
        /**
         * @brief Get the camera zoom (field of view).
         * @return The zoom value.
         */
        float getZoom() const;
        /**
         * @brief Set the camera movement speed.
         * @param movementSpeed The new movement speed.
         */
        void setMovementSpeed(float movementSpeed);
        /**
         * @brief Set the mouse sensitivity for camera rotation.
         * @param mouseSensitivity The new mouse sensitivity.
         */
        void setMouseSensitivity(float mouseSensitivity);
        /**
         * @brief Set the camera zoom (field of view).
         * @param zoom The new zoom value.
         */
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
