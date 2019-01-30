cmake_minimum_required(VERSION 3.13)
project(SAMpp)

set(CMAKE_CXX_STANDARD 17)

include(CTest)

find_package(GSL REQUIRED)
find_package(Boost REQUIRED)
find_package(docopt REQUIRED)
find_package(nlohmann_json REQUIRED)

include_directories(include)
file(GLOB SOURCES "src/*.cpp")

add_executable(SAMpp main.cpp ${SOURCES} src/Experiment.cpp include/Experiment.h src/ExperimentSetup.cpp include/ExperimentSetup.h src/DataGenStrategy.cpp include/DataGenStrategy.h src/Utilities.cpp include/Utilities.h src/RandomNumberGenerator.cpp include/RandomNumberGenerator.h src/Journal.cpp include/Journal.h src/TestStrategy.cpp include/TestStrategy.h src/EffectEstimators.cpp include/EffectEstimators.h src/SubmissionRecord.cpp include/SubmissionRecord.h src/Researcher.cpp include/Researcher.h src/SelectionStrategies.cpp include/SelectionStrategies.h src/HackingStrategies.cpp include/HackingStrategies.h test/src/data_strategy_test.cpp)

target_link_libraries(SAMpp GSL::gsl Boost::boost docopt nlohmann_json::nlohmann_json)