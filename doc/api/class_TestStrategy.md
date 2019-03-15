---
date: Fri Mar 15 2019
parent: API
section: 3
title: TestStrategy
---

NAME
====

TestStrategy - Abstract class for defining test strategies.

SYNOPSIS
========

\

`#include <TestStrategy.h>`

Inherited by **TTest**.

Public Member Functions
-----------------------

virtual void **run** (**Experiment** \*experiment)=0\

Static Public Member Functions
------------------------------

static **TestStrategy** \* **buildTestStrategy** (json &config)\

Detailed Description
====================

Abstract class for defining test strategies.

Statistical test strategies will investigate if there is a meaningful
difference between means of two samples. Every test strategy should
provide a `run()` method. The `run()` method will accept a pointer to
the experiment and update necessary variables, e.g., *statistics* &
*p-value*.

Author
======

Generated automatically by Doxygen for SAMpp from the source code.
