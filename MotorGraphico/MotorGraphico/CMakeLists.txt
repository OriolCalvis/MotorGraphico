cmake_minimum_required(VERSION 3.16)
project(MotorGraficoIsometrico CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(${CMAKE_SOURCE_DIR}/include)

# --- Demo sin dependencias externas: prueba ResourceManager<T> de forma
# aislada. Compila y pasa en cualquier maquina con g++/clang++ C++17. ---
add_executable(demo_resource_manager
    examples/demo_resource_manager.cpp
    examples/TextAssetManager.cpp
)

# --- Fase 1 en adelante: necesitan GLFW + GLAD + stb_image + glm. ---
#
# GLFW y glm se descargan solos via FetchContent (ambos tienen soporte
# CMake oficial, sin pasos manuales).
#
# GLAD y stb_image NO se pueden automatizar del todo: GLAD requiere un
# paso de generacion (no es un simple "git clone") y stb_image es un
# header-only que hay que descargar una vez. Pasos manuales, una sola
# vez, antes de poder compilar motor_core:
#
#   1) GLAD: ir a https://glad.dav1d.de/, generador con
#      Language=C/C++, Specification=OpenGL, API gl=3.3, Profile=Core.
#      Descomprimir el zip resultante dentro de third_party/glad/ de
#      forma que quede: third_party/glad/include/glad/glad.h,
#      third_party/glad/include/KHR/khrplatform.h,
#      third_party/glad/src/glad.c
#
#   2) stb_image: descargar
#      https://raw.githubusercontent.com/nothings/stb/master/stb_image.h
#      y guardarlo en third_party/stb/stb_image.h
#      (el propio proyecto necesita un .cpp que haga
#       #define STB_IMAGE_IMPLEMENTATION antes de incluirlo una unica
#       vez; ver third_party/stb_image_impl.cpp mas abajo).
#
# En cuanto esas dos carpetas existan, "cmake .." detecta motor_core
# automaticamente: no hace falta tocar este fichero.

set(GLAD_DIR ${CMAKE_SOURCE_DIR}/third_party/glad)
set(STB_DIR ${CMAKE_SOURCE_DIR}/third_party/stb)

if (EXISTS ${GLAD_DIR}/src/glad.c AND EXISTS ${STB_DIR}/stb_image.h)
    include(FetchContent)

    set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
    set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
    set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
    FetchContent_Declare(
        glfw
        GIT_REPOSITORY https://github.com/glfw/glfw.git
        GIT_TAG 3.4
        GIT_SHALLOW TRUE
    )
    FetchContent_Declare(
        glm
        GIT_REPOSITORY https://github.com/g-truc/glm.git
        GIT_TAG 1.0.1
        GIT_SHALLOW TRUE
    )
    FetchContent_MakeAvailable(glfw glm)

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
        ${GLAD_DIR}/src/glad.c
        ${CMAKE_BINARY_DIR}/stb_image_impl.cpp
    )
    target_include_directories(motor_core PUBLIC
        ${GLAD_DIR}/include
        ${STB_DIR}
    )
    target_link_libraries(motor_core PUBLIC glfw glm::glm)

    add_executable(sandbox_window examples/sandbox_window.cpp)
    target_link_libraries(sandbox_window PRIVATE motor_core)
else ()
    message(STATUS
        "motor_core / sandbox_window omitidos: falta third_party/glad y/o "
        "third_party/stb (ver instrucciones en CMakeLists.txt). "
        "demo_resource_manager sigue disponible sin ellos.")
endif ()
