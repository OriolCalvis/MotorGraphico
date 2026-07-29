#pragma once

#include "Core/Math/GridCoord.h"
#include "Core/Math/Vector2.h"
#include "Render/IRenderable.h"
#include "Render/IUpdatable.h"

class TextureAtlas;
class SpriteBatch;

// Objeto dibujable posicionado en el grid isometrico (jugador, enemigo,
// prop...). AnimatedEntity/Player/Enemy (motor_grafico_clases.puml) son
// subclases pendientes; Entity ya implementa render()/getSortKey() para
// que cualquier subclase solo tenga que resolver update().
//
// tileWidth/tileHeight (no estan en el diagrama de clases, igual criterio
// que Camera::setViewportSize) son necesarios para convertir
// m_gridPosition a pixeles: deben coincidir con los del TileMap sobre el
// que se renderiza la entidad.
class Entity : public IRenderable, public IUpdatable {
public:
    Entity(GridCoord gridPosition, int spriteID, TextureAtlas* atlas, int tileWidth = 64,
           int tileHeight = 32);

    // Resuelve m_gridPosition+m_offset a pantalla (IsoMath::gridToScreen)
    // y encola un quad del tamano de un tile en "batch" con el UVRect de
    // m_spriteID (TextureAtlas::getUV). No dibuja de verdad hasta que
    // alguien llama a batch.end()/flush() (ver IsometricRenderer).
    void render(SpriteBatch& batch) override;

    // Painter's Algorithm (ver IsometricRenderer::sortQueue): profundidad
    // = (grid_x + grid_y) * tileHeight/2, con grid_x como desempate. Se
    // codifican ambos en un unico int (profundidad domina, ver .cpp)
    // porque IRenderable::getSortKey() solo puede devolver un valor.
    int getSortKey() const override;

    void setGridPosition(const GridCoord& pos) { m_gridPosition = pos; }
    const GridCoord& gridPosition() const { return m_gridPosition; }

    void setOffset(const Vector2& offset) { m_offset = offset; }
    const Vector2& offset() const { return m_offset; }

protected:
    GridCoord m_gridPosition;
    Vector2 m_offset;
    int m_spriteID;
    TextureAtlas* m_atlas;

private:
    int m_tileWidth;
    int m_tileHeight;
};
