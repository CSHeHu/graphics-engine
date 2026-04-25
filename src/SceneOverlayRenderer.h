#ifndef SCENE_OVERLAY_RENDERER_H
#define SCENE_OVERLAY_RENDERER_H

#include <string>

#include "SceneDefinition.h"

class TextManager;

/** @brief Renders scene text and optional runtime overlay stats. */
class SceneOverlayRenderer
{
  public:
    /** @brief Draw configured static text and selected runtime stats. */
    void render(TextManager& textRenderer, const SceneDefinition& definition,
                const UIOverlayConfig& overlayConfig, bool infoOverlayEnabled,
                float fps, float sceneElapsedTime,
                float currentTimeSeconds) const;

  private:
    /** @brief Format one runtime stat line selected by token name. */
    static std::string buildStatLine(const std::string& stat,
                                     const SceneDefinition& definition,
                                     float fps, float sceneElapsedTime,
                                     float currentTimeSeconds);
};

#endif // SCENE_OVERLAY_RENDERER_H
