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
	experiment.statistics = stat_pvalue[0];
	experiment.pvalues = stat_pvalue[1];
}

void Researcher::calculateEffect(){
    experiment.effects = experiment.means;
}

void Researcher::selectTheOutcome() {
	_pvalue_min_arg = std::distance(experiment.pvalues.begin(),
									std::min_element(experiment.pvalues.begin(), experiment.pvalues.end()));
}

void Researcher::prepareTheSubmission() {
	submissionRecord.method = "t.test";
	submissionRecord.simid = 1;
	submissionRecord.pubid = 1;
	submissionRecord.effect = experiment.effects[_pvalue_min_arg];
	submissionRecord.stat = experiment.statistics[_pvalue_min_arg];
	submissionRecord.pvalue = experiment.pvalues[_pvalue_min_arg];
}

void Researcher::submitToJournal() {
	journal.review(submissionRecord);
}

void Researcher::setJournal(Journal &j) {
	// TODO: This is susspicious!
	journal = j;
}
