#pragma once

#include <glm/glm.hpp>

#include "Core/Math/GridCoord.h"
#include "Core/Math/Vector2.h"

// Camara ortografica 2D con paneo suavizado (lerp exponencial hacia
// m_targetPosition) y zoom. Ver motor_grafico_clases.puml: no incluye
// viewport en el diagrama, pero getViewProjectionMatrix()/screenToWorld()
// necesitan conocer el tamano de la ventana para ser correctas, asi que
// se anade setViewportSize() (mismo criterio ya usado para Window, que
// tampoco aparece en el diagrama).
class Camera {
public:
    Camera(int viewportWidth, int viewportHeight);

    // Avanza m_position hacia m_targetPosition (lerp exponencial, suaviza
    // el paneo en vez de saltar de golpe). Sin efecto si ya coinciden.
    void update(float deltaTime);

    // Desplaza el punto de destino del paneo; update() lo alcanza con
    // suavizado en los frames siguientes.
    void move(const Vector2& delta);

    void setZoom(float zoom);
    void setViewportSize(int width, int height);

    const Vector2& position() const { return m_position; }
    float zoom() const { return m_zoom; }

    // Matriz combinada vista*proyeccion para el shader (mundo -> NDC).
    // Y invertida a proposito: en este motor "abajo en pantalla" = +y en
    // mundo (misma convencion que stbi_set_flip_vertically_on_load en
    // TextureManager y que IsoMath), asi que hay que voltear para que
    // OpenGL (NDC +y = arriba) pinte en el sentido correcto.
    glm::mat4 getViewProjectionMatrix() const;

    // Punto de pantalla (pixeles, origen arriba-izquierda) -> mundo,
    // teniendo en cuenta paneo y zoom actuales. Inversa exacta de "que
    // pixel de pantalla ocupa este punto de mundo".
    Vector2 screenToWorld(const Vector2& screenPos) const;

    // Mundo -> celda de grid isometrico (ver IsoMath). tileWidth/Height
    // son parametros con valor por defecto (64x32, tile 2:1 tipico) hasta
    // que TileMap exista y sea quien fije el tamano real del atlas
    // (Fase 2).
    GridCoord worldToGrid(const Vector2& worldPos, float tileWidth = 64.0f,
                          float tileHeight = 32.0f) const;

private:
    Vector2 m_position;
    Vector2 m_targetPosition;
    float m_zoom = 1.0f;
    float m_lerpSpeed = 8.0f;
    int m_viewportWidth;
    int m_viewportHeight;
};
