//
// Created by Amir Masoud Abdol on 2019-01-25.
//

#include <Researcher.h>

#include <iostream>

#include <Utilities.h>

void Researcher::hack() {
    
    Submission sub;
    
    for (auto &h : hackingStrategies){
        
        if (hackingStyle == onOrig){
            // Just send the pointer
            Experiment* tempExpr = experiment;
            
            h->perform(tempExpr, decisionStrategy);
//            experimentsList.push_back(*tempExpr);
//
//            sub = decisionStrategy->selectOutcome(*tempExpr);
            
            decisionStrategy->verdict(*tempExpr,
                                      DecisionStage::DoneHacking);
        }
        else if (hackingStyle == onCopy){
            // Sending the copy
            Experiment copiedExpr = *experiment;
            
            h->perform(&copiedExpr, decisionStrategy);
            copiedExpr.isHacked = true;
//            experimentsList.push_back(copiedExpr);
//
//            sub = decisionStrategy->selectOutcome(copiedExpr);
            
            decisionStrategy->verdict(copiedExpr,
                                      DecisionStage::DoneHacking);
        }
        
//        submissionsList.push_back(sub);
        
        // Checking if the researcher is happy with the current result
//        decisionStrategy->verdict(submissionsList, experimentsList);
        
        
        // If the researcher statisfied, hacking routine will be stopped
        if (!decisionStrategy->isStillHacking){
            break;
        }
        
    }
    
    // Or, I can run all of them, and then decide
}

void Researcher::prepareTheSubmission() {
    
    // decisionStrategy->verdict(submissionsList, experimentsList);

    submissionRecord = decisionStrategy->finalSubmission;

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


void Researcher::setDecisionStrategy(DecisionStrategy* d) {
    decisionStrategy = d;
}


/**
 * \brief      Prepares the research by cleaning up the memeory, 
 * randomizing the ExperimentSetup parameters, allocating data and 
 * finally generating the data using the DataGenStrategy
 */
void Researcher::prepareResearch() {
    
    // Randomize if necessary
    // TODO: I need to handle this better because nobs is being assigned in the
    // ExperimentSetup before this as well, when reading other parameteres.
    if (this->experiment->setup.isNRandomized){
        this->experiment->randomize();
    }
    
    // Allocating memory
    this->experiment->initResources();
    
    // Generating data using the dataStrategy
    this->experiment->generateData();
}

/**
 * \brief      Performs the research by calculating the statistics, calculating the effects,
 * and running the test. In the case where the researcher is a hacker, the researcher will 
 * apply the hacking methods on the `experiment`.
 */
void Researcher::performResearch(){
    
    // hack 1
    
    this->experiment->calculateStatistics();
    
    this->experiment->calculateEffects();
        
    this->experiment->runTest();
    
    // See #50
    // Experiment e = *this->experiment;
    // this->experimentsList.push_back(e);
    // this->submissionsList.push_back(this->decisionStrategy->selectOutcome(e));

    bool isPublishable = this->decisionStrategy->verdict(*this->experiment,
                                                         DecisionStage::Initial);
    
    // I think I need to make a decision here whether the result is signficant or not,
    // or it complies with researcher's preference, if not then, I should hack
    
    if (this->isHacker && !isPublishable){
        this->hack();
    }
}

/**
 * \brief      Prepares the submission record by asking the `decisionStrategy` to pick
 * his prefered submission record from the list of available submission, `submissionsList`.
 * AFter than, it'll submit the submission record to the selected `journal`.
 */
void Researcher::publishResearch(){
    
    // hack 3
    
    this->decisionStrategy->verdict(*this->experiment,
                                    DecisionStage::Final);

    this->journal->review(this->decisionStrategy->finalSubmission);
   
    // this->prepareTheSubmission();
    
    // this->submitToJournal();
    
}
