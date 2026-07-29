#pragma once

class SpriteBatch;

// Cualquier objeto que IsometricRenderer pueda encolar y dibujar
// (entidades por ahora; en el futuro, efectos/props sueltos). Separado
// de IUpdatable: no todo lo que se dibuja necesita logica por frame (y
// viceversa, ver motor_grafico_clases.puml).
class IRenderable {
public:
    virtual ~IRenderable() = default;

    virtual void render(SpriteBatch& batch) = 0;

    // Clave de ordenacion del Painter's Algorithm (ver
    // IsometricRenderer::sortQueue): a menor clave, se dibuja antes.
    virtual int getSortKey() const = 0;
};
