//
// Created by Amir Masoud Abdol on 2019-01-24.
//

#ifndef SAMPP_TESTSTRATEGIES_H
#define SAMPP_TESTSTRATEGIES_H

#include <Experiment.h>
#include <memory>
#include <iostream>
#include <string>

class TestStrategies {

public:

    virtual void runTest() = 0;

    // void setExperiment(std::shared_ptr<Experiment> expr) ;

};

class TTest : public TestStrategies {

public:
    Experiment experiment;
    // std::string name = "t.test";


    TTest(Experiment& e) : experiment(e) {};

    void runTest();



};

#endif //SAMPP_TESTSTRATEGIES_H
