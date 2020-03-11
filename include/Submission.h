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

    class GroupData;

    class Submission {

        std::vector<std::string> columns;
        std::map<std::string, std::string> record;
        
    public:

        std::vector<std::string> cols();

        //! Simulation ID
        int simid = 0;
        //! Publication ID
        int pubid = 0;
        //! Index of the selected group
        
        //! True number of observations
        int tnobs;
        
        
        GroupData group_;
        
        int id_;
        //! Number of observation in submitted group
        int nobs_;
        //! Effect size of the submitted group
        double mean_;
        
        double var_;
        //! Standard error of the submitted group
        double ses_;
        //! Corresponding statistics of the submitted group
        double stats_;
        //! _P_-value of the submitted group
        double pvalue_;
        //! Effect size estimation
        double effect_;
        //! Indicates if the submission is significant or not
        bool sig_;
//        //! The side of the observed effect
//        short eff_side = 1;
//        //! Whether the experiment is hacked or not
//        bool is_hacked = false;
//        //! Stores the ID of hacking strategies applied on the experiment
//        std::vector<int> hHistory;
        
        


        Submission() = default;
        Submission(Experiment& e, const int &index);
        
        
        ~Submission() = default;
        
        
        /**
         @return `true` if the Submission is significant, `false` otherwise
         */
        bool isSig() const {
            return group_.sig_;
        }
        
        template<typename OStream>
        friend OStream &operator<<(OStream &os, const Submission &s) {
            os <<
            s.simid << ",\t" <<
            s.pubid << ",\t" <<
            s.tnobs << ",\t" <<
            s.group_.id_ << ",\t" <<
            s.group_.nobs_ << ",\t" <<
            s.group_.mean_ << ",\t" <<
            s.group_.var_ << ",\t" <<
            s.group_.ses_ << ",\t" <<
            s.group_.stats_ << ",\t" <<
            s.group_.pvalue_ << ",\t" <<
            s.group_.effect_ << ",\t";
//            os << s.eff_side << ",\t";
//            for (auto &hid : s.hHistory) {
//                os << hid << ";";
//            }
            os << ",\t";
            os << s.group_.is_hacked_;
            
            return os;
        }

        operator std::map<std::string, std::string>();

    };

}

#endif //SAMPP_SUBMISSIONRECORD_H
