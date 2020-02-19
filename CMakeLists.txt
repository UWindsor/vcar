cmake_minimum_required(VERSION 3.0)
project(vcar-server)

set(CMAKE_CXX_STANDARD 20)

file(GLOB CXX_EXEC "src/*.cpp")

add_subdirectory(vcar-embedded)

add_executable(vcar-server ${CXX_EXEC})
target_include_directories(vcar-server PUBLIC inc/)
target_link_libraries(vcar-server vcar)
