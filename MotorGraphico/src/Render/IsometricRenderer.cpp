#include "Render/IsometricRenderer.h"

#include "Core/Errors/EngineException.h"
#include "Core/Resources/Shader.h"
#include "Core/Resources/TextureAtlas.h"
#include "Render/Camera.h"
#include "Render/IRenderable.h"
#include "Render/TileMap.h"

#include <algorithm>

IsometricRenderer::IsometricRenderer(Camera* camera, TileMap* map, TextureAtlas* atlas,
                                     Shader* shader)
    : m_camera(camera), m_map(map), m_atlas(atlas), m_shader(shader) {}

void IsometricRenderer::addToQueue(IRenderable* obj) { m_renderQueue.push_back(obj); }

void IsometricRenderer::sortQueue() {
    std::sort(m_renderQueue.begin(), m_renderQueue.end(),
              [](const IRenderable* a, const IRenderable* b) {
                  return a->getSortKey() < b->getSortKey();
              });
}

void IsometricRenderer::renderLayer(int layerIndex) {
    GridBounds visible = m_map->visibleRange(*m_camera);
    if (visible.isEmpty()) {
        return;  // culling: nada de esta capa cae dentro del viewport
    }

    Vector2 tileSize{static_cast<float>(m_map->getTileWidth()),
                     static_cast<float>(m_map->getTileHeight())};

    for (int y = visible.minY; y <= visible.maxY; ++y) {
        for (int x = visible.minX; x <= visible.maxX; ++x) {
            const Tile& tile = m_map->getTile(layerIndex, x, y);
            if (tile.isEmpty()) {
                continue;
            }
            Vector2 pos = m_map->gridToScreen(GridCoord{x, y});
            m_spriteBatch.submit(pos, tileSize, m_atlas->getUV(tile.getTilesetID()),
                                 m_atlas->texture());
        }
    }
}

void IsometricRenderer::applyPostProcessing() {
    // Pendiente (Fase 4 del Gantt: iluminacion, niebla de guerra,
    // paletizado -- ver motor_grafico_dafo.md). No-op deliberado: mejor
    // un metodo vacio y documentado que fingir un efecto a medias.
}

Result<bool> IsometricRenderer::renderFrame() {
    try {
        if (m_camera == nullptr || m_map == nullptr || m_atlas == nullptr || m_shader == nullptr) {
            throw RenderException("Camera/TileMap/TextureAtlas/Shader nulo");
        }

        m_shader->use();
        m_shader->setUniformMat4("uViewProjection", m_camera->getViewProjectionMatrix());
        m_shader->setUniformInt("uTexture", 0);

        m_spriteBatch.begin();
        for (int layer = 0; layer < m_map->getLayerCount(); ++layer) {
            renderLayer(layer);
        }

        sortQueue();
        for (IRenderable* obj : m_renderQueue) {
            obj->render(m_spriteBatch);
        }
        m_spriteBatch.end();

        applyPostProcessing();
    } catch (const std::exception& e) {
        return Result<bool>::Error(std::string("Error en IsometricRenderer::renderFrame(): ") +
                                   e.what());
    }
    return Result<bool>::Ok(true);
}
