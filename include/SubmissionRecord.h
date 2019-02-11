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
    
    int simid = 0;
    int pubid = 0;
//    std::string method;
    int nobs;
    double effect;
    double statistic;
    double pvalue;
    bool sig = false;
    short side = 1;
//    bool isHacked = false;
//    std::string hacking_method = "";

    Submission() = default;
    
    Submission(Experiment& e, int index){
        nobs = e.setup.nobs;        // TODO: I think this needs to be generalized
                                    // This needs to be updated, especially if we perform OptionalStopping, then `setup.nobs` is never updated. All statistics are running on measurements.size(), so they are fine, but not this one.
        effect = e.effects[index];
        statistic = e.statistics[index];
        pvalue = e.pvalues[index];
        
        sig = pvalue < alpha;
        
        // CHECK: This will cause problem if I do GroupPooling!
        double diff = effect - e.setup.true_means[index];
        side = (diff > 0) ? 1 : ((diff < 0) ? -1 : 0);
    };
    
    ~Submission() = default;
    
    bool isSig() {
        return sig;
    }

    friend std::ostream& operator<<(std::ostream& os, const Submission& s);

};



#endif //SAMPP_SUBMISSIONRECORD_H
