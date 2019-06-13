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
        
    }else if (decision_strategy_config["name"] == "PatientDecisionMaker"){

        auto params = decision_strategy_config.get<PatientDecisionMaker::Parameters>();
        return std::make_unique<PatientDecisionMaker>(params);

    }else if (decision_strategy_config["name"] == "HonestDecisionMaker"){

        return std::make_unique<HonestDecisionMaker>();
        
    }else{
        throw std::invalid_argument("Unknown DecisionStrategy");
    }
}

Submission DecisionStrategy::selectOutcome(Experiment& experiment, const DecisionPreference &preference) {
    
    /// We always start with the pre_registered_group, and if we find others
    /// results based on researchers' preference, then we replace it, and report
    /// that one.
    int selectedOutcome = pre_registered_group;
    
    switch (preference) {
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
    }
    
    Submission tmpSub(experiment, selectedOutcome);
    
    return tmpSub;
}


Submission DecisionStrategy::selectBetweenSubmissions(const DecisionPreference &preference){
    
    switch (preference) {
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



/// Impatient decision maker keeps the initial study and stop the hacking
/// process if the results is already satistifactory.
///
/// @param experiment A reference to the experiment.
void ImpatientDecisionMaker::initDecision(Experiment &experiment) {
    
    // Preparing pools anyway
    experiments_pool.push_back(experiment);
    submissions_pool.push_back(current_submission);
    
    is_still_hacking = !isPublishable();
}

/// Impatient decision maker check if the check current submission is publishable
/// or not, if not, it'll continue hacking... and will stop as soon as it
/// finds a publishable solution. This is different in the case of patient
/// decision maker for instance.
void ImpatientDecisionMaker::intermediateDecision(Experiment &experiment) {

    is_still_hacking = !isPublishable();
}


void ImpatientDecisionMaker::afterhackDecision(Experiment &experiment) {
    
    
    if (isPublishable()){
        experiments_pool.push_back(experiment);
        submissions_pool.push_back(current_submission);
    }

    is_still_hacking = !isPublishable();
    
}

void ImpatientDecisionMaker::finalDecision(Experiment &experiment) {

    final_submission = submissions_pool.back();
    
    clearHistory();
    
    // Done Hacking...
    is_still_hacking = false;
}



ImpatientDecisionMaker& ImpatientDecisionMaker::verdict(Experiment &experiment, DecisionStage stage) {
    
    current_submission = selectOutcome(experiment, params.preference);
    
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


/// It'll add the experiment and submission to the pool, regardless of the
/// significance.
///
/// It'll update the verdict on whether the researcher will continue hacking or
/// not. The researcher can check this flag by calling `isStillHacking()` routine.
///
/// @param experiment A reference to the experiment
void PatientDecisionMaker::initDecision(Experiment &experiment) {

    experiments_pool.push_back(experiment);
    submissions_pool.push_back(current_submission);

    is_still_hacking = true;
}


/// A patient decision maker is still optimizing for the effort, he'd not continue
/// hacking if an intermediate result is already publishable
///
/// @param experiment A reference to the experiment
void PatientDecisionMaker::intermediateDecision(Experiment &experiment) {

    is_still_hacking = !isPublishable();
}


/// Patient decision maker keeps track of its intermediate results until it
/// makes the final decision and choose between them.
///
/// @param experiment A reference to the experiment
void PatientDecisionMaker::afterhackDecision(Experiment &experiment) {

    if (isPublishable()){
        experiments_pool.push_back(experiment);
        submissions_pool.push_back(current_submission);
    }

    // Patient Decision maker always continue hacking...
    is_still_hacking = true;
}


/// Impatient decision maker — at the final stage — goes through all solutions
/// and select the one based on its preference.
///
/// @param experiment A reference to the experiment.
void PatientDecisionMaker::finalDecision(Experiment &experiment) {

    final_submission = selectBetweenSubmissions(params.preference);

    clearHistory();

    is_still_hacking = false;
}


PatientDecisionMaker& PatientDecisionMaker::verdict(Experiment &experiment,
                                                        DecisionStage stage) {
    
    current_submission = selectOutcome(experiment, params.preference);
    
    switch (stage) {

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


HonestDecisionMaker& HonestDecisionMaker::verdict(Experiment &experiment,
DecisionStage stage) {
    
    current_submission = selectOutcome(experiment, params.preference);
    
    submissions_pool.push_back(current_submission);
    experiments_pool.push_back(experiment);
    
    final_submission = current_submission;
    
    return *this;
}

