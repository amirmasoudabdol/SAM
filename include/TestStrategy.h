//
// Created by Amir Masoud Abdol on 2019-01-24.
//

#ifndef SAMPP_TESTSTRATEGIES_H
#define SAMPP_TESTSTRATEGIES_H

// #include <Experiment.h>
#include <memory>
#include <iostream>
#include <string>

class Experiment;

class TestStrategy {

public:
    
//    Experiment exp;

    virtual void run(Experiment* experiment) = 0;
//    virtual void updateExperimentPointer(Experiment *e) = 0;
//    virtual std::vector<std::vector<double>> computeStatsPvalue() = 0;
    // void setExperiment(std::shared_ptr<Experiment> expr) ;

};


class TTest : public TestStrategy {
    
public:
//    Experiment* experiment;
//    // std::string name = "t.test";
//
//
//    TTest(Experiment* e) {
//        experiment = e;
//    }
    
    TTest() = default;
    
//    void updateExperimentPointer(Experiment *e);
    void run(Experiment* experiment);
    void computeStatsPvalue(Experiment *experiment);
    
    
    
};


#endif //SAMPP_TESTSTRATEGIES_H
