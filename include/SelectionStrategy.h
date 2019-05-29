//
// Created by Amir Masoud Abdol on 2019-01-25.
//

#ifndef SAMPP_SELECTIONSTRATEGIES_H
#define SAMPP_SELECTIONSTRATEGIES_H

// #include "Journal.h"
#include "Submission.h"
#include "RandomNumberGenerator.h"

#include "nlohmann/json.hpp"
#include "utils/magic_enum.hpp"

namespace sam {
    
    class Journal;

    using json = nlohmann::json;

    enum class SelectionMethod {
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

        SelectionMethod name;
        
        /**
         * @brief      Factory method for building a SelectionStrategy
         *
         * @param      config  A reference to `json['Journal Parameters'].
         * Usually Researcher::Builder is responsible for passing the object
         * correctly.
         *
         * @return     A new SelectionStrategy
         */
        static std::unique_ptr<SelectionStrategy> build(json &selection_straregy_config);
        
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
        
        
        /**
         A type keeping the parameters of the Significat Seleciton strategy.
         */
        struct Parameters {
            //! Selection strategy name
            SelectionMethod name = SelectionMethod::SignificantSelection;
            
            //! The \alpha at which the _selection strategy_ decides the significance
            //! of a publication
            double alpha = 0.05;

            //! Publication bias rate
            double pub_bias = 0.5;

            //! Indicates the _selection stratgy_'s preference toward positive, `1`,
            //! or negative, `-1` effect. If `0`, Journal doesn't have any preferences.
            int side = 1;
            
            //! Seed to be used for deicison distribution
            int seed = 42;
        };
        
        Parameters params;
        
        SignificantSelection(const Parameters &p) : params{p} {};
        
        SignificantSelection(double alpha = 0.05, double pub_bias = 0.5, int side = 1, int seed = 42) {

            
            params.name = SelectionMethod::SignificantSelection;
            params.alpha = alpha;
            params.pub_bias = pub_bias;
            params.side = side;
            params.seed = seed;
            
            mainRngStream = new RandomNumberGenerator(seed);
            
            name = params.name;
        };
        
        bool review(Submission& s);

    };
    
    
    inline
    void to_json(json& j, const SignificantSelection::Parameters& p) {
        j = json{
            {"name", magic_enum::enum_name<SelectionMethod>(p.name)},
            {"alpha", p.alpha},
            {"pub-bias", p.pub_bias},
            {"side", p.side}
            // {"seed", p.seed}
        };
    }
    
    inline
    void from_json(const json& j, SignificantSelection::Parameters& p) {
        
        auto name = magic_enum::enum_cast<SelectionMethod>(j["name"].get<std::string>());
        if (name.has_value())
            p.name = name.value();
        else
            throw std::invalid_argument("Unknown selection method.");
        
        j.at("alpha").get_to(p.alpha);
        j.at("pub-bias").get_to(p.pub_bias);
        j.at("side").get_to(p.side);
        // j.at("seed").get_to(p.seed);
        // FIXME: I do not provide seed in the config file. I need to fix this somehow.
    }

    /**
     @brief Random Selection Strategy
     
     In this method, Journal does not check any criteria for accepting or rejecting a
     submission. Each submission has 50% chance of being accepted or not.

     @note This is technically the SignificantSelection with pub_bias set to 0.
     
     */
    class RandomSelection : public SelectionStrategy {
    
    public:
        
        struct Parameters {
            SelectionMethod name = SelectionMethod::RandomSelection;
            int seed = 42;
        };
        
        Parameters params;
        
        RandomSelection(int seed) : seed(seed) {
            
            params.name = SelectionMethod::RandomSelection;
            mainRngStream = new RandomNumberGenerator(seed);
            
            name = params.name;
        }
        
        RandomSelection(const Parameters &p) : params{p} {};
        
        bool review(Submission& s);
        
        int seed;
    };
    
    inline
    void to_json(json& j, const RandomSelection::Parameters& p) {
        j = json{
            {"name", magic_enum::enum_name<SelectionMethod>(p.name)},
            {"seed", p.seed}
        };
    }
    
    inline
    void from_json(const json& j, RandomSelection::Parameters& p) {
        
        auto name = magic_enum::enum_cast<SelectionMethod>(j["name"].get<std::string>());
        if (name.has_value())
            p.name = name.value();
        else
            throw std::invalid_argument("Unknown selection method.");
        
        j.at("seed").get_to(p.seed);
    }
    
    

}

#endif //SAMPP_SELECTIONSTRATEGIES_H
