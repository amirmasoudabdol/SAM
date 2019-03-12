cmake_minimum_required(VERSION 3.12)
project(SAMpp)

set(CMAKE_CXX_STANDARD 17)

find_package(Boost COMPONENTS unit_test_framework REQUIRED)
find_package(GSL REQUIRED)
find_package(docopt REQUIRED)
find_package(nlohmann_json REQUIRED)
find_package(Armadillo REQUIRED)


find_package(LAPACK)
find_package(BLAS)

include_directories(include)
include_directories(${MLPACK_SRCS})
include_directories(${ARMADILLO_INCLUDE_DIR})
include_directories(${Boost_INCLUDE_DIRS})

file(GLOB INCLUDES "include/*.h")
file(GLOB SOURCES "src/*.cpp")

add_executable(SAMpp main.cpp ${SOURCES} src/Experiment.cpp include/Experiment.h src/ExperimentSetup.cpp include/ExperimentSetup.h src/DataGenStrategy.cpp include/DataGenStrategy.h src/Utilities.cpp include/Utilities.h src/RandomNumberGenerator.cpp include/RandomNumberGenerator.h src/Journal.cpp include/Journal.h src/TestStrategy.cpp include/TestStrategy.h src/EffectEstimators.cpp include/EffectEstimators.h src/SubmissionRecord.cpp include/SubmissionRecord.h src/Researcher.cpp include/Researcher.h src/SelectionStrategies.cpp include/SelectionStrategies.h src/HackingStrategies.cpp include/HackingStrategies.h src/DecisionStrategy.cpp include/DecisionStrategy.h src/ioUtilities.cpp include/ioUtilities.h src/HackerFactory.cpp include/HackerFactory.h include/main.h)

target_link_libraries(SAMpp
        ${ARMADILLO_LIBRARIES}
        ${Boost_LIBRARIES}
        GSL::gsl
        docopt
        nlohmann_json::nlohmann_json)

# Setup Tests
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
                                        GSL::gsl
                                        docopt
                                        nlohmann_json::nlohmann_json)

    #I like to move testing binaries into a testBin directory
    set_target_properties(${testName} PROPERTIES
            RUNTIME_OUTPUT_DIRECTORY  ${CMAKE_CURRENT_SOURCE_DIR}/testBin)

    #Finally add it to test execution -
    #Notice the WORKING_DIRECTORY and COMMAND
    add_test(NAME ${testName}
            WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/testBin
            COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/testBin/${testName} )
endforeach(testSrc)