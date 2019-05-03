//
// Created by Amir Masoud Abdol on 2019-01-24.
//

#ifndef SAMPP_EFFECTESTIMATORS_H
#define SAMPP_EFFECTESTIMATORS_H

#include <vector>
#include <string>
#include "Experiment.h"

#include "nlohmann/json.hpp"

namespace sam {

    using json = nlohmann::json;

    class EffectSizeEstimator {
        

    public:
        
        static std::shared_ptr<EffectSizeEstimator>build(const std::string &name);
        
        virtual ~EffectSizeEstimator() = 0;

        virtual void computeEffects(Experiment *experiment) = 0;

        std::string name;

    };


    class CohensD : public EffectSizeEstimator {

    public:

        explicit CohensD() {
            name = "CohensD";
        };

        void computeEffects(Experiment *experiment);
        
    };


    class HedgesG : public EffectSizeEstimator {
        
    public:

        explicit HedgesG(){
            name = "HedgesG";
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
    //    const std::string name = "OddRatio";
    //};
    //
    //
    //class PearsonR : public EffectSizeEstimator {
    //    
    //public:
    //    void computeEffects(Experiment *experiment);
    //    
    //private:
    //    const std::string name = "PearsonR";
    //};
    //
    //class GlassDelta : public EffectSizeEstimator {
    //    
    //public:
    //    void computeEffects(Experiment *experiment);
    //    
    //private:
    //    const std::string name = "GlassDelta";
    //};
    //
    //class EtaSquared : public EffectSizeEstimator {
    //    
    //public:
    //    void computeEffects(Experiment *experiment);
    //    
    //private:
    //    const std::string name = "EtaSquared";
    //};



    void cohens_d(Experiment *expr);


    double cohens_d(double Sm1, double Sd1, double Sn1,
                     double Sm2, double Sd2, double Sn2);
    double hedges_g(double Sm1, double Sd1, double Sn1,
                    double Sm2, double Sd2, double Sn2);
    double pearsons_r(double Cd);
    double glass_delta(double Sm1, double Sd1, double Sn1,
                        double Sm2, double Sd2, double Sn2);


}

#endif //SAMPP_EFFECTESTIMATORS_H
