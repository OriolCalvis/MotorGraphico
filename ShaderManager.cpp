@startuml MotorGraficoIsometrico_ClassDiagram

skinparam classAttributeIconSize 0
skinparam packageStyle rectangle
skinparam linetype ortho

' ==================== TIPOS BASICOS ====================
package "Core::Math" {
  class Vector2 {
    +float x
    +float y
    +Vector2 operator+(const Vector2& other) const
    +Vector2 operator-(const Vector2& other) const
    +Vector2 operator*(float scalar) const
  }

  class GridCoord {
    +int x
    +int y
  }

  class UVRect {
    +float u0
    +float v0
    +float u1
    +float v1
  }
}

' ==================== SISTEMA DE ERRORES ====================
package "Core::Errors" {
  class "std::exception" as StdException {
  }

  class EngineException {
    #std::string m_message
    +EngineException(const std::string& msg)
    +virtual const char* what() const noexcept override
  }

  class ResourceLoadException {
    +ResourceLoadException(const std::string& path)
  }

  class ShaderCompileException {
    -std::string m_compileLog
    +ShaderCompileException(const std::string& log)
    +const std::string& compileLog() const
  }

  class RenderException {
    +RenderException(const std::string& msg)
  }

  class MapParseException {
    -int m_lineNumber
    +MapParseException(const std::string& reason, int line)
    +int line() const
  }

  StdException <|-- EngineException
  EngineException <|-- ResourceLoadException
  EngineException <|-- ShaderCompileException
  EngineException <|-- RenderException
  EngineException <|-- MapParseException

  ' Alternativa ligera a excepciones para rutas de alta frecuencia (por frame)
  class "Result<T>" as ResultT <<template>> {
    -bool m_success
    -T m_value
    -std::string m_errorMsg
    +{static} Result<T> Ok(T value)
    +{static} Result<T> Error(std::string msg)
    +bool isOk() const
    +T& value()
    +const std::string& errorMessage() const
  }

  note bottom of ResultT
    Se usa en rutas "calientes" (ej. TileMap::getTile,
    IsometricRenderer::renderFrame) para evitar el coste
    de excepciones en el bucle principal.
    Las excepciones (EngineException...) se reservan para
    carga de recursos e inicialización.
  end note
}

' ==================== GESTION DE RECURSOS (TEMPLATES) ====================
package "Core::Resources" {
  abstract class "ResourceManager<T>" as ResourceManagerT <<template>> {
    #std::unordered_map<std::string, std::unique_ptr<T>> m_resources
    +Result<T*> load(const std::string& id, const std::string& path)
    +T* get(const std::string& id)
    +void unload(const std::string& id)
    +void clear()
    #{abstract} std::unique_ptr<T> loadFromDisk(const std::string& path) = 0
  }

  class Texture {
    -unsigned int m_glID
    -int m_width
    -int m_height
    +Texture()
    +~Texture()
    +void bind(unsigned int slot) const
    +int getWidth() const
    +int getHeight() const
  }

  class TextureManager {
    +std::unique_ptr<Texture> loadFromDisk(const std::string& path) override
  }

  class Shader {
    -unsigned int m_programID
    +Shader()
    +~Shader()
    +void use() const
    +void setUniformMat4(const std::string& name, const glm::mat4& mat)
    +void setUniformInt(const std::string& name, int value)
  }

  class ShaderManager {
    +std::unique_ptr<Shader> loadFromDisk(const std::string& path) override
  }

  class TextureAtlas {
    -Texture* m_texture
    -std::unordered_map<int, UVRect> m_regions
    +UVRect getUV(int tileID) const
    +void defineRegion(int id, int col, int row)
  }

  ResourceManagerT <|-- TextureManager
  ResourceManagerT <|-- ShaderManager
  ResourceManagerT ..> Texture : T = Texture >
  ResourceManagerT ..> Shader : T = Shader >
  TextureAtlas o-- Texture
  ResourceManagerT ..> ResultT : devuelve >
}

' ==================== JERARQUIA DE RENDERIZADO ====================
package "Render" {
  interface IRenderable {
    +{abstract} void render(SpriteBatch& batch) = 0
    +{abstract} int getSortKey() const = 0
  }

  interface IUpdatable {
    +{abstract} void update(float deltaTime) = 0
  }

  class Camera {
    -Vector2 m_position
    -Vector2 m_targetPosition
    -float m_zoom
    -float m_lerpSpeed
    +void update(float deltaTime)
    +void move(const Vector2& delta)
    +void setZoom(float zoom)
    +glm::mat4 getViewProjectionMatrix() const
    +Vector2 screenToWorld(const Vector2& screenPos) const
    +GridCoord worldToGrid(const Vector2& worldPos) const
  }

  IUpdatable <|.. Camera

  class SpriteBatch {
    -std::vector<Vertex> m_vertexBuffer
    -unsigned int m_vao
    -unsigned int m_vbo
    -Texture* m_currentTexture
    +void begin()
    +void submit(const Vector2& pos, const UVRect& uv, Texture* tex)
    +void end()
    +Result<bool> flush()
  }

  class Tile {
    -int m_tilesetID
    -int m_variant
    -bool m_collision
    +bool hasCollision() const
    +int getTilesetID() const
  }

  class TileMap {
    -std::vector<std::vector<std::vector<Tile>>> m_layers
    -int m_width
    -int m_height
    +Result<bool> loadFromFile(const std::string& path)
    +Tile& getTile(int layer, int x, int y)
    +GridCoord screenToGrid(const Vector2& screenPos) const
    +Vector2 gridToScreen(const GridCoord& grid) const
  }

  TileMap *-- Tile
  TileMap ..> MapParseException : lanza >

  abstract class Entity {
    #GridCoord m_gridPosition
    #Vector2 m_offset
    #int m_spriteID
    #TextureAtlas* m_atlas
    +void render(SpriteBatch& batch) override
    +int getSortKey() const override
    +{abstract} void update(float deltaTime) override = 0
  }

  IRenderable <|.. Entity
  IUpdatable <|.. Entity

  class AnimatedEntity {
    -std::unordered_map<std::string, std::vector<int>> m_animations
    -float m_frameTime
    -float m_elapsedTime
    -int m_currentFrame
    +void update(float deltaTime) override
    +void addAnimation(const std::string& name, std::vector<int> frames)
    +void play(const std::string& name)
  }

  Entity <|-- AnimatedEntity

  class Player {
    -int m_health
    -std::vector<int> m_inventory
    +void update(float deltaTime) override
    +void handleInput(const struct InputState& input)
  }

  AnimatedEntity <|-- Player

  class Enemy {
    -int m_health
    -int m_aiState
    +void update(float deltaTime) override
  }

  AnimatedEntity <|-- Enemy

  class IsometricRenderer {
    -Camera* m_camera
    -SpriteBatch m_spriteBatch
    -TileMap* m_map
    -std::vector<IRenderable*> m_renderQueue
    -Shader* m_shader
    +void addToQueue(IRenderable* obj)
    +void sortQueue()
    +Result<bool> renderFrame()
    -void renderLayer(int layerIndex)
    -void applyPostProcessing()
  }

  IsometricRenderer o-- Camera
  IsometricRenderer o-- SpriteBatch
  IsometricRenderer o-- TileMap
  IsometricRenderer ..> IRenderable : ordena y dibuja >
  IsometricRenderer ..> RenderException : lanza >

  note bottom of IsometricRenderer
    sortQueue() aplica el Painter's Algorithm:
    orden = (grid_x + grid_y) * tile_height/2, luego X
  end note
}

' ==================== MOTOR PRINCIPAL ====================
package "Engine" {
  class Application {
    -TextureManager m_textureManager
    -ShaderManager m_shaderManager
    -IsometricRenderer m_renderer
    -TileMap m_currentMap
    -bool m_running
    +Result<bool> init(int width, int height, const std::string& title)
    +void run()
    +void shutdown()
    -void processInput()
    -void update(float deltaTime)
    -void render()
  }

  Application o-- TextureManager
  Application o-- ShaderManager
  Application o-- IsometricRenderer
  Application o-- TileMap
  Application ..> EngineException : captura >
}

@enduml
