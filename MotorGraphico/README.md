# Motor Gráfico Isométrico Pixel Art (C++)

Implementación de `Core::Resources::ResourceManager<T>` y sus dependencias
(`Result<T>`, `EngineException`), más el arranque de la Fase 1 del Gantt
(ventana + contexto OpenGL), siguiendo el diagrama de clases del motor
(`motor_grafico_clases.puml`).

## Estructura

```
CMakeLists.txt

include/Core/Math/Vector2.h             Vector2, del diagrama de clases
include/Core/Math/GridCoord.h           Coordenada entera de grid
include/Core/Math/UVRect.h              Region UV de un atlas
include/Core/Errors/EngineException.h   Jerarquía de excepciones del motor
include/Core/Errors/Result.h            Result<T>, para operaciones recuperables
include/Core/Resources/ResourceManager.h  Template base genérico
include/Core/Resources/Texture.h        Wrapper RAII de textura GL
include/Core/Resources/TextureManager.h ResourceManager<Texture>
include/Core/Resources/Shader.h         Wrapper RAII de programa GLSL
include/Core/Resources/ShaderManager.h  ResourceManager<Shader>
include/Engine/Window.h                 Ventana + contexto OpenGL 3.3 (GLFW+GLAD)

src/Core/Resources/Texture.cpp
src/Core/Resources/TextureManager.cpp
src/Core/Resources/Shader.cpp
src/Core/Resources/ShaderManager.cpp
src/Engine/Window.cpp

examples/TextAsset.h              Recurso de prueba sin dependencias gráficas
examples/TextAssetManager.h/.cpp  ResourceManager<TextAsset>
examples/demo_resource_manager.cpp  Demo/test ejecutable (sin GL)
examples/sandbox_window.cpp          Demo de Window: abre ventana y pinta un color

.github/workflows/ci.yml       (antes en la raíz: GitHub Actions solo ejecuta
.github/workflows/release.yml   workflows dentro de .github/workflows/)
```

## Qué está verificado y qué no

- **`ResourceManager<T>`, `Result<T>`, `EngineException`, y el ejemplo `TextAssetManager`/`demo_resource_manager.cpp` están compilados y
  ejecutados** en este mismo entorno (g++ 13, `-std=c++17 -Wall -Wextra`,
  cero warnings, todos los `assert()` pasan; también verificado con
  `-fsanitize=address,undefined`, limpio). Cubre: carga OK, cache por
  `id`, error controlado sin excepción visible para el llamador,
  `get()`/`contains()`/`unload()`/`clear()`.
- **`Texture`/`TextureManager`, `Shader`/`ShaderManager` y `Window` son código real** (llamadas GL reales vía GLAD, carga con `stb_image`,
  compilación GLSL con log de error, contexto OpenGL 3.3 Core vía GLFW),
  pero **no se han podido compilar en este sandbox** porque no tiene
  GLFW/GLAD/stb_image/glm instalados ni acceso de red para descargarlos.
  Se revisaron a mano.

### Corrección aplicada: bug de move-semantics en `Texture`

El `Texture(Texture&&) = default` original **no** pone a cero `m_glID` en
el objeto de origen. Como el destructor llama a `glDeleteTextures(m_glID)`
sin comprobar más que `!= 0`, tras un move ambos objetos (el movido-desde
y el destino) creían poseer el mismo nombre de textura GL: los dos
destructores intentarían borrarlo → doble-free / comportamiento
indefinido. Se sustituyó por un move explícito que "roba" el recurso y
deja el origen en `m_glID = 0`, igual que `std::unique_ptr`.

## Compilar y ejecutar el demo verificado

```
mkdir build && cd build
cmake ..
cmake --build .
./demo_resource_manager
```

Sin `cmake` a mano, también compila directo:

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

GLFW y glm se descargan solos vía `FetchContent` (no requieren paso
manual). En cuanto los dos pasos anteriores estén hechos:

```
mkdir build && cd build
cmake ..
cmake --build .
./sandbox_window
```

`sandbox_window` abre una ventana 1280×720, crea el contexto OpenGL 3.3
Core y pinta un color de fondo sólido cada frame — es la validación
mínima de la primera tarea de la Fase 1 ("Ventana + contexto OpenGL
3.3"). Las siguientes tareas de esa fase (quad texturizado, cámara
ortográfica, grid isométrico de prueba) son el próximo paso.

## CI/CD

`ci.yml` y `release.yml` estaban en la raíz del repositorio: GitHub
Actions **solo** ejecuta workflows que viven en `.github/workflows/`, así
que nunca se habían disparado. Se movieron a esa ruta sin cambiar su
contenido. Ver `BRANCHING.md` para el detalle de qué hace cada uno.

## Próximo paso sugerido

Completar la Fase 1 del Gantt sobre `Window`: quad texturizado con
`GL_NEAREST`, cámara ortográfica con desplazamiento (usando ya
`Vector2`/`GridCoord`), y un grid isométrico de prueba (array 2D). El
diagrama de clases (`motor_grafico_clases.puml`) ya especifica `Camera`,
`SpriteBatch` y `TileMap` para cuando toque esa parte.
