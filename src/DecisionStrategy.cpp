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
    
    switch (preference) {
        case DecisionPreference::PreRegisteredOutcome: {
            selectedOutcome = pre_registered_group;
            this->complying_with_preference = true;
            

        }
            break;
            
        case DecisionPreference::Policies: {
            
            auto begin = experiment.groups_.begin() + 1;
            auto end = experiment.groups_.end();
            
            for (int i{0}; i < policies_type.size(); ++i) {
                
                auto &type = policies_type[i];
                auto &func = policies_func[i];
                
//                std::cout << type << std::endl;
                
                if (type.find("min") != std::string::npos) {
                    auto it = std::min_element(begin, end, func);
//                    std::cout << "\t min: " << *it << std::endl;
                    selectedOutcome = it->id_;
                } else if (type.find("max") != std::string::npos) {
                    auto it = std::max_element(begin, end, func);
//                    std::cout << "\t max: " << *it << std::endl;
                    selectedOutcome = it->id_;
                }
                else {
                    auto pit = std::partition(begin, end, func);
//                    for (auto it {begin}; it != pit; ++it) {
//                        std::cout << "\t cond: " << *it << std::endl;
//                    }
                    end = pit;
                }
                
                if (begin == end) {
                    spdlog::debug("Using the fallback policy.");
                    selectedOutcome = pre_registered_group;
                    break;
                }
                    
            }
            
            std::cout << "------------------------\n\n\n";
        }
            break;
            
//        case DecisionPreference::MinSigPvalue: {
//
//            /// Getting the indexes corresponding to significant results
//            arma::uvec sig_indexes = arma::find(experiment.sigs.tail(experiment.setup.ng() - experiment.setup.nd()) == 1) + experiment.setup.nd();
//
//                if (not sig_indexes.is_empty()) {
//                    /// Selecting the smallest p-value between the groups with significant results,
//                    /// and translating/selecting the correct index.
//                    selectedOutcome = sig_indexes[experiment.pvalues.elem(sig_indexes).index_min()];
//
//                    this->complying_with_preference = true;
//                }else{
//                    /// Returning min p-value if I couldn't find any significant p-value
//                    selectedOutcome = experiment.pvalues.tail(experiment.setup.ng() - experiment.setup.nd()).index_min()
//                    + experiment.setup.nd();
//
//                    this->complying_with_preference = false;
//                }
//            }
//            break;
//
//        case DecisionPreference::MaxSigPvalue: {
//            /// Getting the indexes corresponding to significant results
//            arma::uvec sig_indexes = arma::find(experiment.sigs.tail(experiment.setup.ng() - experiment.setup.nd()) == 1) + experiment.setup.nd();
//
//                if (not sig_indexes.is_empty()) {
//                    /// Selecting the smallest p-value between the groups with significant results,
//                    /// and translating/selecting the correct index.
//                    selectedOutcome = sig_indexes[experiment.pvalues.elem(sig_indexes).index_max()];
//
//                    this->complying_with_preference = true;
//                }else{
//                    /// Returning min p-value if I couldn't find any significant p-value
//                    selectedOutcome = experiment.pvalues.tail(experiment.setup.ng() - experiment.setup.nd()).index_max()
//                    + experiment.setup.nd();
//
//                    this->complying_with_preference = false;
//                }
//            }
//            break;
//
//        case DecisionPreference::MinPvalue: {
//
//            /// By taking the "tail" I'm basically skipping the control group, and shifting it back when I found it
//            selectedOutcome = experiment.pvalues.tail(experiment.setup.ng() - experiment.setup.nd()).index_min()
//                                + experiment.setup.nd();
//
//            this->complying_with_preference = true;
//            }
//            break;
//
//        case DecisionPreference::MaxSigEffect: {
//
//            }
//            break;
//
//        case DecisionPreference::MaxEffect: {
//
//            selectedOutcome = experiment.effects.tail(experiment.setup.ng() - experiment.setup.nd()).index_max()
//                                + experiment.setup.nd();
//
//            this->complying_with_preference = true;
//            }
//            break;
//
//        case DecisionPreference::MinPvalueMaxEffect:
//
//            break;
//
//        case DecisionPreference::RandomSigPvalue: {
//            /// Again taking the tail, and shifting the index back into its original range
//            /// Suffling the index vector before selecting the sig outcome.
//            arma::uvec sig_indexes = arma::shuffle(arma::find(experiment.sigs.tail(experiment.setup.ng() - experiment.setup.nd()) == 1)) + experiment.setup.nd();
//
//                if (not sig_indexes.is_empty()) {
//                    selectedOutcome = sig_indexes.at(0);
//                    this->complying_with_preference = true;
//                }else{
//                    /// Returning min p-value if I couldn't find any significant p-value
//                    selectedOutcome = experiment.pvalues.tail(experiment.setup.ng() - experiment.setup.nd()).index_min()
//                    + experiment.setup.nd();
//                    this->complying_with_preference = false;
//                }
//            }
//            break;
//
//        case DecisionPreference::MarjansHacker: {
//
//            /// Selecting a RandomSigPvalue,
//            /// if not exist, selecting a positive effect,
//            /// if not exist, selecting a min_pvalue
//
//
//            arma::uvec sig_indexes = arma::shuffle(arma::find(experiment.sigs.tail(experiment.setup.ng() - experiment.setup.nd()) == 1)) + experiment.setup.nd();
//
//            if (not sig_indexes.is_empty()) {
//                selectedOutcome = sig_indexes.at(0);
//            }else{
//                /// Returning min p-value if I couldn't find any significant p-value
//
//                arma::uvec pos_effects = arma::find(experiment.effects.tail(experiment.setup.ng() - experiment.setup.nd()) > 0.);
//
//                if (not pos_effects.is_empty()){
//                    selectedOutcome = pos_effects.at(0) + experiment.setup.nd();
//                }else{
//                    selectedOutcome = experiment.pvalues.tail(experiment.setup.ng() - experiment.setup.nd()).index_min()
//                    + experiment.setup.nd();
//                }
//
//            }
//        }
//            break;
//
//        case DecisionPreference::RevisedMarjanHacker: {
//            /// Marjan:
//            ///     Select first significant study without QRPs with a positive ES if non is significant, select best study.
//            ///     The best study is the study with max p-value between those with min negative effect
//
//            arma::uvec pos_effects_inx = arma::find(experiment.effects.tail(experiment.setup.ng() - experiment.setup.nd()) > 0.) + experiment.setup.nd();
//
//            if (not pos_effects_inx.is_empty()) {
//                arma::uvec sig_inx = arma::find(experiment.sigs.elem(pos_effects_inx) == 1);
//
//                if (not sig_inx.is_empty()) {
//                    selectedOutcome = pos_effects_inx.at(sig_inx.at(0));
//                }else{
//                    selectedOutcome = pos_effects_inx.at(experiment.pvalues.elem(pos_effects_inx).index_min());
//                }
//
//            }else{
//                arma::uvec neg_effects_inx = arma::find(experiment.effects.tail(experiment.setup.ng() - experiment.setup.nd()) < 0.)  + experiment.setup.nd();
//
//                if (not neg_effects_inx.is_empty()){
//                    selectedOutcome = neg_effects_inx.at(experiment.pvalues.elem(neg_effects_inx).index_max());
//                }
//
//            }
//
//        }
//            break;
    }
    
    return {experiment, selectedOutcome};
}


/// This is often is being used by PatientDecisionMaker
Submission DecisionStrategy::selectBetweenSubmissions(const DecisionPreference &preference){
    
    switch (preference) {
        case DecisionPreference::PreRegisteredOutcome: {
            this->complying_with_preference = true;
        }
            break;
            
        case DecisionPreference::MinSigPvalue: {

            arma::uword inx;
            arma::vec pvalues(submissions_pool.size());
            pvalues.imbue([&, i = 0]() mutable { return submissions_pool[i++].pvalue; });
            
            arma::vec sigs(submissions_pool.size());
            sigs.imbue([&, i = 0]() mutable { return submissions_pool[i++].sig; });
            
            arma::uvec sig_inxs = arma::find(sigs == 1);
            
            if (not sig_inxs.is_empty()){
                arma::uword min_sig_pvalue_inx = pvalues(sig_inxs).index_min();
                inx = min_sig_pvalue_inx; // + (submissions_pool.size() - sig_inxs.n_elem + 1);
                
                this->complying_with_preference = true;
                return submissions_pool[inx];
            }else{
                /// If there is no sig
                this->complying_with_preference = false;
                return submissions_pool.back();
            }
            
        }
            break;
            
        case DecisionPreference::MaxSigPvalue: {
             arma::uword inx;
             arma::vec pvalues(submissions_pool.size());
             pvalues.imbue([&, i = 0]() mutable { return submissions_pool[i++].pvalue; });
             
             arma::vec sigs(submissions_pool.size());
             sigs.imbue([&, i = 0]() mutable { return submissions_pool[i++].sig; });
             
             arma::uvec sig_inxs = arma::find(sigs == 1);
             
             if (not sig_inxs.is_empty()){
                 arma::uword max_sig_pvalue_inx = pvalues(sig_inxs).index_max();
                 inx = max_sig_pvalue_inx; // + (submissions_pool.size() - sig_inxs.n_elem + 1);
                 
                 this->complying_with_preference = true;
                 return submissions_pool[inx];
             }else{
                 /// If there is no sig
                 this->complying_with_preference = false;
                 return submissions_pool.back();
             }
        }
            break;
            
        case DecisionPreference::RandomSigPvalue: {
            
            arma::uword inx;
            arma::vec pvalues(submissions_pool.size());
            pvalues.imbue([&, i = 0]() mutable { return submissions_pool[i++].pvalue; });
            
            arma::vec sigs(submissions_pool.size());
            sigs.imbue([&, i = 0]() mutable { return submissions_pool[i++].sig; });
            
            arma::uvec sig_inxs = arma::shuffle(arma::find(sigs == 1));
            
            if (not sig_inxs.is_empty()){
                /// It's already shuffled, so I can just take the first one
                inx = sig_inxs.at(0);
                
                this->complying_with_preference = true;
                return submissions_pool[inx];
            }else{
                /// If there is no sig
                this->complying_with_preference = false;
                return submissions_pool.back();
            }
            
        }
            break;
            
        case DecisionPreference::MinPvalue:
            {
                // TODO: This should be a routine in Experiment
                std::vector<double> pvalues;
                std::transform(submissions_pool.begin(), submissions_pool.end(), std::back_inserter(pvalues), [](const Submission &s) {return s.pvalue;} );
                int min_pvalue_inx = std::distance(pvalues.begin(),
                                                   std::min_element(pvalues.begin(),
                                                                    pvalues.end()));
                
                this->complying_with_preference = true;
                return submissions_pool[min_pvalue_inx];
            }
            break;
            
        case DecisionPreference::MaxSigEffect:
            
            break;
            
        case DecisionPreference::MaxEffect:
            
            break;
            
        case DecisionPreference::MinPvalueMaxEffect:
            
            break;
            
            
        case DecisionPreference::MarjansHacker: {
            /// I'm not convinced that this is very representative of what Marjan's does
            
            /// In fact, Marjan doesn't do any sort of patient
            
            arma::vec pvalues(submissions_pool.size());
            pvalues.imbue([&, i = 0]() mutable { return submissions_pool[i++].pvalue; });
            
            arma::vec sigs(submissions_pool.size());
            sigs.imbue([&, i = 0]() mutable { return submissions_pool[i++].sig; });
            
            arma::uvec sig_inxs = arma::find(sigs == 1);
            
            if (not sig_inxs.is_empty()) {
                /// Returning the first sig found
                return submissions_pool[sig_inxs.at(0)];
            }else{
                arma::vec effects(submissions_pool.size());
                effects.imbue([&, i = 0]() mutable { return submissions_pool[i++].effect; });
                arma::uvec pos_effects = arma::find(effects > 0);
                if (not pos_effects.is_empty()){
                    return submissions_pool[pos_effects.at(0)];
                }else{
                    return submissions_pool[pvalues.index_min()];
                }
            }
            
        }
            
            break;
            
        case DecisionPreference::RevisedMarjanHacker: {
            arma::vec pvalues(submissions_pool.size());
            pvalues.imbue([&, i = 0]() mutable { return submissions_pool[i++].pvalue; });
            
            arma::vec sigs(submissions_pool.size());
            sigs.imbue([&, i = 0]() mutable { return submissions_pool[i++].sig; });
            
            arma::vec effects(submissions_pool.size());
            effects.imbue([&, i = 0]() mutable { return submissions_pool[i++].effect; });
            
            arma::uvec pos_effects_inx = arma::find(effects > 0);
            
            if (not pos_effects_inx.empty()) {
                arma::uvec sig_inx = arma::find(sigs.elem(pos_effects_inx));
                
                if(not sig_inx.empty()){
                    return submissions_pool[pos_effects_inx.at(sig_inx.at(0))];
                }else{
                    return submissions_pool[pos_effects_inx.at(pvalues.elem(pos_effects_inx).index_min())];
                }
            }else{
                arma::uvec neg_effects_inx = arma::find(effects < 0);
                
                if (not neg_effects_inx.empty()){
                    return submissions_pool[neg_effects_inx.at(pvalues.elem(neg_effects_inx).index_max())];
                }
            }
        }
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
    
    is_still_hacking = !isSubmittable();
}

/// Impatient decision maker check if the check current submission is publishable
/// or not, if not, it'll continue hacking... and will stop as soon as it
/// finds a publishable solution. This is different in the case of patient
/// decision maker for instance.
void ImpatientDecisionMaker::intermediateDecision(Experiment &experiment) {

    is_still_hacking = !isSubmittable();
}


void ImpatientDecisionMaker::afterhackDecision(Experiment &experiment) {
    
    
    if (isSubmittable()){
        experiments_pool.push_back(experiment);
        submissions_pool.push_back(current_submission);
    }

    is_still_hacking = !isSubmittable();
    
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

    is_still_hacking = !isSubmittable();
}


/// Patient decision maker keeps track of its intermediate results until it
/// makes the final decision and choose between them.
///
/// @param experiment A reference to the experiment
void PatientDecisionMaker::afterhackDecision(Experiment &experiment) {

    if (isSubmittable()){
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

