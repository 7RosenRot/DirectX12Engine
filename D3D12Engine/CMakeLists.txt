set(This D3D12Engine)

file(GLOB_RECURSE Headers
  "Include/*.hpp"
)

file(GLOB_RECURSE Sources
  "Source/*.cpp"
)

add_library(${This} STATIC 
  ${Headers}
  ${Sources}
)

target_include_directories(${This}
  PUBLIC
    "."
    "${CMAKE_SOURCE_DIR}/"
  PRIVATE 
    "${CMAKE_SOURCE_DIR}/MainApplication/"
)

target_link_libraries(${This} PUBLIC
  d3d12 dxgi dxguid d3dcompiler
)