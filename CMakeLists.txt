cmake_minimum_required(VERSION 3.21)

project(SAM
        LANGUAGES CXX
        VERSION 0.1.0.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(ENABLE_TESTS "Enable tests" OFF)
option(BUILD_SAM_EXEC "Build SAMrun executable" ON)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_COLOR_MAKEFILE   ON)
set(CMAKE_VERBOSE_MAKEFILE OFF)

if(CMAKE_BUILD_TYPE STREQUAL "Release") 
    include(CheckCXXCompilerFlag)
    CHECK_CXX_COMPILER_FLAG("-march=native" COMPILER_SUPPORTS_MARCH_NATIVE)
    if(COMPILER_SUPPORTS_MARCH_NATIVE)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -march=native")
        message(STATUS "Build with -march=native")
    endif()
endif()

# This is a hack and I need to manage it, and it is only needed for Lisa
set(CMAKE_PREFIX_PATH "$ENV{HOME}/.local")

find_package(Threads REQUIRED)

find_package(PkgConfig)

pkg_check_modules(MLPACK REQUIRED mlpack)

find_package(Boost COMPONENTS unit_test_framework program_options filesystem REQUIRED)
find_package(nlohmann_json 3.9 REQUIRED)
find_package(Armadillo REQUIRED)
find_package(ensmallen)
find_package(LAPACK)
find_package(BLAS)
find_package(spdlog 1.7 REQUIRED)
find_package(Lua 5.2 REQUIRED)
find_package(fmt 7.0 REQUIRED)


set(NLOHMANN_JSON_LIBRARIES "nlohmann_json::nlohmann_json")
set(SPDLOG_LIBRARIES "spdlog::spdlog")

include_directories(include)
include_directories(baaraan/include/)
include_directories(${BAARAAN_INCLUDE_DIRS})
include_directories(${ARMADILLO_INCLUDE_DIR})
include_directories(${Boost_INCLUDE_DIRS})
include_directories(${LUA_INCLUDE_DIR})
include_directories(${MLPACK_INCLUDE_DIRS})
include_directories(${ENSMALLEN_INCLUDE_DIRS})

add_subdirectory(baaraan)

file(GLOB INCLUDES "include/*.h")
file(GLOB SOURCES "src/*.cpp")

add_library(sam STATIC ${INCLUDES} ${SOURCES})
target_link_libraries(sam baaraan
                            ${BOOST_LIBRARIES}
                            ${NLOHMANN_JSON_LIBRARIES}
                            ${SPDLOG_LIBRARIES}
                            ${LUA_LIBRARIES}
                            fmt::fmt
                            Threads::Threads
                            )

# Building the SAMrun
if (${BUILD_SAM_EXEC})
    add_executable(SAMrun main.cpp)
    target_link_libraries(SAMrun sam ${Boost_LIBRARIES}
                                      ${NLOHMANN_JSON_LIBRARIES}
                                      Threads::Threads)

    file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/sample_config_file.json
         DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
endif()

set(${PROJECT_NAME}_INCLUDE_DIRS ${PROJECT_SOURCE_DIR}/include
                                    ${BOOST_INCLUDE_DIRS}
                                    ${ARMADILLO_INCLUDE_DIR}
                                    CACHE INTERNAL "${PROJECT_NAME}: Includes Directory" FORCE)

# Setup Tests
if (${ENABLE_TESTS})

    include(CTest)

    enable_testing()

    include_directories(tests/include)
    file(GLOB TESTSOURCES "tests/src/*.cpp")

    #Run through each source
    foreach(testSrc ${TESTSOURCES})
        #Extract the filename without an extension (NAME_WE)
        get_filename_component(testName ${testSrc} NAME_WE)

        #Add compile target
        add_executable(${testName} ${testSrc} ${SOURCES})

        #link to Boost libraries AND your targets and dependencies
        target_link_libraries(${testName} baaraan
                                            ${ARMADILLO_LIBRARIES}
                                            ${BOOST_LIBRARIES}
                                            ${NLOHMANN_JSON_LIBRARIES}
                                            ${SPDLOG_LIBRARIES}
                                            ${LUA_LIBRARIES}
                                            fmt::fmt
                                            Threads::Threads
                                            mlpack::mlpack)

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
