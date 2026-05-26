# Рекурсивно собираем файлы исходного кода и заголовков по всей структуре Engine
file(GLOB_RECURSE ENGINE_SOURCES 
    "Source/*.cpp"
    "Source/*.hpp"
    "Source/main.cpp"
)

# Создаем исполняемый файл (флаг WIN32 скрывает консоль ОС и ищет функцию WinMain)
add_executable(CustomGameEngine WIN32 ${ENGINE_SOURCES})

# МАГИЯ ПУТЕЙ: Делает папку Engine/Source корневой точкой поиска для компилятора.
# Теперь из любого места проекта инклуды будут писаться в красивом сквозном стиле:
# #include <Application/Input/Input.hpp> или #include <Renderer/IRenderer/IRenderer.hpp>
target_include_directories(CustomGameEngine PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}/Source
)

# Связываем проект с системными библиотеками DirectX 12, DXGI и компилятором шейдеров
target_link_libraries(CustomGameEngine PRIVATE 
    d3d12
    dxgi
    d3dcompiler
)

# Настройки предупреждений компилятора MSVC (опционально)
if(MSVC)
    target_compile_options(CustomGameEngine PRIVATE /W3 /WX-)
endif()