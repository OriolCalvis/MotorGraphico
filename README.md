# ResourceManager<T> — implementación

Implementación de `Core::Resources::ResourceManager<T>` y sus dependencias
(`Result<T>`, `EngineException`), siguiendo el diagrama de clases del motor.

## Estructura

```
include/Core/Errors/EngineException.h   Jerarquía de excepciones del motor
include/Core/Errors/Result.h            Result<T>, para operaciones recuperables
include/Core/Resources/ResourceManager.h  Template base genérico
include/Core/Resources/Texture.h        Wrapper RAII de textura GL
include/Core/Resources/TextureManager.h ResourceManager<Texture>
include/Core/Resources/Shader.h         Wrapper RAII de programa GLSL
include/Core/Resources/ShaderManager.h  ResourceManager<Shader>

src/Core/Resources/Texture.cpp
src/Core/Resources/TextureManager.cpp
src/Core/Resources/Shader.cpp
src/Core/Resources/ShaderManager.cpp

examples/TextAsset.h              Recurso de prueba sin dependencias gráficas
examples/TextAssetManager.h/.cpp  ResourceManager<TextAsset>
examples/demo_resource_manager.cpp  Demo/test ejecutable
```

## Qué está verificado y qué no

- **`ResourceManager<T>`, `Result<T>`, `EngineException`, y el ejemplo
  `TextAssetManager`/`demo_resource_manager.cpp` están compilados y
  ejecutados** en este mismo entorno (g++ 13, `-std=c++17 -Wall -Wextra`,
  cero warnings, todos los `assert()` pasan). Cubre: carga OK, cache por
  `id`, error controlado sin excepción visible para el llamador,
  `get()`/`contains()`/`unload()`/`clear()`.
- **`Texture`/`TextureManager` y `Shader`/`ShaderManager` son código real**
  (llamadas GL reales vía GLAD, carga con `stb_image`, compilación GLSL
  con log de error), pero **no se han podido compilar en este sandbox**
  porque no tiene GLFW/GLAD/stb_image/glm instalados. Se revisaron a mano
  (y se corrigió un error real: `glm::mat4` no se puede forward-declarar
  como `struct`, se usa `<glm/fwd.hpp>`). Compilarán en cuanto el CMake
  del proyecto real tenga esas dependencias.

## Compilar y ejecutar el demo verificado

```bash
mkdir build && cd build
cmake ..
cmake --build .
./demo_resource_manager
```

## Próximo paso sugerido

Cuando toque la Fase 1 del Gantt (ventana + contexto OpenGL), vendorizar
GLFW/GLAD/stb_image/glm en `third_party/` (o via `FetchContent`),
descomentar el target `motor_core` en `CMakeLists.txt`, y `Texture`/
`Shader` deberían compilar sin más cambios.
