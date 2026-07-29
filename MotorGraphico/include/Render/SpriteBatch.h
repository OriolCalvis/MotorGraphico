#pragma once

#include <vector>

#include "Core/Errors/Result.h"
#include "Core/Math/UVRect.h"
#include "Core/Math/Vector2.h"

class Texture;

// Agrupa quads texturizados en un unico VBO dinamico y los dibuja con el
// minimo de draw calls: mientras la textura no cambie, submit() solo
// acumula vertices; en cuanto cambia (o al llamar end()), flush() sube el
// buffer a la GPU y emite un unico glDrawArrays.
//
// SpriteBatch NO posee shader ni textura: asume que el llamador (por
// ahora, examples/demo_textured_quad.cpp; en la Fase 3, IsometricRenderer)
// ya ha hecho shader->use() y ha fijado el uniform de la matriz antes de
// submit()/flush(). Simplifica la clase y evita acoplarla a Shader.
//
// Nota de rendimiento: flush() sube todo el buffer con glBufferData en
// cada llamada (reemplaza el contenido entero). Es sencillo y correcto,
// pero no la estrategia mas rapida (orphaning/persistent mapping);
// optimizar eso es un problema de la Fase 4, no de esta primera version.
class SpriteBatch {
public:
    SpriteBatch();
    ~SpriteBatch();

    SpriteBatch(const SpriteBatch&) = delete;
    SpriteBatch& operator=(const SpriteBatch&) = delete;

    // Vacia el batch y olvida la textura actual: siempre antes del primer
    // submit() de un frame.
    void begin();

    // Encola un quad en (pos, pos+size) con las UV de "uv" para "tex". Si
    // "tex" es distinta de la textura ya encolada, hace flush() primero
    // (un batch solo puede dibujar una textura a la vez).
    //
    // "size" no aparece en motor_grafico_clases.puml (el diagrama solo
    // lista "pos"): sin el, todo quad seria 1x1. Mismo criterio que
    // Camera::setViewportSize, ver su comentario.
    void submit(const Vector2& pos, const Vector2& size, const UVRect& uv, Texture* tex);

    // Sube el buffer acumulado a la GPU y dibuja. Ok(false) si no habia
    // nada que dibujar (batch vacio); Ok(true) tras un draw call real.
    Result<bool> flush();

    // Cierra el frame: hace flush() del ultimo batch pendiente.
    void end();

private:
    struct Vertex {
        Vector2 pos;
        Vector2 uv;
    };

    std::vector<Vertex> m_vertices;
    unsigned int m_vao = 0;
    unsigned int m_vbo = 0;
    Texture* m_currentTexture = nullptr;
};
