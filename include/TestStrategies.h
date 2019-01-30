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

    virtual void run() = 0;
	virtual std::vector<std::vector<double>> computeStatsPvalue() = 0;
    // void setExperiment(std::shared_ptr<Experiment> expr) ;

};

class TTest : public TestStrategies {

public:
    Experiment* experiment;
    // std::string name = "t.test";


    TTest(Experiment* e) {
    	experiment = e;
    }

    void run();
    std::vector<std::vector<double>> computeStatsPvalue();



};

#endif //SAMPP_TESTSTRATEGIES_H
