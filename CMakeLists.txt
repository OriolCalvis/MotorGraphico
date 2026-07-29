cmake_minimum_required(VERSION 3.16)
project(MotorGraficoIsometrico CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(${CMAKE_SOURCE_DIR}/include)

# --- Demo sin dependencias externas: prueba ResourceManager<T> de forma
#     aislada. Compila y pasa en cualquier maquina con g++/clang++ C++17. ---
add_executable(demo_resource_manager
    examples/demo_resource_manager.cpp
    examples/TextAssetManager.cpp
)

# --- Clases "reales" (Texture/Shader) del diagrama de clases. Necesitan
#     GLFW + GLAD + stb_image + glm (vendorizados en third_party/ o via
#     find_package/FetchContent, segun decidais). Descomentar al llegar a
#     la Fase 1/2 del Gantt. ---
#
# add_library(motor_core
#     src/Core/Resources/Texture.cpp
#     src/Core/Resources/TextureManager.cpp
#     src/Core/Resources/Shader.cpp
#     src/Core/Resources/ShaderManager.cpp
# )
# target_link_libraries(motor_core PUBLIC glfw glad glm)
