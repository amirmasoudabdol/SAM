cmake_minimum_required(VERSION 3.12)
project(SAMpp)

option(ENABLE_TESTS "Enable tests" OFF)

set(CMAKE_COLOR_MAKEFILE   ON)
set(CMAKE_VERBOSE_MAKEFILE OFF)
#set (CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -fno-omit-frame-pointer -fsanitize=address")
#set (CMAKE_LINKER_FLAGS_DEBUG "${CMAKE_LINKER_FLAGS_DEBUG} -fno-omit-frame-pointer -fsanitize=address")

set(CMAKE_CXX_STANDARD 17)

add_subdirectory(${PROJECT_SOURCE_DIR}/../mvrandom ${PROJECT_SOURCE_DIR}/../mvrandom/build)

find_package(Boost COMPONENTS unit_test_framework REQUIRED)
find_package(nlohmann_json REQUIRED)
find_package(Armadillo REQUIRED)
find_package(LAPACK)
find_package(BLAS)

find_library(MLPACK mlpack)

include_directories(include)
include_directories(test/include)
include_directories(${MVRANDOM_INCLUDE_DIRS})
include_directories(${MLPACK_SRCS})
include_directories(${ARMADILLO_INCLUDE_DIR})
include_directories(${Boost_INCLUDE_DIRS})

file(GLOB INCLUDES "include/*.h")
file(GLOB SOURCES "src/*.cpp")

add_library(sam STATIC ${INCLUDES} ${SOURCES})
target_link_libraries(sam mvrandom)

string(TOUPPER ${PROJECT_NAME} PROJECT_NAME_CAP)
set(${PROJECT_NAME_CAP}_INCLUDE_DIRS ${PROJECT_SOURCE_DIR}/include
                                    ${BOOST_INCLUDE_DIRS}
                                    ${ARMADILLO_INCLUDE_DIRS}
                                    CACHE INTERNAL "${PROJECT_NAME}: Includes Directory" FORCE)

# Setup Tests
if (${ENABLE_TESTS})

    enable_testing()
    file(GLOB TESTSOURCES "test/src/*.cpp")

    #Run through each source
    foreach(testSrc ${TESTSOURCES})
        #Extract the filename without an extension (NAME_WE)
        get_filename_component(testName ${testSrc} NAME_WE)

        #Add compile target
        add_executable(${testName} ${testSrc} ${SOURCES})

        #link to Boost libraries AND your targets and dependencies
        target_link_libraries(${testName} ${ARMADILLO_LIBRARIES}
                                            ${Boost_LIBRARIES}
                                            ${Boost_UNIT_TEST_FRAMEWORK_LIBRARY}
                                            ${MLPACK}
                                            nlohmann_json::nlohmann_json)

        #I like to move testing binaries into a build/tests directory
        set_target_properties(${testName} PROPERTIES
                RUNTIME_OUTPUT_DIRECTORY  ${CMAKE_CURRENT_SOURCE_DIR}/build/tests)

        #Finally add it to test execution -
        #Notice the WORKING_DIRECTORY and COMMAND
        add_test(NAME ${testName}
                WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/build/tests
                COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/build/tests/${testName} )
    endforeach(testSrc)

endif()
