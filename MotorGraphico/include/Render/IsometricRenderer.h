#pragma once

#include <vector>

#include "Core/Errors/Result.h"
#include "Render/SpriteBatch.h"

class Camera;
class TileMap;
class TextureAtlas;
class Shader;
class IRenderable;

// Orquesta un frame completo: fondo (TileMap, con culling, Fase 2) +
// entidades (IRenderable, con Painter's Algorithm, Fase 3), todo sobre un
// unico SpriteBatch. No posee Camera/TileMap/Shader (punteros no
// propietarios: los gestiona quien construya el renderer, ver
// demo_isometric_renderer.cpp), pero si el SpriteBatch (por valor, como
// en el diagrama de clases) y el TextureAtlas del tileset del mapa.
//
// m_atlas no aparece en motor_grafico_clases.puml (igual criterio que
// Camera::setViewportSize/Entity::tileWidth): sin el, renderLayer() no
// tiene forma de resolver Tile::getTilesetID() a un UVRect.
class IsometricRenderer {
public:
    IsometricRenderer(Camera* camera, TileMap* map, TextureAtlas* atlas, Shader* shader);

    void addToQueue(IRenderable* obj);

    // Solo lectura, no esta en el diagrama de clases: hook minimo para
    // verificar sortQueue() (y en el futuro, herramientas de debug/HUD)
    // sin tener que inspeccionar pixeles de un framebuffer.
    const std::vector<IRenderable*>& renderQueue() const { return m_renderQueue; }

    // Painter's Algorithm: ordena m_renderQueue por IRenderable::getSortKey()
    // ascendente (a menor clave, mas al fondo). Se llama automaticamente
    // desde renderFrame(); publico tambien para poder verificar el orden
    // en tests sin necesitar un frame de render real.
    void sortQueue();

    // Nunca lanza: cualquier fallo (por ahora, punteros nulos) se
    // convierte en Result<bool>::Error via RenderException, mismo patron
    // que TileMap::loadFromFile()/ResourceManager<T>::load().
    Result<bool> renderFrame();

private:
    void renderLayer(int layerIndex);
    void applyPostProcessing();

    Camera* m_camera;
    SpriteBatch m_spriteBatch;
    TileMap* m_map;
    TextureAtlas* m_atlas;
    std::vector<IRenderable*> m_renderQueue;
    Shader* m_shader;
};
