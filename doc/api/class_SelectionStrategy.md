---
date: Fri Mar 15 2019
parent: API
section: 3
title: SelectionStrategy
---

NAME
====

SelectionStrategy - Abstract class for **Journal**\'s selection
strategies.

SYNOPSIS
========

\

`#include <SelectionStrategies.h>`

Inherited by **SignigicantSelection**.

Public Member Functions
-----------------------

virtual bool **review** (**Submission** &s)=0\

Static Public Member Functions
------------------------------

static **SelectionStrategy** \* **buildSelectionStrategy** (json
&config)\

Detailed Description
====================

Abstract class for **Journal**\'s selection strategies.

A **Journal** will decide if a **Submission** is going to be accepted or
rejected. This decision can be made based on different criteria or
formula. A **SelectionStrategy** provides an interface for implementing
different selection strategies.

Author
======

Generated automatically by Doxygen for SAMpp from the source code.
