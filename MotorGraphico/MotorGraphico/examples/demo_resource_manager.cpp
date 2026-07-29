#include "TextAssetManager.h"

#include <cassert>
#include <fstream>
#include <iostream>

int main() {
    // Preparar un fichero de prueba en disco.
    {
        std::ofstream out("demo_asset.txt");
        out << "Hola desde ResourceManager<T>!";
    }

    TextAssetManager manager;

    // Caso 1: carga exitosa.
    auto result = manager.load("saludo", "demo_asset.txt");
    assert(result.isOk());
    std::cout << "[OK] Cargado 'saludo': " << result.value()->content() << "\n";

    // Caso 2: pedir el mismo id devuelve el recurso cacheado sin releer disco
    // (la ruta pasada aqui ni siquiera existe, y aun asi funciona).
    auto cached = manager.load("saludo", "ruta_que_no_existe.txt");
    assert(cached.isOk());
    assert(cached.value() == result.value()); // mismo puntero: viene de cache
    std::cout << "[CACHE] count() tras segunda peticion = " << manager.count()
               << " (esperado: 1)\n";

    // Caso 3: fichero inexistente -> Result::Error controlado, sin excepcion
    // visible para quien llama a load().
    auto missing = manager.load("inexistente", "no_existe.txt");
    assert(!missing.isOk());
    std::cout << "[OK esperado] Error controlado: " << missing.errorMessage() << "\n";

    // Caso 4: get() / contains() / unload() / clear().
    TextAsset* ptr = manager.get("saludo");
    std::cout << "[GET] get(\"saludo\") != nullptr: " << (ptr != nullptr) << "\n";
    std::cout << "[CONTAINS] contains(\"inexistente\") = "
               << manager.contains("inexistente") << " (esperado: 0, el load fallido no cachea)\n";

    manager.unload("saludo");
    std::cout << "[UNLOAD] count() tras unload = " << manager.count() << " (esperado: 0)\n";

    manager.clear();
    std::cout << "[CLEAR] count() tras clear = " << manager.count() << " (esperado: 0)\n";

    std::cout << "\nTodas las comprobaciones (assert) han pasado correctamente.\n";
    return 0;
}
