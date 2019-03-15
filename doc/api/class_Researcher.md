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

void **performResearch** ()\

void **publishResearch** ()\

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

Author
======

Generated automatically by Doxygen for SAMpp from the source code.
