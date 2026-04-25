#ifndef CAMERAROUTECONTROLLER_H
#define CAMERAROUTECONTROLLER_H

#include <glm.hpp>

#include "SceneDefinition.h"

class Camera;

class CameraRouteController
{
  public:
    /** @brief Apply scripted camera route at scene-local elapsed time. */
    void apply(Camera& camera, const CameraRouteDefinition& route,
               float sceneElapsedTimeSeconds) const;

  private:
    /** @brief Linear interpolation helper for camera path blending. */
    static glm::vec3 lerpVec3(const glm::vec3& a, const glm::vec3& b, float t);
};

#endif // CAMERAROUTECONTROLLER_H
