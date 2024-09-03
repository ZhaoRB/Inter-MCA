cmake_minimum_required(VERSION 3.22)
project(MCA2 VERSION 1.0 LANGUAGES CXX)

include(cmake/dependencies.cmake)

# preprocess
add_executable(
    MCA-Preprocess
    src/preprocess/main.cpp
    src/preprocess/preprocess.cpp
    src/common/parse.cpp
)

target_compile_features(MCA-Preprocess PRIVATE cxx_std_11)

target_include_directories(
    MCA-Preprocess
    PRIVATE include/preprocess include/common
    PRIVATE ${OpenCV_INCLUDE_DIRS}
    PRIVATE ${pugixml_SOURCE_DIR}/src

    # PRIVATE nlohmann_json::nlohmann_json
)

target_link_libraries(
    MCA-Preprocess
    PRIVATE ${OpenCV_LIBS}
    PRIVATE
    "${CMAKE_BINARY_DIR}/_deps/pugixml-build/${CMAKE_STATIC_LIBRARY_PREFIX}pugixml${CMAKE_STATIC_LIBRARY_SUFFIX}"
)

# draw centers
add_executable(
    Draw-Centers
    src/draw-center/main.cpp
    src/common/parse.cpp
)

target_include_directories(
    Draw-Centers
    PRIVATE
    include/draw-center include/common
    PRIVATE ${OpenCV_INCLUDE_DIRS}
    PRIVATE ${pugixml_SOURCE_DIR}/src
)

target_link_libraries(
    Draw-Centers
    PRIVATE ${OpenCV_LIBS}
    PRIVATE
    "${CMAKE_BINARY_DIR}/_deps/pugixml-build/${CMAKE_STATIC_LIBRARY_PREFIX}pugixml${CMAKE_STATIC_LIBRARY_SUFFIX}"
)