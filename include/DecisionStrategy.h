//
// Created by Amir Masoud Abdol on 2019-02-01.
//

/**
* \defgroup DecisionStrategies
* @brief List of available Decision Strategies
*
* Description to come!
*/

#ifndef SAMPP_DECISIONSTRATEGY_H
#define SAMPP_DECISIONSTRATEGY_H

#include <vector>
#include <memory>
#include <sol/sol.hpp>
#include <fmt/core.h>
#include <fmt/format.h>
#include <spdlog/spdlog.h>
#include <nlohmann/json.hpp>

#include "sam.h"

#include "Submission.h"
#include "Experiment.h"
#include "Utilities.h"
#include "GroupData.h"




using json = nlohmann::json;

namespace sam {

    enum class DecisionMethod {
        HonestDecisionMaker,
        PatientDecisionMaker,
        ImpatientDecisionMaker,
        NoDecision
    };

    NLOHMANN_JSON_SERIALIZE_ENUM( DecisionMethod, {
         {DecisionMethod::HonestDecisionMaker, "HonestDecisionMaker"},
         {DecisionMethod::PatientDecisionMaker, "PatientDecisionMaker"},
         {DecisionMethod::ImpatientDecisionMaker, "ImpatientDecisionMaker"},
         {DecisionMethod::NoDecision, "NoDecision"}
    })

    /**
     DecisionStage enum indicates on what stages of the _research_ the
     Researcher is making decision in.
     
     */
    enum class DecisionStage {
        Initial,
        WhileHacking,
        DoneHacking,
        Final
    };

    NLOHMANN_JSON_SERIALIZE_ENUM( DecisionStage, {
        {DecisionStage::Initial, "Initial"},
        {DecisionStage::WhileHacking, "WhileHacking"},
        {DecisionStage::DoneHacking, "DoneHacking"},
        {DecisionStage::Final, "Final"}
    })


    /// @brief An enum class representing different prefeneces.
    /// @ingroup DecisionStrategies
    ///
    /// When it comes to selecting an outcome, the Researcher, i.e., DecisionStrategy,
    /// can choose to report the pre-registered outcome, or any other outcome based on
    /// certain criteria/preference, e.g., MinPvalue, where `researcher->decisionStrategy`
    /// will prefer an outcome with the lowest p-value.
    enum class DecisionPreference {
        PreRegisteredOutcome,
        MinSigPvalue,
        MinPvalue,
        MaxSigEffect,
        MaxEffect,
        MinPvalueMaxEffect,
    
        RandomSigPvalue, ///< Specifies a preference in which the researcher reports a random significant outcome.
                         ///< @note If no significant result found, the outcome with the smallest p-value will be selected.
        
        MaxSigPvalue,
        
        MarjansHacker,
        RevisedMarjanHacker,
        
        Policies
    };


    NLOHMANN_JSON_SERIALIZE_ENUM( DecisionPreference, {
        {DecisionPreference::PreRegisteredOutcome, "PreRegisteredOutcome"},
        {DecisionPreference::MinSigPvalue, "MinSigPvalue"},
        {DecisionPreference::MinPvalue, "MinPvalue"},
        {DecisionPreference::MaxSigEffect, "MaxSigEffect"},
        {DecisionPreference::MaxEffect, "MaxEffect"},
        {DecisionPreference::MinPvalueMaxEffect, "MinPvalueMaxEffect"},
        {DecisionPreference::RandomSigPvalue, "RandomSigPvalue"},
        {DecisionPreference::MaxSigPvalue, "MaxSigPvalue"},
        {DecisionPreference::MarjansHacker, "MarjansHacker"},
        {DecisionPreference::RevisedMarjanHacker, "RevisedMarjanHacker"},
        {DecisionPreference::Policies, "Policies"}
    })

    enum class PolicyType {
        Min,
        Max,
        Comp,
        Random
    };

    /**
     @brief Abstract class for different decision strategies.
     
     */
    class DecisionStrategy {
        
    protected:

        json config_;
        
        //! Indicates the pre-registered outcome in the case where the
        //! Researcher prefers the PreRegisteredOutcome
        int pre_registered_group;
        
        //! List of selected Submission by the researcher.
        std::vector<Submission> submissions_pool;

        //! List of selected Experiment by the researcher.
        std::vector<Experiment> experiments_pool;
        
        std::vector<PolicyType> policies_type;
        std::vector<sol::function> policies_func;

        //! If `true`, the Researcher will continue traversing through the
        //! hacknig methods, otherwise, he/she will stop the hacking and
        //! prepare the finalSubmission. It will be updated on each call of
        //! verdict(). Basically verdict() decides if the Researcher is
        //! happy with the submission record or not.
        bool is_still_hacking = true;
        
        bool will_be_submitting = false;
        
        bool complying_with_preference = true;
        
        /**
         A method implementing the Initial decision making logic.
         
         @return A boolean indicating whether the researcher should proceed
         with the hacking or not.
         */
        virtual void initDecision(Experiment &experiment) = 0;
        
        /**
         A method implementing the Intermediate decision making logic.
         
         @note Decision Strategy at this stage can decide if it wants to add
         an intermediate solution to the pool or not. This is often a solution
         halfway through the hacking process for instance.
         
         @return A boolean indicating whether the researcher should stop the
         hacking procedude or not. E.g., in the case of Optional Stopping this
         routine will decide whether researcher should stop after each attempt
         or continue further.
         */
        virtual void intermediateDecision(Experiment &experiment) = 0;
        
        /**
         A method implementing the decision making routine after completely
         applying a hacking strategy on an experiment.
         
         @return A boolean indicating whether the researcher should proceed to
         the _next_ hacking strategy.
         */
        virtual void afterhackDecision(Experiment &experiment) = 0;
        
        /**
         A method implementing the Final decision making logic.
         
         @note In the case where the Decision Making should look back at the
         history of hacking, this routine will also prepare the `final_submission`.
         
         @return A boolean indicating whether the researcher should proceed
         with submitting the final submission to the Journal or not.
         */
        virtual void finalDecision(Experiment &experiment) = 0;
        
    public:
        
        DecisionMethod name;
        
        sol::state lua;
        
        /**
         DecisionStrategy factory method.
         
         @param decision_strategy_config    A JSON object containing information
         about each decision strategy.
         */
        static std::unique_ptr<DecisionStrategy> build(json &decision_strategy_config);
        
        virtual ~DecisionStrategy() = 0;
        
        
        /// The logic of continoution should be implemented here. Researcher will
        /// ask this method to asses the state of its progress.
        virtual bool isStillHacking() {
            return is_still_hacking;
        }
        
        
        /// It indicates whether the current_submission is significant or not.
        /// This can be overwritten to adopt to different type of studies or
        /// logics.
//        virtual bool isSubmittable() {
//            return current_submission.isSig();
//        }
        
        /**
         It indicates whether the researcher is going to commit to submitting the
         _Submission_. This acts as an another level of decision making where the
         researcher consider another criteria if it couldn't achieve what he was
         "looking for".
         
         For instance, if the researcher is determined to find "MinSigPvalue" during
         his research, and — after all the hacking — he couldn't find anything significant, then he decide whether he wants to submit the "unpreferable"
         result or not.
         */
        virtual bool willBeSubmitting();
        
        DecisionStage current_stage;
        
        Submission current_submission;
        
        //! This will set to the final submission record that the Researcher
        //! is satisfied with.
        Submission final_submission;
        
        
        /**
         Clear the list of submissions and experiments
         */
        void clearHistory(){
            submissions_pool.clear();
            experiments_pool.clear();
        }
        
        /**
         * @brief      Implementation of decision-making procedure.
         *
         * @param      experiment
         * @param[in]  stage       The stage in which the researcher is asking
         *                         for the verdict. The implementation of verdict
         *                         sould provide different procedure for different
         *                         stages of the development.
         *
         * @return     A boolean indicating whether result is satisfactory or not
         */
        virtual DecisionStrategy& verdict(Experiment& experiment, DecisionStage stage) = 0;

        
        /// A helper method to save the current submission. This needs to be called
        /// after verdict.
        void saveCurrentSubmission() {
            submissions_pool.push_back(current_submission);
        };
        
        /**
         * @brief      Based on the DecisionPreference, it'll select the outcome
         * between all groups, `ng`. For instance, the MinPvalue deicison prefenrece will
         *
         * @param      experiment
         *
         * @return     A copy of the selected outcome
         */
        Submission selectOutcome(Experiment &experiment, const DecisionPreference &preference);

        /**
         * @brief      Select the final submission by checking all logged Submissions.
         * Each submission is from a hacked experiment by the researcher. This is often being
         * used by the `PatientDecisionMaker` at the end of the hacking procedure.
         *
         * @return     A copy of the selected outcome
         */
        Submission selectBetweenSubmissions(const DecisionPreference &preference);
        
//        Experiment selectBetweenExperiments();
    };

    /**
     @ingroup DecisionStrategies
     @brief Implementation of an impatient researcher. In this case, the Researcher will stop as soon as find a significant result and will not continue exploring other hacking methods in his arsenal.
     
     
     */
    class ImpatientDecisionMaker : public DecisionStrategy {
        
    public:
        
        struct Parameters {
            DecisionMethod name = DecisionMethod::ImpatientDecisionMaker;
            DecisionPreference preference;
            std::vector<std::string> decision_policies;
            
            std::vector<std::string> submission_policies;
        };
        
        Parameters params;
        
        explicit ImpatientDecisionMaker(const Parameters &p) {
            
            
            lua.open_libraries();
            
            lua.new_usertype<GroupData>("GroupData",
                                        "nobs", &GroupData::nobs_,
                                        "pvalue", &GroupData::pvalue_,
                                        "effect", &GroupData::effect_,
                                        "sig", &GroupData::sig_
                                        );
            
            lua.new_usertype<Submission>("Submission",
                                         "mean", &Submission::mean,
                                         "pvalue", &Submission::pvalue,
                                         "effect", &Submission::effect,
                                         "sig", &Submission::sig
                                         );
     
            
            spdlog::debug("Registering decision policies...");
            for (auto &s : p.decision_policies) {
                
                std::cout << s << std::endl;
                
                auto policy = make_function(s, lua);
                
                decision_policies_type.push_back(policy.first);
                decision_policies_func.push_back(policy.second);
            }
            
            for (auto &s : p.submission_policies) {
                std::cout << s << std::endl;
                
                auto policy = make_function(s, lua);
                
                submission_policies_type.push_back(policy.first);
                submission_policies_func.push_back(policy.second);
            }
        }
        
        bool isStillHacking() override {
            return is_still_hacking;
        }
        
        virtual ImpatientDecisionMaker& verdict(Experiment &experiment, DecisionStage stage) override;
        
        virtual void initDecision(Experiment &experiment) override;
        virtual void intermediateDecision(Experiment &experiment) override;
        virtual void afterhackDecision(Experiment &experiment) override;
        virtual void finalDecision(Experiment &experiment) override;
        
    };
    
    // JSON Parser for ImpatientDecisionStrategy::Parameters
        inline
        void to_json(json& j, const ImpatientDecisionMaker::Parameters& p) {
            j = json{
                {"_name", p.name},
                {"preference", p.preference},
                {"decision_policies", p.decision_policies},
                {"submission_policies", p.submission_policies},
            };
        }
    
        inline
        void from_json(const json& j, ImpatientDecisionMaker::Parameters& p) {
            j.at("_name").get_to(p.name);
            j.at("preference").get_to(p.preference);
            j.at("decision_policies").get_to(p.decision_policies);
            j.at("submission_policies").get_to(p.submission_policies);
        }


    /**
     @ingroup DecisionStrategies
     @brief
     */
    class PatientDecisionMaker : public DecisionStrategy {

    public:

        struct Parameters {
            DecisionMethod name = DecisionMethod::PatientDecisionMaker;
            DecisionPreference preference;
        };

        Parameters params;

        explicit PatientDecisionMaker(const Parameters &p) : params{p} {
        };

        virtual PatientDecisionMaker& verdict(Experiment &experiment, DecisionStage stage) override;

        virtual void  initDecision(Experiment &experiment) override;
        virtual void  intermediateDecision(Experiment &experiment) override;
        virtual void  afterhackDecision(Experiment &experiment) override;
        virtual void  finalDecision(Experiment &experiment) override;

    };

    // JSON Parser for PatientDecisionStrategy::Parameters
    inline
    void to_json(json& j, const PatientDecisionMaker::Parameters& p) {
        j = json{
            {"_name", p.name},
            {"preference", p.preference},
        };
    }

    inline
    void from_json(const json& j, PatientDecisionMaker::Parameters& p) {

        j.at("_name").get_to(p.name);
        j.at("preference").get_to(p.preference);
    }

    /**
     @ingroup DecisionStrategies
     @brief
     */
    class HonestDecisionMaker : public DecisionStrategy {

    public:

        struct Parameters {
            DecisionMethod name = DecisionMethod::HonestDecisionMaker;
            DecisionPreference preference = DecisionPreference::PreRegisteredOutcome;
        };

        /// Parameters of the HonestDecisionMaker are fixed, he is basically the
        /// baseline researcher that report everything he finds.
        Parameters params;
        
        HonestDecisionMaker() {};

        HonestDecisionMaker(const Parameters p) : params(p)  {
        };
        
        bool isStillHacking() override {
            return false;
        };

        virtual HonestDecisionMaker& verdict(Experiment &experiment, DecisionStage stage) override;

        virtual void initDecision(Experiment &experiment) override {};
        virtual void intermediateDecision(Experiment &experiment) override {};
        virtual void afterhackDecision(Experiment &experiment) override {};
        virtual void finalDecision(Experiment &experiment) override {};

    };

    /**
     @ingroup DecisionStrategies
     @brief A placeholder for empty decision strategy.
     
     @todo This should not be allowed actually. Some refactoring and cleanup is needed.
     */
    class NoDecision : public DecisionStrategy {
        
    public:
        
        struct Parameters {
            DecisionMethod name = DecisionMethod::NoDecision;
        };
        
        Parameters params;
        
        NoDecision() { };
        
        bool isStillHacking() override { return true; };
        
        virtual NoDecision& verdict(Experiment &experiment, DecisionStage stage) override { return *this; };
        
        virtual void initDecision(Experiment &experiment) override {};
        virtual void intermediateDecision(Experiment &experiment) override {};
        virtual void afterhackDecision(Experiment &experiment) override {};
        virtual void finalDecision(Experiment &experiment) override {};
    };

}
    
#endif //SAMPP_DECISIONSTRATEGY_H
