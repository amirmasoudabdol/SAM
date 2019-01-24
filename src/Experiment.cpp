//
// Created by Amir Masoud Abdol on 2019-01-22.
//

#include <Experiment.h>

#include "Experiment.h"

void Experiment::setDataStrategy(DataGenStrategy strategy) {
    _data_strategy = strategy;
}

void Experiment::genData() {
    _measurements = _data_strategy.genData();
}


void Experiment::initData() {
	
}