//
// Created by Amir Masoud Abdol on 2019-01-25.
//

#include <Researcher.h>

#include <iostream>

#include <Utilities.h>

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
    
    // Preparing the history ------------------------------
    // With this, I basically save everything, and then later on the decisionStrategy
    // can use it to make the final decision
    Experiment e = *experiment;
    experimentsList.push_back(e);
    submissionsList.push_back(decisionStrategy->selectOutcome(e));
    
    Submission sub;
    
	for (auto &h : hackingStrategies){
        
        if (hackingStyle == onOrig){
            // Just send the pointer
            Experiment* tempExpr = experiment;
            
            h->perform(tempExpr);
            experimentsList.push_back(*tempExpr);
            
//            printVector(tempExpr->means); std::cout <<
//            " :h, res, on orig, means [2]\n";
            sub = decisionStrategy->selectOutcome(*tempExpr);
        }
        else if (hackingStyle == onCopy){
            // Sending the copy
            Experiment copiedExpr = *experiment;
            
            h->perform(&copiedExpr);
            experimentsList.push_back(copiedExpr);
            
//            printVector(copiedExpr.means); std::cout <<
//            " :h, res, copy, means [2]\n";
            
            sub = decisionStrategy->selectOutcome(copiedExpr);
        }

        submissionsList.push_back(sub);
        
        decisionStrategy->verdict(submissionsList, experimentsList);
        
//        std::cout << decisionStrategy->finalSubmission << "\n";
        
        if (!decisionStrategy->isStillHacking){
            break;
        }
        
//        if (decisionStrategy == "asap"){
//            if (sub.pvalue < experiment->setup.alpha){
//                hackedSubmissions.push_back(sub);
//                return;
//            }
//        }
	}
    
    // Or, I can run all of them, and then decide
}

//Submission Researcher::_create_submission_record(int inx){
//    Submission sub;
//    sub.effect = experiment->effects[inx];
//    sub.statistic = experiment->statistics[inx];
//    sub.pvalue = experiment->pvalues[inx];
//
//    return sub;
//}

void Researcher::prepareTheSubmission() {
	if (!isHacker) {
        // Return the pre-registered outcome, always [0].
//        submissionRecord = _create_submission_record(0);
//        submissionRecord = Submission(*experiment, 0);
        submissionRecord = decisionStrategy->selectOutcome(*experiment);
	}else{
        
        submissionRecord = decisionStrategy->finalSubmission;
        
		// TODO: This needs generalization
//        submissionRecord = hackedSubmissions[0];
//        if (decisionStrategy == "asap"){
//            if (hackedSubmissions.size() != 0){
//                submissionRecord = submissionsList.back();
//        submissionRecord = decisionStrategy->selectBetweenSubmission(submissionsList);
//            }else{
//                submissionRecord = _create_submission_record(_selected_outcome_inx);
//            }
//        }else if (decisionStrategy == "all-in"){
//            // Check for the smallest p-value or largest effect
//        }
	}
}

void Researcher::submitToJournal() {
	journal->review(submissionRecord);
}

void Researcher::setJournal(Journal* j) {
	journal = j;
}

void Researcher::registerAHackingStrategy(HackingStrategy *h) {
	hackingStrategies.push_back(h);
}

void Researcher::rest() {
    experimentsList.clear();
    submissionsList.clear();
}

void Researcher::setDecisionStrategy(DecisionStrategy* d) {
    decisionStrategy = d;
}


