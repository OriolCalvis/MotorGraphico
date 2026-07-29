#include "Render/Entity.h"

#include "Core/Math/IsoMath.h"
#include "Core/Resources/TextureAtlas.h"
#include "Render/SpriteBatch.h"

namespace {
// Mayor que cualquier ancho/alto de mapa razonable: la profundidad
// domina el orden, m_gridPosition.x solo desempata entre celdas con la
// misma profundidad (misma diagonal grid_x+grid_y). Ver Entity::getSortKey.
constexpr int kSortKeyMultiplier = 1 << 16;
}  // namespace

Entity::Entity(GridCoord gridPosition, int spriteID, TextureAtlas* atlas, int tileWidth,
               int tileHeight)
    : m_gridPosition(gridPosition)
    , m_spriteID(spriteID)
    , m_atlas(atlas)
    , m_tileWidth(tileWidth)
    , m_tileHeight(tileHeight) {}

void Entity::render(SpriteBatch& batch) {
    Vector2 basePos = IsoMath::gridToScreen(m_gridPosition, static_cast<float>(m_tileWidth),
                                            static_cast<float>(m_tileHeight));
    Vector2 finalPos = basePos + m_offset;
    Vector2 size{static_cast<float>(m_tileWidth), static_cast<float>(m_tileHeight)};

    batch.submit(finalPos, size, m_atlas->getUV(m_spriteID), m_atlas->texture());
}

int Entity::getSortKey() const {
    int depth = (m_gridPosition.x + m_gridPosition.y) * (m_tileHeight / 2);
    return depth * kSortKeyMultiplier + m_gridPosition.x;
}
