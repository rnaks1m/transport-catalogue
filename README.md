# Transport-catalogue
Система маршрутизации общественного транспорта на C++, которая находит оптимальные маршруты между остановками и отрисовывает карты маршрутов в SVG. Все данные загружаются и возвращаются через JSON-интерфейс.

### 🛠 Project Tech Stack

| Component | Technologies & Tools |
| :--- | :--- |
| **Language & Standard** | ![C++20](https://img.shields.io/badge/C++20-00599C?style=for-the-badge&logo=c%2B%2B&logoColor=white) |
| **Build System** | ![CMake](https://img.shields.io/badge/CMake-%E2%89%A5_3.11-064F8C?style=for-the-badge&logo=cmake&logoColor=white) |
| **Library & Core** | ![STL](https://img.shields.io/badge/Standard_Library-STL-00599C?style=for-the-badge&logo=c%2B%2B&logoColor=white) ![RAII](https://img.shields.io/badge/Memory-RAII_/_Smart_Pointers-blue?style=for-the-badge) |
| **Data Formats** | ![JSON](https://img.shields.io/badge/JSON-Handwritten_Parser-000000?style=for-the-badge&logo=json&logoColor=white) ![SVG](https://img.shields.io/badge/SVG-Vector_Graphics-FFB13B?style=for-the-badge&logo=svg&logoColor=white) |
| **Algorithms** | ![Graph Theory](https://img.shields.io/badge/Graph-Directed_Weighted-brown?style=for-the-badge) ![Dijkstra](https://img.shields.io/badge/Routing-Dijkstra_/_Floyd--Warshall-orange?style=for-the-badge) ![Geo](https://img.shields.io/badge/Geography-Haversine_Formula-green?style=for-the-badge) |
| **Design Patterns** | ![Builder](https://img.shields.io/badge/Pattern-Builder-blueviolet?style=for-the-badge) ![Visitor](https://img.shields.io/badge/Pattern-Visitor-blueviolet?style=for-the-badge) ![Template Method](https://img.shields.io/badge/Pattern-Template_Method-blueviolet?style=for-the-badge) |
| **Dev Tools** | ![Git](https://img.shields.io/badge/Git-F05032?style=for-the-badge&logo=git&logoColor=white) ![Compilers](https://img.shields.io/badge/Compilers-GCC_/_Clang_/_MSVC-black?style=for-the-badge) |

### Обязательное ПО
- Компилятор C++ с поддержкой **C++17** (или выше)
  - GCC 7+, Clang 5+, MSVC 2017 15.7+
- **CMake** версии 3.11 или выше

## Сборка проекта
Сборка выполняется через **CMake**. Рекомендуется использовать отдельную папку для сборки.

### 1. Клонируйте репозиторий (или распакуйте архив с исходным кодом).
В папке, где хотите разместить проект, введите команду
```
git clone git@github.com:rnaks1m/transport-catalogue.git
```

### 2. Конфигурация
Из корня проекта выполните:
```
mkdir build
cd build
cmake .. -DCMAKE_BUILD_TYPE=Release   # или Debug
```
Для Windows (MSVC) при необходимости укажите генератор:
```
cmake .. -G "Visual Studio 17 2022" -A x64
```

### 3. Компиляция
```
cmake --build .
```
После успешной сборки в папке `build` появится исполняемый файл:
- **Linux / macOS:** `transport_catalogue`
- **Windows:** `transport_catalogue.exe`
