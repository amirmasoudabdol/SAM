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
        std::unordered_map<std::string, std::string> record;
        
    public:

        static std::string header(const json &effectslist);
        std::vector<std::string> col_names();

        //! Simulation ID
        int simid = 0;
        //! Publication ID
        int pubid = 0;
        int inx;
        //! Number of observation in submitted group
        int nobs;
        //! Effect size of the submitted group
        double yi;
        double vi;
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
        
        operator std::unordered_map<std::string, std::string>() {
            
            record["simid"] = std::to_string(simid);
            record["pubid"] = std::to_string(pubid);
            record["inx"] = std::to_string(inx);
            record["nobs"] = std::to_string(nobs);
            record["yi"] = std::to_string(yi);
            record["vi"] = std::to_string(vi);
            record["sei"] = std::to_string(sei);
            record["statistic"] = std::to_string(statistic);
            record["pvalue"] = std::to_string(pvalue);
            record["effect"] = std::to_string(effect);
            record["sig"] = std::to_string(sig);
            record["side"] = std::to_string(side);
            record["isHacked"] = std::to_string(isHacked);
            record["tnobs"] = std::to_string(tnobs);
            
            return record;
        }

    };

}

#endif //SAMPP_SUBMISSIONRECORD_H
