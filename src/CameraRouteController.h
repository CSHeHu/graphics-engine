#ifndef CAMERAROUTECONTROLLER_H
#define CAMERAROUTECONTROLLER_H

#include <glm.hpp>

#include "SceneDefinition.h"

class Camera;

/**
 * @brief Controls scripted camera routes and applies them to a Camera instance.
 */
class CameraRouteController
{
    public:
        /**
         * @brief Apply a scripted camera route at a given scene-local elapsed
         * time.
         * @param camera Camera instance to modify.
         * @param route Camera route definition to follow.
         * @param sceneElapsedTimeSeconds Elapsed time in the scene (seconds).
         */
        void apply(Camera& camera, const CameraRouteDefinition& route,
                   float sceneElapsedTimeSeconds) const;

    private:
        /**
         * @brief Linear interpolation helper for camera path blending.
         * @param a Start vector.
         * @param b End vector.
         * @param t Interpolation factor [0, 1].
         * @return Interpolated vector between a and b.
         */
        glm::vec3 lerpVec3(const glm::vec3& a, const glm::vec3& b,
                           float t) const;
};

#endif // CAMERAROUTECONTROLLER_H
