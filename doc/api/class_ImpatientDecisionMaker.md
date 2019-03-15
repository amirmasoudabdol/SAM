---
date: Fri Mar 15 2019
parent: API
section: 3
title: ImpatientDecisionMaker
---

NAME
====

ImpatientDecisionMaker

SYNOPSIS
========

\

`#include <DecisionStrategy.h>`

Inherits **DecisionStrategy**.

Public Member Functions
-----------------------

**ImpatientDecisionMaker** (DecisionPreference selection\_pref)\

**Submission** **selectOutcome** (**Experiment** &experiment)\

bool **isPublishable** (const **Submission** &sub)\

void **clearPools** ()\

bool **verdict** (**Experiment** &experiment, DecisionStage stage)\

Public Attributes
-----------------

std::vector\< **Submission** \> **submissionsPool**\

std::vector\< **Experiment** \> **experimentsPool**\

Additional Inherited Members
----------------------------

Detailed Description
====================

Implementation of an impatient researcher. In this case, the
**Researcher** will stop as soon as find a significant result and will
not continue exploring other hacking methods in his arsenal.

Author
======

Generated automatically by Doxygen for SAMpp from the source code.
