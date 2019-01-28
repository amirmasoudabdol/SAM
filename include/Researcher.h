//
// Created by Amir Masoud Abdol on 2019-01-25.
//

#ifndef SAMPP_RESEARCHER_H
#define SAMPP_RESEARCHER_H

#include <vector>
#include "Experiment.h"
#include "TestStrategies.h"
#include "SubmissionRecord.h"
#include "HackingStrategies.h"

class Researcher {

public:

    Experiment experiment;

    TestStrategies* testStrategy;
    std::vector<HackingStrategy*> hackingStrategies;

    Submission submissionRecord;

    Researcher(Experiment& e) : experiment(e) {};

    void setTestStrategy(TestStrategies *t);
    void runTest();
    void setHackingStrategies();

private:

};

#endif //SAMPP_RESEARCHER_H
