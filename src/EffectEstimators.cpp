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
                                          std::sqrt(experiment->setup.true_vars[i]),
                                          experiment->setup.true_nobs[i],
                                          experiment->means[i],
                                          experiment->vars[i],
                                          experiment->measurements[i].size());
    }
}

double cohens_d(double Sm1, double Sd1, double Sn1,
				 double Sm2, double Sd2, double Sn2){

	// Degrees of freedom:
    double df = Sn1 + Sn2 - 2;
    
    // Pooled variance and hence standard deviation:
    double sp = sqrt(((Sn1-1) * Sd1 * Sd1 + (Sn2-1) * Sd2 * Sd2) / df);

    // Cohen's D:
    return std::abs(Sm1 - Sm2) / sp;
}

double hedges_g(double Cd, int Sn, int df){
    return Cd / sqrt(Sn / df);
}

double pearsons_r(double Cd){
    return Cd / sqrt(Cd*Cd + 4);
}

double glass_delta(double Sm1, double Sd1, double Sn1,
					double Sm2, double Sd2, double Sn2){
	return (Sm1 - Sm2) / Sd2;
}
