#include "ShadowManager.h"

#include <algorithm>
#include <glad/glad.h>
#include <gtc/matrix_transform.hpp>
#include <string>

#include "AssetManager.h"
#include "Shader.h"

ShadowManager::ShadowManager() = default;

ShadowManager::~ShadowManager()
{
    shutdown();
}

bool ShadowManager::init(AssetManager& assets, const ShadowConfig& shadowConfig)
{
    shutdown();
    config             = shadowConfig;
    shadowFrameCounter = 0;
    shadowCacheValid   = false;
    return initResources(assets);
}

void ShadowManager::shutdown()
{
    releaseResources();
    depthPassActive = false;
}

bool ShadowManager::isReady() const
{
    return shadowFramebuffer != 0 && shadowDepthTexture != 0 &&
           shadowDepthShader != nullptr;
}

float ShadowManager::getFarPlane() const
{
    return config.farPlane;
}

bool ShadowManager::initResources(AssetManager& assets)
{
    shadowDepthShader = assets.getShader("assets/shaders/shadowDepth.vs",
                                         "assets/shaders/shadowDepth.fs",
                                         "assets/shaders/shadowDepth.gs");
    if (!shadowDepthShader)
    {
        return false;
    }

    glGenFramebuffers(1, &shadowFramebuffer);
    glGenTextures(1, &shadowDepthTexture);

    glBindTexture(GL_TEXTURE_CUBE_MAP, shadowDepthTexture);
    for (int face = 0; face < 6; ++face)
    {
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + face, 0,
                     GL_DEPTH_COMPONENT, config.mapSize, config.mapSize, 0,
                     GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
    }
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    glBindFramebuffer(GL_FRAMEBUFFER, shadowFramebuffer);
    glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
                         shadowDepthTexture, 0);
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);

    const bool isComplete =
        (glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    return isComplete;
}

void ShadowManager::releaseResources()
{
    if (shadowFramebuffer != 0)
    {
        glDeleteFramebuffers(1, &shadowFramebuffer);
        shadowFramebuffer = 0;
    }

    if (shadowDepthTexture != 0)
    {
        glDeleteTextures(1, &shadowDepthTexture);
        shadowDepthTexture = 0;
    }

    shadowDepthShader.reset();
}

std::array<glm::mat4, 6>
ShadowManager::buildShadowCubeMatrices(const glm::vec3& lightPosition) const
{
    const glm::mat4 shadowProjection =
        glm::perspective(glm::radians(config.fovDegrees), 1.0f,
                         config.nearPlane, config.farPlane);

    return {shadowProjection *
                glm::lookAt(lightPosition,
                            lightPosition + glm::vec3(1.0f, 0.0f, 0.0f),
                            glm::vec3(0.0f, -1.0f, 0.0f)),
            shadowProjection *
                glm::lookAt(lightPosition,
                            lightPosition + glm::vec3(-1.0f, 0.0f, 0.0f),
                            glm::vec3(0.0f, -1.0f, 0.0f)),
            shadowProjection *
                glm::lookAt(lightPosition,
                            lightPosition + glm::vec3(0.0f, 1.0f, 0.0f),
                            glm::vec3(0.0f, 0.0f, 1.0f)),
            shadowProjection *
                glm::lookAt(lightPosition,
                            lightPosition + glm::vec3(0.0f, -1.0f, 0.0f),
                            glm::vec3(0.0f, 0.0f, -1.0f)),
            shadowProjection *
                glm::lookAt(lightPosition,
                            lightPosition + glm::vec3(0.0f, 0.0f, 1.0f),
                            glm::vec3(0.0f, -1.0f, 0.0f)),
            shadowProjection *
                glm::lookAt(lightPosition,
                            lightPosition + glm::vec3(0.0f, 0.0f, -1.0f),
                            glm::vec3(0.0f, -1.0f, 0.0f))};
}

bool ShadowManager::beginDepthPass(const glm::vec3& lightPosition,
                                   unsigned int     updateIntervalFrames)
{
    if (!isReady())
    {
        return false;
    }

    const unsigned int safeInterval = std::max(updateIntervalFrames, 1u);
    const bool         shouldUpdate =
        !shadowCacheValid || (shadowFrameCounter++ % safeInterval == 0);
    if (!shouldUpdate)
    {
        return false;
    }

    glGetIntegerv(GL_VIEWPORT, previousViewport);

    glViewport(0, 0, config.mapSize, config.mapSize);
    glBindFramebuffer(GL_FRAMEBUFFER, shadowFramebuffer);
    glClear(GL_DEPTH_BUFFER_BIT);

    cullFaceWasEnabled = glIsEnabled(GL_CULL_FACE);
    if (!cullFaceWasEnabled)
    {
        glEnable(GL_CULL_FACE);
    }
    glCullFace(GL_FRONT);

    shadowDepthShader->use();
    const std::array<glm::mat4, 6> shadowMatrices =
        buildShadowCubeMatrices(lightPosition);
    for (std::size_t face = 0; face < shadowMatrices.size(); ++face)
    {
        shadowDepthShader->setMat4("shadowMatrices[" + std::to_string(face) +
                                       "]",
                                   shadowMatrices[face]);
    }
    shadowDepthShader->setVec3("lightPos", lightPosition);
    shadowDepthShader->setFloat("farPlane", config.farPlane);

    depthPassActive = true;
    return true;
}

void ShadowManager::submitDepthRenderable(const glm::mat4& model,
                                          unsigned int vao, int indexCount)
{
    if (!depthPassActive || !shadowDepthShader)
    {
        return;
    }

    shadowDepthShader->setMat4("model", model);
    glBindVertexArray(vao);
    glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, nullptr);
}

void ShadowManager::endDepthPass()
{
    if (!depthPassActive)
    {
        return;
    }

    glCullFace(GL_BACK);
    if (!cullFaceWasEnabled)
    {
        glDisable(GL_CULL_FACE);
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glViewport(previousViewport[0], previousViewport[1], previousViewport[2],
               previousViewport[3]);

    shadowCacheValid = true;
    depthPassActive  = false;
}

void ShadowManager::bindShadowTexture(int textureUnit) const
{
    if (shadowDepthTexture == 0)
    {
        return;
    }

    glActiveTexture(GL_TEXTURE0 + textureUnit);
    glBindTexture(GL_TEXTURE_CUBE_MAP, shadowDepthTexture);
}
