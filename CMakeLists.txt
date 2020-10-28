cmake_minimum_required(VERSION 3.16)
project(vcar-server)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake-modules")

find_package(Restbed REQUIRED)
find_package(Threads REQUIRED)
find_package(RapidJSON CONFIG REQUIRED)

file(GLOB CXX_EXEC "src/*.cpp")

add_subdirectory(vcar-embedded)

add_executable(vcar-server ${CXX_EXEC})
target_include_directories(vcar-server PUBLIC restbed_INCLUDE ${RapidJSON_INCLUDE_DIRS})
target_link_libraries(vcar-server vcar ${restbed_LIBRARY} Threads::Threads)

set_target_properties(vcar-server PROPERTIES CXX_STANDARD 20)