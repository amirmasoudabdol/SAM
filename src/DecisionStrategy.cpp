//
// Created by Amir Masoud Abdol on 2019-02-01.
//

#include <utils/magic_enum.hpp>

#include "DecisionStrategy.h"

using namespace sam;

/**
 * Pure deconstructor of DecisionStrategy.
 * This is necessary for proper deconstruction of derived
 * classes.
 */
DecisionStrategy::~DecisionStrategy() {
    // pure deconstructor
};

std::unique_ptr<DecisionStrategy> DecisionStrategy::build(json &decision_strategy_config) {

    if (decision_strategy_config["name"] == "ImpatientDecisionMaker"){
        
        auto params = decision_strategy_config.get<ImpatientDecisionMaker::Parameters>();
        return std::make_unique<ImpatientDecisionMaker>(params);
        
//    }else if (decision_strategy_config["name"] == "PatientDecisionMaker"){
//
//        auto params = decision_strategy_config.get<PatientDecisionMaker::Parameters>();
//        return std::make_unique<PatientDecisionMaker>(params);
//
//    }else if (decision_strategy_config["name"] == "HonestDecisionMaker"){
//
//        return std::make_unique<HonestDecisionMaker>();
        
    }else{
        throw std::invalid_argument("Unknown DecisionStrategy");
    }
}

Submission DecisionStrategy::selectOutcome(Experiment& experiment) {
    
    int selectedOutcome = pre_registered_group;
    
    switch (selectionPref) {
        case DecisionPreference::PreRegisteredOutcome:
            selectedOutcome = pre_registered_group;
            break;
            
        case DecisionPreference::MinSigPvalue:
            
            break;
            
        case DecisionPreference::MinPvalue:
            selectedOutcome = std::distance(experiment.pvalues.begin(), std::min_element(experiment.pvalues.begin(), experiment.pvalues.end()));
            break;
            
        case DecisionPreference::MaxSigEffect:
            
            break;
            
        case DecisionPreference::MaxEffect:
            selectedOutcome = std::distance(experiment.effects.begin(), std::max_element(experiment.effects.begin(), experiment.effects.end()));
            break;
            
        case DecisionPreference::MinPvalueMaxEffect:
            
            break;
            
        default:
            selectedOutcome = 0;
            break;
    }
    
    Submission tmpSub(experiment, selectedOutcome);
    
    return tmpSub;
}


Submission DecisionStrategy::selectBetweenSubmissions(){
    
    switch (selectionPref) {
        case DecisionPreference::PreRegisteredOutcome:
            break;
            
        case DecisionPreference::MinSigPvalue:

            break;
            
        case DecisionPreference::MinPvalue:
            {
                // TODO: This should be a routine in Experiment
                std::vector<double> pvalues;
                std::transform(submissions_pool.begin(), submissions_pool.end(), std::back_inserter(pvalues), [](const Submission &s) {return s.pvalue;} );
                int min_pvalue_inx = std::distance(pvalues.begin(),
                                                   std::min_element(pvalues.begin(),
                                                                    pvalues.end()));
                return submissions_pool[min_pvalue_inx];
            }
            break;
            
        case DecisionPreference::MaxSigEffect:
            
            break;
            
        case DecisionPreference::MaxEffect:
            
            break;
            
        case DecisionPreference::MinPvalueMaxEffect:
            
            break;
            
        default:
            break;
    }

    return submissions_pool.back();
    
}


void ImpatientDecisionMaker::initDecision(Experiment &experiment) {
//    Submission sub = selectOutcome(experiment);
    
    // Preparing pools anyway
    experiments_pool.push_back(experiment);
    submissions_pool.push_back(current_submission);
    
//    return !isPublishable();
}

void ImpatientDecisionMaker::intermediateDecision(Experiment &experiment) {

    /// Impatient decision maker check if the check current submission is publishable
    /// or not, if not, it'll continue hacking... and will stop as soon as it
    /// finds a publishable solution. This is different in the case of patient
    /// decision maker for instance.
    is_still_hacking = !isPublishable();
//    return is_still_hacking;
}

void ImpatientDecisionMaker::afterhackDecision(Experiment &experiment) {
//    Submission sub = selectOutcome(experiment);
    
    
    if (isPublishable()){
        experiments_pool.push_back(experiment);
        submissions_pool.push_back(current_submission);
    }

    is_still_hacking = !isPublishable();
    
//    return is_still_hacking;
}

void ImpatientDecisionMaker::finalDecision(Experiment &experiment) {

    final_submission = submissions_pool.back();
    
    clearHistory();
    
    // Done Hacking...
    is_still_hacking = false;
//    return is_still_hacking;
}



ImpatientDecisionMaker& ImpatientDecisionMaker::verdict(Experiment &experiment, DecisionStage stage) {
    
    current_submission = selectOutcome(experiment);
    
    switch(stage){
        case DecisionStage::Initial:
            initDecision(experiment);
            break;
        case DecisionStage::WhileHacking:
            intermediateDecision(experiment);
            break;
        case DecisionStage::DoneHacking:
            afterhackDecision(experiment);
            break;
        case DecisionStage::Final:
            finalDecision(experiment);
            break;
    }
    
    return *this;
    
}



//bool PatientDecisionMaker::initDecision(Experiment &experiment) {
//    Submission sub = selectOutcome(experiment);
//
//    experiments_pool.push_back(experiment);
//    submissions_pool.push_back(sub);
//
//    is_still_hacking = !isPublishable(sub);
//    return is_still_hacking;
//}
//
//bool PatientDecisionMaker::intermediateDecision(Experiment &experiment) {
//
//
//    is_still_hacking = !isPublishable(selectOutcome(experiment));
//    return is_still_hacking;
//}
//
//bool PatientDecisionMaker::afterhackDecision(Experiment &experiment) {
//    Submission sub = selectOutcome(experiment);
//
//    if (isPublishable(sub)){
//        experiments_pool.push_back(experiment);
//        submissions_pool.push_back(sub);
//    }
//
//    is_still_hacking = !isPublishable(sub);
//    return is_still_hacking;
//}
//
//bool PatientDecisionMaker::finalDecision(Experiment &experiment) {
//
//    final_submission = selectBetweenSubmissions();
//
//    clearHistory();
//
//    is_still_hacking = false;
//    return is_still_hacking;
//}
//
//
//bool PatientDecisionMaker::verdict(Experiment &experiment, DecisionStage stage){
//    switch (stage) {
//
//        case DecisionStage::Initial:
//            return initDecision(experiment);
//            break;
//        case DecisionStage::WhileHacking:
//            return intermediateDecision(experiment);
//            break;
//        case DecisionStage::DoneHacking:
//            return afterhackDecision(experiment);
//            break;
//        case DecisionStage::Final:
//            return finalDecision(experiment);
//            break;
//    }
//}
//
//
