#pragma once

// Wrapper RAII sobre una textura de OpenGL. No posee el nombre GL hasta
// que se construye correctamente (ver TextureManager::loadFromDisk):
// no hay estados "a medio construir".
class Texture {
public:
    Texture(unsigned int glID, int width, int height)
        : m_glID(glID), m_width(width), m_height(height) {}

    ~Texture();

    Texture(const Texture&) = delete;
    Texture& operator=(const Texture&) = delete;
    Texture(Texture&&) = default;
    Texture& operator=(Texture&&) = default;

    void bind(unsigned int slot = 0) const;

    int getWidth() const { return m_width; }
    int getHeight() const { return m_height; }

private:
    unsigned int m_glID;
    int m_width;
    int m_height;
};
