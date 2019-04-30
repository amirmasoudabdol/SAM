//
// Created by Amir Masoud Abdol on 2019-01-24.
//

#include <EffectEstimators.h>
#include <iostream>

using namespace sam;

EffectSizeEstimator::~EffectSizeEstimator() {
    // Pure deconstructor
};

EffectSizeEstimator *EffectSizeEstimator::build(const std::string &name){
    if (name == "CohensD") {
        return new CohensD();
    }else if (name == "HedgesG"){
        return new HedgesG();
    }else{
        throw std::invalid_argument("Uknown effect size estimator.\n");
    }
}

void CohensD::computeEffects(Experiment *experiment){
    
//    experiment->effects[this->name].resize(experiment->setup.ng);
//
    for (int i = 0; i < experiment->means.size(); i++) {
        experiment->effects[this->name][i] = cohens_d(experiment->setup.getValueOf("means")[i],
                                                      sqrt(experiment->setup.getValueOf("vars")[i]),
                                                      experiment->setup.getValueOf("nobs")[i],
                                                      experiment->means[i],
                                                      experiment->vars[i],
                                                      experiment->measurements[i].size());
    }
}

void HedgesG::computeEffects(Experiment *experiment){
    
    //    experiment->effects[this->name].resize(experiment->setup.ng);
    //
    for (int i = 0; i < experiment->means.size(); i++) {
        experiment->effects[this->name][i] = hedges_g(experiment->setup.getValueOf("means")[i],
                                                        sqrt(experiment->setup.getValueOf("vars")[i]),
                                                        experiment->setup.getValueOf("nobs")[i],
                                                        experiment->means[i],
                                                        experiment->vars[i],
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
