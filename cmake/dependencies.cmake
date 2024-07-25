# include(FetchContent)
find_package(OpenCV REQUIRED core imgcodecs imgproc)

if(OpenCV_FOUND)
    message("Found OpenCV")
    message("Includes: " ${OpenCV_INCLUDE_DIRS})
endif(OpenCV_FOUND)

find_package(nlohmann_json 3.2.0 REQUIRED)