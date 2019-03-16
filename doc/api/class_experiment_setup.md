---
parent: API
---

ExperimentSetup Class Reference
===============================

ExperimentSetup

Define a class for ExperimentSetup.

`#include <ExperimentSetup.h>`

-   **ExperimentSetup** (json &config)

-   void **setNObs** (int n\_obs)

-   void **setNObs** (std::vector\< int \> &n\_obs\_v)

-   void **randomize\_nObs** ()

<!-- -->

-   ExperimentType **experimentType**

-   TestMethod **testMethod**

-   int nc = 1

    Number of experimental conditions, e.g., treatment 1, treatment 2.

-   int nd = 3

    Number of *dependent variables* in each experimental condition.

-   int ni = 0

    Number of items for each latent variable, if `isFactorModel` is
    `true`.

-   int ng

    Total number of groups Always calculated as n\_g = n\_c \* n\_d,
    unless the simulation contains latent variables, n\_g = n\_c \*
    n\_d \* n\_i.

-   int **nrows**

-   int nobs

    Number of observations in each group.

-   bool **isNRandomized** = false

-   double **alpha** = 0.05

-   arma::Row\< int \> **true\_nobs**

-   arma::Row\< double \> **true\_means**

-   arma::Row\< double \> **true\_vars**

-   double **cov** = 0

-   arma::Mat\< double \> **true\_sigma**

-   arma::Row\< double \> factorLoadings

    \\lambda

-   arma::Row\< double \> **errorMeans**

-   arma::Row\< double \> **errorVars**

-   arma::Mat\< double \> **errorCov**

Detailed Description
--------------------

Define a class for ExperimentSetup.

ExperimentSetup contains the necessary parameters for initiating and
generating the data needed for the Experiment.

The documentation for this class was generated from the following files:

include/ExperimentSetup.h

src/ExperimentSetup.cpp
