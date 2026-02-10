cmake_minimum_required(VERSION 3.11)

project(transport_catalogue CXX)
set(CMAKE_CXX_STANDARD 20)

add_executable(transport_catalogue    
    src/domain.h
    src/domain.cpp
    src/geo.h
    src/geo.cpp
    src/graph.h
    src/json.h
    src/json.cpp
    src/json_builder.h
    src/json_builder.cpp
    src/json_reader.h
    src/json_reader.cpp
    src/main.cpp
    src/map_renderer.h
    src/map_renderer.cpp
    src/ranges.h
    src/request_handler.h
    src/request_handler.cpp
    src/router.h
    src/svg.h
    src/svg.cpp
    src/transport_catalogue.h
    src/transport_catalogue.cpp
    src/transport_router.h
    src/transport_router.cpp
)