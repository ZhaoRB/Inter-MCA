include(FetchContent)

# opencv
find_package(OpenCV REQUIRED core imgcodecs imgproc)

if(OpenCV_FOUND)
    message("Found OpenCV")
    message("Includes: " ${OpenCV_INCLUDE_DIRS})
endif(OpenCV_FOUND)

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

# nlohamn_json
# find_package(nlohmann_json 3.2.0 REQUIRED)