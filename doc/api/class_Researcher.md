---
date: Fri Mar 15 2019
parent: API
section: 3
title: Researcher
---

NAME
====

Researcher

SYNOPSIS
========

\

Classes
-------

class **Builder**\

Public Member Functions
-----------------------

**Researcher** (json &config)\

**Researcher** (**Experiment** \*e)\

**Researcher** (**Experiment** \*e, **Journal** \*j,
**DecisionStrategy** \*ds, std::vector\< **HackingStrategy** \* \> hs,
bool ish)\

void **registerAHackingStrategy** (**HackingStrategy** \*h)\

void **hack** ()\

void **prepareResearch** ()\
Prepares the research by cleaning up the memeory, randomizing the
**ExperimentSetup** parameters, allocating data and finally generating
the data using the **DataGenStrategy**.

void **performResearch** ()\
Performs the research by calculating the statistics, calculating the
effects, and running the test. In the case where the researcher is a
hacker, the researcher will apply the hacking methods on the
`experiment`.

void **publishResearch** ()\
Prepares the submission record by asking the `decisionStrategy` to pick
his prefered submission record from the list of available submission,
`submissionsList`. AFter than, it\'ll submit the submission record to
the selected `journal`.

void **setDecisionStrategy** (**DecisionStrategy** \*d)\

void **prepareTheSubmission** ()\

void **submitToJournal** ()\

void **setJournal** (**Journal** \*j)\

Public Attributes
-----------------

**Experiment** \* **experiment**\

**Journal** \* **journal**\

**DecisionStrategy** \* **decisionStrategy**\

std::vector\< **HackingStrategy** \* \> **hackingStrategies**\

bool **isHacker** = false\

HackingStyle **hackingStyle** = onCopy\

**Submission** **submissionRecord**\

DecisionPreference **selectionPref** =
DecisionPreference::PreRegisteredOutcome\
By default, a researcher always prefer to return the pre-registered
result.

Author
======

Generated automatically by Doxygen for SAMpp from the source code.