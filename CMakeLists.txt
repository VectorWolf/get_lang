project("get-lang")

cmake_minimum_required(VERSION 3.5 FATAL_ERROR)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_BUILD_TYPE RELEASE)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -pedantic -Wextra -Wfatal-errors")

add_executable(${PROJECT_NAME} src/main.cpp
                    src/split.cpp
                    src/wordbooks.cpp
)
                    
TARGET_LINK_LIBRARIES(${PROJECT_NAME})
