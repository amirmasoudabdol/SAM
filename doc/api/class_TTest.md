---
date: Fri Mar 15 2019
parent: API
section: 3
title: TTest
---

NAME
====

TTest

SYNOPSIS
========

\

`#include <TestStrategy.h>`

Inherits **TestStrategy**.

Public Member Functions
-----------------------

**TTest** (TestSide side, double alpha)\

void **run** (**Experiment** \*experiment)\

Additional Inherited Members
----------------------------

Detailed Description
====================

Declration of t-test.

The `run()` method will check the significance of the difference between
two groups. In the current setup, every `experiment->means` is
considered an effect size between a treatment group and a control group
with the mean of zero. Therefore, computing the t-statistics and
computing the p-value would be sufficient. This is technically an
implementation of `one sample t-test`.

Author
======

Generated automatically by Doxygen for SAMpp from the source code.
