//
// Created by Amir Masoud Abdol on 2019-01-25.
//

#include <iostream>
#include <utils/magic_enum.hpp>

#include "Researcher.h"

using namespace sam;

//class ResearcherBuilder;

ResearcherBuilder Researcher::create(std::string name) {
    return ResearcherBuilder(name);
}

void Researcher::hack() {
    
    using namespace magic_enum;
    
    Submission sub;
    
    for (auto &set : hacking_strategies){
        
        // For each set, we make a copy of the experiment and apply the given
        // set of methods over each other.
        
        Experiment copiedExpr = *experiment;
        
        for (auto &h : set){
            
            h->perform(&copiedExpr, decision_strategy.get());
            copiedExpr.is_hacked = true;
            copiedExpr.hacks_history.push_back(enum_integer<HackingMethod>(h->params.name));
            
            decision_strategy->verdict(copiedExpr,
                                      DecisionStage::DoneHacking);
            
            // If the researcher statisfied, hacking routine will be stopped
            if (!decision_strategy->isStillHacking()){
                break;
            }
            
        }
    }
}

/**
 Prepares the research by cleaning up the memeory, randomizing the
 ExperimentSetup parameters, allocating data and finally generating the data
 using the DataGenStrategy
 */
void Researcher::prepareResearch() {
    
    // Randomize if necessary
    // TODO: I need to handle this better because nobs is being assigned in the
    // ExperimentSetup before this as well, when reading other parameteres.
    if (experiment->setup.is_n_randomized){
        experiment->randomize();
    }
    
    // Allocating memory
    experiment->initResources(experiment->setup.ng());
    
    // Generating data using the dataStrategy
    experiment->generateData();
}

/**
 Performs the research by calculating the statistics, calculating the effects,
 and running the test. In the case where the researcher is a hacker, the
 researcher will apply the hacking methods on the `experiment`.
 */
void Researcher::performResearch(){
        
    experiment->calculateStatistics();
    
    experiment->calculateEffects();
    
    experiment->runTest();

    // 
    bool willHack = decision_strategy->verdict(*experiment,
                                               DecisionStage::Initial);
    

    if (is_hacker && willHack){
        hack();
    }    
}

/**
 Prepares the submission record by asking the `decisionStrategy` to pick
 his prefered submission record from the list of available submission,
 `submissionsList`. After than, it'll submit the submission record to the
 selected `journal`.
 */
void Researcher::publishResearch(){
    
    // Ask for the final decision    
    decision_strategy->verdict(*experiment,
                                      DecisionStage::Final);
    
    // Setting researcher's submission record
    submission_record = decision_strategy->final_submission;
    
    // Submit the final submission to the Jouranl
    journal->review(decision_strategy->final_submission);
    
}
