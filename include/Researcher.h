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
#include "Journal.h"

class Researcher {

public:

    Experiment* experiment;

	Journal* journal;

    TestStrategies* testStrategy;


    std::vector<HackingStrategy*> hackingStrategies;
	std::vector<Experiment> hacked_experiments;
	std::vector<Submission> hacked_submissions;

    Submission submissionRecord;

    Researcher(Experiment* e) {
        experiment = e;
    };
//	Researcher(Experiment& e, Journal& j) : experiment(e), journal(j) {};

    void setTestStrategy(TestStrategies *t);
    void runTest();
    void setHackingStrategies();
    void calculateEffect();
    void selectTheOutcome();
    void prepareTheSubmission();
    void submitToJournal();

	void setJournal(Journal* j);


private:
	int _pvalue_min_arg;
	bool _is_phacker = false;
};

#endif //SAMPP_RESEARCHER_H
