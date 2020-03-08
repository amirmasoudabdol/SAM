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


    enum class SubmissionPolicy {
        Anything,
        AnythingSig,
        ComplyingWithPreference,
        PreRegSig
    };


    NLOHMANN_JSON_SERIALIZE_ENUM( SubmissionPolicy, {
        {SubmissionPolicy::Anything, "Anything"},
        {SubmissionPolicy::AnythingSig, "AnythingSig"},
        {SubmissionPolicy::ComplyingWithPreference, "BasedOnThePreference"},
        {SubmissionPolicy::PreRegSig, "PreRegSig"}
    })


    enum class PolicyType {
        Min,
        Max,
        Comp
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
        
        SubmissionPolicy submission_policy;
        
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
        virtual bool isSubmittable() {
            return current_submission.isSig();
        }
        
        /**
         It indicates whether the researcher is going to commit to submitting the
         _Submission_. This acts as an another level of decision making where the
         researcher consider another criteria if it couldn't achieve what he was
         "looking for".
         
         For instance, if the researcher is determined to find "MinSigPvalue" during
         his research, and — after all the hacking — he couldn't find anything significant, then he decide whether he wants to submit the "unpreferable"
         result or not.
         */
        virtual bool willBeSubmitting() {
            
            switch (submission_policy) {
                case SubmissionPolicy::Anything: {
                        return true;
                    }
                    break;
                case SubmissionPolicy::AnythingSig: {
                    if (this->isSubmittable())
                        return true;
                    else
                        return false;
                    }
                    break;
                case SubmissionPolicy::ComplyingWithPreference: {
                    if (this->complying_with_preference)
                        return true;
                    else
                        return false;
                    }
                    break;
                case SubmissionPolicy::PreRegSig: {
                    if (current_submission.inx != pre_registered_group)
                        return false;
                    else if (isSubmittable()){
                        return true;
                    }
                }
                    break;
            }
            
            return will_be_submitting;
        }
        
        
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
            DecisionPreference preference = DecisionPreference::MinPvalue;
            std::vector<std::string> decision_policies;
            SubmissionPolicy publishing_policy = SubmissionPolicy::Anything;
        };
        
        Parameters params;
        
        explicit ImpatientDecisionMaker(const Parameters &p) : params{p} {
            submission_policy = p.publishing_policy;
            
            std::map<std::string, std::string> lua_temp_scripts {
                {"min_script", R"(
                    function {} (l, r)
                        return l.{} < r.{}
                    end
                    )"},

                {"max_script", R"(
                    function {} (l, r)
                        return l.{} > r.{}
                    end
                    )"},

                {"comp_script", R"(
                    function {} (d)
                        return d.{}
                    end
                    )"}
            };
            
            lua.open_libraries();
            
            lua.new_usertype<GroupData>("GroupData",
                                          "nobs", &GroupData::nobs_,
                                          "pvalue", &GroupData::pvalue_,
                                          "effect", &GroupData::effect_,
                                          "sig", &GroupData::sig_
            );
            
            std::vector<std::string> comp_ops = {">=", "<=", ">", "<", "==", "!="};
            std::vector<std::string> ops_names = {"greater_eq", "lesser_eq", "greater", "lesser", "equal", "not_equal"};
            
            std::map<std::string, std::string> cops = {
                {">=", "greater_eq"},
                {"<=", "lesser_eq"},
                {">", "greater"},
                {"<", "lesser"},
                {"==", "equal"},
                {"!=", "not_equal"}
            };
            
            spdlog::debug("Registering decision policies...");
            for (auto &s : p.decision_policies) {
                
                std::cout << s << std::endl;
                
                if (s.find("min") != std::string::npos) {
                    
                    auto open_par = s.find("(");
                    auto close_par = s.find(")");
                    
                    auto var_name = s.substr(open_par+1, close_par - open_par - 1);
                    
                    std::cout << var_name << std::endl;
                    
                    auto f_name = fmt::format("min_{}", var_name);
                    auto f_def = fmt::format(lua_temp_scripts["min_script"],
                                                         f_name, var_name, var_name);
                    
                    lua.script(f_def);
                    
                    policies_type.push_back(PolicyType::Min);
                    policies_func.push_back(lua[f_name]);
                    
                    std::cout << f_def << std::endl;
                    
                }else if (s.find("max") != std::string::npos) {
                    
                    auto open_par = s.find("(");
                    auto close_par = s.find(")");
                    
                    auto var_name = s.substr(open_par+1, close_par - open_par - 1);
                    
                    std::cout << var_name << std::endl;
                    
                    auto f_name = fmt::format("max_{}", var_name);
                    auto f_def = fmt::format(lua_temp_scripts["max_script"],
                                                         f_name, var_name, var_name);
                    
                    lua.script(f_def);
                    
                    policies_type.push_back(PolicyType::Max);
                    policies_func.push_back(lua[f_name]);
                    
                    std::cout << f_def << std::endl;
                }else if (s.find("sig") != std::string::npos) {
                    
//                    auto var_name = "sig";
                    auto f_name = "cond_sig";
                    
                    auto f_def = fmt::format(lua_temp_scripts["comp_script"],
                                             f_name, "sig == 1");
                    
                    lua.script(f_def);
                    
                    policies_type.push_back(PolicyType::Comp);
                    policies_func.push_back(lua[f_name]);
                    
                    std::cout << f_def << std::endl;
                
                
                
                }else if (std::any_of(cops.begin(), cops.end(),
                                      [&s](const auto &op){ return s.find(op.first) != std::string::npos; })) {
                    // Found a comparision
                    
                    std::string s_op {};
                    for (auto &op : cops)
                        if (s.find(op.first) != std::string::npos) {
                            s_op = op.first;
                            break;
                        }
                        
                    auto op_start = s.find(s_op);
                    
                    auto var_name = s.substr(0, op_start - 1);
                    auto f_name = fmt::format("cond_{}", var_name + "_" + cops[s_op]);
                    auto f_def = fmt::format(lua_temp_scripts["comp_script"], f_name, s);    // Full text goes here
                    
                    lua.script(f_def);
                    
                    policies_type.push_back(PolicyType::Comp);
                    policies_func.push_back(lua[f_name]);
                    
                    std::cout << f_def << std::endl;
                    
                    
                }
                
            }
            
            
        };
        
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
                {"submission_policy", p.publishing_policy}
            };
        }
    
        inline
        void from_json(const json& j, ImpatientDecisionMaker::Parameters& p) {
            j.at("_name").get_to(p.name);
            j.at("preference").get_to(p.preference);
            j.at("decision_policies").get_to(p.decision_policies);
            j.at("submission_policy").get_to(p.publishing_policy);
        }


    /**
     @ingroup DecisionStrategies
     @brief
     */
    class PatientDecisionMaker : public DecisionStrategy {

    public:

        struct Parameters {
            DecisionMethod name = DecisionMethod::PatientDecisionMaker;
            DecisionPreference preference = DecisionPreference::MinPvalue;
            SubmissionPolicy publishing_policy = SubmissionPolicy::Anything;
        };

        Parameters params;

        explicit PatientDecisionMaker(const Parameters &p) : params{p} {
            submission_policy = p.publishing_policy;
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
            {"submission_policy", p.publishing_policy}
        };
    }

    inline
    void from_json(const json& j, PatientDecisionMaker::Parameters& p) {

        j.at("_name").get_to(p.name);
        j.at("preference").get_to(p.preference);
        j.at("submission_policy").get_to(p.publishing_policy);
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
            SubmissionPolicy publishing_policy = SubmissionPolicy::Anything;
        };

        /// Parameters of the HonestDecisionMaker are fixed, he is basically the
        /// baseline researcher that report everything he finds.
        Parameters params;
        
        HonestDecisionMaker() {};

        HonestDecisionMaker(const Parameters p) : params(p)  {
            submission_policy = p.publishing_policy;
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
