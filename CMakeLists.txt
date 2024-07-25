cmake_minimum_required(VERSION 3.22)

project(Inter-MCA VERSION 0.1 LANGUAGES CXX)

include(cmake/dependencies.cmake)

add_executable(MCA-PLUS src/main.cpp)

target_compile_features(MCA-PLUS PRIVATE cxx_std_11)

target_include_directories(MCA-PLUS PUBLIC ${OpenCV_INCLUDE_DIRS})
target_link_libraries(MCA-PLUS PRIVATE ${OpenCV_LIBS} nlohmann_json::nlohmann_json)