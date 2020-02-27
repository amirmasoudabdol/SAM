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

#include "spdlog/fmt/ostr.h" // must be included


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
        //! Index of the selected group
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
        //! Effect size estimation
        double effect;
        //! Indicates if the submission is significant or not
        bool sig = false;
        //! The side of the observed effect
        short side = 1;
        //! Whether the experiment is hacked or not
        bool isHacked = false;
        //! Stores the ID of hacking strategies applied on the experiment
        std::vector<int> hHistory;
        
        //! True number of observations
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

//        friend std::ostream& operator<<(std::ostream& os, const Submission& s);
        
        template<typename OStream>
        friend OStream &operator<<(OStream &os, const Submission &s){
            os <<
            s.simid << ",\t" <<
            s.pubid << ",\t" <<
            s.tnobs << ",\t" <<
            s.inx << ",\t" <<
            s.nobs << ",\t" <<
            s.mean << ",\t" <<
            s.var << ",\t" <<
            s.sei << ",\t" <<
            s.statistic << ",\t" <<
            s.pvalue << ",\t" <<
            s.effect << ",\t";
            os << s.side << ",\t";
            for (auto &hid : s.hHistory) {
                os << hid << ";";
            }
            os << ",\t";
            os << s.isHacked;
            
            return os;
        }

        operator std::map<std::string, std::string>();

    };

}

#endif //SAMPP_SUBMISSIONRECORD_H
