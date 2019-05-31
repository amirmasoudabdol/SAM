//
// Created by Amir Masoud Abdol on 2019-01-22.
//

#include <stdexcept>

#include <utils/magic_enum.hpp>

#include "ExperimentSetup.h"
#include "Utilities.h"

using namespace sam;

ExperimentSetupBuilder ExperimentSetup::create() {
    return ExperimentSetupBuilder();
}

arma::Mat<double>
ExperimentSetup::constructCovMatrix(double var, double cov) const {
    arma::Row<double> vars(ng_);
    vars.fill(var);
    return constructCovMatrix(vars, cov);
}

arma::Mat<double>
ExperimentSetup::constructCovMatrix(const arma::Row<double> &vars, double cov) const {
    arma::Mat<double> cov_matrix(ng_, ng_);
    
    cov_matrix.fill(cov);
    cov_matrix.diag() = vars;
    
    return cov_matrix;
}

ExperimentSetup::ExperimentSetup(json& config) {
    
    using namespace magic_enum;
    
    // Setting the seed for number of observation
    rng_stream = new RandomNumberGenerator(rand());
    
    auto data_model =  enum_cast<DataStrategy::DataModel>(config["data-strategy"].get<std::string>());
    if (data_model.has_value()) {
        dsp_.name = data_model.value();
    }
    
    nc_ = config["n-conditions"];
    nd_ = config["n-dep-vars"];
    ni_ = config["n-items"];
    ng_ = nc_ * nd_;
    nrows_ = ng_ * ni_;
        
    nobs_ = get_expr_setup_params<int>(config["n-obs"], ng_);
    means_ = get_expr_setup_params<double>(config["means"], ng_);
    vars_ = get_expr_setup_params<double>(config["vars"], ng_);
    loadings_ = get_expr_setup_params<double>(config["loadings"], ni_);
    error_means_ = get_expr_setup_params<double>(config["err-means"], nrows_);
    error_vars_ = get_expr_setup_params<double>(config["err-vars"], nrows_);
    
    if (config["covs"].is_array()){
        if (config["covs"].size() != ng_){
            throw std::invalid_argument( "Size of covs does not match the size of the experiment.");
        }
        // DOC: Notify the user that the `sds` will be discarded.
        auto sigma = config["covs"].get<std::vector<std::vector<double>>>();
        for (int i = 0; i < sigma.size(); i++) {
            sigma_.row(i) = arma::rowvec(sigma[i]);
        }
        vars_ = sigma_.diag().t();
    }else if (config["covs"].is_number()){
        // Broadcase the --covs to the a matrix, and replace the diagonal values with
        // the value already given by --vars.
        double cov = config["covs"];

        sigma_.zeros(ng_, ng_);
        sigma_.fill(cov);
        sigma_.diag() = vars_;
    }else{
        throw std::invalid_argument("covs is invalid or not provided.");
    }

    // Error's Covariant Matrix
    if (config["err-covs"].is_array()){
        if (config["err-covs"].size() != nrows_){
            throw std::invalid_argument( "Size of --err-covs does not match the size of the experiment.");
        }
        auto covs = config["err-covs"].get<std::vector<std::vector<double>>>();
        for (int i = 0; i < covs.size(); i++) {
            error_sigma_.row(i) = arma::rowvec(covs[i]);
        }
        
    }else if (config["err-covs"].is_number()){
        // Broadcase the --err-covs to the a matrix, and replace the diagonal values with
        // the value already given by --vars.
        double cov = config["err-covs"];

        error_sigma_.zeros(nrows_, nrows_);
        error_sigma_.fill(cov);
        error_sigma_.diag() = error_vars_;
    }else{
        throw std::invalid_argument("err-covs is invalid or not provided.");
    }
    
}

void ExperimentSetup::randomize_nObs() {
//    nobs = RNGEngine->genSampleSize(0.75, 20, 100, 300);
    int n = rng_stream->genSampleSize(intervals, weights);
    std::fill(nobs_.begin(), nobs_.end(), n);
}



ExperimentSetupBuilder& ExperimentSetupBuilder::fromConfigFile(json &config) {

    // Setting the seed for number of observation
    setup.rng_stream = new RandomNumberGenerator(rand());
    
    setup.dsp_.name = config["data-strategy"];
    
    setup.nc_ = config["n-conditions"];
    setup.nd_ = config["n-dep-vars"];
    setup.ni_ = config["n-items"];
    setup.ng_ = setup.nc_ * setup.nd_;
    setup.nrows_ = setup.ng_ * setup.ni_;
    
    setup.nobs_ = get_expr_setup_params<int>(config["n-obs"], setup.ng_);
    setup.means_ = get_expr_setup_params<double>(config["means"], setup.ng_);
    setup.vars_ = get_expr_setup_params<double>(config["vars"], setup.ng_);
    setup.loadings_ = get_expr_setup_params<double>(config["loadings"], setup.ni_);
    setup.error_means_ = get_expr_setup_params<double>(config["err-means"], setup.nrows_);
    setup.error_vars_ = get_expr_setup_params<double>(config["err-vars"], setup.nrows_);
    
    if (config["covs"].is_array()){
        if (config["covs"].size() != setup.ng_){
            throw std::invalid_argument( "Size of covs does not match the size of the experiment.");
        }
        // DOC: Notify the user that the `sds` will be discarded.
        auto sigma = config["covs"].get<std::vector<std::vector<double>>>();
        for (int i = 0; i < sigma.size(); i++) {
            setup.sigma_.row(i) = arma::rowvec(sigma[i]);
        }
    }else if (config["covs"].is_number()){
        // Broadcase the covs to the a matrix, and replace the diagonal values with
        // the value already given by vars.
        double cov = config["covs"];

        setup.sigma_.zeros(setup.ng_, setup.ng_);
        setup.sigma_.fill(cov);
        setup.sigma_.diag() = setup.vars_;
    }else{
        throw std::invalid_argument("covs is invalid or not provided.");
    }

    // Error's Covariant Matrix
    if (config["err-covs"].is_array()){
        if (config["err-covs"].size() != setup.nrows_){
            throw std::invalid_argument( "Size of err-covs does not match the size of the experiment.");
        }
        auto covs = config["err-covs"].get<std::vector<std::vector<double>>>();
        for (int i = 0; i < covs.size(); i++) {
            setup.error_sigma_.row(i) = arma::rowvec(covs[i]);
        }
        
    }else if (config["err-covs"].is_number()){
        // Broadcase the err-covs to the a matrix, and replace the diagonal values with
        // the value already given by vars.
        double cov = config["err-covs"];

        setup.error_sigma_.zeros(setup.nrows_, setup.nrows_);
        setup.error_sigma_.fill(cov);
        setup.error_sigma_.diag() = setup.error_vars_;
    }else{
        throw std::invalid_argument("err-covs is invalid or not provided.");
    }

    return *this;
    
}
