//
// Created by Amir Masoud Abdol on 2019-03-03.
//

#include "GroupData.h"

namespace sam {

    void GroupData::updateStats() {
        
        /// TODO: Check if is_stats_updated, if so, don't recalculate
        
//        if (not is_stats_updated_) {
            nobs_ = measurements_.size();
            mean_ = arma::mean(measurements_);
            var_ = arma::var(measurements_);
            stddev_ = arma::stddev(measurements_);
            ses_ = sqrt( var_ / nobs_ );
            
//        }
        
    }

    GroupData::operator std::map<std::string, std::string>() {
       
       static std::map<std::string, std::string> record;
       
       record["gid"] = std::to_string(id_);
       
       record["nobs"] = std::to_string(nobs_);
       record["mean"] = std::to_string(mean_);
       record["var"] = std::to_string(var_);
       record["stddev"] = std::to_string(stddev_);
       record["ses"] = std::to_string(ses_);
       
       /// This can be replaced with an map.insert() and basically
       /// just insert TestStrategy's map() operator
       record["pvalue"] = std::to_string(pvalue_);
       record["effect"] = std::to_string(effect_);
       record["sig"] = std::to_string(sig_);
       
       record["is_hacked"] = std::to_string(is_hacked_);
       
        return record;
   }


}
