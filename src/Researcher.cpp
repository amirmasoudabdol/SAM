//
// Created by Amir Masoud Abdol on 2019-01-25.
//

#include <Researcher.h>

#include <iostream>

// void Researcher::setTestStrategy(TestStrategies *t) {
//     testStrategy = t;
// }


// void Researcher::runTest() {
//     testStrategy->run();
// }

//void Researcher::calculateEffect(){
//    experiment->effects = experiment->means;
//}

void Researcher::hack() {
    Submission sub;
	for (auto &h : hackingStrategies){
        // I can either check the significance here and stop after finding one
        sub = h->perform();
        
        if (decisionStrategy == "asap"){
            if (sub.pvalue < experiment->setup.alpha){
                hackedSubmissions.push_back(sub);
                return;
            }
        }
        
        
        
	}
    
    // Or, I can run all of them, and then decide
}

// This always select the pre-registered outcome, [0]
//void Researcher::selectTheOutcome() {
//    _selected_outcome_inx = 0;
////    _selected_outcome_inx= std::distance(experiment->pvalues.begin(), std::min_element(experiment->pvalues.begin(), experiment->pvalues.end()));
//}

Submission Researcher::_create_submission_record(int inx){
	Submission sub;
	sub.effect = experiment->effects[inx];
	sub.stat = experiment->statistics[inx];
	sub.pvalue = experiment->pvalues[inx];

	return sub;
}

void Researcher::prepareTheSubmission() {
	if (!isHacker) {
		submissionRecord = _create_submission_record(_selected_outcome_inx);
	}else{
		// TODO: This needs generalization
//        submissionRecord = hackedSubmissions[0];
        if (decisionStrategy == "asap"){
            if (hackedSubmissions.size() != 0){
                submissionRecord = hackedSubmissions.back();
            }else{
                submissionRecord = _create_submission_record(_selected_outcome_inx);
            }
        }else if (decisionStrategy == "all-in"){
            // Check for the smallest p-value or largest effect
        }
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
