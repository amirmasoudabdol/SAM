//
// Created by Amir Masoud Abdol on 2019-02-01.
//

#include "DecisionStrategy.h"

#include <fmt/core.h>
#include <fmt/format.h>
#include <sol/sol.hpp>
#include <spdlog/spdlog.h>

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

    if (decision_strategy_config["_name"] == "ImpatientDecisionMaker"){
        
        auto params = decision_strategy_config.get<ImpatientDecisionMaker::Parameters>();
        return std::make_unique<ImpatientDecisionMaker>(params);
        
    }else if (decision_strategy_config["_name"] == "PatientDecisionMaker"){

        auto params = decision_strategy_config.get<PatientDecisionMaker::Parameters>();
        return std::make_unique<PatientDecisionMaker>(params);

    }else if (decision_strategy_config["_name"] == "HonestDecisionMaker"){

        return std::make_unique<HonestDecisionMaker>();
        
    }else{
        throw std::invalid_argument("Unknown DecisionStrategy");
    }
}

Submission DecisionStrategy::selectOutcome(Experiment& experiment, const DecisionPreference &preference) {
    
    /// We always start with the pre_registered_group, and if we find others
    /// results based on researchers' preference, then we replace it, and report
    /// that one.
    /// CHECK ME: I'm not sure if this is a good way of doing this...
    int selectedOutcome {0};
    
    pre_registered_group = experiment.setup.nd();
                
    auto begin = experiment.groups_.begin() + 1;
    auto end = experiment.groups_.end();
    
    this->complying_with_preference = true;
    for (int i {0}; i < policies_type.size(); ++i) {
        
        auto &type = policies_type[i];
        auto &func = policies_func[i];
        
        switch (type) {
                
            case PolicyType::Min: {
                auto it = std::min_element(begin, end, func);
                spdlog::debug("min: ");
                spdlog::debug(*it);
                selectedOutcome = it->id_;
            }
                break;
                
            case PolicyType::Max: {
                auto it = std::max_element(begin, end, func);
                spdlog::debug("max: ");
                spdlog::debug(*it);
                selectedOutcome = it->id_;
            }
                break;
                
            case PolicyType::Comp: {
                auto pit = std::partition(begin, end, func);
                spdlog::debug("comp: ");
                for (auto it {begin}; it != pit; ++it) {
                    spdlog::debug(*it);
                }
                end = pit;
            }
                
                break;
                
            case PolicyType::Random: {
                /// Shuffling the array and setting the end pointer to the first time,
                /// this basically mimic the process of selecting a random element from
                /// the list.
                Random::shuffle(begin, end);
                spdlog::debug("random");
                spdlog::debug(*begin);
                end = begin;
            }
                break;
        }
        
        if (begin == end) {
            spdlog::debug("Using the fallback policy.");
            selectedOutcome = pre_registered_group;
            this->complying_with_preference = false;
            break;
        }
        
    }
    
    return {experiment, selectedOutcome};
}


/// This is often is being used by PatientDecisionMaker
Submission DecisionStrategy::selectBetweenSubmissions(const DecisionPreference &preference){
    


    return submissions_pool.back();
    
}


bool DecisionStrategy::willBeSubmitting() {
        
    auto is_it_submittable = std::all_of(submission_policies_func.begin(), submission_policies_func.end(),
                                           [this](auto &pred){return pred(this->current_submission); });
    
    return is_it_submittable;
    
}

/// Impatient decision maker keeps the initial study and stop the hacking
/// process if the results is already satistifactory.
///
/// @param experiment A reference to the experiment.
void ImpatientDecisionMaker::initDecision(Experiment &experiment) {
    
    // Preparing pools anyway
    experiments_pool.push_back(experiment);
    submissions_pool.push_back(current_submission);
    
    is_still_hacking = !willBeSubmitting();
}

/// Impatient decision maker check if the check current submission is publishable
/// or not, if not, it'll continue hacking... and will stop as soon as it
/// finds a publishable solution. This is different in the case of patient
/// decision maker for instance.
void ImpatientDecisionMaker::intermediateDecision(Experiment &experiment) {

    is_still_hacking = !willBeSubmitting();
}


void ImpatientDecisionMaker::afterhackDecision(Experiment &experiment) {
    
    
    if (willBeSubmitting()){
        experiments_pool.push_back(experiment);
        submissions_pool.push_back(current_submission);
    }

    is_still_hacking = !willBeSubmitting();
    
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

    is_still_hacking = !willBeSubmitting();
}


/// Patient decision maker keeps track of its intermediate results until it
/// makes the final decision and choose between them.
///
/// @param experiment A reference to the experiment
void PatientDecisionMaker::afterhackDecision(Experiment &experiment) {

    if (willBeSubmitting()){
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

