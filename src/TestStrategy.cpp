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
    this->computeStatsPvalue(experiment);
}

void TTest::computeStatsPvalue(Experiment* experiment) {

    for (int i = 0; i < experiment->setup.ng; ++i) {
        experiment->statistics[i] = experiment->vars[i] / experiment->ses[i];

        experiment->pvalues[i] = gsl_ran_tdist_pdf(experiment->statistics[i], experiment->measurements[i].size() - 1);
    }

}

//void TTest::updateExperimentPointer(Experiment* e) {
//    experiment = e;
//}


