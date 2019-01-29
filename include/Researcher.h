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

    bool isHacker = false;
    std::vector<HackingStrategy*> hackingStrategies;
	std::vector<Experiment> hackedExperiments;
	std::vector<Submission> hackedSubmissions;

    Submission submissionRecord;

    Researcher(Experiment* e) {
        experiment = e;
    };
//	Researcher(Experiment& e, Journal& j) : experiment(e), journal(j) {};

    void setTestStrategy(TestStrategies *t);
    void runTest();

    void registerAHackingStrategy(HackingStrategy* h);
    void hack();
    
    void calculateEffect();
    void selectTheOutcome();
	
    void prepareTheSubmission();
    void submitToJournal();

	void setJournal(Journal* j);

	// Reset the researcher state!
	void rest();


private:
	int _pvalue_min_inx;
	int _max_effect_inx;
	int _selected_outcome_inx = 0;
	Submission _create_submission_record(int inx);

};

#endif //SAMPP_RESEARCHER_H
