//
// Created by Amir Masoud Abdol on 2019-01-25.
//

#ifndef SAMPP_RESEARCHER_H
#define SAMPP_RESEARCHER_H

#include <vector>
#include "Experiment.h"
#include "TestStrategy.h"
#include "SubmissionRecord.h"
#include "HackingStrategies.h"
#include "Journal.h"

enum HackingStyle {
    onOrig,
    onCopy
};

class Researcher {

public:

    Experiment* experiment;

	Journal* journal;

//    TestStrategies* testStrategy;

    bool isHacker = false;
    HackingStyle hackingStyle = onCopy;
    std::vector<HackingStrategy*> hackingStrategies;
	std::vector<Experiment> hackedExperiments;
	std::vector<Submission> hackedSubmissions;

    Submission submissionRecord;

    Researcher(Experiment* e) {
        experiment = e;
    };
//	Researcher(Experiment& e, Journal& j) : experiment(e), journal(j) {};

//    void setTestStrategy(TestStrategies *t);
//    void runTest();

    // This probably needs to be a class of itself as well
    std::string decisionStrategy = "asap";
    void registerAHackingStrategy(HackingStrategy* h);
    void hack();
    void testHack();
    
    void calculateEffect();
    void selectTheOutcome();
	
    // This could be renamed to something like, selectThePreferedSubmission()
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
