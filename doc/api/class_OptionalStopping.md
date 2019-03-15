---
date: Fri Mar 15 2019
section: 3
title: OptionalStopping
---

NAME
====

OptionalStopping

SYNOPSIS
========

\

`#include <HackingStrategies.h>`

Inherits **HackingStrategy**.

Public Member Functions
-----------------------

**OptionalStopping** (std::string mode, std::string level, int num, int
n\_attempts, int max\_attempts)\

void **perform** (**Experiment** \*experiment, **DecisionStrategy**
\*decisionStrategy)\

Additional Inherited Members
----------------------------

Detailed Description
====================

Declartion of **OptionalStopping** hacking strategy.

In order to apply the **OptionalStopping**, use the following format in
the JSON config file where `n` specify number of new observations that
is going to be added to the experiment in each trial, `m`.

    {
       "type": "Optional Stopping",
       "size": n,
       "attempts": m
    }

Member Function Documentation
=============================

void OptionalStopping::perform (**Experiment** \* experiment, **DecisionStrategy** \* decisionStrategy)` [virtual]`
-------------------------------------------------------------------------------------------------------------------

Implementation of optional stopping. This will use two parameters set at
construction of the **OptionalStopping** class, `_n_trials` and
`_n_new_obs` for every trial, the routine will add `_n_new_obs` to all
groups, recalculate the statistics, and run the test. It will then
select an outcome based on researcher\'s preference and check it\'s
significance. If the results is significant, it\'ll not make a new
attempt to add more data, and will return to the hack() routine.

Implements **HackingStrategy**.

Author
======

Generated automatically by Doxygen for SAMpp from the source code.
