//
// Created by Amir Masoud Abdol on 2019-01-25.
//

#include <Researcher.h>

#include <iostream>

void Researcher::setTestStrategy(TestStrategies *t) {
    testStrategy = t;
}

void Researcher::runTest() {
	std::cout << "running t.test" << std::endl;
    testStrategy->runTest();
    // for (int i = 0; i < experiment.setup.ng; ++i)
    // {
    // 	experiment.statistics.push_back(i);
    // 	// experiment.statistics[i] = i;
    // }
}
