#include "Core/Math/IsoMath.h"
#include "Render/Camera.h"

#include <cassert>
#include <cmath>
#include <iostream>

namespace {

bool nearlyEqual(float a, float b, float epsilon = 0.001f) { return std::fabs(a - b) <= epsilon; }

}  // namespace

int main() {
    const float kTileWidth = 64.0f;
    const float kTileHeight = 32.0f;

    // Caso 1: IsoMath::gridToScreen / screenToGrid son inversas exactas en
    // una cuadricula de prueba (grid isometrico de prueba, Fase 1 del
    // Gantt). tileWidth/tileHeight son potencias de dos: la ida y vuelta
    // no acumula error de redondeo en float.
    int roundTripFailures = 0;
    for (int gx = -20; gx <= 20; ++gx) {
        for (int gy = -20; gy <= 20; ++gy) {
            GridCoord original{gx, gy};
            Vector2 screen = IsoMath::gridToScreen(original, kTileWidth, kTileHeight);
            GridCoord back = IsoMath::screenToGrid(screen, kTileWidth, kTileHeight);
            if (back.x != original.x || back.y != original.y) {
                ++roundTripFailures;
            }
        }
    }
    assert(roundTripFailures == 0);
    std::cout << "[ISO] round-trip grid<->screen en 41x41 celdas: " << roundTripFailures
              << " fallos (esperado: 0)\n";

    // Sanity check del signo/orientacion: (1,0) cae a la derecha y abajo
    // del origen; (0,1) a la izquierda y abajo (proyeccion 2:1 estandar).
    Vector2 east = IsoMath::gridToScreen(GridCoord{1, 0}, kTileWidth, kTileHeight);
    Vector2 southwest = IsoMath::gridToScreen(GridCoord{0, 1}, kTileWidth, kTileHeight);
    assert(east.x > 0.0f && east.y > 0.0f);
    assert(southwest.x < 0.0f && southwest.y > 0.0f);
    std::cout << "[ISO] orientacion (1,0)=(" << east.x << "," << east.y << ") (0,1)=("
              << southwest.x << "," << southwest.y << ")\n";

    // Caso 2: Camera::update() suaviza el paneo hacia move() con lerp
    // exponencial, sin overshoot, y converge en un numero finito de pasos.
    Camera camera(800, 600);
    camera.move(Vector2{100.0f, 50.0f});
    assert(nearlyEqual(camera.position().x, 0.0f) && nearlyEqual(camera.position().y, 0.0f));

    for (int i = 0; i < 200 && !nearlyEqual(camera.position().x, 100.0f); ++i) {
        camera.update(1.0f / 60.0f);
    }
    assert(nearlyEqual(camera.position().x, 100.0f, 0.5f));
    assert(nearlyEqual(camera.position().y, 50.0f, 0.5f));
    std::cout << "[CAMERA] tras converger, position() = (" << camera.position().x << ", "
              << camera.position().y << ") (esperado: ~100, ~50)\n";

    // Caso 3: screenToWorld() es la inversa exacta de "que pixel de
    // pantalla ocupa este punto de mundo" para el zoom/posicion actuales.
    camera.setZoom(2.0f);
    Vector2 worldPoint{140.0f, 70.0f};  // position() + (20, 20)
    Vector2 screenPoint{
        800.0f / 2.0f + (worldPoint.x - camera.position().x) * camera.zoom(),
        600.0f / 2.0f + (worldPoint.y - camera.position().y) * camera.zoom(),
    };
    Vector2 recovered = camera.screenToWorld(screenPoint);
    assert(nearlyEqual(recovered.x, worldPoint.x));
    assert(nearlyEqual(recovered.y, worldPoint.y));
    std::cout << "[CAMERA] screenToWorld(screenToWorld^-1(w)) = (" << recovered.x << ", "
              << recovered.y << ") (esperado: 140, 70)\n";

    // Caso 4: Camera::worldToGrid delega en IsoMath con el tile por
    // defecto (64x32): un punto exactamente sobre la celda (3,-2) debe
    // resolver a esa celda.
    GridCoord target{3, -2};
    Vector2 targetWorld = IsoMath::gridToScreen(target, kTileWidth, kTileHeight);
    GridCoord resolved = camera.worldToGrid(targetWorld);
    assert(resolved.x == target.x && resolved.y == target.y);
    std::cout << "[CAMERA] worldToGrid(gridToScreen(3,-2)) = (" << resolved.x << ", " << resolved.y
              << ") (esperado: 3, -2)\n";

    std::cout << "\nTodas las comprobaciones (assert) han pasado correctamente.\n";
    return 0;
}
