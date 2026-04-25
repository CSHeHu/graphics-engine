#include "CameraRouteController.h"

#include <cmath>

#include "Camera.h"

void CameraRouteController::apply(Camera& camera,
                                  const CameraRouteDefinition& route,
                                  float sceneElapsedTimeSeconds) const
{
    const std::vector<CameraKeyframe>& keyframes = route.keyframes;
    if (keyframes.empty())
    {
        return;
    }

    if (keyframes.size() == 1)
    {
        camera.SetPoseLookAt(keyframes.front().position,
                             keyframes.front().lookAt);
        return;
    }

    const float routeEnd = keyframes.back().timeSeconds;
    if (routeEnd <= 0.0f)
    {
        camera.SetPoseLookAt(keyframes.back().position,
                             keyframes.back().lookAt);
        return;
    }

    float routeTime = sceneElapsedTimeSeconds;
    if (route.loop)
    {
        routeTime = std::fmod(routeTime, routeEnd);
    }
    else if (routeTime >= routeEnd)
    {
        camera.SetPoseLookAt(keyframes.back().position,
                             keyframes.back().lookAt);
        return;
    }

    for (std::size_t i = 0; i + 1 < keyframes.size(); ++i)
    {
        const CameraKeyframe& a = keyframes[i];
        const CameraKeyframe& b = keyframes[i + 1];
        if (routeTime >= a.timeSeconds && routeTime <= b.timeSeconds)
        {
            const float segmentDuration = b.timeSeconds - a.timeSeconds;
            const float t = segmentDuration > 0.0f
                                ? (routeTime - a.timeSeconds) / segmentDuration
                                : 0.0f;
            const glm::vec3 position = lerpVec3(a.position, b.position, t);
            const glm::vec3 lookAt   = lerpVec3(a.lookAt, b.lookAt, t);
            camera.SetPoseLookAt(position, lookAt);
            return;
        }
    }

    camera.SetPoseLookAt(keyframes.back().position, keyframes.back().lookAt);
}

glm::vec3 CameraRouteController::lerpVec3(const glm::vec3& a,
                                          const glm::vec3& b, float t)
{
    return a + (b - a) * t;
}
