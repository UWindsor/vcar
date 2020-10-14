cmake_minimum_required(VERSION 3.16)
project(vcar-server)

find_package(Threads REQUIRED)

file(GLOB CXX_EXEC "src/*.cpp")

add_subdirectory(vcar-embedded)

add_executable(vcar-server ${CXX_EXEC})
target_link_libraries(vcar-server vcar "/usr/local/lib/librestbed.so.4" Threads::Threads)

set_target_properties(vcar-server PROPERTIES CXX_STANDARD 20)