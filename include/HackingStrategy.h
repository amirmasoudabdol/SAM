//
// Created by Amir Masoud Abdol on 2019-01-25.
//

/**
* \defgroup HackingStrategies
* @brief List of available hacking strategies
*
* Description to come!
*/

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
     
     @ingroup HackingStrategies
     */
    class OptionalStopping : public HackingStrategy {
        
    public:
        
        /**
         @ingroup HackingStrategiesParameters
         */
        struct Parameters {
            //! Placeholder for hacking strategy name
            HackingMethod name = HackingMethod::OptionalStopping;
            
            //! Indicates where the optional stopping should be applied!
            std::string level = "dv";
            
            //! Number of new observations to be added to each group
            int num = 3;
            
            //! Number of times that Researcher add `num` observations to each group
            int n_attempts = 1;
            
            //! Maximum number of times that Researcher tries to add new observations to
            //! each group
            int max_attempts = 10;
        };

        Parameters params;
        
        OptionalStopping() = default;

        OptionalStopping(const Parameters &p) : params{p} {
            name = params.name;
        } ;

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
            {"_name", p.name},
            {"level", p.level},
            {"num", p.num},
            {"n_attempts", p.n_attempts},
            {"max_attempts", p.max_attempts}
        };
    }
    
    inline
    void from_json(const json& j, OptionalStopping::Parameters& p) {
        
        // Using a helper template function to handle the optional and throw if necessary.
        j.at("_name").get_to(p.name);
        
        j.at("level").get_to(p.level);
        j.at("num").get_to(p.num);
        j.at("n_attempts").get_to(p.n_attempts);
        j.at("max_attempts").get_to(p.max_attempts);
    }

    /**
     @brief Declaration of Outlier Removal hacking method based on items' distance from their
     sample mean.
     
     @ingroup HackingStrategies
     */
    class SDOutlierRemoval : public HackingStrategy {
    public:
        
        
        /// Parameters of Outliers Removal Strategy
        /// 
        struct Parameters {
            HackingMethod name = HackingMethod::SDOutlierRemoval;
            std::string level = "dv";
            std::string order = "max first";
            int num {3};
            int n_attempts {1};
            int max_attempts {10};
            int min_observations {15};
            std::vector<double> multipliers = {3};
        };
        
        Parameters params;
        
        SDOutlierRemoval() = default;
        
        SDOutlierRemoval(const Parameters &p) : params{p} {
            name = params.name;
        };
        
        // Submission hackedSubmission;
        void perform(Experiment* experiment, DecisionStrategy* decisionStrategy);
        
    private:

        int removeOutliers(Experiment *experiment, const int n, const double d);
        
    };
    
    
    inline
    void to_json(json& j, const SDOutlierRemoval::Parameters& p) {
        j = json{
            {"_name", p.name},
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
        j.at("_name").get_to(p.name);
        
        j.at("level").get_to(p.level);
        j.at("order").get_to(p.order);
        j.at("num").get_to(p.num);
        j.at("n_attempts").get_to(p.n_attempts);
        j.at("max_attempts").get_to(p.max_attempts);
        j.at("min_observations").get_to(p.min_observations);
        j.at("multipliers").get_to(p.multipliers);
    }


    /**
     @brief The subjective outlier removal refers to a type of outliers removal
     where the researcher continiously lowers the threshold of identifying an
     outlier, `k`, until it finds a significant (or satisfactory) result.
     
     @sa DecisionStrategy
     @sa DecisionPreference
     */
    class SubjectiveOutlierRemoval : public HackingStrategy {
    public:
        
        
        /**
         @brief SubjectiveOutlierRemoval's parameters.
         
         These are parameters specific to this hacking strategy. You can set them
         either progmatically when you are constructing a new SubjectiveOutlierRemoval,
         e.g., `SubjectiveOutlierRemoval sor{<name>, {min, max}, ssize};`.
         
         Or, when you are using `SAMrun` to run your simulation. In this case, your JSON
         variable must comply with the name and type of paramters here. For example, the
         following JSON defines the default subjective outliers removal.
         
         ```json
         {
            "_name": "SubjectiveOutlierRemoval",
            "range": [2, 4],
            "step_size": 0.1
         }
         ```
         */
        struct Parameters {
            //! A placeholder for the name
            HackingMethod name = HackingMethod::SubjectiveOutlierRemoval;
            
            //! A vector of `{min, max}`, defining the range of `K`.
            std::vector<int> range {2, 4};  // TODO: This can be replaced by std::pair
            
            //! Indicates the step size of walking through K's
            double step_size {0.1};
        };
        
        Parameters params;
        
        SubjectiveOutlierRemoval() = default;
        
        SubjectiveOutlierRemoval(const Parameters &p) : params(p) {
            name = params.name;
        };
        
        void perform(Experiment* experiment, DecisionStrategy* decisionStrategy);
    };


        inline
        void to_json(json& j, const SubjectiveOutlierRemoval::Parameters& p) {
            j = json{
                {"_name", p.name},
                {"range", p.range},
                {"step_size", p.step_size}
            };
        }

        inline
        void from_json(const json& j, SubjectiveOutlierRemoval::Parameters& p) {
            
            // Using a helper template function to handle the optional and throw if necessary.
            j.at("_name").get_to(p.name);
            j.at("range").get_to(p.range);
            j.at("step_size").get_to(p.step_size);
        }
    

    class GroupPooling : public HackingStrategy {
        
    public:
        
        struct Parameters {
            HackingMethod name = HackingMethod::GroupPooling;
            std::vector<int> nums = {2};
        };
        
        Parameters params;
        
        GroupPooling() = default;
        
        GroupPooling(const Parameters &p) : params{p} {
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
            {"_name", p.name},
            {"nums", p.nums}
        };
    }
    
    inline
    void from_json(const json& j, GroupPooling::Parameters& p) {
        
        // Using a helper template function to handle the optional and throw if necessary.
        j.at("_name").get_to(p.name);
        
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
            {"_name", p.name}
        };
    }
    
    inline
    void from_json(const json& j, ConditionDropping::Parameters& p) {
        
        // Using a helper template function to handle the optional and throw if necessary.
        j.at("_name").get_to(p.name);
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
