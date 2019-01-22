cmake_minimum_required(VERSION 3.13)
project(SAMpp)

set(CMAKE_CXX_STANDARD 17)

find_package(GSL REQUIRED)
find_package(Boost REQUIRED)

include_directories(include)
file(GLOB SOURCES "src/*.cpp")

add_executable(SAMpp main.cpp ${SOURCES})

target_link_libraries(SAMpp GSL::gsl Boost::boost)