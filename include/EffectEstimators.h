//
// Created by Amir Masoud Abdol on 2019-01-24.
//

#ifndef SAMPP_EFFECTESTIMATORS_H
#define SAMPP_EFFECTESTIMATORS_H

#include <vector>

class EffectEstimators {
    double effect;

public:

    void computeEffectFromStats(double mean1, double sd1, double nobs1, double mean2, double sd2, double nobs2);
    void computeEffectFromData(std::vector<double> dts1, std::vector<double> dts2);

private:
    double _calculate_effect();

};


class StandardMeanDiffEffectEstimator : public EffectEstimators {
public:
    void computeEffectFromStats(double mean1, double sd1, double nobs1, double mean2, double sd2, double nobs2);
    void computeEffectFromData(std::vector<double> dts1, std::vector<double> dts2);
};


#endif //SAMPP_EFFECTESTIMATORS_H
