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
#include "DecisionStrategy.h"

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
	std::vector<Experiment> experimentsList;
	std::vector<Submission> submissionsList;

    Submission submissionRecord;

    Researcher(Experiment* e) {
        experiment = e;
    };

    // This probably needs to be a class of itself as well
    void registerAHackingStrategy(HackingStrategy* h);
    void hack();
//    void testHack();
    
    ResearcherPreference selectionPref = PreRegisteredOutcome;
//    void selectOutcome(Experiment& e);
    
    // std::string decisionStrategy = "asap";
    DecisionStrategy* decisionStrategy;
    void setDecisionStrategy(DecisionStrategy* d);
    
//    DecisionStrategy* finalDecision;
    // The `finalDecision` can be used alongside the `decisionStrategy`. While
    // `decisionStrategy` is being used to construct a submission from an experiment,
    // the `finalDecision` can be used to travese between all other submisions, and
    // pick the ultimate best. Having two decision strategy allow for more detail
    // implementation.

    
    void calculateEffect();

	
    // This could be renamed to something like, selectThePreferedSubmission()
    void selectOutcome();
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
