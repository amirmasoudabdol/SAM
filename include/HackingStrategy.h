//
// Created by Amir Masoud Abdol on 2019-01-25.
//

#ifndef SAMPP_HACKINGSTRATEGIES_H
#define SAMPP_HACKINGSTRATEGIES_H

#include <string>
#include <map>

#include "Experiment.h"
#include "Submission.h"
#include "HackingStrategyTypes.h"
#include "DecisionStrategy.h"
#include "Utilities.h"

#include "nlohmann/json.hpp"

namespace sam {

    using json = nlohmann::json;

    /**
     @brief Abstract class for hacking strategies.
     
     Each HackingStrategy should provide a `perform()` method. The `perform()` method
     will take over a pointer to an Experiment and apply the implemented hacking on it.
     Researcher decides if this is a pointer to a *fresh* copy of the Experiment or a pointer
     to a previously "hacked" Experiment.

     */
    class HackingStrategy {
        
    public:
        
        //! Hacking Strategy name.
        HackingMethod name;
        
        /**
         * @brief      Factory method for building a HackingStrategy
         *
         * @param      config  A reference to an item of the `json['--hacking-strategy']`.
         *                     Researcher::Builder is responsible for passing this object
         *                     correctly.
         *
         * @return     A new HackingStrategy
         */
        static std::unique_ptr<HackingStrategy> build(json& hacking_strategy_config);
        
        static std::unique_ptr<HackingStrategy> build(HackingMethod method);
        

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
//            params.name = HackingMethod::NoHack;
            name = HackingMethod::NoHack;
        };
        
        void perform(Experiment *experiment, DecisionStrategy *decisionStrategy) { };
        
    };


    /**
     @brief Declartion of OptionalStopping hacking strategy
     */
    class OptionalStopping : public HackingStrategy {
        
    public:
        
        struct Parameters {
            HackingMethod name = HackingMethod::OptionalStopping;
            std::string level = "dv";
            
            //! Number of new observations to be added to each group
            int num = 3;
            
            //! Number of times that Researcher add `num` observations to each group
            int n_attempts = 1;
            
            //! Maximum number of times that Researcher tries to add new observations to
            //! each group
            int max_attempts = 10;
            
            bool is_randomized = false;
        };

        Parameters params;

        OptionalStopping(const Parameters &p) : params{p} {
            name = params.name;
        } ;

        OptionalStopping(std::string level = "dv", int num = 3, int n_attempts = 1, int max_attempts = 10)
        {
            params.name = HackingMethod::OptionalStopping;
            params.level = level;
            params.num = num;
            params.n_attempts = n_attempts;
            params.max_attempts = max_attempts;
            
            name = params.name;
        };

        void perform(Experiment *experiment, DecisionStrategy *decisionStrategy);
        
        
        /**
         Randmoize the parameters of the Optional Stopping
         */
        void randomize(int min_n, int max_n);
        
    private:
        /**
         Add _n_ observations to all groups and return the updated experiment to
         the `perform()` method.

         @param experiment A pointer to the experiment
         @param n number of new observations to be added
         */
        void addObservations(Experiment *experiment, const int &n);

    };
    
    // TODO:
    // I've included the inline for now, but these set of definitions should be moved
    // to a seperate place, maybe a CPP file for each hacking method and all its
    // implementations. This can include the parsing, enum map, ...
    
    inline
    void to_json(json& j, const OptionalStopping::Parameters& p) {
        j = json{
            {"name", magic_enum::enum_name<HackingMethod>(p.name)},
            {"level", p.level},
            {"num", p.num},
            {"n_attempts", p.n_attempts},
            {"max_attempts", p.max_attempts}
        };
    }
    
    inline
    void from_json(const json& j, OptionalStopping::Parameters& p) {
        
        // Using a helper template function to handle the optional and throw if necessary.
        p.name = get_enum_value_from_json<HackingMethod>("name", j);
        
        j.at("level").get_to(p.level);
        j.at("num").get_to(p.num);
        j.at("n_attempts").get_to(p.n_attempts);
        j.at("max_attempts").get_to(p.max_attempts);
    }

    /**
     @brief Declaration of Outlier Removal hacking method based on items' distance from their
     sample mean.
     
     */
    class SDOutlierRemoval : public HackingStrategy {
    public:
        
        struct Parameters {
            HackingMethod name = HackingMethod::SDOutlierRemoval;
            std::string level = "dv";
            std::string order = "max first";
            int num = 3;
            int n_attempts = 1;
            int max_attempts = 10;
            int min_observations = 15;
            std::vector<double> multipliers = {3};
        };
        
        Parameters params;
        
        SDOutlierRemoval();
        
        SDOutlierRemoval(const Parameters &p) : params{p} {
            name = params.name;
        };
        
        SDOutlierRemoval(std::string level = "dv", std::string order = "max first",
                         int num = 3, int n_attempts = 1, int max_attempts = 3,
                         int min_observations = 10, std::vector<double> multipliers = {3})
        {
            params.name = HackingMethod::SDOutlierRemoval;
            params.level = level;
            params.order = order;
            params.num = num;
            params.n_attempts = n_attempts;
            params.max_attempts = max_attempts;
            params.min_observations = min_observations;
            params.multipliers = multipliers;
            
            name = params.name;
        };
        
        // Submission hackedSubmission;
        void perform(Experiment* experiment, DecisionStrategy* decisionStrategy);
        
    private:

        int removeOutliers(Experiment *experiment, const int &n, const int &d);
        
    };
    
    
    inline
    void to_json(json& j, const SDOutlierRemoval::Parameters& p) {
        j = json{
            {"name", magic_enum::enum_name<HackingMethod>(p.name)},
            {"level", p.level},
            {"order", p.order},
            {"num", p.num},
            {"n_attempts", p.n_attempts},
            {"max_attempts", p.max_attempts},
            {"min_observations", p.min_observations},
            {"multipliers", p.multipliers}
        };
    }
    
    inline
    void from_json(const json& j, SDOutlierRemoval::Parameters& p) {
        
        // Using a helper template function to handle the optional and throw if necessary.
        p.name = get_enum_value_from_json<HackingMethod>("name", j);
        
        j.at("level").get_to(p.level);
        j.at("order").get_to(p.order);
        j.at("num").get_to(p.num);
        j.at("n_attempts").get_to(p.n_attempts);
        j.at("max_attempts").get_to(p.max_attempts);
        j.at("min_observations").get_to(p.min_observations);
        j.at("multipliers").get_to(p.multipliers);
    }
    

    class GroupPooling : public HackingStrategy {
        
    public:
        
        struct Parameters {
            HackingMethod name = HackingMethod::GroupPooling;
            std::vector<int> nums = {2};
        };
        
        Parameters params;
        
        GroupPooling(const Parameters &p) : params{p} {
            name = params.name;
        };
        
        explicit GroupPooling(std::vector<int> nums = {2}) {
            params.name = HackingMethod::GroupPooling;
            params.nums = nums;
            
            name = params.name;
        };
        
        // Submission hackedSubmission;
        void perform(Experiment* experiment, DecisionStrategy* decisionStrategy);
        
    private:
        
        void pool(Experiment* experiment, int r);
    };
    
    inline
    void to_json(json& j, const GroupPooling::Parameters& p) {
        j = json{
            {"name", magic_enum::enum_name<HackingMethod>(p.name)},
            {"nums", p.nums}
        };
    }
    
    inline
    void from_json(const json& j, GroupPooling::Parameters& p) {
        
        // Using a helper template function to handle the optional and throw if necessary.
        p.name = get_enum_value_from_json<HackingMethod>("name", j);
        
        j.at("nums").get_to(p.nums);
    }
        
        
    class ConditionDropping : public HackingStrategy {

    public:
        
        struct Parameters {
            HackingMethod name = HackingMethod::ConditionDropping;
        };
        
        Parameters params;
        
        ConditionDropping(const Parameters &p) : params{p} {
            name = params.name;
        };
        
        ConditionDropping() {
            params.name = HackingMethod::ConditionDropping;
            
            name = params.name;
        };
        
        void perform(Experiment* experiment, DecisionStrategy* decisionStrategy) { };
    };
    
    
    inline
    void to_json(json& j, const ConditionDropping::Parameters& p) {
        j = json{
            {"name", magic_enum::enum_name<HackingMethod>(p.name)}
        };
    }
    
    inline
    void from_json(const json& j, ConditionDropping::Parameters& p) {
        
        // Using a helper template function to handle the optional and throw if necessary.
        p.name = get_enum_value_from_json<HackingMethod>("name", j);
    }

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
