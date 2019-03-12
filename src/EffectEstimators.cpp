//
// Created by Amir Masoud Abdol on 2019-01-24.
//

#include <EffectEstimators.h>
#include <iostream>

//void StandardMeanDiffEffectEstimator::computeEffectFromStats(double mean1, double sd1, double nobs1, double mean2,
//                                                             double sd2, double nobs2) {
//    std::cout << "hi";
//}

void CohensD::computeEffects(Experiment *experiment){
    for (int i = 0; i < experiment->means.size(); i++) {
        experiment->effects[i] = cohens_d(experiment->setup.true_means[i],
                                          experiment->setup.true_vars[i],
                                          experiment->means[i],
                                          experiment->vars[i]);
    }
}

double cohens_d(double m1, double s1, double m2, double s2){
    return ((m1 - m2) / sqrt((s1 - s2) / 2));
}

double hedges_g(double d, int n, int df){
    return d / sqrt(n / df);
}

double pearsons_r(double d){
    return d / sqrt(d*d + 4);
}
