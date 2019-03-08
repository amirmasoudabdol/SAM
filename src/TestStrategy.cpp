//
// Created by Amir Masoud Abdol on 2019-01-24.
//

#include <TestStrategy.h>
#include "Utilities.h"
#include <iostream>
#include "gsl/gsl_statistics.h"
#include "gsl/gsl_randist.h"

#include <Experiment.h>

void TTest::run(Experiment* experiment) {
    
    for (int i = 0; i < experiment->setup.ng; ++i) {
        experiment->statistics[i] = experiment->means[i] / experiment->ses[i];
        
        experiment->pvalues[i] = gsl_ran_tdist_pdf(experiment->statistics[i], experiment->measurements[i].size() - 1.);
    }
}

TestStrategy *TestStrategy::buildTestStrategy(ExperimentSetup& setup){
    switch (setup.testMethod) {
        case TestMethod::TTest:
            return new TTest();
            break;
            
        default:
            return new TTest();
            break;
    }
}
