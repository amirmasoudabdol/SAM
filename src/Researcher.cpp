//
// Created by Amir Masoud Abdol on 2019-01-25.
//

#include <Researcher.h>

#include <iostream>

void Researcher::setTestStrategy(TestStrategies *t) {
    testStrategy = t;
}


void Researcher::runTest() {
	// NOTE: This is super inefficient and I had to do it this way,
	// because I wanted to avoid pointer somehow, I need to revisit this
	// TODO: maybe rewrite it with pointers.
	auto stat_pvalue = testStrategy->computeStatsPvalue();
	experiment->statistics = stat_pvalue[0];
	experiment->pvalues = stat_pvalue[1];
}

void Researcher::calculateEffect(){
    experiment->effects = experiment->means;
}

void Researcher::hack() {
	for (auto &h : hackingStrategies){
		hackedSubmissions.push_back(h->perform());
	}
}

// This always select the pre-registered outcome, [0]
void Researcher::selectTheOutcome() {
	_selected_outcome_inx = 0;
//	_selected_outcome_inx= std::distance(experiment->pvalues.begin(), std::min_element(experiment->pvalues.begin(), experiment->pvalues.end()));
}

void Researcher::prepareTheSubmission() {
	if (!isHacker) {
		submissionRecord.method = "t.test";
		submissionRecord.simid = 1;
		submissionRecord.pubid = 1;
		submissionRecord.effect = experiment->effects[_selected_outcome_inx];
		submissionRecord.stat = experiment->statistics[_selected_outcome_inx];
		submissionRecord.pvalue = experiment->pvalues[_selected_outcome_inx];
	}else{
		// TODO: This needs generalization
//		submissionRecord = hackedSubmissions[0];
	}
}

void Researcher::submitToJournal() {
	journal->review(submissionRecord);
}

void Researcher::setJournal(Journal* j) {
	// TODO: This is susspicious!
	journal = j;
}

void Researcher::registerAHackingStrategy(HackingStrategy *h) {
	hackingStrategies.push_back(h);
}
