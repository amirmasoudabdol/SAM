//
// Created by Amir Masoud Abdol on 2019-01-25.
//

#ifndef SAMPP_SUBMISSIONRECORD_H
#define SAMPP_SUBMISSIONRECORD_H

#include <string>
#include <iostream>
#include <Experiment.h>

class Submission {
    
public:
    // TODO: I would like to have this as static, but I cannot set it properly.
    // FIXME: This needs to be more flexible and generalized.
    double alpha = 0.05;
    
    int simid = 0;          ///< Simulation ID
    int pubid = 0;          ///< Publication ID
//    std::string method;
    int nobs;               ///< Number of observation in submitted group
    double yi;              ///< Effect size of the submitted group
    double sei;             ///< Standard error of the submitted group
    double statistic;       ///< Corresponding statistics of the submitted group
    double pvalue;          ///< _P_-value of the submitted group
    bool sig = false;       ///< Indicates if the submission is significant or not
    short side = 1;         ///< The side of the observed effect
//    bool isHacked = false;
//    std::string hacking_method = "";

    Submission() = default;
    
    Submission(Experiment& e, int index){
        nobs = e.setup.nobs;        // TODO: I think this needs to be generalized
                                    // This needs to be updated, especially if we perform OptionalStopping, then `setup.nobs` is never updated. All statistics are running on measurements.size(), so they are fine, but not this one.
        yi = e.effects[index];
        sei = e.ses[index];
        statistic = e.statistics[index];
        pvalue = e.pvalues[index];
        
        sig = pvalue < alpha;
        
        // CHECK: This will cause problem if I do GroupPooling!
        // BUG: This is also a problem if I'm working with the LatentModel because I'm
        // storing latent means, vars with different names. **This is just not a good idea**.
        // Submission should be self-contained and I shouldn't look into another object
//        double diff = yi - e.setup.true_means[index];
//        side = (diff > 0) ? 1 : ((diff < 0) ? -1 : 0);
        side = 1;
    };
    
    ~Submission() = default;
    
    
    /**
     @return `true` if the Submission is significant, `false` otherwise
     */
    bool isSig() {
        return sig;
    }

    friend std::ostream& operator<<(std::ostream& os, const Submission& s);

};



#endif //SAMPP_SUBMISSIONRECORD_H
