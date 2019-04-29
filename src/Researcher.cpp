//
// Created by Amir Masoud Abdol on 2019-01-25.
//

#include <Researcher.h>

#include <iostream>


void Researcher::hack() {
    
    Submission sub;
    
    for (auto &set : hacking_strategies){
        
        // For each set, we make a copy of the experiment and apply the given
        // set of methods over each other.
        
        Experiment copiedExpr = *experiment;
        
        for (auto &h : set){
            
            h->perform(&copiedExpr, decision_strategy);
            copiedExpr.is_hacked = true;
            copiedExpr.hackingHistory.push_back((int)h->hid);
            
            decision_strategy->verdict(copiedExpr,
                                      DecisionStage::DoneHacking);
            
            // If the researcher statisfied, hacking routine will be stopped
            if (!decision_strategy->isStillHacking()){
                break;
            }
            
        }
    }
}

void Researcher::registerAHackingStrategy(HackingStrategy *h) {
    // TODO: The register builder needs some work, especially for hacking methods
//    hackingStrategies.push_back({h});
}


/**
 * @brief      Prepares the research by cleaning up the memeory, 
 * randomizing the ExperimentSetup parameters, allocating data and 
 * finally generating the data using the DataGenStrategy
 */
void Researcher::prepareResearch() {
    
    // Randomize if necessary
    // TODO: I need to handle this better because nobs is being assigned in the
    // ExperimentSetup before this as well, when reading other parameteres.
    if (this->experiment->setup.is_n_randomized){
        this->experiment->randomize();
    }
    
    // Allocating memory
    this->experiment->initResources(experiment->setup.ng);
    
    // Generating data using the dataStrategy
    this->experiment->generateData();
}

/**
 * @brief      Performs the research by calculating the statistics, calculating the effects,
 * and running the test. In the case where the researcher is a hacker, the researcher will 
 * apply the hacking methods on the `experiment`.
 */
void Researcher::performResearch(){
    
    // hack 1
    
    this->experiment->calculateStatistics();
    
    this->experiment->calculateEffects();
    
    this->experiment->runTest();

    // 
    bool willHack = this->decision_strategy->verdict(*this->experiment,
                                                         DecisionStage::Initial);
    
    if (this->is_hacker && willHack){
        this->hack();
    }    
}

/**
 * @brief      Prepares the submission record by asking the `decisionStrategy` to pick
 * his prefered submission record from the list of available submission, `submissionsList`.
 * AFter than, it'll submit the submission record to the selected `journal`.
 */
void Researcher::publishResearch(){
    
    // hack 3
    
    this->decision_strategy->verdict(*this->experiment,
                                    DecisionStage::Final);

    this->journal->review(this->decision_strategy->final_submission);
    
}
