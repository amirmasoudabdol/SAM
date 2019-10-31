//
// Created by Amir Masoud Abdol on 2019-01-24.
//

#include <iostream>

#include "Experiment.h"
#include "EffectStrategy.h"

using namespace sam;

EffectStrategy::~EffectStrategy() {
    // Pure deconstructor
};

std::unique_ptr<EffectStrategy>EffectStrategy::build(json &effect_strategy_config) {
    if (effect_strategy_config["_name"] == "CohensD") {
        return std::make_unique<CohensD>();
    }else if (effect_strategy_config["_name"] == "HedgesG"){
        return std::make_unique<HedgesG>();
    }else{
        throw std::invalid_argument("Uknown effect size estimator.\n");
    }
}

void CohensD::computeEffects(Experiment *experiment){
    
    for (int i{experiment->setup.nd()}, d{0}; i < experiment->means.size(); i++, d%=experiment->setup.nd()) {
        experiment->effects[i] = cohens_d(experiment->means[d],
                                          experiment->stddev[d],
                                          experiment->nobs[d],
                                          experiment->means[i],
                                          experiment->stddev[i],
                                          experiment->measurements[i].size());
    }
}

void HedgesG::computeEffects(Experiment *experiment){
    
    // Skipping the first group as it's the control group
    // TODO: This is an ugly loop. Fix it!
    for (int i{experiment->setup.nd()}, d{0}; i < experiment->means.size(); i++, d%=experiment->setup.nd()) {
        experiment->effects[i] = hedges_g(experiment->means[d],
                                            experiment->stddev[d],
                                            experiment->nobs[d],
                                            experiment->means[i],
                                            experiment->stddev[i],
                                            experiment->measurements[i].size());
    }
}

namespace sam {

    double cohens_d(double Sm1, double Sd1, double Sn1,
    				 double Sm2, double Sd2, double Sn2){

    	// Degrees of freedom:
        double df = Sn1 + Sn2 - 2;
        
        // Pooled variance and hence standard deviation:
        double sp = sqrt(((Sn1-1) * Sd1 * Sd1 + (Sn2-1) * Sd2 * Sd2) / df);

        // Cohen's D:
        return std::abs(Sm1 - Sm2) / sp;
    }

    double hedges_g(double Sm1, double Sd1, double Sn1,
                    double Sm2, double Sd2, double Sn2){
        
        // Degrees of freedom:
        double df = Sn1 + Sn2 - 2;
        
        // Pooled variance and hence standard deviation:
        double sp = sqrt(((Sn1-1) * Sd1 * Sd1 + (Sn2-1) * Sd2 * Sd2) / df);
        
        // Cohen's D:
        double Cd = std::abs(Sm1 - Sm2) / sp;
        
        // Sum of sample sizes
        double n = Sn1 + Sn2;
        
        // Adding the bais correction factor for n < 50
        if (n < 50) {
            return Cd * (n - 3) / (n - 2.25) * sqrt((n-2) / n);
        }
        
        return Cd;
    //    return Cd / sqrt(Sn / df);
    }

    double pearsons_r(double Cd){
        return Cd / sqrt(Cd*Cd + 4);
    }

    double glass_delta(double Sm1, double Sd1, double Sn1,
    					double Sm2, double Sd2, double Sn2){
    	return (Sm1 - Sm2) / Sd2;
    }

}
