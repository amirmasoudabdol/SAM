//
// Created by Amir Masoud Abdol on 2019-01-25.
//

#ifndef SAMPP_SUBMISSIONRECORD_H
#define SAMPP_SUBMISSIONRECORD_H

#include <cmath>
#include <string>
#include <iostream>
#include <Experiment.h>

class Submission {

    
public:

    static std::string header(const json &effectslist);

    int simid = 0;          ///< Simulation ID
    int pubid = 0;          ///< Publication ID
    int inx;
    int nobs;               ///< Number of observation in submitted group
    double yi;              ///< Effect size of the submitted group
    double vi;
    double sei;             ///< Standard error of the submitted group
    double statistic;       ///< Corresponding statistics of the submitted group
    double pvalue;          ///< _P_-value of the submitted group
    std::vector<double> effects;
    bool sig = false;       ///< Indicates if the submission is significant or not
    short side = 1;         ///< The side of the observed effect
    bool isHacked = false;
    std::vector<int> hHistory;
    
//    int tnobs;
//    double tyi;             ///< True mean/effect of the selected submission record
//    double tvi;
    
    // Journal's Parameters
//    double pub_bias;
    

    Submission() = default;
    
    Submission(Experiment& e, const int &index){
        
//        tnobs = e.setup.true_nobs[index];
//        tyi = e.setup.true_means[index];
//        tvi = e.setup.true_vars[index];
//
        inx = index;
        nobs = e.measurements[index].size();        // TODO: I think this needs to be generalized
        yi = e.means[index];
        vi = e.vars[index];
        sei = e.ses[index];

        statistic = e.statistics[index];
        pvalue = e.pvalues[index];
        
        for (auto &estimator : e.effectSizeEstimators){
            effects.push_back(e.effects[estimator->name][index]);
        }
        
        sig = e.sigs[index];
        
        // CHECK: This will cause problem if I do GroupPooling!
        // BUG: This is also a problem if I'm working with the LatentModel because I'm
        // storing latent means, vars with different names. **This is just not a good idea**.
        // Submission should be self-contained and I shouldn't look into another object
//        tyi = e.setup.true_means[index];
        // FIXME: This is fishy!
        side = std::copysign(1.0, yi - e.setup.true_means[index]);
        
        isHacked = e.is_hacked;
        
        hHistory = e.hackingHistory;
    };
    
    ~Submission() = default;
    
    
    /**
     @return `true` if the Submission is significant, `false` otherwise
     */
    const bool isSig() const {
        return sig;
    }

    friend std::ostream& operator<<(std::ostream& os, const Submission& s);

};



#endif //SAMPP_SUBMISSIONRECORD_H
