//
// Created by Amir Masoud Abdol on 2019-01-22.
//

#include <stdexcept>
#include <armadillo>

#include "ExperimentSetup.h"

using namespace sam;

std::ostream& operator<<(std::ostream& os, ExperimentType et)
{
    switch(et)
    {
        case ExperimentType::LinearModel:
            os << "Fixed Model";
            break;
        case ExperimentType::LatentModel:
            os << "Latent Model";
            break;
        default:
            os.setstate(std::ios_base::failbit);
    }
    return os;
}

arma::Mat<double>
ExperimentSetup::constructCovMatrix(double var, double cov) {
    arma::Row<double> vars(ng_);
    vars.fill(var);
    return constructCovMatrix(vars, cov);
}

arma::Mat<double>
ExperimentSetup::constructCovMatrix(arma::Row<double> vars, double cov) {
    arma::Mat<double> cov_matrix(ng_, ng_);
    
    cov_matrix.fill(cov);
    cov_matrix.diag() = vars;
    
    return cov_matrix;
}

ExperimentSetup::ExperimentSetup(json& config) {
    
    // Setting the seed for number of observation
    rng_stream = new RandomNumberGenerator(rand());
    
        
    if (config["data-strategy"].is_null()){
        throw std::invalid_argument("The Data Strategy parameter is not provided. Check the documentation for more info.");
    }else if (stringToExperimentType.find(config["data-strategy"]) == stringToExperimentType.end()){
        throw std::invalid_argument("Unknown Data Strategy.");
    }
    experiment_type = stringToExperimentType.find(config["data-strategy"])->second;
    
    nc_ = config["n-conditions"];
    nd_ = config["n-dep-vars"];
    ni_ = config["n-items"];
    ng_ = nc_ * nd_;
    nrows_ = ng_ * ni_;
    
    // initializeMemory();
    
     nobs_.resize(ng_);
    if (config["n-obs"].is_array() && config["n-obs"][0].is_array()){
        intervals = config["n-obs"][0].get<std::vector<double>>();
        weights = config["n-obs"][1].get<std::vector<double>>();
        int nobs = rng_stream->genSampleSize(intervals, weights);
        std::fill(nobs_.begin(), nobs_.end(), nobs);
        is_n_randomized = true;
//    }else if (config["n-obs"] == "random"){
//        isNRandomized = true;
//        int nobs = RNGEngine->genSampleSize(0.75, 20, 100, 300);
//        std::fill(nob_.begin(), nob_.end(), nobs);
//
//        config["n-obs"] = nobs;
    }else{
        if (config["n-obs"].is_array()){
            if (config["n-obs"].size() != ng_){
                throw std::invalid_argument( "Size of --n-obs does not match the size of the experiment.");
            }
            nobs_ = config["n-obs"].get<std::vector<int>>();
        }else if (config["n-obs"].is_number()){
            // Broadcase the given --n-obs to a vector of length `ng`
            nobs_ = std::vector<int>(ng_, config["n-obs"]);
        }
    }

    
    if (config["means"].is_array()){
        if (config["means"].size() != ng_){
            throw std::invalid_argument( "Size of --means does not match the size of the experiment.");
        }
        means_ = config["means"].get<std::vector<double>>();
    }else if (config["means"].is_number()){
        // Broadcase the given --means to a vector of length `ng`
        means_ = std::vector<double>(ng_, config["means"]);
    }else{
        throw std::invalid_argument("means is invalid or not provided.");
    }

    std::cout << means_ << std::endl;
    
    if (config["vars"].is_array()){
        if (config["vars"].size() != ng_){
            throw std::invalid_argument( "Size of --vars does not match the size of the experiment.");
        }
        vars_ = config["vars"].get<std::vector<double>>();
    }else if (config["vars"].is_number()){
        // Broadcast the given --vars to a vector of length `ng`
        vars_ = std::vector<double>(ng_, config["vars"]);
    }else{
        throw std::invalid_argument("vars is invalid or not provided.");
    }
    // std::cout << var_;
    

    
    if (config["covs"].is_array()){
        if (config["covs"].size() != ng_){
            throw std::invalid_argument( "Size of --covs does not match the size of the experiment.");
        }
        // DOC: Notify the user that the `sds` will be discarded.
        auto sigma = config["covs"].get<std::vector<std::vector<double>>>();
        for (int i = 0; i < sigma.size(); i++) {
            sigma_.row(i) = arma::rowvec(sigma[i]);
        }
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
//    }
    // std::cout << sigm_ << std::endl;

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
    
    // TODO: Maybe add me to the config file
    // true_parameters["error_means"].resize(nrows_);
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
    // std::cout << errorVars << std::endl;
    
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
//        for (int r = 0; r < nrows; r++) {
//            errorCov.push_back(std::vector<double>(nrows, cov));
//            errorCov[r][r] = errorVars[r];
//        }
        error_sigma_.zeros(nrows_, nrows_);
        error_sigma_.fill(cov);
        error_sigma_.diag() = error_vars_;
    }else{
        throw std::invalid_argument("err-covs is invalid or not provided.");
    }
    // std::cout << errorCov << std::endl;
    
}

void ExperimentSetup::randomize_nObs() {
//    nobs = RNGEngine->genSampleSize(0.75, 20, 100, 300);
    int n = rng_stream->genSampleSize(intervals, weights);
    std::fill(nobs_.begin(), nobs_.end(), n);
}

