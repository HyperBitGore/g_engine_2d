cmake_minimum_required(VERSION 3.5.0)
project(g_engine version 0.1.0 LANGUAGES C CXX)
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED On)
set(CMAKE_CXX_EXTENSIONS Off)
set(CMAKE_EXPORT_COMPILE_COMMANDS true)

add_executable(g_engine source/*.cpp source/*.h example/example.cpp)
target_include_directories(g_engine PUBLIC include/)

if(WIN32)
SET(LINK_LIBRARY )
endif(WIN32)
if(UNIX)
SET(LINK_LIBRARY )
endif(UNIX)

target_link_libraries(g_engine ${LINK_LIBRARY})