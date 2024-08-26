cmake_minimum_required(VERSION 3.22)

project(MCA2 VERSION 1.0 LANGUAGES CXX)

include(cmake/dependencies.cmake)

# - preprocess
add_executable(MCA-Preprocess src/preprocess/main.cpp)

target_compile_features(MCA-Preprocess PRIVATE cxx_std_11)

target_include_directories(MCA-Preprocess
    PUBLIC ${OpenCV_INCLUDE_DIRS}
    PRIVATE include/preprocess
)

target_link_libraries(MCA-Preprocess PRIVATE ${OpenCV_LIBS})

# target_link_libraries(MCA-Preprocess PRIVATE nlohmann_json::nlohmann_json)