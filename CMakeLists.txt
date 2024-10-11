cmake_minimum_required(VERSION 3.22)
project(MCA2 VERSION 1.0 LANGUAGES CXX)

include(cmake/dependencies.cmake)

# preprocess
add_executable(
    mca20-pre
    src/preprocess/main.cpp
    src/preprocess/preprocess.cpp
    src/preprocess/offset_vector.cpp
    src/preprocess/crop_realign.cpp
    src/common/parse.cpp
    src/common/utils.cpp
)

target_compile_features(mca20-pre PRIVATE cxx_std_17)

target_include_directories(
    mca20-pre
    PRIVATE include/preprocess include/common
    PRIVATE ${OpenCV_INCLUDE_DIRS}
    PRIVATE ${pugixml_SOURCE_DIR}/src

    # PRIVATE nlohmann_json::nlohmann_json
)

target_link_libraries(
    mca20-pre
    PRIVATE ${OpenCV_LIBS}
    PRIVATE
    "${CMAKE_BINARY_DIR}/_deps/pugixml-build/${CMAKE_STATIC_LIBRARY_PREFIX}pugixml${CMAKE_STATIC_LIBRARY_SUFFIX}"
)

# draw centers
add_executable(
    draw_centers
    src/draw_center/main.cpp
    src/common/parse.cpp
    src/common/utils.cpp
)

target_include_directories(
    draw_centers
    PRIVATE
    include/common
    PRIVATE ${OpenCV_INCLUDE_DIRS}
    PRIVATE ${pugixml_SOURCE_DIR}/src
)

target_link_libraries(
    draw_centers
    PRIVATE ${OpenCV_LIBS}
    PRIVATE
    "${CMAKE_BINARY_DIR}/_deps/pugixml-build/${CMAKE_STATIC_LIBRARY_PREFIX}pugixml${CMAKE_STATIC_LIBRARY_SUFFIX}"
)

# draw centers
add_executable(
    mca20-post
    src/postprocess/main.cpp
    src/postprocess/postprocess.cpp
    src/postprocess/restore_patches.cpp
    src/postprocess/restore_corners.cpp
    src/postprocess/restore_gap.cpp
    src/common/utils.cpp
    src/common/parse.cpp
)

target_include_directories(
    mca20-post
    PRIVATE
    include/common include/postprocess
    PRIVATE ${OpenCV_INCLUDE_DIRS}
    PRIVATE ${pugixml_SOURCE_DIR}/src
)

target_link_libraries(
    mca20-post
    PRIVATE ${OpenCV_LIBS}
    PRIVATE
    "${CMAKE_BINARY_DIR}/_deps/pugixml-build/${CMAKE_STATIC_LIBRARY_PREFIX}pugixml${CMAKE_STATIC_LIBRARY_SUFFIX}"
)