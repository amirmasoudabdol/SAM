//
// Created by Amir Masoud Abdol on 2019-01-22.
//

#include <iostream>
#include <string>

#include "csv/reader.hpp"

#include "Experiment.h"
#include "ExperimentSetup.h"
#include "DataStrategy.h"

using namespace sam;

DataStrategy::~DataStrategy() {
    // Pure deconstructor
}

std::unique_ptr<DataStrategy> DataStrategy::build(json &data_strategy_config) {
    
    if (data_strategy_config["_name"] == "LinearModel"){
        auto params = data_strategy_config.get<LinearModelStrategy::Parameters>();
        return std::make_unique<LinearModelStrategy>(params);
        
    }else if (data_strategy_config["_name"] == "LatentModel") {
        return std::make_unique<LatentDataStrategy>();
        
    }else if (data_strategy_config["_name"] == "GradedResponseModel") {
        auto params = data_strategy_config.get<GRMDataStrategy::Parameters>();
        return std::make_unique<GRMDataStrategy>(params);
        
    }else{
        throw std::invalid_argument("Unknown Data Strategy.");
    }

}


void LinearModelStrategy::genData(Experiment* experiment)  {
    
    arma::mat sample(experiment->setup.ng(), experiment->setup.nobs().max());
    
    // Refactor Me!
    if (params.meas_dist){  // Multivariate Distributions
        sample.each_col([this](arma::vec &v){v = Random::get(this->params.meas_dist.value());});
    }else{
        if (params.meas_dists) {
            
            sample.each_row([this, i = 0](arma::rowvec &v) mutable {
                v.imbue([&](){return Random::get(this->params.meas_dists.value()[i]);});
                i++;
            });
        }
    }
        
    // Adding some noise if specified
    if (params.erro_dist) { // Multivariate Distributions
        arma::mat errors(experiment->setup.ng(), experiment->setup.nobs().max());
        errors.each_col([this](arma::vec &v){v = Random::get(params.erro_dist.value());});
        sample += errors;
    }else{
        if (params.erro_dists) {
            arma::mat errors(experiment->setup.ng(), experiment->setup.nobs().max());
            
            errors.each_row([this, i = 0](arma::rowvec &v) mutable {
                v.imbue([&](){return Random::get(this->params.erro_dists.value()[i]);});
                i++;
            });
            sample += errors;
        }
    }
    
    std::generate(experiment->measurements.begin(), experiment->measurements.end(),
                  [sample, i = 0]() mutable {return sample.row(i++);});
        
}

std::vector<arma::Row<double>>
LinearModelStrategy::genNewObservationsForAllGroups(Experiment* experiment, int n_new_obs) {
    
    arma::mat sample(experiment->setup.ng(), n_new_obs);
    
    if (params.meas_dist){  // Multivariate Distributions
        sample.each_col([this](arma::vec &v){v = Random::get(this->params.meas_dist.value());});
    }else{
        if (params.meas_dists) {
            
            sample.each_row([this, i = 0](arma::rowvec &v) mutable {
                v.imbue([&](){return Random::get(this->params.meas_dists.value()[i]);});
                i++;
            });
        }
    }
        
    // Adding some noise if specified
    if (params.erro_dist) { // Multivariate Distributions
        arma::mat errors(experiment->setup.ng(), experiment->setup.nobs().max());
        errors.each_col([this](arma::vec &v){v = Random::get(params.erro_dist.value());});
        sample += errors;
    }else{
        if (params.erro_dists) {
            arma::mat errors(experiment->setup.ng(), experiment->setup.nobs().max());
            
            errors.each_row([this, i = 0](arma::rowvec &v) mutable {
                v.imbue([&](){return Random::get(this->params.erro_dists.value()[i]);});
                i++;
            });
            sample += errors;
        }
    }
    
    std::vector<arma::Row<double>> new_values(experiment->setup.ng());
    
    // Todo: Similary I think i can use something better than the std::generate
    std::generate(new_values.begin(), new_values.end(),
                  [sample, i = 0]() mutable {return sample.row(i++);});
    
    return new_values;
}

arma::Row<double>
LinearModelStrategy::genNewObservationsFor(Experiment* experiment, int g, int n_new_obs) {
    // TODO: To be implemented still...
    return arma::Row<double>();
}


// LatentDataStrategy


/**
 Construct a structural equation model based on the given paramters specified in
 the ExperimentSteup.
 Beside `experiment->measurements`, LatentDataStrategy populates the `experiment->items` as well.
 
 TODO: Link to the illustration that explain the model

 */
void LatentDataStrategy::genData(Experiment *experiment)  {

//   int ni = experiment->setup.ni_;
//   int nc = experiment->setup.nc_;
//   int nd = experiment->setup.nd_;
//   int ng = experiment->setup.ng_;                  // nc * nd
//   int nrows = experiment->setup.nrows_;            // nc * nd * ni


   // TODO: Check Me!
//   int nobs = experiment->setup.getTrueValueOf("nobs.ma")();
//
//    // This is correct, you have one a for each item.
//    gsl_vector* lambda = gsl_vector_alloc(ni);
//    // FIXME: Commented during the migration
////    lambda->data = experiment->setup.factorLoadings.data();
//
//    // DV ---------------------------------------------
//    // Mean of each dv
//    gsl_vector* dvMeans = gsl_vector_calloc(ng);
//    // FIXME: Commented during the migration
////    dvMeans->data = experiment->setup.getTrueValueOf("means.dat")();
//
//    gsl_matrix* dvSigma = gsl_matrix_alloc(ng, ng);
//    // FIXME: Commented during the migration
////    dvSigma->data = flatten(experiment->setup.getTrueValueOf("sigm")).data();
//    // ------------------------------------------------
//
//    gsl_matrix* factorScores = gsl_matrix_calloc(ng, nobs);
//
//    // Generating factor values
//    // FIXME: Commented during the migration
////    this->mainRngStream->mvnorm_n(dvMeans, dvSigma, factorScores);

//   auto factorScores = this->main_rng_stream->mvnorm(experiment->setup.getTrueValueOf("mean"),
//                                                   experiment->setup.getTrueValueOf("sigm"),
//                                                   experiment->setup.getTrueValueOf("nob"));

//
//    gsl_vector* allErrorMeans = gsl_vector_calloc(nrows);
//
//    gsl_matrix* allErrorsSigma = gsl_matrix_calloc(nrows, nrows);
//    // FIXME: Commented during the migration
////    allErrorsSigma->data = flatten(experiment->setup.errorCov).data();
//
//    gsl_matrix* allErrors = gsl_matrix_calloc(nrows, nobs);
//
//    // Generating errors
//    // FIXME: Commented during the migration
////    this->mainRngStream->mvnorm_n(allErrorMeans, allErrorsSigma, allErrors);

//   auto allErrors = this->main_rng_stream->mvnorm(experiment->setup.errorMeans,
//                                                experiment->setup.errorCov,
//                                                nrows);



//
//    gsl_matrix* allScores = gsl_matrix_calloc(nrows, nobs);

//   arma::Mat<double> allScores(nrows, nobs);

   // allScores = experiment->setup.getTrueValueOf("means.)]() +

   //                              experiment->setup.loadings.t() + 
   //                              allErrors

//
//    int row = 0;
//    int col = 0;
//
//    for (int c = 0; c < nc; c++) {
//
//        for (int d = 0 ; d < nd; d++) {
//            row = c * nd + d;
//            for (int n = 0; n < nobs; n++){
//                col = n;
//                for (int i = 0; i < ni; i++) {
//                    gsl_matrix_set(allScores, row * ni + i, col,
//                                       gsl_vector_get(dvMeans, d) +
//                                       gsl_matrix_get(factorScores, d, col) * gsl_vector_get(lambda, i) +
//                                       gsl_matrix_get(allErrors, row * ni + i, col)
//                                   );
//
//                    // I can compute mean of each row (of items) here and put it into `measurements` if necessary
//                    // item[i][j] =
//                }
//            }
//        }
//    }
//
//    gsl_vector* scoreRow = gsl_vector_alloc(nobs);
//    for (int r = 0; r < nrows; r++) {
//        gsl_matrix_get_row(scoreRow, allScores, r);
//        experiment->items.push_back(std::vector<double>(scoreRow->data, scoreRow->data + scoreRow->size));
//    }
//
////    gsl_vector* itemMeans = gsl_vector_calloc(nrows);
////    gsl_vector* tmpRow = gsl_vector_alloc(nobs);
////    for (int r = 0; r < nrows; r++) {
////        gsl_matrix_get_row(tmpRow, allScores, r);
////        gsl_vector_set(itemMeans, r, gsl_stats_mean(tmpRow->data, 1, nobs));
//////        std::cout << gsl_vector_get(itemMeans, n) << std::endl;
////    }
//
//    for (int g = 0; g < ng; g++) {
//        experiment->measurements.push_back(std::vector<double>(nobs));
//    }
//
//    int rowOffset = 0;
//    gsl_vector* v = gsl_vector_alloc(nrows);
//
//    for (int n = 0; n < nobs; n++) {
//        gsl_matrix_get_col(v, allScores, n);
//
//        for (int c = 0; c < nc; c++) {
//
//            for (int d = 0; d < nd; d++) {
//                rowOffset = c * nd + d;
//
//                gsl_vector_view vsub = gsl_vector_subvector(v, rowOffset * ni, ni);
//
//                experiment->measurements[c * nd + d][n] = gsl_stats_mean(vsub.vector.data, 1, ni);
//
//            }
//        }
//
//    }
//
//    gsl_vector_free(lambda);
//    gsl_vector_free(dvMeans);
//    gsl_vector_free(allErrorMeans);
//    gsl_vector_free(scoreRow);
//    gsl_vector_free(v);
//
//    gsl_matrix_free(dvSigma);
//    gsl_matrix_free(factorScores);
//    gsl_matrix_free(allErrorsSigma);
//    gsl_matrix_free(allErrors);
//    gsl_matrix_free(allScores);
    
    // return meaurements;
}

std::vector<arma::Row<double> >
LatentDataStrategy::genNewObservationsForAllGroups(Experiment* experiment, int n_new_obs) {
    // FIXME: Commented during the migration
//    return this->secRngStream->mvnorm(experiment->setup.getTrueValueOf("mean"), experiment->setup.getTrueValueOf("sigm"), n_new_obs);
    return std::vector<arma::Row<double>>();
}

arma::Row<double>
LatentDataStrategy::genNewObservationsFor(Experiment* experiment, int g, int n_new_obs) {
    
    return arma::Row<double>();
}

