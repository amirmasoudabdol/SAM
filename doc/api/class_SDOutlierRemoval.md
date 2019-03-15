---
date: Fri Mar 15 2019
parent: API
section: 3
title: SDOutlierRemoval
---

NAME
====

SDOutlierRemoval

SYNOPSIS
========

\

`#include <HackingStrategies.h>`

Inherits **HackingStrategy**.

Public Member Functions
-----------------------

**SDOutlierRemoval** (std::string mode, std::string level, std::string
order, int num, int n\_attempts, int max\_attempts, int
min\_observations, std::vector\< double \> multipliers)\

void **perform** (**Experiment** \*experiment, **DecisionStrategy**
\*decisionStrategy)\

Additional Inherited Members
----------------------------

Detailed Description
====================

Declaration of Outlier Removal hacking method based on items\' distance
from their sample mean.

`sd_multiplier` \...

    {
    "type": "SD Outlier Removal",
    "sd_multiplier": d
    }

Member Function Documentation
=============================

void SDOutlierRemoval::perform (**Experiment** \* experiment, **DecisionStrategy** \* decisionStrategy)` [virtual]`
-------------------------------------------------------------------------------------------------------------------

Implementation of Outliers Removal based on an item\'s distance from the
\$\$. The `_sd_multiplier`, \$d\$ is set at the construction, and it\'ll
be used to check whether a measurement should be removed or not. Any
item satisfying the \$ v \> \| - d \* igma\| \$ will be removed from the
dataset.

Implements **HackingStrategy**.

Author
======

Generated automatically by Doxygen for SAMpp from the source code.
