//
// Created by Amir Masoud Abdol on 15/01/2021.
//

#ifndef SAMPP_SAMPLE_EXPERIMENT_SETUP_H
#define SAMPP_SAMPLE_EXPERIMENT_SETUP_H

struct ExperimentSetupSampleConfigs {
  
  json sample_experiment_setup = R"(
  {
      "experiment_parameters": {
          "data_strategy": {
              "name": "LinearModel",
              "measurements": {
                  "dist": "mvnorm_distribution",
                  "means": [
                      0.0,
                      0.0,
                      0.8500000000000001,
                      0.8500000000000001
                  ],
                  "sigma": [
                      [
                          1.0,
                          0.5,
                          0.0,
                          0.0
                      ],
                      [
                          0.5,
                          1.0,
                          0.0,
                          0.0
                      ],
                      [
                          0.0,
                          0.0,
                          1.0,
                          0.5
                      ],
                      [
                          0.0,
                          0.0,
                          0.5,
                          1.0
                      ]
                  ]
              }
          },
          "effect_strategy": {
              "name": "StandardizedMeanDifference"
          },
          "n_conditions": 2,
          "n_dep_vars": 2,
          "n_obs": [10, 10, 10, 10],
          "n_reps": 5,
          "test_strategy": {
              "name": "TTest",
              "alpha": 0.05,
              "alternative": "TwoSided",
              "var_equal": true
          }
      }
    })"_json;

};

#endif // SAMPP_SAMPLE_EXPERIMENT_SETUP_H
