//
// Created by Amir Masoud Abdol on 2019-01-22.
//

#include <DataGenStrategy.h>
#include <iostream>
#include "gsl/gsl_statistics.h"

#include "Utilities.h"
#include "Experiment.h"

void FixedEffectStrategy::genData(Experiment* experiment)  {
    // TODO: This can actually call `genNewObservationForAllGroups`
    if (!experiment->setup.isMultivariate){
        experiment->measurements = this->rngEngine.normal(experiment->setup.true_means, experiment->setup.true_vars, experiment->setup.nobs);
    }else{
        experiment->measurements = this->rngEngine.mvnorm(experiment->setup.true_means, experiment->setup.true_sigma, experiment->setup.nobs);
    }
}

std::vector<std::vector<double>> FixedEffectStrategy::genNewObservationsForAllGroups(Experiment* experiment, int n_new_obs) {
    // I can technically add the data here, or let the hacking method decide if he is happy and wants to add them or not
    if (!experiment->setup.isMultivariate){
        return this->rngEngine.normal(experiment->setup.true_means, experiment->setup.true_vars, n_new_obs);
    }else{
        return this->rngEngine.mvnorm(experiment->setup.true_means, experiment->setup.true_sigma, n_new_obs);
    }
}

std::vector<double> FixedEffectStrategy::genNewObservationsFor(Experiment* experiment, int g, int n_new_obs) {

    return std::vector<double>();
}


// LatentDataStrategy

// This is different from other genData, since it should generate items, but will
// only return the means!
void LatentDataStrategy::genData(Experiment* experiment)  {

//    std::cout << "LatentModel, GenData Started!\n";

//    std::vector<std::vector<double> > items;        // nrows * nobs
//    std::vector<std::vector<double> > meaurements;  // ng * nobs
//    std::vector<double> means;                      // 1 * ng
    
    // Measurement here is technically the mean of each row, it'll be
    // ng * nobs
    
    
    
    int ni = experiment->setup.ni;
    int nc = experiment->setup.nc;
    int nd = experiment->setup.nd;
    int ng = experiment->setup.ng;
    int nrows = experiment->setup.nrows;       // Total number of rows
    
    int nobs = experiment->setup.nobs;
    
    // This is correct, you have one a for each item.
    gsl_vector* lambda = gsl_vector_alloc(ni);
    lambda->data = experiment->setup.factorLoadings.data();
    
    // DV ---------------------------------------------
    // Mean of each dv
    gsl_vector* dvMeans = gsl_vector_calloc(nd);    // this needs to be broadcasted to ng * ng
    dvMeans->data = experiment->setup.factorMeans.data();
    
    gsl_matrix* dvSigma = gsl_matrix_alloc(nd, nd);     // this needs to be broadcasted to ng * ng
    gsl_matrix_set_identity(dvSigma);                   // Set to 1 diagonally
    dvSigma->data = flatten(experiment->setup.factorCov).data();
    // ------------------------------------------------
    
    gsl_matrix* factorScores = gsl_matrix_calloc(nd, nobs);
    
    // Generating factor values
    this->rngEngine.mvnorm_n(dvMeans, dvSigma, factorScores);
    
    gsl_vector* allErrorMeans = gsl_vector_calloc(nrows);
    
    gsl_matrix* allErrorsSigma = gsl_matrix_calloc(nrows, nrows);
    allErrorsSigma->data = flatten(experiment->setup.errorCov).data();
    
    gsl_matrix* allErrors = gsl_matrix_calloc(nrows, nobs);
    
    // Generating errors
    this->rngEngine.mvnorm_n(allErrorMeans, allErrorsSigma, allErrors);
    
    gsl_matrix* allScores = gsl_matrix_calloc(nrows, nobs);
    
    int row = 0;
    int col = 0;
    
    for (int c = 0; c < nc; c++) {

        for (int d = 0 ; d < nd; d++) {
            row = c * nd + d;
            for (int n = 0; n < nobs; n++){
                col = n;
                for (int i = 0; i < ni; i++) {
                    gsl_matrix_set(allScores, row * ni + i, col,
                                       gsl_vector_get(dvMeans, d) +
                                       gsl_matrix_get(factorScores, d, col) * gsl_vector_get(lambda, i) +
                                       gsl_matrix_get(allErrors, row * ni + i, col)
                                   );
                    
                    // I can compute mean of each row (of items) here and put it into `measurements` if necessary
                    // item[i][j] = 
                }
            }
        }
    }
    
    gsl_vector* scoreRow = gsl_vector_alloc(nobs);
    for (int r = 0; r < nrows; r++) {
        gsl_matrix_get_row(scoreRow, allScores, r);
        experiment->items.push_back(std::vector<double>(scoreRow->data, scoreRow->data + scoreRow->size));
    }
    
//    gsl_vector* itemMeans = gsl_vector_calloc(nrows);
//    gsl_vector* tmpRow = gsl_vector_alloc(nobs);
//    for (int r = 0; r < nrows; r++) {
//        gsl_matrix_get_row(tmpRow, allScores, r);
//        gsl_vector_set(itemMeans, r, gsl_stats_mean(tmpRow->data, 1, nobs));
////        std::cout << gsl_vector_get(itemMeans, n) << std::endl;
//    }

    for (int g = 0; g < ng; g++) {
        experiment->measurements.push_back(std::vector<double>(nobs));
    }
    
    int rowOffset = 0;
    gsl_vector* v = gsl_vector_alloc(nrows);
        
    for (int n = 0; n < nobs; n++) {
        gsl_matrix_get_col(v, allScores, n);
            
        for (int c = 0; c < nc; c++) {
                
            for (int d = 0; d < nd; d++) {
                rowOffset = c * nd + d;
            
                gsl_vector_view vsub = gsl_vector_subvector(v, rowOffset * ni, ni);
                
                experiment->measurements[c * nd + d][n] = gsl_stats_mean(vsub.vector.data, 1, ni);
                            
            }
        }
        
    }

    gsl_vector_free(lambda);
    gsl_vector_free(dvMeans);
    gsl_vector_free(allErrorMeans);
    gsl_vector_free(scoreRow);
    gsl_vector_free(v);

    gsl_matrix_free(dvSigma);
    gsl_matrix_free(factorScores);
    gsl_matrix_free(allErrorsSigma);
    gsl_matrix_free(allErrors);
    gsl_matrix_free(allScores);
    
    // return meaurements;
}

std::vector<std::vector<double>> LatentDataStrategy::genNewObservationsForAllGroups(Experiment* experiment, int n_new_obs) {
    // if (!experiment->setup.isMultivariate){
    //     return this->rngEngine.normal(experiment->setup.true_means, experiment->setup.true_vars, n_new_obs);
    // }else{
        return this->rngEngine.mvnorm(experiment->setup.factorMeans, experiment->setup.factorCov, n_new_obs);
    // }
}

std::vector<double> LatentDataStrategy::genNewObservationsFor(Experiment* experiment, int g, int n_new_obs) {
    
    return std::vector<double>();
}
