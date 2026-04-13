cmake_minimum_required(VERSION 3.20)
set(This Render)

project(${This} CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

add_executable(${This}
  "${CMAKE_SOURCE_DIR}/MainApplication/main.cpp"
  "${CMAKE_SOURCE_DIR}/Assets/resource/resource.rc"
)

target_link_libraries(${This} PRIVATE D3D12Engine)
set_target_properties(${This} PROPERTIES WIN32_EXECUTABLE TRUE)

target_include_directories(${This} PRIVATE
  "${CMAKE_SOURCE_DIR}/"
  "${CMAKE_SOURCE_DIR}/MainApplication/"
  "${CMAKE_SOURCE_DIR}/D3D12Engine/Include/Graphics"
  "${CMAKE_SOURCE_DIR}/Assets/resource/*.rc"
)

add_subdirectory(D3D12Engine)