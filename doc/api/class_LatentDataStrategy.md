---
date: Fri Mar 15 2019
parent: API
section: 3
title: LatentDataStrategy
---

NAME
====

LatentDataStrategy

SYNOPSIS
========

\

`#include <DataGenStrategy.h>`

Inherits **DataGenStrategy**.

Public Member Functions
-----------------------

**LatentDataStrategy** (**ExperimentSetup** &setup)\

void **genData** (**Experiment** \*experiment)\

std::vector\< arma::Row\< double \> \>
**genNewObservationsForAllGroups** (**Experiment** \*experiment, int
n\_new\_obs)\

arma::Row\< double \> **genNewObservationsFor** (**Experiment**
\*experiment, int g, int n\_new\_obs)\

Additional Inherited Members
----------------------------

Detailed Description
====================

A Data Strategy for constructing a general `Structural Equaiton Model`.

**Note:**

> **LatentDataStrategy** will generate individual items, therefore it
> might be slower than other models.

Member Function Documentation
=============================

void LatentDataStrategy::genData (**Experiment** \* experiment)` [virtual]`
---------------------------------------------------------------------------

Construct a structural equation model based on the given paramters
specified in the ExperimentSteup. Beside `experiment->measurements`,
**LatentDataStrategy** populates the `experiment->items` as well.

TODO: Link to the illustration that explain the model

Implements **DataGenStrategy**.

Author
======

Generated automatically by Doxygen for SAMpp from the source code.
