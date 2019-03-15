---
date: Fri Mar 15 2019
section: 3
title: 'Researcher::Builder'
---

NAME
====

Researcher::Builder

SYNOPSIS
========

\

Public Member Functions
-----------------------

**Builder** & **setConfig** (json &config)\

**Builder** & **makeExperimentSetup** ()\

**Builder** & **makeExperiment** ()\

**Builder** & **makeJournal** ()\

**Builder** & **makeDecisionStrategy** ()\

**Builder** & **isHacker** ()\

**Builder** & **makeHackingStrategies** ()\

**Builder** & **setResearcherPreference** (DecisionPreference pref)\

**Builder** & **setExperimentSetup** (**ExperimentSetup**)\

**Builder** & **setExperiment** (**Experiment**)\

**Builder** & **setDataStrategy** (**DataGenStrategy** \*dgs)\

**Builder** & **setTestStrategy** (**TestStrategy** \*ts)\

**Builder** & **setJournal** (**Journal** j)\

**Builder** & **setDecisionStrategy** (**DecisionStrategy**)\

**Builder** & **setHackingStrategy** (**HackingStrategy**)\

**Builder** & **setHackingStrategy** (std::vector\< **HackingStrategy**
\* \>)\

**Builder** & **chooseHackingStrategies** (std::vector\<
**HackingStrategy** \>)\

**Researcher** **build** ()\

Author
======

Generated automatically by Doxygen for SAMpp from the source code.
