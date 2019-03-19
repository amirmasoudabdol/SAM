//
// Created by Amir Masoud Abdol on 2019-02-01.
//

#include "DecisionStrategy.h"

/**
 * Pure deconstructor of DecisionStrategy.
 * This is necessary for proper deconstruction of derived
 * classes.
 */
DecisionStrategy::~DecisionStrategy() {
    // pure deconstructor
};

DecisionStrategy *DecisionStrategy::buildDecisionStrategy(json &config) {
    // TODO: Expand
    if (config["type"] == "Impatient Decision Maker"){
        return new ImpatientDecisionMaker(stringToResearcherPreference.find(config["preference"])->second);
    }else if (config["type"] == "Patient Decision Maker"){
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
            // CHECKME: What's this?
            os.setstate(std::ios_base::failbit);
    }
    return os;
}

Submission DecisionStrategy::_select_Outcome(Experiment& experiment) {
    
    int selectedOutcome = preRegGroup;
    
    switch (selectionPref) {
        case DecisionPreference::PreRegisteredOutcome:
            selectedOutcome = preRegGroup;
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



bool ImpatientDecisionMaker::verdict(Experiment &experiment, DecisionStage stage) {
    switch(stage){
        case DecisionStage::Initial:
        {
            Submission sub = selectOutcome(experiment);
            
            // Preparing pools anyway
            experimentsPool.push_back(experiment);
            submissionsPool.push_back(sub);
            
            if (isPublishable(sub)){
                return true;
            }else{
                return false;
            }
        }
            break;
        case DecisionStage::WhileHacking:
        {
            bool publishable = isPublishable(selectOutcome(experiment));
            isStillHacking = publishable;
            return publishable;
        }
            break;
        case DecisionStage::DoneHacking:
        {
            Submission sub = selectOutcome(experiment);
            if (isPublishable(sub)){
                experimentsPool.push_back(experiment);
                submissionsPool.push_back(sub);
                
                return true;
            }else{
                isStillHacking = true;
                return isStillHacking;
            }
        }
            break;
        case DecisionStage::Final:
            // TODO: This can be implemented differenly if necessary
        {
            finalSubmission = submissionsPool.back();
            experimentsPool.clear();
            submissionsPool.clear();
            return true;
            
        }
            break;
    }
    
}



bool PatientDecisionMaker::initDecision(Experiment &experiment) {
    Submission sub = selectOutcome(experiment);
    
    experimentsPool.push_back(experiment);
    submissionsPool.push_back(sub);
    
    isStillHacking = !isPublishable(sub);
    return isStillHacking;
}

bool PatientDecisionMaker::intermediateDecision(Experiment &experiment) {
    bool publishable = isPublishable(selectOutcome(experiment));
    isStillHacking = publishable;
    return isStillHacking;
}

bool PatientDecisionMaker::afterhackDecision(Experiment &experiment) {
    Submission sub = selectOutcome(experiment);
    
    if (isPublishable(sub)){
        experimentsPool.push_back(experiment);
        submissionsPool.push_back(sub);
    }
    
    isStillHacking = true;
    return isStillHacking;
}

bool PatientDecisionMaker::finalDecision(Experiment &experiment) {

    std::vector<double> pvalues(submissionsPool.size());
    std::transform(submissionsPool.begin(), submissionsPool.end(), std::back_inserter(pvalues), [](const Submission &s) {return s.pvalue;} );
    int min_pvalue_inx = std::distance(pvalues.begin(),
                                       std::min_element(pvalues.begin(),
                                                        pvalues.end()));
    
    finalSubmission = submissionsPool[min_pvalue_inx];
    
    experimentsPool.clear();
    submissionsPool.clear();
    
    return false;
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


