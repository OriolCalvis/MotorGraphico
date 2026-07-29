name: CI
on:
  push:
    branches: [ develop, main ]
  pull_request:
    branches: [ develop, main ]

jobs:
  build-test:
    name: build (${{ matrix.compiler }}, ${{ matrix.build_type }})
    runs-on: ubuntu-latest
    strategy:
      fail-fast: false
      matrix:
        build_type: [Debug, Release]
        compiler: [g++, clang++]
    steps:
      - uses: actions/checkout@v4
      - name: Instalar dependencias
        run: |
          sudo apt-get update
          sudo apt-get install -y cmake clang
      - name: Configurar CMake
        env:
          CXX: ${{ matrix.compiler }}
        run: >
          cmake -B build
          -DCMAKE_BUILD_TYPE=${{ matrix.build_type }}
          -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
      - name: Compilar
        run: cmake --build build --config ${{ matrix.build_type }}
      - name: Ejecutar demo (ResourceManager<T>)
        run: ./build/demo_resource_manager
      # Solo una combinacion para no triplicar el tiempo de CI: los
      # sanitizers son sobre todo utiles para detectar UB/fugas de memoria
      # en Debug, y g++ y clang++ comparten el mismo codigo fuente.
      - name: Compilar y ejecutar con ASan + UBSan
        if: matrix.build_type == 'Debug' && matrix.compiler == 'g++'
        run: |
          cmake -B build-sanitize -DCMAKE_BUILD_TYPE=Debug \
            -DCMAKE_CXX_FLAGS="-fsanitize=address,undefined -fno-omit-frame-pointer -g"
          cmake --build build-sanitize
          ./build-sanitize/demo_resource_manager

  static-analysis:
    name: Formato y análisis estático
    runs-on: ubuntu-latest
    steps:
      - uses: actions/checkout@v4
      - name: Instalar herramientas
        run: |
          sudo apt-get update
          sudo apt-get install -y clang-format clang-tidy cppcheck cmake
      - name: Comprobar clang-format
        run: |
          files=$(find include src examples -name '*.h' -o -name '*.cpp')
          echo "$files" | xargs clang-format --dry-run --Werror
      - name: Generar compile_commands.json
        run: cmake -B build -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
      # Importante: solo analizamos los ficheros que realmente forman
      # parte del build actual (compile_commands.json). Asi evitamos que
      # clang-tidy falle sobre Texture.cpp/Shader.cpp, que todavia
      # dependen de GLAD/stb_image/glm sin vendorizar (ver CMakeLists.txt,
      # target motor_core comentado). En cuanto esas fuentes se anadan al
      # build, clang-tidy las cubrira automaticamente sin tocar este job.
      - name: clang-tidy sobre los ficheros del build actual
        run: |
          python3 - <<'PYEOF'
          import json
          import subprocess
          with open("build/compile_commands.json") as f:
              entries = json.load(f)
          files = sorted({entry["file"] for entry in entries})
          if not files:
              print("No hay ficheros en compile_commands.json, nada que analizar.")
          else:
              subprocess.run(["clang-tidy", "-p", "build", *files], check=True)
          PYEOF
      - name: cppcheck
        run: >
          cppcheck --enable=warning,performance,portability
          --error-exitcode=1 --inline-suppr
          -I include include src examples
