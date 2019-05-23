//
// Created by Amir Masoud Abdol on 2019-01-25.
//

#ifndef SAMPP_HACKINGSTRATEGIES_H
#define SAMPP_HACKINGSTRATEGIES_H

#include "Experiment.h"
#include "Submission.h"
#include "DecisionStrategy.h"
#include <string>
#include <map>

#include "nlohmann/json.hpp"

namespace sam {

    using json = nlohmann::json;

    enum class HackingMethod {
        OptionalStopping = 0,
        SDOutlierRemoval,
        GroupPooling,
        ConditionDropping,
        NoHack
    };

    /*
     HackingStage indicates the stage where the hacking is being performed on
     the Experiment. Each method will be assigned a value, and Researcher can
     apply different hacking methods in different stages.
     */
    enum class HackingStage {
        Setup,
        DataCollection,
        DataProcessing,
        Reporting
    };

    /**
     @brief Abstract class for hacking strategies.
     
     Each HackingStrategy should provide a `perform()` method. The `perform()` method
     will take over a pointer to an Experiment and apply the implemented hacking on it.
     Researcher decides if this is a pointer to a *fresh* copy of the Experiment or a pointer
     to a previously "hacked" Experiment.

     */
    class HackingStrategy {
        
    public:

        struct HackingStrategyParameters {
            HackingMethod name;

            HackingStage stage = HackingStage::DataProcessing;

            //! Defensibility of the method
            //! This is a based on the survey results where researchers have been
            //! asked to rate the defensibility of different QRPs.
            //! Defensitbility of 0 indicates that the method is completely
            //! frown upon and shouldn't be used, while defensibility of 1.
            //! means it's a valid pratice and it's not going to be preseved
            //! as a hacking method.
            double defensibility = 1.;
        };

        //! Hacking Strategy parameters.
        HackingStrategyParameters params;
        
        /**
         * @brief      Factory method for building a HackingStrategy
         *
         * @param      config  A reference to an item of the `json['--hacking-strategy']`.
         *                     Researcher::Builder is responsible for passing this object
         *                     correctly.
         *
         * @return     A new HackingStrategy
         */
        static HackingStrategy* build(json& hacking_strategy_config);
        
        static HackingStrategy* build(HackingMethod method);

        static HackingStrategy* build(HackingStrategyParameters &hsp);
        

        /**
         * @brief      Pure deconstuctor of the Base calss. This is important
         * for proper deconstruction of Derived classes.
         */
        virtual ~HackingStrategy() = 0;

        /**
         * @brief      Applies the hacking method on the Experiment.
         *
         * @param      experiment        A pointer to an Experiment.
         *
         * @param      decisionStrategy  A pointer to Researcher's DecisionStrategy.
         *                               The HackingStrategy decides with what flag it
         *                               is going to use the DecisionStrategy.
         */
        virtual void perform(Experiment* experiment, DecisionStrategy* decisionStrategy) = 0;
    };
        
        
    class NoHack : public HackingStrategy {
    public:

        NoHack() {
            params.name = HackingMethod::NoHack;
        };
        
        void perform(Experiment *experiment, DecisionStrategy *decisionStrategy) { };
        
    };


    /**
     @brief Declartion of OptionalStopping hacking strategy
     */
    class OptionalStopping : public HackingStrategy {
        
    public:

        OptionalStopping(std::string level = "dv", int num = 3, int n_attempts = 1, int max_attempts = 10) :
            level(level),
            num(num),
            n_attempts(n_attempts),
            max_attempts(max_attempts)
        {
            params.name = HackingMethod::OptionalStopping;
        };

        void perform(Experiment *experiment, DecisionStrategy *decisionStrategy);
        
        
        /**
         Randmoize the parameters of the Optional Stopping
         */
        void randomize(int min_n, int max_n);
        
    private:
        
        std::string level = "dv";
        
        //! Number of new observations to be added to each group
        int num = 3;
        
        //! Number of times that Researcher add `num` observations to each group
        int n_attempts = 1;
        
        //! Maximum number of times that Researcher tries to add new observations to
        //! each group
        int max_attempts = 10;
        
        
        /**
         Add _n_ observations to all groups and return the updated experiment to
         the `perform()` method.

         @param experiment A pointer to the experiment
         @param n number of new observations to be added
         */
        void addObservations(Experiment *experiment, const int &n);

    };

    /**
     @brief Declaration of Outlier Removal hacking method based on items' distance from their
     sample mean.
     
     */
    class SDOutlierRemoval : public HackingStrategy {
    public:
        
        SDOutlierRemoval(std::string level = "dv", std::string order = "max first", int num = 3, int n_attempts = 1, int max_attempts = 3, int min_observations = 10, std::vector<double> multipliers = {3}) :
            level(level),
            order(order),
            num(num),
            n_attempts(n_attempts),
            max_attempts(max_attempts),
            min_observations(min_observations),
            multipliers(multipliers)
        {
            params.name = HackingMethod::SDOutlierRemoval;
        };
        

        // Submission hackedSubmission;
        void perform(Experiment* experiment, DecisionStrategy* decisionStrategy);
        
        
        
    private:
        
        std::string level = "dv";
        std::string order = "max first";
        int num = 3;
        int n_attempts = 1;
        int max_attempts = 10;
        int min_observations = 15;
        std::vector<double> multipliers = {3};
        
        int removeOutliers(Experiment *experiment, const int &n, const int &d);
        
    };

    class GroupPooling : public HackingStrategy {
        
    public:
        
        explicit GroupPooling(std::vector<int> nums = {2}) : nums(nums)
        {
            params.name = HackingMethod::GroupPooling;
        };
        
        // Submission hackedSubmission;
        void perform(Experiment* experiment, DecisionStrategy* decisionStrategy);
        
    private:

        std::vector<int> nums = {2};
        
        void pool(Experiment* experiment, int r);
    };
        
        
    class ConditionDropping : public HackingStrategy {

    public:
        ConditionDropping() {
            params.name = HackingMethod::ConditionDropping;
        };
        
        void perform(Experiment* experiment, DecisionStrategy* decisionStrategy) { };
    };

    //class QuestionableRounding : public HackingStrategy {
    //
    //public:
    //    QuestionableRounding(int threshold) : _threshold(threshold) {};
    //
    //private:
    //    int _threshold;
    //};
    
}
    
#endif //SAMPP_HACKINGSTRATEGIES_H
