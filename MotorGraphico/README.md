# Motor Gráfico Isométrico Pixel Art (C++)

Implementación de `Core::Resources::ResourceManager<T>` y sus dependencias
(`Result<T>`, `EngineException`), el arranque de la Fase 1 del Gantt
(ventana + contexto OpenGL) y la cámara ortográfica + proyección
isométrica de esa misma fase, siguiendo el diagrama de clases del motor
(`motor_grafico_clases.puml`).

## Estructura

```
CMakeLists.txt

include/Core/Math/Vector2.h             Vector2, del diagrama de clases
include/Core/Math/GridCoord.h           Coordenada entera de grid
include/Core/Math/UVRect.h              Region UV de un atlas
include/Core/Math/IsoMath.h             Proyeccion isometrica 2:1 (grid<->screen), sin GL
include/Core/Errors/EngineException.h   Jerarquía de excepciones del motor
include/Core/Errors/Result.h            Result<T>, para operaciones recuperables
include/Core/Resources/ResourceManager.h  Template base genérico
include/Core/Resources/Texture.h        Wrapper RAII de textura GL
include/Core/Resources/TextureManager.h ResourceManager<Texture>
include/Core/Resources/Shader.h         Wrapper RAII de programa GLSL
include/Core/Resources/ShaderManager.h  ResourceManager<Shader>
include/Engine/Window.h                 Ventana + contexto OpenGL 3.3 (GLFW+GLAD)
include/Render/Camera.h                 Camara ortografica: paneo suavizado, zoom, iso (solo glm)

src/Core/Resources/Texture.cpp
src/Core/Resources/TextureManager.cpp
src/Core/Resources/Shader.cpp
src/Core/Resources/ShaderManager.cpp
src/Engine/Window.cpp
src/Render/Camera.cpp

examples/TextAsset.h              Recurso de prueba sin dependencias gráficas
examples/TextAssetManager.h/.cpp  ResourceManager<TextAsset>
examples/demo_resource_manager.cpp  Demo/test ejecutable (sin GL)
examples/demo_camera.cpp             Demo/test de Camera + IsoMath (sin GL, solo glm)
examples/sandbox_window.cpp          Demo de Window: abre ventana y pinta un color

```

`.github/workflows/{ci,release}.yml`, `.clang-format`, `.clang-tidy` y
`.pre-commit-config.yaml` viven en la raíz del repositorio (un nivel por
encima de este directorio), no aquí: GitHub Actions solo ejecuta workflows
ubicados en `.github/workflows/` en la raíz. Ver `../BRANCHING.md`.

## Qué está verificado y qué no

- **`ResourceManager<T>`, `Result<T>`, `EngineException`, y el ejemplo `TextAssetManager`/`demo_resource_manager.cpp` están compilados y
  ejecutados** en este mismo entorno (g++ 13, `-std=c++17 -Wall -Wextra`,
  cero warnings, todos los `assert()` pasan; también verificado con
  `-fsanitize=address,undefined`, limpio). Cubre: carga OK, cache por
  `id`, error controlado sin excepción visible para el llamador,
  `get()`/`contains()`/`unload()`/`clear()`.
- **`Camera` (paneo suavizado, zoom, `getViewProjectionMatrix()`,
  `screenToWorld()`, `worldToGrid()`) e `IsoMath` (proyección 2:1
  grid↔screen) están compilados y ejecutados** (`demo_camera.cpp`, target
  `motor_math`, solo depende de `glm` vía `FetchContent`): round-trip
  `screenToGrid(gridToScreen(g)) == g` exacto en una cuadrícula de prueba
  de 41×41 celdas, convergencia del lerp de paneo, e inversa exacta de
  `screenToWorld()` para un zoom≠1. Limpio también con
  `-fsanitize=address,undefined`, `clang-format` y `clang-tidy`.
- **`Texture`/`TextureManager`, `Shader`/`ShaderManager` y `Window` son código real** (llamadas GL reales vía GLAD, carga con `stb_image`,
  compilación GLSL con log de error, contexto OpenGL 3.3 Core vía GLFW),
  pero **siguen sin poder compilarse** porque requieren vendorizar GLAD y
  stb_image a mano (ver más abajo) y no hay forma de crear un contexto
  OpenGL real (display) en este entorno. Se revisaron a mano.

### Corrección aplicada: bug de move-semantics en `Texture`

El `Texture(Texture&&) = default` original **no** pone a cero `m_glID` en
el objeto de origen. Como el destructor llama a `glDeleteTextures(m_glID)`
sin comprobar más que `!= 0`, tras un move ambos objetos (el movido-desde
y el destino) creían poseer el mismo nombre de textura GL: los dos
destructores intentarían borrarlo → doble-free / comportamiento
indefinido. Se sustituyó por un move explícito que "roba" el recurso y
deja el origen en `m_glID = 0`, igual que `std::unique_ptr`.

## Compilar y ejecutar los demos verificados

```
mkdir build && cd build
cmake ..
cmake --build .
./demo_resource_manager
./demo_camera
```

`demo_camera` descarga `glm` vía `FetchContent` en la configuración de
CMake (requiere red la primera vez; luego queda cacheado en `build/`).

Sin `cmake` a mano, `demo_resource_manager` también compila directo (no
tiene dependencias externas):

```
g++ -std=c++17 -Wall -Wextra -Iinclude -o demo examples/demo_resource_manager.cpp examples/TextAssetManager.cpp
./demo
```

## Activar `motor_core` (Texture/Shader/Window) y `sandbox_window`

`CMakeLists.txt` los activa **automáticamente** en cuanto detecta:

1. **GLAD**: generar en <https://glad.dav1d.de/> con Language=C/C++,
   Specification=OpenGL, API gl=3.3, Profile=Core. Descomprimir en
   `third_party/glad/` de forma que quede
   `third_party/glad/include/glad/glad.h`,
   `third_party/glad/include/KHR/khrplatform.h`,
   `third_party/glad/src/glad.c`.
2. **stb_image**: descargar
   <https://raw.githubusercontent.com/nothings/stb/master/stb_image.h>
   en `third_party/stb/stb_image.h`.

GLFW se descarga sola vía `FetchContent` (glm ya se resuelve siempre,
lo use o no `motor_core`, ver más abajo). En cuanto los dos pasos
anteriores estén hechos:

```
mkdir build && cd build
cmake ..
cmake --build .
./sandbox_window
```

`sandbox_window` abre una ventana 1280×720, crea el contexto OpenGL 3.3
Core y pinta un color de fondo sólido cada frame — es la validación
mínima de la primera tarea de la Fase 1 ("Ventana + contexto OpenGL
3.3"). Ver "Próximo paso sugerido" más abajo para lo que falta de esa
fase.

## CI/CD

`ci.yml` y `release.yml` viven en `.github/workflows/` en la raíz del
repositorio (GitHub Actions solo ejecuta workflows ubicados ahí) y operan
con `working-directory: MotorGraphico` sobre este subdirectorio. Ver
[`../BRANCHING.md`](../BRANCHING.md) para el detalle de qué hace cada uno,
el modelo de ramas y el versionado.

## Próximo paso sugerido

De las cuatro tareas de la Fase 1 del Gantt, quedan:

- ~~Ventana + contexto OpenGL 3.3~~ (`Window`)
- ~~Cámara ortográfica con desplazamiento~~ (`Camera`, este cambio)
- ~~Grid isométrico de prueba~~ (`IsoMath`, verificado en `demo_camera.cpp`)
- **Quad texturizado con `GL_NEAREST`** — el único paso que sí necesita un
  contexto OpenGL real (`Texture`/`Shader`/`Window` ya existen; falta
  `SpriteBatch::submit()`/`flush()` del diagrama de clases y vendorizar
  GLAD + stb_image, ver más abajo).
