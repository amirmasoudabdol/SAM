//
// Created by Amir Masoud Abdol on 2019-01-25.
//

#ifndef SAMPP_SUBMISSIONRECORD_H
#define SAMPP_SUBMISSIONRECORD_H

#include <map>
#include <cmath>
#include <string>
#include <iostream>
#include <Experiment.h>

namespace sam {

    class Submission {

        std::vector<std::string> columns;
        std::map<std::string, std::string> record;
        
    public:

//        static std::string header(const json &effectslist);
        std::vector<std::string> cols();

        //! Simulation ID
        int simid = 0;
        //! Publication ID
        int pubid = 0;
        int inx;
        //! Number of observation in submitted group
        int nobs;
        //! Effect size of the submitted group
        double mean;
        double var;
        //! Standard error of the submitted group
        double sei;
        //! Corresponding statistics of the submitted group
        double statistic;
        //! _P_-value of the submitted group
        double pvalue;
        double effect;
        //! Indicates if the submission is significant or not
        bool sig = false;
        //! The side of the observed effect
        short side = 1;
        bool isHacked = false;
        std::vector<int> hHistory;
        
       int tnobs;
        


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

        operator std::map<std::string, std::string>();

    };

}

#endif //SAMPP_SUBMISSIONRECORD_H
