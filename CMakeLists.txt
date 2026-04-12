# -- general properties -- #
cmake_minimum_required(VERSION 3.20)
set(ProjectName DirectX12Engine)
project(${ProjectName} CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
enable_language(CXX)

# -- set paths -- #
set(ENGINE_ROOT "${CMAKE_SOURCE_DIR}/DX3D12Engine")

file(GLOB_RECURSE ENGINE_SOURCE "${ENGINE_ROOT}/Source/*.cpp")

set(RESOURCE_FILES "${CMAKE_SOURCE_DIR}/DX3D12Engine/Assets/resource/resource.rc")
set(MAIN_APP "${CMAKE_SOURCE_DIR}/Main Application/main.cpp" ${RESOURCE_FILES})

# -- source files -- #
add_executable(${ProjectName}
    ${MAIN_APP}
    ${ENGINE_SOURCE}
)

# -- include files -- #
target_include_directories(${ProjectName}
PRIVATE
    ${ENGINE_ROOT}
)

# -- link libaries -- #
target_link_libraries(${ProjectName}
PRIVATE
    d3d12 dxgi dxguid d3dcompiler
)