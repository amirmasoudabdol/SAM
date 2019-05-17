//
// Created by Amir Masoud Abdol on 2019-01-25.
//

#ifndef SAMPP_SELECTIONSTRATEGIES_H
#define SAMPP_SELECTIONSTRATEGIES_H

// #include "Journal.h"
#include "Submission.h"
#include "RandomNumberGenerator.h"

#include "nlohmann/json.hpp"

namespace sam {
    
    class Journal;

    using json = nlohmann::json;

    enum class SelectionType {
        SignificantSelection,
        RandomSelection
    };

    /**
     @brief Abstract class for Journal's selection strategies.
     
     A Journal will decide if a Submission is going to be accepted or rejected. This
     decision can be made based on different criteria or formula. A SelectionStrategy
     provides an interface for implementing different selection strategies.
     */
    class SelectionStrategy {
        
    protected:
        RandomNumberGenerator* mainRngStream;

    public:

        struct SelectionStrategyParameters {
            SelectionType name;
            double alpha;
            double pub_bias;
            int side;
            int seed = -1;
        };
        
        SelectionStrategyParameters params;


        SelectionType id;
        
        /**
         * @brief      Factory method for building a SelectionStrategy
         *
         * @param      config  A reference to `json['Journal Parameters'].
         * Usually Researcher::Builder is responsible for passing the object
         * correctly.
         *
         * @return     A new SelectionStrategy
         */
        static SelectionStrategy* build(json &selection_straregy_config);

        static SelectionStrategy* build(SelectionStrategyParameters &ssp);
        
        /**
         * @brief      Pure deconstructor of the base class
         */
        virtual ~SelectionStrategy() = 0;

        /**
         * @brief      Review the Submission and decides if it's
         * going to be accepted or rejected. When deriving from
         * SelectionStrategy, `review` is the main interface and
         * `Journal` relies on its output
         *
         * @param[in]  s     A reference to a Submission
         *
         * @return     A boolean indicating whether the Submission
         * should be accepted or not.
         */
        virtual bool review(Submission& s) = 0 ;
    };

    /**
     @brief Significant-based Selection Strategy
     
     Significant-based selection strategy accepts a publication if the given *p*-value
     is significant. Certain degree of *publication bias*, can be specified. In this case,
     a Submission has a chance of being published even if the statistics is not significant.
     Moreover, the SignificantSelection can be tailored toward either positive or negative
     effect. In this case, the Journal will only accept Submissions with larger or smaller effects.
     */
    class SignificantSelection : public SelectionStrategy {

    public:
        SignificantSelection(double alpha = 0.05, double pub_bias = 0.5, int side = 1, int seed = 42):
            alpha(alpha),  pub_bias(pub_bias), side(side) {
            
            id = SelectionType::SignificantSelection;

            mainRngStream = new RandomNumberGenerator(seed);
        };
        
        // TODO: Fix me!
        SignificantSelection(SelectionStrategyParameters ssp) : alpha(ssp.alpha),  pub_bias(ssp.pub_bias), side(ssp.side) {
            params = ssp;
            
            mainRngStream = new RandomNumberGenerator(ssp.seed);
        }

        bool review(Submission& s);

    private:
        //! The \alpha at which the _selection strategy_ decides the significance
        //! of a publication
        double alpha;
        
        //! Publication bias rate
        double pub_bias;
        
        //! Indicates the _selection stratgy_'s preference toward positive, `1`,
        //! or negative, `-1` effect. If `0`, Journal doesn't have any preferences.
        int side;
    };

    /**
     @brief Random Selection Strategy
     
     In this method, Journal does not check any criteria for accepting or rejecting a
     submission. Each submission has 50% chance of being accepted or not.
     
     */
    class RandomSelection : public SelectionStrategy {
    
    public:
        
        explicit RandomSelection(int seed) : seed(seed) {
            
            id = SelectionType::RandomSelection;

            mainRngStream = new RandomNumberGenerator(seed);
        }
        
        RandomSelection(SelectionStrategyParameters ssp) {
            params = ssp;
            
            mainRngStream = new RandomNumberGenerator(ssp.seed);
        }
        
        bool review(Submission& s);
        
        int seed;
    };

}

#endif //SAMPP_SELECTIONSTRATEGIES_H
