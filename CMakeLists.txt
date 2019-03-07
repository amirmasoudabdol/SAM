cmake_minimum_required(VERSION 3.12)
project(SAMpp)

set(CMAKE_CXX_STANDARD 17)

include(CTest)

find_package(GSL REQUIRED)
find_package(docopt REQUIRED)
find_package(nlohmann_json REQUIRED)
find_package(ARMADILLO REQUIRED)

find_package(LAPACK)
find_package(BLAS)

include_directories(include)
include_directories(${MLPACK_SRCS})
include_directories(${ARMADILLO_INCLUDE_DIR})
file(GLOB SOURCES "src/*.cpp")

add_executable(SAMpp main.cpp ${SOURCES} src/Experiment.cpp include/Experiment.h src/ExperimentSetup.cpp include/ExperimentSetup.h src/DataGenStrategy.cpp include/DataGenStrategy.h src/Utilities.cpp include/Utilities.h src/RandomNumberGenerator.cpp include/RandomNumberGenerator.h src/Journal.cpp include/Journal.h src/TestStrategy.cpp include/TestStrategy.h src/EffectEstimators.cpp include/EffectEstimators.h src/SubmissionRecord.cpp include/SubmissionRecord.h src/Researcher.cpp include/Researcher.h src/SelectionStrategies.cpp include/SelectionStrategies.h src/HackingStrategies.cpp include/HackingStrategies.h test/src/data_strategy_test.cpp src/DecisionStrategy.cpp include/DecisionStrategy.h src/ioUtilities.cpp include/ioUtilities.h src/HackerFactory.cpp include/HackerFactory.h)

target_link_libraries(SAMpp ${ARMADILLO_LIBRARIES} GSL::gsl docopt nlohmann_json::nlohmann_json)
