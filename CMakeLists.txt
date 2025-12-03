# -- general properties -- #
cmake_minimum_required(VERSION 3.20)
project(DirectX12Engine LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# -- set paths -- #
set(ENGINE_ROOT "${CMAKE_SOURCE_DIR}/DX3D12Engine")

file(GLOB_RECURSE ENGINE_SOURCE "${ENGINE_ROOT}/Source/*.cpp")

set(MAIN_APP "${CMAKE_SOURCE_DIR}/Main Application/main.cpp")

# -- source files -- #
add_executable(DirectX12Engine
    ${MAIN_APP}
    ${ENGINE_SOURCE}
)

# -- include files -- #
target_include_directories(DirectX12Engine
PRIVATE
    ${ENGINE_ROOT}
)

# -- link libaries -- #
target_link_libraries(DirectX12Engine
PRIVATE
    d3d12 dxgi d3dcompiler
)