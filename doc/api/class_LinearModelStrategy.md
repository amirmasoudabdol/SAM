---
date: Fri Mar 15 2019
section: 3
title: LinearModelStrategy
---

NAME
====

LinearModelStrategy

SYNOPSIS
========

\

`#include <DataGenStrategy.h>`

Inherits **DataGenStrategy**.

Public Member Functions
-----------------------

**LinearModelStrategy** (**ExperimentSetup** &setup)\

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

The fixed-effect data strategy will produce data from a fixed-effect
model with the given \$\$ and \$sigma\$.

Author
======

Generated automatically by Doxygen for SAMpp from the source code.
