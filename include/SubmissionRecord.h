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
    // TODO: I would like to have this as static, but I cannot set it properly.
    // FIXME: This needs to be more flexible and generalized.
//    double alpha = 0.05;
    
    int simid = 0;          ///< Simulation ID
    int pubid = 0;          ///< Publication ID
//    std::string method;
    int inx;
    int nobs;               ///< Number of observation in submitted group
    double yi;              ///< Effect size of the submitted group
    double sei;             ///< Standard error of the submitted group
    double statistic;       ///< Corresponding statistics of the submitted group
    double pvalue;          ///< _P_-value of the submitted group
    std::vector<double> effects;
    bool sig = false;       ///< Indicates if the submission is significant or not
    short side = 1;         ///< The side of the observed effect
    bool isHacked = false;
//    std::string hacking_method = "";
    
    int tnobs;
    double tyi;             ///< True mean/effect of the selected submission record
    double tvi;
    double tcov;
    
    // Journal's Parameters
//    double alpha = 0.05;         // FIXME: I'm hardcoded, this is a tricky problem, remember all the debuggin. I
    double pubbias;

    Submission() = default;
    
    Submission(Experiment& e, const int &index){
        
        // FIXME: This needs to be updated, especially if we perform OptionalStopping, then `setup.nobs` is never updated. All statistics are running on measurements.size(), so they are fine, but not this one.
        
        tnobs = e.setup.true_nobs[index];               // TODO: This is not good enough either, niether e.setup.true_nobs since I'm using `0` to tell the simulator that I want to randomize the values, and this will only report `0`. See also: [#27](https://github.com/amirmasoudabdol/SAMpp/issues/27) and [#47](https://github.com/amirmasoudabdol/SAMpp/issues/47).
        tyi = e.setup.true_means[index];
        tvi = e.setup.true_vars[index];
        tcov = e.setup.cov;         // FIXME: Not generalized again
        
        inx = index;
        nobs = e.measurements[index].size();        // TODO: I think this needs to be generalized
        yi = e.means[index];
        sei = e.ses[index];

        statistic = e.statistics[index];
        pvalue = e.pvalues[index];
        
//        effect = e.effects[index];
        for (auto &estimator : e.effectSizeEstimators){
//            std::cout << estimator->name;
            effects.push_back(e.effects[estimator->name][index]);
        }
        
        
//        sig = (pvalue < alpha);
        sig = e.sigs[index];
        
        // CHECK: This will cause problem if I do GroupPooling!
        // BUG: This is also a problem if I'm working with the LatentModel because I'm
        // storing latent means, vars with different names. **This is just not a good idea**.
        // Submission should be self-contained and I shouldn't look into another object
//        double diff = yi - e.setup.true_means[index];
//        side = (diff > 0) ? 1 : ((diff < 0) ? -1 : 0);
        tyi = e.setup.true_means[index];
        side = std::copysign(1.0, yi - e.setup.true_means[index]);
        
        isHacked = e.isHacked;
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
