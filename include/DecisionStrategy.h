//
// Created by Amir Masoud Abdol on 2019-02-01.
//

#ifndef SAMPP_DECISIONSTRATEGY_H
#define SAMPP_DECISIONSTRATEGY_H

#include <vector>
#include <memory>

#include "sam.h"

#include "Submission.h"
#include "Experiment.h"
#include "Utilities.h"

namespace sam {

    enum class DecisionMethod {
        HonestDecisionMaker,
        PatientDecisionMaker,
        ImpatientDecisionMaker
    };


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

    /**
     An enum class representing different prefeneces when it comse
     to selecting an outcome. The DecisionStrategy can choose to
     report Pre-registered outcome, or any other outcome based on
     certain criteria, e.g., MinPvalue, where `researcher->decisionStrategy`
     will prefer an outcome with the lowest p-value.
     */
    enum class DecisionPreference {
        PreRegisteredOutcome = 0,
        MinSigPvalue,
        MinPvalue,
        MaxSigEffect,
        MaxEffect,
        MinPvalueMaxEffect
    };

    /**
     @brief Abstract class for different decision strategies.
     
     */
    class DecisionStrategy {
        
    protected:
        
        //! List of selected Submission by the researcher.
        std::vector<Submission> submissions_pool;

        //! List of selected Experiment by the researcher.
        std::vector<Experiment> experiments_pool;

        //! If `true`, the Researcher will continue traversing through the
        //! hacknig methods, otherwise, he/she will stop the hacking and
        //! prepare the finalSubmission. It will be updated on each call of
        //! verdict(). Basically verdict() decides if the Researcher is
        //! happy with the submission record or not.
        bool is_still_hacking = true;
        
        bool will_be_submitting = false;
        
        
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
        
        static std::unique_ptr<DecisionStrategy> build(json &decision_strategy_config);
        
        virtual ~DecisionStrategy() = 0;
        
        //! Indicates researcher's selection preference on how he choose the
        //! outcome variable for submission.
        DecisionPreference selectionPref;
        
        /*
         * The default get method for is_still_hacking
         */
        virtual bool isStillHacking() {
            return is_still_hacking;
        }
        
        virtual bool isPublishable() {
            return current_submission.isSig();
        }
        
        /**
         This can be used by the researcher to decide if he is going with the
         submission or not.
         */
        virtual bool willBeSubmitting() {
            return will_be_submitting;
        }
        
        //! Indicates the pre-registered outcome in the case where the
        //! Researcher prefers the PreRegisteredOutcome
        int pre_registered_group = 0;
        
        
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

        
        void saveCurrentSubmission();
        
        /**
         * @brief      Based on the DecisionPreference, it'll select the outcome
         * between all groups, `ng`. For instance, the MinPvalue deicison prefenrece will
         *
         * @param      experiment
         *
         * @return     A copy of the selected outcome
         */
        Submission selectOutcome(Experiment &experiment);

        /**
         * @brief      Select the final submission by checking all logged Submissions.
         *
         * @return     A copy of the selected outcome
         */
        Submission selectBetweenSubmissions();
        
//        Experiment selectBetweenExperiments();
    };

    /**
     @brief Implementation of an impatient researcher. In this case, the Researcher will stop as soon as find a significant result and will not continue exploring other hacking methods in his arsenal.
     */
    class ImpatientDecisionMaker : public DecisionStrategy {
        
    public:
        
        struct Parameters {
            DecisionMethod name = DecisionMethod::ImpatientDecisionMaker;
            DecisionPreference preference = DecisionPreference::MinPvalue;
        };
        
        Parameters params;
        
        explicit ImpatientDecisionMaker(const Parameters &p) : params{p} {
            // TODO: I totally need to refactor this! I'm using the abstract class
            // paramters and it's confusing!
            selectionPref = params.preference;
        };
        
//        explicit ImpatientDecisionMaker(DecisionPreference selection_pref) {
//            selectionPref = selection_pref;
//        };
        
        bool isStillHacking() override {
            return is_still_hacking;
        }
//
//        bool isPublishable() override {
//            return current_submission.isSig();
//        }
        
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
                {"name", magic_enum::enum_name<DecisionMethod>(p.name)},
                {"preference", magic_enum::enum_name<DecisionPreference>(p.preference)}
            };
        }
    
        inline
        void from_json(const json& j, ImpatientDecisionMaker::Parameters& p) {
            
            // Using a helper template function to handle the optional and throw if necessary.
            p.name = get_enum_value_from_json<DecisionMethod>("name", j);
            
            p.preference = get_enum_value_from_json<DecisionPreference>("preference", j);
        }


//    class PatientDecisionMaker : public DecisionStrategy {
//
//    public:
//
//        struct Parameters {
//            DecisionMethod name = DecisionMethod::ImpatientDecisionMaker;
//            DecisionPreference preference = DecisionPreference::MinPvalue;
//        };
//
//        Parameters params;
//
//        explicit PatientDecisionMaker(const Parameters &p) : params{p} {};
//
//        explicit PatientDecisionMaker(DecisionPreference selection_pref) {
//            selectionPref = selection_pref;
//        };
//
//        bool isPublishable(const Submission &sub) const {
//            return sub.isSig();
//        };
//
//        virtual bool verdict(Experiment &experiment, DecisionStage stage);
//
//        virtual bool initDecision(Experiment &experiment);
//        virtual bool intermediateDecision(Experiment &experiment);
//        virtual bool afterhackDecision(Experiment &experiment);
//        virtual bool finalDecision(Experiment &experiment);
//
//    };
//
//    // JSON Parser for PatientDecisionStrategy::Parameters
//    inline
//    void to_json(json& j, const PatientDecisionMaker::Parameters& p) {
//        j = json{
//            {"name", magic_enum::enum_name<DecisionMethod>(p.name)},
//            {"preference", magic_enum::enum_name<DecisionPreference>(p.preference)}
//        };
//    }
//
//    inline
//    void from_json(const json& j, PatientDecisionMaker::Parameters& p) {
//
//        // Using a helper template function to handle the optional and throw if necessary.
//        p.name = get_enum_value_from_json<DecisionMethod>("name", j);
//
//        p.preference = get_enum_value_from_json<DecisionPreference>("preference", j);
//    }
//
//
//
//    // FIXME: Not fully implemented!
//    // FIXME: Not fully tested!
//    class HonestDecisionMaker : public DecisionStrategy {
//
//    public:
//
//        struct Parameters {
//            DecisionMethod name = DecisionMethod::ImpatientDecisionMaker;
//        };
//
//        Parameters params;
//
//        HonestDecisionMaker(const Parameters &p) : params{p} {};
//
//        HonestDecisionMaker() {
//            selectionPref = DecisionPreference::PreRegisteredOutcome;
//        };
//
//        virtual bool verdict(Experiment &experiment, DecisionStage stage) {
//            return false;
//        };
//
//        virtual bool initDecision(Experiment &experiment) {return false;};
//        virtual bool intermediateDecision(Experiment &experiment) {return false;};
//        virtual bool afterhackDecision(Experiment &experiment) {return false;};
//        virtual bool finalDecision(Experiment &experiment) {return false;};
//
//    };

}
    
#endif //SAMPP_DECISIONSTRATEGY_H
