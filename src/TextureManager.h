#ifndef TEXTURE_MANAGER_H
#define TEXTURE_MANAGER_H

#include <string>
#include <vector>

class TextureManager
{
public:
    /** @brief Construct empty texture manager. */
    TextureManager();
    /** @brief Load texture from disk and store its OpenGL texture id. */
    void loadTexture(const std::string &path);
    /** @brief Bind all loaded textures to consecutive texture units. */
    void bindTextures() const;

private:
    /** Texture handles owned by this manager. */
    std::vector<unsigned int> m_TextureIDs;
};

#endif // TEXTURE_MANAGER_H
