//
// Created by Amir Masoud Abdol on 2019-01-24.
//

#ifndef SAMPP_EFFECTESTIMATORS_H
#define SAMPP_EFFECTESTIMATORS_H

#include <vector>
#include <string>
#include "Experiment.h"

#include "nlohmann/json.hpp"

using json = nlohmann::json;

class EffectSizeEstimator {
    

public:
    
    static EffectSizeEstimator *build(const std::string &name);
    
    virtual ~EffectSizeEstimator() = 0;

    virtual void computeEffects(Experiment *experiment) = 0;

    std::string name;

};


class CohensD : public EffectSizeEstimator {

public:

    explicit CohensD() {
        name = "Cohens D";
    };

    void computeEffects(Experiment *experiment);
    
};


class HedgesG : public EffectSizeEstimator {
    
public:

    explicit HedgesG(){
        name = "Hedges G";
    }

    void computeEffects(Experiment *experiment);
    
};

//
//class OddRatio : public EffectSizeEstimator {
//    
//public:
//    void computeEffects(Experiment *experiment);
//    
//private:
//    const std::string name = "Odd Ratio";
//};
//
//
//class PearsonR : public EffectSizeEstimator {
//    
//public:
//    void computeEffects(Experiment *experiment);
//    
//private:
//    const std::string name = "Pearson R";
//};
//
//class GlassDelta : public EffectSizeEstimator {
//    
//public:
//    void computeEffects(Experiment *experiment);
//    
//private:
//    const std::string name = "Glass Delta";
//};
//
//class EtaSquared : public EffectSizeEstimator {
//    
//public:
//    void computeEffects(Experiment *experiment);
//    
//private:
//    const std::string name = "Eta Squared";
//};





double cohens_d(double Sm1, double Sd1, double Sn1,
				 double Sm2, double Sd2, double Sn2);
double hedges_g(double Sm1, double Sd1, double Sn1,
                double Sm2, double Sd2, double Sn2);
double pearsons_r(double Cd);
double glass_delta(double Sm1, double Sd1, double Sn1,
					double Sm2, double Sd2, double Sn2);

//class StandardMeanDiffEffectEstimator : public EffectEstimators {
//public:
//    void computeEffectFromStats(double mean1, double sd1, double nobs1, double mean2, double sd2, double nobs2);
//    void computeEffectFromData(std::vector<double> dts1, std::vector<double> dts2);
//};


#endif //SAMPP_EFFECTESTIMATORS_H
