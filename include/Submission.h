//
// Created by Amir Masoud Abdol on 2019-01-25.
//

#ifndef SAMPP_SUBMISSIONRECORD_H
#define SAMPP_SUBMISSIONRECORD_H

#include <cmath>
#include <string>
#include <iostream>
#include <Experiment.h>

namespace sam {

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
        double effect;
        bool sig = false;       ///< Indicates if the submission is significant or not
        short side = 1;         ///< The side of the observed effect
        bool isHacked = false;
        std::vector<int> hHistory;
        
       int tnobs;
    //    double tyi;             ///< True mean/effect of the selected submission record
    //    double tvi;
        


        Submission() = default;
        Submission(Experiment& e, const int &index);
        
        
        ~Submission() = default;
        
        
        /**
         @return `true` if the Submission is significant, `false` otherwise
         */
        bool isSig() const {
            return sig;
        }

        friend std::ostream& operator<<(std::ostream& os, const Submission& s);

    };

}

#endif //SAMPP_SUBMISSIONRECORD_H
