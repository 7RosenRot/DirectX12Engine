file(GLOB_RECURSE ENGINE_SOURCES 
    "Source/*.cpp"
    "Source/*.hpp"
    "Source/*.h"
    "Source/*.rc"
    "Source/main.cpp"
)

add_executable(Engine WIN32 ${ENGINE_SOURCES})

target_include_directories(Engine PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}/Source
    ${CMAKE_CURRENT_SOURCE_DIR}/Source/Renderer/D3D12Engine/Backend/RHI/Libraries
    ${CMAKE_CURRENT_SOURCE_DIR}/Source/Renderer/D3D12Engine/Backend/RHI/Libraries/imgui
    ${CMAKE_CURRENT_SOURCE_DIR}/Source/Renderer/D3D12Engine/Backend/RHI/Libraries/imgui/backends
    ${CMAKE_CURRENT_SOURCE_DIR}/Source/Renderer/D3D12Engine/Backend/RHI/Libraries/imguizmo

    ${CMAKE_CURRENT_SOURCE_DIR}/Assets/Images
)

target_link_libraries(Engine PRIVATE 
    d3d12
    dxgi
    d3dcompiler
)

if(MSVC)
    target_compile_options(Engine PRIVATE /W3 /WX-)
endif()