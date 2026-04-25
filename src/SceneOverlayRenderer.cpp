#include "SceneOverlayRenderer.h"

#include <cstdio>

#include "TextManager.h"

void SceneOverlayRenderer::render(TextManager& textRenderer,
                                  const SceneDefinition& definition,
                                  const UIOverlayConfig& overlayConfig,
                                  bool infoOverlayEnabled, float fps,
                                  float sceneElapsedTime,
                                  float currentTimeSeconds) const
{
    for (const TextDefinition& text : definition.texts)
    {
        textRenderer.renderText(text.text, text.x, text.y, text.scale,
                                text.color);
    }

    if (!(infoOverlayEnabled && overlayConfig.enabled))
    {
        return;
    }

    const glm::vec3 overlayColor(1.0f, 1.0f, 1.0f);
    float           yOffset = overlayConfig.y;

    for (const std::string& stat : overlayConfig.stats)
    {
        const std::string statLine =
            buildStatLine(stat, definition, fps, sceneElapsedTime,
                          currentTimeSeconds);
        if (statLine.empty())
        {
            continue;
        }

        textRenderer.renderText(statLine, overlayConfig.x, yOffset,
                                overlayConfig.scale, overlayColor);
        yOffset -= overlayConfig.lineSpacing;
    }
}

std::string SceneOverlayRenderer::buildStatLine(
    const std::string& stat, const SceneDefinition& definition, float fps,
    float sceneElapsedTime, float currentTimeSeconds)
{
    if (stat == "fps")
    {
        return "FPS: " + std::to_string(static_cast<int>(fps));
    }

    if (stat == "sceneName")
    {
        return "Scene: " + definition.name;
    }

    if (stat == "time")
    {
        int  totalMinutes = static_cast<int>(currentTimeSeconds) / 60;
        int  totalSeconds = static_cast<int>(currentTimeSeconds) % 60;
        char totalTimeBuffer[32];
        snprintf(totalTimeBuffer, sizeof(totalTimeBuffer), "Total: %d:%02d",
                 totalMinutes, totalSeconds);

        int  sceneMinutes = static_cast<int>(sceneElapsedTime) / 60;
        int  sceneSeconds = static_cast<int>(sceneElapsedTime) % 60;
        char sceneTimeBuffer[32];
        snprintf(sceneTimeBuffer, sizeof(sceneTimeBuffer), "Scene: %d:%02d",
                 sceneMinutes, sceneSeconds);

        return std::string(totalTimeBuffer) + " | " +
               std::string(sceneTimeBuffer);
    }

    if (stat == "objects")
    {
        std::string statLine = "Objects:";
        for (const SceneObjectDefinition& object : definition.objects)
        {
            statLine += " " + object.id;
        }
        return statLine;
    }

    if (stat == "shaders")
    {
        std::string statLine = "Shaders:";
        for (const MaterialDefinition& material : definition.materials)
        {
            statLine += " " + material.id;
        }
        return statLine;
    }

    return std::string();
}
