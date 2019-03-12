//
// Created by Amir Masoud Abdol on 2019-01-24.
//

#ifndef SAMPP_EFFECTESTIMATORS_H
#define SAMPP_EFFECTESTIMATORS_H

#include <vector>
#include "Experiment.h"

class EffectEstimators {
    double effect;

public:

//    void computeEffectFromStats(double mean1, double sd1, double nobs1, double mean2, double sd2, double nobs2);
//    void computeEffectFromData(std::vector<double> dts1, std::vector<double> dts2);
//    virtual std::vector<double> computeEffects(std::vector<std::vector<double >>);
    virtual void computeEffects(Experiment *experiment) = 0;

//private:
//    double _calculate_effect();

};


class CohensD : public EffectEstimators {
//private:
//
public:
    void computeEffects(Experiment *experiment);
};


double cohens_d(double m1, double s1, double m2, double s2);
double hedges_g(double d, int n, int df);
double pearsons_r(double d);

//class StandardMeanDiffEffectEstimator : public EffectEstimators {
//public:
//    void computeEffectFromStats(double mean1, double sd1, double nobs1, double mean2, double sd2, double nobs2);
//    void computeEffectFromData(std::vector<double> dts1, std::vector<double> dts2);
//};


#endif //SAMPP_EFFECTESTIMATORS_H
