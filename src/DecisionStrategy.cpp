//
// Created by Amir Masoud Abdol on 2019-02-01.
//

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

DecisionStrategy *DecisionStrategy::build(json &config) {

    if (config["name"] == "ImpatientDecisionMaker"){
        return new ImpatientDecisionMaker(stringToResearcherPreference.find(config["preference"])->second);
    }else if (config["name"] == "PatientDecisionMaker"){
        return new PatientDecisionMaker(stringToResearcherPreference.find(config["preference"])->second);
    }else{
        throw std::invalid_argument("Unknown DecisionStrategy");
    }
}

std::ostream& operator<<(std::ostream& os, DecisionPreference dp)
{
    switch(dp)
    {
        case DecisionPreference::PreRegisteredOutcome:
            os << "PreRegistered Outcome";
            break;
        case DecisionPreference::MinSigPvalue:
            os << "Minimum Significant Pvalue";
            break;
        case DecisionPreference::MinPvalue:
            os << "Minimum Pvalue";
            break;
        case DecisionPreference::MaxSigEffect:
            os << "Maximum Significant Effect";
            break;
        case DecisionPreference::MaxEffect:
            os << "Maximum Effect";
            break;
        case DecisionPreference::MinPvalueMaxEffect:
            os << "Maximum Effect with Min Pvalue";
            break;
        default:
            os.setstate(std::ios_base::failbit);
    }
    return os;
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
            // TODO: Activate me, I don't work because there are more than one effects
//            selectedOutcome = std::distance(experiment.effects.begin(), std::max_element(experiment.effects.begin(), experiment.effects.end()));
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
            // TODO: Activate me, I don't work because there are more than one effects
            //            selectedOutcome = std::distance(experiment.effects.begin(), std::max_element(experiment.effects.begin(), experiment.effects.end()));
            break;
            
        case DecisionPreference::MinPvalueMaxEffect:
            
            break;
            
        default:
            break;
    }

    return submissions_pool.back();
    
}


bool ImpatientDecisionMaker::initDecision(Experiment &experiment) {
    Submission sub = selectOutcome(experiment);
    
    // Preparing pools anyway
    experiments_pool.push_back(experiment);
    submissions_pool.push_back(sub);
        
    return !isPublishable(sub);
}

bool ImpatientDecisionMaker::intermediateDecision(Experiment &experiment) {

    is_still_hacking = !isPublishable(selectOutcome(experiment));
    return is_still_hacking;
}

bool ImpatientDecisionMaker::afterhackDecision(Experiment &experiment) {
    Submission sub = selectOutcome(experiment);
                
        
    if (isPublishable(sub)){
        experiments_pool.push_back(experiment);
        submissions_pool.push_back(sub);
        
        is_still_hacking = false;
    }else{
        is_still_hacking = true;
    }

    return is_still_hacking;
}

bool ImpatientDecisionMaker::finalDecision(Experiment &experiment) {
    // TODO: This can be implemented differenly if necessary

    final_submission = submissions_pool.back();
    
    clearHistory();
    
    is_still_hacking = false;
    return is_still_hacking;
}



bool ImpatientDecisionMaker::verdict(Experiment &experiment, DecisionStage stage) {
    switch(stage){
        case DecisionStage::Initial:
            return initDecision(experiment);
            break;
        case DecisionStage::WhileHacking:
            return intermediateDecision(experiment);
            break;
        case DecisionStage::DoneHacking:
            return afterhackDecision(experiment);
            break;
        case DecisionStage::Final:
            return finalDecision(experiment);
            break;
    }
    
}



bool PatientDecisionMaker::initDecision(Experiment &experiment) {
    Submission sub = selectOutcome(experiment);
        
    experiments_pool.push_back(experiment);
    submissions_pool.push_back(sub);
    
    is_still_hacking = !isPublishable(sub);
    return is_still_hacking;
}

bool PatientDecisionMaker::intermediateDecision(Experiment &experiment) {
    
    
    is_still_hacking = !isPublishable(selectOutcome(experiment));
    return is_still_hacking;
}

bool PatientDecisionMaker::afterhackDecision(Experiment &experiment) {
    Submission sub = selectOutcome(experiment);
    
    if (isPublishable(sub)){
        experiments_pool.push_back(experiment);
        submissions_pool.push_back(sub);
    }
    
    is_still_hacking = !isPublishable(sub);
    return is_still_hacking;
}

bool PatientDecisionMaker::finalDecision(Experiment &experiment) {

    final_submission = selectBetweenSubmissions();
    
    clearHistory();
    
    is_still_hacking = false;
    return is_still_hacking;
}


bool PatientDecisionMaker::verdict(Experiment &experiment, DecisionStage stage){
    switch (stage) {
        
        case DecisionStage::Initial:
            return initDecision(experiment);
            break;
        case DecisionStage::WhileHacking:
            return intermediateDecision(experiment);
            break;
        case DecisionStage::DoneHacking:
            return afterhackDecision(experiment);
            break;
        case DecisionStage::Final:
            return finalDecision(experiment);
            break;
    }
}


