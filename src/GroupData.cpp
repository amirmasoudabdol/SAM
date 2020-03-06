//
// Created by Amir Masoud Abdol on 2019-03-03.
//

#include "GroupData.h"

namespace sam {

void GroupData::updateStats() {
    nobs_ = measurements_.size();
    mean_ = arma::mean(measurements_);
    var_ = arma::var(measurements_);
    stddev_ = arma::stddev(measurements_);
    ses_ = sqrt( var_ / nobs_ );
}

}
