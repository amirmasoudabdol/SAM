---
date: Fri Mar 15 2019
parent: API
section: 3
title: ExperimentSetup
---

NAME
====

ExperimentSetup - Define a class for **ExperimentSetup**.

SYNOPSIS
========

\

`#include <ExperimentSetup.h>`

Public Member Functions
-----------------------

**ExperimentSetup** (json &config)\

void **setNObs** (int n\_obs)\

void **setNObs** (std::vector\< int \> &n\_obs\_v)\

void **randomize\_nObs** ()\

Public Attributes
-----------------

ExperimentType **experimentType**\

TestMethod **testMethod**\

int **nc** = 1\
Number of experimental conditions, e.g., treatment 1, treatment 2.

int **nd** = 3\
Number of *dependent variables* in each experimental condition.

int **ni** = 0\
Number of items for each latent variable, if `isFactorModel` is `true`.

int **ng**\
Total number of groups Always calculated as \$n\_g = n\_c \* n\_d\$,
unless the simulation contains latent variables, \$n\_g = n\_c \* n\_d
\* n\_i\$.

int **nrows**\

int **nobs**\
Number of observations in each group.

bool **isNRandomized** = false\

double **alpha** = 0.05\

arma::Row\< int \> **true\_nobs**\

arma::Row\< double \> **true\_means**\

arma::Row\< double \> **true\_vars**\

double **cov** = 0\

arma::Mat\< double \> **true\_sigma**\

arma::Row\< double \> **factorLoadings**\
\$ambda\$

arma::Row\< double \> **errorMeans**\

arma::Row\< double \> **errorVars**\

arma::Mat\< double \> **errorCov**\

Detailed Description
====================

Define a class for **ExperimentSetup**.

**ExperimentSetup** contains the necessary parameters for initiating and
generating the data needed for the **Experiment**.

Author
======

Generated automatically by Doxygen for SAMpp from the source code.
