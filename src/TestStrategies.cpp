//
// Created by Amir Masoud Abdol on 2019-01-24.
//

#include <TestStrategies.h>
#include "Utilities.h"
#include <iostream>

void TTest::runTest() {
	std::cout << "this is t.test ";
	std::cout << "ng: " << experiment.setup.ng << std::endl;
	
    // std::cout << experiment.means;

    for (int i = 0; i < experiment.setup.ng; ++i) {
//        auto t, p = oneSampleTTest()
       this->experiment.means[i] =  i;
//        experiment.statistics.push_back(i);
        // experiment.pvalues.push_back(0.05);
        // experiment.effects.push_back(.147);
    }
}
