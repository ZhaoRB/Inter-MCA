cmake_minimum_required(VERSION 3.22)
project(MCA2 VERSION 1.0 LANGUAGES CXX)
include(FetchContent)

add_executable(
    MCA-Preprocess
    src/preprocess/main.cpp
    src/preprocess/parse.cpp
    src/utils/draw.cpp
)

target_compile_features(MCA-Preprocess PRIVATE cxx_std_11)

target_include_directories(MCA-Preprocess
    PRIVATE include/preprocess include
)

# dependencies
# opencv
find_package(OpenCV REQUIRED core imgcodecs imgproc)

if(OpenCV_FOUND)
    message("Found OpenCV")
    message("Includes: " ${OpenCV_INCLUDE_DIRS})
endif(OpenCV_FOUND)

target_include_directories(MCA-Preprocess PRIVATE ${OpenCV_INCLUDE_DIRS})
target_link_libraries(MCA-Preprocess PRIVATE ${OpenCV_LIBS})

# pugixml
FetchContent_Declare(
    pugixml
    GIT_REPOSITORY https://github.com/zeux/pugixml.git
    GIT_TAG v1.14
)
FetchContent_MakeAvailable(pugixml)

FetchContent_GetProperties(pugixml)

if(NOT pugixml_POPULATED)
    FetchContent_Populate(pugixml)
endif()

target_include_directories(MCA-Preprocess PRIVATE ${pugixml_SOURCE_DIR}/src)
target_link_libraries(MCA-Preprocess PRIVATE
    "${CMAKE_BINARY_DIR}/_deps/pugixml-build/${CMAKE_STATIC_LIBRARY_PREFIX}pugixml${CMAKE_STATIC_LIBRARY_SUFFIX}")

# nlohamn_json
# find_package(nlohmann_json 3.2.0 REQUIRED)
# target_link_libraries(MCA-Preprocess PRIVATE nlohmann_json::nlohmann_json)
