# Ramas, versionado y CI/CD — Motor Gráfico Isométrico

## 1. Modelo de ramas

Git Flow simplificado, pensado para equipo pequeño/unipersonal.

- **`main`** — siempre estable. Solo recibe merges desde `develop` o `hotfix/*`. Sobre ella se etiquetan las versiones.
- **`develop`** — rama de integración. Los `feature/*` se fusionan aquí. Debe pasar el CI completo antes de cualquier merge.

| Tipo    | Nombre                    | Desde     | Hacia                          | Propósito |
|---------|---------------------------|-----------|---------------------------------|-----------|
| Feature | `feature/<nombre-corto>`  | `develop` | `develop`                       | Nueva funcionalidad (ej. `feature/resource-manager`, `feature/sprite-batching`) |
| Bugfix  | `bugfix/<descripcion>`    | `develop` | `develop` (y `main` si es crítico) | Corrección de errores en desarrollo |
| Hotfix  | `hotfix/<version>`        | `main`    | `main` y `develop`              | Arreglo urgente en producción, incrementa PARCHE |
| Release | `release/<version>`       | `develop` | `main` y `develop`               | Congelación de features, solo ajustes finales |

**Ejemplo (Fase 1 del Gantt):** `feature/glfw-opengl-window` → PR a `develop` → `feature/textured-quad`, `feature/ortho-camera`, `feature/iso-grid-test` → `release/v0.1.0` → merge a `main` con tag `v0.1.0`.

## 2. Versionado (SemVer)

```
v<MAYOR>.<MENOR>.<PARCHE>[-alpha.N|-beta.N|-rc.N]
```

Mapeo con las fases ya planificadas en `motor_grafico_gantt.puml`:

| Hito                                   | Tag                |
|-----------------------------------------|--------------------|
| Fin Fase 1 (grid isométrico funcional)   | `v0.1.0-alpha.1`    |
| Fin Fase 2 (mapas y tiles)               | `v0.2.0-alpha.1`    |
| Fin Fase 3 (entidades y animación)       | `v0.3.0-beta`       |
| Primera versión jugable                  | `v0.3.0`            |
| Fin Fase 4 (pulido visual)                | `v0.4.0`            |

Tags ligeros para hitos internos que no son releases públicos: `dev-phase1-complete`, `integration/sprite-batching`.

## 3. CI/CD (`.github/workflows/`)

- **`ci.yml`** — en cada push/PR a `develop`/`main`:
  - `build-test`: matriz Debug/Release × g++/clang++. Compila con `CMAKE_EXPORT_COMPILE_COMMANDS=ON` y ejecuta `demo_resource_manager` (hoy el único binario real del proyecto). En Debug+g++ añade una pasada con ASan+UBSan.
  - `static-analysis`: `clang-format --dry-run --Werror`, `clang-tidy` **solo sobre los ficheros presentes en `compile_commands.json`** (así no falla sobre `Texture.cpp`/`Shader.cpp`, que aún dependen de GLAD/stb_image/glm sin vendorizar), y `cppcheck`.
- **`release.yml`** — al empujar un tag `v*`: compila Release en Linux y Windows, empaqueta (`motor-iso-<tag>-linux.tar.gz` / `...-win64.zip`) y publica el artefacto en GitHub Releases.

## 4. Pre-commit local (opcional, recomendado)

`.pre-commit-config.yaml` instala:
- `clang-format` automático sobre los ficheros modificados.
- Higiene básica (espacios en blanco, fin de fichero, conflictos de merge sin resolver).
- Un build rápido (`Debug`, solo el target `demo_resource_manager`) como hook de `pre-push`, para detectar errores de compilación antes de llegar al CI.

Instalación:
```bash
pip install pre-commit
pre-commit install --hook-type pre-commit --hook-type pre-push
```

## 5. Cuándo activar lo que aún está comentado

`CMakeLists.txt` tiene el target `motor_core` (Texture/Shader) comentado hasta vendorizar GLFW/GLAD/stb_image/glm (Fase 1–2 del Gantt). En cuanto se active:
- `ci.yml` lo recogerá automáticamente (compila lo que haya en `CMakeLists.txt`, sin cambios en el workflow).
- `release.yml` necesitará actualizarse para empaquetar el binario real de la aplicación (hoy solo empaqueta `demo_resource_manager`) junto con los assets mínimos.
