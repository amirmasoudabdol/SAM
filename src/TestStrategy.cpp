//
// Created by Amir Masoud Abdol on 2019-01-24.
//

#include <TestStrategy.h>
#include "Utilities.h"
#include <iostream>
#include "gsl/gsl_statistics.h"
#include "gsl/gsl_randist.h"

void TTest::run() {
	// std::cout << "this is t.test ";
	// std::cout << "ng: " << experiment->setup.ng << std::endl;
	
    // std::cout << experiment->means;

//     std::cout << "hey: " << experiment->statistics.size() << std::endl;
//     for (int i = 0; i < experiment->setup.ng; ++i) {
// //        auto t, p = oneSampleTTest()

// //      experiment->means[]
//         // std::cout << experiment->means[i] << " = ";
//        // experiment->statistics[i] =  15;
//        // experiment->statistics.push_back(10);
//         // experiment->pvalues.push_back(0.05);
//         // experiment->effects.push_back(.147);
//     }
//     std::cout << std::endl;

    auto stat_pvalue = this->computeStatsPvalue();
//    experiment->statistics = stat_pvalue[0];
//    experiment->pvalues = stat_pvalue[1];
}

std::vector<std::vector<double>> TTest::computeStatsPvalue() {
//    std::vector<double> stats(experiment->setup.ng);
//    std::vector<double> pvalue(experiment->setup.ng);

    std::vector<double> stats;
    std::vector<double> pvalue;

//    for (int i = 0; i < experiment->setup.ng; ++i) {
//        stats[i] = experiment->vars[i] / experiment->ses[i];
//
//        pvalue[i] = gsl_ran_tdist_pdf(stats[i], experiment->measurements[i].size() - 1);
//    }

//    t.pdf(self.effects_df.stat, df = self.effects_df.nobs - 1)

    std::vector<std::vector<double> > t_p;
    t_p.push_back(stats);
    t_p.push_back(pvalue);

    return t_p;
}

