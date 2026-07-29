# Motor Gráfico Isométrico Pixel Art (C++)

Implementación de `Core::Resources::ResourceManager<T>` y sus dependencias
(`Result<T>`, `EngineException`), la **Fase 1 completa del Gantt**
(ventana + contexto OpenGL, cámara ortográfica, proyección isométrica y
quad texturizado con `GL_NEAREST`) y la mayor parte de la **Fase 2**
(carga de atlas de tiles, capas de mapa, parser de mapas TMX), siguiendo
el diagrama de clases del motor (`motor_grafico_clases.puml`).

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
include/Render/SpriteBatch.h            Agrupa quads texturizados en un VBO, minimiza draw calls
include/Render/Tile.h                   Celda de TileMap: GID, colision, variante
include/Render/TileMap.h                Mapa de tiles cargado desde TMX (Tiled), varias capas
include/Core/Resources/TextureAtlas.h   Recorta subregiones (UVRect) de un atlas de tiles

src/Core/Resources/Texture.cpp
src/Core/Resources/TextureManager.cpp
src/Core/Resources/TextureAtlas.cpp
src/Core/Resources/Shader.cpp
src/Core/Resources/ShaderManager.cpp
src/Engine/Window.cpp
src/Render/Camera.cpp
src/Render/SpriteBatch.cpp
src/Render/TileMap.cpp

examples/TextAsset.h              Recurso de prueba sin dependencias gráficas
examples/TextAssetManager.h/.cpp  ResourceManager<TextAsset>
examples/demo_resource_manager.cpp  Demo/test ejecutable (sin GL)
examples/demo_camera.cpp             Demo/test de Camera + IsoMath (sin GL, solo glm)
examples/sandbox_window.cpp          Demo de Window: abre ventana y pinta un color
examples/demo_textured_quad.cpp      Fase 1 completa: Window+Camera+SpriteBatch dibujando un quad
examples/demo_tilemap.cpp            Fase 2: TileMap+TextureAtlas sobre un TMX real (sin GL)

assets/shaders/sprite.{vert,frag}    Shader minimo de demo_textured_quad
assets/textures/test_checker.png     Textura de prueba (8x8, tablero rojo/amarillo)
assets/maps/test_map.tmx             Mapa TMX de prueba (4x3, 1 capa, colision en 2 celdas)
assets/maps/test_map_invalid.tmx     Fixture de TMX invalido, para probar Result::Error

third_party/glad/     GLAD (loader de OpenGL 3.3 Core), vendorizado y comprobado en el repo
third_party/stb/      stb_image.h (carga de PNG/JPG...), vendorizado y comprobado en el repo
third_party/tinyxml2/ tinyxml2 (parser XML, para TMX), vendorizado y comprobado en el repo

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
- **`Texture`/`TextureManager`, `Shader`/`ShaderManager`, `Window`,
  `SpriteBatch` y el quad texturizado completo están compilados,
  enlazados y ejecutados con un contexto OpenGL 3.3 Core real**
  (`demo_textured_quad.cpp`, sobre Xvfb + Mesa/llvmpipe software
  rendering, ya que este entorno no tiene GPU/display): `GL_VERSION`
  reportado es `4.5 (Core Profile) Mesa ...`; se carga
  `assets/textures/test_checker.png` (8×8, confirmado por
  `Texture::getWidth()/getHeight()`), se compila/enlaza
  `assets/shaders/sprite.{vert,frag}`, y se vuelca el framebuffer final a
  un PPM que se inspeccionó pixel a pixel: la esquina tiene el color de
  `glClearColor` y el centro el color exacto de la textura — confirma que
  el pipeline completo (Window → Camera → SpriteBatch → Shader →
  Texture) dibuja lo que debería, no solo que "compila". Limpio con
  ASan+UBSan (con `detect_leaks=0` solo para esta demo: las fugas que
  reporta LeakSanitizer son de la cache interna de Mesa/llvmpipe — todos
  los frames del stack están dentro del driver, no de este código).
- **`TileMap` (parser TMX real vía tinyxml2), `Tile` y `TextureAtlas`
  están compilados y ejecutados** (`demo_tilemap.cpp`, target
  `motor_map`; necesita GLAD para el destructor de `Texture` pero NO
  GLFW/ventana, así que corre sin Xvfb): carga
  `assets/maps/test_map.tmx` (tileset embebido, capa CSV, colisión por
  tile vía `<properties>`) y comprueba celda a celda GID y colisión;
  `gridToScreen`/`screenToGrid` con el `tilewidth`/`tileheight` del
  propio TMX; `getTile()` fuera de rango lanza `std::out_of_range`; un
  TMX con menos celdas de las declaradas (`test_map_invalid.tmx`) y un
  fichero inexistente devuelven `Result<bool>::Error` en vez de
  crashear. `TextureAtlas::defineRegion()`/`getUV()` verificados sobre
  un atlas 16×8 con dos regiones de 8×8. Limpio con ASan+UBSan
  (detección de fugas activada: aquí sí, no toca GL de verdad).

### Corrección aplicada: bug de move-semantics en `Texture`

El `Texture(Texture&&) = default` original **no** pone a cero `m_glID` en
el objeto de origen. Como el destructor llama a `glDeleteTextures(m_glID)`
sin comprobar más que `!= 0`, tras un move ambos objetos (el movido-desde
y el destino) creían poseer el mismo nombre de textura GL: los dos
destructores intentarían borrarlo → doble-free / comportamiento
indefinido. Se sustituyó por un move explícito que "roba" el recurso y
deja el origen en `m_glID = 0`, igual que `std::unique_ptr`.

## Compilar y ejecutar los demos verificados

GLAD y stb_image ya están vendorizados en `third_party/` (comprobados en
el repo, ver "GLAD/stb_image vendorizados" más abajo), así que
`motor_core` se activa solo. En Linux hacen falta los headers de
desarrollo de X11/GL que usa GLFW (`sudo apt install xorg-dev
libgl1-mesa-dev`, o el equivalente de tu distro):

```
mkdir build && cd build
cmake ..
cmake --build .
./demo_resource_manager
./demo_camera
./demo_tilemap             # sin GL real: no necesita Xvfb
./sandbox_window           # ventana interactiva: se cierra a mano
./demo_textured_quad       # ventana interactiva: se cierra a mano
```

`demo_camera` y `sandbox_window`/`demo_textured_quad` (via `motor_core`)
descargan `glm` y `GLFW` respectivamente vía `FetchContent` en la
configuración de CMake (requiere red la primera vez; luego queda
cacheado en `build/`).

Sin `cmake` a mano, `demo_resource_manager` también compila directo (no
tiene dependencias externas):

```
g++ -std=c++17 -Wall -Wextra -Iinclude -o demo examples/demo_resource_manager.cpp examples/TextAssetManager.cpp
./demo
```

### Sin display (CI, contenedores, SSH sin X)

`demo_textured_quad` acepta un número de frames como argumento: corre
exactamente esos frames, vuelca el framebuffer a
`demo_textured_quad_output.ppm` y sale solo, sin esperar a que se cierre
la ventana — así es como lo ejecuta `ci.yml`. Con `Xvfb` (pantalla
virtual) y renderizado por software:

```
sudo apt install xvfb
LIBGL_ALWAYS_SOFTWARE=1 xvfb-run -a ./build/demo_textured_quad 5
```

### GLAD/stb_image/tinyxml2 vendorizados

`third_party/glad/`, `third_party/stb/` y `third_party/tinyxml2/` están
en el repositorio (no son un paso manual de cada dev). Se generaron/
descargaron una vez con:

```
pip install glad
python -m glad --profile core --api "gl=3.3" --generator c --out-path third_party/glad
curl -o third_party/stb/stb_image.h https://raw.githubusercontent.com/nothings/stb/master/stb_image.h
curl -o third_party/tinyxml2/tinyxml2.h https://raw.githubusercontent.com/leethomason/tinyxml2/master/tinyxml2.h
curl -o third_party/tinyxml2/tinyxml2.cpp https://raw.githubusercontent.com/leethomason/tinyxml2/master/tinyxml2.cpp
```

Si algún día hace falta regenerarlos (nueva versión de OpenGL, otra API,
actualizar tinyxml2), son esos comandos; `CMakeLists.txt` los detecta
automáticamente por la presencia de `third_party/glad/src/glad.c` y
`third_party/stb/stb_image.h`, sin tocar el propio `CMakeLists.txt`.

## CI/CD

`ci.yml` y `release.yml` viven en `.github/workflows/` en la raíz del
repositorio (GitHub Actions solo ejecuta workflows ubicados ahí) y operan
con `working-directory: MotorGraphico` sobre este subdirectorio. Ver
[`../BRANCHING.md`](../BRANCHING.md) para el detalle de qué hace cada uno,
el modelo de ramas y el versionado.

## Próximo paso sugerido

**Fase 1 del Gantt completa**: las cuatro tareas (ventana + contexto
OpenGL, cámara ortográfica, grid isométrico de prueba, quad texturizado
con `GL_NEAREST`) están implementadas y verificadas end-to-end en
`demo_textured_quad.cpp`.

**Fase 2 (`motor_grafico_gantt.puml`): 4 de 5 tareas hechas.**
`ResourceManager<T>` (ya existía), `Carga de atlas de tiles`
(`TextureAtlas`), `Sistema de capas del mapa` (`TileMap::m_layers`,
varias capas) y `Parser de mapas TMX` (`TileMap::loadFromFile`, vía
tinyxml2) — todo verificado en `demo_tilemap.cpp`.

Queda **`Culling y batching estatico`**: dado un `Camera` (viewport +
posición) y un `TileMap`, calcular el rango de celdas visible para no
recorrer/dibujar el mapa entero cada frame. Es la pieza que conecta
`TileMap`+`TextureAtlas` con el renderizado real (`SpriteBatch`), así que
tiene sentido abordarla junto con el arranque de la Fase 3
(`IsometricRenderer`, `Sprite batching dinámico`, `Painter's Algorithm`
— todos ya en `motor_grafico_clases.puml`), en vez de aislada.
