---
date: Fri Mar 15 2019
parent: API
section: 3
title: ExperimentSetup
---

NAME
====

ExperimentSetup

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

int **nd** = 3\

int **ni** = 0\

int **ng**\

int **nrows**\

int **nobs**\

bool **isNRandomized** = false\

double **alpha** = 0.05\

arma::Row\< int \> **true\_nobs**\

arma::Row\< double \> **true\_means**\

arma::Row\< double \> **true\_vars**\

double **cov** = 0\

arma::Mat\< double \> **true\_sigma**\

arma::Row\< double \> **factorLoadings**\

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
