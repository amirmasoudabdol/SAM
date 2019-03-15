---
date: Fri Mar 15 2019
parent: API
section: 3
title: SignigicantSelection
---

NAME
====

SignigicantSelection

SYNOPSIS
========

\

`#include <SelectionStrategies.h>`

Inherits **SelectionStrategy**.

Public Member Functions
-----------------------

**SignigicantSelection** (double alpha, double pub\_bias, int side, int
seed)\

bool **review** (**Submission** &s)\

Public Attributes
-----------------

double **\_alpha**\

double **\_pub\_bias**\

int **\_side**\

int **\_seed**\

**RandomNumberGenerator** \* **mainRngStream**\

Additional Inherited Members
----------------------------

Detailed Description
====================

Significant-based Selection Strategy.

Significant-based selection strategy accepts a publication if the given
*p*-value is significant. Certain degree of *Publication Bias*, can be
specified. In this case, a **Submission** has a chance of being
published even if the statistics is not significant. Moreover, the
SignificantSelection can be tailored toward either positive or negative
effect. In this case, the **Journal** will only accept Submissions with
larger or smaller effects.

Member Function Documentation
=============================

bool SignigicantSelection::review (**Submission** & s)` [virtual]`
------------------------------------------------------------------

Review the given submission based on its *p*-value and its effect side.

Implements **SelectionStrategy**.

Author
======

Generated automatically by Doxygen for SAMpp from the source code.
