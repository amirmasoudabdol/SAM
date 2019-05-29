//
// Created by Amir Masoud Abdol on 2019-01-22.
//

#include <stdexcept>

#include <utils/magic_enum.hpp>

#include "ExperimentSetup.h"

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
        
     nobs_.resize(ng_);
    if (config["n-obs"].is_array() && config["n-obs"][0].is_array()){
        intervals = config["n-obs"][0].get<std::vector<double>>();
        weights = config["n-obs"][1].get<std::vector<double>>();
        int nobs = rng_stream->genSampleSize(intervals, weights);
        std::fill(nobs_.begin(), nobs_.end(), nobs);
        is_n_randomized = true;
    }else{
        if (config["n-obs"].is_array()){
            if (config["n-obs"].size() != ng_){
                throw std::invalid_argument( "Size of n-obs does not match the size of the experiment.");
            }
            nobs_ = config["n-obs"].get<std::vector<int>>();
        }else if (config["n-obs"].is_number()){
            // Broadcase the given n-obs to a vector of length `ng`
            nobs_ = std::vector<int>(ng_, config["n-obs"]);
        }
    }

    // get_expr_setup_params<int>(config["n-obs"], ng_);


    
    if (config["means"].is_array()){
        if (config["means"].size() != ng_){
            throw std::invalid_argument( "Size of means does not match the size of the experiment.");
        }
        means_ = config["means"].get<std::vector<double>>();
    }else if (config["means"].is_number()){
        // Broadcase the given means to a vector of length `ng`
        means_ = std::vector<double>(ng_, config["means"]);
    }else{
        throw std::invalid_argument("means is invalid or not provided.");
    }
    
    if (config["vars"].is_array()){
        if (config["vars"].size() != ng_){
            throw std::invalid_argument( "Size of vars does not match the size of the experiment.");
        }
        vars_ = config["vars"].get<std::vector<double>>();
    }else if (config["vars"].is_number()){
        // Broadcast the given vars to a vector of length `ng`
        vars_ = std::vector<double>(ng_, config["vars"]);
    }else{
        throw std::invalid_argument("vars is invalid or not provided.");
    }    

    
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

    // Factor Loading ...
    // CHECK: I think there are `ni` of these,
    // CHECK: I think they should be `nrows`
    if (config["loadings"].is_array()){
        if (config["loadings"].size() != ni_){
            throw std::invalid_argument( "Size of --loadings does not match the size of the experiment.");
        }
        loadings_ = config["loadings"].get<std::vector<double>>();
    }else if (config["loadings"].is_number()){
        // Broadcast the given --loadings to a vector of length `ng`
        loadings_ = std::vector<double>(ni_, config["loadings"]);
    }else{
        throw std::invalid_argument("loadings is invalid or not provided.");
    }
    
    // this is always zero anyway
    error_means_.fill(0);
    
    // Error's Standard Deviations
    if (config["err-vars"].is_array()){
        if (config["err-vars"].size() != nrows_){
            throw std::invalid_argument( "Size of --err-vars does not match the size of the experiment.");
        }
        error_vars_ = config["err-vars"].get<std::vector<double>>();
    }else if (config["err-vars"].is_number()){
        // Broadcast the given --err-vars to a vector of length `nrows`
        error_vars_ = std::vector<double>(nrows_, config["err-vars"]);
    }else{
        throw std::invalid_argument("err-vars is invalid or not provided.");
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
        
     setup.nobs_.resize(setup.ng_);
    if (config["n-obs"].is_array() && config["n-obs"][0].is_array()){
        setup.intervals = config["n-obs"][0].get<std::vector<double>>();
        setup.weights = config["n-obs"][1].get<std::vector<double>>();
        int nobs = setup.rng_stream->genSampleSize(setup.intervals, setup.weights);
        std::fill(setup.nobs_.begin(), setup.nobs_.end(), nobs);
        setup.is_n_randomized = true;
    }else{
        if (config["n-obs"].is_array()){
            if (config["n-obs"].size() != setup.ng_){
                throw std::invalid_argument( "Size of n-obs does not match the size of the experiment.");
            }
            setup.nobs_ = config["n-obs"].get<std::vector<int>>();
        }else if (config["n-obs"].is_number()){
            // Broadcase the given n-obs to a vector of length `ng`
            setup.nobs_ = std::vector<int>(setup.ng_, config["n-obs"]);
        }
    }

    
    if (config["means"].is_array()){
        if (config["means"].size() != setup.ng_){
            throw std::invalid_argument( "Size of means does not match the size of the experiment.");
        }
        setup.means_ = config["means"].get<std::vector<double>>();
    }else if (config["means"].is_number()){
        // Broadcase the given means to a vector of length `ng`
        setup.means_ = std::vector<double>(setup.ng_, config["means"]);
    }else{
        throw std::invalid_argument("means is invalid or not provided.");
    }
    
    if (config["vars"].is_array()){
        if (config["vars"].size() != setup.ng_){
            throw std::invalid_argument( "Size of vars does not match the size of the experiment.");
        }
        setup.vars_ = config["vars"].get<std::vector<double>>();
    }else if (config["vars"].is_number()){
        // Broadcast the given vars to a vector of length `ng`
        setup.vars_ = std::vector<double>(setup.ng_, config["vars"]);
    }else{
        throw std::invalid_argument("vars is invalid or not provided.");
    }    

    
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

    // Factor Loading ...
    // CHECK: I think there are `ni` of these,
    // CHECK: I think they should be `nrows`
    if (config["loadings"].is_array()){
        if (config["loadings"].size() != setup.ni_){
            throw std::invalid_argument( "Size of loadings does not match the size of the experiment.");
        }
        setup.loadings_ = config["loadings"].get<std::vector<double>>();
    }else if (config["loadings"].is_number()){
        // Broadcast the given loadings to a vector of length `ng`
        setup.loadings_ = std::vector<double>(setup.ni_, config["loadings"]);
    }else{
        throw std::invalid_argument("loadings is invalid or not provided.");
    }
    
    // this is always zero anyway
    setup.error_means_.fill(0);
    
    // Error's Standard Deviations
    if (config["err-vars"].is_array()){
        if (config["err-vars"].size() != setup.nrows_){
            throw std::invalid_argument( "Size of err-vars does not match the size of the experiment.");
        }
        setup.error_vars_ = config["err-vars"].get<std::vector<double>>();
    }else if (config["err-vars"].is_number()){
        // Broadcast the given err-vars to a vector of length `nrows`
        setup.error_vars_ = std::vector<double>(setup.nrows_, config["err-vars"]);
    }else{
        throw std::invalid_argument("err-vars is invalid or not provided.");
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
