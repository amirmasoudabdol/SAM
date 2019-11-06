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
    
    for (auto &set : hacking_strategies){
        
        // For each set, we make a copy of the experiment and apply the given
        // set of methods over each other.
        
        Experiment copiedExpr = *experiment;
        
        for (auto &h : set){
            
            h->perform(&copiedExpr, decision_strategy.get());
            copiedExpr.is_hacked = true;
            copiedExpr.hacks_history.push_back(enum_integer<HackingMethod>(h->name));
            
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
 Iterating over the registrated methods and run them on the current experiment.
 
 @note This has a very similar implemention to the `hack()` but it doesn't perform any of
 the secondary checks.
 
 @bug I think there is a possible bug here, since pre-processing methods can be much more aggresive,
 they can cause some issues when it comes to calculating statistics.
 */
void Researcher::preProcessData() {
    
    static NoDecision no_decision = NoDecision();
    
    experiment->calculateStatistics();
    
    for (auto &method : pre_processing_methods){
        
        method->perform(experiment, &no_decision);
        
    }
}

/**
 Prepares the research by cleaning up the memeory, randomizing the
 ExperimentSetup parameters, allocating data and finally generating the data
 using the DataGenStrategy
 */
void Researcher::prepareResearch() {
    
    // Randomize if necessary
    // The check is being performed in the `randomize()` function based on whether
    // a distribution is provided for any of the parameters.
    experiment->randomize();
    
    // Allocating memory
    experiment->initResources(experiment->setup.ng());
    
    // Generating data using the dataStrategy
    experiment->generateData();
    
    // Performing the Pre-processing if any
    if (is_pre_processing)
        preProcessData();
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

    if (decision_strategy->verdict(*experiment, DecisionStage::Initial).isStillHacking() && isHacker()){
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
    decision_strategy->verdict(*experiment, DecisionStage::Final);
    
    // Setting researcher's submission record
    submission_record = decision_strategy->final_submission;
    
    // Submit the final submission to the Jouranl
    journal->review(decision_strategy->final_submission);
    
}


void Researcher::research() {

    prepareResearch();

    performResearch();

    publishResearch();
    
}
