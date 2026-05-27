file(GLOB_RECURSE ENGINE_SOURCES 
    "Source/*.cpp"
    "Source/*.hpp"
    "Source/main.cpp"
)

add_executable(Engine WIN32 ${ENGINE_SOURCES})

target_include_directories(Engine PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}/Source
)

target_link_libraries(Engine PRIVATE 
    d3d12
    dxgi
    d3dcompiler
)

if(MSVC)
    target_compile_options(Engine PRIVATE /W3 /WX-)
endif()