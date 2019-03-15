---
date: Fri Mar 15 2019
section: 3
title: RandomNumberGenerator
---

NAME
====

RandomNumberGenerator

SYNOPSIS
========

\

Public Member Functions
-----------------------

**RandomNumberGenerator** (int seed)\

void **setSeed** (int seed)\

int **getSeed** ()\

double **uniform** ()\

double **uniform** (const double &min, const double &max)\

int **genSampleSize** (const double &, const double &, const double &,
const double &)\

arma::Row\< double \> **normal** (const double &mean, const double &var,
const double &n)\

std::vector\< arma::Row\< double \> \> **normal** (const arma::Row\<
double \> &means, const arma::Row\< double \> &vars, const int &n)\

std::vector\< arma::Row\< double \> \> **normal** (const arma::Row\<
double \> &means, const arma::Row\< double \> &vars, const arma::Row\<
int \> &nobs)\

std::vector\< arma::Row\< double \> \> **mvnorm** (const arma::Row\<
double \> &means, const arma::Mat\< double \> &sigma, const int &n)\

std::vector\< arma::Row\< double \> \> **mvnorm** (const arma::Row\<
double \> &means, const arma::Mat\< double \> &sigma, const arma::Row\<
int \> &nobs)\

Public Attributes
-----------------

std::random\_device **rd**\

std::mt19937 **gen**\

std::bernoulli\_distribution **bernoulliDist**\

std::uniform\_real\_distribution\< double \> **uniformDist**\

Member Function Documentation
=============================

int RandomNumberGenerator::genSampleSize (const double & p, const double & a, const double & b, const double & c)
-----------------------------------------------------------------------------------------------------------------

Return a value between \$\[a, b)\$ or \$\[b, c)\$ based on the outcome
of a Bernoulli trial with probability of \$p\$. The return value has
\$p\$ chance of being in \$\[a, b)\$ and \$1-p\$ chance of being in
\$\[b, c)\$.

Author
======

Generated automatically by Doxygen for SAMpp from the source code.
