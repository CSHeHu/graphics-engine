#ifndef SHADOWMANAGER_H
#define SHADOWMANAGER_H

#include <array>
#include <glad/glad.h>
#include <glm.hpp>
#include <memory>

#include "SceneDefinition.h"

class AssetManager;
class Shader;

class ShadowManager
{
  public:
    ShadowManager();
    ~ShadowManager();

    bool init(AssetManager& assets, const ShadowConfig& config);
    void shutdown();

    bool beginDepthPass(const glm::vec3& lightPosition,
                        unsigned int     updateIntervalFrames);
    void submitDepthRenderable(const glm::mat4& model, unsigned int vao,
                               int indexCount);
    void endDepthPass();

    void bindShadowTexture(int textureUnit) const;

    bool  isReady() const;
    float getFarPlane() const;

  private:
    bool initResources(AssetManager& assets);
    void releaseResources();
    std::array<glm::mat4, 6>
    buildShadowCubeMatrices(const glm::vec3& lightPosition) const;

    ShadowConfig            config;
    std::shared_ptr<Shader> shadowDepthShader;
    unsigned int            shadowFramebuffer  = 0;
    unsigned int            shadowDepthTexture = 0;
    unsigned int            shadowFrameCounter = 0;
    bool                    shadowCacheValid   = false;

    bool  depthPassActive     = false;
    GLint previousViewport[4] = {0, 0, 0, 0};
    bool  cullFaceWasEnabled  = false;
};

#endif // SHADOWMANAGER_H
