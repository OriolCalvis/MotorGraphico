cmake_minimum_required(VERSION 3.16)
project(MotorGraficoIsometrico CXX C)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(${CMAKE_SOURCE_DIR}/include)

# assets/ (shaders, texturas, mapas TMX) se referencia con rutas relativas
# desde los ejecutables (ver demo_textured_quad.cpp, demo_tilemap.cpp):
# se copia junto al binario para que funcionen ejecutados desde build/.
file(COPY ${CMAKE_SOURCE_DIR}/assets DESTINATION ${CMAKE_BINARY_DIR})

# --- Demo sin dependencias externas: prueba ResourceManager<T> de forma
# aislada. Compila y pasa en cualquier maquina con g++/clang++ C++17. ---
add_executable(demo_resource_manager
    examples/demo_resource_manager.cpp
    examples/TextAssetManager.cpp
)

# --- Camera + IsoMath: solo necesitan glm (header-only, sin GL). Se
# separan de motor_core (que si necesita GLAD/stb_image/GLFW vendorizados,
# ver mas abajo) para poder compilar y probar la camara y la proyeccion
# isometrica sin ventana ni contexto OpenGL real. ---
include(FetchContent)
FetchContent_Declare(
    glm
    GIT_REPOSITORY https://github.com/g-truc/glm.git
    GIT_TAG 1.0.1
    GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(glm)

add_library(motor_math src/Render/Camera.cpp)
target_link_libraries(motor_math PUBLIC glm::glm)

add_executable(demo_camera examples/demo_camera.cpp)
target_link_libraries(demo_camera PRIVATE motor_math)

# GLAD y stb_image estan vendorizados en third_party/ (comprobados en el
# repositorio, no son un paso manual): GLAD no tiene soporte CMake
# oficial tipo FetchContent (requiere un paso de generacion desde su XML
# de especificacion) y stb_image es header-only. Se generaron una vez
# con:
#
#   pip install glad
#   python -m glad --profile core --api "gl=3.3" --generator c \
#       --out-path third_party/glad
#   curl -o third_party/stb/stb_image.h \
#       https://raw.githubusercontent.com/nothings/stb/master/stb_image.h
#
# (el propio proyecto necesita un .cpp que haga
#  #define STB_IMAGE_IMPLEMENTATION antes de incluir stb_image.h una
#  unica vez; ver stb_image_impl.cpp mas abajo). Los "if EXISTS" de abajo
# se dejan como red de seguridad (por si alguien borra third_party/ a
# mano), no como paso esperado.
set(GLAD_DIR ${CMAKE_SOURCE_DIR}/third_party/glad)
set(STB_DIR ${CMAKE_SOURCE_DIR}/third_party/stb)

# --- Fase 2: TileMap/Tile/TextureAtlas (mapas y tiles). Solo necesitan
# tinyxml2 (vendorizado en third_party/, igual que GLAD/stb_image: XML de
# un solo .h/.cpp, mas simple vendorizarlo que un FetchContent) para
# parsear TMX, y motor_math (Camera, para TileMap::visibleRange). Nada de
# GLAD ni GLFW: TextureAtlas solo usa los getters inline de Texture.h
# (ancho/alto), no su implementacion .cpp (que si necesita GLAD). Por eso
# se pueden compilar y probar sin X11 ni Xvfb (ver demo_tilemap.cpp). ---
set(TINYXML2_DIR ${CMAKE_SOURCE_DIR}/third_party/tinyxml2)

add_library(motor_map
    src/Core/Resources/TextureAtlas.cpp
    src/Render/TileMap.cpp
    ${TINYXML2_DIR}/tinyxml2.cpp
)
target_include_directories(motor_map PUBLIC ${TINYXML2_DIR})
target_link_libraries(motor_map PUBLIC motor_math)

if (EXISTS ${GLAD_DIR}/src/glad.c)
    # demo_tilemap.cpp SI construye un Texture real (aunque con glID=0,
    # ver su comentario), asi que el ejecutable -- no motor_map -- es
    # quien necesita Texture.cpp/glad.c. Mantenerlos fuera de motor_map
    # evita compilarlos dos veces si algo linka motor_map junto a
    # motor_core (que ya trae su propia copia), lo que daria simbolos
    # duplicados en el enlazado.
    add_executable(demo_tilemap
        examples/demo_tilemap.cpp
        src/Core/Resources/Texture.cpp
        ${GLAD_DIR}/src/glad.c
    )
    target_include_directories(demo_tilemap PRIVATE ${GLAD_DIR}/include)
    target_link_libraries(demo_tilemap PRIVATE motor_map)
else ()
    message(STATUS "demo_tilemap omitido: falta third_party/glad.")
endif ()

if (EXISTS ${GLAD_DIR}/src/glad.c AND EXISTS ${STB_DIR}/stb_image.h)
    set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
    set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
    set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
    if (UNIX AND NOT APPLE)
        # Solo X11: evita depender de wayland-client/xkbcommon (no
        # instalados por defecto en CI) cuando solo hace falta X11/Xvfb.
        set(GLFW_BUILD_WAYLAND OFF CACHE BOOL "" FORCE)
    endif ()
    FetchContent_Declare(
        glfw
        GIT_REPOSITORY https://github.com/glfw/glfw.git
        GIT_TAG 3.4
        GIT_SHALLOW TRUE
    )
    FetchContent_MakeAvailable(glfw)

    # third_party/stb_image_impl.cpp: unico .cpp del proyecto donde se
    # define STB_IMAGE_IMPLEMENTATION (stb_image.h es header-only y esa
    # macro debe aparecer en una unica unidad de compilacion).
    file(WRITE ${CMAKE_BINARY_DIR}/stb_image_impl.cpp
        "#define STB_IMAGE_IMPLEMENTATION\n#include <stb_image.h>\n")

    add_library(motor_core
        src/Core/Resources/Texture.cpp
        src/Core/Resources/TextureManager.cpp
        src/Core/Resources/Shader.cpp
        src/Core/Resources/ShaderManager.cpp
        src/Engine/Window.cpp
        src/Render/SpriteBatch.cpp
        src/Render/Entity.cpp
        src/Render/IsometricRenderer.cpp
        ${GLAD_DIR}/src/glad.c
        ${CMAKE_BINARY_DIR}/stb_image_impl.cpp
    )
    target_include_directories(motor_core PUBLIC
        ${GLAD_DIR}/include
        ${STB_DIR}
    )
    # motor_map (TileMap/TextureAtlas) publico: IsometricRenderer.h los
    # expone en su propia interfaz publica, y no trae GLAD/GLFW consigo
    # (ver comentario mas arriba), asi que no reintroduce ninguna
    # dependencia nueva sobre motor_core.
    target_link_libraries(motor_core PUBLIC glfw glm::glm motor_map)

    add_executable(sandbox_window examples/sandbox_window.cpp)
    target_link_libraries(sandbox_window PRIVATE motor_core)

    # Fase 1, ultima tarea: quad texturizado con GL_NEAREST. Usa Window +
    # Texture/Shader (motor_core) y Camera (motor_math) a la vez.
    add_executable(demo_textured_quad examples/demo_textured_quad.cpp)
    target_link_libraries(demo_textured_quad PRIVATE motor_core motor_math)

    # Fase 2 (culling) + Fase 3 (arranque): IsometricRenderer dibujando un
    # TileMap culled + un par de entidades de prueba, con Painter's
    # Algorithm real.
    add_executable(demo_isometric_renderer examples/demo_isometric_renderer.cpp)
    target_link_libraries(demo_isometric_renderer PRIVATE motor_core motor_math)
else ()
    message(STATUS
        "motor_core / sandbox_window omitidos: falta third_party/glad y/o "
        "third_party/stb (ver instrucciones en CMakeLists.txt). "
        "demo_resource_manager sigue disponible sin ellos.")
endif ()
