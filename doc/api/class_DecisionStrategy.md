---
date: Fri Mar 15 2019
parent: API
section: 3
title: DecisionStrategy
---

NAME
====

DecisionStrategy

SYNOPSIS
========

\

`#include <DecisionStrategy.h>`

Inherited by **ImpatientDecisionMaker**, and
**ReportPreregisteredGroup**.

Public Member Functions
-----------------------

virtual **\~DecisionStrategy** ()=0\

virtual **Submission** **selectOutcome** (**Experiment** &experiment)=0\

virtual bool **verdict** (**Experiment** &, DecisionStage)=0\

**Submission** **\_select\_Outcome** (**Experiment** &experiment)\

Static Public Member Functions
------------------------------

static **DecisionStrategy** \* **buildDecisionStrategy** (json &config)\

Public Attributes
-----------------

DecisionPreference **selectionPref**\

bool **isStillHacking** = true\

int **preRegGroup** = 0\

**Submission** **finalSubmission**\

Detailed Description
====================

Abstract class for different decision strategies.

Constructor & Destructor Documentation
======================================

DecisionStrategy::\~DecisionStrategy ()` [pure virtual]`
--------------------------------------------------------

Pure deconstructor of **DecisionStrategy**. This is necessary for proper
deconstruction of derived classes.

Member Function Documentation
=============================

**Submission** DecisionStrategy::\_select\_Outcome (**Experiment** & experiment)
--------------------------------------------------------------------------------

Based on the DecisionPreference, it\'ll select the outcome between all
groups, `ng`. For instance, the MinPvalue deicison prefenrece will.

**Parameters:**

> *experiment* { parameter\_description }

**Returns:**

> A copy of the selected outcome

Author
======

Generated automatically by Doxygen for SAMpp from the source code.
