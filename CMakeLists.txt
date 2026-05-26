cmake_minimum_required(VERSION 3.20)
project(GameEngineWorkspace LANGUAGES CXX)

# Настройка стандартов компиляции
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Указываем, где будут создаваться готовые .exe и файлы сборки.
# Благодаря этому папка bin/ и все скомпилированные файлы появятся внутри build/ в корне
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/lib)

# Автоматически копируем ассеты из папки Engine/Assets в выходную бинарную папку build/bin/Assets
file(COPY "${CMAKE_CURRENT_SOURCE_DIR}/Engine/Assets" DESTINATION "${CMAKE_CURRENT_BINARY_DIR}/bin")

# Заходим внутрь папки Engine, где лежит второй CMakeLists.txt
add_subdirectory(Engine)