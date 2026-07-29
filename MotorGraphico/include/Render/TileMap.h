#pragma once

#include <string>
#include <vector>

#include "Core/Errors/Result.h"
#include "Core/Math/GridCoord.h"
#include "Core/Math/Vector2.h"
#include "Render/Tile.h"

// Mapa de tiles cargado desde TMX (formato de Tiled, XML). Varias capas
// (m_layers[layer][y][x]) para separar fondo/props/colision (Fase 2 del
// Gantt). screenToGrid/gridToScreen reutilizan IsoMath (mismas formulas
// que Camera::worldToGrid).
//
// Subconjunto de TMX soportado (lo suficiente para mapas dibujados a
// mano en Tiled sin funcionalidad avanzada; ver TileMap.cpp):
//  - <tileset> EMBEBIDO (sin "source" a un .tsx externo).
//  - <layer><data encoding="csv"> unicamente (ni XML plano ni
//    base64/zlib, que Tiled tambien puede generar).
//  - Colision por tile: <tile id="N"><properties><property
//    name="collision" type="bool" value="true"/></properties></tile>
//    dentro del <tileset>.
class TileMap {
public:
    // Nunca lanza: cualquier fallo de parseo (XML invalido, encoding no
    // soportado, numero de celdas que no cuadra...) se captura y se
    // convierte en Result<bool>::Error con un mensaje descriptivo, igual
    // que ResourceManager<T>::load() con loadFromDisk().
    Result<bool> loadFromFile(const std::string& path);

    int getWidth() const { return m_width; }
    int getHeight() const { return m_height; }
    int getLayerCount() const { return static_cast<int>(m_layers.size()); }

    // Lanza std::out_of_range si layer/x/y estan fuera de rango: un
    // indice invalido aqui es un bug del llamador (ver
    // IsometricRenderer, Fase 3), no un fallo "esperable" de I/O como
    // loadFromFile().
    const Tile& getTile(int layer, int x, int y) const;
    Tile& getTile(int layer, int x, int y);

    GridCoord screenToGrid(const Vector2& screenPos) const;
    Vector2 gridToScreen(const GridCoord& grid) const;

private:
    void parseOrThrow(const std::string& path);

    std::vector<std::vector<std::vector<Tile>>> m_layers;  // [layer][y][x]
    int m_width = 0;
    int m_height = 0;
    int m_tileWidth = 0;
    int m_tileHeight = 0;
};
