//
// Created by Amir Masoud Abdol on 2019-01-22.
//

#include <stdexcept>
#include <armadillo>

#include "ExperimentSetup.h"

std::ostream& operator<<(std::ostream& os, ExperimentType et)
{
    switch(et)
    {
        case ExperimentType::LinearModel   : os << "Fixed Model" ;  break;
        case ExperimentType::LatentModel   : os << "Latent Model";  break;
        default    : os.setstate(std::ios_base::failbit);
    }
    return os;
}


ExperimentSetup::ExperimentSetup(json& config) {
    
    int metaSeed;
    if (config["--meta-seed"].is_null() || config["--meta-seed"].get<int>() == 0){
        metaSeed = rand();
    }else{
        // --meta-seed is a number
        metaSeed = config["--meta-seed"];
    }
    RNGEngine = new RandomNumberGenerator(metaSeed);
    
        
    if (config["--data-strategy"].is_null()){
        throw std::invalid_argument("The \"--data-strategy\" parameter is not provided. Check README.md for more info.");
    }else if (stringToExperimentType.find(config["--data-strategy"]) == stringToExperimentType.end()){
        throw std::invalid_argument("Unrecognized \"--data-strategy\". See README.md.");
    }
    experimentType = stringToExperimentType.find(config["--data-strategy"])->second;
    
    nc = config["--n-conditions"];
    nd = config["--n-dep-vars"];
    ni = config["--n-items"];
    ng = nc * nd;
    nrows = ng * ni;

    nobs = config["--n-obs"];
    
    if (config["--n-obs"].is_array()){
        if (config["--n-obs"].size() != ng){
            throw std::invalid_argument( "Size of --n-obs does not match the size of the experiment.");
        }
        true_nobs = config["--n-obs"].get<std::vector<int>>();
    }else if (config["--n-obs"].is_number()){
        // Broadcase the given --n-obs to a vector of length `ng`
        true_nobs = std::vector<int>(ng, config["--n-obs"]);
    }
    if (config["--n-obs"].get<int>() == 0){
        isNRandomized = true;
        nobs = RNGEngine->genSampleSize(0.75, 20, 100, 300);
        std::fill(true_nobs.begin(), true_nobs.end(), nobs);
    }
    
    if (config["--means"].is_array()){
        if (config["--means"].size() != ng){
            throw std::invalid_argument( "Size of --means does not match the size of the experiment.");
        }
        true_means = config["--means"].get<std::vector<double>>();
    }else if (config["--means"].is_number()){
        // Broadcase the given --means to a vector of length `ng`
        true_means = std::vector<double>(ng, config["--means"]);
    }else{
        throw std::invalid_argument("--means is invalid or not provided.");
    }
    
    if (config["--vars"].is_array()){
        if (config["--vars"].size() != ng){
            throw std::invalid_argument( "Size of --vars does not match the size of the experiment.");
        }
        true_vars = config["--vars"].get<std::vector<double>>();
    }else if (config["--vars"].is_number()){
        // Broadcast the given --vars to a vector of length `ng`
        true_vars = std::vector<double>(ng, config["--vars"]);
    }else{
        throw std::invalid_argument("--vars is invalid or not provided.");
    }
    // std::cout << true_vars;
    
    // I can drop this and only check if `cov == 0` and assume that the user wants
    // to run a correlated model
    // TODO: I need to change the if here
//    if (config["--is-correlated"]) {
//        isCorrelated = true;
    
        if (config["--covs"].is_array()){
            if (config["--covs"].size() != ng){
                throw std::invalid_argument( "Size of --covs does not match the size of the experiment.");
            }
            // DOC: Notify the user that the `sds` will be discarded.
            auto sigma = config["--covs"].get<std::vector<std::vector<double>>>();
            for (int i = 0; i < sigma.size(); i++) {
                true_sigma.row(i) = arma::rowvec(sigma[i]);
            }
            
            
        }else if (config["--covs"].is_number()){
            // Broadcase the --covs to the a matrix, and replace the diagonal values with
            // the value already given by --vars.
            double cov = config["--covs"];
//            for (int g = 0; g < ng; g++) {
//                true_sigma.push_back(std::vector<double>(ng, cov));
//                true_sigma[g][g] = true_vars[g];
//            }
            true_sigma.zeros(ng, ng);
            true_sigma.fill(cov);
            true_sigma.diag() = true_vars;
        }else{
            throw std::invalid_argument("--covs is invalid or not provided.");
        }
//    }
    // std::cout << true_sigma << std::endl;

    // Factor Loading ...
    // CHECK: I think there are `ni` of these,
    // CHECK: I think they should be `nrows`
    if (config["--loadings"].is_array()){
        if (config["--loadings"].size() != ni){
            throw std::invalid_argument( "Size of --loadings does not match the size of the experiment.");
        }
        factorLoadings = config["--loadings"].get<std::vector<double>>();
    }else if (config["--loadings"].is_number()){
        // Broadcast the given --loadings to a vector of length `ng`
        factorLoadings = std::vector<double>(ni, config["--loadings"]);
    }else{
        throw std::invalid_argument("--loadings is invalid or not provided.");
    }
    
    // TODO: Maybe add me to the config file
    errorMeans.resize(nrows);
    errorMeans.fill(0);
    
    // Error's Standard Deviations
    if (config["--err-vars"].is_array()){
        if (config["--err-vars"].size() != nrows){
            throw std::invalid_argument( "Size of --err-vars does not match the size of the experiment.");
        }
        errorVars = config["--err-vars"].get<std::vector<double>>();
    }else if (config["--err-vars"].is_number()){
        // Broadcast the given --err-vars to a vector of length `nrows`
        errorVars = std::vector<double>(nrows, config["--err-vars"]);
    }else{
        throw std::invalid_argument("--err-vars is invalid or not provided.");
    }
    // std::cout << errorVars << std::endl;
    
    // Error's Covariant Matrix
    if (config["--err-covs"].is_array()){
        if (config["--err-covs"].size() != nrows){
            throw std::invalid_argument( "Size of --err-covs does not match the size of the experiment.");
        }
        auto covs = config["--err-covs"].get<std::vector<std::vector<double>>>();
        for (int i = 0; i < covs.size(); i++) {
            errorCov.row(i) = arma::rowvec(covs[i]);
        }
        
    }else if (config["--err-covs"].is_number()){
        // Broadcase the --err-covs to the a matrix, and replace the diagonal values with
        // the value already given by --vars.
        double cov = config["--err-covs"];
//        for (int r = 0; r < nrows; r++) {
//            errorCov.push_back(std::vector<double>(nrows, cov));
//            errorCov[r][r] = errorVars[r];
//        }
        errorCov.zeros(nrows, nrows);
        errorCov.fill(cov);
        errorCov.diag() = errorVars;
    }else{
        throw std::invalid_argument("--err-covs is invalid or not provided.");
    }
    // std::cout << errorCov << std::endl;
    
}

void ExperimentSetup::randomize_nObs() {
    nobs = RNGEngine->genSampleSize(0.75, 20, 100, 300);
    std::fill(true_nobs.begin(), true_nobs.end(), nobs);
}

