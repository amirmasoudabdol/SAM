//
// Created by Amir Masoud Abdol on 2019-03-03.
//

#include "GroupData.h"

namespace sam {

void GroupData::updateStats() {
    
    /// TODO: Check if is_stats_updated, if so, don't recalculate
    
    nobs_ = measurements_.size();
    mean_ = arma::mean(measurements_);
    var_ = arma::var(measurements_);
    stddev_ = arma::stddev(measurements_);
    ses_ = sqrt( var_ / nobs_ );
}

}
