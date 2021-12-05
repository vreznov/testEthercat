cmake_minimum_required(VERSION 3.5)

project(testEC1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories("/home/kare/soft/soem/include/")
link_directories("/home/kare/soft/soem/lib/linux/")

add_executable(testEC1 main.cpp kecm.cpp)

set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/../bin)

target_link_libraries(testEC1 -lsoem -losal -loshw -lpthread)
