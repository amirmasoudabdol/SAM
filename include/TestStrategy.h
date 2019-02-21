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
class ExperimentSetup;

/**
 \brief Abstract class for defining test strategies.
 
 Statistical test strategies will investigate if there is a meaningful difference
 between means of two samples. Every test strategy should provide a `run()` method.
 The `run()` method will accept a pointer to the experiment and update necessary
 variables, e.g., _statistics_ & _p-value_.
 */
class TestStrategy {

public:
    
    static TestStrategy* buildTestStrategy(ExperimentSetup& setup);
    
    virtual void run(Experiment* experiment) = 0;

};

/**
 Declration of t-test.
 
 The `run()` method will check the significance of the difference between two groups.
 In the current setup, every `experiment->means` is considered an effect size between
 a treatment group and a control group with the mean of zero. Therefore, computing the
 t-statistics and computing the p-value would be sufficient. This is technically an
 implementation of [one sample t-test](https://en.wikipedia.org/wiki/Student%27s_t-test#One-sample_t-test).
 
 */
class TTest : public TestStrategy {
    
public:
    TTest() = default;
    
    void run(Experiment* experiment);
    
};


#endif //SAMPP_TESTSTRATEGIES_H
