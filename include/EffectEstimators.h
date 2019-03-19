//
// Created by Amir Masoud Abdol on 2019-01-24.
//

#ifndef SAMPP_EFFECTESTIMATORS_H
#define SAMPP_EFFECTESTIMATORS_H

#include <vector>
#include "Experiment.h"

#include "nlohmann/json.hpp"

using json = nlohmann::json;

class EffectSizeEstimator {
    double effect;

public:
    
    static EffectSizeEstimator *build(json &config);
    
    virtual ~EffectSizeEstimator() = 0;

    virtual void computeEffects(Experiment *experiment) = 0;

//private:
//    double _calculate_effect();

};


class CohensD : public EffectSizeEstimator {
//private:
//
public:
    void computeEffects(Experiment *experiment);
};


double cohens_d(double Sm1, double Sd1, double Sn1,
				 double Sm2, double Sd2, double Sn2);
double hedges_g(double Cd, int Sn, int df);
double pearsons_r(double Cd);
double glass_delta(double Sm1, double Sd1, double Sn1,
					double Sm2, double Sd2, double Sn2);

//class StandardMeanDiffEffectEstimator : public EffectEstimators {
//public:
//    void computeEffectFromStats(double mean1, double sd1, double nobs1, double mean2, double sd2, double nobs2);
//    void computeEffectFromData(std::vector<double> dts1, std::vector<double> dts2);
//};


#endif //SAMPP_EFFECTESTIMATORS_H
